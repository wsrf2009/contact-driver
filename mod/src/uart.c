
#include <linux/slab.h>
#include <linux/io.h>

#define	SMARTCARD_IO_UART2
//#define	SMARTCARD_IO_UART3

#define UART1_BASE				0x4806A000
#define UART2_BASE				0x4806C000
#define UART3_BASE				0x49020000
#define UART4_BASE				0x49042000

#ifdef	SMARTCARD_IO_UART2
#define	UART_BASE_ADDR			UART2_BASE
#define	EN_UART					(1<<14)
#define	UART_FUNC_CLK_EN		0x48004A00
#define	UART_INTF_CLK_EN		0x48004A10
#else
#define	UART_BASE_ADDR			UART3_BASE
#define	EN_UART					(1<<11)
#define	UART_FUNC_CLK_EN		0x48005000
#define	UART_INTF_CLK_EN		0x48005010
#endif

#define DLL_REG				(uart_base+0x000)
#define RHR_REG				(uart_base+0x000)
#define THR_REG				(uart_base+0x000)
#define DLH_REG				(uart_base+0x004)
#define IER_REG				(uart_base+0x004)
#define IIR_REG				(uart_base+0x008)

#define	RX_FIFO_CLEAR		(1<<1)
#define	FCR_REG				(uart_base+0x008)

#define	ENHANCED_EN			(1<<4)
#define	EFR_REG				(uart_base+0x008)

#define	CHAR_LENGTH_8BITS	(3<<0)
#define	NB_STOP_1BIT		(0<<2)
#define	PARITY_EN			(1<<3)
#define	PARITY_TYPE_EVEN	(1<<4)
#define	BREAK_EN			(1<<6)
#define	DIV_EN				(1<<7)
#define LCR_REG				(uart_base+0x00C)

#define	TCR_TLR				(1<<6)
#define MCR_REG				(uart_base+0x010)
#define XON1_ADDR1_REG		(uart_base+0x010)

#define RX_FIFO_E			(1<<0)          // 1, have data;  0, no data 
#define RX_OE				(1<<1)          // 1, overrun error;  0, no overrun data
#define RX_PE				(1<<2)          // 1, parity error;  0, no parity error
#define RX_FE				(1<<3)          // 1, framing error occurred;  0, No framing error
#define RX_BI				(1<<4)          // 1, a break was detected;  0, no break condition
#define TX_SR_E				(1<<6)          // 1, transmitter hold and shift registers are empty;  0, transmitter hold and shift registers are not empty
#define RX_FIFO_STS			(1<<7)          // 1, at least one parity error, framing error or break occcur in RX FIFO; 0, normal operation
#define LSR_REG				(uart_base+0x014)
#define XON1_ADDR2_REG		(uart_base+0x014)
#define MSR_REG				(uart_base+0x018)
#define TCR_REG				(uart_base+0x018)
#define XOFF1_REG			(uart_base+0x018)
#define SPR_REG				(uart_base+0x01C)
#define TLR_REG				(uart_base+0x01C)
#define XOFF2_REG			(uart_base+0x01C)

#define	MODE_SELECT_DISABLE			(0x07<<0)
#define	MODE_SELECT_UART_16XMODE	(0<<0)
#define MDR1_REG			(uart_base+0x020)
#define MDR2_REG			(uart_base+0x024)
#define UASR_REG			(uart_base+0x038)
#define SCR_REG				(uart_base+0x040)
#define SSR_REG				(uart_base+0x044)

#define MVR_REG				(uart_base+0x050)

#define	SOFTRESET			(1<<1)
#define SYSC_REG			(uart_base+0x054)

#define	RESETDONE			(1<<0)
#define SYSS_REG			(uart_base+0x058)
#define WER_REG				(uart_base+0x05C)

#define RXFIFO_LVL_REG		(uart_base+0x064)
#define TXFIFO_LVL_REG		(uart_base+0x068)
#define IER2_REG			(uart_base+0x06C)
#define ISR2_REG			(uart_base+0x070)
#define MDR3_REG			(uart_base+0x080)


#define SET_CONFIGMODE_A()		__raw_writel(0x80, LCR_REG)
#define SET_CONFIGMODE_B()		__raw_writel(0xBF, LCR_REG)
#define SET_OPERATIONAL_MODE()	__raw_writel(__raw_readl(LCR_REG)&~DIV_EN, LCR_REG)


void __iomem	*uart_base;



void set_baudrate(u32 baudrate)
{
    u32 div_value;
    u32 dlh;
    u32 dll;
    u8 temp;


//    TRACE_TO("enter %s, BaudRate = %X\n", __FUNCTION__, baudrate);

    temp = __raw_readl(LCR_REG);
   
    SET_CONFIGMODE_B();
	__raw_writel(__raw_readl(EFR_REG)|ENHANCED_EN, EFR_REG);	// Enables writing to IER_REG bits [7:4], FCR_REG bits [5:4], and MCR_REG bits [7:5]
    SET_OPERATIONAL_MODE();
	__raw_writel(0, IER_REG);				// Clear register IER,Disable all the UART interrupt and Disable sleep mode
    SET_CONFIGMODE_B();

    div_value=48000000/(16*baudrate);
    dlh = div_value / 256;
    dll = div_value % 256;

    __raw_writel(dlh, DLH_REG);
    __raw_writel(dll, DLL_REG);

    SET_CONFIGMODE_B();      // Disables writing to IER_REG bits [7:4], FCR_REG bits [5:4], and MCR_REG bits [7:5]
	__raw_writel(__raw_readl(EFR_REG)&~ENHANCED_EN, EFR_REG);

    __raw_writel(temp, LCR_REG);

//	TRACE_TO("exit %s\n", __func__);
}

