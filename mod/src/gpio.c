/*
* Name: GPIO source file
* Date: 2012/10/12
* Author: Alex Wang
* Version: 1.0
*/

#include "debug.h"
#include "common.h"


void Gpio_Init(unsigned int *Reg, unsigned char Mode, unsigned char PullUpDownSelect, unsigned char InOut, unsigned char Offset)
{
    unsigned int cfgval;
    unsigned int temp;


    PrtMsg("welcome to entry the function: %s, Mode = %X, PullUpDownSelect = %X, Offset = %X\n", __FUNCTION__, Mode, PullUpDownSelect, Offset);

    cfgval = ((Mode << 0) | (InOut << 8)| (PullUpDownSelect << 3)) << Offset;     // GPIO mode, pull up enable, pin value = 0;
    if(Offset)
    {
        temp = *Reg; 
        temp &= 0x0000FFFF;
    }
    else
    {
        temp = *Reg;
        temp &= 0xFFFF0000;
    }

    cfgval |= temp;

    PrtMsg("exit the function: %s, cfgval = %X\n", __FUNCTION__, cfgval);

    *Reg = cfgval;
}


void Set_GPIO_High(unsigned int *Reg, unsigned char Offset)
{
    SET_BIT(*Reg, (4 + Offset));
}

void Set_GPIO_Low(unsigned int *Reg, unsigned char Offset)
{
    CLEAR_BIT(*Reg, (4 + Offset));
}




