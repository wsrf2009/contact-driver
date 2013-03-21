/*
* Name: Card T0 source file
* Date: 2012/10/18
* Author: Alex Wang
* Version: 1.0
*/

#include <linux/delay.h>

#include "t0.h"
#include "debug.h"
#include "uart.h"
#include "param.h"
#include "icc.h"
#include "timer.h"

static unsigned char DispatchCardT0(unsigned char CardIdx, unsigned char *CmdBuf, unsigned int CmdLen, unsigned char *ResBuf, unsigned int *ResLen)
{
    unsigned char RWFlag;
    unsigned int LEN = *(CmdBuf + APDU_P3);
    unsigned char INS = *(CmdBuf + APDU_INS);
    unsigned char AckCmd;
    bool EndPollFlag = false;
    bool RecFlag = false;
    unsigned int DataCount = 0;
    unsigned int DataOffset = 0;
    unsigned char status[2];
    unsigned char ret;


    PrtMsg("%s: CardIdx = %d, CmdLen = %d\n", __FUNCTION__, CardIdx, CmdLen);

    udelay(CardParam[CardIdx].CurETU * 10);

    if(CmdLen > 5)
    {
        RWFlag = APDU_WRITE;
    }
    else
    {
        if(CmdLen == 4)    // case 1
        {
            CmdLen = 5;
            *(CmdBuf + APDU_P3) = 0x00;
            LEN = 0x00;
        }
        RWFlag = APDU_READ;
    }

    SC_TraMoreBytes(CardIdx, CmdBuf, APDU_HEADER);
    Set_WaiTingTime(CardParam[CardIdx].WT);

    do
    {
        ret = SC_RecMoreBytes(CardIdx, &AckCmd, 1);
        udelay(CardParam[CardIdx].CurETU * 10);
        if(ret != OK)
        {
            EndPollFlag = true;
        }
        else if(AckCmd == INS)    // INS
        {
            // the data transfer of all remaining data bytes(if any bytes remain)
            if(RWFlag == APDU_WRITE)
            {
                SC_TraMoreBytes(CardIdx, CmdBuf + APDU_HEADER + DataOffset, LEN - DataOffset);
                RecFlag = false;
            }
            else
            {
                if(LEN == 0)
                 {
                    DataOffset = 256 - DataCount;
                }
                else
                {
                    DataOffset = (unsigned int)LEN - DataCount;
                }

                ret = SC_RecMoreBytes(CardIdx, ResBuf+ DataCount, DataOffset);

                RecFlag = true;
                DataCount += DataOffset;
            }
        }
        else if((AckCmd & 0xF0) == 0x90)    // '9X', SW1 byte
        {
            // the reception of a SW2 byte
            status[0] = AckCmd;
            *(ResBuf + (DataCount++)) = AckCmd;
            ret = SC_RecMoreBytes(CardIdx, &AckCmd, 0x01);
            RecFlag = true;
            status[1] = AckCmd;
            *(ResBuf+ (DataCount++)) = AckCmd;
            EndPollFlag = true;
        }
        else if((AckCmd & 0xF0) == 0x60)
        {
            if(AckCmd != 0x60)           // '6X', SW1 byte
            { 
                // the reception of a SW2 byte
                status[0] = AckCmd;
                ret = SC_RecMoreBytes(CardIdx, &AckCmd, 0x01);
                RecFlag = true;
                if(ret == OK)
                {
                    status[1] = AckCmd;
                    *(ResBuf + (DataCount++)) = status[0];
                    *(ResBuf+ (DataCount++)) = status[1];
                    EndPollFlag = true;
                }
            }
            else                         // '60', NULL byte
            {
                // the reception of a procedure byte
            }
        }
        else if(AckCmd == (INS ^ 0xFF))    // INS ^ 0xFF
        {
            // the data transfer of the next data byte(if exists)
            if(RWFlag == APDU_WRITE)
            {
                SC_TraMoreBytes(CardIdx, CmdBuf + APDU_HEADER + (DataOffset++), 0x01);
                RecFlag = false;
            }
            else
            {
                ret = SC_RecMoreBytes(CardIdx, ResBuf+ (DataOffset++), 0x01);
                RecFlag = true;
            }
        }
        else
        {
            ret = PROCEDURE_BYTE_CONFLICT;
        }

        if(ret != OK)
        {
            EndPollFlag = true;

            if(ret == SC_PARITY_ERROR)
            {
                if(RecFlag == true)
                {
                    udelay(CardParam[CardIdx].CurETU);
                    SC_PowerOff(CardIdx);
                }
            }
        }
    }while(EndPollFlag == false);
    
    *ResLen = DataCount;

    return(ret);
}


unsigned char ParseCardT0(unsigned char CardIdx, unsigned char *CmdBuf, unsigned int CmdLen, unsigned char *ResBuf, unsigned int *ResLen)
{
    unsigned char ret;


    PrtMsg("%s: CardIdx = %d, CmdLen = %d\n", __FUNCTION__, CardIdx, CmdLen);

    ret = DispatchCardT0(CardIdx, CmdBuf, CmdLen, ResBuf, ResLen);
    PrtMsg("%s: *ResLen = %d\n",__FUNCTION__,*ResLen);
    return(ret);
}


