
#include "chip.h"
#include "os.h"               /* <= operating system header */
#include "ciaaPOSIX_stdio.h"  /* <= device handler header */
#include "ciaaPOSIX_string.h" /* <= string header */
#include "ciaak.h"            /* <= ciaa kernel header */
#include "led_serial.h"         /* <= own header */

static int32_t fd_in;
static int32_t fd_adc;
static int32_t fd_out;
static int32_t fd_uart1;
//static uint32_t Periodic_Task_Counter;

void leerGenerador();
void configurar_entradas();

int main(void)
{
   StartOS(AppMode1);
   return 0;
}
void ErrorHook(void)
{
   ciaaPOSIX_printf("ErrorHook was called\n");
   ciaaPOSIX_printf("Service: %d, P1: %d, P2: %d, P3: %d, RET: %d\n", OSErrorGetServiceId(), OSErrorGetParam1(), OSErrorGetParam2(), OSErrorGetParam3(), OSErrorGetRet());
   ShutdownOS(0);
}


TASK(InitTask)
{

   ciaak_start();
   /* open CIAA ADC */
      fd_adc = ciaaPOSIX_open("/dev/serial/aio/in/0", ciaaPOSIX_O_RDONLY);
      ciaaPOSIX_ioctl(fd_adc, ciaaPOSIX_IOCTL_SET_SAMPLE_RATE, 100000);


   fd_in = ciaaPOSIX_open("/dev/dio/in/0", ciaaPOSIX_O_RDONLY);
   fd_out = ciaaPOSIX_open("/dev/dio/out/0", ciaaPOSIX_O_RDWR);
   configurar_entradas();
   //Configuracion UART
   /* open UART connected to USB bridge (FT2232) */
      fd_uart1 = ciaaPOSIX_open("/dev/serial/uart/1", ciaaPOSIX_O_RDWR);
      /* change baud rate for uart usb */
      ciaaPOSIX_ioctl(fd_uart1, ciaaPOSIX_IOCTL_SET_BAUDRATE, (void *)ciaaBAUDRATE_115200);
      /* change FIFO TRIGGER LEVEL for uart usb */
      ciaaPOSIX_ioctl(fd_uart1, ciaaPOSIX_IOCTL_SET_FIFO_TRIGGER_LEVEL, (void *)ciaaFIFO_TRIGGER_LEVEL3);
   /* Activates the SerialEchoTask task */
  // ActivateTask(SerialEchoTask);
      ciaaPOSIX_ioctl(fd_adc, ciaaPOSIX_IOCTL_SET_CHANNEL, ciaaCHANNEL_1);
   //SetRelAlarm(ActivateInputsTask, 40, 60);
      ActivateTask(InputsTask);

   TerminateTask();
}

TASK(InputsTask)
{
	/* variables to store input/output status */
	   uint8_t inputs = 0, outputs = 0;

	   /* read inputs */
	   ciaaPOSIX_read(fd_in, &inputs, 1);

	   /* read outputs */
	   ciaaPOSIX_read(fd_out, &outputs, 1);

	   if( inputs == 7 ){
		   outputs = 4;
	   }else if( inputs == 11 ){
		   //setADC();
		   outputs = 5;
	   }else if( inputs == 13){
		   //seleccionarPulsos();
		   outputs = 6;
	   }else if( inputs == 15 ){
		   //seleccionarADC();
		   outputs = 4;
	   }
	   ciaaPOSIX_write(fd_out, &outputs, 1);

	   //Periodic_Task_Counter++;


	   /*PEDRO*/
	//leerGenerador();
	boolean value;
	char buf;   /* buffer for uart operation              */
	//uint8_t outputs;
	//Leemos el valor del pin de entrada
	value = Chip_GPIO_ReadPortBit( LPC_GPIO_PORT, 3, 6);

	if(value==1){
		buf=49;
		//buf[1]=0;
		outputs=63;
		//ciaaPOSIX_write(fd_out, &outputs, 1);
		ciaaPOSIX_write(fd_uart1, buf, 1);
	}
	else if(value==0){
		buf=48;
		outputs=0;
		//ciaaPOSIX_write(fd_out, &outputs, 1);
		ciaaPOSIX_write(fd_uart1, buf, 1);
		}
   TerminateTask();
}
void configurar_entradas(){
	//Configuramos el puerto 6.10 como INPUT con PULL UP
	Chip_SCU_PinMux(6,10, SCU_MODE_PULLUP | SCU_MODE_ZIF_DIS | SCU_MODE_INBUFF_EN, FUNC0);

	//Configuramos el GPIO 3 pin 6 como entrada
	Chip_GPIO_SetDir(LPC_GPIO_PORT, 3, (1<<6), 0);

}

void leerGenerador(){

}

