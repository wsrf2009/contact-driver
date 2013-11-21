
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/err.h>
#include <linux/gpio.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/string.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <asm/io.h>
#include <linux/clk.h>
#include <linux/err.h>
#include <plat/clock.h>



#define AT83C26_DEVICE_NAME		"at83c26"
#define AT83C26_RESET_GPIO		161
#define AT83C26_IRQ_GPIO		156

#define GENERAL_CALL_RESET		0x06
#define RESET					0xFF
#define WRITE_CONFIG			0x80
#define WRITE_TIMER				0xFC
#define WRITE_INTERFACE			0X00
#define WRITE_CONFIG_SC_ON_DCDCB	0xF8
#define	WRITE_SC2_INTERFACE			0xF9
#define WRITE_SC3_INTERFACE			0xFA
#define WRITE_SC4_INTERFACE			0xFB
#define WRITE_SC5_INTERFACE			0xFD
#define WRITE_DCDCB_CONFIG			0xFE
#define WRITE_SLEW_CTRL_CONFIG		0xF7

#define WRITE_COMMAND_1				GENERAL_CALL_RESET
#define WRITE_COMMAND_2				RESET
#define WRITE_COMMAND_3				WRITE_CONFIG
#define WRITE_COMMAND_4				WRITE_TIMER
#define WRITE_COMMAND_5				WRITE_INTERFACE
#define WRITE_COMMAND_6				WRITE_CONFIG_SC_ON_DCDCB
#define WRITE_COMMAND_7				WRITE_SC2_INTERFACE
#define WRITE_COMMAND_8				WRITE_SC3_INTERFACE
#define WRITE_COMMAND_9				WRITE_SC4_INTERFACE
#define WRITE_COMMAND_10			WRITE_SC5_INTERFACE
#define WRITE_COMMAND_11			WRITE_DCDCB_CONFIG
#define WRITE_COMMAND_12			WRITE_SLEW_CTRL_CONFIG

#define CIO_DISCON         0x00
#define CIO_CON            0x01
#define CIO_HIGH           0x01
#define CIO_LOW            0x00

#define SC1_VOK            0x10
#define DCB_VOK            0x20
#define SCB_VOK            0x40

#define SCB_V00            0x00        // SCx_CFG0
#define SCB_V18            0x01        // SCx_CFG0
#define SCB_V30            0x02        // SCx_CFG0
#define SCB_V50            0x03	       // SCx_CFG0

#define SCB_CLK0RST0       0x0C        // SCx_CFG2: stop CCLKx with low level, CRSTx = 0
#define SCB_CLK1RST0       0x14        // SCx_CFG2: have CCLKx runing, CRSTx = 0
#define SCB_CLK1RST1       0x34        // SCx_CFG2: have CCLKx runing, CRSTx = 1



struct read_command1_description
{
	u8			sc1_status;
	u8			sc1_cfg0;
	u8			sc1_cfg1;
	u8			sc1_cfg2;
	u8			sc1_cfg3;
	u8			sc1_cfg4;
	u8			sc1_interface;
	u8			timer_msb;
	u8			timer_lsb;
	u8			capture_msb;
	u8			capture_lsb;
}__attribute__((packed, aligned(1)));

struct read_command2_description
{
	u8			statusb;
	u8			io_select;
	u8			interface_b;
	u8			itdis;
}__attribute__((packed, aligned(1)));

struct read_command3_description
{
	u8			sc2_cfg0;
	u8			sc2_cfg1;
	u8			sc2_cfg2;
}__attribute__((packed, aligned(1)));

struct read_command4_description
{
	u8			sc3_cfg0;
	u8			sc3_cfg2;
}__attribute__((packed, aligned(1)));

struct read_command5_description
{
	u8			sc4_cfg0;
	u8			sc4_cfg2;
}__attribute__((packed, aligned(1)));

struct read_command6_description
{
	u8			sc5_cfg0;
	u8			sc5_cfg2;
}__attribute__((packed, aligned(1)));

struct read_command7_description
{
	u8			dcdcB;
	u8			ldo;
}__attribute__((packed, aligned(1)));

struct read_command8_description
{
	u8			slew_ctrl_1;
	u8			slew_ctrl_2;
	u8			slew_ctrl_3;
}__attribute__((packed, aligned(1)));

union read_command_description
{
	char			recv_buf[12];
	struct read_command1_description cmd1;
	struct read_command2_description cmd2;
	struct read_command3_description cmd3;
	struct read_command4_description cmd4;
	struct read_command5_description cmd5;
	struct read_command6_description cmd6;
	struct read_command7_description cmd7;
	struct read_command8_description cmd8;
};


