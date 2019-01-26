/*
Captura de muestras con Arduino SAMD21 captura_muestras_SAMD21.ino

Copyright (C) 2019  Patricio Coronado Collado
This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License
as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License at http://www.gnu.org/licenses .

AUTHOR: Patricio Coronado Collado
WEBSITE: 
HISTORY:
*/
#include <Arduino.h>
#include <segainvex_scpi_Serial.h> // funciones y variables de segainvex_scpi_serial
#include <avdweb_SAMDtimer.h> //Para utilizar los Timers
 /**************** CONFIGURACION DE COMANDOS *******************************/
 tipoNivel INSTRUMENTO[] = //Comandos específicos del instrumento
{
	SCPI_COMANDO(PIDEMUESTRAS,PMS,fs1)//Comando para pedir muestras
	SCPI_COMANDO(NMUESTRAS,NMS,fs2)//Comando para programar el número de muestas
	SCPI_COMANDO(TIEMPOS,TPS,fs3)//Envía al PC el tiempo de conversión y el periodo de muestreo en microsegundos 
};
 tipoNivel NivelDos[] = //Array de estructuras tipo Nivel
{
	SCPI_SUBMENU(INSTRUMENTO,INS)	
	SCPI_COMANDO(ERROR,ERR,fs243)// Envía el ultimo error
	SCPI_COMANDO(*IDN,*IDN,fs240)// Identifica el instrumento
	SCPI_COMANDO(*OPC,*OPC,fs248)// Devuelve un 1 al PC
	SCPI_COMANDO(*CLS,*CLS,fs255)// Borra la pila de errores
};
SCPI_NIVEL_RAIZ// Macro que hace la declaración obligatoria del nivel Raiz
tipoCodigoError CodigoError=
{
  // Errores del sistema SCPI 0...6
  " ",						// ERROR N. 0
  "1 Caracter no valido",                // ERROR N. 1
  "2 Comando desconocido",               // ERROR N. 2
  "3 Cadena demasiado larga",            // ERROR N. 3
  "4 Parametro inexistente",             // ERROR N. 4
  "5 Formato de parametro no valido",    // ERROR N. 5
  "6 Parametro fuera de rango",          // ERROR N. 6
  // Errores personalizados por el usuario 
  "7 La variable no se ha cambiado",		// ERROR N. 7
};	
/**************** FIN CONFIGURACION DE COMANDOS ****************************/
// CONSTANTES
#define MUESTRAS 200
// VARIABLES
int NumeroDeMuestras=80;
unsigned int Array[MUESTRAS];//Array para poner las muestras
unsigned int IndiceArray=0;
int TiempoDeConversion,TiempoDeMuestreo;//Tiempos medidos en microsegundos
short int TimeSampling=1000;
// OBJETOS
//Timer para adquisición. función que atiende a la interrupción
SAMDtimer TimerAdquisicion = SAMDtimer(4, ISR_timer_adquisicion, TimeSampling);
// PROTOTIPOS DE FUNCIONES
void configura_ADC(void); 
void calibracion_ADC_SAM(int offset, uint16_t gain);//No se utiliza
/*	pero si quieres calibrar tu ADC un ejemplo sería
	calibracion_ADC_SAM(6,2057);
	Donde 6 es para corregir el offset de cero y 2057 la pendiente
	Pero primero tienes que saber que valores son los apropiados
*/
/***************************************************************************
								SETUP
****************************************************************************/
void setup() 
{
	NOMBRE_DEL_SISTEMA_64B(Captura de muestras con Arduino SAMD21)
	analogReadResolution(12);
	SerialUSB.begin(115200);
	analogReference(AR_DEFAULT);
	//analogReference(AR_INTERNAL);
	//calibracion_ADC_SAM(15,2090);
	TimerAdquisicion.enable(true);
}
//
/***************************************************************************
								LOOP
****************************************************************************/
void loop()
{
  // Si recibe algo por el puerto serie lo procesa con segainvex_scpi_serial 
  if (SerialUSB.available()){scpi();}
}
/****************************************************************
		Comando INSTRUMENTO:PIDEMUESTRAS Ó INS:PMS
		Envía "NumeroDeMuestras" muestas al PC
*****************************************************************/
void fs1(void)
{
	 int n;
	 char UnaMuestra[32]; // Cadena auxiliar, para poner una sola muestra
	 char CadenaDeMuestras[2048];//Cadena a enviar
	 CadenaDeMuestras[0]='\0'; // Inicializa la cadena de muestras
	 UnaMuestra[0]='\0';  // Inicializa la cadena auxiliar
	 strcat (CadenaDeMuestras,"DAT");//Pone el identificador
	 sprintf (UnaMuestra," %u",TimeSampling); // Añade el periodo de muestreo a la cadena intermedia
	 strcat (CadenaDeMuestras,UnaMuestra); // Añade el TimeSampling
	 //
	 for(n=0;n < NumeroDeMuestras;n++)// Recorre el array de muestras
	 {//El formato es "Identificador espacio muestra espacio muestra ...espacio muestra'\0'"
		 sprintf (UnaMuestra," %u",Array[n]); // Añade la muestra a la cadena intermedia
		 strcat (CadenaDeMuestras,UnaMuestra); // Añade la muestra a la cadena a enviar
	 }
	 strcat (CadenaDeMuestras,"\0"); // Añade a la cadena a enviar un final de cadena
	 SerialUSB.println(CadenaDeMuestras); // Envía la cadena de muestras
}
/*****************************************************************
	Comando INSTRUMENTO:NMUESTRAS Ó INS:NMS
	Cambia el número de muestras a recibir
*****************************************************************/
void fs2(void)
{
	int ValoresPosibles[]={20,40,60,80,100,120,140,160,180,200};
	if(!cambia_variable_int_discreta_del_sistema	(&NumeroDeMuestras,ValoresPosibles,sizeof(ValoresPosibles))) 
	errorscpi(7);
}
/****************************************************************
	Comando INSTRUMENTO:TIEMPOS Ó INS:TPS
	Envía al PC los tiempos de conversión y muestreo medidos
	en microsegundos
*****************************************************************/
void fs3(void)
{
	SerialUSB.print("Tiempo de conversion= ");SerialUSB.println(TiempoDeConversion);	
	SerialUSB.print("Tiempo de muestreo= ");SerialUSB.println(TiempoDeMuestreo);	
}
/*****************************************************************
		FUNCION DE LA INTERRUPCION DEL TIMER
		Captura un dato y lo pone en el array de tamaño MUESTRAS
		Cuando rellena posición MUESTRAS-1 empieza por la 0
		También mide los tiempos de conversión y el periodo
		de muestreo real 
*****************************************************************/
void ISR_timer_adquisicion(struct tc_module *const module_inst)
{
	int short MedidaADC;
	int TiempoActual;
	static int TiempoAnterior;
//	
	TiempoActual=-micros();
	TiempoDeMuestreo=TiempoAnterior-TiempoActual;
	TiempoAnterior=TiempoActual;
//
	TiempoDeConversion=micros();
	Array[IndiceArray] = analogRead(A1);
	TiempoDeConversion=micros()-TiempoDeConversion;
	IndiceArray++;
	if(IndiceArray>MUESTRAS)IndiceArray=0;
}
/****************************************************************
		CONFIGURA EL ADC PARA CONVERSION RÁPIDA 435us
*****************************************************************/
void configura_ADC(void) 
{
	ADC->CTRLA.bit.ENABLE = 0;  // Desabilita el  ADC
	while( ADC->STATUS.bit.SYNCBUSY == 1 ); 
//
	// Divide el clock entre 64 para lecturas más rápidas. 48MHz/64 y Resolucion de 16 bits.
	ADC->CTRLB.reg = ADC_CTRLB_PRESCALER_DIV64 |ADC_CTRLB_RESSEL_16BIT;   
	ADC->AVGCTRL.reg = ADC_AVGCTRL_SAMPLENUM_8 |   // acumula 8 lecturas para promediar.
	ADC_AVGCTRL_ADJRES(0x05ul);          // Desplaza el resultado la derecha para dividir entre 8.
	ADC->SAMPCTRL.reg = 0x3f;  // Por defecto lee el canal medio ciclo de reloj. Con este valor lo aumentamos.
//
	ADC->CTRLA.bit.ENABLE = 1; // Desabilita el  ADC
	while(ADC->STATUS.bit.SYNCBUSY == 1); //Espera sincronización
/*
  PROGRAMACION POR DEFECTO POR SI SE QUIERE RESTIUIR
 ADC->CTRLB.reg = ADC_CTRLB_PRESCALER_DIV512 |    // Divide Clock by 512.
 ADC_CTRLB_RESSEL_10BIT;         // 10 bits resolution as default
 ADC->SAMPCTRL.reg = 0x3f;                        // Set max Sampling Time Length
 while( ADC->STATUS.bit.SYNCBUSY == 1 );          // Wait for synchronization of registers between the clock domains
  Acumulación de muestras y ajuste del resultado
 ADC->AVGCTRL.reg = ADC_AVGCTRL_SAMPLENUM_1 |    // 1 sample only (no oversampling nor averaging)
                    ADC_AVGCTRL_ADJRES(0x0ul);   // Adjusting result by 0
  */
}
/********************************************************************
FUNCION QUE HACE LA CALIBRACION TOCANDO LOS REGISTROS DEL SISTEMA
********************************************************************/
void calibracion_ADC_SAM(int offset, uint16_t gain)
{
  // Set correction values
  ADC->OFFSETCORR.reg = ADC_OFFSETCORR_OFFSETCORR(offset);
  ADC->GAINCORR.reg = ADC_GAINCORR_GAINCORR(gain);

  // Enable digital correction logic
  ADC->CTRLB.bit.CORREN = 1;
  while(ADC->STATUS.bit.SYNCBUSY);
} 
/*******************************************************************/











