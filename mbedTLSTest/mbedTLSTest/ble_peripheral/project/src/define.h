/*----------------------------------------------------------
  Device Information
----------------------------------------------------------*/
#define		TEST_COMPANY_IDENTIFIER        0xFFFF
#define		DEVICE_ID_LENGTH          (8)

/*----------------------------------------------------------
  BLE COMMAND
----------------------------------------------------------*/
// indication
#define MAX_INDICATION_RETRY 3
#define INDICATION_STATUS   0x10
#define INDICATION_OPEN     0x20
#define INDICATION_LOCK     0x40
#define INDICATION_DEV_INFO  0x80
#define INDICATION_INSPECT  0x30

// max length
#define BLE_WRITE_DATA_LENGTH (NRF_SDH_BLE_GATT_MAX_MTU_SIZE-3)
#define BLE_INDICATION_DATA_LENGTH (NRF_SDH_BLE_GATT_MAX_MTU_SIZE-3)
#define BLE_READ_DATA_LENGTH (32)
#define BLE_ALL_DATA_MAX (600)

#define INDICATION_LENGTH_SERVER_HELLO_TYPE (19)
#define INDICATION_LENGTH_APPLICATION_TYPE  (267)

// INDEX
#define BLE_WRITE_ALL_PACKET_INDEX (0)
#define BLE_WRITE_NUM_PACKET_INDEX (1)

#define BLE_REASSEMBLY_WRITE_TYPE_INDEX (0)
#define BLE_REASSEMBLY_WRITE_LEN_UP_INDEX (1)
#define BLE_REASSEMBLY_WRITE_LEN_DOWN_INDEX (2)

#define BLE_INDICATE_TYPE_ALL_NUM   (0)
#define BLE_INDICATE_TYPE_PKT_NUM   (1)
#define BLE_INDICATE_TYPE_INDEX     (0)
#define BLE_INDICATE_LEN_UP_INDEX   (1)
#define BLE_INDICATE_LEN_DOWN_INDEX (2)

#define BLE_SERVER_HELLO_INDICATE_DATA_START_INDEX (3)

#define BLE_APPLICATION_INDICATE_RANDOM_INDEX (3)
#define BLE_APPLICATION_INDICATE_DATA_START_INDEX (11)



/*----------------------------------------------------------
  Timer
----------------------------------------------------------*/
// If all packets cannot be received within 1 second, an error event occurs
#define TIMER_NUM  (1)

#define TEST_TIMER (0)
#define TEST_TIMER_LIMIT TM_30MIN


#define APP_TIMER_PRESCALER		( 0 ) // Value of the RTC1 PRESCALER register
//#define APP_TIMER_MAX_TIMERS	( 3+1 ) // Maximum number of simultaneously created timers
#define APP_TIMER_OP_QUEUE_SIZE	( 10 )  // Size of timer operation queues

#define TM_STOP ( 0xFFFF )  // Timer Stop
#define TM_TMOUT  ( 0 )     // Timeout
#define TM_100MS  ( 2 )     // 50ms software timebase  : 100ms	
#define TM_300MS  ( 6 )     // 50ms software timebase  : 300ms	
#define	TM_500MS  ( 10 )    // 50ms software timebase  : 500ms
#define	TM_600MS  ( 12 )    // 50ms software timebase  : 600msec
#define	TM_1SEC   ( 20 )    // 50ms software timebase  : 1sec		
#define	TM_1500MS ( 30 )    // 50ms software timebase  : 1.5sec	
#define	TM_2SEC   ( 40 )    // 50ms software timebase  : 2sec		
#define	TM_3SEC   ( 60 )    // 50ms software timebase  : 3sec		
#define	TM_4SEC   ( 80 )    // 50ms software timebase  : 4sec	
#define	TM_4400MS ( 88 )    // 50ms software timebase  : 4400msec	
#define	TM_5SEC   ( 100 )   // 50ms software timebase  : 5sec
#define	TM_10SEC  ( 200 )   // 50ms software timebase  : 10sec
#define	TM_30SEC  ( 600 )   // 50ms software timebase  : 30sec
#define	TM_50SEC  ( 1000 )  // 50ms software timebase  : 50sec	
#define	TM_60SEC  ( 1200 )  // 50ms software timebase  : 60sec
#define	TM_2MIN   ( 2400 )  // 50ms software timebase  : 3min
#define	TM_3MIN   ( 3600 )  // 50ms software timebase  : 3min
#define	TM_5MIN   ( 6000 )  // 50ms software timebase  : 5min
#define	TM_10MIN   ( 12000 )  // 50ms software timebase  : 10min
#define	TM_20MIN   ( 24000 )  // 50ms software timebase  : 20min
#define	TM_29MIN   ( 34800 )  // 50ms software timebase  : 20min
#define	TM_30MIN   ( 36000 )  // 50ms software timebase  : 30min
#define	TM_50MIN   ( 60000 )  // 50ms software timebase  : 50min

#define MAIN_INTERVAL APP_TIMER_TICKS(50) // 50ms main interval


/*----------------------------------------------------------
  BLE
----------------------------------------------------------*/
#define BLE_DISCONNECT  0x00
#define BLE_CONNECT     0x01

/*----------------------------------------------------------
  RSA/ECDH/OAEP
----------------------------------------------------------*/
#define RSA_IV_SIZE (4)
#define HKDF_SEED_KEY_SIZE (256)//(128)
#define RSA_HKDF_KEY_SIZE  (32)
#define ECDH_KEY_SIZE  (32)
#define SHARED_KEY_SIZE  (32)
#define RSA_GCM_INOUT_SIZE (512)//(256)//0x01cd//
#define CLIENT_HELLO_SIZE (259)
#define DECRYPT_CLIENT_HELLO_SIZE (136)
#define SERVER_HELLO_DATA_SIZE (128)
#define JWT_SIZE (500)
#define DUMMY_ENTROPY_THRESHOLD (32) //(134)
#define GENERATE_KEY_ENTROPY_THRESHOLD (32)
#define RSA_GCM_IV_SIZE (12)
#define RSA_GCM_RANDOM_SIZE (8)
#define RSA_GCM_TAG_SIZE (16)
#define NONCE_SIZE (16)

#define IV_HASH_BYTE_SIZE (2)
#define IV_HASH_BYTE_1 0x69
#define IV_HASH_BYTE_2 0x76
#define SECRET_HASH_BYTE_SIZE (3)
#define SECRET_HASH_BYTE_1 0x6b
#define SECRET_HASH_BYTE_2 0x65
#define SECRET_HASH_BYTE_3 0x79

#define LONG_KEY  (120)
#define SHORT_KEY (56)