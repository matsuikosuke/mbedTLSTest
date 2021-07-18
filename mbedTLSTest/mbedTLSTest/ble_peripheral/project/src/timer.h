/**
 ***************************************************************************************************
 * @file        timer.h
 * @brief       TIMER Setting
 ***************************************************************************************************
 **************************************************************************************************/
#ifndef TIMER_H
#define TIMER_H

#ifdef TIMER_GLOBAL
#define TIMER_EXTERN
#else
#define TIMER_EXTERN extern
#endif

/*--------------------------------------------------------------------------------------------------
- Function Declaration
--------------------------------------------------------------------------------------------------*/
TIMER_EXTERN void timers_init(void);

TIMER_EXTERN void system_timer_start(void);
TIMER_EXTERN void motor_timer_start(void);
TIMER_EXTERN void pwm_timer_start(void);

TIMER_EXTERN void system_timer_stop(void);
TIMER_EXTERN void motor_timer_stop(void);
TIMER_EXTERN void pwm_timer_stop(void);

TIMER_EXTERN void motor_timer_init(void);

#endif /* TIMER_H */
/***************************************************************************************************
 **************************************************************************************************/