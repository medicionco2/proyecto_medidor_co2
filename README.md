# Medidor CO2 para prevención de la transmisión del Covid19 en ambientes interiores

> Este proyecto está en plena evolución. Este texto, los programas, y demás materiales se están actualizando frecuentemente.

Al respirar, absorbemos oxígeno del aire y al exhalar expulsamos dióxido de carbono (CO2). En ambientes interiores, con el correr de los minutos, si no se ventila, la respiración de las personas allí presentes hace que suba la concentración de CO2. Dicho en números, al aire libre la concentración de CO2 es de 400 ppm (partes por millón), mientras que en la exhalación de las personas, hay 40 mil ppm. Se ha demostrado que hay correlación entre esa concentración y el riesgo de contagio de enfermedades respiratorias. Por supuesto, ese riesgo varía según la proporción de personas infectadas en la región. Se estima que si la concentración de CO2 alcanza las 800 ppm, eso quiere decir que el 1% del aire que respiramos ya fue respirado por otra persona. Cuando se supera ese valor, la permanencia en ese espacio comienza a ser riesgosa. Recordemos que hay una antigua prescripción médica sobre ventilar para prevenir el contagio de enfermedades respiratorias. En este tiempo de pandemia de COVID-19, esa recomendación se transformó en la principal para minimizar el riesgo de contagio. En resumen, es importante medir la concentración de CO2 para saber si un ambiente interior está suficientemente ventilado dada una cierta cantidad de gente y el tiempo transcurrido realizando una actividad. 

