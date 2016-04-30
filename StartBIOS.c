/*
 *  ======== StartBIOS.c ========
 */
/* XDCtools Header files */
#include <xdc/std.h>
#include <stdint.h>
#include <ctype.h>
#include <stdbool.h>

#include <xdc/cfg/global.h>
#include <xdc/runtime/System.h>
#include <xdc/runtime/Error.h>
#include <xdc/runtime/Memory.h>

/* BIOS Header files */
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Semaphore.h>
#include <ti/sysbios/knl/Mailbox.h>
#include <ti/sysbios/knl/Event.h>
#include <ti/sysbios/knl/Clock.h>
#include <ti/sysbios/hal/Timer.h>
#include <ti/sysbios/knl/Task.h>

/* TI-RTOS Header files */
#include <ti/drivers/UART.h>
#include <ti/drivers/PWM.h>


/*Board Header files */
#include <Board.h>
#include <EK_TM4C1294XL.h>

/*application header files*/
#include <ctype.h>
#include <string.h>
#include <ti/ndk/inc/netmain.h>
#include <ti/ndk/inc/_stack.h>

/* project specific files */
#include <Communication.h>
#include <DRV8301.h>
#include <TrackSupervisor.h>

static char *StrBusy  = "\nConsole is busy\n\n";
static char *StrError = "\nCould not spawn console\n\n";

static SOCKET   scon  = INVALID_SOCKET;
static HANDLE   hConsole = 0;
#define         INMAX  32
static char     InBuf[INMAX];
static int      InIdx = 0;
static int      InCnt = 0;

static uint32_t globalSectorIndex = 0;

CommunicationInfrastructure globalCommInfrastructure;


int printfToClient(const char *format, ...)
{
   va_list ap;
   char    buffer[128];
   int     size;

   va_start(ap, format);
   size = NDK_vsprintf(buffer, (char *)format, ap);
   va_end(ap);

   send( scon, buffer, size, 0 );
   return( size );
}

void closeClientConsole()
{
    HANDLE hTmp;

    if( hConsole )
    {
        hTmp = hConsole;
        hConsole = 0;

        /* Close the console socket session. This will cause */
        /* the console app thread to terminate with socket */
        /* error. */
        fdCloseSession( hTmp );
    }
}

char getCharFromConsole()
{
    char   c;
    struct timeval timeout;

    /* Configure our console timeout to be 5 minutes */
    timeout.tv_sec  = 5 * 60;
    timeout.tv_usec = 0;

    while( 1 )
    {
        while( !InCnt )
        {
            fd_set ibits;
            int    cnt;

            FD_ZERO(&ibits);
            FD_SET(scon, &ibits);

            /* Wait for io */
            cnt = fdSelect( (int)scon, &ibits, 0, 0, &timeout );
            if( cnt <= 0 )
                goto abort_console;

            /* Check for input data */
            if( FD_ISSET(scon, &ibits) )
            {
                /* We have characters to input */
                cnt = (int)recv( scon, InBuf, INMAX, 0 );
                if( cnt > 0 )
                {
                    InIdx = 0;
                    InCnt = cnt;
                }
                /* If the socket was closed or error, major abort */
                if( !cnt || (cnt<0 && fdError()!=EWOULDBLOCK) )
                    goto abort_console;
            }
        }

        InCnt--;
        c = InBuf[InIdx++];

        if( c != '\n' )
            return( c );
    }

abort_console:
    ConsoleClose();

    fdClose( scon );
    TaskExit();

    return(0);
}

int getStringFRomConsole( char *buf, int max, int echo )
{
    int idx=0, eat=0;
    char c;

    while( idx < (max-1) )
    {
        c = getCharFromConsole();

        /* Eat char if we're eating */
        if( eat )
        {
            if( eat == 27 && c == 79 )
                eat = 1;
            else
                eat = 0;
            continue;
        }

        /* Start eating if this is an extended char */
        if( !c )
        {
            eat = 255;
            continue;
        }

        /* Start eating if this is an escape code */
        if( c == 27 )
        {
            eat = 27;
            continue;
        }

        /* Back up on backspace */
        if( c == 8 )
        {
            if( idx )
            {
                idx--;
                printfToClient("%c %c",8,8);
            }
            continue;
        }

        /* Return on CR */
        if( c == '\r' )
            break;

        buf[idx++] = c;
        /*
        if( echo == CGSECHO_INPUT )
           ConPrintf("%c",c);
        else if( echo == CGSECHO_PASSWORD )
           ConPrintf("*");
        */
    }

    buf[idx] = 0;
    return( idx );
}

