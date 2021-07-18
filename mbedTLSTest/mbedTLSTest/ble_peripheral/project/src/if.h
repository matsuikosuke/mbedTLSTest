/**
 ***************************************************************************************************
 * @file        if.h
 * @brief       Intrface Pin Assign
 ***************************************************************************************************
 **************************************************************************************************/
#ifndef IF_H /* IF_H */
#define IF_H

// Debug
#define DEBUG_TXD_PIN (25)
#define DEBUG_RXD_PIN (26)

// Accelerometer
#define VIB_INT_PIN (27)
#define VIB_SCL_PIN (28)
#define VIB_SDA_PIN (29)

// EC25J
#define EC25J_RESET_N_PIN (30)
#define EC25J_PWRKEY_PIN (31)
#define EC25J_DTR_PIN (9)
#define EC25J_PWRON_PIN (10)
#define EC25J_RXD_PIN (8)
#define EC25J_TXD_PIN (7)
#define EC25J_GNSSON_PIN (13)

// AD
#define LIBAT_TEST_PIN NRF_SAADC_INPUT_AIN0        // AN0
#define EBICYCLE_BAT_TEST_PIN NRF_SAADC_INPUT_AIN1 // AN1
#define TESTBAT_TRIGER_PIN (4)

// MOTOR
#define MOTOR_INA_PIN (5)
#define MOTOR_INB_PIN (6)
#define LOCK_OPEN_ST_PIN (20)
#define MOTOR_AT_LOCK_PIN (22)
#define MOTOR_AT_OPEN_PIN (23)

// FLASH MEMOERY
#define F_MOSI_PIN (15) // SPI Serial Data Output (SDO)
#define F_CLK_PIN (16)  // SPI Serial Port Clock (SPC)
#define F_NSS_PIN (17)  // SPI mode selection (1: SPI idle mode, 0: SPI communication mode)
#define F_MISO_PIN (18) // SPI Serial Data Input (SDI)

// BUZZER
#define DRV_BUZ_PIN (19)

// SW
#define FRONT_SW_PIN (21)

// //WiO
//#define WIO_ENABLE_PIN		( 32 )
//
// //IRQ
//#define IRQ0_PIN		( 14 )
//#define IRQ1_PIN		( 11 )
//
// //Motor
//#define MOTOR_PWM_PIN           ( 37 )
//#define MOTOR_DIR_PIN		( 36 )
//#define ENCODER_A_PIN		( 39 )
//#define ENCODER_B_PIN		( 38 )
//
////UART
//#define GPS_UART_TX_PIN		( 44 )
//#define GPS_UART_RX_PIN		( 46 )
//#define GPS_UART_RTS_PIN	( 17 )
//#define GPS_UART_CTS_PIN	( 12 )
//
//#define EC21_UART_TX_PIN	( 21 )//( 30 )//
//#define EC21_UART_RX_PIN	( 22 )//( 28 )//
//#define EC21_UART_RTS_PIN	( 20 )//( 31 )//
//#define EC21_UART_CTS_PIN	( 19 )//( 29 )//
//
////SPI0
//#define SPI0_SPC_PIN		( 33 ) // SPI Serial Port Clock (SPC)
//#define SPI0_MISO_PIN		( 13 ) // SPI Serial Data Output (SDO)
//#define SPI0_MOSI_PIN		( 35 ) // SPI Serial Data Input (SDI)
//#define LSM6DS_CS_PIN		( 16 ) // SPI mode selection (1: SPI idle mode, 0: SPI communication mode)
//#define AQUES_CS_PIN		( 17 )
//#define CAN_CS_PIN		( 34 )
//
////SPI1
//#define SPI1_SPC_PIN		( 43 ) // SPI Serial Port Clock (SPC)
//#define SPI1_MISO_PIN		( 47 ) // SPI Serial Data Output (SDO)
//#define SPI1_MOSI_PIN		( 42 ) // SPI Serial Data Input (SDI)
//#define TFT_CS_PIN		( 45 ) // SPI mode selection (1: SPI idle mode, 0: SPI communication mode)
//
//// I2C
//#define M8U_I2C_SCL_PIN         ( 39 )
//#define M8U_I2C_SDA_PIN         ( 38 )


#endif /* IF_H */