/*
* Name: AT83C26 source file
* Date: 2012/10/10
* Author: Alex Wang
* Version: 1.0
*/

//#include "/opt/acr910/2.6.37/arch/arm/mach-omap2/include/mach/gpio.h"
#include <linux/kernel.h>
#include <mach/gpio.h>
//#include <linux/gpio.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/string.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <asm/io.h>



#include "debug.h"
#include "at83c26.h"
#include "gpio.h"
#include "param.h"
#include "icc.h"
#include "uart.h"
#include "common.h"


static struct 
{
    struct i2c_client *client;
} AT83C26;

static const unsigned char SC1_IOon[] = {0x20};             // SC1_INTERFACE: connect CIO1 to IO1
static const unsigned char SC1_IOoff[] = {0x40};            // SC1_INTERFACE: connect CIO1 to CARDIO1 , and set it as 0

static const unsigned char SC1_CLK0RST0[] = {0x21};        // SC1_INTERFACE: Stop CCLK1 with low level, CRST1 = 0
static const unsigned char SC1_CLK1RST0[] = {0x03};        // SC1_INTERFACE: have CCLK1 runing,         CRST1 = 0
static const unsigned char SC1_CLK1RST1[] = {0x13};        // SC1_INTERFACE: have CCLK1 runing,         CRST1 = 1

static unsigned char VDCB_INITIAL[] = {0x03, 0x0F};

static unsigned char SC1_PWR[] = {0x80, 0x0F, 0x00, 0x00, 0x08};        // SC1_CFG0:SC1_CFG1:SC1_CFG2:SC1_CFG3:SC1_CFG4
static unsigned char SC2_PWR[] = {0x00, 0x00, 0x08};              // CMD:SC2_CFG0:SC2_CFG1:SC2_CFG2
static unsigned char SC3_PWR[] = {0x00, 0x08};                    // CMD:SC3_CFG0:SC3_CFG2
static unsigned char SC4_PWR[] = {0x00, 0x08};                    // CMD:SC4_CFG0:SC4_CFG2
static unsigned char SC5_PWR[] = {0x00, 0x08};                    // CMD:SC5_CFG0:SC5_CFG2

static const unsigned char SC1_INTFACE[] = {0x60};
static const unsigned char SCx_INTFACE[] = {0x00, 0x00, 0xF0};


unsigned int *Reg_83C26_RESET = NULL;
unsigned int *Reg_83c26_INT = NULL;

//extern int gpio_request(unsigned gpio, const char *label);
//extern int gpio_direction_input(unsigned gpio);
//extern void gpio_free(unsigned gpio);

int AT83C26_ReadCmd(int Cmd,  unsigned char *data, int size) 
{
    int ret;

    PrtMsg("%s with Cmd = %d\n", __FUNCTION__, Cmd);

    switch (Cmd)
    {
        case 1: if (i2c_master_send(AT83C26.client, "\xFC", 1) < 0) return(-1); break;
        case 2: if (i2c_master_send(AT83C26.client, "\xF8", 1) < 0) return(-1); break;
        case 3: if (i2c_master_send(AT83C26.client, "\xF9", 1) < 0) return(-1); break;
        case 4: if (i2c_master_send(AT83C26.client, "\xFA", 1) < 0) return(-1); break;
        case 5: if (i2c_master_send(AT83C26.client, "\xFB", 1) < 0) return(-1); break;
        case 6: if (i2c_master_send(AT83C26.client, "\xFD", 1) < 0) return(-1); break;
        case 7: if (i2c_master_send(AT83C26.client, "\xFE", 1) < 0) return(-1); break;
        case 8: if (i2c_master_send(AT83C26.client, "\xF7", 1) < 0) return(-1); break;
        default: return(-1);
    }
   
    ret = i2c_master_recv(AT83C26.client, (char*)data, (size > 12) ? 12 : size);
    if (ret<0)
    {
       PrtMsg("fail to receive the i2c data!\n");
       return(-1);
    }
    return(0);
}

