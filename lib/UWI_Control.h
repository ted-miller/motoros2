//UWI_Control.h
//

#define NUM_CUSTOM_PRM	8
typedef	struct 
{
    UCHAR		proc_no;		// welding process no.
    UCHAR		reserved[3];
    BITSTRING	flags;			// valid custom parameter specification.(d0~d7)
    long		param[NUM_CUSTOM_PRM];	// custom parameters.
}	MP_UWI_CUSTOM_DATA;

extern void uwi_user_arcon(int weld_no, int file_no, MP_UWI_CUSTOM_DATA* p);
extern void uwi_user_arcof(int weld_no, int file_no, MP_UWI_CUSTOM_DATA* p);
extern void uwi_user_arcset(int weld_no, int file_no, MP_UWI_CUSTOM_DATA* p, int condition_set);

extern void mpTaskStartLincolnWelder(void);
