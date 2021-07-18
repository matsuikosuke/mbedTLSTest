#include "common.h"
#include <stdint.h>
#include <string.h>

#include "mbedtls/gcm.h"

#include <stdio.h>
#include <stdarg.h>
#include <stddef.h>
#include "nrf_gpio.h"
#include "app_util.h"
#include "nrf_delay.h"
#include "nrf_error.h"
#include "nrf_section.h"
#include "nrf_crypto_rng.h"
#include "nrf_crypto_init.h"
#include "mem_manager.h"

  
#include "mbedtls/asn1.h"
#include "mbedtls/config.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/rsa.h"
#include "mbedtls/entropy.h"
#include "mbedtls/pk.h"

#include "mbedtls/platform.h"
#include "mbedtls/net.h"
#include "mbedtls/ssl.h"
#include "mbedtls/error.h"
#include "mbedtls/certs.h"

#include "mbedtls/hkdf.h"

#include "nrf_soc.h"
#include "nrf_rng.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#define PADDING_TYPE MBEDTLS_RSA_PKCS_V21
#define HASH_TYPE  MBEDTLS_MD_SHA1 //MBEDTLS_MD_SHA256 //


#include "nrf_crypto.h"

#include "mbedtls/base64.h"

#include "app_error.h"
#include "ocrypto_curve25519.h"  // For oberon ECDH

#include "time.h"


/**
 ***************************************************************************************************
 *	@brief			//
 *	@details		//
**************************************************************************************************/

///**
// ***************************************************************************************************
// *	@brief		//
// *	@details	//
//**************************************************************************************************/
static int entropy_dummy_source( void *data, unsigned char *output,
                                 size_t len, size_t *olen )
{
    ((void) data);

    memset( output, 0x6a, len );
    *olen = len;

    return( 0 );
}


/**
 ***************************************************************************************************
 *	@brief		//
 *	@details	//
 **************************************************************************************************/
#define CACHE_SIZE 32
static uint8_t cache[CACHE_SIZE] = {60};
static size_t cache_idx = 0;
static uint8_t rng_generate_uint8() {
    // Find out whether random data is available in the pool.
    uint8_t available;
    sd_rand_application_bytes_available_get(&available);

    if (available > 0) {
        // Generate a new random number and save it in the cache.
        const ret_code_t result = sd_rand_application_vector_get(cache + cache_idx, 1);
        APP_ERROR_CHECK(result);
    }

    // Return a number from the cache.
    const uint8_t ret = cache[cache_idx];
    cache_idx = (cache_idx + 1) % CACHE_SIZE;
    return ret;
}

static void generate_random(uint8_t *output, uint16_t output_len)
{
    nrf_delay_ms(10);
    for(int i=0;  i<output_len; i++)
    {
        *(output+i) = rng_generate_uint8();
    }
}

/**
 ***************************************************************************************************
 *	@brief		RANDOM NUMBER
 *	@details	//
 **************************************************************************************************/
static int entropy_source( void *data, unsigned char *output,
                                 size_t len, size_t *olen )
{
    ((void) data);

    generate_random(output, len);
    *olen = len;

    return( 0 );
}

/**
 ***************************************************************************************************
 *	@brief		ECDH
 *	@details	https://qiita.com/Kosuke_Matsui/items/4390f5e58339e95dcdae
**************************************************************************************************/
#define ECDH_KEY_SIZE (32)

int mbedtls_ecdh_self_test(void)
{
    int ret = 1;

    mbedtls_ecdh_context ctx_cli;
    mbedtls_ecdh_context ctx_srv;
    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;

    unsigned char cli_to_srv[ECDH_KEY_SIZE];
    unsigned char srv_to_cli[ECDH_KEY_SIZE];

    // ECDH Shared Key
    uint32_t ecdhSharedKey1[8] = {};
    uint32_t ecdhSharedKey2[8] = {};

    const char* pers = "ecdh";

    // initialize entropy
    mbedtls_entropy_init(&entropy);    
    mbedtls_ctr_drbg_init(&ctr_drbg); 
    mbedtls_entropy_add_source(&entropy, entropy_source, NULL,
                               GENERATE_KEY_ENTROPY_THRESHOLD, MBEDTLS_ENTROPY_SOURCE_STRONG);

    ret = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy,
                    (const unsigned char*)pers,
                    strlen(pers));
    if (ret != 0)
        goto exit;

    //initialize context and generate keypair
    mbedtls_ecdh_init( &ctx_cli );
    ret = mbedtls_ecp_group_load( &ctx_cli.grp, MBEDTLS_ECP_DP_CURVE25519 );
    if( ret != 0 )
        goto exit;

    ret = mbedtls_ecdh_gen_public( &ctx_cli.grp, &ctx_cli.d, &ctx_cli.Q,
                                   mbedtls_ctr_drbg_random, &ctr_drbg );
    if( ret != 0 )
        goto exit;

    //save Public Key
    ret = mbedtls_mpi_write_binary( &ctx_cli.Q.X, cli_to_srv, ECDH_KEY_SIZE );
    if( ret != 0 )
        goto exit;

    //initialize context and generate keypair
    mbedtls_ecdh_init( &ctx_srv );
    ret = mbedtls_ecp_group_load( &ctx_srv.grp, MBEDTLS_ECP_DP_CURVE25519 );
    if( ret != 0 )
        goto exit;

    ret = mbedtls_ecdh_gen_public( &ctx_srv.grp, &ctx_srv.d, &ctx_srv.Q,
                                   mbedtls_ctr_drbg_random, &ctr_drbg );
    if( ret != 0 )
        goto exit;     

    //save Public Key
    ret = mbedtls_mpi_write_binary( &ctx_srv.Q.X, srv_to_cli, ECDH_KEY_SIZE );
    if( ret != 0 )
        goto exit;

    //read peer's key and generate shared secret
    ret = mbedtls_mpi_lset( &ctx_srv.Qp.Z, 1 );
    if( ret != 0 )
        goto exit;

    //read Public Key
    ret = mbedtls_mpi_read_binary( &ctx_srv.Qp.X, cli_to_srv, ECDH_KEY_SIZE );
    if( ret != 0 )
        goto exit;

    // generate shared key
    ret = mbedtls_ecdh_compute_shared( &ctx_srv.grp, &ctx_srv.z,
                                       &ctx_srv.Qp, &ctx_srv.d,
                                       mbedtls_ctr_drbg_random, &ctr_drbg );
    if( ret != 0 )
        goto exit;

    //save shared key
     for (uint16_t i=0; i<ctx_srv.z.n; i++) {
         ecdhSharedKey1[i] = ctx_srv.z.p[i];
     }

    //read peer's key and generate shared secret
    ret = mbedtls_mpi_lset( &ctx_cli.Qp.Z, 1 );
    if( ret != 0 )
        goto exit;

    //read Public Key
    ret = mbedtls_mpi_read_binary( &ctx_cli.Qp.X, srv_to_cli, ECDH_KEY_SIZE );
    if( ret != 0 )
        goto exit;

    // generate shared key
    ret = mbedtls_ecdh_compute_shared( &ctx_cli.grp, &ctx_cli.z,
                                       &ctx_cli.Qp, &ctx_cli.d,
                                       mbedtls_ctr_drbg_random, &ctr_drbg );
    if( ret != 0 )
        goto exit;


    //save shared key
     for (uint16_t i=0; i<ctx_cli.z.n; i++) {
         ecdhSharedKey2[i] = ctx_cli.z.p[i];
     }

    printf("read mbedtls ecdhSharedKey1=");
     for (uint16_t i=0; i<8; i++) {
         printf("%02lX, ", ecdhSharedKey1[i]);
     }
     printf("\r\n");

    printf("read mbedtls ecdhSharedKey2=");
     for (uint16_t i=0; i<8; i++) {
         printf("%02lX, ", ecdhSharedKey2[i]);
     }
     printf("\r\n");

    //Verification: are the computed secrets equal?
    ret = mbedtls_mpi_cmp_mpi( &ctx_cli.z, &ctx_srv.z );
    if( ret != 0 )
        goto exit;

exit:
    mbedtls_ecdh_free( &ctx_srv );
    mbedtls_ecdh_free( &ctx_cli );
    mbedtls_ctr_drbg_free( &ctr_drbg );
    mbedtls_entropy_free( &entropy );

    return ret;
}

