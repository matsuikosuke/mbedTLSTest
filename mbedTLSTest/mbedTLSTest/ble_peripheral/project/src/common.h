/**
 ***************************************************************************************************
 * @file        common.h
 * @brief       ヘッダーファイル
 ***************************************************************************************************
 **************************************************************************************************/
#ifndef COMMON_H /* COMMON_H */
#define COMMON_H

#include <stdbool.h>

#include "define.h"
#include "if.h"
#include "ram.h"
#include "test.h"

// app
#include "app_error.h"
#include "app_timer.h"
#include "app_util_platform.h"

// nrf drivers
#include "nrf.h"
#include "nrf_delay.h"
#include "nrf_drv_clock.h"
#include "nrf_drv_gpiote.h"
#include "nrf_drv_ppi.h"
#include "nrf_drv_rng.h"
#include "nrf_drv_saadc.h"
#include "nrf_drv_timer.h"

// modules
#include "ble_ctrl.h"
#include "cpu_set.h"
#include "timer.h"


#endif /* COMMON_H */
/***************************************************************************************************
 **************************************************************************************************/