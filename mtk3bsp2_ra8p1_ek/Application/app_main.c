#include <tk/tkernel.h>
#include <tm/tmonitor.h>

LOCAL void task_1(INT stacd, void *exinf);	// task execution function
LOCAL ID	tskid_1;			// Task ID number
LOCAL T_CTSK ctsk_1 = {				// Task creation information
	.itskpri	= 10,
	.stksz		= 1024,
	.task		= task_1,
	.tskatr		= TA_HLNG | TA_RNG3,
};

LOCAL void task_2(INT stacd, void *exinf);	// task execution function
LOCAL ID	tskid_2;			// Task ID number
LOCAL T_CTSK ctsk_2 = {				// Task creation information
	.itskpri	= 10,
	.stksz		= 1024,
	.task		= task_2,
	.tskatr		= TA_HLNG | TA_RNG3,
};

LOCAL void task_3(INT stacd, void *exinf);	// task execution function
LOCAL ID	tskid_3;			// Task ID number
LOCAL T_CTSK ctsk_3 = {				// Task creation information
	.itskpri	= 10,
	.stksz		= 1024,
	.task		= task_3,
	.tskatr		= TA_HLNG | TA_RNG3,
};

LOCAL void task_1(INT stacd, void *exinf)
{
	while(1) {
		tm_printf((UB*)"task 1\n");

		/* Inverts the LED on the board. */
        	out_h(PORT_PODR(6), in_h(PORT_PODR(6))^(1<<0));
		tk_dly_tsk(500);
	}
}

LOCAL void task_2(INT stacd, void *exinf)
{
	while(1) {
		tm_printf((UB*)"task 2\n");
        	out_h(PORT_PODR(3), in_h(PORT_PODR(3))^(1<<3));
		tk_dly_tsk(700);
	}
}

LOCAL void task_3(INT stacd, void *exinf)
{
	while(1) {
		tm_printf((UB*)"task 3\n");
        	out_h(PORT_PODR(A), in_h(PORT_PODR(A))^(1<<7));
		tk_dly_tsk(1000);
	}
}


/* --- AI & Camera Tasks --- */
ID g_ai_app_event;

extern void camera_display_thread_entry(void *pvParameters);
extern void ai_inference_thread_entry(void *pvParameters);

LOCAL void task_camera(INT stacd, void *exinf)
{
    camera_display_thread_entry(NULL);
    tk_ext_tsk();
}
LOCAL T_CTSK ctsk_camera = {
    .itskpri = 9,
    .stksz = 8192,
    .task = task_camera,
    .tskatr = TA_HLNG | TA_RNG3,
};

LOCAL void task_ai(INT stacd, void *exinf)
{
    ai_inference_thread_entry(NULL);
    tk_ext_tsk();
}
LOCAL T_CTSK ctsk_ai = {
    .itskpri = 10,
    .stksz = 32768,
    .task = task_ai,
    .tskatr = TA_HLNG | TA_RNG3,
};
/* ------------------------ */

/* usermain関数 */
EXPORT INT usermain(void)
{
	tm_putstring((UB*)"Start User-main program.\n");

	/* Turn off the LED on the board. */
	/* LED1(BLUE):P600 LED2(GREEN):P303 LED3(RED):PA07 */
	out_h(PORT_PODR(6), in_h(PORT_PODR(6))&~(1<<0));
	out_h(PORT_PODR(3), in_h(PORT_PODR(3))&~(1<<3));
	out_h(PORT_PODR(A), in_h(PORT_PODR(A))&~(1<<7));

	/* Create & Start Tasks */
	tskid_1 = tk_cre_tsk(&ctsk_1);
	tk_sta_tsk(tskid_1, 0);

	tskid_2 = tk_cre_tsk(&ctsk_2);
	tk_sta_tsk(tskid_2, 0);

	tskid_3 = tk_cre_tsk(&ctsk_3);
	tk_sta_tsk(tskid_3, 0);

	
	/* Create AI Event Flag */
	T_CFLG cflg = {
		.flgatr = TA_TFIFO | TA_WMUL,
		.iflgptn = 0
	};
	g_ai_app_event = tk_cre_flg(&cflg);

	/* Start Camera & AI Tasks */
	ID tskid_camera = tk_cre_tsk(&ctsk_camera);
	tk_sta_tsk(tskid_camera, 0);
	
	ID tskid_ai = tk_cre_tsk(&ctsk_ai);
	tk_sta_tsk(tskid_ai, 0);

	tk_slp_tsk(TMO_FEVR);

	return 0;
}
