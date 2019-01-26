/*
 Fichero:PuertoCom.c
 Próposito: Implementa variables y funciones para manejar fácilmente la conexión      
 con un puerto COM.
 
 Para utilizar este módulo hay que incluir PuertoCOM.h en la compilación
 además de estar en el proyecto los módulos listaPaco.c y listaPaco.h

 Para abrir el puerto hay que ejecutar abre_puerto_automaticamente.
 Esta función abre el puerto COM que le devuelva la identificación
 correcta. Si no abre el puerto devuelve -1 (SIN_PUERTO).
 
 TO DO
 
 Mejor gestión de errores
*/
/*
	Copyright © 2017 Patricio Coronado
	
	This file is part of PROYECTO ó PuertoCom.c.

    PuertoCom.c is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    PuertoCom.c is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with CMM.  If not, see <http://www.gnu.org/licenses/>
*/
//==============================================================================
// Include files
#include <userint.h>
#include <utility.h>
#include <formatio.h>
#include <ansi_c.h>
#include "PuertoCOM.h"
#include "ErroresRS232.h"
#include <rs232.h>
#include "PanelesPuertoCom.h" 
#include "listaPaco.h"
//----------------------------------------------------------------------------
#define  ERROR_DATOS_EXCESIVOS -9// Código no utilizado en la libreria RS-232
#define MASTER 0
#define USUARIO 1
#define TECLA_ENTER 1280
#define SI 1
#define NO 0
//--------------------------------------------------------------------------
//Variables privadas
//Handlers
int short ModoMasterUsuario=USUARIO; //Para tener funciones extendidas MASTER
listaPaco sListaCOM; // Lista para anotar los puertos encontrados de puertos
int MensajesHandle=0; // Manipuladores de los paneles que usa el puerto COM
int ConfigHandle=0;
int	EstatusHandle=0;
int EsperaHandle=0;
int ModoPanelConfiguracion;
int short FlagDatosRecibidos=NO;//Se pone a SI cuando se produce recepción automática
static int FlagPanelConfigRecienAbierto=1;// Para reconfigurar o no el panel cada vez que se abra  
// Variables privadas del puerto serie
//-PARAMETROS DEL PUERTO QUE DEBEN COINCIDIR CON LOS ESTABLECIDOS EN EL PANEL--
static int short PuertoCOM;
static int VelocidadPuerto;
static int Paridad;
static int BitsDatos=8;
static int BitsDeStop=2;
static int ColaDeEntrada=512;  		// Establece la long.de la cola de recepción
static int ColaDeSalida=512; 		// Establece la long.de la cola de transmisión
static int ModoX=0;
static int ModoCTS=0;
static int TerminadorTransmision=1; // Selección del terminador de transmisión 
static int TerminadorRecepcion=1; // Seleccion del terminador de recepción 
float Timeout=1.0;
float Retardo=0.3;//Retardo tras enviar un comando esperando una respuesta
//-------------------------------------------------------------------------------
static int ErrorRS232;      //Código de error del puerto serie 
static int FlagPuertoAbierto=0;  // Flag que indica si hay algún puerto abierto  
char NombrePuertoCOM[STRCORTO];	// Nombre lógico del puerto COM1, COM2, etc.
char ComMsg[STRCORTO];
char Msg1[STRCORTO];   // Cadenas de caracteres auxiliares
char IDNinstrumento[STRMEDIO];//Cadena en la que se lee el identificador enviado por el sistema
enum enumColor {Rojo,Azul,Verde};
//==============================================================================
// Funciones privadas
int abrir_puerto(int,const char[],int,int,int,int,int,int);//Unico sitio donde se abre el puerto realmente
int cerrar_puerto(int);//Unico sitio donde se cierra el puerto realmente
int carga_panel_de_configuracion(void);//Pone en memoia el panel de configuración
int carga_panel_de_mensajes(void);//Pone en memoia el panel de mensajes
int hacer_lista_de_puertos_del_sistema(void); 
int identifica_al_sistema(void);//Lee el *IDN del sistema conectado en IDNinstrumento
int test_puerto_abierto(void);// Devuelve 0 si el puerto estáabierto y -3 si está cerrado
void cambia_de_modo_controles_de_configuracion(void);            
void cambia_de_modo_controles_de_mensajes(void);
void led_color(enum enumColor);
void salva_parametros(void);
void descarga_paneles_del_puerto_com(void);//Qita de memoria todos los paneles del puerto
void display_error_RS232(void);
void establece_parametros_com_en_panel(void);
void lee_parametros_com_del_panel (void);
void muestra_mensaje_status(int);
void CVICALLBACK terminador_detectado_en_buffer_de_entrada (int portNo,int eventMask,void *callbackData); 
//==============================================================================
// Variables globales
// Cadenas públicas de comunicación con el programa principal
char CadenaComando[STRMEDIO];   // Aquí se pone el comando a enviar al instrumento
char CadenaRespuesta[STRLARGO]; // Aquí esta la respuesta del instrumento
//==============================================================================       
/****************************************************************************************
VARIABLES Y FUNCIONES PARA GUARDAR Y RECUPERAR DE UN FICHERO LOS DATOS DEL PUERTO SERIE 
*****************************************************************************************/
int ErrorFichero;
ssize_t Dummy;
FILE *ptrFichero;
static char NombreFichero[]="C:\\LabWindowsCVI\\PuertoCOM.ini";
struct strPuertoCOM
{
	char IDNinstrumento[STRMEDIO];
	char NombrePuertoCOM[STRCORTO];
	int  PuertoCOM;
	int  VelocidadPuerto;
	int  Paridad;
	int  BitsDatos;
	int  BitsDeStop;
	int  ColaDeEntrada;
	int  ColaDeSalida;
	int	 ModoX;
	int	 ModoCTS;
	float Timeout;
	int TerminadorTransmision; 
	int TerminadorRecepcion;
	float Retardo;
};
//Estructura global en el módulo con los datos del puerto que hay en fichero
struct strPuertoCOM filePuertoCOM;
//Funciones privadas en el módulo para manejar el fichero con los datos del puerto
int guarda_puerto_com_en_fichero(void);//Salva la configuración actual en un fichero
int lee_puerto_com_de_fichero(void);
int abre_puerto_desde_fichero(void);//Funcion principal
//
/************************************************************************************
	ENTRADA PRINCIPAL PARA ABRIR UN PUERTO COM CON EL SISTEMA CONECTADO
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
*************************************************************************************/
int pcom_abre_puerto_serie(void) 
{
	int Indice;
	ssize_t LongMensaje;
	char CadenaComando[STRLARGO];
	//
	//Para conocer las variables del puerto...
	if(!MensajesHandle) carga_panel_de_mensajes();      
	if(!ConfigHandle) carga_panel_de_configuracion();		
	//Intenta abrir el puerto desde un fichero
	if (abre_puerto_desde_fichero()!=SIN_PUERTO)
	{
		SetCtrlAttribute (ConfigHandle,CONFIGURAP_LEDCONFIG, ATTR_ON_COLOR, VAL_GREEN);//Para saber que se ha abierto desde fichero
		return PuertoCOM;//Si tiene éxito sale ya devolviendo el puerto abierto
	}
	// Si no puede utilizar el fichero para abrir el puerto..
	SetCtrlAttribute (ConfigHandle,CONFIGURAP_LEDCONFIG, ATTR_ON_COLOR, VAL_RED);//Para saber que se ha buscado el puerto
	FlagPuertoAbierto = 0;  // inicializa el flag a "puerto si abrir"
    lee_parametros_com_del_panel (); // Lee los parámetros de configuración del puerto
	DisableBreakOnLibraryErrors (); // Función obsoleta 					
	//SetBreakOnLibraryErrors (0);// Inhabilita errores en tiempo de ejecución     
	for (PuertoCOM=1;PuertoCOM <=255;PuertoCOM++)// Recorre los puertos posibles
	{
		// Intenta abrirlos
		ErrorRS232 = abrir_puerto (PuertoCOM, NombrePuertoCOM, VelocidadPuerto, Paridad,
                                       BitsDatos, BitsDeStop, ColaDeEntrada, ColaDeSalida);//ABRE EL PUERTO
		if (ErrorRS232>=0) // Si el puerto está presente en el equipo y abre... 
		{
			FlagPuertoAbierto = 1; // Indica al sistema que el puerto está abierto
			//Termina de configurar el puerto
			SetXMode (PuertoCOM,ModoX);
            SetCTSMode (PuertoCOM, ModoCTS);
            SetComTime (PuertoCOM, Timeout);
			identifica_al_sistema();//Identifica el sistema en IDNinstrumento
			if(!strcmp (IDNinstrumento, IdentificacionDelInstrumento))//Si son iguales devuelve 0
			{  
			   pcom_recepcion_automatica(SI);//Callback para respuesta automática
			   FlagPuertoAbierto = 1; // Indica al sistema que el puerto está abierto 
			   EnableBreakOnLibraryErrors (); // Habilita errores en tiempo de ejecución
			   SetCtrlVal (ConfigHandle, CONFIGURAP_PUERTOCOM, PuertoCOM);//Actualiza el panel de configuración
			   break; // Sale del bucle for
			}
			else
			{
				cerrar_puerto (PuertoCOM); // Si no está el sistema en el puerto lo cerramos
				FlagPuertoAbierto=0;
			}
		
		}//if (ErrorRS232>=0)
	}// del for
	if (!FlagPuertoAbierto)   // Si vamos a salir si un puerto abierto...
	{
		PuertoCOM=SIN_PUERTO;
		SetCtrlVal (ConfigHandle, CONFIGURAP_PUERTOCOM, PuertoCOM);
	}
	return PuertoCOM;
}
/************************************************************************************
 		FUNCIONES PARA GUARDAR Y LEER UN FICHERO CON LOS DATOS DEL PUERTO
 *************************************************************************************/
/************************************************************************************
 GUARDA EN EL FICHERO "configuracion.com" LOS DATOS DEL PUERTO QUE SE HA ABIERTO
 Y HA FUNCIONADO
 *************************************************************************************/
