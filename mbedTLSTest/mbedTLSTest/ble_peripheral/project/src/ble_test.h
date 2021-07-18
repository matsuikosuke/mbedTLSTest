/**
 ***************************************************************************************************
 * @file        test_service.h
 * @brief       Set SmartLock Service
 ***************************************************************************************************
 **************************************************************************************************/

#ifndef BLE_TEST_H__
#define BLE_TEST_H__

#include <stdint.h>
#include <stdbool.h>
#include "ble.h"
#include "ble_srv_common.h"
#include "nrf_sdh_ble.h"

#ifdef __cplusplus
extern "C" {
#endif

/**@brief   Macro for defining a ble_test instance.
 *
 * @param   _name   Name of the instance.
 * @hideinitializer
 */
#define BLE_TEST_DEF(_name)                                                                          \
static ble_test_t _name;                                                                            \
NRF_SDH_BLE_OBSERVER(_name ## _obs,                                                                 \
                     BLE_TEST_BLE_OBSERVER_PRIO,                                                     \
                     ble_test_on_ble_evt, &_name)

//BASE-UUID:1e36xxxx-3b3f-405f-9c2f-94c3cd5dacf4
#define TEST_UUID_BASE {0xf4, 0xac, 0x5d, 0xcd, 0xc3, 0x94, 0x2f, 0x9c, \
                              0x5f, 0x40, 0x3f, 0x3b, 0x00, 0x00, 0x36, 0x1e}
#define TEST_UUID_SERVICE     0xDEF0
#define LOCK_STATE_UUID_CHAR       0xDEF1
#define COMMAND_INDICATION_UUID_CHAR      0xDEF2 //Indication
#define COMMAND_WRITE_UUID_CHAR    0xDEF3 //Write
#define LOCKSTATE_UUID_CHAR        0xDEF4 //Read

#define	MAX_INDICATION_NUM	20
#define	MAX_WRITE_NUM	20
#define BLE_MAX_DATA_LEN (BLE_GATT_ATT_MTU_DEFAULT - 3) /**< Maximum length of data (in bytes) that can be transmitted to the peer by the Nordic UART service module. */

typedef enum
{
    BLE_TEST_EVT_NOTIFICATION_ENABLED,  
    BLE_TEST_EVT_NOTIFICATION_DISABLED  
} ble_test_evt_type_t;

typedef struct
{
    ble_test_evt_type_t evt_type; 
} ble_test_evt_t;

// Forward declaration of the ble_test_t type.
typedef struct ble_test_s ble_test_t;

typedef void (*ble_test_write_handler_t) (uint16_t conn_handle, ble_test_t * p_test, const uint8_t *receive_buff, uint8_t length);

/**
 ***************************************************************************************************
 *	@brief			Test Service init structure.
 *	@details		This structure contains all options and data needed for
 *                              initialization of the service.
 **************************************************************************************************/
typedef struct
{
    ble_test_write_handler_t test_write_handler; /**< Event handler to be called when the Characteristic is written. */
    //uint8_t *                   p_test_state;
} ble_test_init_t;

/**
 ***************************************************************************************************
 *	@brief			Test Service structure.
 *	@details		This structure contains various status information for the service.
 **************************************************************************************************/
struct ble_test_s
{
    uint16_t                    service_handle;      /**< Handle of Test Service (as provided by the BLE stack). */
    ble_gatts_char_handles_t    lock_state_handles;  /**< Handles related to the LockState Characteristic. */
    ble_gatts_char_handles_t    command_res_handles; /**< Handles related to the CommandResponse Characteristic. */
    ble_gatts_char_handles_t    command_trans_handles;  /**< Handles related to the CommandTransmission Characteristic. */
    uint8_t                     uuid_type;           /**< UUID type for the Test Service. */
    uint16_t                    conn_handle; 
    //uint8_t *                   p_test_state;
    ble_test_write_handler_t test_write_handler;   /**< Event handler to be called when the LED Characteristic is written. */
};

extern uint8_t write_buf[20];
extern uint16_t write_len;

 /**
 ***************************************************************************************************
 *	@brief			Function for initializing the Test Service.
 *	@param[out] p_test
 *                Test Service structure. This structure must be supplied by
 *                the application. It is initialized by this function and will later
 *                be used to identify this particular service instance.
 *	@param[in] p_test_init
 *                Information needed to initialize the service.
 *	@retval NRF_SUCCESS 
 *                If the service was initialized successfully. Otherwise, an error code is returned.
 **************************************************************************************************/
uint32_t ble_test_init(ble_test_t * p_test, const ble_test_init_t * p_test_init);


/**@brief Function for handling the application's BLE stack events.
 *
 * @details This function handles all events from the BLE stack that are of interest to the LED Button Service.
 *
 * @param[in] p_ble_evt  Event received from the BLE stack.
 * @param[in] p_context  Test Service structure.
 */
void ble_test_on_ble_evt(ble_evt_t const * p_ble_evt, void * p_context);


/**@brief Function for sending a button state notification.
 *
 * @param[in] p_test Test Service structure.
 * @param[in] data  Notification data.
 * @param[in] len  length of Notification data.
 *
 * @retval NRF_SUCCESS If the notification was sent successfully. Otherwise, an error code is returned.
 */
extern uint32_t ble_test_notify_test(uint16_t conn_handle, ble_test_t * p_test, uint8_t *data, uint16_t len);
extern uint32_t ble_test_notify(ble_test_t * p_test, uint8_t *data, uint16_t len);

void test_write_handler(uint16_t conn_handle, ble_test_t * p_test, const uint8_t *receive_buf, uint8_t length);

#ifdef __cplusplus
}
#endif

#endif // BLE_TEST_H__

/** @} */