static int generate_ecdh_key(uint8_t * ecdh_public_key, uint8_t *ecdh_private_key)
{
    int ret = 1;
    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;
    mbedtls_ecdh_context ctx_srv;

    const char* pers = "ecdh";

    // initialize context and generate keypair
    mbedtls_ecdh_init( &ctx_srv );

    // initialize entropy
    mbedtls_entropy_init(&entropy);    
    mbedtls_ctr_drbg_init(&ctr_drbg); 
    mbedtls_entropy_add_source(&entropy,entropy_source, NULL,
                               GENERATE_KEY_ENTROPY_THRESHOLD, MBEDTLS_ENTROPY_SOURCE_STRONG);

    ret = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy,
                    (const unsigned char*)pers,
                    strlen(pers));
    if (ret != 0)
        goto exit;

    // initialize context and generate keypair
    ret = mbedtls_ecp_group_load( &ctx_srv.grp, MBEDTLS_ECP_DP_CURVE25519 );
    if( ret != 0 )
        goto exit;

    ret = mbedtls_ecdh_gen_public( &ctx_srv.grp, &ctx_srv.d, &ctx_srv.Q,
                                   mbedtls_ctr_drbg_random, &ctr_drbg );
    if( ret != 0 )
        goto exit;     

    // save Public Key
    ret = mbedtls_mpi_write_binary_le( &ctx_srv.Q.X, ecdh_public_key, ECDH_KEY_SIZE );
    if( ret != 0 )
        goto exit;

    // save Public Key
    ret = mbedtls_mpi_write_binary_le(&ctx_srv.d, ecdh_private_key, ECDH_KEY_SIZE );
    if( ret != 0 )
        goto exit;


     printf("write ecdh_public_key=");
     for (uint16_t i=0; i<ECDH_KEY_SIZE; i++) {
         printf("%02lX, ", ecdh_public_key[i]);
     }
     printf("\r\n");     

    printf("write ecdh_private_key=");
     for (uint16_t i=0; i<ECDH_KEY_SIZE; i++) {
         printf("%02lX, ", ecdh_private_key[i]);
     }
     printf("\r\n");

exit:
    // free context
    mbedtls_ecdh_free( &ctx_srv );
    mbedtls_ctr_drbg_free( &ctr_drbg );
    mbedtls_entropy_free( &entropy );

    return ret;
}

void oberon_ecdh_self_test(void)
{    
    // ECDH Shared Key
    uint8_t ecdhSharedKey1[32] = {};
    uint8_t ecdhSharedKey2[32] = {};

    uint8_t my_ecdh_public_key[ECDH_KEY_SIZE] = {0};
    uint8_t my_ecdh_private_key[ECDH_KEY_SIZE] = {0};

    uint8_t your_ecdh_public_key[ECDH_KEY_SIZE] = {0};
    uint8_t your_ecdh_private_key[ECDH_KEY_SIZE] = {0};

    generate_ecdh_key(my_ecdh_public_key, my_ecdh_private_key);
    generate_ecdh_key(your_ecdh_public_key, your_ecdh_private_key);

    // Compute curve25519
    ocrypto_curve25519_scalarmult(ecdhSharedKey1, my_ecdh_private_key, your_ecdh_public_key);
    ocrypto_curve25519_scalarmult(ecdhSharedKey2, your_ecdh_private_key, my_ecdh_public_key);

    printf("read oberon ecdhSharedKey1=");
     for (uint16_t i=0; i<ECDH_KEY_SIZE; i++) {
         printf("%02lX, ", ecdhSharedKey1[i]);
     }
     printf("\r\n");

    printf("read oberon ecdhSharedKey2=");
     for (uint16_t i=0; i<ECDH_KEY_SIZE; i++) {
         printf("%02lX, ", ecdhSharedKey2[i]);
     }
     printf("\r\n");
}

int mbedtls_ecdh_self_test2(void)
{
    int ret = 1;

    mbedtls_ecdh_context ctx_cli;
    mbedtls_ecdh_context ctx_srv;
    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;

    // ECDH Shared Key
    uint32_t ecdhSharedKey1[8] = {};
    uint32_t ecdhSharedKey2[8] = {};

    const char* pers = "ecdh";

    uint8_t my_ecdh_public_key[ECDH_KEY_SIZE] = {0};
    uint8_t my_ecdh_private_key[ECDH_KEY_SIZE] = {0};

    uint8_t your_ecdh_public_key[ECDH_KEY_SIZE] = {0};
    uint8_t your_ecdh_private_key[ECDH_KEY_SIZE] = {0};

    generate_ecdh_key(my_ecdh_public_key, my_ecdh_private_key);
    generate_ecdh_key(your_ecdh_public_key, your_ecdh_private_key);

    // initialize entropy
    mbedtls_entropy_init(&entropy);    
    mbedtls_ctr_drbg_init(&ctr_drbg); 
    mbedtls_entropy_add_source(&entropy, entropy_source, NULL,
                               GENERATE_KEY_ENTROPY_THRESHOLD, MBEDTLS_ENTROPY_SOURCE_STRONG);

    ret = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy,
                    (const unsigned char*)pers,
                    strlen(pers));
    if (ret != 0)
        goto exit;

    //initialize context
    mbedtls_ecdh_init( &ctx_cli );
    ret = mbedtls_ecp_group_load( &ctx_cli.grp, MBEDTLS_ECP_DP_CURVE25519 );
    if( ret != 0 )
        goto exit;

    //initialize context
    mbedtls_ecdh_init( &ctx_srv );
    ret = mbedtls_ecp_group_load( &ctx_srv.grp, MBEDTLS_ECP_DP_CURVE25519 );
    if( ret != 0 )
        goto exit;

    //read peer's key and generate shared secret
    ret = mbedtls_mpi_lset( &ctx_srv.Qp.Z, 1 );
    if( ret != 0 )
        goto exit;

    //read Public Key
    ret = mbedtls_mpi_read_binary_le( &ctx_srv.Qp.X, my_ecdh_public_key, ECDH_KEY_SIZE );
    if( ret != 0 )
        goto exit;

    //read Private Key
    ret = mbedtls_mpi_read_binary_le( &ctx_srv.d, your_ecdh_private_key, ECDH_KEY_SIZE );
    if( ret != 0 )
        goto exit;

    // generate shared key
    ret = mbedtls_ecdh_compute_shared( &ctx_srv.grp, &ctx_srv.z,
                                       &ctx_srv.Qp, &ctx_srv.d,
                                       mbedtls_ctr_drbg_random, &ctr_drbg );
    if( ret != 0 )
        goto exit;

    //save shared key
     for (uint16_t i=0; i<ctx_srv.z.n; i++) {
         ecdhSharedKey1[i] = ctx_srv.z.p[i];
     }

    //read peer's key and generate shared secret
    ret = mbedtls_mpi_lset( &ctx_cli.Qp.Z, 1 );
    if( ret != 0 )
        goto exit;

    //read Public Key
    ret = mbedtls_mpi_read_binary_le( &ctx_cli.Qp.X, your_ecdh_public_key, ECDH_KEY_SIZE );
    if( ret != 0 )
        goto exit;

    //read Private Key
    ret = mbedtls_mpi_read_binary_le( &ctx_cli.d, my_ecdh_private_key, ECDH_KEY_SIZE );
    if( ret != 0 )
        goto exit;

    // generate shared key
    ret = mbedtls_ecdh_compute_shared( &ctx_cli.grp, &ctx_cli.z,
                                       &ctx_cli.Qp, &ctx_cli.d,
                                       mbedtls_ctr_drbg_random, &ctr_drbg );
    if( ret != 0 )
        goto exit;


    //save shared key
     for (uint16_t i=0; i<ctx_cli.z.n; i++) {
         ecdhSharedKey2[i] = ctx_cli.z.p[i];
     }

    printf("read mbedtls ecdhSharedKey1=");
     for (uint16_t i=0; i<8; i++) {
         printf("%02lX, ", ecdhSharedKey1[i]);
     }
     printf("\r\n");

    printf("read mbedtls ecdhSharedKey2=");
     for (uint16_t i=0; i<8; i++) {
         printf("%02lX, ", ecdhSharedKey2[i]);
     }
     printf("\r\n");

    //step7: Verification: are the computed secrets equal?
    ret = mbedtls_mpi_cmp_mpi( &ctx_cli.z, &ctx_srv.z );
    if( ret != 0 )
        goto exit;

exit:
    mbedtls_ecdh_free( &ctx_srv );
    mbedtls_ecdh_free( &ctx_cli );
    mbedtls_ctr_drbg_free( &ctr_drbg );
    mbedtls_entropy_free( &entropy );

    return ret;
}



