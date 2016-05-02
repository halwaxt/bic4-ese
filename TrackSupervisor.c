/*
 * TrackSupervisor.c
 *
 *  Created on: 29.04.2016
 *      Author: Thomas Halwax
 */
#include <xdc/runtime/System.h>
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>
#include <ctype.h>
#include <stdint.h>
#include <TrackSupervisor.h>

extern CommunicationInfrastructure globalCommInfrastructure;

void TrackSupervisorTask() {
	const uint32_t sectorPower[] = { 3*8192, 4*8192, 5*8192, 6*8192, 7*8192, 8*8192 -1};
	uint32_t sectorIndex = 0;
	uint32_t currentPowerValue = 24; // initial value for start

	if (! Mailbox_post(globalCommInfrastructure.pwmMailbox, &currentPowerValue, BIOS_NO_WAIT)) {
		System_printf("Could not post to pwmMailbox.\n");
		System_flush();
	}

	while(1) {
		if (Mailbox_pend(globalCommInfrastructure.sectorIndexMailbox, &sectorIndex, BIOS_WAIT_FOREVER)) {
			currentPowerValue = sectorPower[sectorIndex];
			if (! Mailbox_post(globalCommInfrastructure.pwmMailbox, &currentPowerValue, BIOS_NO_WAIT)) {
				System_printf("Could not post to pwmMailbox.\n");
			    System_flush();
			}
		}
	}


}



void SetupTrackSupervisorTask(Error_Block *errorBlock) {

	Task_Params trackSupervisorTaskParams;
	Task_Handle trackSupervisorTaskHandle;

	Task_Params_init(&trackSupervisorTaskParams);
	//trackSupervisorTaskParams.stackSize = 1024;/*stack in bytes*/
	trackSupervisorTaskParams.priority = 7;
	trackSupervisorTaskHandle = Task_create((Task_FuncPtr)TrackSupervisorTask, &trackSupervisorTaskParams, errorBlock);
	if (trackSupervisorTaskHandle == NULL) {
		System_abort("TrackSupervisorTask creation failed.");
	}
	else {
		System_printf("TrackSupervisorTask created.\n");
	}
}

