





static void waiting_timer_expire_handler(unsigned long data)
{
	struct ifd_common *common = (struct ifd_common	*)data;

	
    common->time_out = true;
//    WaitingTimer.expires = jiffies + HZ;
    add_timer(&common->waiting_timer);
//    if(BITisSET(*Reg_83C26_RESET, 4))
//    {
//        Set_GPIO_Low(Reg_83C26_RESET, OFFS_0);
//    }
//    else
//    {
//        Set_GPIO_High(Reg_83C26_RESET, OFFS_0);
//    }
}

static void waiting_timer_init(struct ifd_common	 *common)
{
    init_timer(&common->waiting_timer);
	common->time_out = false;
    common->waiting_timer.function = waiting_timer_expire_handler;
    common->waiting_timer.data = (unsigned long)common;                              // it is the argument of the timer_list.function
    common->waiting_timer.expires = jiffies + HZ;
    add_timer(&common->waiting_timer);
}

static void set_waiting_timer(struct icc_info *icc, u32 w_time)
{
	struct ifd_common	 *common = icc->common;
    unsigned  long l_wait;


    l_wait = (w_time / 1000) / (1000 / HZ);
    common->time_out = false;
    mod_timer(&common->waiting_timer, jiffies + l_wait + 1);
}


static void waiting_timer_uninit(struct ifd_common	 *common)
{
    del_timer(&common->waiting_timer);
}


