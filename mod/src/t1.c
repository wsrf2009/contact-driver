

#define T1_HEADER                     0x03               // Apdu header length
#define T1_NAD                        0x00
#define T1_PCB                        0x01
#define T1_LEN                        0x02


#define IFDBlockNumber                0
#define ICCBlockNumber                1
#define IFDChainingBit                2
#define ICCChainingBit                3
#define FirstAPDUBit                  7


struct t1_header
{  
    unsigned char NAD;
    unsigned char PCB;
    unsigned char LEN;
};


struct t1_header IFDHeader;
struct t1_header ICCHeader;
u8 T1_TempBlock[11];


static int icc_t1_xfr_tpdu(struct icc_info *icc, u8 *cmd_buf, u32 cmd_len, 
										u8 *res_buf, u32 *res_len)
{
    u32 i;
    u32 length = cmd_len;
    u8* c_buf = cmd_buf;
    u8* r_buf = res_buf;
    int ret;
    bool parity_error_flag = false;


//    TRACE_TO("enter %s\n", __func__);

    for(i = 0; i < 20; i++)
    {
        udelay(icc->cur_ETU);
    }

    ifd_send_bytes(icc, c_buf, length);

    for(i = 0, length = 3; i < length; i++)
    {
        ret = ifd_receive_bytes(icc, r_buf++, 1);
        if(ret)
        {
            if(ret != -ICC_ERRORCODE_XFR_PARITY_ERROR)
            {
                *res_len = i;
                goto	done;
            }
            else
            {
                parity_error_flag = true;
            }
        }
        if(i == 0)
        {
            
        }
        else if(i == 2)
        {
            length = res_buf[2] + icc->check_sum + 3;
        }
    }
	
    *res_len = i;
    if( parity_error_flag == true)
    {
        ret = -ICC_ERRORCODE_XFR_PARITY_ERROR;
    }

done:

//	TRACE_TO("exit %s\n", __func__);
    return(ret);
}

