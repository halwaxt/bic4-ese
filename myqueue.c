#include <ti/sysbios/knl/Event.h>
#include <ti/sysbios/knl/Queue.h>
#include <xdc/runtime/Error.h>
#include <xdc/runtime/System.h>

Event_Handle uartReadyEvent;

int setup_Events() {
	Error_Block eb;
	/* Default instance configuration params */

	uartReadyEvent = Event_create(NULL, &eb);
	if (uartReadyEvent == NULL) {
		System_abort("UartReadyEvent create failed");
	}

	return (0);
}
