/*
 * DRV8301.c
 *
 *  Created on: 07.03.2016
 *      Author: Thomas Halwax
 */
#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>

/*Board Header files */
#include <Board.h>
#include <EK_TM4C1294XL.h>

#include <xdc/std.h>
#include <xdc/cfg/global.h>
#include <xdc/runtime/System.h>
#include <xdc/runtime/Error.h>
#include <xdc/runtime/Memory.h>

#include <ti/sysbios/BIOS.h>
#include <inc/hw_memmap.h>/*supplies GPIO_PORTx_BASE*/
#include <driverlib/gpio.h>
#include <driverlib/pin_map.h>/*supplies GPIO_PIN_x*/
#include <driverlib/sysctl.h>
#include <ti/drivers/PWM.h>
#include <ti/drivers/SPI.h>
#include <ti/sysbios/knl/Task.h>
#include <DRV8301.h>
#include <Communication.h>


extern CommunicationInfrastructure globalCommInfrastructure;

void initializeDrv8301(void) {

    uint32_t ui32Strength;
    uint32_t ui32PinType;

    /* activate gpio port C */
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);

    /* configure pin for FAULT */
	/* read current config */
	GPIOPadConfigGet(GPIO_PORTC_BASE, GPIO_PIN_4, &ui32Strength, &ui32PinType);
	/* then set */
	GPIOPadConfigSet(GPIO_PORTC_BASE, GPIO_PIN_4, ui32Strength, GPIO_PIN_TYPE_STD_WPU);
	/* use as input */
	GPIOPinTypeGPIOInput(GPIO_PORTC_BASE, GPIO_PIN_4);


	/* activate gpio port M */
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOM);

    /* configure pin for SPI_CS */
	/* read current config */
	GPIOPadConfigGet(GPIO_PORTM_BASE, GPIO_PIN_3, &ui32Strength, &ui32PinType);
	/* then set */
	GPIOPadConfigSet(GPIO_PORTM_BASE, GPIO_PIN_3, ui32Strength, GPIO_PIN_TYPE_STD_WPU);
	/* use as output */
	GPIOPinTypeGPIOOutput(GPIO_PORTM_BASE, GPIO_PIN_3);

	/* activate gpio port N */
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPION);

    /* read current config for EN-GATE */
    GPIOPadConfigGet(GPIO_PORTN_BASE, GPIO_PIN_2, &ui32Strength, &ui32PinType);
	/* then set */
	GPIOPadConfigSet(GPIO_PORTN_BASE, GPIO_PIN_2, ui32Strength, GPIO_PIN_TYPE_STD_WPD);
	/* use as output */
	GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE, GPIO_PIN_2);

}


void EnableDRV8301(void) {
	/* set EN_GATE to high to enable the motor driver */

	uint32_t value = GPIOPinRead(GPIO_PORTN_BASE, GPIO_PIN_2);
	value |= (1 << 2);
	GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_2, value);
}

void DisableDRV8301(void) {
	/* set EN_GATE to high to enable the motor driver */
	uint32_t value = GPIOPinRead(GPIO_PORTN_BASE, GPIO_PIN_2);
	value &= ~(1 << 2);
	GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_2, value);
}

void EnableDRV8301SPI(void) {
	uint32_t value = GPIOPinRead(GPIO_PORTM_BASE, GPIO_PIN_3);
	value &= ~(1 << 3);
	GPIOPinWrite(GPIO_PORTM_BASE, GPIO_PIN_3, value);
}

void DisableDRV8301SPI(void) {
	uint32_t value = GPIOPinRead(GPIO_PORTM_BASE, GPIO_PIN_3);
	value |= (1 << 3);
	GPIOPinWrite(GPIO_PORTM_BASE, GPIO_PIN_3, value);
}

bool isFaulty() {
	uint32_t value = GPIOPinRead(GPIO_PORTC_BASE, GPIO_PIN_4);
	return (!((value & GPIO_PIN_4)==GPIO_PIN_4));
	/* nFAULT -> zero on FAULT */
}

