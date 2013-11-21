#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <memory.h>

#define  ContactCard_PowerOn     0x01
#define  ContactCard_PowerOff    0x02
#define  ContactCard_XfrAPDU     0x03
#define  ContactCard_WarmReset   0x04
#define  ContactCard_PTS         0x05


typedef struct
{
    unsigned char *p_iBuf;
    unsigned char *p_oBuf;
    unsigned int  iDataLen;
    unsigned int  oDataLen;
}ICC_PARAM;


#define DEVICE_NAME "/dev/CONTACT_CARD"
int fd = -1;

int ICC_Open(void)
{
	fd = open(DEVICE_NAME,0);
	if(fd == -1)
	{
		printf("Open dev fail!!!\n");
		return -1;
	}
	else
	{
		return 0;
	}
}

int ICC_Close(void)
{
	if(fd == -1)
	{
		return -1;
	}
	else
	{
		close(fd);
		return 0;
	}
}



int ICC_PowerOn(unsigned char CardIdx, unsigned char *AtrBuf, unsigned int *AtrLen)
{
	int ret = 0;
	unsigned int pCMD = 0;

	ICC_PARAM pICC;

	pICC.p_oBuf = AtrBuf;
	pICC.oDataLen = 33;

	pCMD = ContactCard_PowerOn;
	pCMD = (pCMD<<4)+CardIdx;
	if(fd == -1)
	{
		return -1;
	}
	else
	{
		pCMD = pCMD & 0x7f;
		ret = ioctl(fd,pCMD,&pICC);
		if(!ret)
		{
			*AtrLen = pICC.oDataLen;
		}
		else
		{
			pCMD = pCMD | 0x80;
			ret = ioctl(fd,pCMD,&pICC);
			if(!ret)
				*AtrLen = pICC.oDataLen;
			else
				*AtrLen = 0;
		}
	}
    return ret;
	
}

int ICC_WarmReset(unsigned char CardIdx, unsigned char *AtrBuf, unsigned int *AtrLen)
{
	int ret = 0;
	unsigned int pCMD = 0;

	ICC_PARAM pICC;

	pICC.p_oBuf = AtrBuf;
	pICC.oDataLen = 33;

	pCMD = ContactCard_WarmReset;
	pCMD = (pCMD<<4)+CardIdx;
	if(fd == -1)
	{
		return -1;
	}
	else
	{
		pCMD = pCMD & 0x7f;
		ret = ioctl(fd,pCMD,&pICC);
		if(!ret)
		{
			*AtrLen = pICC.oDataLen;
		}
		else
		{
			pCMD = pCMD | 0x80;
			ret = ioctl(fd,pCMD,&pICC);
			if(!ret)
				*AtrLen = pICC.oDataLen;
			else
				*AtrLen = 0;
		}
	}
    return ret;
	
}

int ICC_PowerOff(unsigned char CardIdx)
{
	int ret = 0;
	unsigned int pCMD = 0;

	ICC_PARAM pICC;

	pCMD = ContactCard_PowerOff;
	pCMD = (pCMD<<4)+CardIdx;
	if(fd == -1)
	{
		return -1;
	}
	else
	{
		ret = ioctl(fd,pCMD,&pICC);
	}
    return ret;
	
}


int ICC_PPSExchange(unsigned char CardIdx, unsigned char *CmdBuf, unsigned int CmdLen, unsigned char *ResBuf, unsigned int *ResLen)
{
	int ret = 0;
	unsigned int pCMD = 0;

	ICC_PARAM pICC;

	pICC.p_iBuf = CmdBuf;
	pICC.iDataLen = CmdLen;
	pICC.p_oBuf = ResBuf;
	pICC.oDataLen = 272;

	pCMD = ContactCard_PTS;
	pCMD = (pCMD<<4)+CardIdx;
	if(fd == -1)
	{
		return -1;
	}
	else
	{
		ret = ioctl(fd,pCMD,&pICC);
		*ResLen = pICC.oDataLen;
	}
    return ret;
}


int ICC_TransAPDU(unsigned char CardIdx, unsigned char *CmdBuf, unsigned int CmdLen, unsigned char *ResBuf, unsigned int *ResLen)
{
	int ret = 0;
	unsigned int pCMD = 0;

	ICC_PARAM pICC;

	pICC.p_iBuf = CmdBuf;
	pICC.iDataLen = CmdLen;
	pICC.p_oBuf = ResBuf;
	pICC.oDataLen = 272;

	pCMD = ContactCard_XfrAPDU;
	pCMD = (pCMD<<4)+CardIdx;
	if(fd == -1)
	{
		return -1;
	}
	else
	{
		ret = ioctl(fd,pCMD,&pICC);
		*ResLen = pICC.oDataLen;
	}
    return ret;
}



int main(int argc,char *argv[])
{
    
    int ret = 0,i;
	unsigned char mATR[33];
	unsigned int ATRlen = 0;

	ICC_Open();

	while(1)
	{
		memset(mATR,0,33);
		ret = ICC_PowerOn(1,mATR,&ATRlen);
		if(!ret)
		{
			printf("ColdReset ");
			for(i=0;i<ATRlen;i++)
			{
				printf("%02X ",mATR[i]);
			}
			printf("\n");
		}
		else
		{
			printf("ColdReset fail!!!\n");
		}
		sleep(5);

	}
	ICC_Close();
	return 0;
         
}
