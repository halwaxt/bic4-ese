/*
 * TrackSupervisor.c
 *
 *  Created on: 29.04.2016
 *      Author: Thomas Halwax
 */
#include <xdc/runtime/System.h>
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>
#include <xdc/runtime/Timestamp.h>
#include <ctype.h>
#include <stdint.h>
#include <TrackSupervisor.h>

extern CommunicationInfrastructure globalCommInfrastructure;

uint32_t sectorPowerA[] = { 6 * 8192, 5 * 8192, 4 * 8192, 3 * 8192, 2 * 8192, 1
		* 8192 - 1, 0 };
uint32_t sectorPowerB[] = { 1 * 8192, 2 * 8192, 3 * 8192, 4 * 8192, 5 * 8192, 6
		* 8192 - 1, 0 };

uint32_t *sectorPower;

uint32_t sectorPosition = 0;

uint32_t getCurrentRoundIdentifier() {
	return sectorPower[6];
}

uint32_t getPowerValueBySectorId(int sectorId) {
	return sectorPower[sectorId];
}

void setSectorData(char sector1Token[], char powerValue1Token[],
		char sector2Token[], char powerValue2Token[],
		char sector3Token[], char powerValue3Token[],
		char roundIdentifierToken[]) {
	/* System_printf(
			"#### Received sector data (sector): %s, (power value): %s, (sector): %s, (power value): %s, (roundInformation): %s\n",
			sector1Token, powerValue1Token, sector2Token, powerValue2Token,
			roundIdentifierToken); */
	System_flush();
	sectorPower = malloc(7 * sizeof(uint32_t));
	if (sectorPosition == 0) {
		sscanf(powerValue1Token, "%d", &sectorPowerB[1]);
		sscanf(powerValue2Token, "%d", &sectorPowerB[2]);
		sscanf(powerValue3Token, "%d", &sectorPowerB[3]);
		sscanf(roundIdentifierToken, "%d", &sectorPowerB[6]);
		sectorPosition = 1;
		memcpy(sectorPower, sectorPowerB, 7 * sizeof(int));
	} else {
		sscanf(powerValue1Token, "%d", &sectorPowerA[1]);
		sscanf(powerValue2Token, "%d", &sectorPowerA[2]);
		sscanf(powerValue3Token, "%d", &sectorPowerA[3]);
		sscanf(roundIdentifierToken, "%d", &sectorPowerA[6]);
		sectorPosition = 0;
		memcpy(sectorPower, sectorPowerA, 7 * sizeof(int));
	}
}

void TrackSupervisorTask() {
	// const uint32_t sectorPower[] = { 3*8192, 4*8192, 5*8192, 6*8192, 7*8192, 8*8192 -1};
	uint32_t sectorIndex = 0;
	uint32_t currentPowerValue = 3 * 8192; // initial value for start

	sectorPower = malloc(7 * sizeof(uint32_t));
	memcpy(sectorPower, sectorPowerA, 7 * sizeof(int));

	if (! Mailbox_post(globalCommInfrastructure.pwmMailbox, &currentPowerValue,
			BIOS_NO_WAIT)) {
		System_printf("Could not post to pwmMailbox.\n");
		System_flush();
	}

	while (1) {
		if (Mailbox_pend(globalCommInfrastructure.sectorIndexMailbox,
				&sectorIndex, BIOS_WAIT_FOREVER)) {
			currentPowerValue = sectorPower[sectorIndex];
			if (! Mailbox_post(globalCommInfrastructure.pwmMailbox,
					&currentPowerValue, BIOS_NO_WAIT)) {
				System_printf("Could not post to pwmMailbox.\n");
				System_flush();
			} else {
				System_printf("Move to .. %d - use %d\n", sectorIndex,
						currentPowerValue);
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
	trackSupervisorTaskHandle = Task_create((Task_FuncPtr) TrackSupervisorTask,
			&trackSupervisorTaskParams, errorBlock);
	if (trackSupervisorTaskHandle == NULL) {
		System_abort("TrackSupervisorTask creation failed.");
	} else {
		System_printf("TrackSupervisorTask created.\n");
	}
}