struct at83c26_common
{
    struct i2c_client	*i2c_client;
	struct i2c_driver	*i2c_driver;
    
    struct work_struct wk_at83c26;
    struct workqueue_struct *wq_at83c26;

	struct clk *sys_clkout2;
	struct clk *sys_clkout2_src;
	struct clk *func96m_clk;


    
    int reset_pin;
	int irq_pin;

	union read_command_description	read_buf;
};



static struct at83c26_common *at83c26;



static const u8 SC1_IOon[] = {0x20};             // SC1_INTERFACE: connect CIO1 to IO1
static const u8 SC1_IOoff[] = {0x40};            // SC1_INTERFACE: connect CIO1 to CARDIO1 , and set it as 0

static const u8 SC1_CLK0RST0[] = {0x21};        // SC1_INTERFACE: Stop CCLK1 with low level, CRST1 = 0
static const u8 SC1_CLK1RST0[] = {0x03};        // SC1_INTERFACE: have CCLK1 runing,         CRST1 = 0
static const u8 SC1_CLK1RST1[] = {0x13};        // SC1_INTERFACE: have CCLK1 runing,         CRST1 = 1

static u8 VDCB_INITIAL[] = {0x03, 0x0F};

static u8 SC1_PWR[] = {0x80, 0x12, 0x00, 0x00, 0x08};        // SC1_CFG0:SC1_CFG1:SC1_CFG2:SC1_CFG3:SC1_CFG4
static u8 SC2_PWR[] = {0x00, 0x02, 0x08};              // CMD:SC2_CFG0:SC2_CFG1:SC2_CFG2
static u8 SC3_PWR[] = {0x00, 0x08};                    // CMD:SC3_CFG0:SC3_CFG2
static u8 SC4_PWR[] = {0x00, 0x08};                    // CMD:SC4_CFG0:SC4_CFG2
static u8 SC5_PWR[] = {0x00, 0x08};                    // CMD:SC5_CFG0:SC5_CFG2

static u8 SC1_INTFACE[] = {0x60};
static u8 SCx_INTFACE[] = {0x00, 0x00, 0xFF};    // CIOx = CARDIOx, and disable these interrupt of smart card interface 2,3,4,5





int at83c26_read_command(u8 cmd) 
{
    int ret;
	char	cmd_byte;


//    TRACE_TO("enter %s with Cmd = %d\n", __func__, cmd);

    switch (cmd)
    {
        case 1:
			cmd_byte = WRITE_COMMAND_4;
			break;
	    
        case 2:
			cmd_byte = WRITE_COMMAND_6;
			break;
	    
        case 3:
			cmd_byte = WRITE_COMMAND_7;
			break;
	    
        case 4:
			cmd_byte = WRITE_COMMAND_8;
			break;
	  
        case 5:
			cmd_byte = WRITE_COMMAND_9;
			break;
	    
        case 6:
			cmd_byte = WRITE_COMMAND_10;
			break;
	    
        case 7:
			cmd_byte = WRITE_COMMAND_11;
			break;
	    
        case 8:
			cmd_byte = WRITE_COMMAND_12;
			break;
	    
        default:
			WARN_TO("invalid paramter\n");
	    	ret = -EINVAL;
	    	goto err;
    }

	if(!at83c26->i2c_client)
	{
		ERROR_TO("have not connect to i2c client\n");
		ret = -EFAULT;
		goto err;
	}
	
	if((ret = i2c_master_send(at83c26->i2c_client, &cmd_byte, 1)) < 0)
	{
		ERROR_TO("fail to sending data to at83c26\n");
		goto err;
	}
	
    if((ret = i2c_master_recv(at83c26->i2c_client, at83c26->read_buf.recv_buf, 12)) < 0)
    {
    	ERROR_TO("fail to reading data fromm at83c26\n");
		goto err;
    }

//	for(i=0; i<12; i++)
//		TRACE_TO("[%d] = %02X\n", i, at83c26->read_buf.recv_buf[i]);

    ret = 0;
    
err:
	
//    TRACE_TO("exit %s\n", __func__);
	
    return	ret;
}

