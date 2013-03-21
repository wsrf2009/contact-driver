/*
* Name: 7816 protocol source file
* Date: 2012/10/15
* Author: Alex Wang
* Version: 1.0
*/

#include <linux/string.h>

#include "param.h"
#include "debug.h"
#include "icc.h"
#include "common.h"
#include "atr.h"
#include "uart.h"

CARD_PARAM* CardParam=NULL;

void TransParam_Init(unsigned char CardIdx)
{
    PrtMsg("welcome to the function: %s, CardIdx = %X\n", __FUNCTION__, CardIdx);

    CLEAR_BIT(CardParam[CardIdx].CardStatus, ICC_ACTIVE_BIT);

    CardParam[CardIdx].Mode       = NEGOTIABLE_MODE;
    CardParam[CardIdx].ClkStop    = NOT_SUPPORT_CLK_STOP;
    CardParam[CardIdx].Class      = CLASS_DEFAULT;
    CardParam[CardIdx].C6Use      = C6_UNUSED;
    CardParam[CardIdx].T          = T0_TYPE;
    CardParam[CardIdx].FiDi       = DEFAULT_FIDI;
    CardParam[CardIdx].CurFiDi    = DEFAULT_FIDI;
    CardParam[CardIdx].Direct     = DIRECT_CONVENTION;
    CardParam[CardIdx].N          = DEFAULT_EXTRA_GUARDTIME;

    CardParam[CardIdx].WI         = DEFAULT_WAITING_TIME;
    CardParam[CardIdx].Check      = DEFAULT_LRC;
    CardParam[CardIdx].BWI        = DEFAULT_BWI;
    CardParam[CardIdx].CWI        = DEFAULT_CWI;
    CardParam[CardIdx].IFSC       = DEFAULT_IFSC;
    CardParam[CardIdx].NAD        = DEFAULT_NAD;

    CardParam[CardIdx].GT         = DEFAULT_GUARDTIME;

}

void Set_Transparam(unsigned char CardIdx)
{
    unsigned int fi;
    unsigned int di;
    unsigned long defCWT;
    unsigned long defWT;

   
    PrtMsg("welcome to the function: %s, CardIdx = %X\n", __FUNCTION__, CardIdx);

    if(CardParam[CardIdx].Mode == SPECIFIC_MODE)
    {
        CardParam[CardIdx].CurFiDi = CardParam[CardIdx].FiDi;
    }
    fi = FI[(CardParam[CardIdx].CurFiDi & 0xF0) >> 4];
    di = DI[CardParam[CardIdx].CurFiDi & 0x0F];    
    CardParam[CardIdx].CurETU =  Caculate_Etu(CardParam[CardIdx].CurFiDi);
    Set_Baudrate(Caculate_BuadRate(CardParam[CardIdx].CurFiDi));   // set default fidi = 11

    if(CardParam[CardIdx].T == T0_TYPE)
    {   // waiting time: WT(us) = (WI * 960 * Fi )/ f     refer to 10.2 in ISO/IEC 78163
        CardParam[CardIdx].WT = ((960 * CardParam[CardIdx].WI * fi) / (FRE_CNTACARD));  // WT = (960 * WI *Fi) / f
    }
    else
    {
        if(CardParam[CardIdx].N == 0xFF)    // refer to 11.2 in ISO/IEC 7816-3
        {
            CardParam[CardIdx].GT -= 1;
            CardParam[CardIdx].N = 0x00;
        }
        if(CardParam[CardIdx].Check == DEFAULT_LRC)
        {
            CardParam[CardIdx].CheckSum = 1;
        }
        else
        {
            CardParam[CardIdx].CheckSum = 2;
        }
        CardParam[CardIdx].BWT = 11 * CardParam[CardIdx].CurETU +  (((1 << CardParam[CardIdx].BWI) * 960 * 9600) / (FRE_CNTACARD * 1000000)); // BWT, refer to 11.4.3 in IEC/ISO 7816-3
        CardParam[CardIdx].CWT = (11 + (1 << CardParam[CardIdx].CWI)) * CardParam[CardIdx].CurETU;     // CWT(etu)=(11+2^CWT)

        defCWT = ((CardParam[CardIdx].GT + CardParam[CardIdx].N)) * CardParam[CardIdx].CurETU;
        if(CardParam[CardIdx].CWT < defCWT)
        {
            CardParam[CardIdx].CWT = defCWT;
        }
        CardParam[CardIdx].WT = CardParam[CardIdx].CWT * CardParam[CardIdx].CurETU;
    }
    
    defWT = DEFAULT_BLOCK_GUARDTIME * CardParam[CardIdx].CurETU;
    if(CardParam[CardIdx].WT < defWT)
    {
        CardParam[CardIdx].WT = defWT;
    }

    CardParam[CardIdx].ExtraGT = CardParam[CardIdx].GT + CardParam[CardIdx].N - 11;
}