void initDRV8301PWM(void) {
	 /* Enable PWM peripherals */
	SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM1);

	/*
	 * Enable PWM output on GPIO pins.  PWM output is connected to an Ethernet
	 * LED on the development board (D4).  The PWM configuration
	 * below will disable Ethernet functionality.
	 */
	GPIOPinConfigure(GPIO_PF1_M0PWM1);
	GPIOPinTypePWM(GPIO_PORTF_BASE, GPIO_PIN_1);

	PWM_init();
}


void PwmControllerTask() {

	/* check if ethernet functionality is still enabled! See EK_TM4C1294XL_initPWM() code comments */
	EK_TM4C1294XL_initPWM();
	initializeDrv8301();
	EnableDRV8301();

	SPI_Handle spiHandle;
	SPI_Params spiParams;
	SPI_Transaction spiTransaction;
	SPI_Params_init(&spiParams);

	spiParams.transferMode = SPI_MODE_BLOCKING;
	spiParams.transferCallbackFxn = NULL;
	spiParams.dataSize = 16; // 16 - 1 (R/W) - 4 (ADDRESS) = 11

	spiHandle = SPI_open(Board_SPI0, &spiParams);
	if (spiHandle == NULL) {
		System_abort("failed to open SPI handle\n");
	}

	UShort spiRxBuffer[2];
	UShort spiTxBuffer[2];

	spiTransaction.txBuf = spiTxBuffer;
	spiTransaction.rxBuf = spiRxBuffer;
	spiTransaction.count = 2;
	//spiTransaction.status = SPI_TRANSFER_STARTED;


	// READ status register
	spiTxBuffer[0] = MODE_WRITE | REGISTER_CONTROL_1 | REGISTER_CONTROL_1_GATE_CURRENT_0700 | REGISTER_CONTROL_1_PWM_MODE_3P | REGISTER_CONTROL_1_OC_ADJ_SET_0097;
	spiTxBuffer[1] = MODE_READ | REGISTER_CONTROL_1;

	EnableDRV8301SPI();
	bool success = SPI_transfer(spiHandle, &spiTransaction);
	if (!success) {
		System_printf("SPI transfer failed!\n");
	}

	spiTxBuffer[0] = MODE_READ | REGISTER_CONTROL_1;
	spiTransaction.count = 1;

	success = SPI_transfer(spiHandle, &spiTransaction);
	if (!success) {
		System_abort("SPI transfer failed!\n");
	}

	PWM_Handle pwmHandle;
	PWM_Params pwmParams;
	PWM_Params_init(&pwmParams);
	pwmParams.dutyMode = PWM_DUTY_SCALAR;
	pwmParams.period = 25000; // mySec
	pwmParams.polarity = PWM_POL_ACTIVE_HIGH;

	pwmHandle = PWM_open(Board_PWM1, &pwmParams);
	if (pwmHandle == NULL) {
		System_abort("PWM handle creation failed!\n");
	}

	uint32_t duty = 0;
	while (1) {
		if (Mailbox_pend(globalCommInfrastructure.pwmMailbox, &duty, BIOS_WAIT_FOREVER)) {
			PWM_setDuty(pwmHandle, duty);
		}
	}
}


void SetupPwmControllerTask(Error_Block *errorBlock) {

	Task_Params pwmControllerTaskParams;
	Task_Handle setupDrv8301TaskHandle;

	Task_Params_init(&pwmControllerTaskParams);
	pwmControllerTaskParams.stackSize = 1024;/*stack in bytes*/
	pwmControllerTaskParams.priority = 7;
	setupDrv8301TaskHandle = Task_create((Task_FuncPtr)PwmControllerTask, &pwmControllerTaskParams, errorBlock);
	if (setupDrv8301TaskHandle == NULL) {
		System_abort("Task setupDrv8301Task create failed");
	}
	else {
		System_printf("Setup for setupDrv8301Task finished :-)\n");
	}
}


