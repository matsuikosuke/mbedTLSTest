#include "common.h"

/**@brief Function for application main entry.
 */
int main(void)
{
    ret_code_t err_code;
    ret_code_t ret_val;

    // Initialize.
    cpu_init();

    power_management_init();

    ble_init();

    // Start execution.
    advertising_start();

    // Timer start
    system_timer_start();
    sys_timer_flag[TEST_TIMER] = true;

   // AES-GCM
    mbedtls_gcm_self_test();

    // ECDH
    mbedtls_ecdh_self_test();
    oberon_ecdh_self_test();
    mbedtls_ecdh_self_test2();

    // HKDF
    mbed_hkdf_test();

    // RSA-OAEP
    mbedtls_rsa_rsaes_oaep_encrypt_test();
    mbedtls_rsa_rsaes_oaep_decrypt_test();
    rsa_oeap_buf_clear();

    // RSA-OAEP & RSA-KEY
    mbedtls_rsa_rsaes_oaep_self_test(SHORT_KEY, 2048); //256byte key
    rsa_oeap_buf_clear();
    mbedtls_rsa_rsaes_oaep_self_test(SHORT_KEY, 1024); //128byte key
    rsa_oeap_buf_clear();
    
    // RSA-KEY
    mbedtls_rsa_key_generation_test(2048); //256byte key
    rsa_oeap_buf_clear();

    // BASE64
    mbedtls_base64_self_test();

    // JWT validation
    mbedtls_jwt_self_test();

    // JWT Signature
//    mbedtls_hash_encrypt_test();

    // Enter main loop.
    for (;;)
    {
        WDT_RESTART();
        idle_state_handle();
    }
}


/**
 * @}
 */