/*
 * DRV8301.h
 *
 *  Created on: 07.03.2016
 *      Author: Thomas Halwax
 */

#ifndef LOCAL_INC_DRV8301_H_
#define LOCAL_INC_DRV8301_H_

#include <ti/sysbios/knl/Mailbox.h>
#include <Communication.h>

#define MODE_READ  0b1000000000000000
#define MODE_WRITE 0b0000000000000000

#define REGISTER_STATUS_1 0b0000000000000000
#define REGISTER_STATUS_2 0b0000100000000000

#define REGISTER_CONTROL_1 0b0001000000000000
#define REGISTER_CONTROL_2 0b0001100000000000


#define REGISTER_CONTROL_1_GATE_CURRENT_1700 0b0000000000000000
#define REGISTER_CONTROL_1_GATE_CURRENT_0700 0b0000000000000001
#define REGISTER_CONTROL_1_GATE_CURRENT_0250 0b0000000000000010

#define REGISTER_CONTROL_1_PWM_MODE_6P 0b0000000000000000
#define REGISTER_CONTROL_1_PWM_MODE_3P 0b0000000000001000

#define REGISTER_CONTROL_1_OC_ADJ_SET_0097 0b0000000100000000


void SetupPwmControllerTask(Error_Block *errorBlock);

#endif /* LOCAL_INC_DRV8301_H_ */