static int icc_t1_tpdu_exchange_error_check(struct icc_info *icc, u8 *ifd_buf, u8 *card_buf)
{
    u8* c_buf = ifd_buf - 3;
    u8* r_buf = card_buf - 3;
    u8 temp_send_block[3];
    u8 temp_recv_block[3];
    u32 i;
    u8 check_sum = 0;
    u8 cur_save_byte = 0;
    int ret;
    u32 temp_len=0;

 
//    TRACE_TO("enter %s\n", __func__);

    memcpy(temp_send_block, c_buf, 3);
    memcpy(temp_recv_block, r_buf, 3);

    if((IFDHeader.PCB & 0x80) == 0x00)        // bit7: 0, I-block
    {
        // bit6 in PCB: send-sequence number
        if(BITisSET(icc->T1_exchange_flag, IFDBlockNumber))    // IFD block number
        {
            IFDHeader.PCB |= 0x40;
        }
        else
        {
            IFDHeader.PCB &= 0xBF;
        }
        icc->T1_exchange_flag ^= 0x01;    // 
    }
    else if((IFDHeader.PCB & 0xC0) == 0x80)   // bit7 = 1, bit6 = 0: R-block
    {
        if(BITisSET(icc->T1_exchange_flag, ICCBlockNumber))    // ICC block number
        {
            IFDHeader.PCB |= 0x10;
        }
        else
        {
            IFDHeader.PCB &= 0xEF;
        }
    }

    c_buf[T1_NAD] = IFDHeader.NAD;
    c_buf[T1_PCB] = IFDHeader.PCB;
    c_buf[T1_LEN] = IFDHeader.LEN;

    for(i = 0; i < ((u32)c_buf[2] + 3); i++)
    {
        check_sum ^= c_buf[i];
    }
    if(BITisSET(icc->T1_exchange_flag, IFDChainingBit))    // IFD chaining bit
    {
        cur_save_byte = c_buf[i];
    }
    c_buf[i++] = check_sum;
    ret = icc_t1_xfr_tpdu(icc, c_buf, i, r_buf, &temp_len);
    if(!ret)
    {
        ICCHeader.NAD = r_buf[T1_NAD];
        ICCHeader.PCB = r_buf[T1_PCB];
        ICCHeader.LEN = r_buf[T1_LEN];
        if(ICCHeader.NAD == 0x00)
        {
            if((ICCHeader.LEN != (temp_len - 4)) || (ICCHeader.LEN == 0xFF))
            {
                if(icc->first_Iblock == 1)
                {
                    if((ICCHeader.PCB & 0x80) == 0x00)    // I-block
                    {
                        ret = -ICC_ERRORCODE_ICC_MUTE;
                    }
                    else
                    {
                        ret = -ICC_ERRORCODE_T1_OTHER_ERROR;
                    }
                }
                else
                {
                    ret = -ICC_ERRORCODE_ICC_MUTE;
                }
            }
            else
            {
                check_sum = 0x00;
                for(i = 0; i < temp_len; i++)
                {
                    check_sum ^= r_buf[i];
                }
                if(check_sum)
                {
                    ret = -ICC_ERRORCODE_T1_CHECKSUM_ERROR;
                }
            }
        }
        else
        {
            ret = -ICC_ERRORCODE_NAD_VALUE_INVALID_OR_NOT_SUPPORTED;
        }
    }
    if(!ret)
    {
        if((ICCHeader.PCB & 0xC0) == 0xC0)
        {
            if(ICCHeader.PCB == 0xC3)       // a WTX request
            {
                if(ICCHeader.LEN != 0x01)
                {
                    ret = -ICC_ERRORCODE_BAD_LENGTGH;
                }
            }   
            else if(ICCHeader.PCB == 0xC1)    // a IFS request
            {
                if(ICCHeader.LEN != 1)
                {
                    ret = -ICC_ERRORCODE_BAD_LENGTGH;
                }
                else if((r_buf[3] < 0x10) || (r_buf[3] > 0xFE))
                {
                    ret = -ICC_ERRORCODE_IFSC_SIZE_INVALID_OR_NOT_SUPPORTED;
                }
            }
            else if((ICCHeader.PCB == 0xC0) || (ICCHeader.PCB == 0xC2)) // a RESYNCH request or an ABORT request
            {
                if(ICCHeader.LEN != 0)
                {
                    ret = -ICC_ERRORCODE_T1_OTHER_ERROR;
                }
            }
            else if(ICCHeader.PCB == 0xE3)    // a WTX response
            {
                if(ICCHeader.LEN != 1)
                {
                    ret = -ICC_ERRORCODE_BAD_LENGTGH;
                }
                else if((IFDHeader.PCB & 0xC0) != 0xC0)
                {
                    ret = -ICC_ERRORCODE_INVALID_BLOCK;
                }
            }
            else if(ICCHeader.PCB == 0xE1)    // a IFS response
            {
                if(ICCHeader.LEN != 1)
                {
                    ret = -ICC_ERRORCODE_BAD_LENGTGH;
                }
                else if((IFDHeader.PCB & 0xC0) != 0xC0)
                {
                    ret = -ICC_ERRORCODE_INVALID_BLOCK;
                }
            }
            else if((ICCHeader.PCB == 0xE0) || (ICCHeader.PCB == 0xE2))    // a RESYNCH response or an ABORT response
            {
                if(ICCHeader.LEN != 0)
                {
                    ret = -ICC_ERRORCODE_BAD_LENGTGH;
                }
                else if((IFDHeader.PCB & 0xC0) != 0xC0)
                {
                    ret = -ICC_ERRORCODE_INVALID_BLOCK;
                }
            }
            else
            {
                ret = -ICC_ERRORCODE_INVALID_BLOCK;
            }
        }
        else if((ICCHeader.PCB & 0xC0) == 0x80)
        {
            if(ICCHeader.LEN != 0)
            {
                ret = -ICC_ERRORCODE_BAD_LENGTGH;
            }
            else if((ICCHeader.PCB & 0x2C) != 0)
            {
                ret = -ICC_ERRORCODE_INVALID_BLOCK;
            }
        }
    }
    if(BITisSET(icc->T1_exchange_flag, IFDChainingBit))    // IFD chaining bit
    {
        c_buf[i-1] = cur_save_byte;
    }
    if((ret) || ((ICCHeader.PCB & 0x80) != 0x00) || (c_buf < r_buf) || (c_buf > (card_buf + ICCHeader.LEN)))
    {
        memcpy(c_buf, temp_send_block, 3);
    }
	
    memcpy(r_buf, temp_recv_block, 3);

//	TRACE_TO("exit %s", __func__);
	
    return(ret);
}

