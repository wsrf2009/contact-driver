/*
* Name: common source file
* Date: 2012/10/22
* Author: Alex Wang
* Version: 1.0
*/

#include "atr.h"
#include "icc.h"
#include "debug.h"
#include "common.h"


unsigned char Byte_Direct2Inverse(unsigned char DirData)
{
    unsigned char InvData = 0;
    unsigned char i = 0;

    
    do
    {
        if(BITisSET(DirData,i))
        {
            CLEAR_BIT(InvData, (7 - i));
        }
        else
        {
            SET_BIT(InvData, (7 - i));
        }
        
//        DirData >>= 1;
//        InvData <<= 1;
        i++;
        
    }while(i < 8);

    return(InvData);
}


unsigned int Caculate_Etu(unsigned char FiDi)
{
    unsigned int fi;
    unsigned int di;
    unsigned int etu;


    fi = FI[(FiDi>>4) & 0x0F];        // Get the Fi and Di by inquiry table FI and DI
    di = DI[FiDi & 0x0F];

    etu =(fi)/(di * FRE_CNTACARD);     // calculate baud rate: 1 etu =F/(D*f)    7816-3

    PrtMsg("%s: FiDi = %X, fi = %X, di = %X, etu = %X\n", __FUNCTION__, FiDi, fi, di, etu);

    return(etu);
}



unsigned int Caculate_BuadRate(unsigned char FiDi)
{
    unsigned int fi;
    unsigned int di;
    unsigned int BaudRate;

    
    fi = FI[(FiDi>>4) & 0x0F];        // Get the Fi and Di by inquiry table FI and DI
    di = DI[FiDi & 0x0F];
    BaudRate = (di * FRE_CNTACARD * 1000000)/fi;

    PrtMsg("%s: FiDi = %X, fi = %X, di = %X, BaudRate = %X\n", __FUNCTION__, FiDi, fi, di, BaudRate);

    return(BaudRate);
}

