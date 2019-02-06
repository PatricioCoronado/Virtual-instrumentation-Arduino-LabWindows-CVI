/************************************************************************************
 	DEPARTAMENTO DE ELECTRÓNICA DE SEGAINVEX. UNIVERSIDA AUTONOMA DE MADRID				
 	LIBRERIA PARA ARDUINO Segainvex_scpi_Serial V1.0
 	SISTEMA PARA COMUNICAR UNA COMPUTADORA CON ARDUINO MEDIANTE PUERTO SERIE 
 	Fichero de cabecera segainvex_scpi_Serial.h


	Copyright © 2017 Mariano Cuenca, Patricio Coronado
	
	This file is part of segainvex_scpi_Serial.

    segainvex_scpi_Serial is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    segainvex_scpi_Serial is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with segainvex_scpi_Serial.  If not, see <http://www.gnu.org/licenses/>.


 
		Mejoras futuras:
		1)Poner el menú de segainvex_scpi_Serial en flash para no consumir RAM
		2)Mejorar la función int cambia_variable_double_del_sistema(double *,double,double);
 *******************************************************************************************/
 
 
#ifndef SEGAINVEX_SCPI_SERIAL_H_INCLUDED
#define SEGAINVEX_SCPI_SERIAL_H_INCLUDED
//
#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif


