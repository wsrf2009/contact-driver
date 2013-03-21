/*
* Name: IFD Message
* Date: 2012/10/08
* Author: Alex Wang
* Version: 1.0
*/


#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/miscdevice.h>
#include <linux/semaphore.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/uaccess.h>
#include <linux/slab.h> 
#include <linux/err.h>     //IS_ERR()
#include <linux/device.h>
#include <linux/cdev.h>

#include "debug.h"
#include "icc.h"
#include "atr.h"
#include "at83c26.h"

#define  ContactCard_PowerOn     0x01
#define  ContactCard_PowerOff    0x02
#define  ContactCard_XfrAPDU     0x03

typedef struct
{
    unsigned char *p_iBuf;
    unsigned char *p_oBuf;
    unsigned int  iDataLen;
    unsigned int  oDataLen;
}IFD_PARAM;

struct ifd_dev
{
    struct cdev cdev;
};

static struct semaphore IFD_mutex;
unsigned char Pre_CardIdx = 0xFF;
static unsigned char sem_inc=0;

static long IFD_ioctl(struct file *filp, unsigned int cmd, unsigned long arg) 
{
    unsigned char CardIdx = cmd & 0x0F;
    unsigned char IFDCMD = (cmd >> 4) & 0x0F;
    IFD_PARAM KerParam;
    IFD_PARAM *UsrParam = (IFD_PARAM *)arg;
    unsigned char *p_iData;
    unsigned char *p_oData;
    long ret = 0;

    PrtMsg("welcome to the function: %s, CardIdx = %X, IFDCMD = %X\n", __FUNCTION__, CardIdx, IFDCMD);
    if(down_interruptible(&IFD_mutex))    // acquire the semaphore
    {
        ret = (-ERESTARTSYS);
        goto err;
    }

    if((!UsrParam) || (copy_from_user(&KerParam, UsrParam, sizeof(KerParam))))
    {
        ret = (-EFAULT);          // bad address
  	goto err;
    }

    if(Pre_CardIdx > 4)
    {
        AT83C26_CIOx(CardIdx, CIO_CON, CIO_LOW);    // connect IO1 to CardIdx
        Pre_CardIdx = CardIdx;
    }
    else if(CardIdx != Pre_CardIdx)
    {
        AT83C26_CIOx(Pre_CardIdx, CIO_DISCON, CIO_HIGH);    // save previous card status
        AT83C26_CIOx(CardIdx, CIO_CON, CIO_LOW);    // connect IO1 to CardIdx
        Pre_CardIdx = CardIdx;
    }

    switch(IFDCMD)
    {
        case ContactCard_PowerOn:
        {
            if(!KerParam.p_oBuf)
            {
                ret = (-EFAULT);       // bad address
                goto err;
            }
            p_oData = kmalloc(KerParam.oDataLen, GFP_KERNEL);
            if(!p_oData)
            {
                ret = (-EFAULT);       // bad address
                goto err;                
            }
            if(SC_PowerOn(CardIdx, p_oData, &KerParam.oDataLen))
            {
                ret = (-ENXIO);        // device error
                goto err;
            }
            if(copy_to_user(KerParam.p_oBuf, p_oData, KerParam.oDataLen))
            {
                ret = (-EFAULT);       // bad address
                goto err;
            }
            if(copy_to_user(&UsrParam->oDataLen, &KerParam.oDataLen, sizeof(KerParam.oDataLen)))
            {
                ret = (-EFAULT);       // bad address
                goto err;
            }
            break; 
        }

        case ContactCard_PowerOff:
        {
            if(SC_PowerOff(CardIdx))
            {
                ret = (-ENXIO);        // device error
                goto err;
            }
            break;
        }

        case ContactCard_XfrAPDU:
        {
            if((KerParam.iDataLen <= 0) || (KerParam.oDataLen <= 0) || (!KerParam.p_iBuf) || (!KerParam.p_oBuf))
            {
                ret = (-EFAULT);       // bad address
                goto err;
            }
            p_iData = kmalloc(KerParam.iDataLen, GFP_KERNEL);
            p_oData = kmalloc(KerParam.oDataLen, GFP_KERNEL);
            if((!p_iData) || (!p_oData) || (copy_from_user(p_iData, KerParam.p_iBuf, KerParam.iDataLen)))
            {
                ret = (-EFAULT);       // bad address
                goto err;
            }
            if(SC_TransAPDU(CardIdx, p_iData, KerParam.iDataLen, p_oData, &KerParam.oDataLen))
            {
                ret = (-ENXIO);        // device error
                goto err;
            }
            if((KerParam.oDataLen <= 0) || (copy_to_user(KerParam.p_oBuf, p_oData, (unsigned long)KerParam.oDataLen)))
            {
                ret = (-EFAULT);       // bad address
                goto err;
            }
			PrtMsg("%s: KerParam.oDataLen = %d\n",__FUNCTION__, KerParam.oDataLen);
            if(copy_to_user(&UsrParam->oDataLen, &KerParam.oDataLen, sizeof(KerParam.oDataLen)))
            {
                ret = (-EFAULT);       // bad address
                goto err;
            }

            if(p_iData)
            {
                kfree(p_iData);
            }
            if(p_oData)
            {
                kfree(p_oData);
            }
            break;
        }

        default:
            break;
    }

err:
    up(&IFD_mutex);                    // release the semaphore
    return(ret);
}

int IFD_Open(struct inode *inode,struct file *filp)
{
    struct ifd_dev *dev;

    PrtMsg("Welcome to entry to the function: %s\n",__FUNCTION__);
    if(sem_inc > 0)    return(-ERESTARTSYS);
    sem_inc++;

    dev = container_of(inode->i_cdev,struct ifd_dev, cdev);
    filp->private_data = dev;

    return(0);
}
int IFD_release(struct inode *inode,struct file *filp)
{
    sem_inc--;
    return(0);
}

static struct file_operations IFD_fops=
{
    .owner = THIS_MODULE,
    .open = IFD_Open,
    .unlocked_ioctl = IFD_ioctl,
    .release = IFD_release
};

static struct miscdevice IFD_misc=
{
    .minor = 220,
    .name = "CONTACT_CARD",
    .fops = &IFD_fops
};


static int IFD_init(void)
{
    PrtMsg("welcome to install SmartCard driver!\n");
    sema_init(&IFD_mutex, 0);    // initial a semaphore, and lock it
    PrtMsg("success to initial the semaphore 'IFD_MUTEX'\n");
    if(SC_Device_Init())
    {
        up(&IFD_mutex);
        PrtMsg("%s: Fail to initial SC device\n",__FUNCTION__);
        return (-1);
    }
    
    if(misc_register(&IFD_misc))
    {
        up(&IFD_mutex);
        PrtMsg("%s: Fail to register device\n",__FUNCTION__);
        return (-1);
    }

    up(&IFD_mutex);                  // release the semaphore
    PrtMsg("%s: Success to install IFD device\n",__FUNCTION__);
    return (0);
}

static void IFD_exit(void)
{
    if (down_interruptible(&IFD_mutex)) return;
    misc_deregister(&IFD_misc);
    SC_Device_Uninit();
    up(&IFD_mutex);
    PrtMsg("%s: Success to uninstall SC device\n",__FUNCTION__);
    return;
}

module_init(IFD_init);
module_exit(IFD_exit);

MODULE_DESCRIPTION("Contact Card Driver");
MODULE_AUTHOR("Alex Wang");
MODULE_LICENSE("GPL");
