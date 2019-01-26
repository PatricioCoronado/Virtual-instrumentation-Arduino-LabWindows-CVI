#ifndef __PuertoCOM_H__
#define __PuertoCOM_H__
/**************************************************************************************
 Fichero: PuertoCom.h
 Próposito: Implementa variables y funciones para manejar fácilmente la 
 conexión a un puerto COM de un instrumento con la librería para 
 Arduino Segainvex_scpi_Serial (no es imprescindible).
 
 INSTRUCCIONES DE USO:
 
 Para utilizar este módulo hay que incluir PuertoCOM.h en la compilación
 además de estar en el proyecto los módulos listaPaco.


 V1.1 Incluye guardar el setup del puerto com en un fichero del que lee
 los datos para abrir el puerto.
 
 Conexión con instrumentos:
 
 Instrumento conectado en un puerto COM que responde a *IDN:
 La configuración del puerto serie y del instrumento debe ser la del panel
 de configuración:
 baudrate 115200, 8 bits de datos, 2 bits de stop,sin protocolo hard. ni soft.
 Si se conecta un instrumento que responde al comando SCPI *IDN enviando una 
 cadena de identificación, lo más sencillo es rellenar la cadena de 
 identificación:
 char IdentificacionDelInstrumento[] ="Identificación del sistema";
 en la aplicación principal y  ejecutar pcom_abre_puerto_serie.Esta función 
 busca un fichero de configuración con la última configuración que funcionó
 e intenta aplicarlo. Si abre el puerto e identifica correctamente ya está.
 Si no, busca el instrumento en todos los puertos serie activos del PC hasta
 que lo encuentra y abre el puerto. Si la función no consigue abrir un 
 puerto devueve -1.
 
 Instrumentos conectado en un puerto COM que no responde a *IDN:
 Si el instrumento está programado con la librería Segainvex_scpi_Serial
 responderá al comando *IDN devolviendo su cadena de identificación.
 Pero también se puede conectar un instrumento  sin Segainvex_scpi_Serial.
 Si no sabemos en que puerto está el instrumento cargamos el panel de 
 configuración: pcom_muestra_el_panel_de_configuracion();
 El modo inicial de funcionamiento es "usuario", solo puede cambiar "Retardo"
 y "Timeout" del puerto serie y el resto de controles están dimados.
 Para empezar en modo master hay que ejecutar pcom_modo_pcom(0);
 y luego pcom_muestra_el_panel_de_configuracion() para mostrar el panel
 de configuración y buscar el puerto donde este conectado el instrumento.
 También se puede empezar en modo "usuario" y hacer doble click con el botón
 derecho del ratón en el led y pasar a modo "master" para poder cambiar
 todos los controles del panel de la configuración del puerto y igualarla
 a la del instrumento que tengamos conectado. También podemos poner la 
 configuración que coincida con la del puerto del instrumento y pulsar "buscar 
 puertos" y seleccionar alguno de los que encuentre y pulsar "aplicar".
 
 
 La recepción automática está activada por defecto, por lo que al enviar un
 comando que provoque una respuesta del instrumento, se ejecutará la función
 void pcom_datos_recibido(void), que tenemos que definir en el código de cada
 aplicación. En esta aplicación hay que discriminar el dato que es y procesarlo.
 Los datos recibidos automáticamente están en char CadenaRespuesta[], esta 
 cadena esta terminada con TERMINADOR y '\0'. TERMINADOR es configurable,
 por defecto es '\n'.

 Si queremos enviar un dato y leer la respuesta de forma no automática, hay que
 desactivar la recepción automática.
 
 Código para enviar un comando y recibir respuesta:

 	char DatosRecibidos[256]; //Cadena para recibir datos del instrumento
    //
 	pcom_recepcion_automatica(NO); //Evita la recepción automática	
	sprintf(CadenaComando,"%s","*IDN"); //Pone el comando en la cadena de salida       
 	ENVIAR_COMANDO_AL_SISTEMA(MOSTRAR) //Envia el comando en la cadena de salida       
 	pcom_recepcion_automatica(SI); //Habilita la recepción automática	
	pcom_recibir_datos (DatosRecibidos,1);//Que rellena la cadena DatosRecibidos
	//Ya podemos procesar los datos recibidos en DatosRecibidos
	
 En la ventana de mensajes, haciendo doble click con el botón derecho del ratón
 se cierra y abre el puerto serie. Esto debe usarse solo para depuración.
	
 */
/*
	Copyright © 2017 Patricio Coronado
	
	This file is part of Proyecto PuertoCOM.

    PRY is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    PRY is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with CMM.  If not, see <http://www.gnu.org/licenses/>
*/
/*************************************************************************************
							CONSTANTES EXPORTADAS
**************************************************************************************/
	