int at83c26_send_command(u8 cmd, u8 *send_buf, u32 send_len)
{
    char data_buf[5];
    int ret;
//	u8 i;
    
	
//	TRACE_TO("enter %s with Cmd = %d\n", __func__, Cmd);

//	for(i=0; i<send_len; i++)
//		TRACE_TO("[%d] = %02X\n", i, send_buf[i]);

    switch (cmd)
    {
        case 1:
	    	data_buf[0] = 0x06;
	    	send_len = 0;
	    	break;
		case 2:
	    	data_buf[0] = 0xFF;
	    	send_len = 0;
	    	break;
		case 3: 
			memcpy(data_buf, (char*)send_buf, send_len);
			data_buf[0] &= ~0xC0;
            data_buf[0] |= 0x80; 
            send_len -=1;
            break;
        case 4:
            memcpy(data_buf+1, (char*)send_buf, send_len);
            data_buf[0] = 0xFC;
            break;
        case 5: 
            data_buf[0] = (char)*send_buf;
			data_buf[0] &= ~0x80;
//            CLEAR_BIT(data_buf[0], 7);
            send_len = 0;
            break;
        case 6:
            memcpy(data_buf+1, (char*)send_buf, send_len);
            data_buf[0] = 0xF8;            
            break;
        case 7:
            memcpy(data_buf+1, (char*)send_buf, send_len);
            data_buf[0] = 0xF9;
            break;
        case 8:
            memcpy(data_buf+1, (char*)send_buf, send_len);
            data_buf[0] = 0xFA;
            break;
        case 9:
            memcpy(data_buf+1, (char*)send_buf, send_len);
            data_buf[0] = 0xFB;
            break;
        case 10:
            memcpy(data_buf+1, (char*)send_buf, send_len);
            data_buf[0] = 0xFD;
            break;
        case 11:
            memcpy(data_buf+1, (char*)send_buf, send_len);
            data_buf[0] = 0xFE;
            break;
        case 12:
            memcpy(data_buf+1, (char*)send_buf, send_len);
            data_buf[0] = 0xF7;
            break;

        default:
	    	WARN_TO("invalid paramter\n");
	    	ret = -EINVAL;
	    	goto err;
    }

	if(!at83c26->i2c_client)
	{
		ERROR_TO("have not connect to i2c client\n");
		ret = -EFAULT;
		goto err;
	}
	
    if((ret = i2c_master_send(at83c26->i2c_client, data_buf, send_len+1)) < 0)
    {
    	ERROR_TO("fail to sending data to at83c26");
        goto err;
    }
	
    ret = 0;

err:

//    TRACE_TO("exit %s\n", __func__);
	
    return	ret;
}

int at83c26_CIOx(u8 slot, u8 con, u8 cio)
{
	int ret = 0;
	union read_command_description *rsp4 = &at83c26->read_buf;
	u8	sc1_interface;
	

//    TRACE_TO("enter %s. slot%d, con = %d, cio = %d\n", __func__, icc->slot, con, cio);

//	INFO_TO("%s ==> %s card[%d] from at83c26\n", __func__, con ? "connect" : "cut", slot);

    if(slot == 0)                     // Card1
    {
        if((ret = at83c26_read_command(1)) < 0)    goto done;

		sc1_interface = rsp4->cmd1.sc1_interface;
		
        if(con)
        {
            if((ret = at83c26_read_command(2)) < 0)
				goto done;

            rsp4->cmd2.io_select = 0x00;				// IO_SELECT = 0x00, CIO1 = IO1;
            if((ret = at83c26_send_command(6, &rsp4->cmd2.io_select, 3)) < 0)
				goto done;
			
            CLEAR_BIT(sc1_interface, 6);
        }
        else
        {
            if(cio)		SET_BIT(sc1_interface, 0);
            else	CLEAR_BIT(sc1_interface, 0);   // set CIO1 level

            SET_BIT(sc1_interface, 6);
        }
        // send SC1_INTERFACE
        if((ret = at83c26_send_command(5, &sc1_interface, 1)) < 0)
			goto done;
		
    }
    else                                     // Card2 ~ Card5
    {
        if((ret = at83c26_read_command(2)) < 0)
			goto done;
        
        switch(slot)
        {
            case 1: 
                if(con)
                {
                    rsp4->cmd2.io_select = 0x01;                 // IO_SELECT
                    CLEAR_BIT(rsp4->cmd2.itdis, 4);           // ITDIS
                }
                else
                {
                    if(cio)		SET_BIT(rsp4->cmd2.interface_b, 2);
                    else	CLEAR_BIT(rsp4->cmd2.interface_b, 2);         // INTERFACEB

                    SET_BIT(rsp4->cmd2.itdis, 4);         // ITDIS
                }
                break; 
            
            case 2: 
                if(con)
                {						
                    rsp4->cmd2.io_select = 0x02;                  // IO_SELECT
                    CLEAR_BIT(rsp4->cmd2.itdis, 5);           // ITDIS
                }
                else
                {
                    if(cio)		SET_BIT(rsp4->cmd2.interface_b, 3);
                    else	CLEAR_BIT(rsp4->cmd2.interface_b, 3);         // INTERFACEB

                    SET_BIT(rsp4->cmd2.itdis, 5);         // ITDIS
                    
                }
                break; 

       
            case 3:
                if(con)
                {						
                    rsp4->cmd2.io_select = 0x03;                  // IO_SELECT
                    CLEAR_BIT(rsp4->cmd2.itdis, 6);           // ITDIS
                }
                else
                {
                    if(cio)		SET_BIT(rsp4->cmd2.interface_b, 4);
                    else	CLEAR_BIT(rsp4->cmd2.interface_b, 4);         // INTERFACEB

                    SET_BIT(rsp4->cmd2.itdis, 6);         // ITDIS
                    
                }
                break;

            case 4: 
                if(con)
                {						
                    rsp4->cmd2.io_select = 0x04;                  // IO_SELECT
                    CLEAR_BIT(rsp4->cmd2.itdis, 7);           // ITDIS
                }
                else
                {
                    if(cio)		SET_BIT(rsp4->cmd2.interface_b, 5);
                    else	CLEAR_BIT(rsp4->cmd2.interface_b, 5);         // INTERFACEB

                    SET_BIT(rsp4->cmd2.itdis, 7);         // ITDIS
                    
                }
                break;
           
            default:
				ret = -EBADSLT;
				goto done;

        }

        if((ret = at83c26_send_command(6, &rsp4->cmd2.io_select, 3)) < 0)
        {
            goto done;
        }
    }

    
done:
	
//    TRACE_TO("exit %s, ret = %X\n", __func__, ret);
	
    return(ret);
}

