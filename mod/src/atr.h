/*
* Name: ATR head file
* Date: 2012/10/18
* Author: Alex Wang
* Version: 1.0
*/

#ifndef PROTOCOL_ATR_H
#define PROTOCOL_ATR_H



#define V_CLASS_A    0x03
#define V_CLASS_B    0x02
#define V_CLASS_C    0x01
#define V_CLASS_0    0x00

#define MAXATRLEN  0x20

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

#define DIRECT_CONVENTION                   0x01                // (H)LHHLHHHLLH direct convention value '3B
#define INVERSE_CONVENTION                  0x02                // (H)LHHLLLLLLH "inverse" convention value '3F'

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

static const unsigned int FI[16] = 
    {372,372,558,744,1116,1488,1860,0, 0, 512,768,1024,1536,2048,0,0};
static const unsigned int DI[16] = 
    {0,  1,  2,  4,  8,   16,  32,  64,12,20, 0,  0,   0,   0,   0,0};

int Card_ColdReset(unsigned char CardIdx, unsigned char CardVoltage);
void Card_WarmReset(unsigned char CardIdx);
int Card_GetATR(unsigned char CardIdx, unsigned char *AtrBuf, unsigned int *AtrLen);
void Card_AnalyzeATR(unsigned char CardIdx, unsigned char *AtrBuf, unsigned int *AtrLen);

#endif

