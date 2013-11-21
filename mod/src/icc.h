

#ifndef SMARTCARD_H
#define SMARTCARD_H








#define ICC_ERRORCODE_NOT_SUPPORTED					0
#define ICC_ERRORCODE_CMD_ABORTED					1		// Host aborted the current activity
#define ICC_ERRORCODE_ICC_MUTE						2		// CCID timed out while talking to the ICC
#define ICC_ERRORCODE_XFR_PARITY_ERROR				3		// Parity error while talking to the ICC
#define ICC_ERRORCODE_XFR_OVERRUN					4		// Overrun error while talking to the ICC
#define ICC_ERRORCODE_HW_ERROR						5		// An all inclusive hardware error occurred
#define ICC_ERRORCODE_BAD_ATR_TS					6
#define ICC_ERRORCODE_BAD_ATR_TCK					7
#define ICC_ERRORCODE_ICC_PROTOCOL_NOT_SUPPORTED	8
#define ICC_ERRORCODE_ICC_CLASS_NOT_SUPPORTED		9
#define ICC_ERRORCODE_PROCEDURE_BYTE_CONFLICT		10
#define ICC_ERRORCODE_DEACTIVATED_PROTOCOL			11
#define ICC_ERRORCODE_BUSY_WITH_AUTO_SEQUENCE		12		// Automatic Sequence Ongoing
#define ICC_ERRORCODE_PIN_TIMEOUT					13
#define ICC_ERRORCODE_PIN_CANCELLED					14
#define ICC_ERRORCODE_CMD_SLOT_BUSY					15		// A second command was sent to a slot which was already processing a command

#define ICC_ERRORCODE_NONE									0
#define ICC_ERRORCODE_BAD_LENGTGH							16
#define ICC_ERRORCODE_SLOT_NO_EXIST							17
#define ICC_ERRORCODE_POWERSELECT_NO_SUPPORTED				18
#define ICC_ERRORCODE_PROTOCOL_INVALID_OR_NOT_SUPPORTED		19
#define ICC_ERRORCODE_BAD_LEVEL_PARAMETER					20
#define ICC_ERRORCODE_FI_DI_PAIR_INVALID_OR_NOT_SUPPORTED	21
#define ICC_ERRORCODE_INVALID_TCCKTS_PARAMETER				22
#define ICC_ERRORCODE_GUARD_TIME_NOT_SUPPORTED				23
#define ICC_ERRORCODE_WI_BWI_CWI_INVALID_OR_NOT_SUPPORTED	24
#define ICC_ERRORCODE_CLOCK_STOP_REQUEST_FALID				25
#define ICC_ERRORCODE_IFSC_SIZE_INVALID_OR_NOT_SUPPORTED	26
#define ICC_ERRORCODE_NAD_VALUE_INVALID_OR_NOT_SUPPORTED	27
// user defined errorcode
#define ICC_ERRORCODE_INVALID_BLOCK 						64
#define ICC_ERRORCODE_T1_OTHER_ERROR						65
#define ICC_ERRORCODE_T1_RETRY_FAIL							66
#define ICC_ERRORCODE_T1_CHECKSUM_ERROR						67
#define ICC_ERRORCODE_FRAMERROR								68
#define	ICC_ERRORCODE_NOT_ACTIVED							69




struct icc_info
{
	u8		slot;
	
    // Flag variable
#define ICC_PRESENT         	BIT(0)
#define ICC_ACTIVE	           	BIT(1)
#define ICC_STATUS_CHANGE		BIT(7)

    u8		status;       // Flag used to know if the power up sequence has been processed
    
    u8		mode;             // After any answer to reset, the card is in either in specific mode or negotiable mode
    u8		clock_stop;          // whether the card supports clock stop
    u8		class;            // classes of operating conditions accepted by the card
    u8		c6_use;

    // T=0 and T=1 common variable
    u8		T;                // a slot card use protocol T = 0 or T = 1
    u8		FiDi;             // Value speed com smart card = TA1
    u8		cur_FiDi;
    u8		direct;           // Define convention direct or inverse 
    u8		N;                // Extra Guard Time 

    // T=0 specific variable
    u8		WI;               // Waiting time, T=0,given by 960 x D x WI;

    // T=1 specific variable
    u8		check;            // epilogue field consist in protocol T = 1
    u8		BWI;              // Block waiting time intger 
    u8		CWI;              // character waiting time intger
    u8		IFSC;             // IFS for the card
    u8		NAD;              // Node address byte

    u32		cur_ETU;
    u8		GT;
    u32		WT;
    u8		check_sum;
    u32		CWT;
    u32		BWT;
    u8		extra_GT;

    u8		first_Iblock;

    // T1ExChangeFlag: |bit7              |bit6 |bit5 |bit4 |bit3                   |bit2                  |bit1                     |bit0                   |
    // T1ExChangeFlag: |FirstAPDUBit |       |       |       |ICCChainingBit |IFDChainingBit |ICCBlockNumber |IFDBlockNumber|                             
    u8		T1_exchange_flag;


	struct	ifd_common	*common;
};







//int SC_PowerOn(unsigned char CardIdx, unsigned char *AtrBuf, unsigned int *AtrLen);
//int SC_PowerOff(unsigned char CardIdx);
//unsigned char SC_TransAPDU(unsigned char CardIdx, unsigned char *CmdBuf, unsigned int CmdLen, unsigned char *ResBuf, unsigned int *ResLen);
//int SC_Device_Init(void);
//void SC_Device_Uninit(void);

#endif