int AT83C26_SendCmd(int Cmd, unsigned char *data, int size)
{
    char dataBuf[5];
    int ret;
    
    PrtMsg("%s with Cmd = %d\n", __FUNCTION__, Cmd);
    switch (Cmd)
    {
        case 3: 
            memcpy(dataBuf, (char*)data, size);
            dataBuf[0] |= 0x80; 
            size -=1;
            break;
        case 4:
            memcpy(dataBuf+1, (char*)data, size);
            dataBuf[0] = 0xFC;
            break;
        case 5: 
            dataBuf[0] = (char)*data;
            CLEAR_BIT(dataBuf[0], 7);
            size = 0;
            break;
        case 6:
            memcpy(dataBuf+1, (char*)data, size);
            dataBuf[0] = 0xF8;            
            break;
        case 7:
            memcpy(dataBuf+1, (char*)data, size);
            dataBuf[0] = 0xF9;
            break;
        case 8:
            memcpy(dataBuf+1, (char*)data, size);
            dataBuf[0] = 0xFA;
            break;
        case 9:
            memcpy(dataBuf+1, (char*)data, size);
            dataBuf[0] = 0xFB;
            break;
        case 10:
            memcpy(dataBuf+1, (char*)data, size);
            dataBuf[0] = 0xFD;
            break;
        case 11:
            memcpy(dataBuf+1, (char*)data, size);
            dataBuf[0] = 0xFE;
            break;
        case 12:
            memcpy(dataBuf+1, (char*)data, size);
            dataBuf[0] = 0xF7;
            break;

        default: return(-1);
    }
    
    ret = i2c_master_send(AT83C26.client, dataBuf, size+1);
    if(ret < 0)
    {
        PrtMsg("%s: Fail to send i2c CMD: %d, error code = %d\n", __FUNCTION__, Cmd, ret);   
        return(-1);
    }
    return(0);
}