#define STRLARGO 1024 // Longitud de una cadena larga	
#define STRCORTO 64 // Longitud de una cadena corto	
#define STRMEDIO 256// Longitud de una cadena media
#define PM_TOP  25		// Coordenadas para posicionar el panel de Mensajes
#define PM_LEFT 550
#define MOSTRAR 1
#define NO_MOSTRAR 0
#define SIN_PUERTO -1//Para indicar que no se ha abierto el puerto
/*************************************************************************************
							VARIABLES EXPORTADAS
**************************************************************************************/
extern char IdentificacionDelInstrumento[];  //Se rellena el el programa principal con el nombre del instrumento
extern char IDNinstrumento[STRMEDIO];// Cadena para leer la identificación que manda el instrumento por el puerto  
//Al buscar un instrumento, la aplicación compara las dos cadenas anteriores
extern char CadenaComando[STRMEDIO];   // Aquí se pone el comando a enviar al instrumento
extern char CadenaRespuesta[STRLARGO]; // Aquí esta la respuesta del instrumento
extern float Retardo;		// Tiempo de retardo entre envío y recepción Delay(Retardo)

//*************************************************************************************
// MACROS			   
// Envía un comando err? al sistema sin mostrarlo en el control que muestra las cadenas de salida
#define	PEDIR_ERROR_AL_SISTEMA(MOSTRAR_O_NO) sprintf(CadenaComando,"%s","ERR?");\
								if(pcom_puerto_abierto())	{ \
					pcom_enviar_datos(CadenaComando,MOSTRAR_O_NO);  \
					Delay (Retardo);}
//Envía por el puerto serie lo que se haya puesto en el array de char "CadenaComano"
//sin mostrarlo en el control que muestra las cadenas de salida 
#define ENVIAR_COMANDO_AL_SISTEMA(MOSTRAR_O_NO) if(pcom_puerto_abierto()){ \
				pcom_enviar_datos(CadenaComando,MOSTRAR_O_NO);		   \
				Delay (Retardo);								   \
			}
/**************************************************************************************
							FUNCIONES PUBLICAS
***************************************************************************************/

/***************************************************************************************
	ENTRADA PRINCIPAL: PARA ABRIR UN PUERTO COM CON EL INSTRUMENTO CONECTADO
	------------------------------------------------------------------------------
	BUSCA EL SISTEMA ENTRE LOS PUERTOS DEL PC Y SI LO ENCUENTRA ABRE EL PUERTO 
	SERIE Y CONFIGURA EL PROTOCOLO Y TIMEOUT.
	Intenta abrir el puerto a partir de los datos de un fichero. Si fracasa
	intenta abrir todos los puertos de 0 a 255.Con la configuración 
	del panel de configuración. Si puede abrir alguno, le envia
	un comando de identificación. Y si la cadena devuelta es la esperada, 
	IdentificacionDelInstrumento[],termina de configurar el puerto.
	DEVUELVE: El número del puerto COM abierto.Si no consigue abrir 
	un puerto devuelve -1 (SIN_PUERTO) y PuertoCOM=-1
*/
int pcom_abre_puerto_serie(void);
/**************************************************************************************
	FUNCION PARA PONER EN PANTALLA EL PANEL DE CONFIGURACION
	Muestra el panel con los parámetros de configuración del puerto
	ARGUMENTOS DE ENTRADA: el primer int es el modo del panel (ver abajo los #define)
	Los otros dos argumentos son las coordenas top y left para posicionar el panel
	Hacer doble click con el botón derecho del ratón en el led CAMBIAR ENTRE MODO MASTER
	Y USUARIO.
*/
#define PANEL_MODO_HIJO 0//Para instalar el panel de configuración del puerto modo hijo
#define PANEL_MODO_TOP 1 //Para instalar el panel de configuración del puerto modo top
void pcom_muestra_el_panel_de_configuracion(int/*PANEL_MODO_HIJO o PANEL_MODO_TOP*/,int,int);

/***************************************************************************************
	FUNCION VIRTUAL A DEFINIR EN EL PROGRAMA PRINCIPAL
	Esta función es llamada cuando se detecta "Terminador2" en el buffer de entrada.
	Se ejecuta si está activada la opción de recibir datos automáticamente.	
	Los datos recibidos están en "CadenaRespuesta". Este array de char finaliza 
	con:'\n' y '\0'.
	TERMINADOR PARA LA RECEPCIÓN AUTOMÁTICA: Serial.println() de Arduino funciona así:
	prints data to the serial port as human-readable ASCII text followed by a carriage 
	return character (ASCII 13, or '\r' or CR) and a newline character (ASCII 10, or
	'\n' or LF).
	La cadena recibida de Serial.println() termina  con "13 10" ó CR LF ó \r\n, 
	por eso la recepción automática se dispara con \n. la función que lee quita  
	\n y deja  \r que sirve para detectar el final de cadena. 
	En esta función, se busca \r en la cadena y cuando se encuentra se sustituye
	por \n y se añade \0.
*/	
	#define TERMINADOR '\n'  //es LF ó 10
	#define TERMINADOR2 '\r' //es CR ó 13

