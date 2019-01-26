/********************************************************************************************
	FICHERO:  CapturaMuestrasSAMD21.c                                                    
	
	Aplicación: CapturaMuestrasSAMD21.c para hacer instrumentación virtual con Arduino
	
	Por Patricio Coronado Collado.17/07/2017.


	Para comunicar con el puerto COM se utilizan funciones y variables exportadas de 
	PuertoCom.h
*********************************************************************************************/
																								
/*********************************************************************************************
	Copyright © 2017 Patricio Coronado
	
	This file is part of CapturaMuestrasSAMD21.c

    CapturaMuestrasSAMD21.c is free software: you can redistribute it and/or 
	modify it under the terms of the GNU General Public License as published 
	by the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    CapturaMuestrasSAMD21.c is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with CapturaMuestrasSAMD21.c  If not, see <http://www.gnu.org/licenses/>

*********************************************************************************************/	
/*
	Esta cadena es la respuesta que tiene que dar el sistema que queremos controlar
	al recibir un comando SCPI *IDN.
	Obligatorio rellenar la cadena IdentificacionDelInstrumento que debe conincidir con 
	NOMBRE_DEL_SISTEMA_64B(identifiacion del instrumento) en la aplicación de Arduino
*/
char IdentificacionDelInstrumento[] ="Captura de muestras con Arduino SAMD21";
//------------------------------------------------------------------------------------------
// Ficheros include                                                          
//------------------------------------------------------------------------------------------
#include <ansi_c.h>
#include <cvirte.h>
#include <userint.h>
#include <rs232.h>
#include <utility.h>
#include <formatio.h>
#include <string.h>
#include <analysis.h>
#include "CapturaMuestrasSAMD21.h"
#include "PuertoCOM/PuertoCOM.h"
//------------------------------------------------------------------------------------------
// Constantes
//------------------------------------------------------------------------------------------
#define SI 1	  
#define NO 0
// Coordenadas de los paneles
#define PP_TOP  60	  
#define PP_LEFT 60	
// Colección de posibles valores de la variable DatoEsperado (dato esperado por el puerto COM)
#define NINGUNO 0
#define DATOS_RECIBIDOS 2
//-----------------------------------------------------------------------------------------
// Variables globales 
char Comando[STRMEDIO]; //Cadena para poner un comando antes de copiarlo en CadenaComando[]
char SParametro1[STRCORTO];	// Cadena para poner un parámetros en los comandos
int Parametro1; //Valor del parámetro 1
int PrincipalHandle; //Handler del panel principal
#define MUESTRAS 1024
int ArrayDeMuestras[MUESTRAS];//Para guardar muestras	
int IndiceArrayDeMuestras;//Para saber las muestras guardadas
//----------------------------------------------------------------------------------------
// Prototipo de funciones propias                                            
//----------------------------------------------------------------------------------------
int plotea_muestras(char *,float); 
int lee_muestras(void);//Lee en CadenaAuxiliar las muestas del DUT  
void activa_controles_principal_para_comunicar(int short);// Dimar y desdimar controles
/******************************************************************************************
		                   FUNCION PRINCIPAL                                     
*******************************************************************************************/
 int main (int argc, char *argv[])
{
	if (InitCVIRTE (0, argv, 0) == 0)
		return -1;	/* out of memory */
	// Carga el panel principal como top-level
	if ((PrincipalHandle = LoadPanel (0, "CapturaMuestrasSAMD21.uir", PRINCIPAL)) < 0)
		return -1;
	
	pcom_modo_pcom(0);
	if(pcom_abre_puerto_serie()==SIN_PUERTO)
	{
		if(ConfirmPopup ("Error de puerto COM", "No se ha podido abrir ningún puerto COM\n\n"
			"¿  Quieres intentarlo manualmente?"))
				pcom_muestra_el_panel_de_configuracion(PANEL_MODO_TOP,PP_TOP+25,PP_LEFT+500);
	}
	// Muestra el panel principal
	DisplayPanel (PrincipalHandle);
	// Coloca el panel principal en una posición determinada en la pantalla
	SetPanelAttribute (PrincipalHandle, ATTR_TOP,PP_TOP );
	SetPanelAttribute (PrincipalHandle, ATTR_LEFT,PP_LEFT);
	// Si el puerto está abierto activa los  controles que lo utilizan
	if(pcom_puerto_abierto()) 
	{
		activa_controles_principal_para_comunicar(SI);
	}
	// Si el puerto no está abierto desactiva los controles que comunican
	else 
	{
		activa_controles_principal_para_comunicar(NO);
	}
	//Muestra el panel de pcom donde se ve el tráfico por le puerto serie
	pcom_muestra_el_panel_de_mensajes(PP_TOP,PP_LEFT+830);
	// Corre la aplicación........................................................
	RunUserInterface (); 
	DiscardPanel (PrincipalHandle);
	return 0;
}
/******************************************************************************************
					FUNCIONES DEL PANEL PRINCIPAL
 	Se utiliza para salir de la aplicación.Cierra el puerto y sale de la aplicación.
*******************************************************************************************/
int CVICALLBACK panel_principal (int panel, int event, void *callbackData,
		int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_GOT_FOCUS:// || EVENT_LOST_FOCUS:
			if(pcom_puerto_abierto()) 
			{
				activa_controles_principal_para_comunicar(SI);
				SetPanelAttribute (PrincipalHandle, ATTR_TITLE,IDNinstrumento);
			}
		// Si el puerto no está abierto desactiva los controles que comunican
		else 
			{
				activa_controles_principal_para_comunicar(NO);
				SetPanelAttribute (PrincipalHandle, ATTR_TITLE,"no hay puerto abierto"); 
			}
		break;
		//case EVENT_LOST_FOCUS:
		//break;
		case EVENT_CLOSE:
			// Hay que cerrar el puerto antes de salir
			pcom_cierra_puerto_serie();
			QuitUserInterface (0);
		break;
		case EVENT_RIGHT_CLICK:
		break;
	}
	return 0;
}
/****************************************************************************************
		FUNCION QUE CARGA EL PANEL DE CONFIGURACION

****************************************************************************************/
void CVICALLBACK Configurar (int menuBar, int menuItem, void *callbackData,
		int panel)
{
	pcom_muestra_el_panel_de_configuracion(PANEL_MODO_TOP,PP_TOP+80,PP_LEFT+25);
}
/****************************************************************************************
			FUNCION QUE CARGA EL PANEL DE MENSAJES
			Para que se vean los mensajes de entrada salida por el puerto COM
****************************************************************************************/
void CVICALLBACK comunicacion_menu (int menuBar, int menuItem, void *callbackData,
		int panel)
{
	pcom_muestra_el_panel_de_mensajes(PP_TOP,PP_LEFT+530);
	
}
/****************************************************************************************
		FUNCION DE MENU PARA MOSTRAR INFORMACION DEL AUTOR
****************************************************************************************/
void CVICALLBACK About (int menuBar, int menuItem, void *callbackData,
		int panel)
{
	MessagePopup 
	   ("   Autor: ",
		"--------------------Patricio Coronado Collado-----------------------\n"
		"This is free software: you can redistribute it and/or modify\n"
    	"it under the terms of the GNU General Public License as published by\n"
    	"the Free Software Foundation, either version 3 of the License, or   \n"
    	"(at your option) any later version.");
}
/*****************************************************************************************

						PONER AQUÍ EL CÓDIGO DE CADA APLICACION		

******************************************************************************************/
/****************************************************************************************
		FUNCION PARA DIMAR O DESDIMAR LOS CONTROLES QUE COMUNICAN
		Si "Accion" vale 1 desdima los controles y se pueden usar 
		Si "Acción" vale 0 dima los controles y no se pueden usar 
*****************************************************************************************/
void activa_controles_principal_para_comunicar( int short Accion)
{

	if(Accion)  // Desbloquea/activa los controles
	{
	//	SetCtrlAttribute (PrincipalHandle,PRINCIPAL_CONTROL, ATTR_DIMMED, 0);
		
	}
	else   // Bloquea/Desactiva los controles
	{
	//	SetCtrlAttribute (PrincipalHandle,PRINCIPAL_CONTROL, ATTR_DIMMED, 1);
				
	}
}
/****************************************************************************************
 	FUNCION DEL PUERTO COM QUE SE DEFINE AQUÍ PARA QUE HAGAS LO QUE 
 	QUIERAS CUANDO RECIBAS UN DATO DE FORMA AUTOMÁTICA.
 	En"CadenaRespuesta" que es donde están los datos recibidos
*****************************************************************************************/
void pcom_datos_recibido(void)
{
	float LSB;
	switch(CadenaRespuesta[0])//Sabemos que tipo de cadena recibimos en función de la firma
	{
		case 'F':
		//	  Para recibir cadenas con otra firma
		break;	
		case 'H':
		//
		break;	
		case 'D':
			lee_muestras();
			LSB=3.3/4096.0; 
			plotea_muestras("Muestras del DAC",LSB);
		break;	
	}
}
/*************************************************************************************
	FUNCION QUE LEE DE LA CadenaRespuesta LAS MUESTRAS RECIBIDAS DE ARDUINO
	La cadena CadenaRespuesta que tiene las adquisicines del DAC empiean por 'D'
**************************************************************************************/
int lee_muestras(void)
{
	int Posicion=1;//Para rastrear la CadenaRespuesta
	int MuestrasLeidas=0;
	IndiceArrayDeMuestras=0;//Global para llevar la cuenta de las muestras leidas
	// Solo lee si no ha llegado a final de cadena
	while(CadenaRespuesta[Posicion]!='\0' && IndiceArrayDeMuestras < MUESTRAS) 
	{
		if(CadenaRespuesta[Posicion]==' ')// Tras cada espacio hay una muestra
		{
			//Va llenando ArrayDeMuestras
			sscanf (CadenaRespuesta+Posicion," %u", &ArrayDeMuestras[IndiceArrayDeMuestras++]);
			MuestrasLeidas++; // Una muestra más. Lleva la cuenta del número de muestas
		}// if(Cadena...)
			Posicion++; //Aumenta el índice
	}//	while(Caden...
	return --MuestrasLeidas;
}
/****************************************************************************************
	FUNCION QUE PRESENTA EN UNA GRÁFICA LOS DATOS DE UN TEST
*****************************************************************************************/
int plotea_muestras(char * TipoDeMuestras, float LSB)
{
	int Indice,PeriodoDeMuestreo;
	float Tiempo[MUESTRAS];//Para guardar el eje de tiempos
	float Muestras[MUESTRAS];//Muestras convertidas en tensión
	float Media=0.0;
	PeriodoDeMuestreo=ArrayDeMuestras[0];//Lee el periodo de muestreo EN MICROSEGUNDOS
	//PeriodoDeMuestreo=10;//Lee el periodo de muestreo
	IndiceArrayDeMuestras--;//Hay que descontar el periodo de muestreo que se ha leido
	
	for(Indice=0;Indice<IndiceArrayDeMuestras;Indice++)
	{
		Tiempo[Indice]=Indice*PeriodoDeMuestreo*1e-6;
		Muestras[Indice]=ArrayDeMuestras[Indice+1]*LSB;
		Media+=Muestras[Indice];
	}
	Media=Media/Indice;
	DeleteGraphPlot (PrincipalHandle, PRINCIPAL_GRAFICA, -1, VAL_IMMEDIATE_DRAW);
	PlotXY (PrincipalHandle, PRINCIPAL_GRAFICA, Tiempo,Muestras, 
		IndiceArrayDeMuestras, VAL_FLOAT, VAL_FLOAT, VAL_THIN_LINE, VAL_EMPTY_SQUARE,
			VAL_SOLID, 1, 52479);
	SetPanelAttribute ( PrincipalHandle, ATTR_TITLE, TipoDeMuestras);
return 1;
}
/****************************************************************************************
 							ENVIA UN COMANDO AL SISTEMA
*****************************************************************************************/
int CVICALLBACK numero_de_muestras (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	unsigned short Muestras;
	switch (event)
	{
		case EVENT_VAL_CHANGED:
		 	GetCtrlVal(PrincipalHandle,PRINCIPAL_NMUESTRAS,&Muestras);
			sprintf(SParametro1," %u",Muestras);
			sprintf(CadenaComando,"%s","INS:NMS"); 
			strcat(CadenaComando,SParametro1);
			ENVIAR_COMANDO_AL_SISTEMA(MOSTRAR)	
		break;
	}
	return 0;
}

/****************************************************************************************
 							ENVIA UN COMANDO AL SISTEMA
*****************************************************************************************/
int CVICALLBACK pedir_muestras (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			sprintf(CadenaComando,"%s","INS:PMS"); 
			ENVIAR_COMANDO_AL_SISTEMA(MOSTRAR)	
			break;
	}
	return 0;
}
//*****************************************  FIN  **************************************// 
