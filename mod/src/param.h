/*
* Name: 7816 protocol head file
* Date: 2012/10/10
* Author: Alex Wang
* Version: 1.0
*/

#ifndef PROTOCOL_PARAM_H
#define PROTOCOL_PARAM_H




typedef struct 
{  
    // Flag variable
    unsigned char CardStatus;       // Flag used to know if the power up sequence has been processed
    unsigned char Mode;             // After any answer to reset, the card is in either in specific mode or negotiable mode
    unsigned char ClkStop;          // whether the card supports clock stop
    unsigned char Class;            // classes of operating conditions accepted by the card
    unsigned char C6Use;

    // T=0 and T=1 common variable
    unsigned char T;                // a slot card use protocol T = 0 or T = 1
    unsigned char FiDi;             // Value speed com smart card = TA1
    unsigned char CurFiDi;
    unsigned char Direct;           // Define convention direct or inverse 
    unsigned char N;                // Extra Guard Time 

    // T=0 specific variable
    unsigned char WI;               // Waiting time, T=0,given by 960 x D x WI;

    // T=1 specific variable
    unsigned char Check;            // epilogue field consist in protocol T = 1
    unsigned char BWI;              // Block waiting time intger 
    unsigned char CWI;              // character waiting time intger
    unsigned char IFSC;             // IFS for the card
    unsigned char NAD;              // Node address byte

    unsigned int   CurETU;
    unsigned char  GT;
    unsigned long  WT;
    unsigned char  CheckSum;
    unsigned long  CWT;
    unsigned long  BWT;
    unsigned char  ExtraGT;

    unsigned char FirstIBlock;

    // T1ExChangeFlag: |bit7              |bit6 |bit5 |bit4 |bit3                   |bit2                  |bit1                     |bit0                   |
    // T1ExChangeFlag: |FirstAPDUBit |       |       |       |ICCChainingBit |IFDChainingBit |ICCBlockNumber |IFDBlockNumber|                             
    unsigned char T1ExChangeFlag;
}CARD_PARAM;




extern CARD_PARAM* CardParam;



void TransParam_Init(unsigned char CardIdx);
void Set_Transparam(unsigned char CardIdx);

#endif