int guarda_puerto_com_en_fichero(void)
{
		int Resultado;                                                            
		DisableBreakOnLibraryErrors ();
		Resultado=MakeDir ("C:\\LabWindowsCVI");                                      
		EnableBreakOnLibraryErrors ();
		if(Resultado!=0 && Resultado!=-9)                                         
		{                                                                         
			return -1;                                                            
		}                                                                         
	ptrFichero=fopen(NombreFichero,"wb");// Crea el fichero
   	if (!ptrFichero){return 0;} //Si falla la apertura sale con 0
	//Si abre el fichero rellena la estructura con los datos del fichero
    strcpy(filePuertoCOM.IDNinstrumento,IDNinstrumento);
	strcpy(filePuertoCOM.NombrePuertoCOM,NombrePuertoCOM);
	filePuertoCOM.PuertoCOM=PuertoCOM;
	filePuertoCOM.VelocidadPuerto=VelocidadPuerto;
	filePuertoCOM.Paridad=Paridad;
	filePuertoCOM.BitsDatos=BitsDatos;
	filePuertoCOM.BitsDeStop=BitsDeStop;
	filePuertoCOM.ColaDeEntrada=ColaDeEntrada;
	filePuertoCOM.ColaDeSalida=ColaDeSalida;
	filePuertoCOM.ModoX=ModoX;
	filePuertoCOM.ModoCTS=ModoCTS;
	filePuertoCOM.Timeout=Timeout;
	filePuertoCOM.PuertoCOM=PuertoCOM;
	filePuertoCOM.TerminadorTransmision=TerminadorTransmision;
	filePuertoCOM.TerminadorRecepcion=TerminadorRecepcion; 
	filePuertoCOM.Retardo=Retardo;
	// Si abre el fichero sin errores..guarda la estructura en el fichero.
	fwrite(&filePuertoCOM,sizeof(struct strPuertoCOM),1,ptrFichero);
	fclose(ptrFichero);//Cierra el fichero
	return filePuertoCOM.PuertoCOM; 
}
/************************************************************************************
 FUNCION QUE BUSCA EL FICHERO "PuertoCOM.ini" 
 DEVUELVE: Si existe el fichero, lo abre y lee con exito devuelve el nº de puerto COM
 y una estructura llamada filePuertoCOM con los datos del puerto.
 Si no hay fichero ó no se abre devuelve -1.
 *************************************************************************************/
int lee_puerto_com_de_fichero(void)
{
	DisableBreakOnLibraryErrors ();//Para evitar errores en debug  
	ErrorFichero=GetFileSize (NombreFichero, &Dummy);//Pregunta el tamaño del fichero    
	EnableBreakOnLibraryErrors ();
	if (ErrorFichero>=0)//Si no da error, el fichero existe
	{
		ptrFichero=fopen(NombreFichero,"rb");// Abre el fichero
		if (!ptrFichero){return -1;} //Si falla la apertura, sale con 0
		// Si abre el fichero sin errores lee los datos del puerto COM
		fread(&filePuertoCOM,sizeof(struct strPuertoCOM),1,ptrFichero);
		fclose(ptrFichero);//Cierra el fichero
		return filePuertoCOM.PuertoCOM; //Sale con un número de puerto
	}//if (ErrorFichero>=0)
	// Si el fichro no existe sale con 0
	else {return -1;}
}
/**************************************************************************************
   			LEE EL FICHERO DE CONFIGURACIÓN DEL PUERTO Y LO APLICA
			Abre el puerto que indica el fichero y busca el sistema indicado por:
			char IdentificacionDelInstrumento[] (se rellena en el main)			
			DEVUELVE:el número de puerto si abre el puerto donde está el sistema o
			 -1 si no hay fichero, no pudo abrir el puerto o el sistema no esta en 
			 el puerto.
***************************************************************************************/
int abre_puerto_desde_fichero(void)
{
	int Indice,FlagPuerto;
	ssize_t LongMensaje;
	int short PCOM;//Para guardar el COM que había cuando se llama a la función
	//Guardamos el PuertoCOM que tenga el sistema y el FlagPuertoAbierto
	PCOM=PuertoCOM;
	FlagPuerto=FlagPuertoAbierto; 
	// Si no encuentra el fichero o no lo puede abrir sale con -1 
	if (lee_puerto_com_de_fichero()==-1) return -1;
	// Si encuentra el fichero y lee una estructura con los datos la utiliza  
    // Abre el puerto con la nueva configuración 
    DisableBreakOnLibraryErrors ();// Inhabilita mensajes de error en tiempo de ejecución
	FlagPuertoAbierto=1;//Para enviar y recibir del puerto 
	ErrorRS232 = abrir_puerto (filePuertoCOM.PuertoCOM, filePuertoCOM.NombrePuertoCOM,// ABRE EL PUERTO
		filePuertoCOM.VelocidadPuerto,filePuertoCOM.Paridad,filePuertoCOM.BitsDatos, 
		filePuertoCOM.BitsDeStop, filePuertoCOM.ColaDeEntrada, filePuertoCOM.ColaDeSalida);
	EnableBreakOnLibraryErrors (); // Habilita mensajes de error en tiempo de ejecución
    if (ErrorRS232) //Si da error al abrir el puerto, ¡adios!
	{
		FlagPuertoAbierto=FlagPuerto; //Deja el flag como estaba al entar en la función
		return -1;//   devuelve el control 
	}
    else //(ErrorRS232 == 0) Si no da error al abrir el puerto lo configura
    {
        //Comprueba que el sistema está en el puerto
		PuertoCOM = filePuertoCOM.PuertoCOM; //Este es el puerto que hemos abierto
		//Termina de configura el puerto
		SetXMode (filePuertoCOM.PuertoCOM,filePuertoCOM.ModoX);
	    SetCTSMode (filePuertoCOM.PuertoCOM, filePuertoCOM.ModoCTS);
	    SetComTime (filePuertoCOM.PuertoCOM, filePuertoCOM.Timeout);
		TerminadorTransmision=filePuertoCOM.TerminadorTransmision; 
		TerminadorRecepcion=filePuertoCOM.TerminadorRecepcion; 
		identifica_al_sistema();//Identifica el sistema en IDNinstrumento
		if(!strcmp (IDNinstrumento, IdentificacionDelInstrumento))//Compara identificaciones, si son iguales devuelve 0
		{  // Y si es correcta termina de configurar el puerto con los datos del fichero
			// Configura la opción protocolo software 
	        SetXMode (filePuertoCOM.PuertoCOM,filePuertoCOM.ModoX);
	        // Configura la opción protocolo hardware 
	        SetCTSMode (filePuertoCOM.PuertoCOM, filePuertoCOM.ModoCTS);
	        // Configura la opción "Timeout" 
	        SetComTime (filePuertoCOM.PuertoCOM, filePuertoCOM.Timeout);
			// Indica al sistema que puerto se ha abierto
			PuertoCOM=filePuertoCOM.PuertoCOM;
			//Configura los terminadores
			TerminadorTransmision=filePuertoCOM.TerminadorTransmision; 
			TerminadorRecepcion=filePuertoCOM.TerminadorRecepcion; 
	 		//Anota el puerto en el panel de configuración
			SetCtrlVal (ConfigHandle, CONFIGURAP_PUERTOCOM, PuertoCOM);
			//Identifica al instrumento
			strcpy (IDNinstrumento, filePuertoCOM.IDNinstrumento);   
			// Activa la recepción automática para ese puerto
			pcom_recepcion_automatica(SI);
			//Actualiza las variables del puerto desde los datos del fichero
			FlagPuertoAbierto=1;
			Timeout=filePuertoCOM.Timeout;
			Retardo=filePuertoCOM.Retardo;
			VelocidadPuerto=filePuertoCOM.VelocidadPuerto;
			Paridad=filePuertoCOM.Paridad;
			BitsDatos=filePuertoCOM.BitsDatos;
			BitsDeStop=filePuertoCOM.BitsDeStop;
			ColaDeEntrada=filePuertoCOM.ColaDeEntrada;
			ColaDeSalida=filePuertoCOM.ColaDeSalida;
			ModoCTS=filePuertoCOM.ModoCTS;
			ModoX=filePuertoCOM.ModoX;
			TerminadorRecepcion=filePuertoCOM.TerminadorRecepcion;
			TerminadorTransmision=filePuertoCOM.TerminadorTransmision;
			establece_parametros_com_en_panel();//Actualiza el panel de configuración
			//sale devolviendo el puerto COM que ha abierto abierto y el flag lebantado
			return PuertoCOM;	
		}
		else // Si no está el sistema en el puerto lo cerramos
		{
			cerrar_puerto (PuertoCOM); 
			FlagPuertoAbierto=FlagPuerto; //Deja el flag como estaba al entrar en la función
			PuertoCOM=PCOM;//Deja PuertoCOM como estaba al entar en la función
			return -1;
		}
	}//else //(ErrorRS232 == 0) Si no hay errores configura el protocolo 
}
/**************************************************************************************
 		FIN FUNCIONES PARA GUARDAR Y LEER UN FICHERO CON LOS DATOS DEL PUERTO
 **************************************************************************************/
 /************************************************************************************** 
 		FUNCIONES PARA HACER UNA LISTA DE PUERTOS COM ENCONTRADOS EN EL SISTEMA
 **************************************************************************************/
