#include "sdk_common.h"
#if NRF_MODULE_ENABLED(BLE_TEST)
#include "ble_test.h"
#include "ble_srv_common.h"


static bool ble_command_receive( const uint8_t *data, uint16_t len);

/**@brief Function for handling the Connect event.
 *
 * @param[in]   p_test       TEST Service structure.
 * @param[in]   p_ble_evt   Event received from the BLE stack.
 */
static void on_connect(ble_test_t * p_test, ble_evt_t const * p_ble_evt)
{
    p_test->conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
}


/**@brief Function for handling the Disconnect event.
 *
 * @param[in]   p_test       Heart Rate Service structure.
 * @param[in]   p_ble_evt   Event received from the BLE stack.
 */
static void on_disconnect(ble_test_t * p_test, ble_evt_t const * p_ble_evt)
{
    UNUSED_PARAMETER(p_ble_evt);
    p_test->conn_handle = BLE_CONN_HANDLE_INVALID;
}

/**
 ***************************************************************************************************
 *	@brief			on read function
 *	@details		Function for handling the Read event.
 *      @param[in] p_test  SmartLock Service structure.
 *      @param[in] p_ble_evt    Event received from the BLE stack.
 *      @param[in] data         Response data.
 **************************************************************************************************/
static void on_read(ble_test_t * p_test, ble_evt_t const * p_ble_evt, uint8_t data)
{
    uint32_t err_code;

    ble_gatts_rw_authorize_reply_params_t reply_params;

    memset(&reply_params, 0, sizeof(reply_params));
      
    reply_params.type = BLE_GATTS_AUTHORIZE_TYPE_READ;
    reply_params.params.read.p_data    = &data;
    reply_params.params.read.len       = sizeof(data);
    reply_params.params.read.offset    = 0;
    reply_params.params.read.update    = 1;
    reply_params.params.read.gatt_status = NRF_SUCCESS;
      
    sd_ble_gatts_rw_authorize_reply(p_ble_evt->evt.gap_evt.conn_handle, &reply_params);
}

/**
 ***************************************************************************************************
 *	@brief			on write function
 *	@details		Function for handling the Write event.
 *      @param[in] p_test  SmartLock Service structure.
 *      @param[in] p_ble_evt    Event received from the BLE stack.
 **************************************************************************************************/
static void on_write(ble_test_t * p_test, ble_evt_t const * p_ble_evt)
{
    bool result;
    ble_gatts_evt_write_t const * p_evt_write = &p_ble_evt->evt.gatts_evt.params.write;

    if ((p_evt_write->handle == p_test->command_trans_handles.value_handle)
        && (p_evt_write->len >= 1)
        && (p_test->test_write_handler != NULL))
    {
        p_test->test_write_handler(p_ble_evt->evt.gap_evt.conn_handle, p_test, &p_evt_write->data[0], p_evt_write->len);

        result = ble_command_receive(&p_evt_write->data[0], p_evt_write->len);
        if (false == result)
        {
           //BLE Command Error Event //★：未実装
        }
    }
}

/**
 ***************************************************************************************************
 *	@brief			on BLE EVENT function
 *	@details		Function for handling the BLE event.
 *      @param[in] p_ble_evt    Event received from the BLE stack.
 **************************************************************************************************/
void ble_test_on_ble_evt(ble_evt_t const * p_ble_evt, void * p_context)
{
    ble_test_t * p_test = (ble_test_t *)p_context;

    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
            on_connect(p_test, p_ble_evt);
            break;

        case BLE_GAP_EVT_DISCONNECTED:
            on_disconnect(p_test, p_ble_evt);
            break;

        case BLE_GATTS_EVT_WRITE:
            on_write(p_test, p_ble_evt);
            break;

        case BLE_GATTS_EVT_RW_AUTHORIZE_REQUEST:
            on_read(p_test, p_ble_evt, 0x5A);
            break;

        default:
            // No implementation needed.
            break;
    }
}

/**
 ***************************************************************************************************
 *	@brief			LockState Characteristic Setting
 *	@details		Setting LockState Characteristic
 *      @param[in] p_test  SmartLock Service structure.
 **************************************************************************************************/
