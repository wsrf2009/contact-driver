

//#include "t0.h"


#define APDU_WRITE                          0
#define APDU_READ                           1

#define APDU_HEADER                         0x05               // Apdu header length
#define APDU_CLA                            0x00               // Location of the class in APDU frame
#define APDU_INS                            0x01               // Location of the instruction in APDU frame 
#define APDU_P1                             0x02               // Location of the parameter 1 in APDU frame
#define APDU_P2                             0x03               // Location of the parameter 2 in APDU frame
#define APDU_P3                             0x04               // Location of the parameter 3 in APDU frame

#define PROCEDURE_BYTE_CONFLICT             0xF4


static int icc_t0_dispatch(struct icc_info *icc, u8 *cmd_buf, u32 cmd_len, u8 *res_buf, u32 *res_len)
{
    u8 read_write_flag;
    u32 length = *(cmd_buf + APDU_P3);
    u8 ins = *(cmd_buf + APDU_INS);
    u8 ack_cmd;
    bool end_poll_flag = false;
    bool receive_flag = false;
    u32 data_count = 0;
    u32 data_offset = 0;
    u8 status[2];
    int ret;


//    TRACE_TO("enter %s\n", __func__);

    udelay(icc->cur_ETU * 10);

    if(cmd_len > 5)
    {
        read_write_flag = APDU_WRITE;
    }
    else
    {
        if(cmd_len == 4)    // case 1
        {
            cmd_len = 5;
            *(cmd_buf + APDU_P3) = 0x00;
            length = 0x00;
        }
        read_write_flag = APDU_READ;
    }

    ifd_send_bytes(icc, cmd_buf, APDU_HEADER);
    set_waiting_timer(icc, icc->WT);

    do
    {
        ret = ifd_receive_bytes(icc, &ack_cmd, 1);
        udelay(icc->cur_ETU * 10);
        if(ret)
        {
            end_poll_flag = true;
        }
        else if(ack_cmd == ins)    // ins
        {
            // the data transfer of all remaining data bytes(if any bytes remain)
            if(read_write_flag == APDU_WRITE)
            {
                ifd_send_bytes(icc, cmd_buf + APDU_HEADER + data_offset, length - data_offset);
                receive_flag = false;
            }
            else
            {
                if(length == 0)
               	{
                    data_offset = 256 - data_count;
                }
                else
                {
                    data_offset = (unsigned int)length - data_count;
                }

                ret = ifd_receive_bytes(icc, res_buf+ data_count, data_offset);

                receive_flag = true;
                data_count += data_offset;
            }
        }
        else if((ack_cmd & 0xF0) == 0x90)    // '9X', SW1 byte
        {
            // the reception of a SW2 byte
            status[0] = ack_cmd;
            *(res_buf + (data_count++)) = ack_cmd;
            ret = ifd_receive_bytes(icc, &ack_cmd, 0x01);
            receive_flag = true;
            status[1] = ack_cmd;
            *(res_buf+ (data_count++)) = ack_cmd;
            end_poll_flag = true;
        }
        else if((ack_cmd & 0xF0) == 0x60)
        {
            if(ack_cmd != 0x60)           // '6X', SW1 byte
            { 
                // the reception of a SW2 byte
                status[0] = ack_cmd;
                ret = ifd_receive_bytes(icc, &ack_cmd, 0x01);
                receive_flag = true;
                if(!ret)
                {
                    status[1] = ack_cmd;
                    *(res_buf + (data_count++)) = status[0];
                    *(res_buf+ (data_count++)) = status[1];
                    end_poll_flag = true;
                }
            }
            else                         // '60', NULL byte
            {
                // the reception of a procedure byte
            }
        }
        else if(ack_cmd == (ins ^ 0xFF))    // INS ^ 0xFF
        {
            // the data transfer of the next data byte(if exists)
            if(read_write_flag == APDU_WRITE)
            {
                ifd_send_bytes(icc, cmd_buf + APDU_HEADER + (data_offset++), 0x01);
                receive_flag = false;
            }
            else
            {
                ret = ifd_receive_bytes(icc, res_buf+ (data_offset++), 0x01);
                receive_flag = true;
            }
        }
        else
        {
            ret = -ICC_ERRORCODE_PROCEDURE_BYTE_CONFLICT;
        }

        if(ret)
        {
            end_poll_flag = true;

            if(ret == -ICC_ERRORCODE_XFR_PARITY_ERROR)
            {
                if(receive_flag == true)
                {
                    udelay(icc->cur_ETU);
                    icc_power_off(icc);
                }
            }
        }
    }while(end_poll_flag == false);
    
    *res_len = data_count;

//	TRACE_TO("exit %s, res_len = %d, ret = %d\n",
//				__func__, *res_len, ret);

    return(ret);
}


static int icc_t0_parse(struct icc_info *icc, u8 *cmd_buf, u32 cmd_len, u8 *res_buf, u32 *res_len)
{
    unsigned char ret;


    ret = icc_t0_dispatch(icc, cmd_buf, cmd_len, res_buf, res_len);

    return(ret);
}


