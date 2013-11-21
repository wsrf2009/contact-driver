/*******************************************************************************
ICC Driver.
Language: C
Platform: A8/linux2.6.37
Compiler:

History:
--------------------------------------------------------------------------------
[Ver: 0.1	Date: 2013/10/18	By: Felix Mo (Logyi)]
The First Draft.

*******************************************************************************/

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/sched.h> 
#include <linux/err.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <plat/dmtimer.h>
#include <asm/io.h>
#include <linux/clk.h>


#define PWM_OVERFLOW_VALUE   2
#define PWM_MACH_VALUE       1
#define PWM_TCRR_REG_VALUE   (0xffffffff - PWM_OVERFLOW_VALUE)
#define PWM_MACH_REG_VALUE   (0xffffffff - PWM_MACH_VALUE)


struct omap_dm_timer *gptimer;

int Init_PWM(void)
{
	unsigned int tmp;
	unsigned int *ioport_reg;

	ioport_reg= ioremap(0x480020b4, 4);      
	tmp = __raw_readl(ioport_reg);
	tmp = tmp & 0x0000ffff;
	tmp = tmp | 0x00030000;
	__raw_writel(tmp,ioport_reg);
	iounmap(ioport_reg);


	gptimer = omap_dm_timer_request_specific(9);
    omap_dm_timer_set_source(gptimer, OMAP_TIMER_SRC_SYS_CLK);
    omap_dm_timer_set_pwm(gptimer,1,1,2);

    omap_dm_timer_set_load_start(gptimer,1,PWM_TCRR_REG_VALUE);
	omap_dm_timer_set_match(gptimer,1,PWM_MACH_REG_VALUE);

    omap_dm_timer_start(gptimer);
    
	return 0;
}

int Deinit_PWM(void)
{
	omap_dm_timer_free(gptimer);
	return 0;
}
