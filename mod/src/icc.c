

#include <linux/delay.h>
#include "common.h"


#define V_CLASS_A    0x03
#define V_CLASS_B    0x02
#define V_CLASS_C    0x01
#define V_CLASS_0    0x00


#define DIRECT_CONVENTION                   0x01                // (H)LHHLHHHLLH direct convention value '3B
#define INVERSE_CONVENTION                  0x02                // (H)LHHLLLLLLH "inverse" convention value '3F'

#define SPECIFIC_MODE                       0x02                // Specific mode if TA2 is present
#define NEGOTIABLE_MODE                     0x03                // Negotiable mode if TA2 is absent

#define NOT_SUPPORT_CLK_STOP                0x00
#define SUPPORT_CLK_STOP_LOW                0x01
#define SUPPORT_CLK_STOP_HIGH               0x02

#define CLASS_DEFAULT                       0x00
#define CLASS_A                             0x01
#define CLASS_B                             0x02
#define CLASS_C                             0x04

#define C6_UNUSED                           0x00
#define C6_STANDARD_USE                     0x01
#define C6_PROPRIETARY_USE                  0x02

#define T0_TYPE                             0x00
#define T1_TYPE                             0x01

#define DEFAULT_FIDI                        0x11

#define DEFAULT_GUARDTIME                   0x0C                // Minimum Guard time value: 12 etu in T=0
#define DEFAULT_EXTRA_GUARDTIME             0x00
#define DEFAULT_BLOCK_GUARDTIME             0x17
#define DEFAULT_WAITINGTIME                 9600
#define DEFAULT_WAITING_TIME                0x0A                // Default waiting time

#define DEFAULT_BWI_CWI                     0x4D                // Block Wait Time value BWI = 4 and Character Wait Time value CWI = 13            
#define DEFAULT_IFSC                        0x20
#define DEFAULT_NAD                         0x00
#define DEFAULT_CRC                         0x01
#define DEFAULT_LRC                         0x00

#define DEFAULT_BWI                         0x04
#define DEFAULT_CWI                         0x0D

#define CHARACTER_TA                        0x00                // Character TAx
#define CHARACTER_TB                        0x01                // Character TBx
#define CHARACTER_TC                        0x02                // Character TCx
#define CHARACTER_TD                        0x03                // Character TDx

#define MAX_ATR_LEN  				32	




static const u32 FI[16] = 
    {372,372,558,744,1116,1488,1860,0, 0, 512,768,1024,1536,2048,0,0};
	
static const u32 DI[16] = 
    {0,  1,  2,  4,  8,   16,  32,  64,12,20, 0,  0,   0,   0,   0,0};


static int icc_power_off(struct icc_info *icc);



static void icc_status_changed(u8 present)
{

	common->icc[0].status |= ICC_STATUS_CHANGE;
	common->icc[0].status &= ~ICC_ACTIVE;
	
	if(present)
		common->icc[0].status |= ICC_PRESENT;
	else
		common->icc[0].status &= ~ICC_PRESENT;
}

#include "common.c"
#include "timer.c"
#include "uart.c"
#include "param.c"
#include "at83c26.c"
#include "atr.c"
#include "t0.c"
#include "t1.c"

static const u8 at_to_78[] = {CLASS_C, CLASS_B, CLASS_A};