static uint32_t lock_state_char_add(ble_test_t * p_test, const ble_test_init_t * p_test_init)
{
//    uint32_t              err_code;
//    ble_uuid_t            ble_uuid;
//    ble_add_char_params_t add_char_params;
//
//    memset(&add_char_params, 0, sizeof(add_char_params));
//    add_char_params.uuid              = LOCKSTATE_UUID_CHAR;
//    add_char_params.uuid_type         = p_test->uuid_type;
//    add_char_params.init_len          = sizeof(uint8_t);
//    add_char_params.is_var_len       = true;
//    add_char_params.max_len           = sizeof(uint8_t);
//    add_char_params.char_props.read   = 1;
//    //add_char_params.char_props.notify = 1;
//
//    add_char_params.read_access       = SEC_OPEN;
//    //add_char_params.cccd_write_access = SEC_OPEN;
//
//    return err_code = characteristic_add(p_test->service_handle,
//                                  &add_char_params,
//                                  &p_test->lock_state_handles);



    ble_gatts_char_md_t char_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          ble_uuid;
    ble_gatts_attr_md_t attr_md;

    memset(&char_md, 0, sizeof(char_md));

    char_md.char_props.read   = 1;
    char_md.char_props.notify   = 0;
    char_md.p_char_user_desc  = NULL;
    char_md.p_char_pf         = NULL;
    char_md.p_user_desc_md    = NULL;
    char_md.p_cccd_md         = NULL;
    char_md.p_sccd_md         = NULL;
    
    ble_uuid.type = p_test->uuid_type;
    ble_uuid.uuid = LOCKSTATE_UUID_CHAR;

    memset(&attr_md, 0, sizeof(attr_md));

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);
    attr_md.vloc       = BLE_GATTS_VLOC_STACK;
    attr_md.rd_auth    = 1;
    attr_md.wr_auth    = 0;
    attr_md.vlen       = 1;
    //attr_md.read_perm.sm = 1;      // Security Mode 1 Level 1: Open link
    //attr_md.read_perm.lv = 1;

    memset(&attr_char_value, 0, sizeof(attr_char_value));

    attr_char_value.p_uuid    = &ble_uuid;
    attr_char_value.p_attr_md = &attr_md;
    attr_char_value.init_len  = sizeof(uint8_t);
    attr_char_value.init_offs = 0;
    attr_char_value.max_len   = sizeof(uint8_t);
    attr_char_value.p_value   = NULL;

    return sd_ble_gatts_characteristic_add(p_test->service_handle, 
                                               &char_md,
                                               &attr_char_value,
                                               &p_test->lock_state_handles);
}

/**
 ***************************************************************************************************
 *	@brief			CommandResponse Characteristic Setting
 *	@details		Setting CommandResponse Characteristic
 *      @param[in] p_test  SmartLock Service structure.
 **************************************************************************************************/
static uint32_t command_trans_char_add(ble_test_t * p_test, const ble_test_init_t * p_test_init)
{

    uint32_t              err_code;
    ble_uuid_t            ble_uuid;
    ble_add_char_params_t add_char_params;

    memset(&add_char_params, 0, sizeof(add_char_params));
    add_char_params.uuid             = COMMAND_TRANS_UUID_CHAR;
    add_char_params.uuid_type        = p_test->uuid_type;
    add_char_params.init_len         = sizeof(uint8_t);
    add_char_params.is_var_len       = true;
    add_char_params.max_len          = MAX_WRITE_NUM;//sizeof(uint8_t);
    //add_char_params.char_props.read  = 1;
    add_char_params.char_props.write = 1;

    //add_char_params.read_access  = SEC_OPEN;
    add_char_params.write_access = SEC_OPEN;

    return characteristic_add(p_test->service_handle, 
                                  &add_char_params, 
                                  &p_test->command_trans_handles);

//
//    ble_gatts_char_md_t char_md;
//    ble_gatts_attr_md_t cccd_md;
//    ble_gatts_attr_t    attr_char_value;
//    ble_uuid_t          ble_uuid;
//    ble_gatts_attr_md_t attr_md;
//
//    memset(&cccd_md, 0, sizeof(cccd_md));
//
//    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.read_perm);
//    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.write_perm);
//    cccd_md.vloc       = BLE_GATTS_VLOC_STACK;
//
//    memset(&char_md, 0, sizeof(char_md));
//
//    char_md.char_props.read   = 1;
//    char_md.char_props.write  = 1;
//    char_md.char_props.notify = 1;
//    char_md.p_char_user_desc  = NULL;
//    char_md.p_char_pf         = NULL;
//    char_md.p_user_desc_md    = NULL;
//    char_md.p_cccd_md         = &cccd_md;
//    char_md.p_sccd_md         = NULL;
//
//    ble_uuid.type = p_test->uuid_type;
//    ble_uuid.uuid = COMMAND_TRANS_UUID_CHAR;
//
//    memset(&attr_md, 0, sizeof(attr_md));
//
//    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);
//    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.write_perm);
//    attr_md.vloc       = BLE_GATTS_VLOC_STACK;
//    attr_md.rd_auth    = 0;
//    attr_md.wr_auth    = 0;
//    attr_md.vlen       = 1;
//
//    memset(&attr_char_value, 0, sizeof(attr_char_value));
//
//    attr_char_value.p_uuid    = &ble_uuid;
//    attr_char_value.p_attr_md = &attr_md;
//    attr_char_value.init_len  = sizeof(uint8_t);
//    attr_char_value.init_offs = 0;
//    attr_char_value.max_len   = MAX_WRITE_NUM;//sizeof(uint8_t);
//    attr_char_value.p_value   = NULL;
//
//    return sd_ble_gatts_characteristic_add(p_test->service_handle, 
//                                               &char_md,
//                                               &attr_char_value,
//                                               &p_test->command_trans_handles);
}

