/*
 * Publisher.c
 *
 *  Created on: 03.06.2016
 *      Author: Patrik Szabo
 */
#include <ti/sysbios/BIOS.h>
#include <xdc/runtime/Error.h>
#include <xdc/runtime/System.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/ndk/inc/netmain.h>
#include <ti/ndk/inc/_stack.h>
#include <Communication.h>
#include <myinformation.h>
#include <TrackSupervisor.h>
#include <string.h>

#define  socklen_t int
#define  sockopt_t char
#define  sockopt_t int

int getLocalUdpSocketReceive(SOCKET *socketDescriptor) {

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
		if (socketfd == -1)
			continue;
		break;
	}

	freeaddrinfo(result); /* No longer needed */
	//freeaddrinfo(rp);

	int optVal = 1;
	int optValLen = sizeof(optVal);

	if (setsockopt(socketfd, SOL_SOCKET, SO_BROADCAST, (char*) &optVal,
			optValLen) == -1) {
		System_abort("calling setsockopt failed!\n");
	}

	*socketDescriptor = socketfd;
	return 0;
}

void ReceiverTask() {
	int bytesReceived = 0;
	socklen_t addr_len;
	int status;
	uint32_t val;

	char buf[20];

	char *search = ";";
	char *sector1Token;
	char *powerValue1Token;
	char *sector2Token;
	char *powerValue2Token;
	char *roundIdentifierToken;

	SOCKET udpSocket;
	while (getLocalUdpSocketReceive(&udpSocket) != 0) {
		System_printf("Getting local UDP socket failed! Retrying ...\n");
		System_flush();
		Task_sleep(5000);
	}

	struct sockaddr_in targetAddress;
	memset(&targetAddress, 0, sizeof(targetAddress));
	targetAddress.sin_family = AF_INET;
	targetAddress.sin_port = htons(44444);
	//targetAddress.sin_addr.s_addr = INADDR_BROADCAST;
	targetAddress.sin_addr.s_addr = htonl(0xC0A801FF);

	status = bind(udpSocket, (struct sockaddr *) &targetAddress,
			sizeof(targetAddress));
	if (status < 0) {
		System_printf("udpHandler: bind failed: returned: %d, error: %d\n",
				status, fdError());
		fdClose(udpSocket);
		Task_exit();
		return;
	} else {
		System_printf("Listening on Socket...\n");
	}

	System_printf("Waiting for packets on UDP port 44444 ...\n");
	System_flush();

	while (1) {
		Task_sleep(500);

		//addr_len = sizeof targetAddress;
		bytesReceived = recv(udpSocket, buf, 20*sizeof(char), 0);
		if (bytesReceived > 0) {
			sector1Token = strtok(buf, search);
			powerValue1Token = strtok(NULL, search);
			sector2Token = strtok(NULL, search);
			powerValue2Token = strtok(NULL, search);
			roundIdentifierToken = strtok(NULL, search);

			System_printf("Received sector data (sector): %s, (power value): %s, (sector): %s, (power value): %s, (roundInformation): %s\n", sector1Token, powerValue1Token, sector2Token, powerValue2Token, roundIdentifierToken);
			System_flush();

			setSectorData(sector1Token, powerValue1Token, sector2Token, powerValue2Token, roundIdentifierToken);

			// TODO: This information has to be sent to TrackSupervisorTask

		} else {
			System_printf("recvfrom");
		}

	}
}

void SetupReceiverTask(Error_Block *errorBlock) {
	Task_Params publisherTaskParams;
	Task_Handle publisherTaskHandle;

	Task_Params_init(&publisherTaskParams);
	publisherTaskParams.stackSize = 1024;/*stack in bytes*/
	publisherTaskParams.priority = 3;
	publisherTaskHandle = Task_create((Task_FuncPtr) ReceiverTask,
			&publisherTaskParams, errorBlock);
	if (publisherTaskHandle == NULL) {
		System_abort("ReceiverTask creation failed");
	} else {
		System_printf("Setup for ReceiverTask finished :-)\n");
	}
}