static int icc_power_on(struct icc_info *icc, u8 *atr_buf, u32 *atr_len)
{
	u8 c_vcc = V_CLASS_C;
	int ret;
//    u8 i;


//    TRACE_TO("enter %s\n", __func__);

	if(!(icc->status & ICC_PRESENT))
	{
		ret = -ICC_ERRORCODE_ICC_MUTE;
		goto err;
	}
	
    do
    {

//        PrtMsg("welcome to the do-while cycle\n");
        icc_param_init(icc);
        set_icc_trans_param(icc);

        ret = icc_cold_reset(icc, c_vcc);
//		if(!ret)
//		{
		
			ret = icc_get_atr(icc, atr_buf, atr_len);
        	if(!ret)
			{
				icc_analyze_atr(icc, atr_buf, atr_len);
            	if(icc->class)
            	{
                	// ATR carries a class indicator 
                	if(icc->class & at_to_78[c_vcc])
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
//		}
		
        icc_power_off(icc);
        mdelay(10);
        c_vcc++;
        
    }while(c_vcc <= V_CLASS_A);

    if(ret)
    {
        ERROR_TO("fail to power on the card!\n");
        goto	err;           // accroding to 6.2.4
    }

    set_icc_trans_param(icc);
    SET_BIT(icc->T1_exchange_flag, FirstAPDUBit);        // set the first APDU bit
    icc->status |= ICC_ACTIVE;
#if 0
    for(i = 0; i < *atr_len; i++ )
    {
        TRACE_TO("AtrBuf[%d] = %X\n", i, *(atr_buf+i));
        TRACE_TO("Invervalue = %X\n", byte_direct_2_inverse(*(atr_buf+i)));
    }
#endif
err:
//	TRACE_TO("exit %s\n", __func__);
	return ret;
}

static int icc_power_off(struct icc_info *icc)
{
//	TRACE_TO("enter %s\n", __func__);
	
    at83c26_CRSTx(icc->slot, 0);
    at83c26_CCLKx(icc->slot, 0, 0);

    at83c26_CIOx(icc->slot, CIO_DISCON, CIO_LOW);
    at83c26_CVCCx(icc->slot, V_CLASS_0);

    mdelay(10);
    icc->status &= ~ICC_ACTIVE;
    at83c26_CIOx(icc->slot, CIO_CON, CIO_LOW);
	mdelay(20);

//	TRACE_TO("exit %s\n", __func__);
   
    return(0);
}

static int icc_xfr_apdu(struct icc_info *icc, u8 *cmd_buf, u32 cmd_len,
							u8 *res_buf, u32 *res_len)
{
	int	ret;

	
//    TRACE_TO("enter %s, slot = %d\n", __func__, icc->slot);

	if(!(icc->status & ICC_PRESENT))
	{
		ret = -ICC_ERRORCODE_ICC_MUTE;
		goto err;
	}

    if(icc->status & ICC_ACTIVE)
    {
        if(icc->T == T0_TYPE)
        {
            ret = icc_t0_parse(icc, cmd_buf, cmd_len, res_buf, res_len);
        }
        else
        {
            ret = icc_t1_parse(icc, cmd_buf, cmd_len, res_buf, res_len);
        }
    }
    else
    {
        ret = -ICC_ERRORCODE_NOT_ACTIVED;
    }

err:
//	TRACE_TO("exit %s, ret = %d\n", __func__, ret);

	return	ret;
}


static int icc_init(struct ifd_common	*common)
{
	int	ret;
	u8	i;

	
    TRACE_TO("enter %s\n", __func__);

//    CardParam = (CARD_PARAM*)kzalloc(5 * sizeof(CARD_PARAM), GFP_KERNEL);

//    memset(CardParam, 0, 5 * sizeof(CARD_PARAM));

	for(i=0; i<IFD_MAX_SLOT_NUMBER;	i++)
	{
		common->icc[i].slot = i;
		common->icc[i].status = ICC_PRESENT;
		common->icc[i].common = common;
	}

	ret = uart_init();
    if(ret)    
		goto	done;

	ret = at83c26_init();
    if(ret)
        goto err;

	
    
    waiting_timer_init(common); 

	goto	done;


err:
    uart_uninit();
	
done:

	TRACE_TO("exit %s\n", __func__);
	
    return	ret;;
}

static void icc_uninit(struct ifd_common *common)
{
//    kfree(CardParam);
    
    waiting_timer_uninit(common);

    at83c26_uninit();

    uart_uninit();
}
