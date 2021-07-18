/**
 ***************************************************************************************************
 * @file        test_service.c
 * @brief       Set Test Service
 ***************************************************************************************************
 **************************************************************************************************/
#include "sdk_common.h"
#if NRF_MODULE_ENABLED(BLE_TEST)
#include "test_service.h"
#include "ble_srv_common.h"
#include  "common.h"

/**@brief Function for handling the Connect event.
 *
 * @param[in]   p_test      TEST Service structure.
 * @param[in]   p_ble_evt   Event received from the BLE stack.
 */
static void on_connect(ble_test_t * p_test, ble_evt_t const * p_ble_evt)
{
    p_test->conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
}


/**@brief Function for handling the Disconnect event.
 *
 * @param[in]   p_test      Test Service structure.
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
 *      @param[in] p_test  Test Service structure.
 *      @param[in] p_ble_evt    Event received from the BLE stack.
 *      @param[in] data         Response data.
 **************************************************************************************************/
static void on_read(ble_test_t * p_test, ble_evt_t const * p_ble_evt, uint8_t* data, uint8_t data_size)
{
    uint32_t err_code;

    ble_gatts_rw_authorize_reply_params_t reply_params;

    memset(&reply_params, 0, sizeof(reply_params));
      
    reply_params.type = BLE_GATTS_AUTHORIZE_TYPE_READ;
    reply_params.params.read.p_data    = data;
    reply_params.params.read.len       = data_size;//sizeof(&data);//
    reply_params.params.read.offset    = 0;
    reply_params.params.read.update    = 1;
    reply_params.params.read.gatt_status = NRF_SUCCESS;
      
    sd_ble_gatts_rw_authorize_reply(p_ble_evt->evt.gap_evt.conn_handle, &reply_params);

}

/**
 ***************************************************************************************************
 *	@brief			on BLE EVENT function
 *	@details		Function for handling the BLE event.
 *      @param[in] p_ble_evt    Event received from the BLE stack.
 **************************************************************************************************/
void ble_test_on_ble_evt(ble_evt_t const * p_ble_evt, void * p_context)
{
    uint32_t err_code;
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
            break; 

        case BLE_GATTS_EVT_RW_AUTHORIZE_REQUEST:
            if ( p_ble_evt->evt.gatts_evt.params.authorize_request.type == BLE_GATTS_AUTHORIZE_TYPE_READ  )
            {
                if (p_ble_evt->evt.gatts_evt.params.authorize_request.request.read.handle == p_test->lock_state_handles.value_handle)
                {
                }
            }
            else if(p_ble_evt->evt.gatts_evt.params.authorize_request.type == BLE_GATTS_AUTHORIZE_TYPE_WRITE)
            {
            }
            break;

        case BLE_GATTS_EVT_HVC:
            ble_indicate_ack = true;
            ble_indicate_enable = true;
            break;


        default:
            // No implementation needed.
            break;
    }
}

/**
 ***************************************************************************************************
 *	@brief			Read Characteristic Setting
 *	@details		Setting Read Characteristic
 *      @param[in] p_test  Test Service structure.
 **************************************************************************************************/
static uint32_t read_char_add(ble_test_t * p_test, const ble_test_init_t * p_test_init)
{
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          ble_uuid;
    ble_gatts_attr_md_t attr_md;

    memset(&char_md, 0, sizeof(char_md));

    char_md.char_props.read   = 1;
    char_md.char_props.write  = 0;
    char_md.char_props.notify   = 0;
    char_md.char_props.indicate = 0;
    char_md.p_char_user_desc  = NULL;
    char_md.p_char_pf         = NULL;
    char_md.p_user_desc_md    = NULL;
    char_md.p_cccd_md         = NULL;
    char_md.p_sccd_md         = NULL;
    
    ble_uuid.type = p_test->uuid_type;
    ble_uuid.uuid = READ_UUID_CHAR;

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
    attr_char_value.max_len   = MAX_READ_NUM;//sizeof(uint8_t);
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
 *      @param[in] p_test  Test Service structure.
 **************************************************************************************************/
static uint32_t command_write_char_add(ble_test_t * p_test, const ble_test_init_t * p_test_init)
{
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          ble_uuid;
    ble_gatts_attr_md_t attr_md;

    memset(&char_md, 0, sizeof(char_md));

    char_md.char_props.read   = 0;
    char_md.char_props.write  = 1;
    char_md.char_props.notify = 0;
    char_md.char_props.indicate = 0;
    char_md.p_char_user_desc  = NULL;
    char_md.p_char_pf         = NULL;
    char_md.p_user_desc_md    = NULL;
    char_md.p_cccd_md         = NULL;
    char_md.p_sccd_md         = NULL;

    ble_uuid.type = p_test->uuid_type;
    ble_uuid.uuid = COMMAND_WRITE_UUID_CHAR;

    memset(&attr_md, 0, sizeof(attr_md));

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.write_perm);
    attr_md.vloc       = BLE_GATTS_VLOC_STACK;
    attr_md.rd_auth    = 0;
    attr_md.wr_auth    = 1;  //Write with Response
    attr_md.vlen       = 1;

    memset(&attr_char_value, 0, sizeof(attr_char_value));

    attr_char_value.p_uuid    = &ble_uuid;
    attr_char_value.p_attr_md = &attr_md;
    attr_char_value.init_len  = sizeof(uint8_t);
    attr_char_value.init_offs = 0;
    attr_char_value.max_len   = MAX_WRITE_NUM;//sizeof(uint8_t);
    attr_char_value.p_value   = NULL;

    return sd_ble_gatts_characteristic_add(p_test->service_handle, 
                                               &char_md,
                                               &attr_char_value,
                                               &p_test->command_trans_handles);
}