#ifdef __cplusplus            
    extern "C" {              
#endif    
                    
/**********************************************************************************
 
		La librería funciona con 3 puertos serie: Serial, SerialUSB y Serial1
		Pero antes de hacer el include de la librería. Hay que indicar que
		puerto se va a utilizar con un define.
		
		#define SERIALUSB
		o
		#define SERIAL1
		
		Si no se incluye el #define el puerto por defecto es Serial
 
	HAY QUE DEJAR DESCOMENTADO EL DEFINE QUE SE CORRESPONDA CON EL PUERTO A UTILIZAR
	SI NO SE DESCOMENTA NINGUNA LINEA, EL PUERTO ES "Serial"
*************************************************************************************/
  #define SERIALUSB
 //#define SERIAL1
/***********************************************************************************
  DESDE AQUÍ ESTE FICHERO NO DEBE SER EDITADO. ES EL MISMO EN TODAS LAS APLICACIONES
 ***********************************************************************************/                             
                              

/*******************************************************************************
							CONSTANTES EXPORTADAS
Hay que tener en cuenta que el buffer de puerto serie de Arduino es 64 bytes							
Por lo que BUFFCOM_SIZE y LONG_SCPI no pueden ser mayores de 64 bytes.
Estas constantes pueden ser redefinidas.
********************************************************************************/
#define BUFFCOM_SIZE 32 //Longitud del buffer de lectura de de segainvex_scpi_Serial
#define LONG_SCPI 32 // Longitud máxima del comando mas largo sin contar parámetros
#define MAXERR 3   // Profundidad de la pila de errores
/*******************************************************************************
						DEFINICION DE TIPOS EXPORTADOS
*******************************************************************************/            
//TIPOS PARA LA ESTRUCTURA DEL MENU DE SEGAINVEX-SCPI
//typedef void (*tpf)();// tpf es ahora puntero a función
// Declaración de tipo estructura para menús de comandos SEGAINVEX-SCPI
typedef struct tipoNivel
{             
  int   NumNivInf;       // Número de niveles por debajo de este
  char *largo, *corto;   // Nombre largo y nombre corto del comando
  /*tpf pf*/void (*pf)();// Puntero a función a ejecutar
  tipoNivel *sub;     // Puntero a la estructura de niveles inferiores
};
// TIPO PARA EL ARRAY DE ERRORES
typedef char *tipoCodigoError[];
/********************************************************************************
      PROTOTIPOS DE FUNCIONES DE SEGAINVEX-SCPI QUE TIENEN QUE SER VISIBLES
*********************************************************************************/
/*********************************************************************************
	Funcion que gestiona la pila de errores
	El parámetro de entrada es el número del error a poner el la pila de errores
**********************************************************************************/
extern void errorscpi(int); // 
/*********************************************************************************
	Funcion principal de SEGAINVEX-SCPI
	Se debe ejecutar cuando se detecten datos en el puerto serie
**********************************************************************************/
extern void scpi(void);     
/**********************************************************************************
 FUNCIONES AUXILIARES, PARA LEER UN ARGUMENTO ENTERO QUE SE PASA CON EL COMANDO,
 CUYO FIN ES ACTUALIZAR EL VALOR DE UNA VARIABLE ENTERA O BOOLENAN DEL SISTEMA.
 SI EL COMANDO VA SEGUIDO DE ? LO QUE HACE LA FUNCIÓN ES DEVOLVER EL VALOR DE 
 LA VARIABLE.
 Leen el parámetro del array de char FinComando[]. El carácter FinComando[0] se 
 espera que sea un espacio. A continuación tiene que estar el parámetro a leer.
*********************************************************************************/
/*******************************************************************************
	Cambia la variable entera cuya dirección se pasa como argumento.
	Los argumentos de entrada son:
	La dirección de la variable entera, el valor máximo, el valor mínimo.
	Devuelve 1 si cambió la variable. 0, si no cambió la variable por errores.
	Y 2 si devolvió el valor de la variable.
*******************************************************************************/
extern int cambia_variable_int_del_sistema(int *,int,int);//
/*******************************************************************************
	Cambia la variable entera discreta cuya dirección se pasa como argumento.
	Los argumentos de entrada son:
	La dirección de la variable entera,array de entero con los valores posibles,
	tamaño del array.
	Devuelve 1 si cambió la variable. 0, si no cambió la variable por errores.
	Y 2 si devolvió el valor de la variable.
*******************************************************************************/
extern int cambia_variable_int_discreta_del_sistema(int *,int*,int);//
/*******************************************************************************
	Cambia la variable booleana cuya dirección se pasa como argumento.
	Los argumentos de entrada son:
	La dirección de la variable entera, el valor máximo, el valor mínimo y el
	número de error a anotar si la variable está fuera de rango.
	Devuelve 1 si cambió la variable. 0, si no cambió la variable por errores.
	Y 2 si devolvió el valor de la variable.
********************************************************************************/	
extern int cambia_variable_bool_del_sistema(bool *);
/*******************************************************************************
	Cambia la variable double cuya dirección se pasa como argumento.
	Los argumentos de entrada son:
	La dirección de la variable float, el valor máximo, el valor mínimo.
	Devuelve 1 si cambió la variable. 0, si no cambió la variable por errores.
	Y 2 si devolvió el valor de la variable. 
*******************************************************************************/
extern int cambia_variable_double_del_sistema(double *,double,double);
/********************************************************************************
   VARIABLES GLOBALES EXPORTADAS DE SEGAINVEX-SCPI QUE TIENEN QUE SER VISIBLES
  Son globales todos los modulos con el include a segainvex_scpi_Serial.h  						
********************************************************************************/
//Esta variable tiene que estar definida en el código del usuario.
extern tipoNivel Raiz[]; //Array de la estructura raiz de comandos
//Estas variable tiene que estar definida en el código del usuario.
extern tipoCodigoError CodigoError;//Puntero al array de cadenas explicativas del error
// FinComando es un array definido en seginvex_scpi.c de char que contiene los 
//parámetros que envia el PC tras el comando.
extern char *FinComando;// Puntero al final del comando para leer parámetros
//Cadena donde el usuario debe escribir el nombre de su sistema. La cadena se 
// envía al PC cuando este envíe a Arduino el comando *IDN
extern char IdentificacionDelSistema[];
/********************************************************************************
									MACROS EXPORTADAS 
*********************************************************************************/      
// Para definir submenús 
#define SCPI_SUBMENU(X,Y) sizeof(X)/sizeof(*X), #X,#Y,NULL,X,  
// Para definir comandos
#define SCPI_COMANDO(X,Y,Z) 0, #X,#Y,Z,NULL, //Para definir comandos 
//Para definir el nivel raiz de comandos
#define SCPI_NIVEL_RAIZ tipoNivel Raiz[]={sizeof(NivelDos)/sizeof(*NivelDos),"","",NULL,NivelDos};
//Macro que pone en el array "IdentificacionDelSistema" el nombre del sistema que implementa Arduino
#define NOMBRE_DEL_SISTEMA_64B(X) strcpy(IdentificacionDelSistema,#X);
/*********************************************************************************
 EDITABLE
 PROTOTIPOS DE FUNCIONES DEL PROGRAMA PRINCIPAL LLAMADAS POR SEGAINVEX-SCPI
 Añadir tantas funciones como se necesiten. El límite es las direcciones
 que puede acceder un puntero que será del tamaño de los bits que sea el 
 microprocesador/microcontrolador
 Aquí hay de fs0 a fs100 y de fs240 a fs255
 *********************************************************************************/
void fs0(void);
void fs1(void);
void fs2(void);
void fs3(void);
void fs4(void);
void fs5(void);
void fs6(void);
void fs7(void);
void fs8(void);
void fs9(void);
void fs10(void);
void fs11(void);
void fs12(void);
void fs13(void);
void fs14(void);
void fs15(void);
void fs16(void);
void fs17(void);
void fs18(void);
void fs19(void);
void fs20(void);
void fs21(void);
void fs22(void);
void fs23(void);
void fs24(void);
void fs25(void);
void fs26(void);
void fs27(void);
void fs28(void);
void fs29(void);
void fs30(void);
void fs31(void);
void fs32(void);
void fs33(void);
void fs34(void);
void fs35(void);
void fs36(void);
void fs37(void);
void fs38(void);
void fs39(void);
void fs40(void);
void fs41(void);
void fs42(void);
void fs43(void);
void fs44(void);
void fs45(void);
void fs46(void);
void fs47(void);
void fs48(void);
void fs49(void);
void fs50(void);
void fs51(void);
void fs52(void);
void fs53(void);
void fs54(void);
void fs55(void);
void fs56(void);
void fs57(void);
void fs58(void);
void fs59(void);
void fs60(void);
void fs61(void);
void fs62(void);
void fs63(void);
void fs64(void);
void fs65(void);
void fs66(void);
void fs67(void);
void fs68(void);
void fs69(void);
void fs70(void);
void fs71(void);
void fs72(void);
void fs73(void);
void fs74(void);
void fs75(void);
void fs76(void);
void fs77(void);
void fs78(void);
void fs79(void);
void fs80(void);
void fs81(void);
void fs82(void);
void fs83(void);
void fs84(void);
void fs85(void);
void fs86(void);
void fs87(void);
void fs88(void);
void fs89(void);
void fs90(void);
void fs91(void);
void fs92(void);
void fs93(void);
void fs94(void);
void fs95(void);
void fs96(void);
void fs97(void);
void fs98(void);
void fs99(void);
void fs100(void);
// definir tantas funciones como sean necesarias
void fs240(void);
void fs241(void);
void fs242(void);
void fs243(void);
void fs244(void);
void fs245(void);
void fs246(void);
void fs247(void);
void fs248(void);
void fs249(void);
void fs250(void);
void fs251(void);
void fs252(void);
void fs253(void);
void fs254(void);
void fs255(void);
/**************************************************************************/
/*
NOTAS:
1)  El buffer de recepción de Arduino es de 64 bytes, por lo que hay que dimensionar
	la cadena de recepción como mucho a 64 bytes. BUFFCOM_SIZE=<64 
2)  Si se lee el puerto serie desde Arduino con 
	SerialUSB.readBytesUntil('r',buffer,tamaño_buffer);
	hay que	asegurarse de que el terminador con que el PC cierre las cadenas a enviar
	sea '\r'.
3)	Si Arduino lee con SerialUSB.readBytesUntil('\r',buffer,tamaño_buffer);.
	Esta función no mete	el terminador '\r' en la cadena que devuelve, por lo que 
	hay que ponerselo, ya que segainvex_scpi_Serial necesita un terminador para dar por 
	buena una cadena. Al hacerlo hay que incrementar la variable que indica
	la longitud del comando "locCom" en 1. 
						
  Se ha definido un tipo de estructura denominado "tipoNivel". Las variables de este 
  tipo puede contener un nivel de comandos o un comando.
  
  Si contiene un nivel, el formato es:
  
  int número_de_comandos_del_nivel,
  char[] Nombre_largo_del_nivel,  
  char[] Nombre_corto_del_nivel,
  (*tpf)() NULL;                    //puntero a función   
  tipoNivel* Nombre_del_nivel,		//puntero a tipoNivel
  
  Y si contiene un comando, el formato es:
  
  int 0,
  char[] Nombre_largo_del_comando,
  char[] Nombre_corto_del_comando,
  (*tpf)() funcion_a_ejecutar,       //puntero a función
  tipoNivel* NULL,					//puntero a tipoNivel  
  
  Como se vé la diferencia es que si es un nivel, el puntero a función está a NULL, ya que
  al no ser un comando, no ejecuta ninguna función. Si es un comando, su puntero a función
  si que apunta a la función que se quiere ejecutar con ese comando. Pero su puntero a
  "tipoNivel" está a NULL, ya que de el no cuelga un nivel, porque es solo un comando.
  
  Se declaran array de variables tipoNivel.Siempre ha de existir este array:
  
  tipoNivel Raiz[] = {sizeof(NivelDos)/(5*sizeof(int)),"","",NULL, NivelDos};
	  
  Que tiene una única estructura "tipoNivel" y siempre el mismo formanto. 
  Es el nivel raiz del menú de submenús y comandos.
  
  Otro array que debe existir siempre es el denominado "NivelDos" (como se ve, 
  último miembro de la única estructura del "Raiz") 
	  
  El formato del array de estructuras "tipoNivel" "NivelDos" es:
  tipoNivel NivelDos[] ={
	  estructura_nivel_1,estructura_nivel_2,...,estructura_nivel_n,
	  estructura_comandos_1,estructura_comandos_2,..,estructura_comandos_n,
  };
  Cada nivel_estructura_i y nivel_comando_i tiene los formatos vistos arriba.
  
      El formato de cada array con un nivel de comandos es:
	tipoNivel NOMBRE_SUBNIVEL1[]={ 
		estructura_comandos_1,estructura_comandos_2,..,estructura_comandos_n,
  };
  
  Hay que añadir tantos arrays  "tipoNivel" como se necesiten. Y a su vez, en los
  arrays incluir tantos comandos como se necesiten. 
  
  Todo esto se puede declara en un fichero .h o dentro del .ino, lo que resulte
  más apropiado en cada caso.
*/
/**************************************************************************/
#ifdef __cplusplus            
    }                         
#endif                        


#endif // SEGAINVEX_SCPI_SERIAL_H_INCLUDED

