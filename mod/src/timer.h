/*
* Name: timer source file
* Date: 2012/10/25
* Author: Alex Wang
* Version: 1.0
*/

#ifndef TIMER_H
#define TIMER_H

extern volatile bool timeOut;


void Waitingtimer_Init(void);
void Set_WaiTingTime(unsigned long wTime);
void WaitingTimer_Uninit(void);



#endif
