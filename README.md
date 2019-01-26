# Virtual-instrumentation-Arduino-LabWindows-CVI
Para montar este proyecto se requiere un Arduino Zero o otra placa con un microcontrolador SAMD21.
Para utilizar un timer se usa la librería:

https://github.com/avandalen/avdweb_SAMDtimer

En esta aplicación se muestra como hacer instrumentación virtual con Arduino y la librería 
segainvex_scpi_Serial más elentormo de programación de National Instrumentos LabWindows CVI, 
con el que se ha programado la librería para este entorno "PuertoCOM" para manejar fácilmente un puerto
COM a través del cual se comunica con el puerto USB donde se conectar el Arduino.

Para ejecutar la aplicación hay que instalar el Runtime Engine de LabWindows CVI.
http://www.ni.com/download/labwindowscvi-run-time-engine-2012/3032/en/

Una vez instalado, con copiar los ficheros de la aplicación en un directorio ya se puede ejecutar 
el fichero CapturaMuestras.exe. No hay que tener el entorno de programación LabWindows CVI. Es un 
entorno caro, yo lo uso bajo licencia pagada por la Universidad Autónoma de Madrid (SEGAINVEX-ELECTRÓNICA).

La aplicación tiene un botón para solicitarle muestras al Arduino (SAMD21, Arduino Zero). Este 
envía las muestras y la aplicación las plotea. Arduino lee las muestras de la entrada analógica A1.
En esta entrada se puede poner un cable al aire con unas espiras y se captará la inducción de la red 
electrica que en España es de 50Hz y eso es lo que se verá en la gráfica.

Lo importante es ver como se usa la librería segainvex_scpi_Serial en Arduino y PuertoCOM en el PC. 
Ahora se da una explicación de como se utiliza esta última para hacer instrumentación virtual.

Para utilizar este módulo hay que incluir PuertoCOM.c PuertoCOM.h listaPaco.c listaPaco.h.
 
 Conexión con instrumentos:
 
Instrumento conectado en un puerto COM con la librería segainvex_scpi_Serial:

La configuración del puerto serie y del instrumento debe ser la del panel de configuración:
 baudrate 115200, 8 bits de datos, 2 bits de stop,sin protocolo hard. ni soft.
 Si se conecta un instrumento que responde al comando SCPI IDN (segainvex_scpi_Serial instalada)
devolviendo  una cadena de identificación, lo más sencillo es rellenar la cadena de  identificación:
	char IdentificacionDelInstrumento[] ="Identificación del sistema";
 en la aplicación principal y  ejecutar pcom_abre_puerto_serie. Esta función  busca un fichero de 
configuración con la última configuración que funcionó  e intenta aplicarlo. Si abre el puerto e
identifica correctamente ya está el puerto listo para funcionar.  Si no, busca el instrumento en
todos los puertos serie activos del PC hasta que lo encuentra y abre el puerto. Si la función no 
consigue abrir un puerto devueve -1.
 
Instrumento conectado en un puerto COM sin la librería segainvex_scpi_Serial:

 Si el instrumento está programado con la librería Segainvex_scpi_Serial  responderá al comando IDN
devolviendo su cadena de identificación. Pero también se puede conectar un instrumento  sin 
Segainvex_scpi_Serial.  Si no sabemos en que puerto está el instrumento cargamos el panel de  configuración:
	pcom_muestra_el_panel_de_configuracion();
 Pero el modo inicial de funcionamiento es "usuario", solo puede cambiar "Retardo" y "Timeout" del puerto 
serie y el resto de controles están dimados. Hacer doble click con el botón  derecho del ratón en el led 
y pasar a modo "master" para poder cambiar  todos los controles del panel de la configuración del puerto y
 igualarla a la del instrumento que tengamos conectado (configuración que tenemos que conocer de antemano).
 Para empezar directamente en modo master hay que ejecutar primero pcom_modo_pcom(0); y luego 
	pcom_muestra_el_panel_de_configuracion(). 

Para encontrar el instrumento podemos poner la  configuración que coincida con la del puerto del instrumento 
y pulsar "buscar puertos" y seleccionar alguno de los que encuentre y pulsar "aplicar".
 
 
 La recepción automática está activada por defecto, por lo que al enviar un  comando a Arduino que provoque 
una respuesta del instrumento, se ejecutará la función void pcom_datos_recibido(void), que tenemos que definir
en el código de cada aplicación. En esta aplicación hay que discriminar el dato que se ha recibido es y 
procesarlo. Los datos recibidos automáticamente están en char CadenaRespuesta[], esta  cadena esta terminada
 con TERMINADOR y '\0'. TERMINADOR es configurable, por defecto es '\n'.

 Si queremos enviar un dato y leer la respuesta de forma no automática, hay que desactivar la recepción 
automática.
 
Código para enviar un comando y recibir respuesta sin modo automático:

 	char DatosRecibidos[256]; //Cadena para recibir datos del instrumento
  //...
 	pcom_recepcion_automatica(NO); //Evita la recepción automática	
	sprintf(CadenaComando,"%s","*IDN"); /*Pone el comando en la cadena de salida       */
 	ENVIAR_COMANDO_AL_SISTEMA(MOSTRAR) /*Envia el comando en la cadena de salida       */
 	pcom_recepcion_automatica(SI); /*Habilita la recepción automática	           */
	pcom_recibir_datos (DatosRecibidos,1);/*Que rellena la cadena DatosRecibidos       */
	/*Ya podemos procesar los datos recibidos en DatosRecibidos*/
  
Al ejecutar el macro ENVIAR_COMANDO_AL_SISTEMA se envía por el puerto serie y se produce un delay de duración
"Retardo", que es un parámetro configurable.
	
En la ventana de mensajes, haciendo doble click con el botón derecho del ratón  se cierra y abre el puerto 
serie. Esto debe usarse solo para depuración.

Cuando se cierra la aplicación y esta ha funcionado  con un puerto COM ,se guarda la configuración en un fichero:
C:\\LabWindowsCVI\\PuertoCOM.ini
Cuando abra de nuevo la aplicación, esta aplicará el fichero si el instrumento está conectado al sistema,
esto es más rápido que si la aplicación tiene que buscar el instrumento en el puerto.



