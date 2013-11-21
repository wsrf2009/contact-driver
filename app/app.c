#include <stdio.h>
#include <fcntl.h>
#include <ctype.h>
#include <fcntl.h> //文件控制
#include <linux/ioctl.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h> 

#define  PowerOn     0x01
#define  PowerOff    0x02
#define  XfrAPDU     0x03

#define SLOT_ICC     0
#define SLOT_SAM1    1
#define SLOT_SAM2    2
#define SLOT_SAM3    3

#define IFD_CMD(n,m) ((n << 4) | m)

#define Debug

#ifdef Debug
#define PrtMsg(arg...) printf(arg)
#else
#define PrtMsg(arg...)
#endif

unsigned char RecBuf[271];
unsigned char CmdBuf[271];
int fd;

typedef struct
{
    unsigned char *p_iBuf;
    unsigned char *p_oBuf;
    unsigned int  iDataLen;
    unsigned int  oDataLen;

}IFD_PARAM;

IFD_PARAM UsrParam;



unsigned char StrToHex(char* src, int len)
{
   int i=0;
   unsigned char a=0;

   if(src == NULL)   return 0;
   else
   {
      while(len--)
      {
//        PrtMsg("src + %d = 0x%X\n", i, *(src+i));
        if(*(src + i) >= '0' && *(src + i) <= '9')
            a = a*16 + (src[i]-'0');
        else if(*(src + i) >= 'a' && *(src + i) <= 'f')
            a = a*16 + (src[i]-'a')+10;
        else if(*(src + i) >= 'A' && *(src + i) <= 'F')
            a = a*16 + (src[i]-'A')+10;
        else if(*src + i == 'x' || *src + i == 'X');
            
        ++i;
      }
   return a;
   }
}

unsigned char ArrayCompare(unsigned char* arr1, unsigned char* arr2, unsigned int len)
{
    while(len)
    {
        if(*arr1 != *arr2)   return(0);
        else 
        {
            arr1++;
            arr2++;
            len--;
        }
    }
    return(1);
}

unsigned char CardPowerOn(unsigned char slot)
{
    unsigned char retry;
    long retval;
    int i;

    retry = 0;
    do
    {
        ioctl(fd, IFD_CMD(PowerOff, slot), &UsrParam);
        usleep(50000);
        UsrParam.oDataLen = 32;
        if((retval = ioctl(fd, IFD_CMD(PowerOn, slot), &UsrParam)) >= 0)
        {
            break;
        }
        retry++;
    }while(retry < 3);

    if(retry >= 3)
    {
        printf("\nOperation fail with errorcode = %lX\n", retval);
        return(0);
    }
    else
    {
        printf("\n\nATR Len: %d\n", UsrParam.oDataLen);
        printf("slot%d ATR:", slot);
        for(i = 0; i < UsrParam.oDataLen; i++ )
        {
            printf(" %02X", RecBuf[i]);
        }
        printf("\n\n");
        return(1);
    }
}