/**************************************************************************************
** FunctionName: static int AT83C26_CIO(int CardIdx, unsigned char OpFlag)
** Description:  connect CIOx to IO1, and IO1 = CARDIOx, disconnect CIOx to IO1, and CARDIOx=IO1
** Paramater: CardIdx: Card index, OpFlag:CIO_SAVE,disconnect and CARDIOx=IO1;CIO_RESTORE,connect and IO1 = CARDIOx
** Author: Alex Wang
** Update: 2012-10-16
** Version: 1.0
***************************************************************************************/
int AT83C26_CIOx(unsigned char CardIdx, unsigned char CON, unsigned char CIO)
{
    unsigned char RecBuf[12];
    unsigned char RecBuf_1[4];

    PrtMsg("%s[%d] Start, CON = %d, CIO = %d\n", __FUNCTION__, CardIdx, CON, CIO);

    if(CardIdx == 0)                     // Card1
    {
        if(AT83C26_ReadCmd(1, RecBuf, 11))    return(-1);
        if(CON)
        {
            BITisSET(CardParam[CardIdx].CardStatus, ICC_ACTIVE_BIT) ? Put_Uart_OutLine_Normal() : Put_Uart_OutLine_LOW(); // 

            if(AT83C26_ReadCmd(2, RecBuf_1, 4)) 
            {
                return(-1);
            }
            RecBuf_1[1] = 0x00;                       // IO_SELECT = 0x00, CIO1 = IO1;
            if(AT83C26_SendCmd(6, RecBuf_1 + 1, 3)) 
            {
                return(-1);
            }
            CLEAR_BIT(RecBuf[6], 6);         
        }
        else
        {
            if(CIO)
            {
                SET_BIT(RecBuf[6], 0);
            }
            else
            {
                CLEAR_BIT(RecBuf[6], 0);   // set CIO1 level
            }
            SET_BIT(RecBuf[6], 6);
        }
        // send SC1_INTERFACE
        if(AT83C26_SendCmd(5, RecBuf + 6, 1))
        {   
            return(-1);
        }
    }
    else                                     // Card2 ~ Card5
    {
        if(AT83C26_ReadCmd(2, RecBuf, 4)) 
        {
            return(-1);
        }
        
        switch(CardIdx)
        {
            case 1: 
                if(CON)
                {
                    BITisSET(CardParam[CardIdx].CardStatus, ICC_ACTIVE_BIT) ? Put_Uart_OutLine_Normal() : Put_Uart_OutLine_LOW(); // 
                    RecBuf[1] = 0x01;                  // IO_SELECT
                    CLEAR_BIT(RecBuf[3], 4);           // ITDIS
                }
                else
                {
                    if(CIO)
                    {
                        SET_BIT(RecBuf[2], 2);
                    }
                    else
                    {
                        CLEAR_BIT(RecBuf[2], 2);         // INTERFACEB
                    }
                    SET_BIT(RecBuf[3], 4);         // ITDIS
                }
                break; 
            
            case 2: 
                if(CON)
                {
                    BITisSET(CardParam[CardIdx].CardStatus, ICC_ACTIVE_BIT) ? Put_Uart_OutLine_Normal() : Put_Uart_OutLine_LOW(); // 
                    RecBuf[1] = 0x02;                  // IO_SELECT
                    CLEAR_BIT(RecBuf[3], 5);           // ITDIS
                }
                else
                {
                    if(CIO)
                    {
                        SET_BIT(RecBuf[2], 3);
                    }
                    else
                    {
                        CLEAR_BIT(RecBuf[2], 3);         // INTERFACEB
                    }
                    SET_BIT(RecBuf[3], 5);         // ITDIS
                }
                break; 

       
            case 3:
                if(CON)
                {
                    BITisSET(CardParam[CardIdx].CardStatus, ICC_ACTIVE_BIT) ? Put_Uart_OutLine_Normal() : Put_Uart_OutLine_LOW(); // 
                    RecBuf[1] = 0x03;                  // IO_SELECT
                    CLEAR_BIT(RecBuf[3], 6);           // ITDIS
                }
                else
                {
                    if(CIO)
                    {
                        SET_BIT(RecBuf[2], 4);
                    }
                    else
                    {
                        CLEAR_BIT(RecBuf[2], 4);         // INTERFACEB
                    }
                    SET_BIT(RecBuf[3], 6);         // ITDIS
                }
                break;

            case 4: 
                if(CON)
                {
                    BITisSET(CardParam[CardIdx].CardStatus, ICC_ACTIVE_BIT) ? Put_Uart_OutLine_Normal() : Put_Uart_OutLine_LOW(); // 
                    RecBuf[1] = 0x04;                  // IO_SELECT
                    CLEAR_BIT(RecBuf[3], 7);           // ITDIS
                }
                else
                {
                    if(CIO)
                    {
                        SET_BIT(RecBuf[2], 5);
                    }
                    else
                    {
                        CLEAR_BIT(RecBuf[2], 5);         // INTERFACEB
                    }
                    SET_BIT(RecBuf[3], 7);         // ITDIS
                }
                break;
           
            default: 
                return(-1);
        }

        if(AT83C26_SendCmd(6, RecBuf+1, 3))
        {
            return(-1);
        }
    }

    return(0);
}

