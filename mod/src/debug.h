/*
* Name: Debug Message
* Date: 2012/10/08
* Author: Alex Wang
* Version: 1.0
*/

#ifndef DEBUG_H
#define DEBUG_H

#include <linux/kernel.h>

#define Debug

#ifdef Debug
#define PrtMsg(arg...)  printk(arg)
#else
#define PrtMsg(arg...)
#endif





#endif
