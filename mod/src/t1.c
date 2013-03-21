/*
* Name: Card T1 source file
* Date: 2012/10/18
* Author: Alex Wang
* Version: 1.0
*/

#include <linux/string.h>
#include <linux/delay.h>

#include "t1.h"
#include "debug.h"
#include "param.h"
#include "uart.h"
#include "common.h"

T1Header IFDHeader;
T1Header ICCHeader;
unsigned char T1_TempBlock[11];


static unsigned char T1_xfrTPDU(unsigned char CardIdx, unsigned char *CmdBuf, unsigned int CmdLen, unsigned char *ResBuf, unsigned int *ResLen)
{
    unsigned int i;
    unsigned int Length = CmdLen;
    unsigned char* pCmd = CmdBuf;
    unsigned char* pBuf = ResBuf;
    unsigned char ret;
    bool ParityErrorFlag = false;


    PrtMsg("%s: CardIdx = %d, CmdLen = %d\n", __FUNCTION__, CardIdx, CmdLen);

    for(i = 0; i < 20; i++)
    {
        udelay(CardParam[CardIdx].CurETU);
    }

    SC_TraMoreBytes(CardIdx, pCmd, Length);

    for(i = 0, Length = 3; i < Length; i++)
    {
        ret = SC_RecMoreBytes(CardIdx, pBuf++, 1);
        if(ret != OK)
        {
            if(ret != SC_PARITY_ERROR)
            {
                *ResLen = i;
                return(ret);
            }
            else
            {
                ParityErrorFlag = true;
            }
        }
        if(i == 0)
        {
            
        }
        else if(i == 2)
        {
            Length = ResBuf[2] + CardParam[CardIdx].CheckSum + 3;
        }
    }
  
    *ResLen = i;
    if( ParityErrorFlag == true)
    {
        ret = SC_PARITY_ERROR;
    }

    return(ret);
}

