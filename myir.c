#include <xdc/std.h>
#include <stdbool.h>
#include <stdint.h>

#include <driverlib/gpio.h>
#include <ti/drivers/GPIO.h>
#include <ti/drivers/I2C.h>
#include <driverlib/sysctl.h>
#include <Board.h>
#include <EK_TM4C1294XL.h>
#include <driverlib/i2c.h>
#include <driverlib/pin_map.h>
#include <inc/hw_memmap.h>

#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>
#include <xdc/runtime/System.h>
#include <xdc/runtime/Error.h>

#include <math.h>
#include "myqueue.h"

#define F_CPU 16000000

#define PORT_SECTOR_1 GPIO_PORTH_BASE
#define PIN_SECTOR_1 GPIO_PIN_0
#define INTERRUPT_SECTOR_1 GPIO_INT_PIN_0

#define PORT_SECTOR_2 GPIO_PORTH_BASE
#define PIN_SECTOR_2 GPIO_PIN_1
#define INTERRUPT_SECTOR_2 GPIO_INT_PIN_1

#define PORT_SECTOR_3 GPIO_PORTK_BASE
#define PIN_SECTOR_3 GPIO_PIN_4
#define INTERRUPT_SECTOR_3 GPIO_INT_PIN_4

#define PORT_SECTOR_4 GPIO_PORTK_BASE
#define PIN_SECTOR_4 GPIO_PIN_5
#define INTERRUPT_SECTOR_4 GPIO_INT_PIN_5

#define PORT_SECTOR_5 GPIO_PORTK_BASE
#define PIN_SECTOR_5 GPIO_PIN_6
#define INTERRUPT_SECTOR_5 GPIO_INT_PIN_6

#define PORT_SECTOR_6 GPIO_PORTK_BASE
#define PIN_SECTOR_6 GPIO_PIN_7
#define INTERRUPT_SECTOR_6 GPIO_INT_PIN_7

void initialize_IR() {
	/// GPIOPinTypeGPIOInput(GPIO_PORTF_BASE, GPIO_PIN_1);
	// GPIOPinTypeGPIOInput(GPIO_PORTF_BASE, GPIO_PIN_2);
	GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_1, GPIO_STRENGTH_2MA,
	GPIO_PIN_TYPE_STD_WPD);
	GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_2, GPIO_STRENGTH_2MA,
	GPIO_PIN_TYPE_STD_WPD);

	GPIOPadConfigSet(GPIO_PORTH_BASE, GPIO_PIN_0, GPIO_STRENGTH_2MA,
	GPIO_PIN_TYPE_STD_WPD); // SEKTOR 1
	GPIOPadConfigSet(GPIO_PORTH_BASE, GPIO_PIN_1, GPIO_STRENGTH_2MA,
	GPIO_PIN_TYPE_STD_WPD); // SEKTOR 2
	GPIOPadConfigSet(GPIO_PORTK_BASE, GPIO_PIN_4, GPIO_STRENGTH_2MA,
	GPIO_PIN_TYPE_STD_WPD); // SEKTOR 3
	GPIOPadConfigSet(GPIO_PORTK_BASE, GPIO_PIN_5, GPIO_STRENGTH_2MA,
	GPIO_PIN_TYPE_STD_WPD); // SEKTOR 4
	GPIOPadConfigSet(GPIO_PORTK_BASE, GPIO_PIN_6, GPIO_STRENGTH_2MA,
	GPIO_PIN_TYPE_STD_WPD); // SEKTOR 5
	GPIOPadConfigSet(GPIO_PORTK_BASE, GPIO_PIN_7, GPIO_STRENGTH_2MA,
	GPIO_PIN_TYPE_STD_WPD); // SEKTOR 6
}
