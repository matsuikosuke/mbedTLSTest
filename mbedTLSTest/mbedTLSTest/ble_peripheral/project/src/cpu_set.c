/**
 ***************************************************************************************************
 * @file    cpu_set.c
 * @brief   CPU Setting
 ***************************************************************************************************
 **************************************************************************************************/
#define CPU_SET_GLOBAL
#include "common.h"


static void gpio_init(void);
static void wdt_init(void);

/**
 ***************************************************************************************************
 *	@brief			CPU Modules initialization function
 *	@details		Initialize CPU Modules
 **************************************************************************************************/
void cpu_init(void)
{
    // Port initialize setting
    gpio_init();

    // Timer initialize setting
    timers_init();

    // WDT initialize setting
    //wdt_init();

    // RAM initialize setting
    ram_init();
}

/**
 * @brief Function for configuring: PIN_IN pin for input, PIN_OUT pin for output,
 * and configures GPIOTE to give an interrupt on pin change.
 */
/**
***************************************************************************************************
*	@brief			CPU Modules initialization function
*	@details		Initialize CPU Modules
**************************************************************************************************/
static void gpio_init(void)
{
    ret_code_t err_code;

    err_code = nrf_drv_gpiote_init();
    APP_ERROR_CHECK(err_code);
}

/**
***************************************************************************************************
*	@brief			WDT initialization function
*	@details		Initialize WDT Module
**************************************************************************************************/
static void wdt_init(void)
{
    NRF_WDT->CONFIG = (WDT_CONFIG_HALT_Pause << WDT_CONFIG_HALT_Pos) | (WDT_CONFIG_SLEEP_Pause << WDT_CONFIG_SLEEP_Pos);
    NRF_WDT->CRV = 3 * 32768;          // 3sec
    NRF_WDT->RREN |= WDT_RREN_RR0_Msk; // Enable reload register 0
    NRF_WDT->TASKS_START = 1;
}