void CardOPeration(int choice1, unsigned char slot)
{
	long retval;
	FILE * txtfd;
	char fbuf[1024];
	char *pBuf;
	unsigned int i;
	unsigned int TempLen;
	int choice2;
	unsigned char tmpData;
	unsigned char n;
	char *preCmd = NULL;
	char *curPro;

	UsrParam.p_oBuf = RecBuf;
	UsrParam.p_iBuf = CmdBuf;


	switch(choice1)
	{
		case '1':
		{
			CardPowerOn(slot);
			break;            
		}

		case '2':
		{
 			ioctl(fd, IFD_CMD(PowerOff, slot), &UsrParam);
			break;
 		}

		case '3':
		{
			do
			{
				printf("Exchange Apdu with slot%d:\n",slot);
				printf("==============================================\n\n");
				printf("1: ACOS3 T0 Test\n");
				printf("2: ACOS5 Test\n");
				printf("3: ACOS6 Test1\n");
//				printf("4: ACOS6 Test2\n");
				printf("6: JCOP30 T1 Test\n");
				printf("\n");
				printf("0: Exit the test program\n\n");
				printf("==============================================\n\n");

				choice2 = getc(stdin);
				if(choice2 == 10)
				{
					choice2 = getc(stdin);
				}

				if(choice2 == '1')
				{
					curPro = "acos3 test";
					txtfd = fopen("acos3Test.txt","r");
					if(txtfd == NULL)
					{
						printf("fail to open the file: ACOS3T0Test.txt");
					}
				}
				else if(choice2 == '2')
				{
					curPro = "acos5 test";
					txtfd = fopen("acos5Test.txt","r");
					if(txtfd == NULL)
					{
						printf("fail to open the file: ACOS5Test.txt");
					}
				}
				else if(choice2 == '3')
				{
					curPro = "acos6 test1";
					txtfd = fopen("acos6Test1.txt","r");
 					if(txtfd == NULL)
					{
						printf("fail to open the file: ACOS6Test1.txt");
					}
				}
				#if 0
				else if(choice2 == '4')
				{
					curPro = "acos6 test2";
					txtfd = fopen("acos6Test2.txt","r");
					if(txtfd == NULL)
					{
						printf("fail to open the file: ACOS6Test2.txt");
					}
				}
				#endif
				else if(choice2 == '6')
				{
					curPro = "jcop30 test";
					txtfd = fopen("jcop30Test.txt","r");
					if(txtfd == NULL)
					{
						printf("fail to open the file: JCOP30.txt");
					}
				}

				if(txtfd != NULL)
				{
			
					while( fgets(fbuf, 1024, txtfd) != NULL)
					{
						if(*fbuf == ';')
						{
							printf("%s",fbuf);
						}
						else if(*fbuf == '\n');
						else if(strstr(fbuf, ".RESET") != NULL)
						{
							CardPowerOn(slot);
						}
						else if(strstr(fbuf, "Command:") != NULL)
						{
							preCmd = "Command:";
							pBuf = fbuf + 9;
							i = 0;
							while(*pBuf != '\n' && *pBuf != ' ')
							{
								CmdBuf[i] = StrToHex(pBuf, 2);
								pBuf += 3;
								i++;
							}
         
							UsrParam.p_iBuf = CmdBuf;
							UsrParam.iDataLen = i;
							UsrParam.oDataLen = 271;
	
							PrtMsg("%s",fbuf);


							if((retval = ioctl(fd, IFD_CMD(XfrAPDU, slot), &UsrParam)) < 0)
							{
								PrtMsg("\nOperation fail with errorcode = %lX\n", retval);
								break;
							}
							else
							{
								PrtMsg("Response:");
								for(i = 0; i < UsrParam.oDataLen; i++ )
								{
									PrtMsg(" %02X", RecBuf[i]);
								}
								PrtMsg("\n\n");

							}
						}
						else if(strstr(fbuf, "Response:") != NULL)
						{
							if(strcmp(preCmd, "Command:") != 0)
								goto err;
			    
							preCmd = "Response:";
			    
						    pBuf = fbuf + 10;
							i = 0;
							n = 0;

							while((*pBuf != '\n') && (*pBuf != ' ') && (*pBuf != '\0') && (*pBuf != 0))
							{
								if(*pBuf == 'x' || *pBuf == 'X')
								{
								#if 0
									pBuf++;
									n++;
									if(n == 2)
									{
										n = 0;
										pBuf++;
										i++;
									}
								#endif
								}
								else
								{
									tmpData = StrToHex(pBuf, 2);
									if(RecBuf[i] != tmpData)
									{ 
										printf("faild at index = %d, card data = %02X, expect data = %02X\n\n", i, RecBuf[i], tmpData);
										goto exchangefail;
									}
								}
								pBuf += 3;
								i++;
							}
							TempLen = i;
			
							if(TempLen != UsrParam.oDataLen)
							{
exchangefail:                    	
								PrtMsg("expect %s",fbuf);
								PrtMsg("\n");
								goto err;
							}
                           
						}
					}
					printf("^_^ %s sucessfully ^_^\n\n", curPro);
				}
				else
				{
err:
					printf("@_@ %s failed @_@\n\n", curPro);
				}
            
			}while(choice2 != '0');
		}

		break;
        	
		default:
			printf("Invalid selection: %d\n",choice1);
	}
}

