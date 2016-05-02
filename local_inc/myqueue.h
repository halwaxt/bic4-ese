#include <ti/sysbios/knl/Event.h>
#include <ti/sysbios/knl/Queue.h>

Queue_Handle uartQueue;
// the event which is used to indicate the the UART has done its work with the queue element
// extern Event_Handle uartReadyEvent;

// information, hold by the queue
typedef struct Information {
	int id;
} Information;

typedef struct RoundInformation {
	unsigned int one;
	unsigned int two;
	int finished;
} RoundInformation;

// queue object with above's information
typedef struct QueueObject {
	Queue_Elem _elem;
	struct RoundInformation* myInformationPointer;
} QueueObject;

int setup_Events();
