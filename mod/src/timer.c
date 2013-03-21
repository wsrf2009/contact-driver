/*
* Name: timer source file
* Date: 2012/10/18
* Author: Alex Wang
* Version: 1.0
*/

#include <linux/timer.h>

#include "gpio.h"
#include "common.h"
#include "debug.h"
#include "timer.h"

volatile bool timeOut;


extern unsigned int *Reg_83C26_RESET;

struct timer_list WaitingTimer;



static void Handler_WaitTimerExpire(unsigned long data)
{

    timeOut = true;
//    WaitingTimer.expires = jiffies + HZ;
    add_timer(&WaitingTimer);
//    if(BITisSET(*Reg_83C26_RESET, 4))
//    {
//        Set_GPIO_Low(Reg_83C26_RESET, OFFS_0);
//    }
//    else
//    {
//        Set_GPIO_High(Reg_83C26_RESET, OFFS_0);
//    }
}

void Waitingtimer_Init(void)
{
    init_timer(&WaitingTimer);
    WaitingTimer.function = Handler_WaitTimerExpire;
    WaitingTimer.data = 0;                              // it is the argument of the timer_list.function
    WaitingTimer.expires = jiffies + HZ;
    add_timer(&WaitingTimer);
}

void Set_WaiTingTime(unsigned long wTime)
{
    unsigned  long L_wTime;


    L_wTime = (wTime / 1000) / (1000 / HZ);
    timeOut = false;
    mod_timer(&WaitingTimer, jiffies + L_wTime + 1);
}


void WaitingTimer_Uninit(void)
{
    del_timer(&WaitingTimer);
}


