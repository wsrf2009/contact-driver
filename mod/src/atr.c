/*
* Name: ATR source file
* Date: 2012/10/18
* Author: Alex Wang
* Version: 1.0
*/

#include <linux/delay.h>

#include "atr.h"
#include "debug.h"
#include "common.h"
#include "uart.h"
#include "param.h"
#include "at83c26.h"
#include "icc.h"
#include "timer.h"

int Card_ColdReset(unsigned char CardIdx, unsigned char CardVoltage)
{
    PrtMsg("welcome to the function: %s, CardIdx = %X, CardVoltage = %X\n", __FUNCTION__, CardIdx, CardVoltage);

    if(AT83C26_CVCCx(CardIdx, CardVoltage))
    {
        return(-1);
    }
    Put_Uart_OutLine_Normal();
    if(AT83C26_CCLKx(CardIdx, 1, 0))
    {
        return(-1);
    }
    udelay(400 / FRE_CNTACARD);
    CLearFIFO();
    if(AT83C26_CRSTx(CardIdx, 1)) 
    {
        return(-1);
    }
    Set_WaiTingTime(40000 / FRE_CNTACARD);
    // ignore the data accroding to 6.2.2 in ISO/IEC 7816-3
    
    return(0);
}

void Card_WarmReset(unsigned char CardIdx)
{
    PrtMsg("welcome to the function: %s, CardIdx = %X\n", __FUNCTION__, CardIdx);
    AT83C26_CRSTx(CardIdx, 0);
    udelay(400 / FRE_CNTACARD);
    CLearFIFO();
    AT83C26_CRSTx(CardIdx, 1);
    Set_WaiTingTime(40000 / FRE_CNTACARD);
    // ignore the data accroding to 6.2.3 in ISO/IEC 7816-3
}

int Card_GetATR(unsigned char CardIdx, unsigned char *AtrBuf, unsigned int *AtrLen)
{
    unsigned char RecLen;
    unsigned char AtrBufIdx;
    unsigned char HisBytes;
    unsigned char CheckCharacter=0;
    bool LastTDi = false;

    
    PrtMsg("welcome to the function: %s\n", __FUNCTION__);
    // receive TS
    AtrBufIdx = 0;
    if(SC_RecMoreBytes(CardIdx, (AtrBuf + AtrBufIdx), 1) == SC_PARITY_ERROR)    // parity error
    {
        if(AtrBuf[0] == 0x03)    // inverse convention
        {
            AtrBuf[0] = 0x3F;
            CardParam[CardIdx].Direct = INVERSE_CONVENTION;
        }
        else
        {
            PrtMsg("%s:Fail to get ATR with TS error\n",__FUNCTION__);
            return(-1);
        }
    }
    else if(AtrBuf[0] == 0x3B)
    {
        CardParam[CardIdx].Direct = DIRECT_CONVENTION;
    }
    else if(AtrBuf[0] == 0x03)
    {
        AtrBuf[0] = 0x3F;
        CardParam[CardIdx].Direct = INVERSE_CONVENTION;
    }
    else
    {
        PrtMsg("%s:Fail to get ATR with Bad ATR, AtrBuf[0] = %X\n", __FUNCTION__, AtrBuf[0]);
        return(-1);
    }
    AtrBufIdx++;

    // receive T0
    if(SC_RecMoreBytes(CardIdx, AtrBuf + AtrBufIdx, 1))
    {
        return(-1);
    }
    AtrBufIdx++;
    RecLen = 0;
    if(AtrBuf[1] & 0x10)    RecLen++;
    if(AtrBuf[1] & 0x20)    RecLen++;
    if(AtrBuf[1] & 0x40)    RecLen++;
    if(AtrBuf[1] & 0x80)    RecLen++;
    HisBytes = AtrBuf[1] & 0x0F;

    // receive TA1,TB1,TC1,TD1
    if(RecLen)
    {
        if(SC_RecMoreBytes(CardIdx, AtrBuf + AtrBufIdx, RecLen))
  {
	    return(-1);
	}
    }
    AtrBufIdx += RecLen;

    if(AtrBuf[1] & 0x80)
    {
        do
        {
            if(AtrBuf[AtrBufIdx -1] & 0x0F)   
	    {
	        CheckCharacter = 1;
	    }

            RecLen = 0;
            if(AtrBuf[AtrBufIdx -1] & 0x10)
	    {
	        RecLen++;
	    }
            if(AtrBuf[AtrBufIdx -1] & 0x20)
	    {
	        RecLen++;
	    }
            if(AtrBuf[AtrBufIdx -1] & 0x40)
	    {
	        RecLen++;
	    }
            if(AtrBuf[AtrBufIdx -1] & 0x80)
	    {
	        RecLen++;
	    }
            else 
	    {
	        LastTDi = true;
	    }
            
            if(SC_RecMoreBytes(CardIdx, AtrBuf + AtrBufIdx, RecLen)) 
	    {
	        return(-1);
	    }
            AtrBufIdx += RecLen;

            if((RecLen == 0) || (LastTDi == true))
	    {
	        break;
	    }
        }while(AtrBufIdx <= MAXATRLEN);
    }

    if(SC_RecMoreBytes(CardIdx, AtrBuf + AtrBufIdx, HisBytes + CheckCharacter))
    {
        return(-1);
    }
    *AtrLen = AtrBufIdx + HisBytes + CheckCharacter;
    if(*AtrLen > MAXATRLEN)
    {
        PrtMsg("%s:Fail to get ATR with Length error\n",__FUNCTION__);
        return(-1);
    }

    if(CheckCharacter)
    {
        unsigned char CheckValue = 0;

        for(AtrBufIdx = 1; AtrBufIdx < *AtrLen; AtrBufIdx++)
	{
	    CheckValue ^= AtrBuf[AtrBufIdx];
	}
        if(CheckValue)
        {
            PrtMsg("%s:Fail to get ATR with TCK error\n",__FUNCTION__);
            return(-1);
        }
    }
    
    return(0);
}