static unsigned char T1_TPDUExchange_ErrorCheck(unsigned char CardIdx, unsigned char *IFD_DataBuf, unsigned char *Card_DataBuf)
{
    unsigned char* pSend = IFD_DataBuf - 3;
    unsigned char* pRec = Card_DataBuf - 3;
    unsigned char TempSendBlock[3];
    unsigned char TempRecBlock[3];
    unsigned int i;
    unsigned char CheckSum = 0;
    unsigned char CurSaveByte = 0;
    unsigned char ret;
    unsigned int TempLen=0;

 
    PrtMsg("%s: CardIdx = %d\n", __FUNCTION__, CardIdx);

    memcpy(TempSendBlock, pSend, 3);
    memcpy(TempRecBlock, pRec, 3);

    if((IFDHeader.PCB & 0x80) == 0x00)        // bit7: 0, I-block
    {
        // bit6 in PCB: send-sequence number
        if(BITisSET(CardParam[CardIdx].T1ExChangeFlag, IFDBlockNumber))    // IFD block number
        {
            IFDHeader.PCB |= 0x40;
        }
        else
        {
            IFDHeader.PCB &= 0xBF;
        }
        CardParam[CardIdx].T1ExChangeFlag ^= 0x01;    // 
    }
    else if((IFDHeader.PCB & 0xC0) == 0x80)   // bit7 = 1, bit6 = 0: R-block
    {
        if(BITisSET(CardParam[CardIdx].T1ExChangeFlag, ICCBlockNumber))    // ICC block number
        {
            IFDHeader.PCB |= 0x10;
        }
        else
        {
            IFDHeader.PCB &= 0xEF;
        }
    }

    pSend[T1_NAD] = IFDHeader.NAD;
    pSend[T1_PCB] = IFDHeader.PCB;
    pSend[T1_LEN] = IFDHeader.LEN;

    for(i = 0; i < ((unsigned int)pSend[2] + 3); i++)
    {
        CheckSum ^= pSend[i];
    }
    if(BITisSET(CardParam[CardIdx].T1ExChangeFlag, IFDChainingBit))    // IFD chaining bit
    {
        CurSaveByte = pSend[i];
    }
    pSend[i++] = CheckSum;
    ret = T1_xfrTPDU(CardIdx, pSend,i, pRec, &TempLen);
    PrtMsg("%s: ret = %X\n",__FUNCTION__,ret);
    if(ret == OK)
    {
        ICCHeader.NAD = pRec[T1_NAD];
        ICCHeader.PCB = pRec[T1_PCB];
        ICCHeader.LEN = pRec[T1_LEN];
        if(ICCHeader.NAD == 0x00)
        {
            if((ICCHeader.LEN != (TempLen - 4)) || (ICCHeader.LEN == 0xFF))
            {
                if(CardParam[CardIdx].FirstIBlock == 1)
                {
                    if((ICCHeader.PCB & 0x80) == 0x00)    // I-block
                    {
                        ret = SLOTERROR_ICC_MUTE;
                    }
                    else
                    {
                        ret = SLOTERROR_T1_OTHER_ERROR;
                    }
                }
                else
                {
                    ret = SLOTERROR_ICC_MUTE;
                }
            }
            else
            {
                CheckSum = 0x00;
                for(i = 0; i < TempLen; i++)
                {
                    CheckSum ^= pRec[i];
                }
                if(CheckSum)
                {
                    ret = SLOTERROR_T1_CHECKSUM_ERROR;
                }
            }
        }
        else
        {
            ret = SLOTERROR_BAD_NAD;
        }
    }
    if(ret == OK)
    {
        if((ICCHeader.PCB & 0xC0) == 0xC0)
        {
            if(ICCHeader.PCB == 0xC3)       // a WTX request
            {
                if(ICCHeader.LEN != 0x01)
                {
                    ret = SLOTERROR_BAD_LENTGH;
                }
            }   
            else if(ICCHeader.PCB == 0xC1)    // a IFS request
            {
                if(ICCHeader.LEN != 1)
                {
                    ret = SLOTERROR_BAD_LENTGH;
                }
                else if((pRec[3] < 0x10) || (pRec[3] > 0xFE))
                {
                    ret = SLOTERROR_BAD_IFSC;
                }
            }
            else if((ICCHeader.PCB == 0xC0) || (ICCHeader.PCB == 0xC2)) // a RESYNCH request or an ABORT request
            {
                if(ICCHeader.LEN != 0)
                {
                    ret = SLOTERROR_T1_OTHER_ERROR;
                }
            }
            else if(ICCHeader.PCB == 0xE3)    // a WTX response
            {
                if(ICCHeader.LEN != 1)
                {
                    ret = SLOTERROR_BAD_LENTGH;
                }
                else if((IFDHeader.PCB & 0xC0) != 0xC0)
                {
                    ret = SLOTERROR_INVALIDBLOCK;
                }
            }
            else if(ICCHeader.PCB == 0xE1)    // a IFS response
            {
                if(ICCHeader.LEN != 1)
                {
                    ret = SLOTERROR_BAD_LENTGH;
                }
                else if((IFDHeader.PCB & 0xC0) != 0xC0)
                {
                    ret = SLOTERROR_INVALIDBLOCK;
                }
            }
            else if((ICCHeader.PCB == 0xE0) || (ICCHeader.PCB == 0xE2))    // a RESYNCH response or an ABORT response
            {
                if(ICCHeader.LEN != 0)
                {
                    ret = SLOTERROR_BAD_LENTGH;
                }
                else if((IFDHeader.PCB & 0xC0) != 0xC0)
                {
                    ret = SLOTERROR_INVALIDBLOCK;
                }
            }
            else
            {
                ret = SLOTERROR_INVALIDBLOCK;
            }
        }
        else if((ICCHeader.PCB & 0xC0) == 0x80)
        {
            if(ICCHeader.LEN != 0)
            {
                ret = SLOTERROR_BAD_LENTGH;
            }
            else if((ICCHeader.PCB & 0x2C) != 0)
            {
                ret = SLOTERROR_INVALIDBLOCK;
            }
        }
    }
    if(BITisSET(CardParam[CardIdx].T1ExChangeFlag, IFDChainingBit))    // IFD chaining bit
    {
        pSend[i-1] = CurSaveByte;
    }
    if((ret != OK) || ((ICCHeader.PCB & 0x80) != 0x00) || (pSend < pRec) || (pSend > (Card_DataBuf + ICCHeader.LEN)))
    {
        memcpy(pSend, TempSendBlock, 3);
    }
    memcpy(pRec, TempRecBlock, 3);
    return(ret);
}

