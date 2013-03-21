/*
* Name: GPIO source file
* Date: 2012/10/12
* Author: Alex Wang
* Version: 1.0
*/

#ifndef GPIO_H
#define GPIO_H


#define GPIO13_12               0x480025D8
#define GPIO23_22               0x480025EC
#define GPIO19_18               0x480025E4
#define GPIO157_156             0x4800218C


#define MODE4_GPIO              4
#define PULLUPDOWNENABLE        1
#define PULLUPDOWNDISABLE       0
#define DIREC_INPUT             1
#define DIREC_OUTPUT            0
#define Pin_HIGH                1



#define OFFS_16                 16
#define OFFS_0                  0


void Gpio_Init(unsigned int *Reg, unsigned char Mode, unsigned char PullUpDownSelect, unsigned char InOut, unsigned char Offset);
void Set_GPIO_High(unsigned int *Reg, unsigned char Offset);
void Set_GPIO_Low(unsigned int *Reg, unsigned char Offset);
void Release_Gpio_Resource(unsigned int *VirAddr);


#endif