/*
int mbedtls_ecdh_test(void)
{
    int ret = 1;

    mbedtls_ecdh_context ctx_srv;
    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;

    // ECDH Shared Key
    uint32_t ecdhSharedKey1[8] = {};

    const char* pers = "ecdh";

    read_ecdh_private_key(my_ecdh_private_key);
    read_ecdh_public_key(your_ecdh_public_key);

    // initialize entropy
    mbedtls_entropy_init(&entropy);    
    mbedtls_ctr_drbg_init(&ctr_drbg); 
    mbedtls_entropy_add_source(&entropy, entropy_source, NULL,
                               GENERATE_KEY_ENTROPY_THRESHOLD, MBEDTLS_ENTROPY_SOURCE_STRONG);

    ret = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy,
                    (const unsigned char*)pers,
                    strlen(pers));
    if (ret != 0)
        goto exit;

    //initialize context
    mbedtls_ecdh_init( &ctx_srv );
    ret = mbedtls_ecp_group_load( &ctx_srv.grp, MBEDTLS_ECP_DP_CURVE25519 );
    if( ret != 0 )
        goto exit;

    //read peer's key and generate shared secret
    ret = mbedtls_mpi_lset( &ctx_srv.Qp.Z, 1 );
    if( ret != 0 )
        goto exit;

    //read Public Key
    ret = mbedtls_mpi_read_binary_le( &ctx_srv.Qp.X, my_ecdh_public_key, ECDH_KEY_SIZE );
    if( ret != 0 )
        goto exit;

    //read Private Key
    ret = mbedtls_mpi_read_binary_le( &ctx_srv.d, your_ecdh_private_key, ECDH_KEY_SIZE );
    if( ret != 0 )
        goto exit;

    // generate shared key
    ret = mbedtls_ecdh_compute_shared( &ctx_srv.grp, &ctx_srv.z,
                                       &ctx_srv.Qp, &ctx_srv.d,
                                       mbedtls_ctr_drbg_random, &ctr_drbg );
    if( ret != 0 )
        goto exit;

    //save shared key
     for (uint16_t i=0; i<ctx_srv.z.n; i++) {
         ecdhSharedKey1[i] = ctx_srv.z.p[i];
     }

exit:
    mbedtls_ecdh_free( &ctx_srv );
    mbedtls_ctr_drbg_free( &ctr_drbg );
    mbedtls_entropy_free( &entropy );

    return ret;
}


void oberon_ecdh_test(void)
{    
    // ECDH Shared Key
    uint8_t ecdhSharedKey1[32] = {};

    uint8_t my_ecdh_private_key[ECDH_KEY_SIZE] = {0};
    uint8_t your_ecdh_public_key[ECDH_KEY_SIZE] = {0};

    read_ecdh_private_key(my_ecdh_private_key);
    read_ecdh_public_key(your_ecdh_public_key);

    // Compute curve25519
    ocrypto_curve25519_scalarmult(ecdhSharedKey1, my_ecdh_private_key, your_ecdh_public_key);
}
*/
/**
***************************************************************************************************
 *	@brief		AES-GCM
 *	@details	https://qiita.com/Kosuke_Matsui/items/340c6b906fed2c6ea40b
 **************************************************************************************************/ 
#define GCM_LENGTH (512)

static const unsigned char key[32] =
{
    0xfe, 0xff, 0xe9, 0x92, 0x86, 0x65, 0x73, 0x1c,
    0x6d, 0x6a, 0x8f, 0x94, 0x67, 0x30, 0x83, 0x08,
    0xfe, 0xff, 0xe9, 0x92, 0x86, 0x65, 0x73, 0x1c,
    0x6d, 0x6a, 0x8f, 0x94, 0x67, 0x30, 0x83, 0x08 
};

static unsigned char iv[64] =
{
    0x93, 0x13, 0x22, 0x5d, 0xf8, 0x84, 0x06, 0xe5,
    0x55, 0x90, 0x9c, 0x5a, 0xff, 0x52, 0x69, 0xaa,
    0x6a, 0x7a, 0x95, 0x38, 0x53, 0x4f, 0x7d, 0xa1,
    0xe4, 0xc3, 0x03, 0xd2, 0xa3, 0x18, 0xa7, 0x28,
    0xc3, 0xc0, 0xc9, 0x51, 0x56, 0x80, 0x95, 0x39,
    0xfc, 0xf0, 0xe2, 0x42, 0x9a, 0x6b, 0x52, 0x54,
    0x16, 0xae, 0xdb, 0xf5, 0xa0, 0xde, 0x6a, 0x57,
    0xa6, 0x37, 0xb3, 0x9b, 0xb0, 0xee, 0x7a, 0x67
};
static int iv_len = 64;

static unsigned char input[GCM_LENGTH] =
{
  0xd9, 0x31, 0x32, 0x25, 0xf8, 0x84, 0x06, 0xe5,
  0xa5, 0x59, 0x09, 0xc5, 0xaf, 0xf5, 0x26, 0x9a,
  0x86, 0xa7, 0xa9, 0x53, 0x15, 0x34, 0xf7, 0xda,
  0x2e, 0x4c, 0x30, 0x3d, 0x8a, 0x31, 0x8a, 0x72,
  0x1c, 0x3c, 0x0c, 0x95, 0x95, 0x68, 0x09, 0x53,
  0x2f, 0xcf, 0x0e, 0x24, 0x49, 0xa6, 0xb5, 0x25,
  0xb1, 0x6a, 0xed, 0xf5, 0xaa, 0x0d, 0xe6, 0x57,
  0xba, 0x63, 0x7b, 0x39, 0x1a, 0xaf, 0xd2, 0x55,
  0xd9, 0x31, 0x32, 0x25, 0xf8, 0x84, 0x06, 0xe5,
  0xa5, 0x59, 0x09, 0xc5, 0xaf, 0xf5, 0x26, 0x9a,
  0x86, 0xa7, 0xa9, 0x53, 0x15, 0x34, 0xf7, 0xda,
  0x2e, 0x4c, 0x30, 0x3d, 0x8a, 0x31, 0x8a, 0x72,
  0x1c, 0x3c, 0x0c, 0x95, 0x95, 0x68, 0x09, 0x53,
  0x2f, 0xcf, 0x0e, 0x24, 0x49, 0xa6, 0xb5, 0x25,
  0xb1, 0x6a, 0xed, 0xf5, 0xaa, 0x0d, 0xe6, 0x57,
  0xba, 0x63, 0x7b, 0x39, 0x1a, 0xaf, 0xd2, 0x55,
  0xd9, 0x31, 0x32, 0x25, 0xf8, 0x84, 0x06, 0xe5,
  0xa5, 0x59, 0x09, 0xc5, 0xaf, 0xf5, 0x26, 0x9a,
  0x86, 0xa7, 0xa9, 0x53, 0x15, 0x34, 0xf7, 0xda,
  0x2e, 0x4c, 0x30, 0x3d, 0x8a, 0x31, 0x8a, 0x72,
  0x1c, 0x3c, 0x0c, 0x95, 0x95, 0x68, 0x09, 0x53,
  0x2f, 0xcf, 0x0e, 0x24, 0x49, 0xa6, 0xb5, 0x25,
  0xb1, 0x6a, 0xed, 0xf5, 0xaa, 0x0d, 0xe6, 0x57,
  0xba, 0x63, 0x7b, 0x39, 0x1a, 0xaf, 0xd2, 0x55,
  0xd9, 0x31, 0x32, 0x25, 0xf8, 0x84, 0x06, 0xe5,
  0xa5, 0x59, 0x09, 0xc5, 0xaf, 0xf5, 0x26, 0x9a,
  0x86, 0xa7, 0xa9, 0x53, 0x15, 0x34, 0xf7, 0xda,
  0x2e, 0x4c, 0x30, 0x3d, 0x8a, 0x31, 0x8a, 0x72,
  0x1c, 0x3c, 0x0c, 0x95, 0x95, 0x68, 0x09, 0x53,
  0x2f, 0xcf, 0x0e, 0x24, 0x49, 0xa6, 0xb5, 0x25,
  0xb1, 0x6a, 0xed, 0xf5, 0xaa, 0x0d, 0xe6, 0x57,
  0xba, 0x63, 0x7b, 0x39, 0x1a, 0xaf, 0xd2, 0x55,
  0xd9, 0x31, 0x32, 0x25, 0xf8, 0x84, 0x06, 0xe5,
  0xa5, 0x59, 0x09, 0xc5, 0xaf, 0xf5, 0x26, 0x9a,
  0x86, 0xa7, 0xa9, 0x53, 0x15, 0x34, 0xf7, 0xda,
  0x2e, 0x4c, 0x30, 0x3d, 0x8a, 0x31, 0x8a, 0x72,
  0x1c, 0x3c, 0x0c, 0x95, 0x95, 0x68, 0x09, 0x53,
  0x2f, 0xcf, 0x0e, 0x24, 0x49, 0xa6, 0xb5, 0x25,
  0xb1, 0x6a, 0xed, 0xf5, 0xaa, 0x0d, 0xe6, 0x57,
  0xba, 0x63, 0x7b, 0x39, 0x1a, 0xaf, 0xd2, 0x55,
  0xd9, 0x31, 0x32, 0x25, 0xf8, 0x84, 0x06, 0xe5,
  0xa5, 0x59, 0x09, 0xc5, 0xaf, 0xf5, 0x26, 0x9a,
  0x86, 0xa7, 0xa9, 0x53, 0x15, 0x34, 0xf7, 0xda,
  0x2e, 0x4c, 0x30, 0x3d, 0x8a, 0x31, 0x8a, 0x72,
  0x1c, 0x3c, 0x0c, 0x95, 0x95, 0x68, 0x09, 0x53,
  0x2f, 0xcf, 0x0e, 0x24, 0x49, 0xa6, 0xb5, 0x25,
  0xb1, 0x6a, 0xed, 0xf5, 0xaa, 0x0d, 0xe6, 0x57,
  0xba, 0x63, 0x7b, 0x39, 0x1a, 0xaf, 0xd2, 0x55,
  0xd9, 0x31, 0x32, 0x25, 0xf8, 0x84, 0x06, 0xe5,
  0xa5, 0x59, 0x09, 0xc5, 0xaf, 0xf5, 0x26, 0x9a,
  0x86, 0xa7, 0xa9, 0x53, 0x15, 0x34, 0xf7, 0xda,
  0x2e, 0x4c, 0x30, 0x3d, 0x8a, 0x31, 0x8a, 0x72,
  0x1c, 0x3c, 0x0c, 0x95, 0x95, 0x68, 0x09, 0x53,
  0x2f, 0xcf, 0x0e, 0x24, 0x49, 0xa6, 0xb5, 0x25,
  0xb1, 0x6a, 0xed, 0xf5, 0xaa, 0x0d, 0xe6, 0x57,
  0xba, 0x63, 0x7b, 0x39, 0x1a, 0xaf, 0xd2, 0x55,
  0xd9, 0x31, 0x32, 0x25, 0xf8, 0x84, 0x06, 0xe5,
  0xa5, 0x59, 0x09, 0xc5, 0xaf, 0xf5, 0x26, 0x9a,
  0x86, 0xa7, 0xa9, 0x53, 0x15, 0x34, 0xf7, 0xda,
  0x2e, 0x4c, 0x30, 0x3d, 0x8a, 0x31, 0x8a, 0x72,
  0x1c, 0x3c, 0x0c, 0x95, 0x95, 0x68, 0x09, 0x53,
  0x2f, 0xcf, 0x0e, 0x24, 0x49, 0xa6, 0xb5, 0x25,
  0xb1, 0x6a, 0xed, 0xf5, 0xaa, 0x0d, 0xe6, 0x57,
  0xba, 0x63, 0x7b, 0x39, 0x1a, 0xaf, 0xd2, 0x55
};
static int inout_len = GCM_LENGTH;


