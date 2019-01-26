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
Para verlo, leer las instrucciones en PuertoCOM.h y segainvex_scpi_Serial.h.


