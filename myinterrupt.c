#include <xdc/std.h>
#include <stdbool.h>
#include <xdc/cfg/global.h>
#include <xdc/runtime/System.h>
#include <xdc/runtime/Error.h>
#include <xdc/runtime/Memory.h>
#include <driverlib/sysctl.h>

#include <driverlib/gpio.h>
#include <driverlib/pin_map.h>
#include <driverlib/interrupt.h>

#include <Board.h>

#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include <ti/sysbios/hal/Hwi.h>
#include <ti/sysbios/Bios.h>
#include <ti/drivers/Watchdog.h>
#include <ti/sysbios/knl/Clock.h>
#include <ti/sysbios/knl/Mailbox.h>
#include <time.h>

#include "myqueue.h"
#include "myinterrupt.h"
#include <Communication.h>

#define F_CPU 120000000 // 120 * 1000 * 1000

uint32_t state = 0;
Information myInformation;
QueueObject myQueueObject;
int countWatchdog = 0;
int countSensor1 = 0;
int countSensor2 = 0;
int countSensor3 = 0;
int countSensor4 = 0;
int countSensor5 = 0;
int countSensor6 = 0;



uint32_t  *sectorPointer;
uint32_t s1 = 1;
uint32_t s2 = 2;
uint32_t s3 = 3;
uint32_t s4 = 4;
uint32_t s5 = 5;
uint32_t s6 = 0;

extern CommunicationInfrastructure globalCommInfrastructure;
extern uint32_t tickCount;
extern uint32_t lastInterruptTick;

RoundInformation myRoundInformation;

/**
 * usage: this method sets up the environment to make sure that interrupts (hw) are working
 * @method initializeInterrupts
 * @author: patrik.szabo
 * @param *none*
 * @return *none*
 */
void initializeInterrupts() {
	IntMasterEnable();
	// SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
	// SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOH);
	// SysCtlPeripheralEnable(SYSCTL_PERIPH_GPION);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOH);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOK);
	// SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOJ);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_WDOG0);

	tickCount = 0;
}

void isr_1_2_sensor_method(UArg arg0) {
	state = GPIOIntStatus(GPIO_PORTH_BASE, true);
	GPIOIntClear(GPIO_PORTH_BASE, GPIO_INT_PIN_0 | GPIO_INT_PIN_1);

	if (state == GPIO_INT_PIN_0 && (tickCount - lastInterruptTick) > 5) {
		lastInterruptTick = tickCount;
		System_printf("Finished Sektor 1\n");
		if (! Mailbox_post(globalCommInfrastructure.sectorIndexMailbox, &s1, BIOS_NO_WAIT)) {
			System_printf("failed to post duty value to pwm mailbox.\n");
			// System_flush();
		}

		/*
		GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_4, 0);
		GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, 0b0000001);
		countSensor1 = countWatchdog + 1;

		System_printf("Finished Sektor 1 - %d\n", tickCount);
		myRoundInformation.one = tickCount;
		tickCount = 0;
		System_flush();

		 */
	}
	if (state == GPIO_INT_PIN_1 &&(tickCount - lastInterruptTick) > 5) {
		lastInterruptTick = tickCount;
		System_printf("Finished Sektor 2\n");
		if (! Mailbox_post(globalCommInfrastructure.sectorIndexMailbox, &s2, BIOS_NO_WAIT)) {
			System_printf("failed to post duty value to pwm mailbox.\n");
			// System_flush();
		}
		/*
		GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_4, 0b0010000);
		GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, 0);
		countSensor2 = countWatchdog + 1;

		System_printf("Finished Sektor 2 - %d\n", tickCount);
		myRoundInformation.one = tickCount;
		tickCount = 0;
		*/
		// System_flush();
	}
}

