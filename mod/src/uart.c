/*
* Name: UART source file
* Date: 2012/10/08
* Author: Alex Wang
* Version: 1.0
*/

#include <asm/io.h>
#include <linux/delay.h>

#include "uart.h"
#include "debug.h"
#include "common.h"
#include "param.h"
#include "timer.h"
#include "atr.h"
#include "icc.h"

uart_reg_array *UartReg = NULL;


void Set_Baudrate(int BaudRate)
{
    unsigned int Divalue;
    unsigned int DLH;
    unsigned int DLL;
    unsigned char temp;


    PrtMsg("welcome to entry the function: %s, BaudRate = %X\n", __FUNCTION__, BaudRate);

    temp = UartReg->LCR;
   
    SET_CONFIGMODE_B(UartReg);
    SET_BIT(UartReg->Reg3.EFR,4);         // Enables writing to IER_REG bits [7:4], FCR_REG bits [5:4], and MCR_REG bits [7:5]
    SET_OPERATIONAL_MODE(UartReg);
    UartReg->Reg2.IER=0x0;                // Clear register IER,Disable all the UART interrupt and Disable sleep mode
    SET_CONFIGMODE_B(UartReg);

    Divalue=48000000/(16*BaudRate);
    DLH = Divalue / 256;
    DLL = Divalue % 256;

    UartReg->Reg2.DLH = DLH;
    UartReg->Reg1.DLL = DLL;

    SET_CONFIGMODE_B(UartReg);      // Disables writing to IER_REG bits [7:4], FCR_REG bits [5:4], and MCR_REG bits [7:5]
    CLEAR_BIT(UartReg->Reg3.EFR,4);

    UartReg->LCR = temp;
}

static void Uart_init_Reg(void)
{
    unsigned int temp;


    PrtMsg("welcome to entry the function: %s, %X\n",__FUNCTION__, (unsigned int)UartReg);
    SET_BIT(UartReg->SYSC, 1);         // reset UART module
    while(BITisCLEAR(UartReg->SYSS, 0));            // wait for the end of the reset operation complete

    SET_CONFIGMODE_B(UartReg);
    SET_BIT(UartReg->Reg3.EFR, 4);
    SET_CONFIGMODE_A(UartReg);
    SET_BIT(UartReg->Reg4.MCR, 6);
    UartReg->Reg3.FCR = 0x01;    //  FIFO_EN: enbale
    SET_CONFIGMODE_B(UartReg);
    UartReg->Reg7.TLR = 0xFF;    // RX_FIFO_TRIG: 15 * 4 bytes, TX_FIFO_TRIG: 15 * 4 bytes,

    UartReg->MDR1 = 0x07;              // Disable UART, it is mandatory that MODE_SELECT = Disable before initializing or modifying clock parameter controls
    temp = Caculate_BuadRate(0x11);
    Set_Baudrate(temp);   // set default fidi = 11
    
    UartReg->LCR = ((DATALEN_8BITS << 0) | (STOPLEN_1BIT << 2) | (PARITY_EN << 3) | (PARITY_EVEN << 4));  // Even parity, 1 stop bit, 8 data bits
    UartReg->MDR1 = 0x00;              // UART 16x mode
    SET_OPERATIONAL_MODE(UartReg);
}

static void Uart_TraOneByte(unsigned char CardIdx, unsigned char Data)
{
    unsigned char i;


    UartReg->Reg1.THR = Data;
    while(BITisCLEAR(UartReg->Reg5.LSR, b_TX_SR_E));    // waiting for transmitter complete

    for(i = 0; i < (CardParam[CardIdx].ExtraGT + 1); i++)
    {
        udelay(CardParam[CardIdx].CurETU);
    }
}

