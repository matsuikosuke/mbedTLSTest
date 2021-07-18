/**
 ***************************************************************************************************
 * @file        ram.c
 * @brief       global variables
 ***************************************************************************************************
 **************************************************************************************************/
#include "common.h"
#include "test_service.h"
#include <stdint.h>
#include <string.h>

// BLE Command
bool ble_indicate_flag;
bool ble_indicate_enable;
bool ble_indicate_ack;
bool ble_write_flag;
bool ble_connect_flag;

uint16_t mtu_max_size;

// ble_test_t stoptexDoorService;


// app system timer
bool sys_timer_flag[TIMER_NUM];
bool sys_timer_limit[TIMER_NUM];
uint16_t sys_timer_count[TIMER_NUM];

// Device Information
uint8_t device_id[DEVICE_ID_LENGTH];

// Test
uint8_t rsa_oeap_decrypted_buf[256];
uint8_t rsa_oeap_encrypted_buf[256];


/**@brief Function for handling the Connect event.
 *
 * @param[in]   p_test       TEST Service structure.
 * @param[in]   p_ble_evt   Event received from the BLE stack.
 */
static uint8_t id_to_string(uint8_t num)
{
    uint8_t trans_num;

    if (num >= 0x00 && num < 0x0A)
    {
        trans_num = num + '0';
    }
    else if (num >= 0x0A && num < 0x10)
    {
        trans_num = num - 0x0A + 'A';
    }
    else
    {
        trans_num = '0';
    }

    return trans_num;
}

// initialize RAM variables
void ram_init(void)
{
    // BLE Command
    ble_indicate_ack = false;
    ble_write_flag = false;
    ble_connect_flag = false;
    
    ble_indicate_flag = false;
    ble_indicate_enable = false;
    ble_indicate_ack = false;
    ble_write_flag = false;
    ble_connect_flag = false;

    // Device Information
    uint32_t temp_device_id = NRF_FICR->DEVICEID[0];
    device_id[0] = id_to_string((uint8_t)((0xF0000000 & temp_device_id) >> 28));
    device_id[1] = id_to_string((uint8_t)((0x0F000000 & temp_device_id) >> 24));
    device_id[2] = id_to_string((uint8_t)((0x00F00000 & temp_device_id) >> 20));
    device_id[3] = id_to_string((uint8_t)((0x000F0000 & temp_device_id) >> 16));
    device_id[4] = id_to_string((uint8_t)((0x0000F000 & temp_device_id) >> 12));
    device_id[5] = id_to_string((uint8_t)((0x00000F00 & temp_device_id) >> 8));
    device_id[6] = id_to_string((uint8_t)((0x000000F0 & temp_device_id) >> 4));
    device_id[7] = id_to_string((uint8_t)(0x0000000F & temp_device_id));
}