int AT83C26_CVCCx(unsigned char CardIdx, unsigned char CVCC)
{
    int i = 0;
    int mask;
    unsigned char RecBuf[12];

    PrtMsg("%s[%d,%d] Start\n", __FUNCTION__, CardIdx, CVCC);

    if(CardIdx)                         // Card2 ~ Card5
    {
        if(AT83C26_ReadCmd(7, RecBuf, 2))
        { 
            return(-1);
        }
        CLEAR_BIT(RecBuf[1], (CardIdx + 3));
        if(AT83C26_SendCmd(11, RecBuf, 2))    // enable VDCB, VDCB=5.2v
        {
           return(-1);    
        }
    }

    // enable CVCC
    switch(CardIdx)
    {
        case 0:
            if(AT83C26_ReadCmd(1, RecBuf, 11))   
            {
                return(-1);
            }
            mask = SC1_VOK;                  // VCARD1=CVCC, drive a low level on the CRST1 pin, indicate the card presence detector is open when no
            RecBuf[1] &= 0xFC;        // card is insert, enable the internal pull-up on the CPRES pin, DCK[2:0]=1,prescaler factor=2, DCCLK=CLK/2 
            RecBuf[1] |= CVCC;
            if(AT83C26_SendCmd(3, RecBuf+1, 5))
            {
                return(-1);    // CCLK1=A2, drive the CRST1 through the CARDRST bit
            }
            break;

        case 1:
            if(AT83C26_ReadCmd(3, RecBuf, 3))
            {
                return(-1);
            }
            mask = SCB_VOK;                  // VCARD2=CVCC, drive a low level on the CRST2 pin, stop CCLK2 with a low level, CCLK2=A2, 
            RecBuf[0] &= 0xFC;
            RecBuf[0] |= CVCC;      // indicate the card presence detector is open when no card is insert 
            if(AT83C26_SendCmd(7, RecBuf, 3))
            {
                return(-1);   // enable the internal pull-up on the CPRES2 pin, SC2 working at SAM mode
            }
            break;      

        case 2:
            if(AT83C26_ReadCmd(4, RecBuf, 2))
            {
                return(-1);
            }
            mask = SCB_VOK;                  // VCARD3=CVCC, drive a low level on the CRST3 pin, stop CCLK3 with a low level,CCLK3=A2
            RecBuf[0] &= 0xFC;
            RecBuf[0] |= CVCC;
            if(AT83C26_SendCmd(8, RecBuf, 2))
            {
                return(-1);
            }
            break;    

        case 3:   
            if(AT83C26_ReadCmd(5, RecBuf, 2))
            {
                return(-1);
            }
            mask = SCB_VOK;                 // VCARD4=CVCC, drive a low level on the CRST4 pin, stop CCLK4 with a low level, CCLK4=A2
            RecBuf[0] &= 0xFC;
            RecBuf[0] |= CVCC;
            if(AT83C26_SendCmd(9, RecBuf, 2))
            {
                return(-1);
            }
            break; 

        case 4:
            if(AT83C26_ReadCmd(6, RecBuf, 2))
            {
                return(-1);
            }
            mask = SCB_VOK;                 // VCARD5=CVCC, drive a low level on the CRST5 pin, stop CCLK5 with a low level, CCLK5=A2
            RecBuf[0] &= 0xFC;
            RecBuf[0] |= CVCC;
            if(AT83C26_SendCmd(10, RecBuf, 2))
            {
                return(-1);
            }
            break; 

        default:
             PrtMsg("%s: Invalid Card slot! your slot: %d\n", __FUNCTION__, CardIdx);
             return(-1);                                               
    }

    if(CVCC == 0)
    {
        return(0);
    }

    do
    {
        if(i2c_master_recv(AT83C26.client, (char*)RecBuf, 4) < 0)    
        {
            PrtMsg("%s: CVCC fail, i2c device error!\n", __FUNCTION__);
            return(-1);
        }
    }while((!(RecBuf[0] & mask)) && (++i < 100));    // whether the output voltage remains within the voltage range specified by VCARDx[1:0] bits

    if(!(RecBuf[0] & mask))
    {
        PrtMsg("%s: fail to vcc to the slot %d\n", __FUNCTION__, CardIdx);
        return(-1);
    }

    PrtMsg("%s[%d,%d]: CVCC Ok\n", __FUNCTION__, CardIdx, CVCC);

    return(0);
}

