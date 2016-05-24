#include <ti/sysbios/knl/Event.h>
#include <ti/sysbios/knl/Queue.h>

Queue_Handle uartQueue;
// the event which is used to indicate the the UART has done its work with the queue element
// extern Event_Handle uartReadyEvent;

// information, hold by the queue
typedef struct SensorInformation {
	uint32_t id;
	uint32_t ticks;
	uint32_t powerValue;
	uint32_t roundIdentifier;
} SensorInformation;

// queue object with above's information
typedef struct QueueObject {
	Queue_Elem _elem;
	struct SensorInformation* myInformationPointer;
} QueueObject;

int setup_Events();
