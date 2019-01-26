/**************************************************************************/
/* LabWindows/CVI User Interface Resource (UIR) Include File              */
/* Copyright (c) National Instruments 2019. All Rights Reserved.          */
/*                                                                        */
/* WARNING: Do not add to, delete from, or otherwise modify the contents  */
/*          of this include file.                                         */
/**************************************************************************/

#include <userint.h>

#ifdef __cplusplus
    extern "C" {
#endif

     /* Panels and Controls: */

#define  CONFIGURAP                       1       /* callback function: panel_configuracion */
#define  CONFIGURAP_PUERTOCOM             2       /* control type: numeric, callback function: (none) */
#define  CONFIGURAP_VELOCIDADPUERTO       3       /* control type: ring, callback function: (none) */
#define  CONFIGURAP_BITSTOP               4       /* control type: slide, callback function: (none) */
#define  CONFIGURAP_BITDATOS              5       /* control type: slide, callback function: (none) */
#define  CONFIGURAP_TERMINADORESCRITURA   6       /* control type: ring, callback function: (none) */
#define  CONFIGURAP_PARIDAD               7       /* control type: slide, callback function: (none) */
#define  CONFIGURAP_RINGCOM               8       /* control type: ring, callback function: seleciona_un_puerto_de_la_lista */
#define  CONFIGURAP_COLASALIDA            9       /* control type: numeric, callback function: (none) */
#define  CONFIGURAP_TIMEOUT_MSG1_3        10      /* control type: textMsg, callback function: (none) */
#define  CONFIGURAP_COLAENTRADA           11      /* control type: numeric, callback function: (none) */
#define  CONFIGURAP_TERMINADORLECTURA     12      /* control type: ring, callback function: (none) */
#define  CONFIGURAP_MODOCTS               13      /* control type: binary, callback function: (none) */
#define  CONFIGURAP_TIMEOUT_MSG1_4        14      /* control type: textMsg, callback function: (none) */
#define  CONFIGURAP_MODOXONOFF            15      /* control type: binary, callback function: (none) */
#define  CONFIGURAP_TIMEOUT_MSG1_5        16      /* control type: textMsg, callback function: (none) */
#define  CONFIGURAP_TIMEOUT_MSG1_2        17      /* control type: textMsg, callback function: (none) */
#define  CONFIGURAP_RETARDO               18      /* control type: numeric, callback function: (none) */
#define  CONFIGURAP_TIMEOUT               19      /* control type: numeric, callback function: (none) */
#define  CONFIGURAP_DECORATION_2          20      /* control type: deco, callback function: (none) */
#define  CONFIGURAP_CERRARPCOM            21      /* control type: command, callback function: CerrarPuertoCallback */
#define  CONFIGURAP_BUSCARPUERTOS         22      /* control type: command, callback function: buscar_puertos_serie */
#define  CONFIGURAP_CLOSECONFIG           23      /* control type: command, callback function: AplicarConfigCallback */
#define  CONFIGURAP_DECORATION_7          24      /* control type: deco, callback function: (none) */
#define  CONFIGURAP_DECORATION_4          25      /* control type: deco, callback function: (none) */
#define  CONFIGURAP_DECORATION_5          26      /* control type: deco, callback function: (none) */
#define  CONFIGURAP_DECORATION_6          27      /* control type: deco, callback function: (none) */
#define  CONFIGURAP_IDENTIFICACION        28      /* control type: string, callback function: (none) */
#define  CONFIGURAP_LEDCONFIG             29      /* control type: LED, callback function: cambiar_de_modo */

#define  ESPERA                           2
#define  ESPERA_PICTURE                   2       /* control type: picture, callback function: (none) */

#define  ESTATUS                          3
#define  ESTATUS_ERROR                    2       /* control type: command, callback function: com_muestra_error */
#define  ESTATUS_COMSTATUS                3       /* control type: command, callback function: com_muestra_status */
#define  ESTATUS_GETOUTQ                  4       /* control type: command, callback function: com_longitud_cola_salida */
#define  ESTATUS_CLOSE_ESTATUS            5       /* control type: command, callback function: cierra_panel_status */
#define  ESTATUS_GETINQ                   6       /* control type: command, callback function: com_longitud_cola_entrada */
#define  ESTATUS_FLUSHOUTQ                7       /* control type: command, callback function: com_borra_cola_salida */
#define  ESTATUS_FLUSHINQ                 8       /* control type: command, callback function: com_borra_cola_entrada */

#define  MENSAJES                         4       /* callback function: panel_mensajes */
#define  MENSAJES_LIMPIAR_RECIBIDO_2      2       /* control type: command, callback function: LimpiarEnviar */
#define  MENSAJES_LIMPIAR_RECIBIDO        3       /* control type: command, callback function: LimpiarRecibir */
#define  MENSAJES_LEDCOM                  4       /* control type: LED, callback function: abre_cierra_com */
#define  MENSAJES_CADENA_ENVIAR           5       /* control type: textBox, callback function: (none) */
#define  MENSAJES_CADENA_RECIBIDA         6       /* control type: textBox, callback function: (none) */
#define  MENSAJES_X_POS                   7       /* control type: numeric, callback function: (none) */
#define  MENSAJES_Y_POS                   8       /* control type: numeric, callback function: (none) */
#define  MENSAJES_COMANDO                 9       /* control type: string, callback function: enviar_comando */


     /* Control Arrays: */

          /* (no control arrays in the resource file) */


     /* Menu Bars, Menus, and Menu Items: */

#define  MENUSCPI                         1
#define  MENUSCPI_COM                     2       /* callback function: Estatus */
#define  MENUSCPI_MENUERROR               3       /* callback function: error_scpi */
#define  MENUSCPI_MENUVERSION             4       /* callback function: version_scpi */
#define  MENUSCPI_MENUCLS                 5       /* callback function: cls_scpi */


     /* Callback Prototypes: */

int  CVICALLBACK abre_cierra_com(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK AplicarConfigCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK buscar_puertos_serie(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cambiar_de_modo(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CerrarPuertoCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cierra_panel_status(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
void CVICALLBACK cls_scpi(int menubar, int menuItem, void *callbackData, int panel);
int  CVICALLBACK com_borra_cola_entrada(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK com_borra_cola_salida(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK com_longitud_cola_entrada(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK com_longitud_cola_salida(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK com_muestra_error(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK com_muestra_status(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK enviar_comando(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
void CVICALLBACK error_scpi(int menubar, int menuItem, void *callbackData, int panel);
void CVICALLBACK Estatus(int menubar, int menuItem, void *callbackData, int panel);
int  CVICALLBACK LimpiarEnviar(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK LimpiarRecibir(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK panel_configuracion(int panel, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK panel_mensajes(int panel, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK seleciona_un_puerto_de_la_lista(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
void CVICALLBACK version_scpi(int menubar, int menuItem, void *callbackData, int panel);


#ifdef __cplusplus
    }
#endif
