
#ifndef COMMON_H
#define COMMON_H



#define SET_BIT(reg,n)       reg |= (1<<n)
#define CLEAR_BIT(reg,n)     reg &= ~(1<<n)
#define BITisSET(reg,n)      reg & (1<<n)
#define BITisCLEAR(reg,n)    ((reg & (1<<n)) == 0)




#endif
