#include <ti/sysbios/knl/Mailbox.h>

extern uint32_t tickCount;
extern Mailbox_Handle sectorIndexMailbox;

void initializeInterrupts();
void setup_Interrupts();
int setup_Clock_Task(uint32_t wait_ticks);
