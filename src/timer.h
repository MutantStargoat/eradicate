#ifndef TIMER_H_
#define TIMER_H_

#ifdef __cplusplus
extern "C" {
#endif

/* expects the required timer resolution in hertz
 * if res_hz is 0, the current resolution is retained
 */
void init_timer(int res_hz);

void reset_timer(void);
unsigned long get_msec(void);

#ifdef __cplusplus
}
#endif

#endif	/* TIMER_H_ */
