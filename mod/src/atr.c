



int icc_cold_reset(struct icc_info *icc, u8 card_vcc)
{
	int	ret = 0;

//	put_uart_outline_normal();
	ret = tda8026_card_cold_reset(icc->slot, card_vcc);
	uart_clear_fifo();
//	mdelay(10);
	set_waiting_timer(icc, 40000 / (icc->common->freq / 1000000));

//	uart_clear_fifo();

//	mdelay(10);
//	udelay(800);
	
    return ret;

}

int icc_warm_reset(struct icc_info *icc)
{
	int ret;


	ret = tda8026_card_warm_reset(icc->slot, icc->class);

	// ignore the data accroding to 6.2.2 in ISO/IEC 7816-3
	// WaiTingTime = 42100/FRE_CNTACARD = 9.8ms
	mdelay(10);
	udelay(510);
	
    return ret;
}

int icc_get_atr(struct icc_info *icc, u8 *atr_buf, u32 *atr_len)
{
    u8 rec_len;
    u8 atr_buf_idx;
    u8 his_bytes;
    u8 check_character=0;
    bool last_tdi = false;
	int ret;

    
    TRACE_TO("enter %s\n", __func__);

//	u8 temp;
//	u32 len = 19;

//	while(len--)
//	{
//		ifd_receive_bytes(icc, &temp, 1);
//		TRACE_TO(" %02X", temp);
//	}
    // receive TS
    atr_buf_idx = 0;
    if(ifd_receive_bytes(icc, (atr_buf + atr_buf_idx), 1) == -ICC_ERRORCODE_XFR_PARITY_ERROR)    // parity error
    {
        if(atr_buf[0] == 0x03)    // inverse convention
        {
            atr_buf[0] = 0x3F;
            icc->direct = INVERSE_CONVENTION;
        }
        else
        {
            ERROR_TO("TS error[%02X] when get atr\n", atr_buf[0]);
            ret = -ICC_ERRORCODE_BAD_ATR_TS;
			goto	err;
        }
    }
    else if(atr_buf[0] == 0x3B)
    {
        icc->direct = DIRECT_CONVENTION;
    }
    else if(atr_buf[0] == 0x03)
    {
        atr_buf[0] = 0x3F;
        icc->direct = INVERSE_CONVENTION;
    }
    else
    {
    	
		ERROR_TO("TS error[%02X]\n", atr_buf[0]);
        ret = -ICC_ERRORCODE_BAD_ATR_TS;
//		mdelay(500);
		goto	err;
    }
    atr_buf_idx++;

    // receive T0
    ret = ifd_receive_bytes(icc, atr_buf+atr_buf_idx, 1);
    if(ret)		
		goto	err;

    atr_buf_idx++;
    rec_len = 0;
    if(atr_buf[1] & 0x10)    rec_len++;
    if(atr_buf[1] & 0x20)    rec_len++;
    if(atr_buf[1] & 0x40)    rec_len++;
    if(atr_buf[1] & 0x80)    rec_len++;
    his_bytes = atr_buf[1] & 0x0F;

    // receive TA1,TB1,TC1,TD1
    if(rec_len)
    {
    	ret = ifd_receive_bytes(icc, atr_buf+atr_buf_idx, rec_len);
        if(ret)
			goto	err;
    }
    atr_buf_idx += rec_len;

    if(atr_buf[1] & 0x80)
    {
        do
        {
            if(atr_buf[atr_buf_idx -1] & 0x0F)   check_character = 1;

            rec_len = 0;
            if(atr_buf[atr_buf_idx -1] & 0x10)	rec_len++;
            if(atr_buf[atr_buf_idx -1] & 0x20)	rec_len++;
            if(atr_buf[atr_buf_idx -1] & 0x40)	rec_len++;
            if(atr_buf[atr_buf_idx -1] & 0x80)	rec_len++;
            else	last_tdi = true;

            ret = ifd_receive_bytes(icc, atr_buf+atr_buf_idx, rec_len);
            if(ret) 
				goto	err;
			
            atr_buf_idx += rec_len;

            if((rec_len == 0) || (last_tdi == true))
				break;
			
        }while(atr_buf_idx <= MAX_ATR_LEN);
    }

	ret = ifd_receive_bytes(icc, atr_buf+atr_buf_idx, his_bytes+check_character);
    if(ret)
		goto err;
	
    *atr_len = atr_buf_idx + his_bytes + check_character;
    if(*atr_len > MAX_ATR_LEN)
    {
        ERROR_TO("fail to get ATR with Length error\n");
        ret = -ICC_ERRORCODE_BAD_LENGTGH;
    }

    if(check_character)
    {
        unsigned char CheckValue = 0;

        for(atr_buf_idx = 1; atr_buf_idx < *atr_len; atr_buf_idx++)
		{
	    	CheckValue ^= atr_buf[atr_buf_idx];
			TRACE_TO(" %02X", atr_buf[atr_buf_idx]);
		}
		TRACE_TO("\n");
		
        if(CheckValue)
        {
            ERROR_TO("fail to get ATR with TCK error\n");
            ret = -ICC_ERRORCODE_BAD_ATR_TCK;
			goto	err;
        }
    }
    
    ret = 0;

err:
//	TRACE_TO("exit %s\n", __func__);
	return	ret;
}