/****************************************************************************************
	FUNCION QUE SE EJECUTA CUANDO SE PULSA EL BOTON DE "buscar"
	Pone un pop-up de espera y ejecuta 	com_hacer_lista_de_puertos_del_sistema(); 
	Cuando termina actualiza los controles que muestran los puertos del panel
*****************************************************************************************/
int CVICALLBACK buscar_puertos_serie (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
int Resultado,indice,ValorPuerto;
if(event==EVENT_LEFT_CLICK)
{

	if (FlagPuertoAbierto){
		cerrar_puerto(PuertoCOM);//Cierra el puerto actual antes de abrir el nuevo
		FlagPuertoAbierto=0;
	}
	SetCtrlVal (ConfigHandle, CONFIGURAP_IDENTIFICACION,"Selecciona un puerto");	
	EsperaHandle = LoadPanel (ConfigHandle, "PanelesPuertoCom.uir", ESPERA);
   	InstallPopup (EsperaHandle); // Pone el panel en memoria
	hacer_lista_de_puertos_del_sistema();
	RemovePopup (1);
	DiscardPanel (EsperaHandle);
	EsperaHandle=0;
	//anota el puerto que se ve en el RING en los controles
	GetCtrlIndex (ConfigHandle, CONFIGURAP_RINGCOM, &indice);//Lee el índice del puerto seleccionado
	Resultado=GetValueFromIndex (ConfigHandle, CONFIGURAP_RINGCOM, indice,&ValorPuerto);
	if(Resultado<0)
	{
		MessagePopup ("ERROR", "No se han encontrado puertos");
	}
	else
	{
		copia_cadena_de_lista_paco(&sListaCOM,  ValorPuerto, IDNinstrumento);
		SetCtrlVal (ConfigHandle, CONFIGURAP_IDENTIFICACION, IDNinstrumento);				
		SetCtrlVal (ConfigHandle, CONFIGURAP_PUERTOCOM, ValorPuerto); // Anota el valor del puerto seleccionado   
	}
}
	return 0;
}
/*****************************************************************************************
   SELECCIONA UN PUERTO DEL RING DONDE ESTÁN LOS PUERTOS DEL SISTEMA Y LO COLOCA
   EN EL CONTROL DEL QUE SE LEE EL PUERTO A ABRIR.
******************************************************************************************/
int CVICALLBACK seleciona_un_puerto_de_la_lista (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
int indice,ValorPuerto,Resultado;
//nodoStructPaco *pNodoCOM;
if(event== EVENT_VAL_CHANGED)
{
	GetCtrlIndex (ConfigHandle, CONFIGURAP_RINGCOM, &indice);//Lee el índice del puerto seleccionado
	Resultado=GetValueFromIndex (ConfigHandle, CONFIGURAP_RINGCOM, indice,&ValorPuerto);// Le el valor apuntado por le índice 
	if(Resultado<0)
	{
		MessagePopup ("ERROR", "No se han encontrado puertos");
	}
	else
	{
		copia_cadena_de_lista_paco(&sListaCOM,  ValorPuerto, IDNinstrumento);
		SetCtrlVal (ConfigHandle, CONFIGURAP_IDENTIFICACION, IDNinstrumento);				
		SetCtrlVal (ConfigHandle, CONFIGURAP_PUERTOCOM, ValorPuerto); // Anota el valor del puerto seleccionado   
	}
}
return 0;
}
/****************************************************************************************
	LEE LOS PUERTOS DEL SISTEMA, LOS PONE EN UNA LISTA DINÁMICA Y LOS PONE EN UN RING
	Intenta abrir todos los puertos con los parámetros del sistema conectado. Los que
	consigue abrir les mando un *IDN y lee la respuesta. Pone el número y nombre del 
	puerto, así como el identificador, lo pone en un nodo y lo añade a una lista
	listaPaco. Finalmente con los datos de la lista rellena un control RING y
	elimina la lista tipo listaPaco.
	DEVUELVE:El número de puertos encontrados.
****************************************************************************************/
int hacer_lista_de_puertos_del_sistema(void) 
{
	int short PCOM;
	int Indice;
	int status_puerto;
	int LongMensaje;
	int PuertosEncontrados;
	char CadenaComando[STRLARGO];
	char NombrePuerto[STRCORTO];
	nodoStructPaco *pNodoCOM; // Puntero auxiliar a nodo con la información de un puerto 
	PCOM=PuertoCOM;
	elimina_nodos_lista_paco(&sListaCOM); // Elimina el contenido de la lista antigua
	inicializa_lista_paco(&sListaCOM);	
	FlagPuertoAbierto = 0;  // inicializa el flag a puerto no abierto
    lee_parametros_com_del_panel(); // Lee los parámetros de configuración
	DisableBreakOnLibraryErrors (); // Inhabilita errores en tiempo de ejecución     
	PuertosEncontrados=0;
	for (PuertoCOM=0;PuertoCOM<=255;PuertoCOM++)// Recorre los puertos posibles
	{
		// Intenta abrirlos
		status_puerto = abrir_puerto (PuertoCOM, NombrePuertoCOM, VelocidadPuerto, Paridad,
                                       BitsDatos, BitsDeStop, ColaDeEntrada, ColaDeSalida);
		if (status_puerto>=0) // Si el puerto está presente en el equipo y abre... 
		{
			//Termina de configurar el puerto
			SetXMode (PuertoCOM,ModoX);    
			SetCTSMode (PuertoCOM, ModoCTS);
			SetComTime (PuertoCOM, Timeout);
			FlagPuertoAbierto=1; // Para utilizar el puerto
			identifica_al_sistema();//Lee la identidad del instrumento en IDNinstrumento			
			cerrar_puerto (PuertoCOM); // Cierra el puerto
			FlagPuertoAbierto=0;// Indica al sistema que el puerto está cerrado
			// Crea un nodo en la lista y guarda la información encontrada
			inicializa_nodo_struct_paco( &pNodoCOM); // Crea un nuevo nodo para la lista de puertos
			Fmt (NombrePuerto, "%s<COM %i", PuertoCOM);  // Crea la cadena de identificación del puerto
			strcpy (pNodoCOM->NombrePuerto, NombrePuerto); // Copia la cadena de identificación en el nodo
			if (!strcmp(IDNinstrumento,""))strcpy(IDNinstrumento,"instrumento sin identificación;");
			strcpy (pNodoCOM->NombreInstrumento, IDNinstrumento);
			pNodoCOM->NumeroPuerto=PuertoCOM; // Apunta en el nodo creado el número del puerto
			pNodoCOM->VelocidadPuerto=VelocidadPuerto; // Copia la cadena de identificación en el nodo  
			inserta_nodo_lista_paco(&sListaCOM,pNodoCOM,DETRAS);// Inserta el nodo en la lista
			PuertosEncontrados++;
		}//if (status_puerto>=0)
		
	}// del for
	// Rellena el control RINGCOM con los puertos encontrados
	SetCtrlAttribute (ConfigHandle,CONFIGURAP_RINGCOM, ATTR_DIMMED, 0);
	ClearListCtrl (ConfigHandle,CONFIGURAP_RINGCOM);// Primero limpia el control destino 
	// Rellena el control del panel para visualizar la lista
	pNodoCOM=sListaCOM.primer_nodo;// Con el puntero auxiliar apunta al primer nodo de la lista
	while (pNodoCOM!=NULL)
	{
		InsertListItem (ConfigHandle,CONFIGURAP_RINGCOM,0, pNodoCOM->NombrePuerto, pNodoCOM->NumeroPuerto); 
		pNodoCOM=pNodoCOM->siguiente;
	}	
	PuertoCOM=PCOM;
	return PuertosEncontrados;
}
/************************************************************************************** 
 		FIN FUNCIONES PARA HACER UNA LISTA DE PUERTOS COM ENCONTRADOS EN EL SISTEMA         
**************************************************************************************/
/**************************************************************************************
 		       	FUNCION DEL PANEL DE CONFIGURACION                          
				ESTA FUNCIÓN PUEDE CERRAR EL PANEL 
				Recupera la configuración realizada por el usuario	
***************************************************************************************/
int CVICALLBACK panel_configuracion (int panel, int event, void *callbackData,
		int eventData1, int eventData2)
{   
	if( event==EVENT_GOT_FOCUS)
	{   // Si el panel está recien abierto...
		if(FlagPanelConfigRecienAbierto)//Usa el flag para que esto lo haga una sola vez
		{	
			// Llama a la función que establece parámetros del panel
			establece_parametros_com_en_panel();
			FlagPanelConfigRecienAbierto=0;
		}
		// Si no hay un puerto abierto borra la identificación del sistema
		if(PuertoCOM==-SIN_PUERTO) 
		{
			SetCtrlVal (ConfigHandle, CONFIGURAP_IDENTIFICACION, "no hay puerto abierto");			
		}
		return 0;
	}//if( event==EVENT_GOT_FOCUS)	
	// Para cerrar el panel sin aplicar cambios
	if (event==EVENT_CLOSE)
	{
		FlagPanelConfigRecienAbierto=1;  
		if(ModoPanelConfiguracion==PANEL_MODO_HIJO)
		RemovePopup (0);
		if(ModoPanelConfiguracion==PANEL_MODO_TOP)
		HidePanel (ConfigHandle);
		return 0;
	}//if (event==EVENT_CLOSE)
	//Cambia entre modo Master y Usuario
	if(event==EVENT_RIGHT_CLICK)
	{
	 	//Aquí iba el cambio de modo, pero se lo he puesto al led                            
	}//if(event==EVENT_LEFT_CLICK)
return 0;
}
/************************************************************************************** 
 			FUNCIONES PARA CAMBIAR ENTRE MODO MASTER Y USUARIO
**************************************************************************************/ 
/****************************************************************************************
		FUNCION QUE REALIZA EL CAMBIO DE LA VARIABLE ModoMasterUsuario
		Y cambia los controles de los paneles a dimados no desdimados	
****************************************************************************************/
int CVICALLBACK cambiar_de_modo (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	if(event== EVENT_RIGHT_DOUBLE_CLICK)
	{
		char MensajeTopMaster[]="Estas en modo master";
		char MensajeTopUsuario[]="Estas en modo usuario";			
		char MensajeDownMaster[]="¿Quieres cambiar a modo usuario?";		
		char MensajeDownUsuario[]="¿Quieres cambiar a modo master?";
		if(ModoMasterUsuario==MASTER)
		{
			if(ConfirmPopup (MensajeTopMaster,MensajeDownMaster))
			{	
				pcom_modo_pcom(USUARIO);
				/*
				ModoMasterUsuario=USUARIO;
				cambia_de_modo_controles_de_configuracion();
				cambia_de_modo_controles_de_mensajes();
				*/
			}
		}
		else 
		{                                                                
			if(ConfirmPopup (MensajeTopUsuario,MensajeDownUsuario))         
			{
				pcom_modo_pcom(MASTER);
				/*
				ModoMasterUsuario=MASTER;
				cambia_de_modo_controles_de_configuracion();
				cambia_de_modo_controles_de_mensajes();				
				*/
			}
		}                                                                
	}//if(event==EVENT_LEFT_CLICK)
	
	return 0;
}
/******************************************************************************************
			FUNCION PARA MODIFICAR LOS CONTROLES EN UN CABIO DE MODO MASTER / USUARIO
			Actualiza la variable ModoMasterUsuario.                                  			
			Dima o desdima los controles que configuran el puerto en funcion del modo.		
*******************************************************************************************/
void cambia_de_modo_controles_de_configuracion(void)
{
	if(ConfigHandle)
	{
		SetCtrlAttribute (ConfigHandle, CONFIGURAP_CERRARPCOM, ATTR_DIMMED,ModoMasterUsuario);
		SetCtrlAttribute (ConfigHandle, CONFIGURAP_VELOCIDADPUERTO, ATTR_DIMMED,ModoMasterUsuario);
		SetCtrlAttribute (ConfigHandle, CONFIGURAP_BITSTOP, ATTR_DIMMED, ModoMasterUsuario);
		SetCtrlAttribute (ConfigHandle, CONFIGURAP_BITDATOS, ATTR_DIMMED, ModoMasterUsuario);
		SetCtrlAttribute (ConfigHandle, CONFIGURAP_PARIDAD, ATTR_DIMMED, ModoMasterUsuario);
		SetCtrlAttribute (ConfigHandle, CONFIGURAP_TERMINADORLECTURA, ATTR_DIMMED, ModoMasterUsuario);
		SetCtrlAttribute (ConfigHandle, CONFIGURAP_TERMINADORESCRITURA, ATTR_DIMMED, ModoMasterUsuario);
		SetCtrlAttribute (ConfigHandle, CONFIGURAP_COLAENTRADA, ATTR_DIMMED, ModoMasterUsuario);
		SetCtrlAttribute (ConfigHandle, CONFIGURAP_COLASALIDA, ATTR_DIMMED, ModoMasterUsuario);
		SetCtrlAttribute (ConfigHandle, CONFIGURAP_MODOCTS, ATTR_DIMMED, ModoMasterUsuario);
		SetCtrlAttribute (ConfigHandle, CONFIGURAP_MODOXONOFF, ATTR_DIMMED, ModoMasterUsuario);
		SetCtrlAttribute (ConfigHandle, CONFIGURAP_PUERTOCOM, ATTR_DIMMED, ModoMasterUsuario);
		SetCtrlAttribute (ConfigHandle, CONFIGURAP_RINGCOM, ATTR_DIMMED, ModoMasterUsuario);
		SetCtrlAttribute (ConfigHandle, CONFIGURAP_BUSCARPUERTOS, ATTR_DIMMED, ModoMasterUsuario);
	}
	//Menú
}
/****************************************************************************************** 
			FUNCION PARA MODIFICAR LOS CONTROLES DEL PANEL MENSAJES SEGUN EL MODO       
			Dima o desdima los controles del panel "mensajes" en funcion del modo.		
*******************************************************************************************/
void cambia_de_modo_controles_de_mensajes(void)                                                      
{
	int menuHandle;
	if(MensajesHandle)
	{		
		menuHandle=GetPanelMenuBar(MensajesHandle); 		                      
		SetMenuBarAttribute (menuHandle, MENUSCPI_COM, ATTR_DIMMED, ModoMasterUsuario);        
		SetMenuBarAttribute (menuHandle, MENUSCPI_MENUERROR, ATTR_DIMMED, ModoMasterUsuario);  
		SetMenuBarAttribute (menuHandle, MENUSCPI_MENUVERSION, ATTR_DIMMED, ModoMasterUsuario);
		SetMenuBarAttribute (menuHandle, MENUSCPI_MENUCLS, ATTR_DIMMED, ModoMasterUsuario);
		SetCtrlAttribute (MensajesHandle, MENSAJES_COMANDO, ATTR_DIMMED, ModoMasterUsuario);
	}
}
/*************************************************************************************** 
		FUNCION PARA INICIALIZAR LAS VARIABLES Y PANELES DEL PUERTO                                              
	pcom no necesita inicialización, por defecto está en modo USUARIO. Pero podemos                          
	inicializarlo para que empiece en modo MASTER                                                            
	ARGUMENTO DE ENTRADA:0 para modo MASTER, 1 Para modo USUARIO  
	Hay que asegurarnos de que se han cargado los 2 paneles del puerto "Configuración" 
	y "Mensajes". Luego cambia la variable global ModoMasterUsurio, y por fin dimar
	o desdimar los controles de los paneles en funcion del modo solicitado.
****************************************************************************************/
int pcom_modo_pcom(int short ModoNuevo)                                       
{                                           
	 //Carga paneles en memoria si no lo estaban  	
	if(!MensajesHandle) carga_panel_de_mensajes();
	if(!ConfigHandle) carga_panel_de_configuracion();	
	// Cambia la variable global ModoMasterUsurio en función del parámetro de entrada
	// Y cambia el estado de los paneles
	if(ModoNuevo==USUARIO)                                       
	{                                                                   
		ModoMasterUsuario=USUARIO;                                  
		cambia_de_modo_controles_de_configuracion();                
		cambia_de_modo_controles_de_mensajes();                     
		return 1;
	}                                                                   
	if(ModoNuevo==MASTER)                                       
	{                                                                   
		ModoMasterUsuario=MASTER;                                   
		cambia_de_modo_controles_de_configuracion();                
		cambia_de_modo_controles_de_mensajes();				        
		return 1;
	}
	return 0;
}   
/************************************************************************************** 
 			FIN FUNCIONES PARA CAMBIAR ENTRE MODO MASTER Y USUARIO                          
**************************************************************************************/ 
/**************************************************************************************
         FUNCION PARA APLICAR LAS OPCIONES DEL PANEL DE CONFIGURACION DEL PUERTO 
		 SI ESTÁ EN MODO USUARIO SOLO PUEDE CAMBIAR EL TIMEOUT Y EL RETARDO
		 SI ESTÁ EN MODO MASTER PUEDE CAMBIAR CUALQUIER PARÁMETRO, AL APLICAR
		 ABRE EL PUERTO COM CON LOS PARAMETROS SELECCIONADOS
***************************************************************************************/
int CVICALLBACK AplicarConfigCallback (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	if(event== EVENT_COMMIT)
	{
		lee_parametros_com_del_panel();//Vamos a aplicar los nuevos parámetros de puerto						
		if(ModoMasterUsuario==USUARIO)
		{	
			//Aplica parámetros que pueden cambiar en modo USUARIO
			if(FlagPuertoAbierto) SetComTime (PuertoCOM, Timeout);//Aplica el nuevo TimeOut
		}
		//En modo master se permite tocar todo
		if(ModoMasterUsuario==MASTER) //Cierra el puerto actual, configura el nuevo puerto y lo intenta abrir
		{	
			DisableBreakOnLibraryErrors ();// Inhabilita mensajes de error en tiempo de ejecución			
			cerrar_puerto(PuertoCOM);//Cierra el puerto actual antes de abrir el nuevo
			Delay(1);
			ErrorRS232=abrir_puerto(PuertoCOM,NombrePuertoCOM,VelocidadPuerto,Paridad,BitsDatos,BitsDeStop,ColaDeEntrada,ColaDeSalida);//ABRE EL PUERTO
			Delay(1);
    		if (ErrorRS232!=0) //Si da error al abrir el puerto....
			{                                                                                    			
				cerrar_puerto(PuertoCOM);//Por si acaso lo cierra
				FlagPuertoAbierto=0; //Deja el flag a puerto cerrado
				MessagePopup("Error","No se pudo abrir el puerto.\n No hay ningún puerto abierto."); 
				PuertoCOM=SIN_PUERTO;
				strcpy(IDNinstrumento,"no hay puerto abierto conectado");//Actualiza IDNinstrumento
			}                                                                                    			
			else
			{//Si no da errores al abrir termina de configurar el puerto
				FlagPuertoAbierto=1;
				//Leemos los parámetros de configuración del puerto
				SetXMode(PuertoCOM,ModoX);                                                
				SetCTSMode(PuertoCOM, ModoCTS);                                           
				SetComTime (PuertoCOM, Timeout);//Aplica el nuevo TimeOut  
				//Los terminadores son variables de la aplicación que ya están actualizadas
				// Si el panele de comunicación no esta en memoria lo carga
				if(!MensajesHandle) carga_panel_de_mensajes();
				// Intenta recibir una identificación del sistema
				identifica_al_sistema();//Actualiza IDNinstrumento	
				// Activa la recepción automática para ese puerto        
				pcom_recepcion_automatica(SI);                           
				//Como los parámetros del puerto aplicados son correctos, los salvamos
			}
			SetCtrlVal (ConfigHandle, CONFIGURAP_IDENTIFICACION, IDNinstrumento);				
			SetCtrlVal (ConfigHandle, CONFIGURAP_PUERTOCOM,PuertoCOM);				
		}                                                                                     			
		EnableBreakOnLibraryErrors (); // Habilita mensajes de error en tiempo de ejecución  			
		FlagPanelConfigRecienAbierto=1;
		HidePanel(ConfigHandle);       
	}//if(event== EVENT_COMMIT)
   	return 0;                      
}
/*************************************************************************************
	FUNCION QUE DA SERVICIO AL BOTON DE CERRAR EL PUERTO SERIE
**************************************************************************************/
int CVICALLBACK CerrarPuertoCallback (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
if(event==EVENT_COMMIT)
{
	cerrar_puerto(PuertoCOM);
	FlagPuertoAbierto=0;
}
	return 0;
}
/*************************************************************************************
	FUNCION SECRETA PARA ABRIR Y CERRAR EL PUERTO CON EL LED DE "MENSAJES"
	Si el puerto está abierto lo cierra y si está cerrado lo abre. Utiliza las 
	variables globales de configuración del puerto para abririlo (no del panel).
**************************************************************************************/
int CVICALLBACK abre_cierra_com (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	if(event== EVENT_RIGHT_DOUBLE_CLICK)
	{
		if (FlagPuertoAbierto)
		{
			pcom_recepcion_automatica(NO);
			cerrar_puerto(PuertoCOM);
			FlagPuertoAbierto=0;
		
		}
		else 
		{
			if(abrir_puerto(PuertoCOM, NombrePuertoCOM, VelocidadPuerto, Paridad,
            	BitsDatos, BitsDeStop, ColaDeEntrada, ColaDeSalida)==0);//ABRE EL PUERTO
			{
				FlagPuertoAbierto=1;
				pcom_recepcion_automatica(SI);//Callback para respuesta automática
			}
		}
	}
return 0;
}
/****************************************************************************************
	FUNCION QUE ENVIA UN COMANDO POR EL PUERTO SERIE CUANDO 
	SE PULSA ENTER EN EL TEXTBOX enviar
****************************************************************************************/                                
int CVICALLBACK enviar_comando (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	if(event== EVENT_KEYPRESS && eventData1==TECLA_ENTER)
{	
	GetCtrlVal(MensajesHandle, MENSAJES_COMANDO,CadenaComando);
	pcom_enviar_datos(CadenaComando,1);
}
return 0;
}
/************************************************************************************
			FUNCION QUE ACTIVA O DESACTIVA LA RECEPCION AUTOMATICA DEL PUERTO COM
			ARGUMENTO DE ENTRADA: int 1 para activar o int 0 para desactivar
			DEVUELVE:nada
*************************************************************************************/
int pcom_recepcion_automatica(int activar)
{
	static int short FuncionActiva=0;
	if(!FlagPuertoAbierto) return -3;
	{	
		if(activar /*&& !FuncionActiva*/)
		{
		InstallComCallback (PuertoCOM, LWRS_RXFLAG, 0,
			TERMINADOR,terminador_detectado_en_buffer_de_entrada, NULL);
			FuncionActiva=1;
		}
		else
		{
			if(1/*FuncionActiva*/)
			{
				InstallComCallback (PuertoCOM, 0, 0, TERMINADOR,
			 		terminador_detectado_en_buffer_de_entrada, 0);
				FuncionActiva=0;
			}
		}
	}
	return PuertoCOM;
}
/**************************************************************************************
	FUNCIONES QUE DEVUELVE FlagPuertoAbierto (1 si el puerto está abierto y 0 si no)
***************************************************************************************/
int pcom_puerto_abierto (void)
{
		return FlagPuertoAbierto; //Devuelve 1 abierto, 0 cerrado
}
/****************************************************************************************
		FUNCION QUE CARGA EL PANEL DE ESTATUS
****************************************************************************************/
void CVICALLBACK Estatus (int menuBar, int menuItem, void *callbackData,int panel)
{	
	char MensajeDeError[STRMEDIO];
	if(FlagPuertoAbierto)	
	{
		if(!EstatusHandle)EstatusHandle = LoadPanel (MensajesHandle, "PanelesPuertoCom.uir", ESTATUS);
    	InstallPopup (EstatusHandle); // Pone el panel en memoria 

	}
	else
	{
		Fmt (MensajeDeError, "%s", "Error:¡No hay ningún puerto abierto!\n\n"
			"Abra el panel de configuración\n"
               "en el menú COM/Configurar\n");
          	MessagePopup ("Mensaje RS232", MensajeDeError);
	}
}
/**************************************************************************************
 		       				FUNCIONES DEL PANEL STATUS                           
***************************************************************************************/
int CVICALLBACK cierra_panel_status (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:
			if(EstatusHandle)
			{
				DiscardPanel (EstatusHandle);
				EstatusHandle=0;
			}
			break;
		case EVENT_RIGHT_CLICK:

			break;
		}
	return 0;
}
/*****************************************************************************************
					FUNCIONES DEL PANEL MENSAJES
****************************************************************************************/
// Función del Panel
int CVICALLBACK panel_mensajes (int panel, int event, void *callbackData,
		int eventData1, int eventData2)
{
	Rect CoordenadasPanel;
	switch (event)
	{
		case EVENT_PANEL_MOVING:
			GetPanelEventRect (eventData2, &CoordenadasPanel/*Rect *rectangle*/);
			SetCtrlVal(MensajesHandle,MENSAJES_X_POS,CoordenadasPanel.top);
			SetCtrlVal(MensajesHandle,MENSAJES_Y_POS,CoordenadasPanel.left);
		break;		
		case EVENT_CLOSE:
			HidePanel(MensajesHandle);
		break;
	}
	return 0;
}
/****************************************************************************************
      				 FUNCION PARA LIMPIAR LA VENTANA DE RECEPCION          
****************************************************************************************/
int CVICALLBACK LimpiarRecibir (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{																						 
			case EVENT_COMMIT:
			ResetTextBox (MensajesHandle, MENSAJES_CADENA_RECIBIDA, "");
			break;
		}
	return 0;
}
/****************************************************************************************
      				 FUNCION PARA LIMPIAR LA VENTANA DE COMANDOS          
****************************************************************************************/
int CVICALLBACK LimpiarEnviar (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	if (event==EVENT_COMMIT)
	{
		ResetTextBox (MensajesHandle, MENSAJES_CADENA_ENVIAR,"");
	}
	return 0;
}
/*********************************************************************************************
				PONE EN MEMORIA EL PANEL DE MENSAJES COMO TOP-LEVEL
**********************************************************************************************/
int carga_panel_de_mensajes(void)
{
	#define PANEL_PADRE 0 //0 para top-level 1 para child
	if ((MensajesHandle = LoadPanel (PANEL_PADRE, "PanelesPuertoCom.uir", MENSAJES)) < 0)
		return -1;
	
	else
	{
		cambia_de_modo_controles_de_mensajes();
		return MensajesHandle;
	}	
}
/*********************************************************************************************
				MUESTRA EL PANEL DE MENSAJES
**********************************************************************************************/
void pcom_muestra_el_panel_de_mensajes(int Top,int Left)
{
	//Antes de mostrar el panel comprueba si está en memoria, si no lo carta
	if(!MensajesHandle) carga_panel_de_mensajes();
	DisplayPanel(MensajesHandle);// Muestra el panel de mensajes
	// Coloca el panel en una posición determinada en la pantalla
	SetPanelAttribute (MensajesHandle, ATTR_TOP,Top);
	SetPanelAttribute (MensajesHandle, ATTR_LEFT,Left);
	SetCtrlVal(MensajesHandle,MENSAJES_X_POS,Top);
	SetCtrlVal(MensajesHandle,MENSAJES_Y_POS,Left);
}
/*********************************************************************************************
	CARGA EL PANEL DE CONFIGURACION EN MEMORIA COMO HIJO DEL PARÁMETRO DE ENTRADA
	Parámetro de entrada: tipo int; tiene que ser un Handle de panel activo
	o 0 para cargarlo como top-level.
**********************************************************************************************/
int carga_panel_de_configuracion(void)
{
#define PANEL_PADRE 0	//0 para que sea top level 1 para que no lo sea
	if ((ConfigHandle = LoadPanel (PANEL_PADRE, "PanelesPuertoCom.uir", CONFIGURAP)) < 0)
	return -1;
	else 
	{
		//cambia_de_modo_controles_de_mensajes();
		return ConfigHandle;
	}
}
/*********************************************************************************************
			MUESTRA EL PANEL DE CONFIGURACION DEL PUERTO COM
**********************************************************************************************/
void pcom_muestra_el_panel_de_configuracion(int ModoPanel,int Top,int Left)
{
	//Antes de mostrar el panel tiene que ver si está en memoria. Si no está lo carga.
	if(!ConfigHandle) carga_panel_de_configuracion();
	lee_parametros_com_del_panel ();//Actualiza las variables del puerto desde el panel
	if(ModoPanel==PANEL_MODO_HIJO)	
	{if(ConfigHandle) InstallPopup (ConfigHandle);ModoPanelConfiguracion=PANEL_MODO_HIJO;}
	if(ModoPanel==PANEL_MODO_TOP)	
	{
		if(ConfigHandle) DisplayPanel (ConfigHandle);ModoPanelConfiguracion=PANEL_MODO_TOP;
	}
	SetPanelAttribute (ConfigHandle, ATTR_TOP,Top);
	SetPanelAttribute (ConfigHandle, ATTR_LEFT,Left);

}
/*********************************************************************************************
				QUITA DE LA MEMORIA LOS PANELES DEL PUERTO COM
**********************************************************************************************/
void descarga_paneles_del_puerto_com(void)
{
	if(ConfigHandle) {DiscardPanel (ConfigHandle);ConfigHandle=0;}
	if(MensajesHandle) {DiscardPanel (MensajesHandle);MensajesHandle=0;}
}
/*********************************************************************************************
 				LEE LOS PARAMETROS DE CONFIGURACION DEL PUERTO                            
 *********************************************************************************************/
