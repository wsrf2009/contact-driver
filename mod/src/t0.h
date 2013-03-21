/*
* Name: Card T0 head file
* Date: 2012/10/18
* Author: Alex Wang
* Version: 1.0
*/

#ifndef PROTOCOL_T0_H
#define PROTOCOL_T0_H

// T0
#define APDU_WRITE                          0
#define APDU_READ                           1

#define APDU_HEADER                         0x05               // Apdu header length
#define APDU_CLA                            0x00               // Location of the class in APDU frame
#define APDU_INS                            0x01               // Location of the instruction in APDU frame 
#define APDU_P1                             0x02               // Location of the parameter 1 in APDU frame
#define APDU_P2                             0x03               // Location of the parameter 2 in APDU frame
#define APDU_P3                             0x04               // Location of the parameter 3 in APDU frame

#define PROCEDURE_BYTE_CONFLICT             0xF4



unsigned char ParseCardT0(unsigned char CardIdx, unsigned char *CmdBuf, unsigned int CmdLen, unsigned char *ResBuf, unsigned int *ResLen);

#endif

