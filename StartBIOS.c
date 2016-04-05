/*
 *  ======== StartBIOS.c ========
 */
/* XDCtools Header files */
#include <xdc/std.h>
#include <xdc/cfg/global.h>
#include <xdc/runtime/System.h>
#include <xdc/runtime/Error.h>
#include <xdc/runtime/Memory.h>

/* BIOS Header files */
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Semaphore.h>
#include <ti/sysbios/knl/Mailbox.h>
#include <ti/sysbios/knl/Event.h>

#include <ti/sysbios/hal/Timer.h>
#include <ti/sysbios/knl/Event.h>

/* TI-RTOS Header files */
#include <ti/drivers/UART.h>
#include <ti/drivers/PWM.h>


/*Board Header files */
#include <Board.h>
#include <EK_TM4C1294XL.h>

/*application header files*/
#include <ctype.h>
#include <string.h>

/* project specific files */
#include <DRV8301.h>



int main(void) {

    /* Call board init functions. */
	Board_initGeneral(120*1000*1000);

	Board_initGPIO();
	Board_initSPI();
	//PWM_init();

	ScheduleDrv8301SetupTask();
    /* SysMin will only print to the console upon calling flush or exit */

    System_printf("Starting Autonomous Carrera/Driver Challenge\n");
    System_flush();

    /* Start BIOS */
    BIOS_start();

}
