

#include <asm/io.h>



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


struct uart_reg_array
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
    volatile unsigned int LCR;	// Line control register

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
};

#define SET_CONFIGMODE_A(UReg)        UReg->LCR=0x80
#define SET_CONFIGMODE_B(UReg)        UReg->LCR=0xBF
#define SET_OPERATIONAL_MODE(UReg)    CLEAR_BIT(UReg->LCR,7)


struct uart_reg_array *uart_reg = NULL;


void set_baudrate(u32 baudrate)
{
    u32 div_value;
    u32 dlh;
    u32 dll;
    u8 temp;


//    TRACE_TO("enter %s, BaudRate = %X\n", __FUNCTION__, baudrate);

    temp = uart_reg->LCR;
   
    SET_CONFIGMODE_B(uart_reg);
    SET_BIT(uart_reg->Reg3.EFR, 4);         // Enables writing to IER_REG bits [7:4], FCR_REG bits [5:4], and MCR_REG bits [7:5]
    SET_OPERATIONAL_MODE(uart_reg);
    uart_reg->Reg2.IER = 0x0;                // Clear register IER,Disable all the UART interrupt and Disable sleep mode
    SET_CONFIGMODE_B(uart_reg);

//	INFO_TO("%s ==> BaudRate = %d\n", __FUNCTION__, baudrate);
	
    div_value=48000000/(16*baudrate);
    dlh = div_value / 256;
    dll = div_value % 256;

    uart_reg->Reg2.DLH = dlh;
    uart_reg->Reg1.DLL = dll;

    SET_CONFIGMODE_B(uart_reg);      // Disables writing to IER_REG bits [7:4], FCR_REG bits [5:4], and MCR_REG bits [7:5]
    CLEAR_BIT(uart_reg->Reg3.EFR, 4);

    uart_reg->LCR = temp;

//	TRACE_TO("exit %s\n", __func__);
}

static void uart_init_reg(void)
{

    TRACE_TO("enter %s\n", __func__);

	SET_BIT(uart_reg->SYSC, 1);         // reset UART module
    while(BITisCLEAR(uart_reg->SYSS, 0));            // wait for the end of the reset operation complete

    SET_CONFIGMODE_B(uart_reg);
    SET_BIT(uart_reg->Reg3.EFR, 4);
    SET_CONFIGMODE_A(uart_reg);
    SET_BIT(uart_reg->Reg4.MCR, 6);
    uart_reg->Reg3.FCR = 0x01;    //  FIFO_EN: enbale
    SET_CONFIGMODE_B(uart_reg);
    uart_reg->Reg7.TLR = 0xFF;    // RX_FIFO_TRIG: 15 * 4 bytes, TX_FIFO_TRIG: 15 * 4 bytes,

    uart_reg->MDR1 = 0x07;              // Disable UART, it is mandatory that MODE_SELECT = Disable before initializing or modifying clock parameter controls
    set_baudrate(9600);   // set default fidi = 11
    
    uart_reg->LCR = ((DATALEN_8BITS << 0) | (STOPLEN_1BIT << 2) | (PARITY_EN << 3) | (PARITY_EVEN << 4));  // Even parity, 1 stop bit, 8 data bits
    uart_reg->MDR1 = 0x00;              // UART 16x mode
    SET_OPERATIONAL_MODE(uart_reg);

	TRACE_TO("exit %s\n", __func__);
}

void uart_clear_fifo(void)
{
    SET_BIT(uart_reg->Reg3.FCR, 1); // clears the receive FIFO and resets its counter logic to 0 after clearing FIFO
}

void put_uart_outline_low(void)
{
    SET_BIT(uart_reg->LCR, 6);
}
void put_uart_outline_normal(void)
{
    CLEAR_BIT(uart_reg->LCR, 6);
}

