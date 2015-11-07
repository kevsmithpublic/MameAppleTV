#ifndef __CPUCTRL_H__
#define __CPUCTRL_H__

extern void cpuctrl_init(void); /* call this at first */
extern void save_system_regs(void); /* save some registers */
extern void cpuctrl_deinit(void);
extern void set_display_clock_div(unsigned divider);
extern void set_FCLK(unsigned MHZ); /* adjust the clock frequency (in Mhz units) */
extern void set_920_Div(unsigned short divider); /* 0 to 7 divider (freq=FCLK/(1+div)) */
extern void set_DCLK_Div(unsigned short divider); /* 0 to 7 divider (freq=FCLK/(1+div)) */
extern void Disable_940(void); /* 940t down */
extern void gp2x_video_wait_vsync(void);

/* Memory RAM Tweaks */
extern void set_tRC(unsigned short timing); //ACTIVE to ACTIVE /AUTOREFRESH command delay
extern void set_tRAS(unsigned short timing); //ACTIVE to PRECHARGE delay
extern void set_tWR(unsigned short timing); //Write recovery time
extern void set_tMRD(unsigned short timing); //LOAD MODE REGISTER command cycle time
extern void set_tRFC(unsigned short timing); //AUTO REFRESH command period
extern void set_tRP(unsigned short timing); //PRECHARGE command period
extern void set_tRCD(unsigned short timing); //RAS to CAS Delay
extern void set_gamma(int g100);

#endif