void lee_parametros_com_del_panel (void)
{
	GetCtrlVal (ConfigHandle, CONFIGURAP_PUERTOCOM, &PuertoCOM);//Este es el puerto que queremos utilizar	
	GetCtrlVal (ConfigHandle, CONFIGURAP_TIMEOUT, &Timeout);                                                  	
	GetCtrlVal (ConfigHandle, CONFIGURAP_RETARDO, &Retardo);                                                  	
	GetCtrlVal (ConfigHandle, CONFIGURAP_MODOCTS, &ModoCTS);                                                  	
	GetCtrlVal (ConfigHandle, CONFIGURAP_MODOXONOFF, &ModoX);                                                 	
	GetCtrlIndex (ConfigHandle, CONFIGURAP_TERMINADORLECTURA, &TerminadorRecepcion);                          	
	GetCtrlIndex (ConfigHandle, CONFIGURAP_TERMINADORESCRITURA, &TerminadorTransmision);                      	
	GetCtrlVal (ConfigHandle, CONFIGURAP_COLASALIDA, &ColaDeSalida);                       			          	
	GetCtrlVal (ConfigHandle, CONFIGURAP_COLAENTRADA, &ColaDeEntrada);                     			          	
	GetCtrlVal (ConfigHandle, CONFIGURAP_BITSTOP, &BitsDeStop);                            	                  	
	GetCtrlVal (ConfigHandle, CONFIGURAP_BITDATOS, &BitsDatos);                            			          	
	GetCtrlVal (ConfigHandle, CONFIGURAP_PARIDAD, &Paridad);                                                  	
	GetCtrlVal (ConfigHandle, CONFIGURAP_VELOCIDADPUERTO, &VelocidadPuerto);                                  	
}
/********************************************************************************************
			ESTABLECE LOS PARAMETROS DE CONFIGURACION DEL PUERTO                     
*********************************************************************************************/
void establece_parametros_com_en_panel(void)
{
	
	SetCtrlVal (ConfigHandle, CONFIGURAP_TIMEOUT, Timeout);
	SetCtrlVal (ConfigHandle, CONFIGURAP_RETARDO, Retardo);
    SetCtrlVal (ConfigHandle, CONFIGURAP_PUERTOCOM, PuertoCOM);
    SetCtrlVal (ConfigHandle, CONFIGURAP_VELOCIDADPUERTO, VelocidadPuerto);
    SetCtrlVal (ConfigHandle, CONFIGURAP_PARIDAD, Paridad);
    SetCtrlVal (ConfigHandle, CONFIGURAP_BITDATOS, BitsDatos);
    SetCtrlVal (ConfigHandle, CONFIGURAP_BITSTOP, BitsDeStop);
    SetCtrlVal (ConfigHandle, CONFIGURAP_COLAENTRADA, ColaDeEntrada);
    SetCtrlVal (ConfigHandle, CONFIGURAP_COLASALIDA, ColaDeSalida);
    SetCtrlVal (ConfigHandle, CONFIGURAP_MODOCTS, ModoCTS);
    SetCtrlVal (ConfigHandle, CONFIGURAP_MODOXONOFF,ModoX);
    SetCtrlIndex (ConfigHandle, CONFIGURAP_TERMINADORLECTURA, TerminadorRecepcion);
	SetCtrlIndex (ConfigHandle, CONFIGURAP_TERMINADORESCRITURA, TerminadorTransmision);
	SetCtrlVal (ConfigHandle, CONFIGURAP_IDENTIFICACION, IDNinstrumento);				
}

