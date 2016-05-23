/*
 * Publisher.c
 *
 *  Created on: 02.05.2016
 *      Author: Thomas Halwax
 */
#include <ti/sysbios/BIOS.h>
#include <xdc/runtime/Error.h>
#include <xdc/runtime/System.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/ndk/inc/netmain.h>
#include <ti/ndk/inc/_stack.h>
#include <Communication.h>
#include <Publisher.h>
#include <myinformation.h>

extern CommunicationInfrastructure globalCommInfrastructure;


int getLocalUdpSocket(SOCKET *socketDescriptor) {

	 struct addrinfo hints;
	 struct addrinfo *result, *rp;
	 SOCKET socketfd = 0;
	 /* Requested length of sequence */
	 /* Start of granted sequence */

	 fdOpenSession(Task_self());

	 memset(&hints, 0, sizeof(struct addrinfo));
	 hints.ai_family = AF_UNSPEC;
	 hints.ai_socktype = SOCK_DGRAM;
	 hints.ai_protocol = IPPROTO_UDP;

	 int s = getaddrinfo(NULL, "44444", &hints, &result);
	 if (s != 0) {
		 System_abort("calling getaddrinfo failed!\n");
	 }

	 for (rp = result; rp != NULL; rp = rp->ai_next) {
		 socketfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
	     if (socketfd == -1) continue;
	     break;
	 }

	 freeaddrinfo(result);           /* No longer needed */
	 //freeaddrinfo(rp);

	 *socketDescriptor = socketfd;
	 return 0;
}


void PublisherTask() {

	SectorData sectorData;
	SOCKET udpSocket;
	if (getLocalUdpSocket(&udpSocket) == 0) {
		int optVal = 1;
		int optValLen = sizeof(optVal);

		if (setsockopt(udpSocket, SOL_SOCKET, SO_BROADCAST, (char*)&optVal, optValLen) == -1) {
			System_abort("calling setsockopt failed!\n");
		}

		struct sockaddr_in targetAddress;
		memset(&targetAddress, 0, sizeof(targetAddress));
		targetAddress.sin_family = AF_INET;
		targetAddress.sin_port = htons(44444);
		targetAddress.sin_addr.s_addr = htonl(0xFFFFFFFF);

		int counter = 0;
		while(1) {
			sectorData.powerValue = 23412;
			sectorData.sectorIndex = ++counter;
			sectorData.tickCount = counter;

			int sectorDataLen = sizeof(SectorData);

			int bytesSent = sendto(udpSocket, &sectorData, sectorDataLen, 0, (struct sockaddr *)&targetAddress, sizeof(targetAddress));
			if (bytesSent > 0) {
				System_printf("Publishing sector data (power value): %d\n", bytesSent);
				System_flush();
			}
			Task_sleep(5000);
		}
	}
	while (1) {
		if (Mailbox_pend(globalCommInfrastructure.sectorDataMailbox, &sectorData, BIOS_WAIT_FOREVER)) {
			// TODO: use correct fields for sectorData
			//System_printf("Publishing sector data (indicator): %c, (sector): %d, (ticks): %d, (power value): %d, (roundInformation): %d\n", sectorData.indicator, sectorData.id, sectorData.ticks, sectorData.powerValue, sectorData.roundIdentifier);
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
