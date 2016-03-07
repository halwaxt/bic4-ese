/*
 * CCSv6 project using TI-RTOS
 *
 */



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
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/hal/Timer.h>
#include <ti/sysbios/knl/Event.h>

/* TI-RTOS Header files */
#include <ti/drivers/UART.h>


/*Board Header files */
#include <Board.h>
#include <EK_TM4C1294XL.h>

/*application header files*/
#include <ctype.h>
#include <string.h>


int main(void)

{

    uint32_t ui32SysClock;

	/* Call board init functions. */
	ui32SysClock = Board_initGeneral(120*1000*1000);
	(void)ui32SysClock; // We don't really need this (yet)



    /* SysMin will only print to the console upon calling flush or exit */

    System_printf("Starting Autonomous Carrera/Driver Challenge\n");
    System_flush();

    /* Start BIOS */
    BIOS_start();

}