/**
 ***************************************************************************************************
 *	@brief			CommandTransmission Characteristic Setting
 *	@details		Setting CommandTransmission Characteristic
 *      @param[in] p_test  SmartLock Service structure.
 **************************************************************************************************/
static uint32_t command_res_char_add(ble_test_t * p_test, const ble_test_init_t * p_test_init)
{

    uint32_t              err_code;
    ble_uuid_t            ble_uuid;
    ble_add_char_params_t add_char_params;

    memset(&add_char_params, 0, sizeof(add_char_params));
    add_char_params.uuid              = COMMAND_RES_UUID_CHAR;
    add_char_params.uuid_type         = p_test->uuid_type;
    add_char_params.init_len          = sizeof(uint8_t);
    add_char_params.is_var_len        = true;
    add_char_params.max_len           = MAX_WRITE_NUM;//sizeof(uint8_t);
    //add_char_params.char_props.read   = 1;
    add_char_params.char_props.notify = 1;

    //add_char_params.read_access       = SEC_OPEN;
    add_char_params.cccd_write_access = SEC_OPEN;

    return  characteristic_add(p_test->service_handle, 
                                  &add_char_params,
                                  &p_test->command_res_handles);
//
//    ble_gatts_char_md_t char_md;
//    ble_gatts_attr_md_t cccd_md;
//    ble_gatts_attr_t    attr_char_value;
//    ble_uuid_t          ble_uuid;
//    ble_gatts_attr_md_t attr_md;
//
//    memset(&cccd_md, 0, sizeof(cccd_md));
//
//    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.read_perm);
//    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.write_perm);
//    cccd_md.vloc       = BLE_GATTS_VLOC_STACK;
//
//    memset(&char_md, 0, sizeof(char_md));
//
//    char_md.char_props.read   = 1;
//    char_md.char_props.write  = 1;
//    char_md.char_props.notify = 1;
//    char_md.p_char_user_desc  = NULL;
//    char_md.p_char_pf         = NULL;
//    char_md.p_user_desc_md    = NULL;
//    char_md.p_cccd_md         =  &cccd_md;
//    char_md.p_sccd_md         = NULL;
//
//    ble_uuid.type = p_test->uuid_type;
//    ble_uuid.uuid = COMMAND_RES_UUID_CHAR;
//
//    memset(&attr_md, 0, sizeof(attr_md));
//
//    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);
//    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&attr_md.write_perm);
//    attr_md.vloc       = BLE_GATTS_VLOC_STACK;
//    attr_md.rd_auth    = 0;
//    attr_md.wr_auth    = 0;
//    attr_md.vlen       = 1;
//
//    memset(&attr_char_value, 0, sizeof(attr_char_value));
//
//    attr_char_value.p_uuid    = &ble_uuid;
//    attr_char_value.p_attr_md = &attr_md;
//    attr_char_value.init_len  = sizeof(uint8_t);
//    attr_char_value.init_offs = 0;
//    attr_char_value.max_len   = MAX_NOTIFY_NUM;//sizeof(uint8_t);
//    attr_char_value.p_value   = NULL;
//
//    return sd_ble_gatts_characteristic_add(p_test->service_handle,                       
//    &char_md,
//    &attr_char_value,
//    &p_test->command_res_handles);
}


/**
 ***************************************************************************************************
 *	@brief			SmartLock Service initialization
 *	@details		Initializing SmartLock Service
 *      @param[in] p_test  SmartLock Service structure.
 **************************************************************************************************/
