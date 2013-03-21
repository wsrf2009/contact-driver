/*
* Name: UART head file
* Date: 2012/10/08
* Author: Alex Wang
* Version: 1.0
*/

#ifndef UART_H
#define UART_H

#define BaseAddr_UART1 0x4806A000
#define BaseAddr_UART2 0x4806C000
#define BaseAddr_UART3 0x49020000
#define BaseAddr_UART4 0x49042000

#define AddrSize_UART 4096

#define PhyAddr_UART BaseAddr_UART3


// LCR_REG config value
#define DATALEN_8BITS  3
#define STOPLEN_1BIT   0
#define PARITY_EN      1
#define PARITY_EVEN    1

// LSR_REG flag bit
#define b_RX_FIFO_E       0          // 1, have data;  0, no data 
#define b_RX_OE           1          // 1, overrun error;  0, no overrun data
#define b_RX_PE           2          // 1, parity error;  0, no parity error
#define b_RX_FE           3          // 1, framing error occurred;  0, No framing error
#define b_RX_BI           4          // 1, a break was detected;  0, no break condition
#define b_TX_SR_E         6          // 1, transmitter hold and shift registers are empty;  0, transmitter hold and shift registers are not empty
#define b_RX_FIFO_STS     7          // 1, at least one parity error, framing error or break occcur in RX FIFO; 0, normal operation





#define SC_OVERRUN_ERROR    0xFA
#define SC_BREAK_ERROR      0XFB
#define SC_FRAME_ERROR      0xFC
#define SC_PARITY_ERROR     0xFD
#define SC_TIMEOUT_ERROR    0xFE
#define OK                  0