int AT83C26_CCLKx(unsigned char CardIdx, unsigned char RUN, unsigned char CCLK)
{
    unsigned char RecBuf[12];

    PrtMsg("%s[%d] =>RUN = %d, CCLK = %d\n", __FUNCTION__, CardIdx, RUN, CCLK);

    // Enable clock
    switch(CardIdx)
    {
        case 0:
            if(AT83C26_ReadCmd(1, RecBuf, 11))
            {
                return(-1);
            }

            // stop or run CCLK1 ?
            if(RUN)
            {
                CLEAR_BIT(RecBuf[6], 5);
            }
            else
            {
                SET_BIT(RecBuf[6], 5);
                // drive a high  or low level on CCLK1
                if(CCLK)
                {
                    SET_BIT(RecBuf[6], 1);
                }
                else
                {
                    CLEAR_BIT(RecBuf[6], 1);
                }
            }
            if (AT83C26_SendCmd(5, RecBuf+6, 1))
            {
                return(-1); // have CCLK1 runing ccroding to CKS1, CCLK1=A2, CRST1=0, CC81=0, CC41=0,CIO1=1
            }
            break;
 
        case 1:
            if(AT83C26_ReadCmd(3, RecBuf, 3))
            {
                return(-1);
            }

            // stop or run CCLK2 ?
            if(RUN)
            {
                CLEAR_BIT(RecBuf[2], 3);
            }
            else
            {
                SET_BIT(RecBuf[2], 3);
                // drive a high  or low level on CCLK2
                if(CCLK)
                {
                    SET_BIT(RecBuf[2], 4);
                }
                else
                {
                    CLEAR_BIT(RecBuf[2], 4);
                }
            }

            if(AT83C26_SendCmd(7, RecBuf, 3))    //  have CCLK2 runing accroding to CKS2, CCLK2=A2, CRST2=0
            {
                return(-1);
            }
            break;

        case 2:
            if(AT83C26_ReadCmd(4, RecBuf, 2))
            {
                return(-1);
            }

            // stop or run CCLK3 ?
            if(RUN)
            {
                CLEAR_BIT(RecBuf[1], 3);
            }
            else
            {
                SET_BIT(RecBuf[1], 3);
                // drive a high  or low level on CCLK3
                if(CCLK)
                {
                    SET_BIT(RecBuf[1], 4);
                }
                else
                {
                    CLEAR_BIT(RecBuf[1], 4);
                }
            }
            if (AT83C26_SendCmd(8, RecBuf, 2))   // have CCLK3 runing accroding to CKS3, CCLK3=A2, CRST3=0
            {
                return(-1);
            }
            break;

        case 3:
             if(AT83C26_ReadCmd(5, RecBuf, 2))
             {
                 return(-1);
             }
         
             // stop or run CCLK4 ?
             if(RUN)
             {
                 CLEAR_BIT(RecBuf[1], 3);
             }
             else
             {
                 SET_BIT(RecBuf[1], 3);
                 // drive a high  or low level on CCLK4
                 if(CCLK)
                 {
                     SET_BIT(RecBuf[1], 4);
                 }
                 else
                 {
                     CLEAR_BIT(RecBuf[1], 4);
                 }
             }
            if (AT83C26_SendCmd(9, RecBuf, 2))   // have CCLK4 runing accroding to CKS4, CCLK4=A2, CRST4=0;
            {
                return(-1);
            }
            break;

        case 4:
             if(AT83C26_ReadCmd(6, RecBuf, 2))
             {
                 return(-1);
             }
         
             // stop or run CCLK5 ?
             if(RUN)
             {
                 CLEAR_BIT(RecBuf[1], 3);
             }
             else
             {
                 SET_BIT(RecBuf[1], 3);
                 // drive a high  or low level on CCLK5
                 if(CCLK)
                 {
                     SET_BIT(RecBuf[1], 4);
                 }
                 else
                 {
                     CLEAR_BIT(RecBuf[1], 4);
                 }
             }
            if (AT83C26_SendCmd(10, RecBuf, 2))   // have CCLK5 runing accroding to CKS5, CCLK5=A2, CRST5=0;
            {
                return(-1);
            }
            break;

        default:
            return(-1);
    }

    return(0);
}