static u8 get_atr_byte_position(u8 *atr_buf, u32 *atr_len, u8 atr_byte, u8 atr_byte_number)
{
    u8 temp_byte_number = 1;
    u8 temp_byte;
    u8 atr_len_without_his;
    u8 atr_buf_idx = 1;

	
    temp_byte = atr_buf[atr_buf_idx];
    atr_len_without_his = *atr_len - (atr_buf[1] & 0x0F); // Get the ATR length without the historical characters
    while(atr_buf_idx < atr_len_without_his)              // find "atr_byte" in "atr_buf"
    {
        if(temp_byte & 0x10)                          // TAi present
        {
            if(atr_byte != CHARACTER_TA ||  atr_byte_number != temp_byte_number) 
	    	{
	        	atr_buf_idx++;
	    	}
            else
	    	{
	        	return(atr_buf_idx+1);
	    	}
        }
        if(temp_byte & 0x20)                          // TBi present
        {
            if(atr_byte != CHARACTER_TB ||  atr_byte_number != temp_byte_number) 
	    	{
	        	atr_buf_idx++;
	    	}
            else
	    	{
	        	return(atr_buf_idx+1);
	    	}
        }
        if(temp_byte & 0x40)                          // TCi present
        {
            if(atr_byte != CHARACTER_TC ||  atr_byte_number != temp_byte_number) 
	    	{
	        	atr_buf_idx++;
	    	}
            else
	    	{
	        	return(atr_buf_idx+1);
	    	}
        } 
        if(temp_byte & 0x80)                          // TDi present
        {
            if(atr_byte != CHARACTER_TD ||  atr_byte_number != temp_byte_number)
            {
                atr_buf_idx++;
                temp_byte_number++;
                temp_byte = atr_buf[atr_buf_idx];
            }
            else 
	    	{
	        	return(atr_buf_idx+1);
	    	}
        }
        else 
		{
	    	return(0);     
		}
    }
    
    return(0);
}

