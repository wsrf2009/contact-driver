/*
* Name: Smart card source file
* Date: 2012/10/08
* Author: Alex Wang
* Version: 1.0
*/

#include <linux/delay.h>
#include <linux/slab.h> 

#include "debug.h"
#include "gpio.h"
#include "at83c26.h"
#include "atr.h"
#include "param.h"
#include "icc.h"
#include "uart.h"
#include "t1.h"
#include "t0.h"
#include "timer.h"
#include "common.h"

static const unsigned char at_to_78[] = {CLASS_C, CLASS_B, CLASS_A};

int SC_PowerOn(unsigned char CardIdx, unsigned char *AtrBuf, unsigned int *AtrLen)
{
    unsigned char CardVoltage = V_CLASS_C;
    unsigned char ret;
    unsigned char i;


    PrtMsg("welcome to the function: %s\n", __FUNCTION__);
    do
    {

        PrtMsg("welcome to the do-while cycle\n");
        TransParam_Init(CardIdx);
        Set_Transparam(CardIdx);

        Card_ColdReset(CardIdx, CardVoltage);
        ret = Card_GetATR(CardIdx, AtrBuf, AtrLen);
        if(ret == OK)
        {
            Card_AnalyzeATR(CardIdx, AtrBuf, AtrLen);
            if(CardParam[CardIdx].Class)
            {
                // ATR carries a class indicator 
                if(CardParam[CardIdx].Class & at_to_78[CardVoltage])
                {
                    // the class being applied, normal operation continue
                    break;
                }
                else
                {
                    // the class not being applied, the interface device may perform a deactivation
                }
            }
            else
            {
                // ATR carries no class indicator, the interface devices shall maintain the current class
                break;
            }
        }
        else
        {
            //  if the card does not answer to reset, then the interface device shall perform a deactivation
        }

        SC_PowerOff(CardIdx);
        mdelay(10);
        CardVoltage++;
        
    }while(CardVoltage <= V_CLASS_A);

    if(ret != OK)
    {
        PrtMsg("%s: Fail to power on the card!\n", __FUNCTION__);
        return(-1);           // accroding to 6.2.4
    }

    Set_Transparam(CardIdx);
    SET_BIT(CardParam[CardIdx].T1ExChangeFlag, FirstAPDUBit);        // set the first APDU bit
    SET_BIT(CardParam[CardIdx].CardStatus, ICC_ACTIVE_BIT);

    for(i = 0; i < *AtrLen; i++ )
    {
        PrtMsg("AtrBuf[%d] = %X\n", i, *(AtrBuf+i));
        PrtMsg("Invervalue = %X\n",Byte_Direct2Inverse(*(AtrBuf+i)));
    }
    
    return(0);
}

int SC_PowerOff(unsigned char CardIdx)
{

    AT83C26_CRSTx(CardIdx, 0);
    if(AT83C26_CCLKx(CardIdx, 0, 0))
    {
        return(-1);
    }
    AT83C26_CIOx(CardIdx, CIO_DISCON, CIO_LOW);
    if(AT83C26_CVCCx(CardIdx, V_CLASS_0))
    {
        return(-1);
    }
    mdelay(10);
    CLEAR_BIT(CardParam[CardIdx].CardStatus, ICC_ACTIVE_BIT);
    AT83C26_CIOx(CardIdx, CIO_CON, CIO_LOW);
    mdelay(20);
   
    return(0);
}

unsigned char SC_TransAPDU(unsigned char CardIdx, unsigned char *CmdBuf, unsigned int CmdLen, unsigned char *ResBuf, unsigned int *ResLen)
{
    PrtMsg("%s: CardIdx = %d, CmdLen = %d\n", __FUNCTION__, CardIdx, CmdLen);

    if(BITisSET(CardParam[CardIdx].CardStatus, ICC_ACTIVE_BIT))
    {
        if(CardParam[CardIdx].T== T0_TYPE)
        {
            return(ParseCardT0(CardIdx, CmdBuf, CmdLen, ResBuf, ResLen));
        }
        else
        {
            return(ParseCardT1(CardIdx, CmdBuf, CmdLen, ResBuf, ResLen));
        }
    }
    else
    {
        PrtMsg("%s: The card has not being activated\n",__FUNCTION__);
        return(-1);
    }
}


int SC_Device_Init(void)
{
    PrtMsg("welcome to entry the Function: %s\n",__FUNCTION__);

    CardParam = (CARD_PARAM*)kmalloc(5 * sizeof(CARD_PARAM), GFP_KERNEL);

    memset(CardParam, 0, 5 * sizeof(CARD_PARAM));

    if(Uart_Init())    
    {
        return(-1);
    }

    if(AT83C26_Init())
    {
        goto err1;
    }
    
    Waitingtimer_Init(); 

    return(0);

err1:
    kfree(CardParam);
    Uart_Uninit();
    
    return(-1);
}

void SC_Device_Uninit(void)
{
    kfree(CardParam);
    
    WaitingTimer_Uninit();

    AT83C26_Uninit();

    Uart_Uninit();
}