void console( SOCKET sCon, PSA pClient )
{

    uint   tmp;
    char   tstr[80];
    char   *tok[10];
    int    i,connected=0;

    fdOpenSession( TaskSelf() );

    /* Get our socket */
    scon = sCon;

    /* Close the console */
       printfToClient("\nHallo AC/DC Projekt!\n");

       connected = 1;
       while (connected) {
    	   //ConGetString(tstr, 5, 1);
    	   getStringFRomConsole(tstr, 5, 1);
    	   if (stricmp( tstr, "exit") == 0) {
    		   connected = 0;
    	   }
    	   else if (stricmp(tstr[0], 'P') == 0) {
    		   printfToClient("setting power level to: %s\n", tstr);
    	   }
    	   else {
    		   printfToClient("unknown command: %s\n", tstr);
    	   }
       }



       /* Close console thread */
       closeClientConsole();

       fdClose( scon );
       TaskExit();
}


SOCKET TelnetClientHandler(PSA pClient) {
	  HANDLE fd1, fd2;

	    // Create the local pipe - abort on error
	    if( pipe( &fd1, &fd2 ) != 0 )
	        return( INVALID_SOCKET );

	    /* If an instance is already running, abort */
	    if( hConsole )
	    {
	        /* If the console is already running, return a quick message and */
	        /* close the pipe. */
	        send( fd2, StrBusy, strlen(StrBusy), 0 );
	        fdClose( fd2 );
	    }
	    else
	    {
	        /* Create the console thread */
	        hConsole = TaskCreate( console, "Console", OS_TASKPRINORM, 0x1000,
	                               (UINT32)fd2, (UINT32)pClient, 0 );

	        /* Close the pipe and abort on an error */
	        if( !hConsole )
	        {
	            send( fd2, StrError, strlen(StrError), 0 );
	            fdClose( fd2 );
	        }
	    }

	    /* Return the local fd */
	    return( fd1 );
}


void onClockElapsed(UArg arg) {
	globalSectorIndex++;
	if (globalSectorIndex > 5) globalSectorIndex=0;


	if (! Mailbox_post(globalCommInfrastructure.sectorIndexMailbox, &globalSectorIndex, BIOS_NO_WAIT)) {
	    System_printf("failed to post duty value to pwm mailbox.\n");
	    System_flush();
	}
}

int main(void) {

    /* Call board init functions. */
	Board_initGeneral(120*1000*1000);

	Board_initGPIO();
	Board_initSPI();
	Board_initEMAC();
	PWM_init();

	Error_Block errorBlock;
	Error_init(&errorBlock);


	Mailbox_Params pwmMailboxParams;
	Mailbox_Params_init(&pwmMailboxParams);

	globalCommInfrastructure.pwmMailbox = Mailbox_create(sizeof(uint32_t), 1, &pwmMailboxParams, &errorBlock);
	if (globalCommInfrastructure.pwmMailbox == NULL) {
		System_abort("creating mailbox for pwm failed!\n");
	}

	Mailbox_Params sectorIndexMailboxParams;
	Mailbox_Params_init(&sectorIndexMailboxParams);

	globalCommInfrastructure.sectorIndexMailbox = Mailbox_create(sizeof(uint32_t), 1, &sectorIndexMailboxParams, &errorBlock);
	if (globalCommInfrastructure.sectorIndexMailbox == NULL) {
		System_abort("creating mailbox for sector indexes failed!\n");
	}

	Clock_Params clockParameters;
	Clock_Params_init(&clockParameters);
	clockParameters.period = 250; /* ms */
	clockParameters.startFlag = TRUE;

	Clock_create((Clock_FuncPtr)onClockElapsed, 1000, &clockParameters, NULL);

	SetupPwmControllerTask(&errorBlock);
	SetupTrackSupervisorTask(&errorBlock);


    /* SysMin will only print to the console upon calling flush or exit */

    System_printf("Starting Autonomous Carrera/Driver Challenge\n");
    System_flush();

    /* Start BIOS */
    BIOS_start();

}