static int icc_t1_block_transceive_error_handler(struct icc_info *icc, u8 *cmd_buf, u8 *res_buf)
{
	int ret;
    u8 prolog_buf[3];
    u8 block_head_buf[5];
    u8 num_retry = 0;
    u8 num_s_req = 0;
    u8 num_resend = 0;
    u8 last_block_error = 0;


//    TRACE_TO("enter %s\n", __func__);

    prolog_buf[0] = IFDHeader.NAD;
    prolog_buf[1] = IFDHeader.PCB;
    prolog_buf[2] = IFDHeader.LEN;
	
    memcpy(block_head_buf, cmd_buf, 5);
	
    do
    {
        ret = icc_t1_tpdu_exchange_error_check(icc, cmd_buf, res_buf);
        do
        {
            
            if(!ret)
            {
                if((ICCHeader.PCB & 0xC0) == 0xC0)    // S-Block received
                {
                    if(ICCHeader.PCB == 0xC1)
                    {
                        icc->IFSC = *res_buf;
                    }
                    else if(ICCHeader.PCB == 0xC2)
                    {
                        ret = -ICC_ERRORCODE_CMD_ABORTED;
						goto err;
                    }
                    else if((ICCHeader.PCB != 0xC0) && (ICCHeader.PCB != 0xC3))
                    {
                        ret = -ICC_ERRORCODE_T1_OTHER_ERROR;
                        break;
                    }
                    IFDHeader.PCB = ICCHeader.PCB | 0x20;
                    IFDHeader.LEN = ICCHeader.LEN;
                    *cmd_buf = *res_buf;
                    num_s_req++;
                    if(num_s_req >= 5)
                    {
                        ret = -ICC_ERRORCODE_CMD_ABORTED;
						goto err;
                    }
                }
                else if((ICCHeader.PCB & 0xC0) == 0x80)  // R-Block received
                {
                    if((BITisSET(icc->T1_exchange_flag, IFDBlockNumber) && (ICCHeader.PCB & 0x10)) ||  // IFDBlockNum && R-block with a error
                     (BITisCLEAR(icc->T1_exchange_flag, IFDBlockNumber) && ((ICCHeader.PCB & 0x10) == 0x00)))
                    {
                        if(BITisSET(icc->T1_exchange_flag, ICCChainingBit))    // ICC chaining bit
                        {
                            num_resend++;             // resend last block
                            if(num_resend >= 3)
                            {
                                num_retry = 3;
                            }
                            break;
                        }
                        if(BITisSET(icc->T1_exchange_flag, IFDChainingBit))    // IFD chaining bit
                        {
                            num_retry |= 0x80;
                        }
                        else
                        {
                            ret = -ICC_ERRORCODE_T1_OTHER_ERROR;
                        }
                    }
                    else
                    {
                        IFDHeader.NAD = prolog_buf[0];
                        IFDHeader.PCB = prolog_buf[1];
                        IFDHeader.LEN = prolog_buf[2];
                        memcpy(cmd_buf, block_head_buf, 5);
                        num_resend++;
                        if(num_resend >= 3)
                        {
                            num_resend = 3;
                        }
                        else
                        {
                            if((IFDHeader.PCB & 0x80) == 0x00)
                            {
                                icc->T1_exchange_flag ^= 0x01;    // IFD block number
                                icc->first_Iblock = 0;
                            }
                        }
                    }
                }
                else                                                    // I-Block received
                {
                    if(BITisSET(icc->T1_exchange_flag, IFDChainingBit))       // IFD chaining bit
                    {
                        ret = -ICC_ERRORCODE_T1_OTHER_ERROR;
                        break;
                    }
                    if((BITisSET(icc->T1_exchange_flag,ICCBlockNumber) && (ICCHeader.PCB & 0x40)) ||   // ICCBlockNum && 
                    	((BITisCLEAR(icc->T1_exchange_flag,ICCBlockNumber) && ((ICCHeader.PCB & 0x40) == 0x00))))
                    {
                        if(ICCHeader.PCB & 0x20)
                        {
                            SET_BIT(icc->T1_exchange_flag, ICCChainingBit);    // set ICC chaining bit
                        }
                        else
                        {
                            CLEAR_BIT(icc->T1_exchange_flag, ICCChainingBit);    // reset ICC chaining bit
                        }
                        num_retry |= 0x80;
                        icc->T1_exchange_flag ^= 0x02;
                    }
                    else
                    {
                        ret = -ICC_ERRORCODE_INVALID_BLOCK;
                    }
                }
            }
        }while(0);

	if(ret == -ICC_ERRORCODE_T1_CHECKSUM_ERROR)
        {
            ret = -ICC_ERRORCODE_XFR_PARITY_ERROR;
        }
        if(ret)
        {
            if(ret != last_block_error)
            {
                last_block_error = ret;
            }
            if(ret != -ICC_ERRORCODE_XFR_PARITY_ERROR)
            {
                ret = -ICC_ERRORCODE_T1_OTHER_ERROR;
            }
        }
        else
        {
            last_block_error = 0;
        }

        if(ret)
        {
            if((last_block_error == -ICC_ERRORCODE_XFR_PARITY_ERROR) 
				|| (last_block_error == -ICC_ERRORCODE_T1_OTHER_ERROR))
            {
                ret = last_block_error;
            }
            else
            {
                last_block_error = ret;
            }
        }
        else
        {
            last_block_error = 0;
            break;
        }
        if((ret == -ICC_ERRORCODE_T1_OTHER_ERROR) || (ret == -ICC_ERRORCODE_XFR_PARITY_ERROR))
        {
            if(ret == -ICC_ERRORCODE_XFR_PARITY_ERROR)
            {
                IFDHeader.PCB = 0x81;
            }
            else
            {
                IFDHeader.PCB = 0x82;
            }
            if(BITisSET(icc->T1_exchange_flag, ICCChainingBit))    // ICC chaining bit
            {
                IFDHeader.PCB = 0x80;
            }
            IFDHeader.LEN = 0x00;
            num_retry++;
        }

    }while(num_retry < 3);

    icc->first_Iblock = 0;
    if(num_retry == 3)
    {
        num_retry = 0;
        while(num_retry < 3)
        {
            IFDHeader.NAD = 0x00;
            IFDHeader.PCB = 0xC0;
            IFDHeader.LEN = 0x00;
            ret = icc_t1_tpdu_exchange_error_check(icc, cmd_buf, res_buf);
            if(!ret)
            {
                if(ICCHeader.PCB == 0xE0)
                {
                    CLEAR_BIT(icc->T1_exchange_flag, IFDBlockNumber);
					CLEAR_BIT(icc->T1_exchange_flag, ICCBlockNumber);
                    ret = -ICC_ERRORCODE_T1_RETRY_FAIL;
                    break;
                }
                else
                {
                    num_retry++;
                }
            }
            else
            {
                num_retry++;
            }
        }

        if(num_retry == 3)
        {
            ret = -ICC_ERRORCODE_T1_RETRY_FAIL;
        }
    }

err:

//	TRACE_TO("exit %s", __func__);
	
    return(ret);
}