/**
 ***************************************************************************************************
 *	@brief			CommandTransmission Characteristic Setting
 *	@details		Setting CommandTransmission Characteristic
 *      @param[in] p_test  Test Service structure.
 **************************************************************************************************/
static uint32_t command_indication_char_add(ble_test_t * p_test, const ble_test_init_t * p_test_init)
{
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_md_t cccd_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          ble_uuid;
    ble_gatts_attr_md_t attr_md;

    memset(&cccd_md, 0, sizeof(cccd_md));

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.write_perm);
    cccd_md.vloc       = BLE_GATTS_VLOC_STACK;

    memset(&char_md, 0, sizeof(char_md));

    char_md.char_props.read   = 0;
    char_md.char_props.write  = 0;
    char_md.char_props.notify = 0;
    char_md.char_props.indicate = 1;
    char_md.p_char_user_desc  = NULL;
    char_md.p_char_pf         = NULL;
    char_md.p_user_desc_md    = NULL;
    char_md.p_cccd_md         =  &cccd_md;
    char_md.p_sccd_md         = NULL;

    ble_uuid.type = p_test->uuid_type;
    ble_uuid.uuid = COMMAND_INDICATION_UUID_CHAR;

    memset(&attr_md, 0, sizeof(attr_md));

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&attr_md.write_perm);
    attr_md.vloc       = BLE_GATTS_VLOC_STACK;
    attr_md.rd_auth    = 0;
    attr_md.wr_auth    = 0;
    attr_md.vlen       = 1;

    memset(&attr_char_value, 0, sizeof(attr_char_value));

    attr_char_value.p_uuid    = &ble_uuid;
    attr_char_value.p_attr_md = &attr_md;
    attr_char_value.init_len  = sizeof(uint8_t);
    attr_char_value.init_offs = 0;
    attr_char_value.max_len   = MAX_INDICATION_NUM;//sizeof(uint8_t);
    attr_char_value.p_value   = NULL;

    return sd_ble_gatts_characteristic_add(p_test->service_handle,                       
    &char_md,
    &attr_char_value,
    &p_test->command_res_handles);
}

/**
 ***************************************************************************************************
 *	@brief			Test Service initialization
 *	@details		Initializing Test Service
 *      @param[in] p_test  Test Service structure.
 **************************************************************************************************/
uint32_t ble_test_init(ble_test_t * p_test, const ble_test_init_t * p_test_init)
{
    uint32_t              err_code;
    ble_uuid_t            ble_uuid;
    ble_add_char_params_t add_char_params;

    // Initialize service structure.
    p_test->conn_handle       = BLE_CONN_HANDLE_INVALID;
    p_test->test_write_handler = p_test_init->test_write_handler;

    // Add service.
    ble_uuid128_t base_uuid = {TEST_UUID_BASE};
    err_code = sd_ble_uuid_vs_add(&base_uuid, &p_test->uuid_type);
    VERIFY_SUCCESS(err_code);

    ble_uuid.type = p_test->uuid_type;
    ble_uuid.uuid = TEST_UUID_SERVICE;

    err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY, &ble_uuid, &p_test->service_handle);
    VERIFY_SUCCESS(err_code);

    // Add read characteristic
    err_code = read_char_add(p_test, p_test_init);
    VERIFY_SUCCESS(err_code);

    // Add CommandResponse characteristic
    err_code = command_indication_char_add(p_test, p_test_init);
    VERIFY_SUCCESS(err_code);

    // Add CommandTransmission characteristic
    err_code = command_write_char_add(p_test, p_test_init);
    VERIFY_SUCCESS(err_code);

    return err_code;
}







#endif // NRF_MODULE_ENABLED(BLE_TEST)
