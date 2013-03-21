/*
* Name: common head file
* Date: 2012/10/22
* Author: Alex Wang
* Version: 1.0
*/

#ifndef COMMON_H
#define COMMON_H

#define bool   unsigned char
#define true   1
#define false  0

#define SET_BIT(reg,n)       reg |= (1<<n)
#define CLEAR_BIT(reg,n)     reg &= ~(1<<n)
#define BITisSET(reg,n)      reg & (1<<n)
#define BITisCLEAR(reg,n)    ((reg & (1<<n)) == 0)


unsigned char Byte_Direct2Inverse(unsigned char DirData);
unsigned int Caculate_Etu(unsigned char FiDi);
unsigned int Caculate_BuadRate(unsigned char FiDi);

#endif
