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
            printf(" 0x%X", RecBuf[i]);
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
    int i;
    int TempLen;
    int choice2;

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
                printf("3: ACOS6 Test\n");
//                printf("4: ACOS7 Test\n");
//                printf("5: ACOS9 Test\n");
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
                    txtfd = fopen("ACOS3T0Test.txt","r");
                    if(txtfd == NULL)
                    {
                        printf("fail to open the file: ACOS3T0Test.txt");
                    }
                }
                else if(choice2 == '2')
                {
                    txtfd = fopen("ACOS5Test.txt","r");
                    if(txtfd == NULL)
                    {
                        printf("fail to open the file: ACOS5Test.txt");
                    }
                }
                else if(choice2 == '3')
                {
                    txtfd = fopen("ACOS6Test.txt","r");
                    if(txtfd == NULL)
                    {
                        printf("fail to open the file: ACOS6Test.txt");
                    }
                }
/*                else if(choice2 == '4')
                {
                    txtfd = fopen("ACOS7Test.txt","r");
                    if(txtfd == NULL)
                    {
                        printf("fail to open the file: ACOS7Test.txt");
                    }
                }
                else if(choice2 == '5')
                {
                    txtfd = fopen("ACOS9Test.txt","r");
                    if(txtfd == NULL)
                    {
                        printf("fail to open the file: ACOS9Test.txt");
                    }
                }
*/                else if(choice2 == '6')
                {
                    txtfd = fopen("JCOP30.txt","r");
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

                        PrtMsg("\n\nCMD Len: %d\n", UsrParam.iDataLen);
                        PrtMsg("slot%d CMD:", slot);
                        for(i = 0; i < UsrParam.iDataLen; i++ )
                        {
                            PrtMsg("0x%X ", UsrParam.p_iBuf[i]);
                        }
                        PrtMsg("\n\n");

                        if((retval = ioctl(fd, IFD_CMD(XfrAPDU, slot), &UsrParam)) < 0)
                        {
                            PrtMsg("\nOperation fail with errorcode = %lX\n", retval);
                            break;
                        }
                        else
                        {

                            PrtMsg("Response Length: %d\n", UsrParam.oDataLen);
                            PrtMsg("Response Data:");
                            for(i = 0; i < UsrParam.oDataLen; i++ )
                            {
                                PrtMsg("0x%X ", RecBuf[i]);
                            }
                            PrtMsg("\n\n");

                            fgets(fbuf, 1024, txtfd);
                            if(strstr(fbuf, "Response:") != NULL)
                            { 
                                pBuf = fbuf + 10;
                                i = 0;
                                while(*pBuf != '\n' && *pBuf != ' ')
                                {
                                    if(*pBuf == 'x' || *pBuf == 'X');
                                    else
                                    {
                                        if(RecBuf[i] != StrToHex(pBuf, 2))
                                        { 
                                            goto exchangefail;
                                        }
                                    }
                                    pBuf += 3;
                                    i++;
                                }
                                    TempLen = i;

                                if(TempLen == UsrParam.oDataLen)
                                {
                                    PrtMsg("Success to Exchange with the slot%d!\n", slot);
                                    PrtMsg("\n\n");
                                }
                                    else
                                {
exchangefail:
                                    PrtMsg("Fail to Exchange with the slot%d!\n", slot);
                                    PrtMsg("The expected data:\n");
                                    PrtMsg("%s",fbuf);
                                    PrtMsg("\n\n");
                                    break;
                                }
                            }
                        }
                    }
                }
            }
            
        }while(choice2 != '0');
#if 0
            txtfd = fopen("ACOS3T0Test.txt","r");
            if(txtfd == NULL)
            {
                printf("fail to open the file: JCOP30.txt");
            }
            else
            {

                while( fgets(fbuf, 1024, txtfd) != NULL)
                {

                    while(*fbuf == 0x0D && *(fbuf + 1) == 0x0A) fgets(fbuf, 1024, txtfd);
                    pBuf = fbuf;
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

                    PrtMsg("\n\nCMD Len: %d\n", UsrParam.iDataLen);
                    PrtMsg("slot%d CMD:", slot);
                    for(i = 0; i < UsrParam.iDataLen; i++ )
                    {
                        PrtMsg("0x%X ", UsrParam.p_iBuf[i]);
                    }
                    PrtMsg("\n\n");

                    if((retval = ioctl(fd, IFD_CMD(XfrAPDU, slot), &UsrParam)) < 0)
                    {
                        PrtMsg("\nOperation fail with errorcode = %lX\n", retval);
                        break;
                    }
                    else
                    {

                        PrtMsg("Response Length: %d\n", UsrParam.oDataLen);
                        PrtMsg("Response Data:");
                        for(i = 0; i < UsrParam.oDataLen; i++ )
                        {
                            PrtMsg("0x%X ", RecBuf[i]);
                        }
                        PrtMsg("\n\n");

                        PrtMsg("Success to Exchange with the slot%d!\n", slot);
                        PrtMsg("\n\n");

                    }
                }
            }
//#else
            txtfd = fopen("JCOP30.txt","r");
            if(txtfd == NULL)
            {
                printf("fail to open the file: JCOP30.txt");
            }
            else
            {

                while( fgets(fbuf, 1024, txtfd) != NULL)
                {
                    while(strstr(fbuf, "Command:") == NULL) fgets(fbuf, 1024, txtfd);

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

                    PrtMsg("\n\nCMD Len: %d\n", UsrParam.iDataLen);
                    PrtMsg("slot%d CMD:", slot);
                    for(i = 0; i < UsrParam.iDataLen; i++ )
                    {
                        PrtMsg("0x%X ", UsrParam.p_iBuf[i]);
                    }
                    PrtMsg("\n\n");

                    if((retval = ioctl(fd, IFD_CMD(XfrAPDU, slot), &UsrParam)) < 0)
                    {
                        PrtMsg("\nOperation fail with errorcode = %lX\n", retval);
                        break;
                    }
                    else
                    {

                        PrtMsg("Response Length: %d\n", UsrParam.oDataLen);
                        PrtMsg("Response Data:");
                        for(i = 0; i < UsrParam.oDataLen; i++ )
                        {
                            PrtMsg("0x%X ", RecBuf[i]);
                        }
                        PrtMsg("\n\n");

                        fgets(fbuf, 1024, txtfd);
                        while(strstr(fbuf, "Response:") == NULL) fgets(fbuf, 1024, txtfd);

                        pBuf = fbuf + 10;
                        i = 0;
                        while(*pBuf != '\n' && *pBuf != ' ')
                        {
                            CmdBuf[i] = StrToHex(pBuf, 2);
                            pBuf += 3;
                            i++;
                        }
                        TempLen = i;

                        if( (TempLen == UsrParam.oDataLen) && ArrayCompare(UsrParam.p_iBuf, UsrParam.p_oBuf, TempLen))
                        {
                            PrtMsg("Success to Exchange with the slot%d!\n", slot);
                            PrtMsg("\n\n");
                        }
                        else
                        {
                            PrtMsg("Fail to Exchange with the slot%d!\n", slot);
                            PrtMsg("The expected data:\n");
                            for(i = 0; i < TempLen; i++ )
                            {
                                PrtMsg("0x%X ", CmdBuf[i]);
                            }
                            PrtMsg("\n\n");
                            break;
                        }

                    }
                }
            }
#endif
            break;
        }
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