extern void pcom_datos_recibido(void);
/**************************************************************************************
	FUNCION QUE ACTIVA O DESACTIVA LA RECEPCION AUTOMATICA DEL PUERTO COM
	El puerto tiene que estar abierto. 
	Si se activa la recepción automática, cuando entra un dato por el 
	puerto COM con el "Terminador2" se ejecuta la función "pcom_datos_recibido".
	Esta función la tiene que definir el usuario para que se haga lo que el quiera
	al recibir un dato. Los datos recibidos están en "CadenaRespuesta". Este array 
	de char finaliza con:'\n' y '\0'.
	ARGUMENTO DE ENTRADA: int 1 para activar o int 0 para desactivar
	DEVUELVE:El Puerto COM para el que activa la recepción automática.Si el puerto
	no está abierto devuelve el código de error -3.
*/
int pcom_recepcion_automatica(int);
/***************************************************************************************                                        
	FUNCION PARA RECIBIR DATOS                                                                                                  
	El puerto tiene que estar abierto. 
	Lee datos del buffer buscando el terminador de recepción seleccionado en el panel de
	"configuración del puerto COM" del puerto.
	ARGUMENTOS DE ENTRADA: La cadena es para que la rellene con los datos recibidos
	El segundo argumento  pone a 1 para que muestre los datos en el control de recepción
	en el panel "comunicacion" y a cero para que no.
	DEVUELVE: El número de bytes leidos o el código de error si se produce.Si el puerto
	no está abierto devuelve el código de error -3.
*/
int pcom_recibir_datos(char[],short int);
/***************************************************************************************                                        
	FUNCION PARA ENVIAR DATOS                                                                                                   
	El puerto tiene que estar abierto. 
	Envía la cadena pasada como primer argumento por el puerto serie
	Envía datos poniendo al final  el terminador de transmisión seleccionado en el panel
	de "configuración del puerto COM" del puerto.
	ARGUMENTOS DE ENTRADA: La cadena con los datos a enviar
	El segundo argumento  pone a 1 para que muestre los datos en el control de transmisión
	en el panel "comunicacion" y a cero para que no.
	DEVUELVE: El número de bytes enviados o el código de error si se produce.
	Si el puerto no está abierto devuelve el código de error -3.
*/                  
int pcom_enviar_datos (char[],int);
/*****************************************************************************************
	FUNCIONES QUE DEVUELVE 1 si el puerto está abierto y 0 si no
*/
int pcom_puerto_abierto(void);//Devuelve 1 si el puerto está abierto 0 si no
/*****************************************************************************************
	FUNCION QUE CIERRA EL PUERTO: Hay que cerrar el puerto antes de salir de la aplicación
*/
void pcom_cierra_puerto_serie(void);
/*****************************************************************************************
	 FUNCION QUE LIMPIA LA PILA DE ERRORES SCPI
	 Es conveniente limpiar la pila de errores al principio de la aplicación
*/
void pcom_limpia_pila_errores(void);
/*****************************************************************************************
	FUNCION PARA PONER EN PANTALLA EL PANEL DE MENSAJES
	Muestra el panel en el que se ve el tráfico del puerto serie
	ARGUMENTOS DE ENTRADA: Los argumentos son las coordenas top y left para
	posicionar el panel.
*/
void pcom_muestra_el_panel_de_mensajes(int,int);
/*****************************************************************************************      
	FUNCION PARA INICIALIZAR LAS VARIABLES Y PANELES DEL PUERTO
	pcom  por defecto está en modo USUARIO. Pero podemos inicializarlo para que empiece
	en modo MASTER ejecutando esta función.
	ARGUMENTOS DE ENTRADA: 0 para pasar a modo MASTER y 1 para modo USUARIO
	
*/                                                                                              
int pcom_modo_pcom(int short);                                       
/*****************************************************************************************      
	FUNCIONES QUE DEVUELVE: 1 cuando se produce recepción automática de cadena 
	por el puerto serie. Cada vez que se ejecuta se borra la información, si se ejecuta
	de nuevo y no ha recibido un dato automático por el puerto serie devuelve 0.
*/                                                                                              
int short com_flag_dato_recibido (void);                                                          
/******************************************************************************************/
#endif  /* ndef __PuertoCOM_H__ */  