static int icc_t1_dispatch(struct icc_info *icc, u8 *cmd_buf, u32 cmd_len, u8 *res_buf, u32 *res_len)
{
    u8 num_retry = 0;
    int ret = 0;
    u8 *c_buf;
    u8 *r_buf;
    u32 send_apdu_len;
    u8 temp_ifsc;
    u8 ifd_change_temp_block;

    
//    TRACE_TO("enter %s\n", __func__);

    if(BITisSET(icc->T1_exchange_flag, FirstAPDUBit))    // first APDU ?
    {
        icc->first_Iblock = 0;
        icc->T1_exchange_flag = 0;
        while(num_retry < 3)
        {
            IFDHeader.NAD = 0x00;    // the addressing is not used
            IFDHeader.PCB = 0xC1;
            IFDHeader.LEN = 0x01;
            T1_TempBlock[3] = 0xFE;
            ret = icc_t1_tpdu_exchange_error_check(icc, T1_TempBlock + 3, T1_TempBlock+3);
            if(!ret)
            {
                if((ICCHeader.PCB != 0xE1) || (T1_TempBlock[3] != 0xFE))
                {
                    ret = -ICC_ERRORCODE_T1_OTHER_ERROR;
                }
            }
            if(ret)
            {
                num_retry++;
            }
            else
            {
                break;
            }
        }
        if(num_retry == 3)
        {
            ret = -ICC_ERRORCODE_T1_RETRY_FAIL;
        }
        udelay(icc->cur_ETU);
        icc->first_Iblock = 1;
    }

APDUExchangeBegin:
    c_buf = cmd_buf;
    r_buf = res_buf;
    send_apdu_len = cmd_len;
    memcpy(T1_TempBlock, c_buf, 10);
    temp_ifsc = icc->IFSC;
    while(send_apdu_len)
    {
        CLEAR_BIT(icc->T1_exchange_flag, ICCChainingBit);    // reset ICC chaining bit
        if(send_apdu_len <= temp_ifsc)
        { 
            CLEAR_BIT(icc->T1_exchange_flag, IFDChainingBit);    // reset IFD chaining bit
            IFDHeader.NAD = 0x00;
            IFDHeader.PCB = 0x00;
            IFDHeader.LEN = (u8)send_apdu_len;
            ret = icc_t1_block_transceive_error_handler(icc, c_buf, r_buf);
            icc->first_Iblock = 0;
            send_apdu_len = 0;
            if(ret == -ICC_ERRORCODE_T1_RETRY_FAIL)
            {
                num_retry++;
                if(num_retry < 3)
                {
                    memcpy(cmd_buf, T1_TempBlock, 10);
                    c_buf = cmd_buf;
                    r_buf = res_buf;
                    send_apdu_len = cmd_len;
                }
            }
            else if(!ret)
            {
                *res_len = ICCHeader.LEN;
            }
            else
            {
                CLEAR_BIT(icc->T1_exchange_flag, ICCChainingBit);    // reset ICC chaining bit;
            }
        }
        else
        {
            SET_BIT(icc->T1_exchange_flag, IFDChainingBit);    // set IFD chaining bit
            IFDHeader.NAD = 0x00;
            IFDHeader.PCB = 0x20;
            IFDHeader.LEN = temp_ifsc;
            ifd_change_temp_block = c_buf[temp_ifsc];
            ret = icc_t1_block_transceive_error_handler(icc, c_buf, r_buf);
            c_buf[temp_ifsc] = ifd_change_temp_block;
            icc->first_Iblock = 0;
            CLEAR_BIT(icc->T1_exchange_flag, IFDChainingBit);    // reset IFD chaining bit
            if(ret == -ICC_ERRORCODE_T1_RETRY_FAIL)
            {
                num_retry++;
                if(num_retry < 3)
                {
                    c_buf = cmd_buf;
                    r_buf = res_buf;
                    memcpy(cmd_buf, T1_TempBlock, 10);
                    send_apdu_len = cmd_len;
                }
            }
            else if(!ret)
            {
                send_apdu_len -= temp_ifsc;
                c_buf += temp_ifsc;
            }
            else
            {
                send_apdu_len = 0;
                CLEAR_BIT(icc->T1_exchange_flag, ICCChainingBit);    // reset ICC chaining bit
            }
        }
    }
    while(icc->T1_exchange_flag & 0x08)
    {
        IFDHeader.NAD = 0x00;
        IFDHeader.PCB = 0x80;
        IFDHeader.LEN = 0x00;
        r_buf += ICCHeader.LEN;
        ret = icc_t1_block_transceive_error_handler(icc, r_buf, r_buf);
        if(ret == -ICC_ERRORCODE_T1_RETRY_FAIL)
        {
            num_retry++;
            if(num_retry < 3)
            {
                memcpy(cmd_buf, T1_TempBlock, 10);
                goto APDUExchangeBegin;
            }
            icc->T1_exchange_flag &= ~0x08;    // reset ICC chaining bit
        }
        else if(!ret)
        {
            *res_len += ICCHeader.LEN;
        }
        else
        {
            icc->T1_exchange_flag &= ~0x08;    // reset ICC chaining bit
        }
    }

//	TRACE_TO("exit %s", __func__);
	
    return(ret);
}

static int icc_t1_parse(struct icc_info *icc, u8 *cmd_buf, u32 cmd_len, u8 *res_buf, u32 *res_len)
{
    int ret = 0;


    ret = icc_t1_dispatch(icc, cmd_buf, cmd_len, res_buf, res_len);
	
    return(ret);
}