uint32_t ble_test_init(ble_test_t * p_test, const ble_test_init_t * p_test_init)
{
    uint32_t              err_code;
    ble_uuid_t            ble_uuid;
    ble_add_char_params_t add_char_params;

    // Initialize service structure.
    p_test->test_write_handler = p_test_init->test_write_handler;

    // Add service.
    ble_uuid128_t base_uuid = {TEST_UUID_BASE};
    err_code = sd_ble_uuid_vs_add(&base_uuid, &p_test->uuid_type);
    VERIFY_SUCCESS(err_code);

    ble_uuid.type = p_test->uuid_type;
    ble_uuid.uuid = TEST_UUID_SERVICE;

    err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY, &ble_uuid, &p_test->service_handle);
    VERIFY_SUCCESS(err_code);

    // Add Lock State characteristic
    err_code = lock_state_char_add(p_test, p_test_init);
    VERIFY_SUCCESS(err_code);

    // Add CommandResponse characteristic
    err_code = command_res_char_add(p_test, p_test_init);
    VERIFY_SUCCESS(err_code);

    // Add Lock CommandTransmission characteristic
    err_code = command_trans_char_add(p_test, p_test_init);
    VERIFY_SUCCESS(err_code);

    return err_code;
}
//
//uint32_t ble_test_init(ble_test_t * p_test, const ble_test_init_t * p_test_init)
//{
//    uint32_t              err_code;
//    ble_uuid_t            ble_uuid;
//    ble_add_char_params_t add_char_params;
//
//    // Initialize service structure.
//    p_test->conn_handle       = BLE_CONN_HANDLE_INVALID;
//    p_test->test_write_handler = p_test_init->test_write_handler;
//    //p_test->p_test_state = p_test_init->p_test_state;
//
//    // Add service.
//    ble_uuid128_t base_uuid = {TEST_UUID_BASE};
//    err_code = sd_ble_uuid_vs_add(&base_uuid, &p_test->uuid_type);
//    VERIFY_SUCCESS(err_code);
//
//    ble_uuid.type = p_test->uuid_type;
//    ble_uuid.uuid = TEST_UUID_SERVICE;
//
//    err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY, &ble_uuid, &p_test->service_handle);
//    VERIFY_SUCCESS(err_code);
//
//    // Add Lock State characteristic
//    err_code = lock_state_char_add(p_test, p_test_init);
//    VERIFY_SUCCESS(err_code);
//
//    // Add CommandResponse characteristic
//    err_code = command_res_char_add(p_test, p_test_init);
//    VERIFY_SUCCESS(err_code);
//
//    // Add Lock CommandTransmission characteristic
//    err_code = command_trans_char_add(p_test, p_test_init);
//    VERIFY_SUCCESS(err_code);
//
//    return err_code;
//}

uint32_t ble_test_notify(ble_test_t * p_test, uint8_t *data, uint16_t len)
{
    ble_gatts_hvx_params_t params;

    memset(&params, 0, sizeof(params));
    params.type = BLE_GATT_HVX_NOTIFICATION;
    params.handle = p_test->command_res_handles.value_handle;
    params.p_data = data;
    params.p_len = &len;

    return sd_ble_gatts_hvx(p_test->conn_handle, &params);
}

/**
 ***************************************************************************************************
 *	@brief			BLE command Receive process
 *	@details		Checks the received data for errors, 
 *				stores the data in the command buffer, 
 *				and starts counting the data receive timer.
 **************************************************************************************************/ 
static bool ble_command_receive( const uint8_t *data, uint16_t len)
{
  // If the received data is less than 20 bytes, an error event occurs
//  if(len != 20)
//  {
//    return false;
//  }
//
//  // Store received data in command buffer
//  for(int i=0; i<len; i++)
//  {
//    write_buf[i] = data[i];
//  }
//
//  total_packets = data[0];
//  uint8_t packet_num = data[1];
//  
//  for(int i=0; i<len-2; i++)
//  {
//    ble_command[packet_num][i] = data[i+2];
//  }
//  
//  // If all packets cannot be received within 1 second, an error event occurs
//  packet_num_count |= (0x01 << packet_num); 
//
//  // If all packets are received, timer stop and reset counts.
//  if(true == check_packet_num_count()) {
//    sys_timer_flag[BLE_COMMNAD_ALL_PACKETS_TIMER] = false;
//    sys_timer_count[BLE_COMMNAD_ALL_PACKETS_TIMER] = 0;
//    //BLE Command Event //★：未実装
//  } else {
//    sys_timer_flag[BLE_COMMNAD_ALL_PACKETS_TIMER] = true;
//  }
}


/**@brief Function for handling write events to the LED characteristic.
 *
 * @param[in] p_test     Instance of LED Button Service to which the write applies.
 * @param[in] led_state Written/desired state of the LED.
 */
void test_write_handler(uint16_t conn_handle, ble_test_t * p_test, const uint8_t *receive_buf, uint8_t length)
{
  uint8_t  i;

  for(i=0;  i<length;  i++)
  {
      write_buf[i] = receive_buf[i];
  }                       
  write_len = length;
}


#endif // NRF_MODULE_ENABLED(BLE_TEST)