![Contagio por aerosoles](https://user-images.githubusercontent.com/30849839/124362371-c81c1780-dc0a-11eb-9723-1243cc2c4425.png)

> Tiene que quedar claro que si en un ambiente se mide más de 2000 ppm de CO2, hay que tomar medidas urgentemente porque realizar actividades allí es peligroso.

Se propone el diseño de un dispositivo medidor de CO2, de uso orientativo, para espacios interiores, como medida de prevención de enfermedades infecciosas que se contagian mediante aerosoles (particularmente, COVID-19). 

El objetivo es que el dispositivo propuesto sea de uso sencillo, lo más económico posible, de diseño libre, y con el potencial para desarrollar toda la funcionalidad de los dispositivos comerciales más avanzados. El dispositivo solamente tiene cuatro componentes. Además del sensor y la computadora, hay un LED RGB para indicar, mediante colores, el nivel de concentración de CO2 en el ambiente interior donde se use. El cuarto componente, es simplemente un pulsador. 

El diseño cuenta con una computadora de placa reducida, también llamado de placa única, de placa simple (SBC), o sistema en un chip (SoC), de bajo costo. En este caso, el sistema propuesto es de la familia ESP32 con tecnología Bluetooth y WiFi integrada. Por eso último, el instrumento no va a necesitar una pantalla, en su lugar, una app para Android o una aplicación web, van a mostrar los valores de CO2 actuales, las series de tiempo, y otros análisis para estudio y toma de decisiones (por ejemplo, ventilar más, reducir el aforo, etc.).

La situación sanitaria evidencia la necesidad de usar este tipo de dispositivos, no solo en esta coyuntura, sino post-pandemia, como elemento que apunta a asegurar un nivel de calidad de aire desde el punto de vista sanitario. En otras palabras, sería un elemento más de seguridad e higiene en espacios donde se realicen diferentes tipos de actividades humanas, incluida la educación en todos sus niveles.

### ¿Qué son los aerosoles?

Cuando hablamos, gritamos, cantamos, tosemos, estornudamos o simplemente respiramos, exhalamos partículas grandes y también partículas pequeñas ("aerosoles") que están formadas por saliva o líquido proveniente de la tráquea, los pulmones, etc. Lo mismo que las partículas grandes, estos aerosoles pueden "portar" virus, que pueden infectar cuando se inhalan.

![Aerosoles](https://user-images.githubusercontent.com/30849839/124509071-a310db80-dda7-11eb-9a73-ab920b335489.png)


## Lista de materiales

Cualquier persona o grupo con entusiasmo, con un poco de conocimiento de electrónica e informática, y muy pocas herramientas puede construír este medidor. 

Se necesitan cuatro componentes más los cables de conexión y el gabinete.
1. ESP32: Hay diversos modelos y versiones del SoC ESP32.
2. Sensor MH-Z19b: asegurarse que es original (Winsen)

![Sensor MH-Z19b](https://user-images.githubusercontent.com/30849839/124179068-a5fa8c00-da88-11eb-9282-0e8343943e51.png)

3. Pulsador
4. LED RGB más 3 resistencias 220 ohm. Por simplicidad, se recomienda que sea en un solo módulo, como en la siguiente figura:

<img src="https://user-images.githubusercontent.com/30849839/124174887-45b51b80-da83-11eb-9d41-7c1573fa00e8.png" alt="Modulo LED RGB" width="200"/>

En esta aplicación, el significado de los colores es el siguiente:
- Verde significa baja concentración de CO2, bajo riesgo de contagio por aerosoles.
- Amarillo es acercándose a riesgo alto.
- Rojo es riesgo alto, debe ventilarse el ambiente.
- Violeta es riesgo alto, debería evacuarse momentáneamente el ambiente.

El LED en azul significa que el sensor se está calentando o que el aparato está en modo de calibración.

La caja puede ser una caja de paso de las que se usan en electricidad, de las más pequeñas. Es importante que hagas dos ranuras por donde el sensor vea el exterior de la caja. Las ranuras son para las dos "ventanitas" blancas que se ven en la figura del sensor.

### Sobre el sensor

El sensor seleccionado, MH-Z19b, se basa en tecnología de detección de gas por absorción de luz infrarroja. Más precisamente, la tecnología que usa se conoce como NDIR (del inglés nondispersive infrared).

No todos los sensores MH-Z19b que se pueden adquirir son iguales, su diseño y su firmware han ido evolucionando. La librería que usamos permite acceder al "Firmware Version", por ejemplo los que tenemos son versión 5.02. Hay que notar que el fabricante (Winsen, China) no ha documentado todos los cambios y algunas veces hay que recurrir a ingeniería inversa para descubrir algunas de sus características. Aún así, se considera que es un sensor económico con una relación precio/prestaciones muy buena.

Advertencia: El sensor es sensible a la electricidad estática. Tratar al sensor con ciudado, es un componente delicado.

## Conexión de componentes

![Diagrama de conexiones](https://github.com/medicionco2/proyecto_medidor_co2/blob/main/diagramas_conexion/diagrama_esp32.png)

La alimentación del dispositivo se hace mediante un cargador USB estándar.

El sensor puede venir con un cable armado con una ficha que no es compatible con los conectores del ESP32. Por eso, una alternativa es cortar el cable y empalmar cables dupont.

Para más ayuda, [este video](https://www.youtube.com/watch?v=vmDCqNXVZKQ) muestra en detalle cómo se conecta el sensor MH-Z19b al ESP32. 

En resumen, conectar el sensor requiere de 4 cables: 
- Alimentación (5V): cable rojo del sensor 
- tierra (GND): cable negro
- y los dos cables de la conexión serie: cable azul a Tx y cable verde a Rx. 

## Programación

1. Instalar IDE Arduino en una PC. 
2. Instalar soporte para ESP32.
Las instrucciones para estos dos primeros pasos se pueden ver en [este enlace](https://randomnerdtutorials.com/installing-the-esp32-board-in-arduino-ide-windows-instructions/) 
	
3. Instalar librería para el sensor MH-Z19b.
	https://downloads.arduino.cc/libraries/github.com/WifWaf/MH_Z19-1.5.3.zip
	
   En los ejemplos de hasta 27 Junio 2020 usábamos:
	https://github.com/piot-jp-Team/mhz19_uart 
4. Copiar y compilar el código fuente seleccionado usando Arduino IDE.

5. Descargar el código ejecutable en el ESP32 de manera que quede programado con la aplicación seleccionada.

## Funcionamiento

Una vez conectado el cable USB, el dispositivo se pone en funcionamiento. No hay botón de encendido. El sensor tarda tres minutos para iniciar sus medidas, eso se indica con el LED encendido en color azul. Pasado ese tiempo, el LED indica la concentración de CO2 mediante colores: verde (poca concentracón de CO2, más seguro), amarillo, rojo y violeta (muy alta concentración de CO2, menos seguro). A su vez, se envían las medidas de CO2 y temperatura por WiFi o Bluetooth, según cuál programa se haya cargado en el dispositivo. 

Si se seleccionó la versión Bluetooth, se pueden ver las medidas en un teléfono inteligente. Una posibilidad es usar la app "Serial Bluetooth Terminal" que se pude descargar en el teléfono desde https://play.google.com. En ese caso vas a ver los datos que vienen del medidor de esta manera:

<img src="https://user-images.githubusercontent.com/30849839/124181533-fe7f5880-da8b-11eb-9bb3-cf72c6b72cdd.png" alt="Captura Serial Bluetooth Terminal" width="300"/>

### Cómo debería usarse el medidor

En un espacio interior con cierta ventilación fija, por ejemplo con las ventanas parcialmente abiertas, se debería llegar a una situación en la cual el CO2 que exhalan las personas sea igual a la cantidad de CO2 que sale por las ventanas, de modo que el nivel de CO2 en la habitación se mantenga aproximadamente constante. El objetivo debe ser que ese valor estable de CO2 no supere los 700 a 800 ppm.

Si se observan valores estables de CO2 demasiado altos, superiores a 800 ppm, se deben abrir las ventanas un poco más. Si eso no es posible porque la temperatura del exterior es demasiado baja, se debe reducir el número de personas que realiza la actividad. Si la temperatura exterior fuera confortable, se recomienda abrir las ventanas completamente.

El viento hace que mejore la ventilación. En días ventosos, es probable que se logren los valores deseados de CO2 con menor apertura de las ventanas.

> Ventilar los pasillos en las escuelas (y otros edificios con varios espacios comunicados) es muy importante, ya que de lo contrario transfieren el aire exhalado entre las aulas.

### En qué lugar del espacio interior colocar el medidor

Lo ideal es colocarlo a la altura de la respiración de las personas (1,5 m) del lado opuesto a las ventanas. No colocarlo muy cerca de las personas.

Se puede experimentar moviendo el medidor de CO2 a diferentes lugares de la habitación. Si se detecta una zona menos ventilada, allí debe colocarse el medidor para que indique el caso más desfaborable.

### Calibración

Al aire libre, la concentración de CO2 es cercana a las 400 ppm. Por eso, el dispositivo se calibra al aire libre y de esa manera se establece una referencia.

Para la calibración, hay que poner el dispositivo al aire libre, oprimir el pulsador por al menos un segundo y esperar al menos 20 minutos. En ese estado, el LED está encendido en color azul con intensidades variables que se repiten cíclicamente. 

## Propuesta

Con motivaciones parecidas, en Internet existen varios proyectos como éste. En este caso, el concepto que nos guía es el de IoT, Internet de las Cosas, donde el hardware es mínimo, con conexión a Internet, y donde el valor está en el software, hasta llegar a usar una plataforma IoT. Esas plataformas (existen más de 600 diferentes) proveen servicios como seguridad, actualización remota del software, visualización de los datos, gestión de eventos, gestión de los propios dispositivos en el caso que se usen varios, o muchos, y por qué no miles.

Otra diferencia es que este proyecto se centra en un solo tipo de sensor y un solo tipo de "computadora de placa simple". Existen varias alternativas para ambos, pero para quien comienza, creemos que es más sencillo tener una sola plataforma. Como decíamos, el concepto que nos guía es que el valor va a estar en el software. En cuanto al software, sí proponemos diferentes versiones para Wifi, Bluetooth, con funcionalidad básica, y esperemos que pronto, funcionalidad más avanzada.

> Y lo más importante de todo: Generar conciencia sobre la seguridad e higiene del aire que respiramos en ambientes interiores. <img src="https://user-images.githubusercontent.com/30849839/124317329-6053c700-db4d-11eb-964f-cf9fad7fc8e3.png" alt="Higiene" width="200"/>


Te invitamos a sumarte a este proyecto, a construír tu propio medidor, compartir resultados, y a extender el hardware pero sobre todo, el software. Para consultas, por favor, escribir a etodorov@exa.unicen.edu.ar.

## Más Información

- Este artículo del diario "El País", de octubre de 2020, es insuperable desde el punto de vista comunicacional. https://elpais.com/ciencia/2020-10-24/un-salon-un-bar-y-una-clase-asi-contagia-el-coronavirus-en-el-aire.html

- [Importancia de la ventilación para la prevención de la COVID-19](https://youtu.be/fpMb1TzraGw).  Canal del Ministerio de Ciencia, Tecnología e Innovación, 26 marzo de 2021.   

- [Guía de evaluación de la frecuencia de ventilación](https://schools.forhealth.org/ventilation-guide/). Programa "Healthy Buildings" de la Escuela de Salud Pública T.H. Chan, Universidad de Harvard. 

- [Guía del Consejo Superior de Investigaciones Científicas](https://www.csic.es/en/node/1259002) (España) sobre cómo se deben ventilar para reducir el riesgo de contagio por Covid-19. 

- [Estimación de riesgo de contagio simplificada](https://www.nationalgeographic.com/science/article/how-to-measure-risk-airborne-coronavirus-your-office-classroom-bus-ride-cvd) de National Geographic.

- Natalia Rubinstein, Ciclo Ciencia para ventilar: [Yo ventilo, ella ventila, vos ventilás. ¿Pero lo hacemos bien?](https://youtu.be/4q3odycwvYs).

- Sandra Cordo, Ciclo Ciencia para ventilar: [Transmisión de Covid-19 por aire. Principales medidas de prevención](https://youtu.be/4AUWS4Dpn0A).

- Alejandro Nadra, Juan Fernández Zaragoza, Ciclo Ciencia para ventilar: [¿Qué son las tecnologías libres y qué aportan en el contexto de pandemia?](https://youtu.be/301sSYFhCg8).

- Proyectos similares y material técnico

	- Página del Dr. Jorge Aliaga ([Medición de CO2](http://www.jorgealiaga.com.ar/?page_id=2864)) 
      Sw: https://github.com/jlaliaga/Medidor-de-CO2/releases/tag/V2 
      
	- [Un medidor de CO2](https://emariete.com/medidor-casero-co2/).

	- [Ciudado con los sensores *fake*](https://youtu.be/dWYMuFZt8kk).

	- [Proyecto CODOS](https://github.com/miguelangelcasanova/codos).

	- [Comparativa de sensores](https://soloelectronicos.com/2020/11/01/monitor-de-co2-con-sensor-mh-z19/): Recomiendan el MH-Z19, lo mismo que Jorge Aliaga. La ventaja adicional, que no mencionan, es que al tener medida de temperatura junto con CO2, se puede controlar la habitabilidad de un espacio con un criterio CO2 y temperatura combinado.
      
 - Artículos científicos

	- Zhe Peng and Jose L. Jimenez, "Exhaled CO2 as a COVID-19 Infection Risk Proxy for Different Indoor Environments and Activities", Environmental Science & Technology Letters, 2021, 8 (5), 392-397. https://pubs.acs.org/doi/10.1021/acs.estlett.1c00183

	- Greenhalgh T, Jimenez JL, Prather KA, Tufekci Z, Fisman D, Schooley R. "Ten scientific reasons in support of airborne transmission of SARS-CoV-2". Lancet. 2021; 397: 1603-1605. https://www.thelancet.com/journals/lancet/article/PIIS0140-6736(21)00869-2/fulltext

	- Liu, Y, Misztal, PK, Xiong, J, et al., "Detailed investigation of ventilation rates and airflow patterns in a northern California residence", Indoor Air, 2018; 28: 572–584. https://onlinelibrary.wiley.com/doi/10.1111/ina.12462

	- Johansson MA, Quandelacy TM, Kada S, et al. SARS-CoV-2 transmission from people without COVID-19 symptoms. JAMA Netw Open 2021; 4: e2035057.

- El proyecto en los medios

	- Boletín de la Facultad de Ciencias Exactas - Número 273 - Crean un medidor de Dióxido de Carbono que busca concientizar la importancia de la ventilación de los ambientes para evitar la propagación del COVID-19, 22 JUN 2021. (En redes sociales de la Facultad de Ciencias Exactas, UNICEN)

	- Diario "ABC Hoy" de Tandil, 23 JUN 2021. http://www.abchoy.com.ar/leernota.php?id=175998&titulo=medidor_dioxido_carbono_para_concientizar_sobre_ventilacion_para_prevencion_del_covid_19

	- Diario "El Eco" de Tandil, 23 JUN 2021. https://www.eleco.com.ar/la-ciudad/crean-un-medidor-de-dioxido-de-carbono-que-busca-concientizar-la-importancia-de-la-ventilacion-de-los-ambientes

	- Boletín de la Facultad de Ciencias Exactas - Número 274 - Reunión con la Dirección Nacional de Desarrollo Tecnológico e Innovación del MinCYT, 28 JUN 2021. (En redes sociales de la Facultad de Ciencias Exactas, UNICEN)

	- Diario "El Eco" de Tandil, 3 JUL 2021. https://www.eleco.com.ar/la-ciudad/investigadores-de-exactas-inventaron-un-medidor-de-dioxido-de-carbono-con-un-diseno-economico-y-simple


¿Se puede usar esta información en otras publicaciones?

Si, se puede utilizar. De hecho, nosotros consultamos numerosas fuentes que tratamos de documentar en esta sección. No es necesario que te pongas en contacto con nosotros para pedirnos permiso. Sólo te pedimos que indiques el enlace a esta página como fuente de la información. Si es posible, incluir la fecha en la que viste la información. Si necesitas una lista de autores, sólo tienes que enumerar a las personas que contribuyen en este proyecto en la fecha que accediste.

Con respecto al código fuente, recuerda que tiene licencia GPL.