int mbedtls_gcm_self_test(void)
{
    mbedtls_gcm_context ctx;
    unsigned char output[GCM_LENGTH] = {0};
    unsigned char decrypt_input[GCM_LENGTH] = {0};
    unsigned char tag_buf[16];
    int ret;
    mbedtls_cipher_id_t cipher = MBEDTLS_CIPHER_ID_AES;
    int key_len = 256; //128 or 196 or 256

    // ENCRYPT
    // ENCRYPT step1: init the context
    mbedtls_gcm_init( &ctx );

    // ENCRYPT step2: set the key
    ret = mbedtls_gcm_setkey( &ctx, cipher, key,  key_len );
    if( ret != 0 )
        goto exit;

    // ENCRYPT step3: Initialise the GCM cipher
    ret = mbedtls_gcm_starts( &ctx, MBEDTLS_GCM_ENCRYPT, iv, iv_len, NULL, 0);
    if( ret != 0 )
        goto exit;

    // ENCRYPT step4: Send the intialised cipher some data and store it
    ret = mbedtls_gcm_update( &ctx, inout_len, input, output );
    if( ret != 0 )
        goto exit;

    // ENCRYPT step5: write the 16-byte auth tag that's appended to the end
    ret = mbedtls_gcm_finish( &ctx, tag_buf, 16 );
    if( ret != 0 )
        goto exit;

    // ENCRYPT step6: Free up the context
    mbedtls_gcm_free( &ctx );

    // DECRYPT
    // DECRYPT step1: init the context
    mbedtls_gcm_init( &ctx );

    // DECRYPT step2: set the key
    ret = mbedtls_gcm_setkey( &ctx, cipher, key,  key_len );
    if( ret != 0 )
        goto exit;

    // DECRYPT step3: Initialise the GCM cipher
    ret = mbedtls_gcm_starts( &ctx, MBEDTLS_GCM_DECRYPT, iv, iv_len, NULL, 0);
    if( ret != 0 )
        goto exit;

    // DECRYPT step4: Send the intialised cipher some data and store it
    ret = mbedtls_gcm_update( &ctx, inout_len, output, decrypt_input );
    if( ret != 0 )
        goto exit;

    // DECRYPT step5: write the 16-byte auth tag that's appended to the end
    ret = mbedtls_gcm_finish( &ctx, tag_buf, 16 );
    if( ret != 0 )
        goto exit;

    // DECRYPT step6: Check result
    if( memcmp( input, decrypt_input, GCM_LENGTH ) != 0 )
    {
        ret = 1;
        printf("AES-GCM error\r\n");
        goto exit;
    }
    printf("AES-GCM success\r\n");

    // DECRYPT step7: Free up the context
    mbedtls_gcm_free( &ctx );

exit:
    if( ret != 0 )
        mbedtls_gcm_free( &ctx );

    return ret;
}

/**
***************************************************************************************************
 *	@brief		HKDF	
 *	@details	https://qiita.com/Kosuke_Matsui/items/93f25f47a6171fe94d30	
 **************************************************************************************************/ 
int mbed_hkdf_test(void)
{
    nrf_crypto_hmac_context_t   context;
    unsigned char hkdf_ikm_len_22[22]   = { 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b };
    unsigned char expected_hkdf_okm[42]   = { 0x8d, 0xa4, 0xe7, 0x75, 0xa5, 0x63, 0xc1, 0x8f, 0x71, 0x5f, 0x80, 0x2a, 0x06, 0x3c, 0x5a, 0x31, 0xb8, 0xa1, 0x1f, 0x5c, 0x5e, 0xe1, 0x87, 0x9e, 0xc3, 0x45, 0x4e, 0x5f, 0x3c, 0x73, 0x8d, 0x2d, 0x9d, 0x20, 0x13, 0x95, 0xfa, 0xa4, 0xb6, 0x1a, 0x96, 0xc8 };
    nrf_crypto_hmac_context_t   hkdf_context;

    uint32_t i;
    int ret;
    size_t   ikm_len = 22;//sizeof(hkdf_ikm_len_22);
    size_t   okm_len = sizeof(expected_hkdf_okm);
    size_t   expected_okm_len = okm_len;

    unsigned char hkdf_okm[42];
    memset(hkdf_okm,  0xFF, sizeof(hkdf_okm));

    ret = mbedtls_hkdf(
        mbedtls_md_info_from_type(MBEDTLS_MD_SHA256),
        NULL, //const unsigned char *   salt
        0, //size_t     salt_len
        &hkdf_ikm_len_22[0], //const unsigned char *    ikm,
        22, //size_t    ikm_len,
        NULL, //const unsigned char *   info,
        0, //size_t     info_len,
        &hkdf_okm[0], //unsigned char *     okm,
        okm_len //size_t    okm_len 
    );

    printf("hkdf_okm=");
     for (uint16_t i=0; i<okm_len; i++) {
         printf("%02lX, ", hkdf_okm[i]);
     }
     printf("\r\n");

    //Verification: are the computed hkdf and the expected hkdf equal?
    ret = memcmp(&hkdf_okm, &expected_hkdf_okm, okm_len);
    if( ret != 0 )
        printf("hkdf error\r\n");
    printf("hkdf success\r\n");

    return ret;
}


