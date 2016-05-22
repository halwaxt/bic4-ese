/*
 * Publisher.c
 *
 *  Created on: 02.05.2016
 *      Author: Thomas Halwax
 */
#include <xdc/runtime/Error.h>
#include <xdc/runtime/System.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/BIOS.h>
#include <Publisher.h>
#include <Communication.h>

extern CommunicationInfrastructure globalCommInfrastructure;

void PublisherTask() {

	SectorData sectorData;

	while (1) {
		if (Mailbox_pend(globalCommInfrastructure.sectorDataMailbox, &sectorData, BIOS_WAIT_FOREVER)) {
			System_printf("Publishing sector data (power value): %d\n", sectorData.powerValue);
			System_flush();
		}
	}
}


void SetupPublisherTask(Error_Block *errorBlock) {
	Task_Params publisherTaskParams;
	Task_Handle publisherTaskHandle;

	Task_Params_init(&publisherTaskParams);
	publisherTaskParams.stackSize = 1024;/*stack in bytes*/
	publisherTaskParams.priority = 3;
	publisherTaskHandle = Task_create((Task_FuncPtr)PublisherTask, &publisherTaskParams, errorBlock);
	if (publisherTaskHandle == NULL) {
		System_abort("PublisherTask creation failed");
	}
	else {
		System_printf("Setup for PublisherTask finished :-)\n");
	}
}
