

void icc_param_init(struct icc_info	*icc)
{

    icc->status &= ~ICC_ACTIVE;

    icc->mode		= NEGOTIABLE_MODE;
    icc->clock_stop	= NOT_SUPPORT_CLK_STOP;
    icc->class		= CLASS_DEFAULT;
    icc->c6_use		= C6_UNUSED;
    icc->T			= T0_TYPE;
    icc->FiDi		= DEFAULT_FIDI;
    icc->cur_FiDi	= DEFAULT_FIDI;
    icc->direct		= DIRECT_CONVENTION;
    icc->N			= DEFAULT_EXTRA_GUARDTIME;

    icc->WI			= DEFAULT_WAITING_TIME;
    icc->check		= DEFAULT_LRC;
    icc->BWI		= DEFAULT_BWI;
    icc->CWI		= DEFAULT_CWI;
    icc->IFSC		= DEFAULT_IFSC;
    icc->NAD		= DEFAULT_NAD;

    icc->GT			= DEFAULT_GUARDTIME;

}

void set_icc_trans_param(struct icc_info	*icc)
{
    u32 fi;
    u32 def_cwt;
    u32 def_wt;

   
//    TRACE_TO("enter %s\n", __func__);

    if(icc->mode == SPECIFIC_MODE)
    {
        icc->cur_FiDi = icc->FiDi;
    }
    fi = FI[(icc->cur_FiDi & 0xF0) >> 4];
    icc->cur_ETU =  caculate_etu(icc->cur_FiDi, icc->common->freq);
    set_baudrate(caculate_buadrate(icc->cur_FiDi, icc->common->freq));   // set default fidi = 11

    if(icc->T == T0_TYPE)
    {   // waiting time: WT(us) = (WI * 960 * Fi )/ f     refer to 10.2 in ISO/IEC 78163
        icc->WT = ((960 * icc->WI * fi) / (icc->common->freq/1000000));  // WT = (960 * WI *Fi) / f
    }
    else
    {
        if(icc->N == 0xFF)    // refer to 11.2 in ISO/IEC 7816-3
        {
            icc->GT -= 1;
            icc->N = 0x00;
        }
        if(icc->check == DEFAULT_LRC)
        {
            icc->check_sum = 1;
        }
        else
        {
            icc->check_sum = 2;
        }
        icc->BWT = 11 * icc->cur_ETU +  (((1 << icc->BWI) * 960 * 9600) / icc->common->freq); // BWT, refer to 11.4.3 in IEC/ISO 7816-3
        icc->CWT = (11 + (1 << icc->CWI)) * icc->cur_ETU;     // CWT(etu)=(11+2^CWT)

        def_cwt = ((icc->GT + icc->N)) * icc->cur_ETU;
        if(icc->CWT < def_cwt)
        {
            icc->CWT = def_cwt;
        }
        icc->WT = icc->CWT * icc->cur_ETU;
    }
    
    def_wt = DEFAULT_BLOCK_GUARDTIME * icc->cur_ETU;
    if(icc->WT < def_wt)
    {
        icc->WT = def_wt;
    }

	icc->extra_GT = icc->GT + icc->N - 11;

//	TRACE_TO("exit %s\n", __func__);
	
}



