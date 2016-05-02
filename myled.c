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

#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>
#include <xdc/runtime/System.h>
#include <xdc/runtime/Error.h>
#include <inc/hw_memmap.h>

#include <math.h>

#define F_CPU 16000000

void configurePINOutput(uint32_t port, uint8_t pin, uint32_t type) {
    uint32_t ui32Strength;
    uint32_t ui32PinType;
	GPIOPadConfigGet(port, pin, &ui32Strength, &ui32PinType);
	GPIOPadConfigSet(port, pin,ui32Strength,type);
	GPIOPinTypeGPIOOutput(port,pin);
}

void initialize_LED() {
	configurePINOutput(GPIO_PORTF_BASE, GPIO_PIN_4, GPIO_PIN_TYPE_STD);
	configurePINOutput(GPIO_PORTF_BASE, GPIO_PIN_0, GPIO_PIN_TYPE_STD);
}