int AT83C26_CRSTx(unsigned char CardIdx, int CRST)
{
    unsigned char RecBuf[12];
  
    PrtMsg("%s[%d] => CRST = %d\n", __FUNCTION__, CardIdx, CRST);

    switch(CardIdx)
    {
        case 0:
            if(AT83C26_ReadCmd(1, RecBuf, 11))
            {
                return(-1);
            }
            if(CRST)
            {
                SET_BIT(RecBuf[6], 4);
            }
            else
            {
                CLEAR_BIT(RecBuf[6], 4);
            }
            if (AT83C26_SendCmd(5, RecBuf+6, 1))    // whether enter a reset sequence 
            {
                return(-1);                         // accroding to ART1 bit value
            }
            break;

        case 1:
            if(AT83C26_ReadCmd(3, RecBuf, 3))
            {
                return(-1);
            }
            if(CRST)
            {
                SET_BIT(RecBuf[2], 5);
            }
            else
            {
                CLEAR_BIT(RecBuf[2], 5);
            }
            if (AT83C26_SendCmd(7, RecBuf, 3))      // whether enter a reset sequence accroding to ART2 bit value
            {
                return(-1);
            }
            break;

        case 2:
            if(AT83C26_ReadCmd(4, RecBuf, 2))
            {
                return(-1);
            }
            if(CRST)
            {
                SET_BIT(RecBuf[1], 5);
            }
            else
            {
                CLEAR_BIT(RecBuf[1], 5);
            }
            if (AT83C26_SendCmd(8, RecBuf, 2))      // whether enter a reset sequence accroding to ART3 bit value
            {
                return(-1);
            }
            break;  

        case 3:
            if(AT83C26_ReadCmd(5, RecBuf, 2))
            {
                return(-1);
            }
            if(CRST)
            {
                SET_BIT(RecBuf[1], 5);
            }
            else
            {
                CLEAR_BIT(RecBuf[1], 5);
            }
            if (AT83C26_SendCmd(9, RecBuf, 2))      // whether enter a reset sequence accroding to ART4 bit value
            {
                return(-1);
            }
            break;

        case 4:
            if(AT83C26_ReadCmd(6, RecBuf, 2))
            {
                return(-1);
            }
            if(CRST)
            {
                SET_BIT(RecBuf[1], 5);
            }
            else
            {
                CLEAR_BIT(RecBuf[1], 5);
            }
            if (AT83C26_SendCmd(10, RecBuf, 2))      // whether enter a reset sequence accroding to ART5 bit value
            {
                return(-1);
            }
            break;

        default:
            return(-1);    
    }

    return(0);
}