/**
 ***************************************************************************************************
 *	@brief		RSA-OAEP
 *	@details	https://qiita.com/Kosuke_Matsui/items/4c64c993e5979a65620d
**************************************************************************************************/
// 256 byte
#define TEST_PRIVATE_KEY \
"-----BEGIN PRIVATE KEY-----\r\n"  \
"MIIEvAIBADANBgkqhkiG9w0BAQEFAASCBKYwggSiAgEAAoIBAQCNOyZsRMQ4Kibt\r\n"  \
"HOwC+vGD4Y0yRiYgX7OsxtgJlvlJZQV9Jm5ShlFP9eabkMO5Fy/5dFQp4ubunD5M\r\n"  \
"nItxaBIT4cW5Tth/+Xc/MeKGxtDaO2fBN6RYAML4Uecn5EYryhjlyXnDx7Lf0DpZ\r\n"  \
"13X+DrW9DTx9Rgq6K0MxKE8BQup8H8AM5vJ2w1hVo7miQZg/+PkdqXKn/geoPuvf\r\n"  \
"aDpBTFWw6sNmnXuIs0YlGY0ICPMyM0N1MXd+3GrgXAY5CR7H/zE8+Hi0aAxYLNke\r\n"  \
"6dt2lhF+VCCBgu8SJ/08lfuIwpstjV7+6AgYXl7JhafQ+WqF1lYaBI5Ry7Nv9G+D\r\n"  \
"DLi9eXyhAgMBAAECggEAOjMY518DOV3tgqL7g1LkOgO1yvTre0XU5itfFbyYaC1w\r\n"  \
"MJ2osutVEM11xb/HWDv88iCFGmm93rCWX73XHNnVMUpFz9J1F2+3O1yUTBQcNwJq\r\n"  \
"BE6os/dEYU5nIqJIZH04+2CmhA6EYQUPNry6jAKF1C1jMwCtJQD1BInm3M81kAZs\r\n"  \
"teNVy/8Q2QtrM+B6GyhXzBEXB8bypL4PPHHilPsbcFmjooytOxJH4ybhclKsJYP0\r\n"  \
"ZidjoJc2B7lV3Mxnz9C45ALwygqFATsIAbRAQXX2WcRGcE+qlgav5QDUcOG30Uyb\r\n"  \
"uEdKmahtJbpKuDQv/GSnLxCGq7uO3PKYMqlJDU50vQKBgQDD8qUDmlXCXZajYAcI\r\n"  \
"GgChvYwkaBFfBOWMiiTg7Z389mwF4nRqwPapF9qreK93UEUBP6O24dcuyxGGF2yD\r\n"  \
"FClHPuFNo74xcwGvYaxHz0QqqsBVhexH0P2MlzaQAabX/HO+12joBIPXR5p+vWGP\r\n"  \
"NzAVKz0HQXlkNZg2/zZX+i3vPwKBgQC4g6A+KzzsByh9C+N4T+HPYq8yR0LsYMy5\r\n"  \
"DBflltozuj91Xp7anpVe5zVgn9Ua2n0bu5k+OFhjZxxI4uLyPm7N0sq7NAu3UDSV\r\n"  \
"O95NsQ/yyqRjg39r1V4jreMz431QX0mPk5hjV1gSRPu3UGbqJ8UXyXaWtg9hxgxb\r\n"  \
"zY+I0zl8HwKBgEjGEWspIrixMYUz4OitX5ayYx4SWIFFvEtZ8yBmn8qXar80v6/2\r\n"  \
"QqXCakBM7j4N3dL847zEW7rRSIzYnkCAeGQxglPngHe1EuckC2wQGm0ORah9uKdj\r\n"  \
"gwd4EZeiRWjqJcK3Fgs1whFuUn7HrzgVcxIpoNzVAFSrYwDqADDkhbLZAoGAEOjP\r\n"  \
"KKNhw+esCDmssPvxH8ZVc/Af6/W7DUJUdk/q007APgtb14EtAzOBKse7Cej4CjCZ\r\n"  \
"DKCxSPrMFsnkLnsWQHqO5bURVc1d5EdGSixt37w+cHg+ly1IoVBwP/MbKuBwp13J\r\n"  \
"W2FHyvTTdKtDozXd1B48ZfdtVXKG82rVOp7Q59UCgYB22DUA5GYps5Qg1YsvV9K7\r\n"  \
"S9zI7rhg0fK/OlyG8dCGFvMpBMRwBtt0r6MJ9dQA+lPz0Cp8h+5xsxq05stbdEPX\r\n"  \
"Htd2KHeBSuim8aJI5RGvIhVSkygaZz5cp5WjK5iOZqRfonDenfj9o8lVj1RYLd1i\r\n"  \
"QqAbVBfsxu9P/yj8Sb23aQ==\r\n"  \
"-----END PRIVATE KEY-----\r\n"

// 256 byte
#define TEST_PUBLIC_KEY \
"-----BEGIN PUBLIC KEY-----\r\n"  \
"MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAjTsmbETEOCom7RzsAvrx\r\n"  \
"g+GNMkYmIF+zrMbYCZb5SWUFfSZuUoZRT/Xmm5DDuRcv+XRUKeLm7pw+TJyLcWgS\r\n"  \
"E+HFuU7Yf/l3PzHihsbQ2jtnwTekWADC+FHnJ+RGK8oY5cl5w8ey39A6Wdd1/g61\r\n"  \
"vQ08fUYKuitDMShPAULqfB/ADObydsNYVaO5okGYP/j5Halyp/4HqD7r32g6QUxV\r\n"  \
"sOrDZp17iLNGJRmNCAjzMjNDdTF3ftxq4FwGOQkex/8xPPh4tGgMWCzZHunbdpYR\r\n"  \
"flQggYLvEif9PJX7iMKbLY1e/ugIGF5eyYWn0PlqhdZWGgSOUcuzb/Rvgwy4vXl8\r\n"  \
"oQIDAQAB\r\n"  \
"-----END PUBLIC KEY-----\r\n"

static char g_plaintext_56[56] = "===Hello! This is plaintext! ABCDEFGHIJKLMNQWSXCVGYZZ===";
static char g_plaintext_120[120] = "===Hello! This is plaintext! This has 120 characters. ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJK===";

void rsa_oeap_buf_clear(void)
{  
    for(int i=0; i<256; i++)
    {
        rsa_oeap_decrypted_buf[i] = 0;
        rsa_oeap_encrypted_buf[i] = 0;
    }
}


int mbedtls_rsa_rsaes_oaep_encrypt_test(void)
{  
    int ret = 1;

    size_t olen;
    mbedtls_pk_context p_pk;
    mbedtls_rsa_context *p_rsa;
    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;
    const char* pers = "my_app_specific_string";

    mbedtls_entropy_init(&entropy);    
    mbedtls_ctr_drbg_init(&ctr_drbg); 
    mbedtls_entropy_add_source(&entropy, entropy_source, NULL,
                               GENERATE_KEY_ENTROPY_THRESHOLD, MBEDTLS_ENTROPY_SOURCE_STRONG);

    ret = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy,
                    (const unsigned char*)pers,
                    strlen(pers));
   if (ret != 0)
        return ret;

    mbedtls_pk_init(&p_pk);

    ret = mbedtls_pk_parse_public_key(&p_pk, TEST_PUBLIC_KEY, strlen(TEST_PUBLIC_KEY) + 1);
    if (ret != 0)
        return ret;
 
    mbedtls_rsa_init(p_rsa, PADDING_TYPE, HASH_TYPE);
    //mbedtls_sha1_init(p_rsa);

    p_rsa = mbedtls_pk_rsa( p_pk );
    mbedtls_rsa_set_padding(p_rsa, PADDING_TYPE, HASH_TYPE);

    // The output buffer must be as large as the size of ctx->N
    //unsigned char output[mbedtls_mpi_size(&p_rsa->N)];
    ret = mbedtls_rsa_rsaes_oaep_encrypt(p_rsa, 
                        mbedtls_ctr_drbg_random, &ctr_drbg,
                        MBEDTLS_RSA_PUBLIC, NULL, 0,
                        120, &g_plaintext_120[0], &rsa_oeap_encrypted_buf[0]);
   if (ret != 0)
        return ret;

    // free context
    mbedtls_entropy_free(&entropy); 
    mbedtls_ctr_drbg_free(&ctr_drbg); 
    mbedtls_rsa_free(p_rsa);
    mbedtls_pk_free(&p_pk);

    return ret;
}


int mbedtls_rsa_rsaes_oaep_decrypt_test(void)
{
    int ret = 1;

    size_t olen;
    mbedtls_pk_context p_pk;
    mbedtls_rsa_context *p_rsa;
    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;
    const char* pers = "my_app_specific_string";

    mbedtls_entropy_init(&entropy);    
    mbedtls_ctr_drbg_init(&ctr_drbg); 
    mbedtls_entropy_add_source(&entropy, entropy_source, NULL,
                               GENERATE_KEY_ENTROPY_THRESHOLD, MBEDTLS_ENTROPY_SOURCE_STRONG);

    // Initialize entropy.
    ret = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy,
                    (const unsigned char*)pers,
                    strlen(pers));
   if (ret != 0)
        return ret;

    mbedtls_rsa_init(p_rsa, PADDING_TYPE, HASH_TYPE);
    mbedtls_pk_init(&p_pk);


    ret = mbedtls_pk_parse_key(&p_pk, TEST_PRIVATE_KEY, strlen(TEST_PRIVATE_KEY) + 1, NULL, NULL);
    if (ret != 0)
        return ret;

    p_rsa = mbedtls_pk_rsa( p_pk );
    mbedtls_rsa_set_padding(p_rsa, PADDING_TYPE, HASH_TYPE);


    // The output buffer must be as large as the size of ctx->N
    //unsigned char output[mbedtls_mpi_size(&p_rsa->N)];
ret = mbedtls_rsa_rsaes_oaep_decrypt(p_rsa, 
                        mbedtls_ctr_drbg_random, &ctr_drbg,
                        MBEDTLS_RSA_PRIVATE, NULL, 0, // label = NULL and label_len = 0
                        &olen, rsa_oeap_encrypted_buf, rsa_oeap_decrypted_buf, 120);
    if (ret != 0)
        return ret;    
        
    //Verification: are the computed hkdf and the expected hkdf equal?
    ret = memcmp(&g_plaintext_120, &rsa_oeap_decrypted_buf, 120);
    if( ret != 0 )
        printf("RSA-OAEP error\r\n");
    printf("RSA-OAEP success\r\n");

    // free context
    mbedtls_entropy_free(&entropy); 
    mbedtls_ctr_drbg_free(&ctr_drbg); 
    mbedtls_rsa_free(p_rsa);
    mbedtls_pk_free(&p_pk);
    return ret;
}