static unsigned char T1_BlockTransceive_ErrorHandle(unsigned char CardIdx, unsigned char *CmdBuf, unsigned char *ResBuf)
{
    unsigned char ret;
    unsigned char prologBuf[3];
    unsigned char BlockHeadBuf[5];
    unsigned char NumRetry = 0;
    unsigned char NumSReq = 0;
    unsigned char NumResend = 0;
    unsigned char lastBlockError = OK;


    PrtMsg("%s: CardIdx = %d\n", __FUNCTION__, CardIdx);

    prologBuf[0] = IFDHeader.NAD;
    prologBuf[1] = IFDHeader.PCB;
    prologBuf[2] = IFDHeader.LEN;
	
    memcpy(BlockHeadBuf, CmdBuf, 5);
	
    do
    {
        ret = T1_TPDUExchange_ErrorCheck(CardIdx, CmdBuf, ResBuf);
        PrtMsg("%s: ret = %X\n",__FUNCTION__, ret);
        do
        {
            
            if(ret == OK)
            {
                if((ICCHeader.PCB & 0xC0) == 0xC0)    // S-Block received
                {
                    if(ICCHeader.PCB == 0xC1)
                    {
                        CardParam[CardIdx].IFSC = *ResBuf;
                    }
                    else if(ICCHeader.PCB == 0xC2)
                    {
                        return(SLOTERROR_ICC_ABORT); 
                    }
                    else if((ICCHeader.PCB != 0xC0) && (ICCHeader.PCB != 0xC3))
                    {
                        ret = SLOTERROR_T1_OTHER_ERROR;
                        break;
                    }
                    IFDHeader.PCB = ICCHeader.PCB | 0x20;
                    IFDHeader.LEN = ICCHeader.LEN;
                    *CmdBuf = *ResBuf;
                    NumSReq++;
                    if(NumSReq >= 5)
                    {
                        return(SLOTERROR_ICC_ABORT);
                    }
                }
                else if((ICCHeader.PCB & 0xC0) == 0x80)  // R-Block received
                {
                    if((BITisSET(CardParam[CardIdx].T1ExChangeFlag, IFDBlockNumber) && (ICCHeader.PCB & 0x10)) ||  // IFDBlockNum && R-block with a error
                     (BITisCLEAR(CardParam[CardIdx].T1ExChangeFlag, IFDBlockNumber) && ((ICCHeader.PCB & 0x10) == 0x00)))
                    {
                        if(BITisSET(CardParam[CardIdx].T1ExChangeFlag, ICCChainingBit))    // ICC chaining bit
                        {
                            NumResend++;             // resend last block
                            if(NumResend >= 3)
                            {
                                NumRetry = 3;
                            }
                            break;
                        }
                        if(BITisSET(CardParam[CardIdx].T1ExChangeFlag, IFDChainingBit))    // IFD chaining bit
                        {
                            NumRetry |= 0x80;
                        }
                        else
                        {
                            ret = SLOTERROR_T1_OTHER_ERROR;
                        }
                    }
                    else
                    {
                        PrtMsg("+++++++++++++++++++++++++++++++++++++++++++++++\n");
                        IFDHeader.NAD = prologBuf[0];
                        IFDHeader.PCB = prologBuf[1];
                        IFDHeader.LEN = prologBuf[2];
                        memcpy(CmdBuf, BlockHeadBuf, 5);
                        NumResend++;
                        if(NumResend >= 3)
                        {
                            NumResend = 3;
                        }
                        else
                        {
                            if((IFDHeader.PCB & 0x80) == 0x00)
                            {
                                CardParam[CardIdx].T1ExChangeFlag ^= 0x01;    // IFD block number
                                CardParam[CardIdx].FirstIBlock = 0;
                            }
                        }
                    }
                }
                else                                                    // I-Block received
                {
                    if(BITisSET(CardParam[CardIdx].T1ExChangeFlag, IFDChainingBit))       // IFD chaining bit
                    {
                        ret = SLOTERROR_T1_OTHER_ERROR;
                        break;
                    }
                    if((BITisSET(CardParam[CardIdx].T1ExChangeFlag,ICCBlockNumber) && (ICCHeader.PCB & 0x40)) ||   // ICCBlockNum && 
                    	((BITisCLEAR(CardParam[CardIdx].T1ExChangeFlag,ICCBlockNumber) && ((ICCHeader.PCB & 0x40) == 0x00))))
                    {
                        if(ICCHeader.PCB & 0x20)
                        {
                            SET_BIT(CardParam[CardIdx].T1ExChangeFlag, ICCChainingBit);    // set ICC chaining bit
                        }
                        else
                        {
                            CLEAR_BIT(CardParam[CardIdx].T1ExChangeFlag, ICCChainingBit);    // reset ICC chaining bit
                        }
                        NumRetry |= 0x80;
                        CardParam[CardIdx].T1ExChangeFlag ^= 0x02;
                    }
                    else
                    {
                        ret = SLOTERROR_INVALIDBLOCK;
                    }
                }
            }
        }while(0);

	if(ret == SLOTERROR_T1_CHECKSUM_ERROR)
        {
            ret = SLOTERROR_XFR_PARITY_ERROR;
        }
        if(ret != OK)
        {
            if(ret != lastBlockError)
            {
                lastBlockError = ret;
            }
            if(ret != SLOTERROR_XFR_PARITY_ERROR)
            {
                ret = SLOTERROR_T1_OTHER_ERROR;
            }
        }
        else
        {
            lastBlockError = OK;
        }

        if(ret != OK)
        {
            if((lastBlockError == SLOTERROR_XFR_PARITY_ERROR) || (lastBlockError == SLOTERROR_T1_OTHER_ERROR))
            {
                ret = lastBlockError;
            }
            else
            {
                lastBlockError = ret;
            }
        }
        else
        {
            lastBlockError = OK;
            break;
        }
        if((ret == SLOTERROR_T1_OTHER_ERROR) || (ret == SLOTERROR_XFR_PARITY_ERROR))
        {
            if(ret == SLOTERROR_XFR_PARITY_ERROR)
            {
                IFDHeader.PCB = 0x81;
            }
            else
            {
                IFDHeader.PCB = 0x82;
            }
            if(BITisSET(CardParam[CardIdx].T1ExChangeFlag, ICCChainingBit))    // ICC chaining bit
            {
                IFDHeader.PCB = 0x80;
            }
            IFDHeader.LEN = 0x00;
            NumRetry++;
        }
		PrtMsg("%s: NumRetry = %d\n",__FUNCTION__,NumRetry);
    }while(NumRetry < 3);

    CardParam[CardIdx].FirstIBlock = 0;
    if(NumRetry == 3)
    {
        NumRetry = 0;
        while(NumRetry < 3)
        {
            IFDHeader.NAD = 0x00;
            IFDHeader.PCB = 0xC0;
            IFDHeader.LEN = 0x00;
            ret = T1_TPDUExchange_ErrorCheck(CardIdx, CmdBuf, ResBuf);
            if(ret == OK)
            {
                if(ICCHeader.PCB == 0xE0)
                {
                    CLEAR_BIT(CardParam[CardIdx].T1ExChangeFlag, IFDBlockNumber);
					CLEAR_BIT(CardParam[CardIdx].T1ExChangeFlag, ICCBlockNumber);
                    ret = SLOTERROR_T1_3RETRY_FAIL_RESYNCH_PASS;
                    break;
                }
                else
                {
                    NumRetry++;
                }
            }
            else
            {
                NumRetry++;
            }
        }

        if(NumRetry == 3)
        {
            ret = SLOTERROR_T1_3RETRY_FAIL;
        }
    }
    return(ret);
}