typedef struct
{
    union 
    {   // OFFSET:0X000
        volatile unsigned int DLL;    // Configuration_Mode_A/B(r/w): Divisor latches low for generation of baud clock
        volatile unsigned int RHR;    // Operation_Mode(r): receive holding register
        volatile unsigned int THR;    // Operation_Mode(w): Transmit holding register
    }Reg1;

    union
    {   // OFFSET:0X004
        volatile unsigned int DLH;    // Configuration_Mode_A/B(r/w): Divisor latches high for generation of baud clock
        volatile unsigned int IER;    // Operation_Mode(r/w): Interrupt enable register
    }Reg2;

    union
    {   // OFFSET:0X008
        volatile unsigned int IIR;    // Configuration_Mode_A(r)/Operation_Mode(r): Interrupt Identification register
        volatile unsigned int FCR;    // Configuration_Mode_A(w)/Operation_Mode(w): FIFO control register
        volatile unsigned int EFR;    // Configuration_Mode_B(r/w): Enhanced feature register
    }Reg3;

    // OFFSET:0X00C
    volatile unsigned int LCR;  // Line control register

    union
    {   // OFFSET:0X010
        volatile unsigned int MCR;           // Configuration_Mode_A(r/w)/Operation_Mode(r/w): Modem control register
        volatile unsigned int XON1_ADDR1;    // Configuration_Mode_B(r/w): UART mode: XON1 character, IrDA mode: ADDR1 address
    }Reg4;

    union
    {   // OFFSET:0X014
        volatile unsigned int LSR;           // Configuration_Mode_A(r)/Operation_Mode(r): Line status register
        volatile unsigned int XON2_ADDR2;    // Configuration_Mode_B(r/w): 
    }Reg5;

    union
    {   // OFFSET:0X018
        volatile unsigned int MSR;       // Modem status register. UART mode only.
        volatile unsigned int TCR;       // Transmission control register
        volatile unsigned int XOFF1;     // UART mode XOFF1 character
    }Reg6;

    union
    {   // OFFSET:0X01C
        volatile unsigned int SPR;       // Scratchpad register
        volatile unsigned int TLR;       // Trigger level register.
        volatile unsigned int XOFF2;     // UART mode XOFF2 character
    }Reg7;

   	// OFFSET:0X020
    volatile unsigned int MDR1;          // Mode definition register 1.
    // OFFSET:0X024
    volatile unsigned int MDR2;          // Mode definition register 2

    union
    {   // OFFSET:0X028
        volatile unsigned int SFLSR;     // Configuration_Mode_A(r)/Configuration_Mode_B(r)/Operation_Mode(r): Status FIFO line status register
        volatile unsigned int TXFLL;     // Configuration_Mode_A(w)/Configuration_Mode_B(w)/Operation_Mode(w): Transmit frame length register low
    }Reg8;

    union
    {   // OFFSET:0X02C
        volatile unsigned int RESUME;    // Configuration_Mode_A(r)/Configuration_Mode_B(r)/Operation_Mode(r): IR-IrDA and IR-CIR modes only
        volatile unsigned int TXFLH;     // Configuration_Mode_A(w)/Configuration_Mode_B(w)/Operation_Mode(w): Transmit frame length register low
    }Reg9;

    union
    {   // OFFSET:0X030
        volatile unsigned int SFREGL;    // Configuration_Mode_A(r)/Configuration_Mode_B(r)/Operation_Mode(r): Status FIFO register low
        volatile unsigned int RXFLL;     // Configuration_Mode_A(w)/Configuration_Mode_B(w)/Operation_Mode(w): Received frame length register low
    }Reg10;

    union
    {   // OFFSET:0X034
        volatile unsigned int SFREGH;    // Configuration_Mode_A(r)/Configuration_Mode_B(r)/Operation_Mode(r): Status FIFO register high
        volatile unsigned int RXFLH;     // Configuration_Mode_A(w)/Configuration_Mode_B(w)/Operation_Mode(w): Received frame length register high
    }Reg11;

    union
    {   // OFFSET:0X038
        volatile unsigned int UASR;      // Configuration_Mode_A(r)/Configuration_Mode_B(r): UART autobauding status register
        volatile unsigned int BLR;       // Operation_Mode(r/w): BOF control register
    }Reg12;

    // OFFSET:0X03C
    volatile unsigned int ACREG;         // Auxiliary control register
    // OFFSET:0X040
    volatile unsigned int SCR;           // Supplementary control register
    // OFFSET:0X044
    volatile unsigned int SSR;           // Supplementary status register
    // OFFSET:0X048
    volatile unsigned int EBLR;          // BOF length register
    // OFFSET:0X04C
    volatile unsigned int RFU1;          // ---------------------------
    // OFFSET:0X050
    volatile unsigned int MVR;           // Module version register
    // OFFSET:0X054
    volatile unsigned int SYSC;          // System configuration register
    // OFFSET:0X058
    volatile unsigned int SYSS;          // System status register
    // OFFSET:0X05C
    volatile unsigned int WER;           // Wake-up enable register
    // OFFSET:0X060
    volatile unsigned int CFPS;          // Carrier frequency prescaler
    // OFFSET:0X064
    volatile unsigned int RXFIFO_LVL;    // Level of the RX FIFO
    // OFFSET:0X068
    volatile unsigned int TXFIFO_LVL;    // Level of the TX FIFO
    // OFFSET:0X06C
    volatile unsigned int IER2;          // Enables RX/TX FIFOs empty corresponding interrupts
    // OFFSET:0X070
    volatile unsigned int ISR2;          // Status of RX/TX FIFOs empty corresponding interrupts
    // OFFSET:0X074
    volatile unsigned int RFU2;          // ----------------------------------
    // OFFSET:0X078
    volatile unsigned int RFU3;          // ----------------------------------
    // OFFSET:0X07C
    volatile unsigned int RFU4;          // ----------------------------------
    // OFFSET:0X080
    volatile unsigned int MDR3;          // ----------------------------------
}uart_reg_array;

#define SET_CONFIGMODE_A(UReg)        UReg->LCR=0x80
#define SET_CONFIGMODE_B(UReg)        UReg->LCR=0xBF
#define SET_OPERATIONAL_MODE(UReg)    CLEAR_BIT(UReg->LCR,7)

void Set_Baudrate(int BaudRate);
void CLearFIFO(void);
void SC_TraMoreBytes(unsigned char CardIdx, unsigned char *SendBuf, unsigned int SendLen);
unsigned char SC_RecMoreBytes(unsigned char CardIdx, unsigned char *RecBuf, unsigned int RecLen);
void Put_Uart_OutLine_LOW(void);
void Put_Uart_OutLine_Normal(void);
int Uart_Init(void);
void Uart_Uninit(void);

#endif