static unsigned char GetATRBytePosition(unsigned char *AtrBuf, unsigned int *AtrLen, unsigned char ATRByte, unsigned char AtrByteNmbr)
{
    unsigned char TempByteNmbr = 1;
    unsigned char TempByte;
    unsigned char AtrLenWithoutHis;
    unsigned char AtrBufIdx = 1;

    
    PrtMsg("welcome to the function: %s\n", __FUNCTION__);
    TempByte = AtrBuf[AtrBufIdx];
    AtrLenWithoutHis = *AtrLen - (AtrBuf[1] & 0x0F); // Get the ATR length without the historical characters
    while(AtrBufIdx < AtrLenWithoutHis)              // find "ATRByte" in "AtrBuf"
    {
        if(TempByte & 0x10)                          // TAi present
        {
            if(ATRByte != CHARACTER_TA ||  AtrByteNmbr != TempByteNmbr) 
	    {
	        AtrBufIdx++;
	    }
            else
	    {
	        return(AtrBufIdx+1);
	    }
        }
        if(TempByte & 0x20)                          // TBi present
        {
            if(ATRByte != CHARACTER_TB ||  AtrByteNmbr != TempByteNmbr) 
	    {
	        AtrBufIdx++;
	    }
            else
	    {
	        return(AtrBufIdx+1);
	    }
        }
        if(TempByte & 0x40)                          // TCi present
        {
            if(ATRByte != CHARACTER_TC ||  AtrByteNmbr != TempByteNmbr) 
	    {
	        AtrBufIdx++;
	    }
            else
	    {
	        return(AtrBufIdx+1);
	    }
        } 
        if(TempByte & 0x80)                          // TDi present
        {
            if(ATRByte != CHARACTER_TD ||  AtrByteNmbr != TempByteNmbr)
            {
                AtrBufIdx++;
                TempByteNmbr++;
                TempByte = AtrBuf[AtrBufIdx];
            }
            else 
	    {
	        return(AtrBufIdx+1);
	    }
        }
        else 
	{
	    return(0);     
	}
    }
    
    return(0);
}

