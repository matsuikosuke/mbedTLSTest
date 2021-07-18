/**
 ***************************************************************************************************
 * @file    timer.c
 * @brief   TIMER Setting
 ***************************************************************************************************
 **************************************************************************************************/
#define TIMER_GLOBAL
#include "common.h"

/*--------------------------------------------------------------------------------------------------
 Static RAM
--------------------------------------------------------------------------------------------------*/
APP_TIMER_DEF(sys_timer_id);    // TimerID: for System

/*--------------------------------------------------------------------------------------------------
 Static Function
--------------------------------------------------------------------------------------------------*/
static void sys_timer_handler(void * p_context);   // System Timer Handler

/**
 ***************************************************************************************************
 *	@brief			Timer initialization function
 *	@details		Initialize timer module and create Timers
 **************************************************************************************************/
void timers_init(void)
{
    // Initialize timer module.
    ret_code_t err_code = app_timer_init();
    APP_ERROR_CHECK(err_code);

    // Create system timer.
    err_code = app_timer_create(&sys_timer_id, APP_TIMER_MODE_REPEATED, sys_timer_handler);
    APP_ERROR_CHECK(err_code);
}


/**
 ***************************************************************************************************
 *	@brief			System timer starting function
 *	@details		Start system timer
 **************************************************************************************************/
void system_timer_start(void)
{
    ret_code_t err_code;

    // Start application timers
    err_code = app_timer_start(sys_timer_id, MAIN_INTERVAL, NULL);
    APP_ERROR_CHECK(err_code);
}


/**
 ***************************************************************************************************
 *	@brief			System timer stopping function
 *	@details		Stop System timer
 **************************************************************************************************/
void system_timer_stop(void)
{
    ret_code_t err_code;

    // Stop application timers
    err_code = app_timer_stop(sys_timer_id);
    APP_ERROR_CHECK(err_code);
}

/**
 ***************************************************************************************************
 *	@brief			System Timer Handler
 *	@details		Perform System Timer Handler processing
 **************************************************************************************************/
static void sys_timer_handler(void * p_context)
{
    /* 50ms */
    // stSystem.fgInt50ms = ON;

    for (int i = 0; i < TIMER_NUM; i++)
    {
        if (true == sys_timer_flag[i])
        {
            sys_timer_count[i] += 1;
        }
        else
        {
            sys_timer_count[i] = 0;
        }
    }

    if (sys_timer_count[TEST_TIMER] >= TEST_TIMER_LIMIT)
    {
        sys_timer_count[TEST_TIMER] = 0;
        sys_timer_limit[TEST_TIMER] = true;
    }
}



/***************************************************************************************************
 **************************************************************************************************/