int at83c26_CVCCx(u8 slot, u8 cvcc)
{
    int ret;
    int i = 0;
    int mask;
	char *card_vcc;
	union read_command_description *rsp4 = &at83c26->read_buf;


//    TRACE_TO("enter %s. slot%d, cvcc = %d\n", __func__, slot, cvcc);

	if(cvcc == 1)
		card_vcc = "1.8v";
	else if(cvcc == 2)
		card_vcc = "3.3v";
	else if(cvcc == 3)
		card_vcc = "5.0v";
	else 
		card_vcc = "0v";
	
	INFO_TO("%s ==> set card[%d] as %s\n", __func__, slot, card_vcc);

    if(slot)                         // Card2 ~ Card5
    {
        if((ret = at83c26_read_command(7)) < 0)	
			goto err;

        CLEAR_BIT(rsp4->cmd7.ldo, (slot+3));
		
        if((ret = at83c26_send_command(11, &rsp4->cmd7.dcdcB, 2)) < 0)	// enable VDCB, VDCB=5.2v
			goto err;
		
    }

    // enable CVCC
    switch(slot)
    {
        case 0:
            if((ret = at83c26_read_command(1)) < 0)
				goto err;
			
            mask = SC1_VOK;                  // VCARD1=CVCC, drive a low level on the CRST1 pin, indicate the card presence detector is open when no
            rsp4->cmd1.sc1_cfg0 &= 0xFC;        // card is insert, enable the internal pull-up on the CPRES pin, DCK[2:0]=1,prescaler factor=2, DCCLK=CLK/2 
            rsp4->cmd1.sc1_cfg0 |= cvcc;
			
            if((ret = at83c26_send_command(3, &rsp4->cmd1.sc1_cfg0, 5)) < 0 )	// CCLK1=A2, drive the CRST1 through the CARDRST bit
				goto err;
			
			break;

        case 1:
            if((ret = at83c26_read_command(3)) < 0)	
				goto err;
			
            mask = SCB_VOK;                  // VCARD2=CVCC, drive a low level on the CRST2 pin, stop CCLK2 with a low level, CCLK2=A2, 
            rsp4->cmd3.sc2_cfg0 &= 0xFC;
            rsp4->cmd3.sc2_cfg0 |= cvcc;      // indicate the card presence detector is open when no card is insert 

			if((ret = at83c26_send_command(7, &rsp4->cmd3.sc2_cfg0, 3)) < 0)	// enable the internal pull-up on the CPRES2 pin, SC2 working at SAM mode
				goto err;
			
            break;      

        case 2:
            if((ret = at83c26_read_command(4)) < 0)	
				goto err;

            mask = SCB_VOK;                  // VCARD3=CVCC, drive a low level on the CRST3 pin, stop CCLK3 with a low level,CCLK3=A2
            rsp4->cmd4.sc3_cfg0 &= 0xFC;
            rsp4->cmd4.sc3_cfg0 |= cvcc;
			
            if((ret = at83c26_send_command(8, &rsp4->cmd4.sc3_cfg0, 2)) < 0)
				goto err;

            break;    

        case 3:   
            if((ret = at83c26_read_command(5)) < 0)
				goto err;

            mask = SCB_VOK;                 // VCARD4=CVCC, drive a low level on the CRST4 pin, stop CCLK4 with a low level, CCLK4=A2
            rsp4->cmd5.sc4_cfg0 &= 0xFC;
            rsp4->cmd5.sc4_cfg0 |= cvcc;
			
            if((ret = at83c26_send_command(9, &rsp4->cmd5.sc4_cfg0, 2)) < 0)
				goto err;

            break; 

        case 4:
            if((ret = at83c26_read_command(6)) < 0)	
				goto err;

            mask = SCB_VOK;                 // VCARD5=CVCC, drive a low level on the CRST5 pin, stop CCLK5 with a low level, CCLK5=A2
            rsp4->cmd6.sc5_cfg0 &= 0xFC;
            rsp4->cmd6.sc5_cfg0 |= cvcc;
			
            if((ret = at83c26_send_command(10, &rsp4->cmd6.sc5_cfg0, 2)) < 0)
				goto err;

            break; 

        default:
	    	ret = -EBADSLT;
            goto err;                                               
    }

    if(cvcc == 0)
    {
        return(0);
    }

    do
    {
        if((ret = i2c_master_recv(at83c26->i2c_client, at83c26->read_buf.recv_buf, 4)) < 0)
			goto err;

    }while((!(at83c26->read_buf.recv_buf[0] & mask)) && (++i < 100));    // whether the output voltage remains within the voltage range specified by VCARDx[1:0] bits

    if(!(at83c26->read_buf.recv_buf[0] & mask))
    {
    	ERROR_TO("fail to power on card slot%d\n", slot);
        ret = -EIO;
		goto err;
    }

    ret = 0;
    
err:
	
//    TRACE_TO("exit %s\n", __func__);
	
    return(ret);
}

