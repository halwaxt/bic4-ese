/*
 * TrackSupervisor.h
 *
 *  Created on: 29.04.2016
 *      Author: Thomas Halwax
 */

#ifndef LOCAL_INC_TRACKSUPERVISOR_H_
#define LOCAL_INC_TRACKSUPERVISOR_H_

#include <xdc/runtime/Error.h>
#include <Communication.h>

void SetupTrackSupervisorTask(Error_Block *errorBlock);

void setSectorData(char sector1Token[], char powerValue1Token[], char sector2Token[], char powerValue2Token[], char roundIdentifierToken[]);
uint32_t getCurrentRoundIdentifier();
uint32_t getPowerValueBySectorId(int sectorId);

#endif /* LOCAL_INC_TRACKSUPERVISOR_H_ */
