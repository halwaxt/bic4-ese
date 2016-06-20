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

#include "myinterrupt.h"
#include "hashmap.h"
#include "Communication.h"
#include "myinformation.h"

#define F_CPU 120000000 // 120 * 1000 * 1000

uint32_t state = 0;
SensorInformation myInformation;
QueueObject myQueueObject;

uint32_t s1 = 1;
uint32_t s2 = 2;
uint32_t s3 = 3;
uint32_t s4 = 4;
uint32_t s5 = 5;
uint32_t s6 = 6;

int i = 2;

extern CommunicationInfrastructure globalCommInfrastructure;
extern uint32_t tickCount;

extern uint32_t lastInterruptTick;

#define KEY_MAX_LENGTH (256)
#define KEY_PREFIX ("")
#define KEY_COUNT (2)

typedef struct data_struct_s {
	char key_string[KEY_MAX_LENGTH];
	int number;
} data_struct_t;

int error;
map_t mymap;
char key_string[KEY_MAX_LENGTH];
data_struct_t* value;

void initTickCount() {
	tickCount = 0;
}

/**
 * usage: this method sets up the environment to make sure that interrupts (hw) are working
 * @method initializeInterrupts
 * @author: patrik.szabo
 * @param *none*
 * @return *none*
 */
void initializeInterrupts() {
	IntMasterEnable();
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOH);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOK);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_WDOG0);

	initTickCount();
}

uint32_t possibleDebounce() {
	if ((tickCount - lastInterruptTick) > 5) {
		return 1;
	}
	return 0;

}

void setTickVariables() {
	lastInterruptTick = tickCount;
}

void postSectorIndexEvent(uint32_t* s) {
	if (!Mailbox_post(globalCommInfrastructure.sectorIndexMailbox, s,
	BIOS_NO_WAIT)) {
		System_printf("failed to post duty value to pwm mailbox.\n");
	}

}

void postSectorDataEvent(uint32_t id, uint32_t powerValue, uint32_t roundIdentifier) {
	myInformation.id = id;
	myInformation.ticks = tickCount;
	myInformation.powerValue = powerValue;
	myInformation.roundIdentifier = roundIdentifier;
	if (!Mailbox_post(globalCommInfrastructure.sectorDataMailbox, &myInformation,
	BIOS_NO_WAIT)) {
		System_printf("failed to post duty value to pwm mailbox.\n");
	}
}

void isr_1_2_sensor_method(UArg arg0) {
	state = GPIOIntStatus(GPIO_PORTH_BASE, true);
	GPIOIntClear(GPIO_PORTH_BASE, GPIO_INT_PIN_0 | GPIO_INT_PIN_1);

	snprintf(key_string, KEY_MAX_LENGTH, "%c%d", 'H', state);
	if (((hashmap_get(mymap, key_string, (void**) (&value)) == 0) && possibleDebounce())) {
		setTickVariables();
		System_printf("Finished Sektor %d\n", value->number);
		postSectorIndexEvent(&(value->number));
		postSectorDataEvent(value->number, (uint32_t) 0, (uint32_t) 0);
	}
	else {
		if(arg0 == 1) {
			setTickVariables();
			if(i == 1) {
				i = 2;
			}
			else {
				i = 1;
			}
			postSectorIndexEvent(&i);
			postSectorDataEvent(i, (uint32_t) 0, (uint32_t) 0);
		}
	}
}

void isr_3_6_sensor_method(UArg arg0) {
	state = GPIOIntStatus(GPIO_PORTK_BASE, true);
	GPIOIntClear(GPIO_PORTK_BASE,
	GPIO_INT_PIN_4 | GPIO_INT_PIN_5 | GPIO_INT_PIN_6 | GPIO_INT_PIN_7);

	snprintf(key_string, KEY_MAX_LENGTH, "%c%d", 'K', state);
	if ((hashmap_get(mymap, key_string, (void**) (&value)) == 0) && possibleDebounce()) {
		setTickVariables();
		System_printf("Finished Sektor %d\n", value->number);
		postSectorIndexEvent(&(value->number));
		postSectorDataEvent(value->number, (uint32_t) 0, (uint32_t) 0);
	}
}

void tick(void) {
	tickCount++;
	/* if(tickCount%500 == 0) {
		isr_1_2_sensor_method(1);
	} */
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

void pushMap(char pin, uint32_t interuppt, uint32_t sector) {
	value = malloc(sizeof(data_struct_t));
	snprintf(value->key_string, KEY_MAX_LENGTH, "%c%d", pin, interuppt);
	value->number = sector;
	error = hashmap_put(mymap, value->key_string, value);
}

void setupInterruptMap() {
	mymap = hashmap_new();

	// SETUP H1
	pushMap('H', GPIO_INT_PIN_0, 1);

	// SETUP H2
	pushMap('H', GPIO_INT_PIN_1, 2);

	// SETUP K4
	pushMap('K', GPIO_INT_PIN_4, 3);

	// SETUP K5
	pushMap('K', GPIO_INT_PIN_5, 4);

	// SETUP K6
	pushMap('K', GPIO_INT_PIN_6, 5);

	// SETUP K7
	pushMap('K', GPIO_INT_PIN_7, 6);

	/*
	 // CHECK H1
	 snprintf(key_string, KEY_MAX_LENGTH, "%c%d", 'H', GPIO_INT_PIN_0);
	 error = hashmap_get(mymap, key_string, (void**) (&value));
	 System_printf("\nERROR: %d\n", error);
	 System_printf("\nVALUE1: %d\n", value->number);

	 // CHECK H2
	 snprintf(key_string, KEY_MAX_LENGTH, "%c%d", 'H', GPIO_INT_PIN_1);
	 error = hashmap_get(mymap, key_string, (void**) (&value));
	 System_printf("\nERROR: %d\n", error);
	 System_printf("\nVALUE2: %d\n", value->number);
	 */

	/* Now, destroy the map */
	// hashmap_free(mymap);
	System_flush();

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

	setupInterruptMap();

	myQueueObject.myInformationPointer = &myInformation;

// END

}