static void uart_trans_one_byte(struct icc_info *icc, u8 data)
{
    u8 i;


    uart_reg->Reg1.THR = data;
    while(BITisCLEAR(uart_reg->Reg5.LSR, b_TX_SR_E));    // waiting for transmitter complete

    for(i = 0; i < (icc->extra_GT + 1); i++)
    {
        udelay(icc->cur_ETU);
    }
}

static int uart_recv_one_byte(struct icc_info *icc, u8 *data)
{
    u8 tempReg;


    while(BITisCLEAR(uart_reg->Reg5.LSR, b_RX_FIFO_E))        // waiting for a data be received
    {
        if(icc->common->time_out == true)
			return	-ICC_ERRORCODE_ICC_MUTE;
    }

    set_waiting_timer(icc, icc->WT);

    *data = uart_reg->Reg1.RHR;
    tempReg = uart_reg->Reg5.LSR;
	
    if(BITisSET(tempReg, b_RX_FIFO_STS))    // whether parity error, framing error, or break occure
    {
        if(BITisSET(tempReg, b_RX_BI))
        {
            return	-ICC_ERRORCODE_HW_ERROR;
        }
        if(BITisSET(tempReg, b_RX_FE))
        {
            return	-ICC_ERRORCODE_FRAMERROR;
        }
        if(BITisSET(tempReg, b_RX_PE))
        {
            return	-ICC_ERRORCODE_XFR_PARITY_ERROR;
        }
    }
    if(BITisSET(tempReg, b_RX_OE))
    {
        return	-ICC_ERRORCODE_XFR_OVERRUN;
    } 
    
    return	0; 
}

void ifd_send_bytes(struct icc_info *icc, u8 *send_buf, u32 send_len)
{
//	TRACE_TO("enter %s\n", __func__);
	
    set_waiting_timer(icc, icc->WT);
    if(icc->direct == DIRECT_CONVENTION)    // Direct convention, states H encodes value 1, LSB first
    {
        while(send_len--)
            uart_trans_one_byte(icc, *(send_buf++));
    }
    else                                                  // Inverse convention, states L encodes value 1, MSB first
    {
        while(send_len--)
            uart_trans_one_byte(icc, byte_direct_2_inverse(*send_buf++));

    }
	
    uart_clear_fifo();

//	TRACE_TO("exit %s\n", __func__);

}

int ifd_receive_bytes(struct icc_info *icc, u8 *rec_buf, u32 rec_len)
{
    int ret = 0;


//    TRACE_TO("enter %s\n", __func__);
	
    if(icc->direct == DIRECT_CONVENTION)    // Direct convention, states H encodes value 1, LSB first
    {
        while(rec_len--)
        {
            ret = uart_recv_one_byte(icc, rec_buf);

            if(ret)
            {
                ERROR_TO("fail to receive uart data, ret = %d\n", ret);
                goto	err;
            }

            rec_buf++;
        }
    }
    else                                                  // Inverse convention, states L encodes value 1, MSB first
    {
        while(rec_len--)
        {
            ret = uart_recv_one_byte(icc, rec_buf);
            *rec_buf = byte_direct_2_inverse(*rec_buf);
            if(ret == -ICC_ERRORCODE_XFR_PARITY_ERROR)
            {
                ret = 0;
            }
			
            if(ret)
            {
                ERROR_TO("fail to receive uart data, ret = %d\n", ret);
                goto	err;
            }

            rec_buf++;
        }
    }

err:
//	TRACE_TO("exit %s\n", __func__);
	return	ret;
}

int uart_init(void)
{
	int	ret;

	
    TRACE_TO("enter %s\n", __func__);
	
    uart_reg = (struct uart_reg_array *)ioremap(PhyAddr_UART, AddrSize_UART);
    if(!uart_reg)
    {
        ERROR_TO("unable to ioremap\n");
        ret = -EFAULT;
		goto err;
    }
	
    uart_init_reg();
	
    ret = 0;

err:
	TRACE_TO("exit %s\n", __func__);
	
	return	ret;
}

void uart_uninit(void)
{
    iounmap(uart_reg);
    uart_reg = NULL;
}