int mbedtls_rsa_rsaes_oaep_self_test(uint8_t key_length, uint32_t key_bits)
{
  
    int ret = 1;

    size_t olen;
    mbedtls_pk_context p_pk;
    mbedtls_rsa_context *p_rsa;
    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;
    const char* pers = "my_app_specific_string";

    sys_timer_count[TEST_TIMER] = 0;
    mbedtls_entropy_init(&entropy);    
    mbedtls_ctr_drbg_init(&ctr_drbg); 
    mbedtls_entropy_add_source(&entropy, entropy_source, NULL,
                               GENERATE_KEY_ENTROPY_THRESHOLD, MBEDTLS_ENTROPY_SOURCE_STRONG);

    // Initialize entropy.
    ret = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy,
                    (const unsigned char*)pers,
                    strlen(pers));
    if (ret != 0)
        return ret;
    printf("Execution time of mbedtls_ctr_drbg_seed %d ms\n",sys_timer_count[TEST_TIMER]*50);
    sys_timer_count[TEST_TIMER] = 0;

    mbedtls_rsa_init(p_rsa, PADDING_TYPE, HASH_TYPE);
    mbedtls_pk_init(&p_pk);
    ret = mbedtls_pk_setup( &p_pk, mbedtls_pk_info_from_type( MBEDTLS_PK_RSA ) );    
    if (ret != 0)
        return ret;
    printf("Execution time of mbedtls_pk_setup %d ms\n",sys_timer_count[TEST_TIMER]*50);
    sys_timer_count[TEST_TIMER] = 0;
                
    p_rsa = mbedtls_pk_rsa( p_pk );
    ret = mbedtls_rsa_gen_key(p_rsa, mbedtls_ctr_drbg_random, &ctr_drbg, key_bits, 0x010001);
    if (ret != 0)
        return ret;
    printf("Execution time of mbedtls_rsa_gen_key %d ms\n",sys_timer_count[TEST_TIMER]*50);
    sys_timer_count[TEST_TIMER] = 0;

    //mbedtls_rsa_context *rsa = mbedtls_pk_rsa( p_pk );
    mbedtls_rsa_set_padding(p_rsa, PADDING_TYPE, HASH_TYPE);

    if (LONG_KEY == key_length)
    {
        ret = mbedtls_rsa_rsaes_oaep_encrypt(p_rsa, 
                            mbedtls_ctr_drbg_random, &ctr_drbg,
                            MBEDTLS_RSA_PUBLIC, NULL, 0,
                            key_length, &g_plaintext_120[0], &rsa_oeap_encrypted_buf[0]);
    } else {
        ret = mbedtls_rsa_rsaes_oaep_encrypt(p_rsa, 
                            mbedtls_ctr_drbg_random, &ctr_drbg,
                            MBEDTLS_RSA_PUBLIC, NULL, 0,
                            key_length, &g_plaintext_56[0], &rsa_oeap_encrypted_buf[0]);
    }
    if (ret != 0)
    return ret;
    printf("Execution time of mbedtls_rsa_rsaes_oaep_encrypt %d ms\n",sys_timer_count[TEST_TIMER]*50);
    sys_timer_count[TEST_TIMER] = 0;


    ret = mbedtls_rsa_rsaes_oaep_decrypt(p_rsa, 
                        mbedtls_ctr_drbg_random, &ctr_drbg,
                        MBEDTLS_RSA_PRIVATE, NULL, 0, // label = NULL and label_len = 0
                        &olen, rsa_oeap_encrypted_buf, rsa_oeap_decrypted_buf, key_length);
    if (ret != 0)
        return ret;
    printf("Execution time of mbedtls_rsa_rsaes_oaep_decrypt %d ms\n",sys_timer_count[TEST_TIMER]*50);
    sys_timer_count[TEST_TIMER] = 0;

    //Verification: are the computed hkdf and the expected hkdf equal?
    if (LONG_KEY == key_length)
      ret = memcmp(&g_plaintext_120, &rsa_oeap_decrypted_buf, key_length);
    else
        ret = memcmp(&g_plaintext_56, &rsa_oeap_decrypted_buf, key_length);
    if( ret != 0 )
        printf("RSA-OAEP error\r\n");
    printf("RSA-OAEP success\r\n");

    // free context
    mbedtls_entropy_free(&entropy); 
    mbedtls_ctr_drbg_free(&ctr_drbg); 
    mbedtls_rsa_free(p_rsa);
    mbedtls_pk_free(&p_pk);
    return ret;
}

/**
 ***************************************************************************************************
 *	@brief		RSA Key Pair
 *	@details	https://qiita.com/Kosuke_Matsui/items/786e88eace7646c8a7e6
**************************************************************************************************/
int mbedtls_rsa_key_generation_test(uint32_t key_bits)
{
  
    int ret = 1;

    size_t olen;
    mbedtls_pk_context p_pk;
    mbedtls_rsa_context *p_rsa;
    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;
    const char* pers = "rsa_genkey";

    mbedtls_entropy_init(&entropy);    
    mbedtls_ctr_drbg_init(&ctr_drbg); 
    mbedtls_entropy_add_source(&entropy, entropy_source, NULL,
                               GENERATE_KEY_ENTROPY_THRESHOLD, MBEDTLS_ENTROPY_SOURCE_STRONG);

    // Initialize entropy.
    ret = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy,
                    (const unsigned char*)pers,
                    strlen(pers));
    if (ret != 0)
        return ret;

    mbedtls_rsa_init( p_rsa, MBEDTLS_RSA_PKCS_V15, 0 );
    mbedtls_pk_init(&p_pk);
    ret = mbedtls_pk_setup( &p_pk, mbedtls_pk_info_from_type( MBEDTLS_PK_RSA ) );    
    if (ret != 0)
        goto exit;
            
    p_rsa = mbedtls_pk_rsa( p_pk );
    ret = mbedtls_rsa_gen_key(p_rsa, mbedtls_ctr_drbg_random, &ctr_drbg, key_bits, 0x010001);
    if (ret != 0)
        return ret;

    mbedtls_rsa_context *rsa = mbedtls_pk_rsa( p_pk );

    size_t private_key_pem_len;
    size_t publick_key_pem_len;
    uint8_t private_key_pem[5000];
    uint8_t publick_key_pem[5000];

    ret = mbedtls_pk_write_key_pem(&p_pk, private_key_pem, sizeof(private_key_pem));
    if (ret != 0)
        goto exit;

    ret = mbedtls_pk_write_pubkey_pem(&p_pk, publick_key_pem, sizeof(publick_key_pem));
    if (ret != 0)
        goto exit;

    ret = mbedtls_rsa_rsaes_pkcs1_v15_encrypt(p_rsa, 
                        mbedtls_ctr_drbg_random, &ctr_drbg,
                        MBEDTLS_RSA_PUBLIC,
                        120, 
                        &g_plaintext_120[0], &rsa_oeap_encrypted_buf[0]);
    if (ret != 0)
        goto exit;


    ret = mbedtls_rsa_rsaes_pkcs1_v15_decrypt(p_rsa, 
                        mbedtls_ctr_drbg_random, &ctr_drbg,
                        MBEDTLS_RSA_PRIVATE,
                        &olen, rsa_oeap_encrypted_buf, rsa_oeap_decrypted_buf, 
                        120);
    if (ret != 0)
        goto exit;

    ret = memcmp(&g_plaintext_120, &rsa_oeap_decrypted_buf, 120);
    if( ret != 0 )
        printf("RSA-Key error\r\n");
    else
        printf("RSA-Key success\r\n");

exit:
    // free context
    mbedtls_entropy_free(&entropy); 
    mbedtls_ctr_drbg_free(&ctr_drbg); 
    mbedtls_rsa_free(p_rsa);
    mbedtls_pk_free(&p_pk);
    return ret;
}

/**
 ***************************************************************************************************
 *	@brief		BASE64
 *	@details	https://qiita.com/Kosuke_Matsui/private/915e6607b2b81de7f55d
**************************************************************************************************/
static const unsigned char base64_test_dec[64] =
{
    0x24, 0x48, 0x6E, 0x56, 0x87, 0x62, 0x5A, 0xBD,
    0xBF, 0x17, 0xD9, 0xA2, 0xC4, 0x17, 0x1A, 0x01,
    0x94, 0xED, 0x8F, 0x1E, 0x11, 0xB3, 0xD7, 0x09,
    0x0C, 0xB6, 0xE9, 0x10, 0x6F, 0x22, 0xEE, 0x13,
    0xCA, 0xB3, 0x07, 0x05, 0x76, 0xC9, 0xFA, 0x31,
    0x6C, 0x08, 0x34, 0xFF, 0x8D, 0xC2, 0x6C, 0x38,
    0x00, 0x43, 0xE9, 0x54, 0x97, 0xAF, 0x50, 0x4B,
    0xD1, 0x41, 0xBA, 0x95, 0x31, 0x5A, 0x0B, 0x97
};

static const unsigned char base64_test_enc[] =
    "JEhuVodiWr2/F9mixBcaAZTtjx4Rs9cJDLbpEG8i7hPK"
    "swcFdsn6MWwINP+Nwmw4AEPpVJevUEvRQbqVMVoLlw==";

