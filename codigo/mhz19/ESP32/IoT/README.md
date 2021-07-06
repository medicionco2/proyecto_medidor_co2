
## thingspeak

Thingspeak (https://thingspeak.com) es un proyecto open source (https://github.com/iobridge/thingspeak) creado en 2010. Actualmente, está integrado con el software MATLAB de MathWorks al punto que hay que crear una cuenta en MathWorks para usar la plataforma en thingspeak.com. 

Para usar esta alternativa hay que: 

1. Comenzar siguiendo los pasos que se explican en https://la.mathworks.com/help/thingspeak/collect-data-in-a-new-channel.html.

2. Crear un channel y configurarlo para recibir dos "Fields"

![Channel Settings](https://user-images.githubusercontent.com/30849839/124671698-40454000-de8c-11eb-8f03-300eba48fb20.png)

3. Editar el código indicando tu "Write API Key" en thingspeak.

4. Configurar el ESP32 con ese programa. 

El medidor en funcionamiento va a comenzar a enviar datos a la plataforma thingspeak. Vas a poder configurar visualizaciones de manera más o menos intuitiva de manera que veas algo así:

![thingspeak Visualization](https://user-images.githubusercontent.com/30849839/124672197-03c61400-de8d-11eb-9660-80e39350f08e.png)

Licencia seleccionada: FREE (For small non-commercial projects)

  - Scalable for larger projects:   No. Annual usage is capped.
  - Number of messages:             3 million/year (~8200/day)
  - Message update interval limit:  Every 15 seconds 
  - Number of channels:             4
  - MATLAB Compute Timeout:         20 seconds
  - Number of simultaneous MQTT subscriptions: Limited to 3
  - Private channel sharing:        Limited to 3 shares
  - Technical Support:            	Community Support 

Para tener más nivel de servicio (conectar más sensores, compartir con más gente, mejores tasas de datos, etc.), hay que adquirir alguna licencia de pago: Standard, Academic, Student, o Home.