void isr_3_6_sensor_method(UArg arg0) {
	state = GPIOIntStatus(GPIO_PORTK_BASE, true);
	GPIOIntClear(GPIO_PORTK_BASE,
	GPIO_INT_PIN_4 | GPIO_INT_PIN_5 | GPIO_INT_PIN_6 | GPIO_INT_PIN_7);

	if (state == GPIO_INT_PIN_4) {// && countWatchdog > countSensor3) { // 3
		if (!Mailbox_post(globalCommInfrastructure.sectorIndexMailbox, &s3, BIOS_NO_WAIT)) {
			System_printf("failed to post duty value to pwm mailbox.\n");
			// System_flush();
		}

		GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_4, 0b0010000);
		GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, 0);
		countSensor3 = countWatchdog + 1;

		System_printf("Finished Sektor 3 - %d\n", tickCount);
		myRoundInformation.one = tickCount;
		tickCount = 0;
		// System_flush();
	}

	if (state == GPIO_INT_PIN_5 && countWatchdog > countSensor4) { // 4
		if (! Mailbox_post(globalCommInfrastructure.sectorIndexMailbox, &s4, BIOS_NO_WAIT)) {
			System_printf("failed to post duty value to pwm mailbox.\n");
			// System_flush();
		}

		GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_4, 0b0010000);
		GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, 0);
		countSensor4 = countWatchdog + 1;

		System_printf("Finished Sektor 4 - %d\n", tickCount);
		myRoundInformation.one = tickCount;
		tickCount = 0;
		// System_flush();
	}

	if (state == GPIO_INT_PIN_6 && countWatchdog > countSensor5) { // 5
		if (! Mailbox_post(globalCommInfrastructure.sectorIndexMailbox, &s5, BIOS_NO_WAIT)) {
			System_printf("failed to post duty value to pwm mailbox.\n");
			// System_flush();
		}

		GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_4, 0b0010000);
		GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, 0);
		countSensor5 = countWatchdog + 1;

		System_printf("Finished Sektor 5 - %d\n", tickCount);
		myRoundInformation.one = tickCount;
		tickCount = 0;
		// System_flush();
	}

	if (state == GPIO_INT_PIN_7 && countWatchdog > countSensor6) { // 6
		if (! Mailbox_post(globalCommInfrastructure.sectorIndexMailbox, &s6, BIOS_NO_WAIT)) {
			System_printf("failed to post duty value to pwm mailbox.\n");
			// System_flush();
		}

		GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_4, 0b0010000);
		GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, 0);
		countSensor6 = countWatchdog + 1;

		System_printf("Finished Sektor 6 - %d\n", tickCount);
		myRoundInformation.one = tickCount;
		tickCount = 0;
		// System_flush();
	}
}

void watchdog_method(UArg handle) {
	countWatchdog++;
	Watchdog_clear((Watchdog_Handle) handle);
}

void tick(void) {
	tickCount++;
}

int setup_Clock_Task(uint32_t wait_ticks) {
	Clock_Params myclockParams;
	Clock_Handle clock;
	Error_Block eb;

	Error_init(&eb);
	Clock_Params_init(&myclockParams);

	myclockParams.period = wait_ticks;
	myclockParams.startFlag = TRUE;

	clock = Clock_create(tick, 1, &myclockParams, &eb);
	if (clock == NULL) {
		System_abort("Clock create failed");
	}

	return (0);
}

void setup_Interrupts() {
	Hwi_Params HWIParams;
	Hwi_Handle Hwi;
	Error_Block eb;
	Error_init(&eb);

	// Interrupt for I2C - START

	Hwi_Params_init(&HWIParams);
	HWIParams.arg = 0;
	HWIParams.enableInt = false;
	HWIParams.priority = 32;


	Hwi = Hwi_create(INT_GPIOH_TM4C129, isr_1_2_sensor_method, &HWIParams, &eb);
	if (Hwi == NULL) {
		System_abort("HWI create failed");
	}

	Hwi_enableInterrupt(INT_GPIOH_TM4C129);

	Hwi = Hwi_create(INT_GPIOK_TM4C129, isr_3_6_sensor_method, &HWIParams, &eb);
	if (Hwi == NULL) {
		System_abort("HWI create failed");
	}

	Hwi_enableInterrupt(INT_GPIOK_TM4C129);

	GPIOIntEnable(GPIO_PORTH_BASE, GPIO_INT_PIN_0);
	GPIOIntEnable(GPIO_PORTH_BASE, GPIO_INT_PIN_1);
	GPIOIntEnable(GPIO_PORTK_BASE, GPIO_INT_PIN_4);
	GPIOIntEnable(GPIO_PORTK_BASE, GPIO_INT_PIN_5);
	GPIOIntEnable(GPIO_PORTK_BASE, GPIO_INT_PIN_6);
	GPIOIntEnable(GPIO_PORTK_BASE, GPIO_INT_PIN_7);

	// END

	// Interrupt for WATCHDOG - START
	/*
	Watchdog_Params params;
	Watchdog_Handle watchdog;

	(void) Board_initWatchdog();
	*/
	/* Create and enable a Watchdog with resets enabled */
	/*
	Watchdog_Params_init(&params);
	params.callbackFxn = watchdog_method;
	params.resetMode = Watchdog_RESET_OFF;

	watchdog = Watchdog_open(Board_WATCHDOG0, &params);
	if (watchdog == NULL) {
		System_abort("Error opening Watchdog!\n");
	}
	*/
	// END

	// Mailbox - START
	/* Mailbox_Params sectorIndexMailboxParams;
	Mailbox_Params_init(&sectorIndexMailboxParams);
	Error_Block errorBlock;

	globalCommInfrastructure.sectorIndexMailbox = Mailbox_create(sizeof(uint32_t), 1,
			&sectorIndexMailboxParams, &errorBlock);
	if (globalCommInfrastructure.sectorIndexMailbox == NULL) {
		System_abort("creating mailbox for sectorIndex failed!\n");
	} */
	// END

}