int mbedtls_base64_self_test(void)
{
    int ret = 1;
    size_t len;
    const unsigned char *src;
    unsigned char buffer[200];

    src = base64_test_dec;
    if( mbedtls_base64_encode( buffer, sizeof( buffer ), &len, src, 64 ) != 0 ||
         memcmp( base64_test_enc, buffer, 88 ) != 0 )
    {        
        printf("BASE64 encode error\r\n");
        return( 1 );
    }    
    printf("BASE64 encode success\r\n");

    src = base64_test_enc;
    if( mbedtls_base64_decode( buffer, sizeof( buffer ), &len, src, 88 ) != 0 ||
         memcmp( base64_test_dec, buffer, 64 ) != 0 )
    {
        printf("BASE64 decode error\r\n");
        return( 1 );
    }
    printf("BASE64 decode success\r\n");

    return( 0 );
}

/**
 ***************************************************************************************************
 *	@brief		Verify JWT validation
 *	@details	https://qiita.com/Kosuke_Matsui/private/915e6607b2b81de7f55d
**************************************************************************************************/
// 256 byte
#define RSA1_TEST_PRIVATE_KEY \
"-----BEGIN PRIVATE KEY-----\r\n"  \
"MIIEvgIBADANBgkqhkiG9w0BAQEFAASCBKgwggSkAgEAAoIBAQCn7wJ2SLBWm0S8\r\n"  \
"8DPOn2qGLlbHCZ9sfjlpLtjV5JshGCUW/T8a6T/lGhud3W8RXRtmdLGeRJCLN3nl\r\n"  \
"9TJhWXNb0uMtlApvMNfe/H6dOvScOWMZ+sRNse1TIZO7pSt6LS5DCx7trHwZLxYL\r\n"  \
"E7nSLsA3ljOT7uIl5zV07raVpbw2GApnZJdnYcmTjnw+iHScEj+vO0MTCddyo7Mw\r\n"  \
"8T0obMLe4GJtv1YlmS69krsRWABVfls9ZGxTvrk2SpAXHBcFwKfLiQ2q2jghYfYl\r\n"  \
"wjKJMx78eUn+ODPfqB9qgtw5A8to3VpxijG2ifbGU4SQnTHe+vreLMSCfF3GJlYW\r\n"  \
"wQx53NuxAgMBAAECggEAALUDpUAv4QAkQOVnk1Pd8+aSKNspkmdt4Ffj4CwDvoim\r\n"  \
"jDe7iVgxhYNDPYFsdSFs89Tf17Fd4VO8+Jn1MryDZ8b2Cj/PtsTq0uII8xwyYgat\r\n"  \
"1PoBRXsksmSiE+Mhih9Sd5l3VQnbt6+mkX+0fG0ghbtlHaVJdbO6zCtpXRB/hvzG\r\n"  \
"LA5A0TI6GE166/9LHcq6+PhaaUIFX8vpiUuLt/XiOqOglaIfR/VSIPShlqzMEpYS\r\n"  \
"pfyDatanVT0G5m5gu4/3LfrSzla55Q9VRu2YjgEIeVkImvBn7PMhYjyHUO7AaExZ\r\n"  \
"EAWWtbcCA1oIl3DSZjhjGK62ao2Iw5vtjJDyOmUXLwKBgQDkODKC7DGqP3d+uP11\r\n"  \
"6HGp8TKRDeV3MrjR7uwgFo9XkLzYc5eJTFHZHYJ4WNAQNxbDJnoTiobbLcxVv2Jw\r\n"  \
"8nB34gDO3Am2+mdRYVlcdAbkyeJV+95Fb6OvZ/ZQKom9cnjNNu3TP1s6lrSaH+nH\r\n"  \
"nfDoeRQiznbpfLxv/MKnJYiivwKBgQC8YC0y1XIHpOL78eMF8PKZAm9Ro9h9SA2D\r\n"  \
"7rTaUchrzKYJjhcT2RS+j/Kv6HdeuqSAyjptLcI/i1QyfvJy2TK9ST1MayhRhX+d\r\n"  \
"J5Zp6QZDOX0s1O8ccTPNhjZf7DD24LcX8Plj56W8a100Aw1YsPoBvAP5vsmhnLsH\r\n"  \
"6ta/AInNjwKBgQDat2DxSBtw5dJHiFKgpVwJWjbz/TVkvx+RUkDJn9VQPk49wsn/\r\n"  \
"szzdrwJTBAqi/6i47i8geyoy8/lYVEqrpC1VNys/FHHbAq/xtjJGAIx6x2A6t1+V\r\n"  \
"fCCLAj91JM9M2GAdi/7w7U8pHzWGX/9UZ3Fx2lgJ44Tiz8BoMvz542bHUQKBgCXS\r\n"  \
"zeCCGRZN1kGuTN45hgyQ/5cN5f8pw4T1Hh0kBLEYc1JB1IJouRCWSK1naOh9Y2B0\r\n"  \
"uoyHfpRRvoMxGC9VSynUldlNPtg3jOsaMsprPpWq6mZhDu3QEQ24YByciSxY7IqD\r\n"  \
"GSAlOBMAH8O8xSZEyr4kaUvxMf0gQjGRxrKtQf4jAoGBAK7Xo8uDmBqHCnNjGUlH\r\n"  \
"qtf4ipwBRI2aH/izHHL2JUkHMvWac0e6VdQxdLPsqulcaoHzJRrX/gGYJJ3YFJ4V\r\n"  \
"jevRMirQ9ZKnT3mNJWU6XfU7TpH3Up8JQ0bQH/B8AV1Ogl4RNOVeZQwsSAf/Ba+V\r\n"  \
"CHNhfFeMKqz2jS6+VRHEpvyd\r\n"  \
"-----END PRIVATE KEY-----\r\n"

// 256 byte
#define RSA1_TEST_PUBLIC_KEY \
"-----BEGIN PUBLIC KEY-----\r\n"  \
"MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAp+8CdkiwVptEvPAzzp9q\r\n"  \
"hi5WxwmfbH45aS7Y1eSbIRglFv0/Guk/5Robnd1vEV0bZnSxnkSQizd55fUyYVlz\r\n"  \
"W9LjLZQKbzDX3vx+nTr0nDljGfrETbHtUyGTu6Urei0uQwse7ax8GS8WCxO50i7A\r\n"  \
"N5Yzk+7iJec1dO62laW8NhgKZ2SXZ2HJk458Poh0nBI/rztDEwnXcqOzMPE9KGzC\r\n"  \
"3uBibb9WJZkuvZK7EVgAVX5bPWRsU765NkqQFxwXBcCny4kNqto4IWH2JcIyiTMe\r\n"  \
"/HlJ/jgz36gfaoLcOQPLaN1acYoxton2xlOEkJ0x3vr63izEgnxdxiZWFsEMedzb\r\n"  \
"sQIDAQAB\r\n"  \
"-----END PUBLIC KEY-----\r\n"

// 466 chars
static unsigned char jwt_sign_sample[] =    
    "eyJhbGciOiJSUzI1NiIsInR5cCI6IkpXVCJ9.eyJzdWIiOiJIZWxsbyxXb3JsZCEiLCJuYW1lIjoiS29zdWtlIE1hdHN1aSIsImlhdCI6ODc1Njc0ODM5Mjg0fQ.N_zzSrKu4emL6OzpAQ7nBsR6XkV3I2TEkkrqM4Ldm7m42a78HEXsHx5X7AQQlApSPzCuIL3elXfrr4Tl2IRlstlsU6Im79V7hpWigAWnT4HyINEX74LKGeGHwve1iJpXKvTYIsjwTaKWwVGmHG2CPqVz_gVlNJJoe9PyMGzLnzZcUIYj20ATaE1NgdSoZEc9xA4T7EQWdTS4WRtSffPTREPG1Wgf0LVQZueW2P1kYsf9-_ItJTJk2GRqnzqaob-5hRANrCqWcxEr-HKw4PIftnxiVN3WqT3NWG7qT6UKfsKVNfFJZJOWcmr0UJGIpHavuvjnd-5P_9AbYchkKCucMg";

// 123 chars
static unsigned char jwt_sign_data[] =    
    "eyJhbGciOiJSUzI1NiIsInR5cCI6IkpXVCJ9.eyJzdWIiOiJIZWxsbyxXb3JsZCEiLCJuYW1lIjoiS29zdWtlIE1hdHN1aSIsImlhdCI6ODc1Njc0ODM5Mjg0fQ";

// 86 chars
//static unsigned char jwt_sign_payload[] =    
//    "eyJzdWIiOiJIZWxsbyxXb3JsZCEiLCJuYW1lIjoiS29zdWtlIE1hdHN1aSIsImlhdCI6ODc1Njc0ODM5Mjg0fQ";