/******************************************************************************************      
	FUNCIONES QUE DEVUELVE EL FLAG DE DATO RECIBIDO AUTOMÁTICAMENTE
	Devuelve 1 cuando se produce recepción automática de cadena por el puerto serie	
	Borra el flag tras ejecutarse.
*******************************************************************************************/     
int short com_flag_dato_recibido (void)                                                          
{                                                                                            
	if(FlagDatosRecibidos)
	{	
		FlagDatosRecibidos=NO;
		return SI;
	}
	return NO;
}                                                                                            
/******************************************************************************************
				FUNCION QUE MUESTRA LOS ERRORES DEL PUERTO                  
/******************************************************************************************/
void display_error_RS232 (void)
{
	int Respuesta;
	char ErrorMessage[STRLARGO];
	switch (ErrorRS232)
        {
        case 0  :
            MessagePopup ("Mensaje RS232", "No hay errores.");
            break;
        case -1  :
            //For error code -1 (UnknownSystemError), call the GetErrorRS232String 
			//function to obtain a specific Windows message string. 
			Respuesta=ConfirmPopup ("Mensaje RS232",
				"¡El puerto se desconectó!\n¿Cerrar la aplicación?");
			if(Respuesta==1)
			{
				pcom_cierra_puerto_serie();
				QuitUserInterface (0);
			}
            break;
		case -2 :
            Fmt (ErrorMessage, "%s", "número de puerto invalido (debe valer "
                                     "entre 1 y 255).");
            MessagePopup ("Mensaje RS232", ErrorMessage);
            break;
        case -3 :
            Fmt (ErrorMessage, "%s", "El puerto no está abierto.\n"
                 "Entre en el menú  Com/configurar ");
            MessagePopup ("Mensaje RS232", ErrorMessage);
            break;
		case -5 :
            Fmt (ErrorMessage, "%s", "Error en el puerto serie");
            MessagePopup ("Mensaje RS232", ErrorMessage);
        break;
		case -6 :
            Fmt (ErrorMessage, "%s", "Puerto no encontrado");
            MessagePopup ("Mensaje RS232", ErrorMessage);
        break;
        case -99 :
            Fmt (ErrorMessage, "%s", "error de Timeout.\n\n"
                 "Incremente el valor de Timeout,\n"
                 "       Compruebe la configuración, or\n"
                 "       Compruebe el puerto.");
            MessagePopup ("Mensaje RS232", ErrorMessage);
            break;
			case ERROR_DATOS_EXCESIVOS :
            Fmt (ErrorMessage, "%s", "error, demasiados datos en el buffer.\n");
            MessagePopup ("Mensaje RS232", ErrorMessage);
            break;
		default :
            if (ErrorRS232 < 0)
                {  
                Fmt (ErrorMessage, "%s<RS232 error número %i", ErrorRS232);
                MessagePopup ("Mensaje RS232", ErrorMessage);
                }
            break;
        }
}
/***************************************************************************************
								FUNCIONES DE MENU
****************************************************************************************/
/**************************************************************************************                                                   
			FUNCION QUE  MUESTRA EL NUMERO DE BYTES EN LA COLA DE ENTRADA                                                                 	 
***************************************************************************************/                                                  
int CVICALLBACK com_longitud_cola_entrada (int panel, int control, int event,                                                             
                                void *callbackData, int eventData1,                                                                       
                                int eventData2)                                                                                           
{                                                                                                                                         
    int BytesEnBufferDeEntrada;                                                                                                           
	if (event == EVENT_COMMIT && FlagPuertoAbierto)                                                                                                           
        {                                                                                                                                 
        BytesEnBufferDeEntrada = GetInQLen (PuertoCOM);                                                                                   
        Fmt (Msg1, "%s<Longitud de la cola de entrada = %i", BytesEnBufferDeEntrada);                                                     
        MessagePopup ("Mensaje RS232", Msg1);                                                                                             
        }                                                                                                                                 
    return 0;                                                                                                                             
}                                                                                                                                         
/**************************************************************************************                                                   
		FUNCION QUE  MUESTRA EL NUMERO DE BYTES EN LA COLA DE SALIDA                                                                      
*******	********************************************************************************/                                                 
int CVICALLBACK com_longitud_cola_salida (int panel, int control, int event,                                                              
                                 void *callbackData, int eventData1,                                                                      
                                 int eventData2)                                                                                          
                                                                                                                                          