int AT83C26_Reset(void)
{

    PrtMsg("Welcome to entry to the function: %s\n",__FUNCTION__);

    // initial slot 1:  VCARD1 = 0V, CPRES1: normal open,  CCLK1 = CLK, internal pull-up
    if(AT83C26_SendCmd(3, SC1_PWR, sizeof(SC1_PWR)))
    {
        return(-1);
    }
    // initial slot 1: CIO1 = 0, CCLK1 = 0, CRST1 = 0
    if(AT83C26_SendCmd(5 , (unsigned char*)SC1_INTFACE, sizeof(SC1_INTFACE)))
    {
        return(-1);
    }

    // initial DCDCB: VDCB = 5.2V
    if(AT83C26_SendCmd(11, VDCB_INITIAL, sizeof(VDCB_INITIAL)))
    {
        return(-1);
    }

    // initial slot 2: VCARD2 = 0V, CRST2 = 0, CCLK2 = 0,  CCLK2 = CLK
    if(AT83C26_SendCmd(7, SC2_PWR, sizeof(SC2_PWR)))
    {
        return(-1);
    }
    // initial slot 3: VCARD3 = 0V, CRST3 = 0, CCLK3 = 0,  CCLK3 = CLK
    if(AT83C26_SendCmd(8, SC3_PWR, sizeof(SC3_PWR)))
    {
        return(-1);
    }
    // initial slot 4: VCARD4 = 0V, CRST4 = 0, CCLK4 = 0,  CCLK4 = CLK
    if(AT83C26_SendCmd(9, SC4_PWR, sizeof(SC4_PWR)))
    {
        return(-1);
    }
    // initial slot 5: VCARD5 = 0V, CRST5 = 0, CCLK5 = 0,  CCLK5 = CLK
    if(AT83C26_SendCmd(10, SC5_PWR, sizeof(SC5_PWR)))
    {
        return(-1);
    }

    if(AT83C26_SendCmd(6, (unsigned char*)SCx_INTFACE, sizeof(SCx_INTFACE)))
    {
        return(-1);
    }


/*    AT83C26_CRSTx(0, 1);
    if(AT83C26_CCLKx(0, 1, 0))    return(-1);
    if(AT83C26_CVCCx(0, 1)) return(-1);
    mdelay(20);

    if(AT83C26_CVCCx(1, 2)) return(-1);
    if(AT83C26_CCLKx(1, 1, 0))    return(-1);
    AT83C26_CRSTx(1, 1);*/
    return(0);
}

static irqreturn_t AT83C26_Irq(int irq, void *dev_id)
{
//    unsigned char RecBuf[12];
/*    
    AT83C26_ReadCmd(1, RecBuf, 2);

    if(BITisSET(RecBuf[1], 4))    // a change in card status
    {
        SET_BIT(CardParam[SlotICC].CardStatus, ICC_STATUS_CHANGE_BIT);
        if(BITisSET(RecBuf[0], 5))    // a card is detected
        {
            SET_BIT(CardParam[SlotICC].CardStatus, ICC_PRESENT_BIT);
        }
        else
        {
            SET_BIT(CardParam[SlotICC].CardStatus, ICC_PRESENT_BIT);
        }
    }
*/
    PrtMsg("welcome to entry the interrupt function: %s\n\n", __FUNCTION__);
    return(IRQ_HANDLED);
}

static int AT83C26_INT_CFG(struct i2c_client *client)
{
    int ret;
    
    PrtMsg("%s: start, irq = %d\n", __FUNCTION__, client->irq);
    PrtMsg("%s: start, name = %s\n", __FUNCTION__, client->name);
    PrtMsg("%s: start, addr = %02X\n", __FUNCTION__, client->addr);
//    omap_cfg_reg(157);
    Reg_83c26_INT = ioremap(GPIO157_156, 1);
    if(Reg_83c26_INT == NULL)
    {
        PrtMsg("%s:IORemap register memory faild!!!\n",__FUNCTION__);
        return(-1);
    }
    PrtMsg("*Reg_83c26_INT = %X\n", *Reg_83c26_INT);
    Gpio_Init(Reg_83c26_INT, MODE4_GPIO, PULLUPDOWNENABLE, DIREC_INPUT, OFFS_16);

//    ret = gpio_request(client->irq, "at83c26_INT");
//    if(ret < 0)
//    {
//        PrtMsg("%s: fail to got gpio for at83c26 INT pin(ret = %08X)!\n",__FUNCTION__, ret);
//        return(-1);
//    }
    gpio_direction_input(client->irq);
    set_irq_type(OMAP_GPIO_IRQ(client->irq), IRQ_TYPE_LEVEL_HIGH);
//    PrtMsg("OMAP_GPIO_IRQ(157) = %d, gpio_to_irq(157) = %d \n", OMAP_GPIO_IRQ(157),gpio_to_irq(157));
    ret = request_irq(OMAP_GPIO_IRQ(client->irq), AT83C26_Irq, IRQF_DISABLED , (char*)"at83c26_INT", NULL);
    if(ret)
    {
       PrtMsg("fail to request irq: ret = %X\n",ret); 
       return(-1);    
    }
    
    enable_irq(gpio_to_irq(client->irq));
    
    return(0);
}