static unsigned char Uart_RecOneByte(unsigned char CardIdx, unsigned char *Data)
{
    unsigned char tempReg;


    while(BITisCLEAR(UartReg->Reg5.LSR, b_RX_FIFO_E))        // waiting for a data be received
    {
        if(timeOut == true)
        {
            return(SC_TIMEOUT_ERROR);
        }
    }

    Set_WaiTingTime(CardParam[CardIdx].WT);

    *Data = UartReg->Reg1.RHR;
    tempReg = UartReg->Reg5.LSR;
  
    if(BITisSET(tempReg, b_RX_FIFO_STS))    // whether parity error, framing error, or break occure
    {
        if(BITisSET(tempReg, b_RX_BI))
        {
            return(SC_BREAK_ERROR);
        }
        if(BITisSET(tempReg, b_RX_FE))
        {
            return(SC_FRAME_ERROR);
        }
        if(BITisSET(tempReg, b_RX_PE))
        {
            return(SC_PARITY_ERROR);
        }
    }
    if(BITisSET(SC_PARITY_ERROR, b_RX_OE))
    {
        return(SC_OVERRUN_ERROR);
    } 
    
    return(OK); 
}

void SC_TraMoreBytes(unsigned char CardIdx, unsigned char *SendBuf, unsigned int SendLen)
{
//    PrtMsg("%s: CardIdx = %d, SendLen = %d\n", __FUNCTION__, CardIdx, SendLen);
    Set_WaiTingTime(CardParam[CardIdx].WT);
    if(CardParam[CardIdx].Direct == DIRECT_CONVENTION)    // Direct convention, states H encodes value 1, LSB first
    {
        while(SendLen--)
        {
//            PrtMsg("0x%X ", *SendBuf);
            Uart_TraOneByte(CardIdx, *(SendBuf++));
        }
    }
    else                                                  // Inverse convention, states L encodes value 1, MSB first
    {
        while(SendLen--)
        {
            Uart_TraOneByte(CardIdx, Byte_Direct2Inverse(*SendBuf));
            SendBuf++;
        }
    }
    CLearFIFO();
//	SC_RecMoreBytes(CardIdx, TempBuf, TempLen);
}

unsigned char SC_RecMoreBytes(unsigned char CardIdx, unsigned char *RecBuf, unsigned int RecLen)
{
    unsigned char ret = OK;


    PrtMsg("%s: CardIdx = %d, RecLen = %d\n", __FUNCTION__, CardIdx, RecLen);
    if(CardParam[CardIdx].Direct == DIRECT_CONVENTION)    // Direct convention, states H encodes value 1, LSB first
    {
        while(RecLen--)
        {
            ret = Uart_RecOneByte(CardIdx, RecBuf);
            PrtMsg("0x%X ", *RecBuf);
            if(ret != OK)
            {
                PrtMsg("%s: Fail to receive uart data, error code = %X\n",__FUNCTION__, ret);
                return(ret);
            }

            RecBuf++;
        }
    }
    else                                                  // Inverse convention, states L encodes value 1, MSB first
    {
        while(RecLen--)
        {
            ret = Uart_RecOneByte(CardIdx, RecBuf);
            PrtMsg("*RecBuf = %X\n", *RecBuf);
            *RecBuf = Byte_Direct2Inverse(*RecBuf);
            PrtMsg("*RecBuf = %X\n", *RecBuf);
            if(ret == SC_PARITY_ERROR)
            {
                ret = OK;
            }
            if(ret != OK)
            {
                PrtMsg("%s: Fail to receive uart data, error code = %X\n",__FUNCTION__, ret);
                return(ret);
            }

            RecBuf++;
        }
    }
    return(OK);
}

void CLearFIFO(void)
{
    SET_BIT(UartReg->Reg3.FCR, 1); // clears the receive FIFO and resets its counter logic to 0 after clearing FIFO
}

void Put_Uart_OutLine_LOW(void)
{
    SET_BIT(UartReg->LCR, 6);
}
void Put_Uart_OutLine_Normal(void)
{
    CLEAR_BIT(UartReg->LCR, 6);
}

int Uart_Init(void)
{
    PrtMsg("welcom to entry the funtion: %s\n", __FUNCTION__);
    UartReg = (uart_reg_array *)ioremap(PhyAddr_UART,AddrSize_UART);
    if(UartReg==NULL)
    {
        PrtMsg("%s:IORemap register memory faild!!!\n",__FUNCTION__);
        return(-1);
    }
    PrtMsg("Success to got the virtual address of UART, UartReg = %X\n", (unsigned int)UartReg);
    Uart_init_Reg();
    PrtMsg("exit the funtion: %s, good bye!\n", __FUNCTION__);
    return(0);
}

void Uart_Uninit(void)
{
    iounmap(UartReg);
    UartReg = NULL;
}

