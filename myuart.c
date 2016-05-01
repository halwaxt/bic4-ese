#include <stdio.h>
#include <xdc/std.h>
#include <stdbool.h>
#include <xdc/runtime/System.h>
#include <xdc/runtime/Error.h>

#include <ti/drivers/UART.h>
#include <ti/drivers/GPIO.h>
#include <driverlib/pin_map.h>
#include <ti/sysbios/knl/Mailbox.h>
#include <myqueue.h>
#include <Board.h>
#include <driverlib/sysctl.h>
#include <ti/sysbios/BIOS.h>

#include <ti/sysbios/knl/Task.h>
#include <driverlib/gpio.h>
#include <Communication.h>

#include "myinterrupt.h"

extern CommunicationInfrastructure globalCommInfrastructure;

/**
 * usage: this method sets up the environment to make sure the UART interface can be used
 * @method initializeUART
 * @author: patrik.szabo
 * @param *none*
 * @return *none*
 */
void initializeUART() {
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
	GPIOPinConfigure(GPIO_PA0_U0RX);
	GPIOPinConfigure(GPIO_PA1_U0TX);
	GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
	UART_init();
}

/**
 * usage: this method does the work, checkes the queue, writes to the UART and post's
 * 		  the Event
 * @method uart_method
 * @author: patrik.szabo
 * @param arg0 - not used param for the task
 * @return *none*
 */
void uart_method(UArg arg0) {

	// char input;
	UART_Handle uart;
	UART_Params uartParams;

	/* Create a UART with data processing off. */
	UART_Params_init(&uartParams);
	uartParams.writeDataMode = UART_DATA_TEXT;
	uartParams.readDataMode = UART_DATA_TEXT;
	uartParams.readReturnMode = UART_RETURN_FULL;
	uartParams.readEcho = UART_ECHO_OFF;
	uartParams.baudRate = 9600;
	uart = UART_open(Board_UART0, &uartParams);

	if (uart == NULL) {
		System_abort("Error opening the UART");
	}

	// QueueObject* queueObjectPointer;

	const uint32_t powerValues[] = {50, 100, 25, 75, 100, 25};
	uint32_t ip = 0;

	while (1) {
		if (Mailbox_pend(globalCommInfrastructure.sectorIndexMailbox, &ip, BIOS_WAIT_FOREVER)) {
			System_printf("Use: %d\n", powerValues[ip]);
			System_flush();
		}
	}
}

/**
 * usage: this method creates the UART task, interacting with the queue,
 * 		  which is filled in the i2c_method
 * @method setup_UART_Task
 * @author: patrik.szabo
 * @param *none*
 * @return 0, if everything succeeded
 */
int setup_UART_Task(void) {
	Task_Params taskUARTParams;
	Task_Handle taskUART;
	Error_Block eb;

	Error_init(&eb);
	Task_Params_init(&taskUARTParams);
	taskUARTParams.stackSize = 1024; // stack in bytes
	taskUARTParams.priority = 15; // 15 is default 16 is highest priority -> see RTOS configuration
	taskUART = Task_create((Task_FuncPtr) uart_method, &taskUARTParams, &eb);
	if (taskUART == NULL) {
		System_abort("TaskUART create failed");
	}

	return (0);
}