// 342+2 chars
//static unsigned char jwt_sign_signature[] =    
//    "N_zzSrKu4emL6OzpAQ7nBsR6XkV3I2TEkkrqM4Ldm7m42a78HEXsHx5X7AQQlApSPzCuIL3elXfrr4Tl2IRlstlsU6Im79V7hpWigAWnT4HyINEX74LKGeGHwve1iJpXKvTYIsjwTaKWwVGmHG2CPqVz_gVlNJJoe9PyMGzLnzZcUIYj20ATaE1NgdSoZEc9xA4T7EQWdTS4WRtSffPTREPG1Wgf0LVQZueW2P1kYsf9-_ItJTJk2GRqnzqaob-5hRANrCqWcxEr-HKw4PIftnxiVN3WqT3NWG7qT6UKfsKVNfFJZJOWcmr0UJGIpHavuvjnd-5P_9AbYchkKCucMg==";

static int rsa_jwt_verification_test(uint8_t *data, uint16_t data_len, uint8_t *signature, uint16_t signature_len)
{
    int ret = 1;

    mbedtls_pk_context p_pk;
    mbedtls_rsa_context *p_rsa;
    uint8_t hash[32] = {0};

    // hash SHA-256
    ret =  mbedtls_sha256_ret( data, data_len, hash, 0 );
    if( 0 != ret)
        goto exit;

    // public key
    mbedtls_rsa_init( p_rsa, MBEDTLS_RSA_PKCS_V15, 0 );
    mbedtls_pk_init(&p_pk);

    ret = mbedtls_pk_parse_public_key(&p_pk, RSA1_TEST_PUBLIC_KEY, strlen(RSA1_TEST_PUBLIC_KEY) + 1);
    if (ret != 0)
        goto exit;
 
    p_rsa = mbedtls_pk_rsa( p_pk );

    // 64Base encode signature
    size_t len;
    unsigned char jwt_decoded_buffer[256] = {0};

    ret = mbedtls_base64_decode( jwt_decoded_buffer, sizeof( jwt_decoded_buffer ), &len, signature, signature_len );
    if (ret != 0)
        goto exit;
 
    // verification
    ret = mbedtls_rsa_rsassa_pkcs1_v15_verify(p_rsa, NULL, NULL, MBEDTLS_RSA_PUBLIC, MBEDTLS_MD_SHA256, 0, hash, jwt_decoded_buffer);
     if (ret != 0)
        goto exit;

    // free context
    mbedtls_rsa_free(p_rsa);
    mbedtls_pk_free(&p_pk);

    printf("JWT validation success\r\n");
    return ret;

exit:
    // free context
    mbedtls_rsa_free(p_rsa);
    mbedtls_pk_free(&p_pk);

    printf("JWT validation error\r\n");
    return ret;
}

int mbedtls_jwt_self_test(void)
{
    int ret = 1; 
    uint16_t data_len;
    uint16_t payload_len;
    uint16_t signature_len;
    uint16_t token_len;

    size_t len;
    uint8_t dot_count = 0;
    uint16_t payload_start_index = 0;
    uint16_t signatue_start_index = 0;

    sys_timer_count[TEST_TIMER] = 0;
    uint8_t expand_jwt_sign_test[500] = {0};
    for(int i=0; i<466; i++)
    {
        expand_jwt_sign_test[i] = jwt_sign_sample[i];
    }

    for(int i=0; i<JWT_SIZE; i++)
    {
        if(0x00 == expand_jwt_sign_test[i])
        {
            token_len = i;
            break;
        }
    }

        
    for(int i=0; i<token_len; i++)
    {
        if(*(expand_jwt_sign_test+i) == '-')
        {
            *(expand_jwt_sign_test+i) = '+';
        }

        if(*(expand_jwt_sign_test+i) == '_')
        {
            *(expand_jwt_sign_test+i) = '/';
        }
    }   

    uint8_t jwt_buffer[200];
    
    // check data format 
    // 01234.6789.BCDEF(token_len = 16chars)
    // data_len = A(10)    -> sig_len = token_len - data_len - 1 = 16 - 10 - 1 = 5chars
    // pay_start_index = 6 -> pay_len = 16 - pay_start_index - sig_len - 1 = 16 - 6 - 5 - 1 = 4chars
    for(int i=0; i<token_len; i++)
    {
        if(*(expand_jwt_sign_test+i) == '.')
        {
            dot_count += 1;
            if(1 == dot_count)
            {
                payload_start_index = i+1;                
                dot_count = 0;
                break;
            }
        }
    }    
    
    for(int i=0; i<token_len; i++)
    {
        if(*(expand_jwt_sign_test+i) == '.')
        {
            dot_count += 1;

            if(2 == dot_count)
            {
                signatue_start_index = i+1;
                data_len = i;
                signature_len = token_len - data_len -1; 
                break;
            }
        }
    }
    payload_len = token_len - payload_start_index - signature_len -1;

    // check data format error
    if(2 != dot_count)
        return 0x0FFF;

    // separate JWT into data and signature
    uint8_t data[data_len];
    uint8_t payload[payload_len];
    uint8_t signature[signature_len];

    for(int i=0; i<data_len; i++)
    {
        *(data+i) = *(expand_jwt_sign_test+i);
    }

    for(int i=0; i<payload_len; i++)
    {
        *(payload+i) = *(expand_jwt_sign_test+payload_start_index+i);
    }    

    for(int i=0; i<signature_len; i++)
    {
        *(signature+i) = *(expand_jwt_sign_test+signatue_start_index+i);
    }    

    // signature must be a multiple of 4
    uint16_t old_signature_len = signature_len;
    uint16_t signature_len_mod = signature_len%4;
    if(0 != signature_len_mod)
    {
        signature_len = signature_len + (4 - signature_len_mod);

        for(int i=old_signature_len; i<signature_len; i++)
        {
            *(signature+i) = '=';
        }
    }

    // verification
    ret = rsa_jwt_verification_test(data, data_len, signature, signature_len);

    // decode Base64
    mbedtls_base64_decode(jwt_buffer, sizeof( jwt_buffer ), &len, payload, payload_len);
    printf("JWT=");
     for (uint16_t i=0; i<len; i++) {
         printf("%c", jwt_buffer[i]);
     }
     printf("\r\n");

    printf("Execution time of rsa_jwt_verification_test %d ms\n",sys_timer_count[TEST_TIMER]*50);
    sys_timer_count[TEST_TIMER] = 0;


    return ret;
}


/**
 ***************************************************************************************************
 *	@brief		JWT Signature
 *	@details	https://qiita.com/Kosuke_Matsui/private/915e6607b2b81de7f55d
**************************************************************************************************/
int mbedtls_hash_encrypt_test(void)
{
    int ret = 1;

    uint8_t hash[32] = {0};
    uint8_t hash_encrypted[256] = {0};

    // hash SHA-256
    ret =  mbedtls_sha256_ret( jwt_sign_data, strlen(jwt_sign_data), hash, 0 );
    if( 0 != ret)
        return ret;

    size_t olen;
    mbedtls_pk_context p_pk;
    mbedtls_rsa_context *p_rsa;
    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;
    const char *pers = "rsa key generation";

    mbedtls_entropy_init(&entropy);    
    mbedtls_ctr_drbg_init(&ctr_drbg); 
    mbedtls_entropy_add_source(&entropy, entropy_source, NULL,
                               GENERATE_KEY_ENTROPY_THRESHOLD, MBEDTLS_ENTROPY_SOURCE_STRONG);

    // Initialize entropy.
    ret = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy,
                    (const unsigned char *) pers,
                    strlen( pers ));
    if (ret != 0)
        goto exit;

    mbedtls_rsa_init( p_rsa, MBEDTLS_RSA_PKCS_V15, 0 );
    mbedtls_pk_init(&p_pk);
    ret = mbedtls_pk_parse_key(&p_pk, RSA1_TEST_PRIVATE_KEY, strlen(RSA1_TEST_PRIVATE_KEY) + 1, NULL, NULL);
    if (ret != 0)
        goto exit;

    mbedtls_rsa_init(p_rsa, MBEDTLS_RSA_PKCS_V15, MBEDTLS_MD_SHA256);

    p_rsa = mbedtls_pk_rsa( p_pk );
    //mbedtls_rsa_set_padding(p_rsa, MBEDTLS_RSA_PKCS_V15, MBEDTLS_MD_SHA256);


    ret = mbedtls_rsa_rsaes_pkcs1_v15_encrypt(p_rsa, 
                        mbedtls_ctr_drbg_random, &ctr_drbg,
                        MBEDTLS_RSA_PRIVATE,
                        32, &hash[0], &hash_encrypted[0]);
    if (ret != 0)
        goto exit;

    // decode Base64
    size_t len;
    unsigned char jwt_encoded_buffer[500] = {0};
    ret = mbedtls_base64_encode( jwt_encoded_buffer, sizeof( jwt_encoded_buffer ), &len, hash_encrypted, 256 );
    if (ret != 0)
        goto exit;

    printf("signature=");
     for (uint16_t i=0; i<len; i++) {
         printf("%c", jwt_encoded_buffer[i]);
     }
     printf("\r\n");

exit:
    // free context
    mbedtls_entropy_free(&entropy); 
    mbedtls_ctr_drbg_free(&ctr_drbg); 
    mbedtls_rsa_free(p_rsa);
    mbedtls_pk_free(&p_pk);

    return ret;
}