int main()
{

    int choice;
    int choice1;
    unsigned char slot;



    fd=open("/dev/CONTACT_CARD",O_RDWR);
    if(-1 == fd)
    {
        perror("error open\n");
        return(-1);
    }

    do
    {
        printf("\nifd_ICC test program:\n");
        printf("==============================================\n\n");
        printf("1: Operation for ICC\n");
        printf("2: Operation for SAM1\n");
        printf("3: Operation for SAM2\n");
        printf("4: Operation for SAM3\n");
        printf("\n");
        printf("0: Exit the test program\n\n");
        printf("==============================================\n\n");

        choice = getc(stdin);
        if(choice == 10)   choice = getc(stdin);
            switch(choice)
            {
                case '0': break;
                case '1':
                {
                    do
                    {
                        printf("Operation for ICC:\n");
                        printf("==============================================\n\n");
                        printf("1: Power on ICC\n");
                        printf("2: Power off ICC\n");
                        printf("3: Exchange Apdu with ICC\n");
                        printf("\n");
                        printf("0: Exit the test program\n\n");
                        printf("==============================================\n\n");

                        choice1 = getc(stdin);
                        if(choice1 == 10)   choice1 = getc(stdin);
                        if((choice1 >= '1') && (choice1 <= '4'))
                        {
                            slot = choice - '1';
                            CardOPeration(choice1, slot);
                        }
                    }while(choice1 != '0');
                    break;
                }
 
                case '2':
                {
                    do
                    {
                        printf("Operation for SAM1:\n");
                        printf("==============================================\n\n");
                        printf("1: Power on SAM1\n");
                        printf("2: Power off SAM1\n");
                        printf("3: Exchange Apdu with SAM1\n");
                        printf("\n");
                        printf("0: Exit the test program\n\n");
                        printf("==============================================\n\n");

                        choice1 = getc(stdin);
                        if(choice1 == 10)   choice1 = getc(stdin);
                        if((choice1 >= '1') && (choice1 <= '4'))
                        {
                            slot = choice - '1';
                            CardOPeration(choice1, slot);
                        }
                    }while(choice1 != '0');
                    break;
                }

                case '3':
                {
                    do
                    {
                        printf("Operation for SAM2:\n");
                        printf("==============================================\n\n");
                        printf("1: Power on SAM2\n");
                        printf("2: Power off SAM2\n");
                        printf("3: Exchange Apdu with SAM2\n");
                        printf("\n");
                        printf("0: Exit the test program\n\n");
                        printf("==============================================\n\n");

                        choice1 = getc(stdin);
                        if(choice1 == 10)   choice1 = getc(stdin);
                        if((choice1 >= '1') && (choice1 <= '4'))
                        {
                            slot = choice - '1';
                            CardOPeration(choice1, slot);
                        }
                    }while(choice1 != '0');
                    break;
                }

                case '4':
                {
                    do
                    {
                        printf("Operation for SAM3:\n");
                        printf("==============================================\n\n");
                        printf("1: Power on SAM3\n");
                        printf("2: Power off SAM3\n");
                        printf("3: Exchange Apdu with SAM3\n");
                        printf("\n");
                        printf("0: Exit the test program\n\n");
                        printf("==============================================\n\n");

                        choice1 = getc(stdin);
                        if(choice1 == 10)   choice1 = getc(stdin);
                        if((choice1 >= '1') && (choice1 <= '4'))
                        {
                            slot = choice - '1';
                            CardOPeration(choice1, slot);
                        }
                    }while(choice1 != '0');
                    break;
                }
                default:
                    printf("\nInvalid Selection: %d\n", choice);
                    break;
            }
        

    }while(choice != '0');

    close(fd);;
    return(0);
}