int at83c26_CCLKx(u8 slot, u8 run, u8 cclk)
{
    int ret;
	union read_command_description *rsp4 = &at83c26->read_buf;


    
//    TRACE_TO("enter %s. slot%d, run = %d, cclk = %d\n", __func__, slot, run, cclk);

//	INFO_TO("%s == > set card[%d] clock %s\n", __func__, slot, run ? "run" : "stop");

    // Enable clock
    switch(slot)
    {
        case 0:
            if((ret = at83c26_read_command(1)) < 0)	
				goto err;

            // stop or run CCLK1 ?
            if(run)
            {
                CLEAR_BIT(rsp4->cmd1.sc1_interface, 5);
            }
            else
            {
                SET_BIT(rsp4->cmd1.sc1_interface, 5);
				
                // drive a high  or low level on CCLK1
                if(cclk)	SET_BIT(rsp4->cmd1.sc1_interface, 1);
                else	CLEAR_BIT(rsp4->cmd1.sc1_interface, 1);

            }
			
            if ((ret = at83c26_send_command(5, &rsp4->cmd1.sc1_interface, 1)) < 0)	// have CCLK1 runing ccroding to CKS1, CCLK1=A2, CRST1=0, CC81=0, CC41=0,CIO1=1
				goto err;

            break;
 
        case 1:
            if((ret = at83c26_read_command(3)) < 0)
				goto err;

            // stop or run CCLK2 ?
            if(run)
            {
                CLEAR_BIT(rsp4->cmd3.sc2_cfg2, 3);
            }
            else
            {
                SET_BIT(rsp4->cmd3.sc2_cfg2, 3);
				
                // drive a high  or low level on CCLK2
                if(cclk)	SET_BIT(rsp4->cmd3.sc2_cfg2, 4);
                else	CLEAR_BIT(rsp4->cmd3.sc2_cfg2, 4);

            }

            if((ret = at83c26_send_command(7, &rsp4->cmd3.sc2_cfg0, 3)) < 0)
				goto err;

            break;

        case 2:
            if((ret = at83c26_read_command(4)) < 0)
				goto err;

            // stop or run CCLK3 ?
            if(run)
            {
                CLEAR_BIT(rsp4->cmd4.sc3_cfg2, 3);
            }
            else
            {
                SET_BIT(rsp4->cmd4.sc3_cfg2, 3);
				
                // drive a high  or low level on CCLK3
                if(cclk)	SET_BIT(rsp4->cmd4.sc3_cfg2, 4);
                else	CLEAR_BIT(rsp4->cmd4.sc3_cfg2, 4);

            }
            if ((ret = at83c26_send_command(8, &rsp4->cmd4.sc3_cfg0, 2)) < 0)
				goto err;

            break;

        case 3:
			if((ret = at83c26_read_command(5)) < 0)	
				goto err;
         
			// stop or run CCLK4 ?
			if(run)
			{
				CLEAR_BIT(rsp4->cmd5.sc4_cfg2, 3);
			}
			else
			{
				SET_BIT(rsp4->cmd5.sc4_cfg2, 3);
				 
				// drive a high  or low level on CCLK4
				if(cclk)	SET_BIT(rsp4->cmd5.sc4_cfg2, 4);
				else	CLEAR_BIT(rsp4->cmd5.sc4_cfg2, 4);

			}
			if ((ret = at83c26_send_command(9, &rsp4->cmd5.sc4_cfg0, 2)) < 0)	// have CCLK4 runing accroding to CKS4, CCLK4=A2, CRST4=0;
				goto err;	

            break;

        case 4:
             if((ret = at83c26_read_command(6)) < 0)		goto err;
         
             // stop or run CCLK5 ?
             if(run)
             {
                 CLEAR_BIT(rsp4->cmd6.sc5_cfg2, 3);
             }
             else
             {
                 SET_BIT(rsp4->cmd6.sc5_cfg2, 3);
				 
                 // drive a high  or low level on CCLK5
                 if(cclk)	SET_BIT(rsp4->cmd6.sc5_cfg2, 4);
                 else	CLEAR_BIT(rsp4->cmd6.sc5_cfg2, 4);

             }
            if ((ret = at83c26_send_command(10, &rsp4->cmd6.sc5_cfg0, 2)) < 0)	// have CCLK5 runing accroding to CKS5, CCLK5=A2, CRST5=0;
				goto err;   

            break;

        default:
	    	ret = -EBADSLT;
            goto err;   
    }


err:
	
//    TRACE_TO("exit %s\n", __func__);
	
    return(ret);
}

