/**
 ***************************************************************************************************
 * @file        ram.h
 * @brief       global variables
 ***************************************************************************************************
 **************************************************************************************************/
#include "test_service.h"

#include "define.h"

// BLE Command
extern bool ble_indicate_flag;
extern bool ble_indicate_enable;
extern bool ble_indicate_ack;
extern bool ble_write_flag;
extern bool ble_connect_flag;

extern uint16_t mtu_max_size;

// app system timet
extern bool sys_timer_flag[TIMER_NUM];
extern bool sys_timer_limit[TIMER_NUM];
extern uint16_t sys_timer_count[TIMER_NUM];

// initialize RAM variables
extern void ram_init(void);

// Device Information
extern uint8_t device_id[DEVICE_ID_LENGTH];

// Test
extern uint8_t rsa_oeap_decrypted_buf[256];
extern uint8_t rsa_oeap_encrypted_buf[256];

