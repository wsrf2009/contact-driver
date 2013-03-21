/*
* Name: Card T1 head file
* Date: 2012/10/18
* Author: Alex Wang
* Version: 1.0
*/

#ifndef PROTOCOL_T1_H
#define PROTOCOL_T1_H




#define T1_HEADER                     0x03               ///< \brief Apdu header length
#define T1_NAD                        0x00
#define T1_PCB                        0x01
#define T1_LEN                        0x02

#define IFDBlockNumber                0
#define ICCBlockNumber                1
#define IFDChainingBit                2
#define ICCChainingBit                3
#define FirstAPDUBit                  7


#define WTX_RESPONSE                           0xE3

#define SLOTERROR_ICC_MUTE                     0xFE
#define SLOTERROR_T1_OTHER_ERROR               0xE4
#define SLOTERROR_T1_CHECKSUM_ERROR            0xE3
#define SLOTERROR_INVALIDBLOCK                 0xD3
#define SLOTERROR_T1_3RETRY_FAIL               0xFC
#define SLOTERROR_ICC_ABORT                    0xD4
#define SLOTERROR_XFR_PARITY_ERROR             0xFD
#define SLOTERROR_T1_3RETRY_FAIL_RESYNCH_PASS  0xE1



#define SLOTERROR_BAD_NAD                      0x10
#define SLOTERROR_BAD_LENTGH                   0x01
#define SLOTERROR_BAD_IFSC                     0x0F


typedef struct 
{  
    unsigned char NAD;
    unsigned char PCB;
    unsigned char LEN;
}T1Header;


unsigned char ParseCardT1(unsigned char CardIdx, unsigned char *CmdBuf, unsigned int CmdLen, unsigned char *ResBuf, unsigned int *ResLen);

#endif