int at83c26_CRSTx(u8 slot, u8 crst)
{
    int ret;
	union read_command_description *rsp4 = &at83c26->read_buf;


    
//    TRACE_TO("enter %s. slot%d, crst = %d\n", __func__, slot, crst);

//	INFO_TO("%s ==> set card[%d] crst %s\n", __func__, slot, crst ? "high" : "low");

    switch(slot)
    {
        case 0:
            if((ret = at83c26_read_command(1)) < 0)	
				goto err;

            if(crst)	SET_BIT(rsp4->cmd1.sc1_interface, 4);
            else	CLEAR_BIT(rsp4->cmd1.sc1_interface, 4);

            if ((ret = at83c26_send_command(5, &rsp4->cmd1.sc1_interface, 1)) < 0)	// whether enter a reset sequence accroding to ART1 bit value
				goto err;

            break;

        case 1:
            if((ret = at83c26_read_command(3)) < 0)
				goto err;

            if(crst)	SET_BIT(rsp4->cmd3.sc2_cfg2, 5);
            else	CLEAR_BIT(rsp4->cmd3.sc2_cfg2, 5);

			// whether enter a reset sequence accroding to ART2 bit value
            if ((ret = at83c26_send_command(7, &rsp4->cmd3.sc2_cfg0, 3)) < 0)
				goto err;

            break;

        case 2:
            if((ret = at83c26_read_command(4)) < 0)		goto err;

            if(crst)	SET_BIT(rsp4->cmd4.sc3_cfg2, 5);
            else	CLEAR_BIT(rsp4->cmd4.sc3_cfg2, 5);

			// whether enter a reset sequence accroding to ART3 bit value
            if ((ret = at83c26_send_command(8, &rsp4->cmd4.sc3_cfg0, 2)) < 0)
				goto err;

            break;  

        case 3:
            if((ret = at83c26_read_command(5)) < 0)	
				goto err;

            if(crst)	SET_BIT(rsp4->cmd5.sc4_cfg2, 5);
            else	CLEAR_BIT(rsp4->cmd5.sc4_cfg2, 5);

			// whether enter a reset sequence accroding to ART4 bit value
            if ((ret = at83c26_send_command(9, &rsp4->cmd5.sc4_cfg0, 2)) < 0)
				goto err;

            break;

        case 4:
            if((ret = at83c26_read_command(6)) < 0)		
				goto err;

            if(crst)	SET_BIT(rsp4->cmd6.sc5_cfg2, 5);
            else	CLEAR_BIT(rsp4->cmd6.sc5_cfg2, 5);

			// whether enter a reset sequence accroding to ART5 bit value
            if ((ret = at83c26_send_command(10, &rsp4->cmd6.sc5_cfg0, 2)) < 0)
				goto err;	

            break;

        default:
	    	ret = -EBADSLT;
            goto err;       
    }

    
err:
//    TRACE_TO("exit %s\n", __func__);
	
    return(ret);
}

