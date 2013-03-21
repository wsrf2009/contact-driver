/*
* Name: Smart card head file
* Date: 2012/10/15
* Author: Alex Wang
* Version: 1.0
*/

#ifndef SMARTCARD_H
#define SMARTCARD_H

#define SlotICC  0
#define SlotSAM1 1
#define SlotSAM2 2


#define FRE_CNTACARD  4        // card clock 4Mhz


#define ICC_PRESENT_BIT          0
#define ICC_ACTIVE_BIT           1
#define ICC_STATUS_CHANGE_BIT    7



int SC_PowerOn(unsigned char CardIdx, unsigned char *AtrBuf, unsigned int *AtrLen);
int SC_PowerOff(unsigned char CardIdx);
unsigned char SC_TransAPDU(unsigned char CardIdx, unsigned char *CmdBuf, unsigned int CmdLen, unsigned char *ResBuf, unsigned int *ResLen);
int SC_Device_Init(void);
void SC_Device_Uninit(void);

#endif