void Card_AnalyzeATR(unsigned char CardIdx, unsigned char *AtrBuf, unsigned int *AtrLen)
{
    unsigned char TempPosition;
    unsigned char TempByte;
    unsigned char TempSaveTD;
    unsigned char TempNewPosition;
    unsigned char TD15Position = 0;
    unsigned char TDSecondT1Position = 0;
    unsigned char TempNmbr = 2;
    unsigned char TempTA2 = 0;

    
    PrtMsg("welcome to the function: %s, CardIdx = %X\n", __FUNCTION__, CardIdx);
    TempPosition = GetATRBytePosition(AtrBuf, AtrLen, CHARACTER_TD, 1);
    if(TempPosition != 0)                                   // TD1 is present?
    {
        TempByte = AtrBuf[TempPosition] & 0x0F;
        CardParam[CardIdx].T = TempByte;
        TempSaveTD = TempByte;

        while(1)
        {
            TempNewPosition = GetATRBytePosition(AtrBuf, AtrLen, CHARACTER_TD, TempNmbr);
            if(TempNewPosition == 0)
	    {
	      break;
	    }

            TempByte = AtrBuf[TempNewPosition];      // TDi 
            if((TempByte & 0x0F) >= (TempSaveTD & 0x0F))    // Normal case the new T value should be higher than the previous one
            {
                TempNmbr++;
                TempSaveTD = TempByte;
            }
            else
            {
                PrtMsg("%s:Fail to analyze ATR with TD error\n", __FUNCTION__);
                break;
            }
            
            // T=15 is present
            if((TempByte & 0x0F) == 0x0F) 
	    {
	        TD15Position = TempNewPosition;
	    }
            // T=1 is present, but TDi >= TD2
            if((TempByte & 0x0F) == 0x01)
	    {
	        TDSecondT1Position = TempNewPosition;
	    }
        }
    }
    else 
    {
        CardParam[CardIdx].T = T0_TYPE;               // if TD1 is absent, then the only offer is T=0 
    }

    // analyze TA2
    TempPosition = GetATRBytePosition(AtrBuf, AtrLen, CHARACTER_TA, 2);    // Get TA2
    if(TempPosition)
    {   // TA2 is present
        TempTA2  = AtrBuf[TempPosition];
        CardParam[CardIdx].Mode = SPECIFIC_MODE;    // if TA2 is present in ATR, card in specific mode, refer to 6.3.1 in ISO/IEC 7816-3
        if((TempTA2 & 0x0F) < 2)
        {
            CardParam[CardIdx].T = TempTA2 & 0x0F;
        }
    }

    // analyze TA1
    TempPosition = GetATRBytePosition(AtrBuf, AtrLen, CHARACTER_TA, 1);    // Get TA1
    if(TempPosition)
    {
        CardParam[CardIdx].FiDi = AtrBuf[TempPosition];
        if(CardParam[CardIdx].Mode == SPECIFIC_MODE)
        {
            if(TempTA2 & 0x10)
            {
                CardParam[CardIdx].FiDi = DEFAULT_FIDI;
            }
        }
    }

    // analyze TC1, extra guard time
    TempPosition = GetATRBytePosition(AtrBuf, AtrLen, CHARACTER_TC, 1);    // Get TC1
    if(TempPosition)
    {
        CardParam[CardIdx].N = AtrBuf[TempPosition];
    }

    // analyze TC2, waiting time
    TempPosition = GetATRBytePosition(AtrBuf, AtrLen, CHARACTER_TC, 2);    // Get TC2
    if(TempPosition && (AtrBuf[TempPosition] != 0))
    {   // TC2 present and do not is 0x00, refer to 10.2 in ISO/IEC 7816-3
        CardParam[CardIdx].WI = AtrBuf[TempPosition];
    }  

    TempPosition = GetATRBytePosition(AtrBuf, AtrLen, CHARACTER_TD, 2);    // Get TD2 
    TempNewPosition = TempPosition;
    if(TempPosition)
    {
        if(AtrBuf[TempNewPosition] & 0x10)                 // TA3 is present
        {
            CardParam[CardIdx].IFSC = AtrBuf[++TempPosition];    // refer to 11.4.2 in ISO/IEC 7816-3
        }
        if(AtrBuf[TempNewPosition] & 0x20)                 // TB3 is present
        {
            // refer to 11.4.3 in ISO/IEC 7816-3
            TempByte = (AtrBuf[++TempPosition] & 0xF0) >> 4;
            if(TempByte < 0x0A)         // values A ~ F are reserved for future use
            {
                CardParam[CardIdx].BWI = TempByte;  
            }
            CardParam[CardIdx].CWI = AtrBuf[TempPosition] & 0x0F;
        }
        if(AtrBuf[TempNewPosition] & 0x40)                 // TC3 is present
        {
            CardParam[CardIdx].Check = AtrBuf[++TempPosition] & 0x01;    // refer to 11.4.4 in ISO/IEC 7816-3
        }
    }

    // T = 15 parse
    if(TD15Position != 0)
    {
        if((AtrBuf[TD15Position] & 0x10) == 0x10)    // the first TA for T=15
        {
            CardParam[CardIdx].ClkStop = (AtrBuf[TD15Position+1] & 0xC0) >> 6;   // Clock stop indicator (X)
            CardParam[CardIdx].Class   = AtrBuf[TD15Position+1] & 0x3F;    // class indicator (Y)
        }

        if((AtrBuf[TD15Position] & 0x20) == 0x20)    // the first TB for T=15
        {
            TempByte = AtrBuf[TD15Position+2];
            if(TempByte == 0)
	    {
                CardParam[CardIdx].C6Use = C6_UNUSED;
	    }
            else if((TempByte & 0x80) == 0x00)
	    {
	        CardParam[CardIdx].C6Use = C6_STANDARD_USE;
	    }
            else
	    {
	        CardParam[CardIdx].C6Use = C6_PROPRIETARY_USE;
	    }
        }
    }
}


