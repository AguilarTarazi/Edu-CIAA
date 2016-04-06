/* Copyrigth 2016
 * Con una switch prendemos un LED externo.
 * Con otro switch prendemos otro LED externo.
 * Con una entrada GPIO prendemos TODOS los LEDS de la placa.
 */

#include "chip.h"
#include "os.h"               /* <= operating system header */
#include "ciaaPOSIX_stdio.h"  /* <= device handler header */
#include "ciaaPOSIX_string.h" /* <= string header */
#include "ciaak.h"            /* <= ciaa kernel header */
#include "project_leds_gpio.h"         /* <= own header */

static int32_t fd_out, fd_in;


//==================================================//
// Funciones //
void configurar_salidas(void);
void configurar_entradas(void);

int main(void){

	StartOS(AppMode1); //Inicia el Sistema Operativo

	return 0;
}

void ErrorHook(void)
{
   ciaaPOSIX_printf("ErrorHook was called\n");
   ciaaPOSIX_printf("Service: %d, P1: %d, P2: %d, P3: %d, RET: %d\n", OSErrorGetServiceId(), OSErrorGetParam1(), OSErrorGetParam2(), OSErrorGetParam3(), OSErrorGetRet());
   ShutdownOS(0);
}

TASK(InitTask){

	//Inicializa el Kernel y los dispositivos
	ciaak_start();

	//Abrimos un archivo donde estan todas las salidas y le decimos que lo vamos a LEER y ESCRIBIR
	fd_out = ciaaPOSIX_open("/dev/dio/out/0",ciaaPOSIX_O_RDWR);

	//Abrimos el archivo donde estan todas las entradas y le decimos que lo vamos a LEER
	fd_in = ciaaPOSIX_open("/dev/dio/in/0", ciaaPOSIX_O_RDONLY);

	//Llamadas a las funciones del ususario
	configurar_salidas();
	configurar_entradas();


	SetRelAlarm(ActivateSwitchesTask, 20, 50);
	SetRelAlarm(ActivateInputsTask, 40, 60);

	TerminateTask();
}

TASK(SwitchesTask){
	uint8_t inputs;

	//Leemos 1 byte del fd_input
	ciaaPOSIX_read(fd_in, &inputs, 1);

	if(inputs==11){
		Chip_GPIO_SetPinState(LPC_GPIO_PORT, 5,8,0); //Apagamos a la derecha
		Chip_GPIO_SetPinState(LPC_GPIO_PORT, 2,8,1); //Encendemos a la izquierda
	}
	if(inputs==7){
		Chip_GPIO_SetPinState(LPC_GPIO_PORT, 2,8,0); //Apagamos a la izquierda
		Chip_GPIO_SetPinState(LPC_GPIO_PORT, 5,8,1); //Encendemos a la derecha
	}

	TerminateTask();
}

TASK(InputsTask){
	uint8_t outputs;
	boolean value;

	//Leemos el valor del pin de entrada
	value = Chip_GPIO_ReadPortBit( LPC_GPIO_PORT, 3, 6);

	//Si el pin esta en alto, encendemos los leds
	if(value==1){
		outputs=63;
		ciaaPOSIX_write(fd_out, &outputs, 1);
	}

	//Si estan en bajo, apagamos los leds
	if(value==0){
		outputs=0;
		ciaaPOSIX_write(fd_out, &outputs, 1);
	}

	TerminateTask();
}

void configurar_salidas(){
	//Configuramos el puerto 6.12 como GPIO
	//Configuramos el puerto 3.1 como GPIO (pin CAN_RD)
	//PinMux(puerto, pin, modo, funcion)
	Chip_SCU_PinMux(3,1,SCU_MODE_INACT | SCU_MODE_ZIF_DIS, FUNC4);
	Chip_SCU_PinMux(6,12,SCU_MODE_INACT | SCU_MODE_ZIF_DIS, FUNC0);

	//Configura el GPIO 2 pin 8 como SALIDA
	//SetDir(puerto, puertoGPIO, pinGPIO, salida)
	Chip_GPIO_SetDir(LPC_GPIO_PORT, 5, (1<<8), 1);
	Chip_GPIO_SetDir(LPC_GPIO_PORT, 2, (1<<8), 1);

	//Inicializacion del estado del pin
	Chip_GPIO_SetPinState(LPC_GPIO_PORT, 5, 8, 0);
	Chip_GPIO_SetPinState(LPC_GPIO_PORT, 2, 8, 0);
}

void configurar_entradas(){
	//Configuramos el puerto 6.10 como INPUT con PULL UP
	Chip_SCU_PinMux(6,10, SCU_MODE_PULLUP | SCU_MODE_ZIF_DIS | SCU_MODE_INBUFF_EN, FUNC0);

	//Configuramos el GPIO 3 pin 6 como entrada
	Chip_GPIO_SetDir(LPC_GPIO_PORT, 3, (1<<6), 0);

}