{                                                                                                                                         
int BytesEnBufferDeSalida;                                                                                                                
	if (event == EVENT_COMMIT && FlagPuertoAbierto)
        {                                                                                                                                 
        BytesEnBufferDeSalida = GetOutQLen (PuertoCOM);                                                                                   
        Fmt (Msg1, "%s<Longitud de la cola de salida = %i", BytesEnBufferDeSalida);                                                       
        MessagePopup ("Mensaje RS232", Msg1);                                                                                             
        }                                                                                                                                 
    return 0;                                                                                                                             
}                                                                                                                                         
/**************************************************************************************                                                   
             FUNCION QUE LIMPIA LA COLA DE ENTRADA                                                                                        
***************************************************************************************/                                                  
int CVICALLBACK com_borra_cola_entrada (int panel, int control, int event,                                                                
                                 void *callbackData, int eventData1,                                                                      
                                 int eventData2)                                                                                          
{                                                                                                                                         
    if (event == EVENT_COMMIT && FlagPuertoAbierto)
        {                                                                                                                                 
        FlushInQ (PuertoCOM);                                                                                                             
        MessagePopup ("Mensaje RS232", "¡Cola de entrada limpia!");                                                                       
        }                                                                                                                                 
    return 0;                                                                                                                             
}                                                                                                                                         
                                                                                                                                          
