/*
 *  ======== Board.h ========
 *  Configured TI-Drivers module declarations
 *
 *  DO NOT EDIT - This file is generated for the CC1352P1_LAUNCHXL
 *  by the SysConfig tool.
 */
#ifndef Board_h
#define Board_h

#define Board_SYSCONFIG_PREVIEW

#define Board_CC1352P1_LAUNCHXL

/* Temporary define for backwards compatibility!!! */
#define __CC1352P1_LAUNCHXL_BOARD_H__

#ifndef DeviceFamily_CC13X2
#define DeviceFamily_CC13X2
#endif

#include <stdint.h>

/* support C++ sources */
#ifdef __cplusplus
extern "C" {
#endif


/*
 *  ======== GPIO ========
 */

#define Board_GPIO0                 0
#define Board_GPIO1                 1
#define Board_GPIO2                 2

/* LEDs are active high */
#define Board_GPIO_LED_ON  (1)
#define Board_GPIO_LED_OFF (0)

#define Board_LED_ON  (Board_GPIO_LED_ON)
#define Board_LED_OFF (Board_GPIO_LED_OFF)


/*
 *  ======== NVS ========
 */

#define Board_NVS0                  0


/*
 *  ======== PIN ========
 */

/* Includes */
#include <ti/drivers/PIN.h>

/* Externs */
extern const PIN_Config BoardGpioInitTable[];

/* XDS110 UART, Parent Signal: Board_UART0 TX, (DIO13) */
#define Board_PIN2    0x0000000d
/* XDS110 UART, Parent Signal: Board_UART0 RX, (DIO12) */
#define Board_PIN3    0x0000000c
/* Board_RF_24GHZ (DIO28) */
#define Board_RF_24GHZ    0x0000001c
/* Board_RF_HIGH_PA (DIO29) */
#define Board_RF_HIGH_PA    0x0000001d
/* Board_RF_SUB1GHZ (DIO30) */
#define Board_RF_SUB1GHZ    0x0000001e
/* LaunchPad LED Red, Parent Signal: Board_GPIO0 GPIO Pin, (DIO6) */
#define Board_PIN0    0x00000006
/* LaunchPad LED Green, Parent Signal: Board_GPIO1 GPIO Pin, (DIO7) */
#define Board_PIN1    0x00000007
/* SPI Flash Device Chip Select, Parent Signal: Board_GPIO2 GPIO Pin, (DIO20) */
#define Board_PIN4    0x00000014
/* SPI Bus, Parent Signal: Board_SPI0 SCLK, (DIO10) */
#define Board_PIN5    0x0000000a
/* SPI Bus, Parent Signal: Board_SPI0 MISO, (DIO8) */
#define Board_PIN6    0x00000008
/* SPI Bus, Parent Signal: Board_SPI0 MOSI, (DIO9) */
#define Board_PIN7    0x00000009


/*
 *  ======== RF ========
 */
#define Board_DIO30_RFSW 0x0000001e


/*
 *  ======== SPI ========
 */

#define Board_SPI0                  0


/*
 *  ======== Timer ========
 */

#define Board_TIMER0                0


/*
 *  ======== UART ========
 */

#define Board_UART0                 0


/*
 *  ======== Board_init ========
 *  Perform all required TI-Drivers initialization
 *
 *  This function should be called once at a point before any use of
 *  TI-Drivers.
 */
extern void Board_init(void);

/*
 *  ======== Board_initGeneral ========
 *  (deprecated)
 *
 *  Board_initGeneral() is defined purely for backward compatibility.
 *
 *  All new code should use Board_init() to do any required TI-Drivers
 *  initialization _and_ use <Driver>_init() for only where specific drivers
 *  are explicitly referenced by the application.  <Driver>_init() functions
 *  are idempotent.
 */
#define Board_initGeneral Board_init

#ifdef __cplusplus
}
#endif

#endif /* include guard */
