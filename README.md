# Virtual-instrumentation-Arduino-LabWindows-CVI
To build this project an Arduino Zero or another board with a SAMD21 microcontroller is required.
To use a timer the library is used:

https://github.com/avandalen/avdweb_SAMDtimer

In this application it shows how to make virtual instrumentation with Arduino and the library
segainvex_scpi_Serial plus the programming environment of National Instruments LabWindows CVI,
with which the library has been programmed for this "PuertoCOM" environment to easily manage a port
COM through which communicates with the USB port where the Arduino is connected.

To run the application, you must install the LabWindows CVI Runtime Engine.
http://www.ni.com/download/labwindowscvi-run-time-engine-2012/3032/en/

Once installed, copying the application files into a directory can now be executed
the file CapturaMuestras.exe. (Actually, you only need to copy CapturaMuestrasSAMD21.uir and
CapturaMuestras.exe) It is not peremtry installing LabWindows CVI programming environment. Is a
expensive environment, I use it under with the UNIVERSIDAD AUTONOMA DE MADRID (SEGAINVEX-ELECTRÓNICA).

The application has a button to request samples from the Arduino (SAMD21, Arduino Zero). East
send the samples and the application plots them. Arduino reads samples from analog input A1.
In this entrance you can put a cable to the air with some turns and you will catch the induction of the network
electricity that in Spain is 50Hz and that is what you will see in the graph.

The important thing is to see how the segainvex_scpi_Serial library is used in Arduino and PuertoCOM in the PC.
To see it, read the instructions in PuertoCOM.h and segainvex_scpi_Serial.h.

En Español:

Para montar este proyecto se requiere un Arduino Zero o otra placa con un microcontrolador SAMD21.
Para utilizar un timer se usa la librería:

https://github.com/avandalen/avdweb_SAMDtimer

En esta aplicación se muestra como hacer instrumentación virtual con Arduino y la librería 
segainvex_scpi_Serial más el entormo de programación de National Instrumentos LabWindows CVI, 
con el que se ha programado la librería para este entorno "PuertoCOM" para manejar fácilmente un puerto
COM a través del cual se comunica con el puerto USB donde se conectar el Arduino.

Para ejecutar la aplicación hay que instalar el Runtime Engine de LabWindows CVI.
http://www.ni.com/download/labwindowscvi-run-time-engine-2012/3032/en/

Una vez instalado, con copiar los ficheros de la aplicación en un directorio ya se puede ejecutar 
el fichero CapturaMuestras.exe. (En realidad solo hace falta copiar CapturaMuestrasSAMD21.uir y 
CapturaMuestras.exe) No hay que tener el entorno de programación LabWindows CVI. Es un 
entorno caro, yo lo uso bajo con de la Universidad Autónoma de Madrid (SEGAINVEX-ELECTRÓNICA).

La aplicación tiene un botón para solicitarle muestras al Arduino (SAMD21, Arduino Zero). Este 
envía las muestras y la aplicación las plotea. Arduino lee las muestras de la entrada analógica A1.
En esta entrada se puede poner un cable al aire con unas espiras y se captará la inducción de la red 
electrica que en España es de 50Hz y eso es lo que se verá en la gráfica.

Lo importante es ver como se usa la librería segainvex_scpi_Serial en Arduino y PuertoCOM en el PC. 
Para verlo, leer las instrucciones en PuertoCOM.h y segainvex_scpi_Serial.h.


