/*
* Name: AT83C26 head file
* Date: 2012/10/10
* Author: Alex Wang
* Version: 1.0
*/

#ifndef AT83C26_H
#define AT83C26_H

#define CIO_DISCON         0x00
#define CIO_CON            0x01
#define CIO_HIGH           0x01
#define CIO_LOW            0x00

#define SC1_VOK            0x10
#define DCB_VOK            0x20
#define SCB_VOK            0x40

#define SCB_V00            0x00        // SCx_CFG0
#define SCB_V18            0x01        // SCx_CFG0
#define SCB_V30            0x02        // SCx_CFG0
#define SCB_V50            0x03         // SCx_CFG0

#define SCB_CLK0RST0       0x0C        // SCx_CFG2: stop CCLKx with low level, CRSTx = 0
#define SCB_CLK1RST0       0x14        // SCx_CFG2: have CCLKx runing, CRSTx = 0
#define SCB_CLK1RST1       0x34        // SCx_CFG2: have CCLKx runing, CRSTx = 1





int AT83C26_ReadCmd(int Cmd,  unsigned char *data, int size);
int AT83C26_SendCmd(int Cmd, unsigned char *data, int size);
int AT83C26_CIOx(unsigned char CardIdx, unsigned char CON, unsigned char CIO);
int AT83C26_CVCCx(unsigned char CardIdx, unsigned char CVCC);
int AT83C26_CCLKx(unsigned char CardIdx, unsigned char RUN, unsigned char CCLK);
int AT83C26_CRSTx(unsigned char CardIdx, int CRST);
int AT83C26_SLOTx_Init(unsigned char CardIdx);
int AT83C26_Reset(void);
int AT83C26_Init(void);
int AT83C26_Uninit(void);


#endif