static unsigned char DispatchCardT1(unsigned char CardIdx, unsigned char *CmdBuf, unsigned int CmdLen, unsigned char *ResBuf, unsigned int *ResLen)
{
    unsigned char NumRetry = 0;
    unsigned char ret = OK;
    unsigned char *pSend;
    unsigned char *pRec;
    unsigned int SendAPDULen;
    unsigned char TempIFSC;
    unsigned char IFDChangeTempBlock;

    
    PrtMsg("%s: CardIdx = %d, CmdLen = %d\n", __FUNCTION__, CardIdx, CmdLen);

    if(BITisSET(CardParam[CardIdx].T1ExChangeFlag, FirstAPDUBit))    // first APDU ?
    {
        CardParam[CardIdx].FirstIBlock = 0;
        CardParam[CardIdx].T1ExChangeFlag = 0;
        while(NumRetry < 3)
        {
            IFDHeader.NAD = 0x00;    // the addressing is not used
            IFDHeader.PCB = 0xC1;
            IFDHeader.LEN = 0x01;
            T1_TempBlock[3] = 0xFE;
            ret = T1_TPDUExchange_ErrorCheck(CardIdx, T1_TempBlock + 3, T1_TempBlock+3);
            if(ret == OK)
            {
                if((ICCHeader.PCB != 0xE1) || (T1_TempBlock[3] != 0xFE))
                {
                    ret = SLOTERROR_T1_OTHER_ERROR;
                }
            }
            if(ret != OK)
            {
                NumRetry++;
            }
            else
            {
                break;
            }
        }
        if(NumRetry == 3)
        {
            ret = SLOTERROR_T1_3RETRY_FAIL;
        }
        udelay(CardParam[CardIdx].CurETU);
        CardParam[CardIdx].FirstIBlock = 1;
    }

APDUExchangeBegin:
    pSend = CmdBuf;
    pRec = ResBuf;
    SendAPDULen = CmdLen;
    memcpy(T1_TempBlock, pSend, 10);
    TempIFSC = CardParam[CardIdx].IFSC;
    while(SendAPDULen)
    {
        CLEAR_BIT(CardParam[CardIdx].T1ExChangeFlag, ICCChainingBit);    // reset ICC chaining bit
        if(SendAPDULen <= TempIFSC)
        { 
            PrtMsg("%s: T1ExChangeFlag = %X\n", __FUNCTION__, CardParam[CardIdx].T1ExChangeFlag);
            CLEAR_BIT(CardParam[CardIdx].T1ExChangeFlag, IFDChainingBit);    // reset IFD chaining bit
            IFDHeader.NAD = 0x00;
            IFDHeader.PCB = 0x00;
            IFDHeader.LEN = (unsigned char)SendAPDULen;
            ret = T1_BlockTransceive_ErrorHandle(CardIdx, pSend, pRec);
            CardParam[CardIdx].FirstIBlock = 0;
            SendAPDULen = 0;
            if(ret == SLOTERROR_T1_3RETRY_FAIL_RESYNCH_PASS)
            {
                NumRetry++;
                if(NumRetry < 3)
                {
                    memcpy(CmdBuf, T1_TempBlock, 10);
                    pSend = CmdBuf;
                    pRec = ResBuf;
                    SendAPDULen = CmdLen;
                }
            }
            else if(ret == OK)
            {
                *ResLen = ICCHeader.LEN;
            }
            else
            {
                CLEAR_BIT(CardParam[CardIdx].T1ExChangeFlag, ICCChainingBit);    // reset ICC chaining bit;
            }
        }
        else
        {
            SET_BIT(CardParam[CardIdx].T1ExChangeFlag, IFDChainingBit);    // set IFD chaining bit
            IFDHeader.NAD = 0x00;
            IFDHeader.PCB = 0x20;
            IFDHeader.LEN = TempIFSC;
            IFDChangeTempBlock = pSend[TempIFSC];
            ret = T1_BlockTransceive_ErrorHandle(CardIdx, pSend, pRec);
            pSend[TempIFSC] = IFDChangeTempBlock;
            CardParam[CardIdx].FirstIBlock = 0;
            CLEAR_BIT(CardParam[CardIdx].T1ExChangeFlag, IFDChainingBit);    // reset IFD chaining bit
            if(ret == SLOTERROR_T1_3RETRY_FAIL_RESYNCH_PASS)
            {
                NumRetry++;
                if(NumRetry < 3)
                {
                    pSend = CmdBuf;
                    pRec = ResBuf;
                    memcpy(CmdBuf, T1_TempBlock, 10);
                    SendAPDULen = CmdLen;
                }
            }
            else if(ret == OK)
            {
                SendAPDULen -= TempIFSC;
                pSend += TempIFSC;
            }
            else
            {
                SendAPDULen = 0;
                CLEAR_BIT(CardParam[CardIdx].T1ExChangeFlag, ICCChainingBit);    // reset ICC chaining bit
            }
        }
    }
    while(CardParam[CardIdx].T1ExChangeFlag & 0x08)
    {
        IFDHeader.NAD = 0x00;
        IFDHeader.PCB = 0x80;
        IFDHeader.LEN = 0x00;
        pRec += ICCHeader.LEN;
        ret = T1_BlockTransceive_ErrorHandle(CardIdx, pRec, pRec);
        if(ret == SLOTERROR_T1_3RETRY_FAIL_RESYNCH_PASS)
        {
            NumRetry++;
            if(NumRetry < 3)
            {
                memcpy(CmdBuf, T1_TempBlock, 10);
                goto APDUExchangeBegin;
            }
            CardParam[CardIdx].T1ExChangeFlag &= ~0x08;    // reset ICC chaining bit
        }
        else if(ret == OK)
        {
            *ResLen += ICCHeader.LEN;
        }
        else
        {
            CardParam[CardIdx].T1ExChangeFlag &= ~0x08;    // reset ICC chaining bit
        }
    }
    return(ret);
}

unsigned char ParseCardT1(unsigned char CardIdx, unsigned char *CmdBuf, unsigned int CmdLen, unsigned char *ResBuf, unsigned int *ResLen)
{
    unsigned char ret = OK;

    PrtMsg("%s: CardIdx = %d, CmdLen = %d\n", __FUNCTION__, CardIdx, CmdLen);
    ret = DispatchCardT1(CardIdx, CmdBuf, CmdLen, ResBuf, ResLen);
    return(ret);
}