void icc_analyze_atr(struct icc_info *icc, u8 *atr_buf, u32 *atr_len)
{
    u8 temp_position;
    u8 temp_byte;
    u8 temp_save_td;
    u8 temp_new_position;
    u8 td15_position = 0;
    u8 temp_number = 2;
    u8 temp_ta2 = 0;

    
//    TRACE_TO("enter %s\n", __func__);
	
    temp_position = get_atr_byte_position(atr_buf, atr_len, CHARACTER_TD, 1);
    if(temp_position != 0)                                   // TD1 is present?
    {
        temp_byte = atr_buf[temp_position] & 0x0F;
        icc->T = temp_byte;
        temp_save_td = temp_byte;

        while(1)
        {
            temp_new_position = get_atr_byte_position(atr_buf, atr_len, CHARACTER_TD, temp_number);
            if(temp_new_position == 0)	break;

            temp_byte = atr_buf[temp_new_position];      // TDi 
            if((temp_byte & 0x0F) >= (temp_save_td & 0x0F))    // Normal case the new T value should be higher than the previous one
            {
                temp_number++;
                temp_save_td = temp_byte;
            }
            else
            {
                ERROR_TO("fail to analyze ATR with TD error\n");
                break;
            }
            
            // T=15 is present
            if((temp_byte & 0x0F) == 0x0F)	td15_position = temp_new_position;

            // T=1 is present, but TDi >= TD2
//            if((temp_byte & 0x0F) == 0x01)	TDSecondT1Position = temp_new_position;

        }
    }
    else 
    {
        icc->T = T0_TYPE;               // if TD1 is absent, then the only offer is T=0 
    }

    // analyze TA2
    temp_position = get_atr_byte_position(atr_buf, atr_len, CHARACTER_TA, 2);    // Get TA2
    if(temp_position)
    {   // TA2 is present
        temp_ta2  = atr_buf[temp_position];
        icc->mode = SPECIFIC_MODE;    // if TA2 is present in ATR, card in specific mode, refer to 6.3.1 in ISO/IEC 7816-3
        if((temp_ta2 & 0x0F) < 2)
        {
            icc->T = temp_ta2 & 0x0F;
        }
    }

    // analyze TA1
    temp_position = get_atr_byte_position(atr_buf, atr_len, CHARACTER_TA, 1);    // Get TA1
    if(temp_position)
    {
        icc->FiDi = atr_buf[temp_position];
        if(icc->mode == SPECIFIC_MODE)
        {
            if(temp_ta2 & 0x10)
            {
                icc->FiDi = DEFAULT_FIDI;
            }
        }
    }

    // analyze TC1, extra guard time
    temp_position = get_atr_byte_position(atr_buf, atr_len, CHARACTER_TC, 1);    // Get TC1
    if(temp_position)
    {
        icc->N = atr_buf[temp_position];
    }

    // analyze TC2, waiting time
    temp_position = get_atr_byte_position(atr_buf, atr_len, CHARACTER_TC, 2);    // Get TC2
    if(temp_position && (atr_buf[temp_position] != 0))
    {   // TC2 present and do not is 0x00, refer to 10.2 in ISO/IEC 7816-3
        icc->WI = atr_buf[temp_position];
    }  

    temp_position = get_atr_byte_position(atr_buf, atr_len, CHARACTER_TD, 2);    // Get TD2 
    temp_new_position = temp_position;
    if(temp_position)
    {
        if(atr_buf[temp_new_position] & 0x10)                 // TA3 is present
        {
            icc->IFSC = atr_buf[++temp_position];    // refer to 11.4.2 in ISO/IEC 7816-3
        }
        if(atr_buf[temp_new_position] & 0x20)                 // TB3 is present
        {
            // refer to 11.4.3 in ISO/IEC 7816-3
            temp_byte = (atr_buf[++temp_position] & 0xF0) >> 4;
            if(temp_byte < 0x0A)         // values A ~ F are reserved for future use
            {
                icc->BWI = temp_byte;  
            }
            icc->CWI = atr_buf[temp_position] & 0x0F;
        }
        if(atr_buf[temp_new_position] & 0x40)                 // TC3 is present
        {
            icc->check = atr_buf[++temp_position] & 0x01;    // refer to 11.4.4 in ISO/IEC 7816-3
        }
    }

    // T = 15 parse
    if(td15_position != 0)
    {
        if((atr_buf[td15_position] & 0x10) == 0x10)    // the first TA for T=15
        {
            icc->clock_stop = (atr_buf[td15_position+1] & 0xC0) >> 6;   // Clock stop indicator (X)
            icc->class   = atr_buf[td15_position+1] & 0x3F;    // class indicator (Y)
        }

        if((atr_buf[td15_position] & 0x20) == 0x20)    // the first TB for T=15
        {
            temp_byte = atr_buf[td15_position+2];
            if(temp_byte == 0)	icc->c6_use = C6_UNUSED;
            else if((temp_byte & 0x80) == 0x00)	icc->c6_use = C6_STANDARD_USE;
            else	icc->c6_use = C6_PROPRIETARY_USE;
        }
    }

//	TRACE_TO("exit %s\n", __func__);
	
}