int at83c26_reset(void)
{
    int ret;
	

//    TRACE_TO("enter %s\n", __func__);
    
    if((ret = at83c26_send_command(2, 0, 0)) < 0)
    {
        goto err;
    }
    udelay(100);

    // initial slot 1:  VCARD1 = 0V, CPRES1: normal open,  CCLK1 = CLK, internal pull-up
    if((ret = at83c26_send_command(3, SC1_PWR, sizeof(SC1_PWR))) < 0)
    {
        goto err;
    }

    // initial slot 1: CIO1 = 0, CCLK1 = 0, CRST1 = 0
    if((ret = at83c26_send_command(5 , SC1_INTFACE, sizeof(SC1_INTFACE))) < 0)
    {
        goto err;
    }

    // initial DCDCB: VDCB = 5.2V
    if((ret = at83c26_send_command(11, VDCB_INITIAL, sizeof(VDCB_INITIAL))) < 0)
    {
        goto err;
    }

    // initial slot 2: VCARD2 = 0V, CRST2 = 0, CCLK2 = 0,  CCLK2 = CLK
    if((ret = at83c26_send_command(7, SC2_PWR, sizeof(SC2_PWR))) < 0)
    {
        goto err;
    }
	
    // initial slot 3: VCARD3 = 0V, CRST3 = 0, CCLK3 = 0,  CCLK3 = CLK
    if((ret = at83c26_send_command(8, SC3_PWR, sizeof(SC3_PWR))) < 0)
    {
        goto err;
    }
    // initial slot 4: VCARD4 = 0V, CRST4 = 0, CCLK4 = 0,  CCLK4 = CLK
    if((ret = at83c26_send_command(9, SC4_PWR, sizeof(SC4_PWR))) < 0)
    {
        goto err;
    }
    // initial slot 5: VCARD5 = 0V, CRST5 = 0, CCLK5 = 0,  CCLK5 = CLK
    if((ret = at83c26_send_command(10, SC5_PWR, sizeof(SC5_PWR))) < 0)
    {
        goto err;
    }

    if((ret = at83c26_send_command(6, SCx_INTFACE, sizeof(SCx_INTFACE))) < 0)
    {
        goto err;
    }

	return 0;
    
err:
	
    ERROR_TO("fail to initial at83c26 registers\n");
	
    return(ret);
}

static void at83c26_wq_func(struct work_struct *work)
{
    u8 temp_sc1_status;
    u8 temp_sc1_cfg0;
	union read_command_description *rsp4 = &at83c26->read_buf;

    
    
//	TRACE_TO("enter %s\n", __func__);
    
    at83c26_read_command(1);
    temp_sc1_status = rsp4->cmd1.sc1_status;
    temp_sc1_cfg0 = rsp4->cmd1.sc1_cfg0;
    
    at83c26_read_command(7);

    if(BITisSET(temp_sc1_cfg0, 4))    // a change in card status
    {
        if(BITisSET(temp_sc1_status, 5))    // a card is detected
        {
			icc_status_changed(1);
        }
        else
        {
			icc_status_changed(0);
        }
    }

//	TRACE_TO("exit %s\n", __func__);
	
}

static irqreturn_t at83c26_irq(int irq, void *dev_id)
{
    struct at83c26_common *at83c26 = dev_id;


//	TRACE_TO("enter %s\n", __func__);
	
    schedule_work(&at83c26->wk_at83c26);

//	TRACE_TO("exit %s\n", __func__);
    
    return(IRQ_HANDLED);
}