/**************************************************************************************                                                   
                FUNCION QUE LIMPIA LA COLA DE SALIDA                                                                                      
***************************************************************************************/                                                  
int CVICALLBACK com_borra_cola_salida (int panel, int control, int event,                                                                 
                                   void *callbackData, int eventData1,                                                                    
                                   int eventData2)                                                                                        
{                                                                                                                                         
    if (event == EVENT_COMMIT && FlagPuertoAbierto)
        {                                                                                                                                 
        FlushOutQ (PuertoCOM);                                                                                                            
		MessagePopup ("Mensaje RS232", "¡Cola de salida limpia!");                                                                        
        }                                                                                                                                 
    return 0;                                                                                                                             
}                                                                                                                                         
/**************************************************************************************                                                   
           FUNCION QUE LEE EL ESTATUS DEL PUERTO SERIE                                                                                    
***************************************************************************************/                                                  
int CVICALLBACK com_muestra_status (int panel, int control, int event,                                                                    
                                   void *callbackData, int eventData1,                                                                    
                                   int eventData2)                                                                                        
{                                                                                                                                         
    int com_status;      // Código de estatus del puerto                                                                                  
	if (event == EVENT_COMMIT && FlagPuertoAbierto)                                                                                                            
        {                                                                                                                                 
        com_status = GetComStat (PuertoCOM);                                                                                              
        muestra_mensaje_status (com_status);                                                                                          
        }                                                                                                                                 
    return 0;                                                                                                                             
}                                                                                                                                         
/**************************************************************************************                                                   
 		       FUNCION QUE LEE ERRORES DEL PUERTO                                                                                         
***************************************************************************************/                                                  
int CVICALLBACK com_muestra_error (int panel, int control, int event,                                                                     
                               void *callbackData, int eventData1,                                                                        
                               int eventData2)                                                                                            
{                                                                                                                                         
	switch (event)                                                                                                                        
        {                                                                                                                                 
        case EVENT_COMMIT:                                                                                                                
            ErrorRS232 = ReturnRS232Err (); // El sistema informa del error registrado                                                    
            display_error_RS232 ();			// en la última operación del puerto                                                      
            break;                                                                                                                        
        case EVENT_RIGHT_CLICK :                                                                                                          
            break;                                                                                                                        
        }                                                                                                                                 
    return 0; 

/*
		int i;
		if (event==EVENT_COMMIT)                                                                                                                        
        {                                                                                                                                 
            ErrorRS232 = ReturnRS232Err (); // El sistema informa estado de errores del puerto
			if (ErrorRS232==0)
			{
				MessagePopup ("Mensaje RS232","No hay errores"); 
				return 0;
			}
			for(i=0;i<=48;i++) 
			if (ErrorRS232 == RS232DescripcionDeError[i].CodigoError) {break;} 
			if(i==49)
			{
				MessagePopup ("Mensaje RS232","Código de error no encontrado"); 
				return 0;
			}
			if(i==0)
			{
				MessagePopup ("Mensaje RS232", GetRS232ErrorString(-1)); 
				return 0;
			}
			
			MessagePopup ("Mensaje RS232",RS232DescripcionDeError[i].DescripcionError); 
        }                                                                                                                                 
    	return 0; 
*/		
// GetRS232ErrorString() 
}                                                                                                                                         
/**************************************************************************************                                                   
          FUNCION QUE MUESTRA EL ESTATUS DEL PUERTO SERIE                                                                                 
***************************************************************************************/                                                  
void muestra_mensaje_status (com_status)                                                                                              
{                                                                                                                                         
    ComMsg[0] = '\0';                                                                                                                     
    if (com_status & 0x0001)                                                                                                              
        strcat (ComMsg, "Input lost: Input queue"                                                                                         
                " filled and characters were lost.\n");                                                                                   
    if (com_status & 0x0002)                                                                                                              
        strcat (ComMsg, "Asynch error: Problem "                                                                                          
                "determining number of characters in input queue.\n");                                                                    
    if (com_status & 0x0010)                                                                                                              
        strcat (ComMsg, "Paridad error.\n");                                                                                              
    if (com_status & 0x0020)                                                                                                              
        strcat (ComMsg, "Overrun error: Received"                                                                                         
                " characters were lost.\n");                                                                                              
    if (com_status & 0x0040)                                                                                                              
        strcat (ComMsg, "Framing error: Stop bits were not received"                                                                      
                " as expected.\n");                                                                                                       
    if (com_status & 0x0080)                                                                                                              
        strcat (ComMsg, "Break: A break signal was detected.\n");                                                                         
    if (com_status & 0x1000)                                                                                                              
        strcat (ComMsg, "Remote XOFF: An XOFF character was received."                                                                    
                "\nIf XON/XOFF was enabled, no characters are removed"                                                                    
                " from the output queue and sent to another device "                                                                      
                "until that device sends an XON character.\n");                                                                           
    if (com_status & 0x2000)                                                                                                              
        strcat (ComMsg, "Remote XON: An XON character was received."                                                                      
                "\nTransmisson can resume.\n");                                                                                           
    if (com_status & 0x4000)                                                                                                              
        strcat (ComMsg, "Local XOFF: An XOFF character was sent to\n"                                                                     
                " the other device.  If XON/XOFF was enabled, XOFF is\n"                                                                  
                " transmitted when the input queue is 50%, 75%, and 90%\n"                                                                
                " full.\n");                                                                                                              
    if (com_status & 0x8000)                                                                                                              
        strcat (ComMsg, "Local XON: An XON character was sent to\n"                                                                       
                " the other device.  If XON/XOFF was enabled, XON is\n"                                                                   
                " transmitted when the input queue empties after XOFF\n"                                                                  
                " was sent.  XON tells the other device that it can \n"                                                                   
                " resume sending data.\n");                                                                                               
    if (strlen (ComMsg) == 0)                                                                                                             
        strcat (ComMsg, "No status bits are set.");                                                                                       
    MessagePopup ("Mensaje RS232", ComMsg);                                                                                               
}                                                                                                                                         
/****************************************************************************************                                                     
				FUNCIONES DEL MENU CON COMANDOS SCPI COMUNES                                                                                  
			Envian por el puerto serie el comando seleccionado                                                                                
			Además muestra en el panel MENSAJES las cadenas de salida                                                                         
****************************************************************************************/                                                     
void CVICALLBACK opc_scpi (int menuBar, int menuItem, void *callbackData,                                                                     
		int panel)                                                                                                                            
{                                                                                                                                             
	sprintf(CadenaComando,"%s","*OPC");                                                                                                       
	ENVIAR_COMANDO_AL_SISTEMA(NO_MOSTRAR)                                                                                                     
}                                                                                                                                             
void CVICALLBACK error_scpi (int menuBar, int menuItem, void *callbackData,                                                                   
		int panel)                                                                                                                            
{                                                                                                                                             
	sprintf(CadenaComando,"%s","ERR?");                                                                                                       
	ENVIAR_COMANDO_AL_SISTEMA(NO_MOSTRAR)                                                                                                     
}                                                                                                                                             
                                                                                                                                              
void CVICALLBACK version_scpi (int menuBar, int menuItem, void *callbackData,                                                                 
		int panel)                                                                                                                            
{                                                                                                                                             
	sprintf(CadenaComando,"%s","*IDN");                                                                                                       
	ENVIAR_COMANDO_AL_SISTEMA(NO_MOSTRAR)                                                                                                     
}                                                                                                                                             
void CVICALLBACK cls_scpi (int menuBar, int menuItem, void *callbackData,                                                                     
		int panel)                                                                                                                            
{                                                                                                                                             
	sprintf(CadenaComando,"%s","*CLS");                                                                                                       
	ENVIAR_COMANDO_AL_SISTEMA(NO_MOSTRAR)                                                                                                     
} 

/*************************************************************************************** 
								FIN FUNCIONES DE MENU                                        
****************************************************************************************/
/*************************************************************************************** 
				FUNCION QUE PIDE LA IDENTIFICACION DEL SISTEMA     
		Envía al puerto serie PuertoCOM (global en el módulo) un *IDN
		Si todo va bien rellena la cadena IDNinstrumento[]
		DEVUELVE:  la longitud de la cadena de identificación. Si el puerto está
		cerrado devuelve -3.
		
****************************************************************************************/
int identifica_al_sistema(void)
{
	int short Indice=0;
	int LongMensaje;
	int Resultado;
	IDNinstrumento[0]='\0';//Borra la cadena donde se lee la identificación
	Fmt ( CadenaComando, "%s", "*IDN"); // Prepara la cadena con el comando de identificación
	Resultado=pcom_enviar_datos( CadenaComando,NO_MOSTRAR);
	Delay(Retardo); // Espera un poco Retardo es el del panel de configuración del puerto
	pcom_recibir_datos(IDNinstrumento,0);// Lee la posible respuesta
	// Busca el terminador (ver de #define Terminador) y pone tras el un finalizador de cadena '\0'
	LongMensaje=strlen (IDNinstrumento);
	while(IDNinstrumento[Indice]!=TERMINADOR && Indice<=LongMensaje)Indice++;IDNinstrumento[Indice+1]='\0';
	return LongMensaje=strlen (IDNinstrumento);
}	
/****************************************************************************************    
			FUNCION PUBLICA PARA LIMPIAR LA PILA DE ERRORES SCPI DEL MICRO                          
			                                                                                 
****************************************************************************************/    
void pcom_limpia_pila_errores()                                                              
{                                                                                            
	int LongCadenaCls;                                                                       
	LongCadenaCls = StringLength ("*CLS");                                                   
	CopyString (CadenaComando, 0,"*CLS", 0,LongCadenaCls);                                   
	pcom_enviar_datos(CadenaComando,NO_MOSTRAR); // envia el comando completo por el puerto	 
}                                                                                            
/****************************************************************************************    
					FUNCION QUE CAMBIA DE COLOR EL LED DEL PANEL MENSAJES
****************************************************************************************/    
void led_color(enum enumColor ColorNuevo)
{
	switch (ColorNuevo)
	{
		case Rojo:
		SetCtrlAttribute (MensajesHandle,MENSAJES_LEDCOM, ATTR_ON_COLOR, VAL_RED);
		break;
		case Verde:
		SetCtrlAttribute (MensajesHandle,MENSAJES_LEDCOM, ATTR_ON_COLOR, VAL_GREEN);
		break;
		case Azul:
		SetCtrlAttribute (MensajesHandle,MENSAJES_LEDCOM, ATTR_ON_COLOR, VAL_BLUE);
		break;
	}
	
}
/*************************************************************************************                                          

		FUNCIONES QUE LEEN, ESCRIBEN, CONFIGURAN O CIERRAN EL PUERTO
		Susceptibles de producir un error a nivel de librería RS-232
				
**************************************************************************************/                                         
/*************************************************************************************                                          
							CIERRA EL PUERTO SERIE                                                                              
	Si el puerto está abierto, limpia los                                                                                       
	buffers de entrada y salida y cierra                                                                                        
	el puerto.                                                                                                                  
**************************************************************************************/                                         
void pcom_cierra_puerto_serie(void)                                                                                             
{                                                                                                                               
	if (FlagPuertoAbierto)                                                                                                      
    {                                                                                                                           
		// Primero guardo los datos del puerto en el fichero, si no hay errores en el puerto
		ErrorRS232 = ReturnRS232Err();
		if(ErrorRS232==0)guarda_puerto_com_en_fichero();    
		FlushOutQ (PuertoCOM);                                                                                                  
        FlushInQ (PuertoCOM);                                                                                                   
		ErrorRS232 = cerrar_puerto (PuertoCOM);                                                                                      
		FlagPuertoAbierto=0; // Para que el programa sepa que el puerto está cerrado                                            
	    // TO DO Gestión de errores
		if (ErrorRS232) display_error_RS232 ();                                                                             
		PuertoCOM=SIN_PUERTO;                                                                                                   
		descarga_paneles_del_puerto_com();//Quita los paneles de la memoria
    }                                                                                                                           
 }                                                                                                                               
