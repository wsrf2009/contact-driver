


u8 byte_direct_2_inverse(u8 data)
{
    u8 inverse_data = 0;
    u8 i = 0;

    
    do
    {
        if(BITisSET(data, i))	CLEAR_BIT(inverse_data, (7-i));
        else		SET_BIT(inverse_data, (7-i));

        i++;
        
    }while(i < 8);

    return(inverse_data);
}


u32 caculate_etu(u8 FiDi, u32 f)
{
    u32 fi;
    u32 di;
    u32 etu;


    fi = FI[(FiDi>>4) & 0x0F];        // Get the Fi and Di by inquiry table FI and DI
    di = DI[FiDi & 0x0F];

    etu =(fi)/(di * (f / 1000000));     // calculate baud rate: 1 etu =F/(D*f)    7816-3

    INFO_TO("%s ==> FiDi = %X, fi = %d, di = %d, etu = %d\n",
				__func__, FiDi, fi, di, etu);

    return(etu);
}



u32 caculate_buadrate(u8 FiDi, u32 f)
{
	u32 fi;
	u32 di;
	u32 baudrate;


//	TRACE_TO("exit %s\n", __func__);
    
    fi = FI[(FiDi>>4) & 0x0F];        // Get the Fi and Di by inquiry table FI and DI
    di = DI[FiDi & 0x0F];
    baudrate = (di * f)/fi;

    INFO_TO("%s ==> FiDi = %X, fi = %d, di = %d, BaudRate = %d\n",
				__func__, FiDi, fi, di, baudrate);

    return(baudrate);
}