int AT83C26_Probe(struct i2c_client *client, const struct i2c_device_id *id)
{
    PrtMsg("Welcome to entry the function: %s\n",__FUNCTION__);

    
    AT83C26.client = client;
    

    
    if(AT83C26_Reset())          // initial each slot with low level
    {
        AT83C26.client = NULL;
        return(-1);
    }

    if(AT83C26_INT_CFG(AT83C26.client))    return(-1);

    return (0);
}

int AT83C26_Remove(struct i2c_client *client)
{

//    AT83C26_ChipInit();

    // initial DCDCA: shutdown DC/DCA
    SC1_PWR[1] = 0x20;
    if(AT83C26_SendCmd(3, SC1_PWR, 2))
    {
        return(-1);
    }
    // initial DCDCB: shutdown DC/DCB
    VDCB_INITIAL[0] = 0x80;
    if(AT83C26_SendCmd(11, VDCB_INITIAL, 1))
    {
        return(-1);
    }
    
    disable_irq(OMAP_GPIO_IRQ(client->irq));
    free_irq(OMAP_GPIO_IRQ(client->irq), NULL);

    irq_to_gpio(client->irq);
//    gpio_free(19);
    
    return(0);
}

static const struct i2c_device_id at83c26_id[] = {  
    {"at83c26", 0 }, //该i2c_driver所支持的i2c_client 
    {}  
};

static struct i2c_driver AT83C26_Driver=
{
    .probe = AT83C26_Probe,
    .remove= AT83C26_Remove,
    .driver=
    {
        .owner = THIS_MODULE,
        .name  = "at83c26",
    },
    .id_table = at83c26_id, 
};



int AT83C26_Init(void)
{
    PrtMsg("welcome to entry the function: %s\n",__FUNCTION__);

    Reg_83C26_RESET = ioremap(GPIO13_12, 1);
    if(Reg_83C26_RESET == NULL)
    {
        PrtMsg("%s:IORemap register memory faild!!!\n",__FUNCTION__);
        goto err1;
    }
    PrtMsg("*Reg_83C26_RESET = %X\n", *Reg_83C26_RESET);
    Gpio_Init(Reg_83C26_RESET, MODE4_GPIO, PULLUPDOWNENABLE, DIREC_OUTPUT, OFFS_16);
    udelay(10);

    // config AT83C26 address at i2c bus
    Set_GPIO_Low(Reg_83C26_RESET, OFFS_16);
    udelay(160);
    Set_GPIO_High(Reg_83C26_RESET, OFFS_16);
    udelay(400);

    if(i2c_add_driver(&AT83C26_Driver))
    {
        PrtMsg("%s: Fail on AT83C26\n", __FUNCTION__);
        goto err2;
    }

    return (0);

err2:
    iounmap(Reg_83C26_RESET);
err1:
    iounmap(Reg_83c26_INT);
    return(-1);
}

int AT83C26_Uninit(void)
{
//    free_irq(OMAP_GPIO_IRQ(19), NULL);
//    disable_irq(gpio_to_irq(19));
//    irq_to_gpio(19);
//    gpio_free(19);

    i2c_del_driver(&AT83C26_Driver);

    iounmap(Reg_83C26_RESET);
    Reg_83C26_RESET = NULL;
    iounmap(Reg_83c26_INT);
    Reg_83c26_INT = NULL;   
    

    return(0);
}