/***************************************************************************************                                        
	FUNCION PARA ENVIAR DATOS                                                                                                   
	Función que comprueba si se pueden enviar datos por el puerto serie. Si se puede                                            
	 añade el terminador y los envia; si no, da un mensaje de error en pantalla.                                                
	 "Mostrar"  se pone a 1 para que la cadena se vea en el test box MENSAJES_CADENA_ENVIAR                                     
*****************************************************************************************/                                      
int pcom_enviar_datos (char CadenaEnviar[], int Mostrar)  {                                                                     
// Si el pueto no está abierto no podemos transmitir                                                                            
	int LongitudString;                                                                                                         
	                                                                                                                            
	//  -3, "kRS_PortNotOpen Port", " is not open. ",                                                                           
	if(!FlagPuertoAbierto) return -3;// No accede al puerto si no está abierto                                             
    // Si el puerto está abierto transmitimos                                                                                   
    else                                                                                                                        
	{                                                                                                                           
    	switch (TerminadorTransmision)                                                                                          
    	{                                                                                                                       
    	case 1:                                                                                                                 
       		strcat(CadenaEnviar, "\r");                                                                                         
       		break;                                                                                                              
    	case 2:                                                                                                                 
       		strcat(CadenaEnviar, "\n");                                                                                         
       		break;                                                                                                              
		case 3:                                                                                                                 
       		strcat(CadenaEnviar, "\r\n");                                                                                       
       		break;                                                                                                              
    	}                                                                                                                       
	    LongitudString = StringLength (CadenaEnviar);                                                                           
		// Envía la cadena  por el puerto                                                                                       
		ComWrt (PuertoCOM, CadenaEnviar, LongitudString);                                                                       
		ErrorRS232 = ReturnRS232Err ();
		if (ErrorRS232)                                                                                                         
		{ 
			display_error_RS232 ();                                                                                           
			return ErrorRS232;                                                                                                               
		}                                                                                                                       
		else                                                                                                                    
		{                                                                                                                       
			//if(Mostrar)ResetTextBox (MensajesHandle, MENSAJES_CADENA_ENVIAR, CadenaEnviar);                                   
			if(Mostrar)
			{                                                                                                       
				if(MensajesHandle)SetCtrlVal (MensajesHandle, MENSAJES_CADENA_ENVIAR, CadenaEnviar);                                
				//if(MensajesHandle)InsertListItem (MensajesHandle, MENSAJES_LISTCOMANDOS, 0,CadenaEnviar,23);                      
			}                                                                                                                  
		}                                                                                                                       
		return ErrorRS232;                                                                                                      
	}                                                                                                                           
}                                                                                                                               
/***************************************************************************************                                        
	FUNCION PARA RECIBIR DATOS                                                                                                  
	Lee datos del buffer según el terminador seleccionado.
	DEVUELVE: El número de bytes leidos o el código de error si se produce
****************************************************************************************/                                       
int pcom_recibir_datos (char CadenaRecibida[],int short Mostrar)                                                                                  
{                                                                                                                               
	int BytesEnBuffer;                                                                                                          
	int BytesLeidos;                                                                                                            
	char CadenaLeida[STRLARGO]="\0"; // Lee sobre una cadena limpia                                                             
	static int CaracterTerminador; // Caracter a incluir en la cadena de emisión como terminador 
	led_color(Verde);
	if(!FlagPuertoAbierto) return -3;// No accede al puerto si no está abierto                                             	                                                                                                                            
	// Si el pueto  está abierto  podemos recibir                                                                       
	else                                                                                                                        
	{                                                                                                                           
       	// Selecciona el terminador de recepción                                                                                
       	switch (TerminadorRecepcion)                                                                                            
        {                                                                                                                       
          	case 0:                                                                                                             
            CaracterTerminador = 0;//NONE                                                                                       
            break;                                                                                                              
         	case 1:                                                                                                             
            CaracterTerminador = 13;  //CR                                                                                      
            break;                                                                                                              
         	case 2:CaracterTerminador = 10; //LF                                                                                
            break;                                                                                                              
        }                                                                                                                       
                                                                                                                                
		// RECEPION                                                                                                             
		// Si el nº de bytes en el buffer del COM es razonable..                                                                
		BytesEnBuffer = GetInQLen (PuertoCOM);                                                                                  
		if(BytesEnBuffer<0)                                                                                                     
		{display_error_RS232 (); return BytesEnBuffer;led_color(Rojo);}// Si la lectura da un error lo avisa y sale   		                            
		if(BytesEnBuffer > STRLARGO)                                                                                            
		// TO DO Gestión de errores
		{ErrorRS232=ERROR_DATOS_EXCESIVOS;display_error_RS232 ();return ErrorRS232;led_color(Rojo);}                                                 
		if (CaracterTerminador) // Lee con terminador                                                                           
			{   BytesLeidos = ComRdTerm (PuertoCOM, CadenaLeida, STRLARGO,CaracterTerminador);// LECTURA DEL PUERTO !!!                                  
				if( BytesLeidos <0)                                                                                              
				// TO DO Gestión de errores
				{ErrorRS232=BytesLeidos; display_error_RS232 (); return BytesLeidos;led_color(Rojo);}// Si la lectura da un error lo avisa y sale   	
			}                                                                                                                   
	           else // Lee sin terminador.                                                                                      
			{   BytesLeidos = ComRd (PuertoCOM, CadenaLeida, BytesEnBuffer);                                                    
				if(BytesLeidos<0)                                                                                               
				{ErrorRS232=BytesLeidos;display_error_RS232 (); return ErrorRS232;led_color(Rojo);}// Si la lectura da un error lo avisa y sale   	
			}                                                                                                                   
           	// Tras hacer una lectura copia lo leido en CadenaRecibida y limpia el buffer del puerto COM                        
		CopyString (CadenaRecibida, 0, CadenaLeida, 0, BytesLeidos);
		if(Mostrar)
		{	
			SetCtrlVal (MensajesHandle, MENSAJES_CADENA_RECIBIDA, CadenaRecibida);// Muestra la cadena                                 
			SetCtrlVal (MensajesHandle, MENSAJES_CADENA_RECIBIDA, "\n");//                                  
		}
		led_color(Azul);
		return BytesLeidos;                                                                                                     
		}//if 
}                                                                                                                               
/*****************************************************************************************************                                 
	FUNCION QUE SE EJECUTA AUTOMATICAMENTE CUANDO SE DETECTA "Terminador" EN EL BUFFER                                            
 	Esta función se ejecuta cuando se detecta Terminador de recepción en el buffer de entrada                                                
	del puerto COM.                                                                                                             
    Esta función no utiliza pcom_recibir_datos(..) sino que utiliza ComRd(..). No utiliza                                       
	ComRdTerm(..) porque quita el terminador.                                                                                   
	Antes de salir limpia el buffer del COM abierto                                                                             
	Devuelve el resultado de la lectura en CadenaRespuesta que es global                                                                       
******************************************************************************************************/                                
void CVICALLBACK terminador_detectado_en_buffer_de_entrada(int portNo,int eventMask,void *callbackData)                                                                               
{                                                                                                                               
	//void (*callbackData)();                                                                                                   
	int BytesLeidos;                                                                                                            
	int Indice; // Para revisar la cadena de entrada                                                                            
	CadenaRespuesta[0]='\0';// Resetea la cadena para leer datos del puerto                                                     
	BytesLeidos=ComRdTerm (PuertoCOM, CadenaRespuesta, STRLARGO,TERMINADOR);//LECTURA DEL PUERTO                                               
	// Si la lectura da un error lo avisa y sale                                                                                
	// TO DO Gestión de errores
	if (BytesLeidos<0){ErrorRS232=BytesLeidos;display_error_RS232 (); return;}//                                            
	// Revisa la cadena buscando el Terminador y la cierra con '\n' y '\0'                                                      
	Indice=-1;                                                                                                                  
	do                                                                                                                          
	{                                                                                                                           
	Indice++;                                                                                                               
	}while(CadenaRespuesta[Indice]!=TERMINADOR2 && Indice<(STRLARGO-2));// Busca el terminador                                  
	CadenaRespuesta[Indice]=TERMINADOR; // Le añade un retorno de carro eliminanado "TERMINADOR2"                                                               
	CadenaRespuesta[Indice+1]='\0'; // Le añade un fin de cadena                                                                
	// Muestra la cadena                                                                                                        
	SetCtrlVal (MensajesHandle, MENSAJES_CADENA_RECIBIDA, CadenaRespuesta);// Muestra la cadena                                 
	// TO DO Gestión de errores
	ErrorRS232 = ReturnRS232Err ();                                                                                    
	if (ErrorRS232) display_error_RS232 ();
	pcom_datos_recibido();//Llama a la función que se define en la aplicación principal                                         
	FlagDatosRecibidos=SI;//Flag para lebantar cuando se recibe en automático
return;                                                                                                                         
}	                                                                                                                            
/*****************************************************************************************************                                 
	FUNCION QUE ABRE EL PUERTO SERIE
	Devuelve el resultado de ejecutar la apertura del puerto con OpenConfig
******************************************************************************************************/                                
int abrir_puerto(int PCOM, const char Nombre[],int Vel,int Par,int Bits,int Stop,int ColEn,int ColSal)
{
	ErrorRS232 = OpenComConfig (PCOM,Nombre,Vel,Par,Bits,Stop,ColEn,ColSal);//ABRE EL PUERTO	
	if(!ErrorRS232)
	{
		SetCtrlVal(MensajesHandle,MENSAJES_LEDCOM,1); 
		SetCtrlVal(ConfigHandle,CONFIGURAP_LEDCONFIG,1); 
	}		
	return ErrorRS232;
}
/*****************************************************************************************************
	FUNCION QUE CIERRA EL PUERTO SERIE
	Cierro el puerto pasado como argumento
	FlagPuertoAbierto lo tiene que poner a cero la función llamante
******************************************************************************************************/                                
int cerrar_puerto(int PCOM)
{
	int Resultado;
	Resultado=CloseCom (PCOM);
	SetCtrlVal(MensajesHandle,MENSAJES_LEDCOM,0);
	SetCtrlVal(ConfigHandle,CONFIGURAP_LEDCONFIG,0); 
	return Resultado;
}
/******************************************************************************************************/