static void uart_init_reg(void)
{
	unsigned int *mreg;
	unsigned int *reg_oe;
	unsigned int tmp;
	unsigned int *reg;



//    TRACE_TO("enter %s\n", __func__);

	// clock enable
	reg = ioremap(UART_FUNC_CLK_EN, 1);
	__raw_writel(__raw_readl(reg)|EN_UART, reg);
	iounmap(reg);
	reg = ioremap(UART_INTF_CLK_EN, 1);
	__raw_writel(__raw_readl(reg)|EN_UART, reg);
	iounmap(reg);

	// port initial
#ifdef	SMARTCARD_IO_UART2
	mreg = ioremap(0x48002170, 4);      
	tmp = __raw_readl(mreg);
	tmp = tmp & 0x00000000;
	tmp = tmp | 0x01190001;
	__raw_writel(tmp,mreg);
#else
	reg_rx = ioremap(0x4800219C, 4);	// 165, rx
	tmp = __raw_readl(reg_rx);
	tmp = tmp & 0x0000FFFF;
	tmp = tmp | 0x01190000;
	__raw_writel(tmp,reg_rx);
	reg_tx = ioremap(0x480021A0, 4);	// 166, tx
	tmp = __raw_readl(reg_tx);
	tmp = tmp & 0xFFFF0000;
	tmp = tmp | 0x00000001;
	__raw_writel(tmp,reg_tx);
#endif

#if 0
#ifdef	SMARTCARD_IO_UART2
	reg_oe = ioremap(0x49056034, 4);
	tmp = __raw_readl(reg_oe);
	tmp = tmp | 0x00008000;
	tmp = tmp & 0xffffbfff;
	__raw_writel(tmp,reg_oe);
#else
	reg_oe = ioremap(0x49058034, 4);
	tmp = __raw_readl(reg_oe);
	tmp = tmp | 0x00000020;
	tmp = tmp & 0xffffffbf;
	__raw_writel(tmp,reg_oe);
#endif
#endif

	__raw_writel(SOFTRESET, SYSC_REG);		// reset UART module        
	while(!(__raw_readl(SYSS_REG)&RESETDONE));// wait for the end of the reset operation complete

    SET_CONFIGMODE_B();
	__raw_writel(__raw_readl(EFR_REG)|ENHANCED_EN, EFR_REG);
    SET_CONFIGMODE_A();
	__raw_writel(__raw_readl(MCR_REG)|TCR_TLR, MCR_REG);   
	__raw_writel(0x07, FCR_REG);				//  FIFO_EN: enbale
    SET_CONFIGMODE_B();
    __raw_writel(0xFF, TLR_REG);	// RX_FIFO_TRIG: 15 * 4 bytes, TX_FIFO_TRIG: 15 * 4 bytes,
             
	__raw_writel(MODE_SELECT_DISABLE, MDR1_REG);				// Disable UART, it is mandatory that MODE_SELECT = Disable before initializing or modifying clock parameter controls
    set_baudrate(9600);   // set default fidi = 11
    
    __raw_writel(PARITY_TYPE_EVEN|PARITY_EN|NB_STOP_1BIT|CHAR_LENGTH_8BITS, LCR_REG);             
	__raw_writel(MODE_SELECT_UART_16XMODE, MDR1_REG);// UART 16x mode
    SET_OPERATIONAL_MODE();

//	TRACE_TO("exit %s\n", __func__);
}

void uart_clear_fifo(void)
{
	__raw_writel(__raw_readl(FCR_REG)|RX_FIFO_CLEAR, FCR_REG);	// clears the receive FIFO and resets its counter logic to 0 after clearing FIFO
}

void put_uart_outline_low(void)
{
	__raw_writel(__raw_readl(LCR_REG)|BREAK_EN, LCR_REG);
}
void put_uart_outline_normal(void)
{
	__raw_writel(__raw_readl(LCR_REG)&~BREAK_EN, LCR_REG);
}

static void uart_trans_one_byte(struct icc_info *icc, u8 data)
{
    u8 i;


	__raw_writeb(data, THR_REG);  
	while(!(__raw_readl(LSR_REG)&TX_SR_E));		// waiting for transmitter complete

    for(i = 0; i < (icc->extra_GT + 1); i++)
    {
        udelay(icc->cur_ETU);
    }
}

static int uart_recv_one_byte(struct icc_info *icc, u8 *data)
{
    u8 tempReg;


	while(!(__raw_readl(LSR_REG)&RX_FIFO_E))			// waiting for a data be received
    {
        if(icc->common->time_out == true)
			return	-ICC_ERRORCODE_ICC_MUTE;
    }	

    set_waiting_timer(icc, icc->WT);

    *data = __raw_readl(RHR_REG);
    tempReg = __raw_readl(LSR_REG);
	
    if(tempReg&RX_FIFO_STS)    // whether parity error, framing error, or break occure
    {
        if(tempReg&RX_BI)
        {
            return	-ICC_ERRORCODE_HW_ERROR;
        }
        if(tempReg&RX_FE)
        {
            return	-ICC_ERRORCODE_FRAMERROR;
        }
        if(tempReg&RX_PE)
        {
            return	-ICC_ERRORCODE_XFR_PARITY_ERROR;
        }
    }
    if(tempReg&RX_OE)
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
//			TRACE_TO("ret = %d\n", ret);
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
	
    uart_base = ioremap(UART_BASE_ADDR, 4096);
    if(!uart_base)
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
    iounmap(uart_base);
    uart_base = NULL;
}