static int at83c26_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
    int ret;
    struct at83c26_common *at83c26 = (struct at83c26_common *)(id->driver_data);
 
	struct device *dev;
	
    
    TRACE_TO("enter %s\n", __func__);
    
    at83c26->i2c_client = client;

	dev = &client->dev;

	at83c26->sys_clkout2_src = clk_get(dev, "clkout2_src_ck");
	if (IS_ERR(at83c26->sys_clkout2_src)) {
		dev_err(dev, "Could not get clkout2_src_ck clock\n");
		ret = PTR_ERR(at83c26->sys_clkout2_src);
		goto err;
	}

	at83c26->sys_clkout2 = clk_get(dev, "sys_clkout2");
	if (IS_ERR(at83c26->sys_clkout2)) {
		dev_err(dev, "Could not get sys_clkout2\n");
		ret = PTR_ERR(at83c26->sys_clkout2);
		goto err1;
	}

	at83c26->func96m_clk = clk_get(dev, "cm_96m_fck");
	if (IS_ERR(at83c26->func96m_clk)) {
		dev_err(dev, "Could not get func cm_96m_fck clock\n");
		ret = PTR_ERR(at83c26->func96m_clk);
		goto err2;
	}

	clk_set_parent(at83c26->sys_clkout2_src, at83c26->func96m_clk);
	clk_set_rate(at83c26->sys_clkout2, at83c26->func96m_clk->rate);
	clk_enable(at83c26->sys_clkout2);

	ret = gpio_request(at83c26->reset_pin, "at83c26_reset_pin");
	if(ret)
	{
		ERROR_TO("fail to requesting gpio%d\n", at83c26->reset_pin);
		goto err3;
	}

	// at83c26 hardware reset
	gpio_direction_output(at83c26->reset_pin, 1);
	udelay(400);
	gpio_set_value(at83c26->reset_pin, 0);
	udelay(200);
	gpio_set_value(at83c26->reset_pin, 1);
	udelay(400);
	
	ret = at83c26_reset();
    if(ret < 0)          // initial each slot with low level
        goto err5;
    
    INIT_WORK(&at83c26->wk_at83c26, at83c26_wq_func);

	at83c26_wq_func(NULL);
	
	at83c26_read_command(3);
	at83c26_read_command(4);
	at83c26_read_command(5);
	at83c26_read_command(6);
	
    ret = request_irq(client->irq, at83c26_irq, IRQF_TRIGGER_FALLING ,
						"at83c26_interrupt", at83c26);
    if(ret)
    {
       ERROR_TO("fail to requesting irq for at83c26\n");
       goto err6;
    }


	TRACE_TO("exit %s\n", __func__);
	
	return 0;

err6:
	destroy_work_on_stack(&at83c26->wk_at83c26);
err5:
//	gpio_free(at83c26->irq_pin);
//err4:
	gpio_free(at83c26->reset_pin);
err3:
	clk_disable(at83c26->sys_clkout2);
	clk_put(at83c26->func96m_clk);
err2:
	clk_put(at83c26->sys_clkout2);
err1:
	clk_put(at83c26->sys_clkout2_src);
err:
	at83c26->i2c_client = NULL;
	
	TRACE_TO("exit %s\n", __func__);
	
    return (ret);
}

static int at83c26_remove(struct i2c_client *client)
{
    int ret;


    TRACE_TO("enter %s\n", __func__);

    disable_irq_nosync(client->irq);
    free_irq(client->irq, at83c26);

	destroy_work_on_stack(&at83c26->wk_at83c26);

	gpio_free(at83c26->reset_pin);

    // initial DCDCA: shutdown DC/DCA
    SC1_PWR[1] = 0x20;
    if((ret = at83c26_send_command(3, SC1_PWR, 2)) < 0)
    {
        return(ret);
    }
	
    // initial DCDCB: shutdown DC/DCB
    VDCB_INITIAL[0] = 0x80;
    if((ret = at83c26_send_command(11, VDCB_INITIAL, 1)) < 0)
    {
        return(ret);
    }

	clk_disable(at83c26->sys_clkout2);
	clk_put(at83c26->func96m_clk);

	clk_put(at83c26->sys_clkout2);

	clk_put(at83c26->sys_clkout2_src);

	at83c26->i2c_client = NULL;

	TRACE_TO("exit %s\n", __func__);
	
    return(0);
}

static struct i2c_device_id at83c26_id[] = {  
    {AT83C26_DEVICE_NAME, 0 },
    {}  
};

static struct i2c_driver at83c26_driver =
{
    .probe = at83c26_probe,
    .remove= at83c26_remove,
    .driver=
    {
        .owner = THIS_MODULE,
        .name  = AT83C26_DEVICE_NAME,
    },
    
//    .id_table = at83c26_id, 
};



static int at83c26_init(void)
{
    int ret;

    
    TRACE_TO("enter %s\n", __func__);

	at83c26 = kzalloc(sizeof *at83c26, GFP_KERNEL);
	if (!at83c26)
	{
		ret = -ENOMEM;
		goto err1;
	}

	at83c26->reset_pin = AT83C26_RESET_GPIO;
	at83c26->irq_pin = AT83C26_IRQ_GPIO;

	at83c26_id[0].driver_data = (kernel_ulong_t)at83c26;
	at83c26->i2c_driver = &at83c26_driver;
	at83c26->i2c_driver->id_table = at83c26_id;

    // config AT83C26 address at i2c bus    
    if((ret = i2c_add_driver(at83c26->i2c_driver)) < 0)
    {
        ERROR_TO("adding i2c device fail\n");
        goto err2;
    }

	TRACE_TO("exit %s\n", __func__);

    return (0);

err2:
    kfree(at83c26);
err1:
	
	TRACE_TO("exit %s\n", __func__);

    return(ret);
}

static int at83c26_uninit(void)
{
    i2c_del_driver(at83c26->i2c_driver);
    
	kfree(at83c26);

    return(0);
}


