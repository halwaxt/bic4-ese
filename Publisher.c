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
#include <TrackSupervisor.h>

extern CommunicationInfrastructure globalCommInfrastructure;


unsigned char *serialize_uint32(unsigned char *buffer, uint32_t value)
{
  /* Write big-endian int value into buffer; assumes 32-bit int and 8-bit char. */
  buffer[0] = value >> 24;
  buffer[1] = value >> 16;
  buffer[2] = value >> 8;
  buffer[3] = value;
  return buffer + 4;
}

unsigned char *serialize_char(unsigned char *buffer, char value)
{
  buffer[0] = value;
  return buffer + 1;
}

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

	 int s = getaddrinfo(NULL, "55555", &hints, &result);
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

	 int optVal = 1;
	 int optValLen = sizeof(optVal);

	if (setsockopt(socketfd, SOL_SOCKET, SO_BROADCAST, (char*)&optVal, optValLen) == -1) {
		System_abort("calling setsockopt failed!\n");
	}

	 *socketDescriptor = socketfd;
	 return 0;
}




void PublisherTask() {
	const int MAX_SECTORS = 3;
	//SensorInformation track[MAX_SECTORS];
	SensorInformation currentTrackData;
	//int currentSectorIndex = 0;
	int udpTrackPacketLength = MAX_SECTORS * sizeof(SensorInformation);
	int bytesSent = 0;
	unsigned char *sendBuffer = malloc(8 * sizeof(unsigned int) + 2 * sizeof(unsigned char));
	unsigned char *startOfBuffer = sendBuffer;


	SOCKET udpSocket;
	while (getLocalUdpSocket(&udpSocket) != 0) {
		System_printf("Getting local UDP socket failed! Retrying ...\n");
		System_flush();
		Task_sleep(5000);
	}

	struct sockaddr_in targetAddress;
	memset(&targetAddress, 0, sizeof(targetAddress));
	targetAddress.sin_family = AF_INET;
	targetAddress.sin_port = htons(55555);
	targetAddress.sin_addr.s_addr = htonl(0xFFFFFFFF);

	System_printf("Waiting for sector data to broadcast on UDP port 55555 ...\n");
	System_flush();

	while (1) {
		if (Mailbox_pend(globalCommInfrastructure.sectorDataMailbox, &currentTrackData, BIOS_WAIT_FOREVER)) {

			System_printf("Send sector data (sector): %d, (ticks): %d, (power value): %d, (roundInformation): %d\n", currentTrackData.id, currentTrackData.ticks, currentTrackData.powerValue, currentTrackData.roundIdentifier);
			System_flush();

			// invalid sector index
			if ((currentTrackData.id - 1) >= MAX_SECTORS) {
				System_printf("Sector index is out of range: %d\n", currentTrackData.id - 1);
				System_flush();
				continue;
			}

			sendBuffer = serialize_uint32(sendBuffer, currentTrackData.id);
			sendBuffer = serialize_uint32(sendBuffer, currentTrackData.ticks);
			sendBuffer = serialize_uint32(sendBuffer, getCurrentRoundIdentifier());
			sendBuffer = serialize_uint32(sendBuffer, getPowerValueBySectorId(currentTrackData.id));

			if (currentTrackData.id == MAX_SECTORS) {
				bytesSent = sendto(udpSocket, startOfBuffer, 16 * MAX_SECTORS, 0, (struct sockaddr *)&targetAddress, sizeof(targetAddress));
				if (bytesSent <= 0) {
					System_printf("Failed to send track via UDP broadcast.\n");
				}
				else {
					System_printf("Sent %d bytes via UDP.\n", bytesSent);
				}
				System_flush();

				// reset
				sendBuffer = startOfBuffer;

			}


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
