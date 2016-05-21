/*
 * Communication.h
 *
 *  Created on: 29.04.2016
 *      Author: Thomas Halwax
 */

#ifndef LOCAL_INC_COMMUNICATION_H_
#define LOCAL_INC_COMMUNICATION_H_

#include <ti/sysbios/knl/Mailbox.h>

typedef struct CommunicationInfrastructure {
	Mailbox_Handle pwmMailbox;
	Mailbox_Handle sectorIndexMailbox;
	Mailbox_Handle sectorDataMailbox;
} CommunicationInfrastructure;

typedef struct SectorData {
	uint32_t sectorIndex;
	uint32_t tickCount;
	uint32_t powerValue;
} SectorData;


#endif /* LOCAL_INC_COMMUNICATION_H_ */
