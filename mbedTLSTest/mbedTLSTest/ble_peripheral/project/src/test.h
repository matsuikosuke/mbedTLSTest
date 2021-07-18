/**
 ***************************************************************************************************
 * @file 		test.h
 * @brief 		TEST
 ***************************************************************************************************
 **************************************************************************************************/
#ifndef TEST_H_ /* TEST_H_ */
#define TEST_H_

/**
 * @brief	extern declaration
 */
#ifdef TEST_GLOBAL
#define TEST_EXTERN
#else
#define TEST_EXTERN extern
#endif

/*--------------------------------------------------------------------------------------------------
- Function Declaration
--------------------------------------------------------------------------------------------------*/
TEST_EXTERN int mbedtls_gcm_self_test(void);
TEST_EXTERN int mbedtls_ecdh_self_test(void);
TEST_EXTERN int mbedtls_ecdh_self_test2(void);
TEST_EXTERN void oberon_ecdh_self_test(void);
TEST_EXTERN void rsa_oeap_buf_clear(void);
TEST_EXTERN int mbedtls_rsa_rsaes_oaep_decrypt_test(void);
TEST_EXTERN int mbedtls_rsa_rsaes_oaep_encrypt_test(void);
TEST_EXTERN int mbedtls_rsa_rsaes_oaep_self_test(uint8_t key_length, uint32_t key_bits);
TEST_EXTERN int mbedtls_rsa_key_generation_test(uint32_t key_bits);
TEST_EXTERN int mbedtls_base64_self_test(void);
TEST_EXTERN int mbedtls_jwt_self_test(void);
TEST_EXTERN int mbedtls_hash_encrypt_test(void);
TEST_EXTERN int mbed_hkdf_test(void);


#endif /* TEST_H_ */
/***************************************************************************************************
 **************************************************************************************************/