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

En este momento estamos desarrollando una caja a medida imprimible 3D. Gracias por su colaboración a [Industria Jarandat](https://www.facebook.com/jarandat)! 

<img src="https://user-images.githubusercontent.com/30849839/156411221-d3dcaaec-e206-4a31-9bd7-23d9b3e94b43.png" alt="Caja" width="300"/>

Mientras tanto, o alternativamente, a caja puede ser una caja de paso de las que se usan en electricidad, de las más pequeñas. Es importante que hagas dos ranuras por donde el sensor vea el exterior de la caja. Las ranuras son para las dos "ventanitas" blancas que se ven en la figura del sensor.

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
- y los dos cables de la conexión serie: cable azul a pin Tx2 (o puede aparecer impreso con número 17) y cable verde a Rx2 (o 16). 

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

![imagen](https://user-images.githubusercontent.com/30849839/126645507-74f327bf-ace0-428e-a21a-a52ff74a0b01.png)


### En qué lugar del espacio interior colocar el medidor

Lo ideal es colocarlo a la altura de la respiración de las personas (1,5 m) del lado opuesto a las ventanas. No colocarlo muy cerca de las personas.

Se puede experimentar moviendo el medidor de CO2 a diferentes lugares de la habitación. Si se detecta una zona menos ventilada, allí debe colocarse el medidor para que indique el caso más desfaborable.

### Calibración

Al aire libre, la concentración de CO2 es cercana a las 400 ppm. Si ves que en un ambiente muy bien ventilado, el medidor está dando una medida alejada de esas 400 ppm, seguramente haga falta calibrarlo.

Si bien existen diferentes [técnicas de calibración](https://www.co2meter.com/blogs/news/7512282-co2-sensor-calibration-what-you-need-to-know), en este caso vamos a poner el dispositivo al aire libre. Hay que notar que el proceso se ve afectado por el viento y las temperaturas extremas. Alternativamente, el medidor se puede calibrar en un espacio MUY bien ventilado. Para comenzar el proceso de calibración, oprimir el pulsador por al menos un segundo y esperar al menos 20 minutos. En ese estado, el LED está encendido en color azul con intensidades variables que se repiten cíclicamente. 

## Propuesta

Con motivaciones parecidas, en Internet existen varios proyectos como éste. En este caso, el concepto que nos guía es el de *IoT, Internet de las Cosas*, donde el hardware es mínimo, con conexión a Internet, y donde el valor está en el software, hasta llegar a usar una plataforma IoT. Esas plataformas (existen más de 600 diferentes) proveen servicios como seguridad, actualización remota del software, visualización de los datos, gestión de eventos, gestión de los propios dispositivos en el caso que se usen varios, o muchos, y por qué no miles.

Otra diferencia es que este proyecto se centra en *un solo tipo de sensor y un solo tipo de "computadora de placa simple"*. Existen varias alternativas para ambos, pero para quien comienza, creemos que es más sencillo tener una sola plataforma. Como decíamos, el concepto que nos guía es que el valor va a estar en el software. En cuanto al software, sí proponemos diferentes versiones para Wifi, Bluetooth, con funcionalidad básica, y esperemos que pronto, funcionalidad más avanzada.

> Y lo más importante de todo: Generar conciencia sobre la seguridad e higiene del aire que respiramos en ambientes interiores. <img src="https://user-images.githubusercontent.com/30849839/124317329-6053c700-db4d-11eb-964f-cf9fad7fc8e3.png" alt="Higiene" width="200"/>

Te invitamos a sumarte a este proyecto, a construír tu propio medidor, compartir resultados, y a extender el hardware pero sobre todo, el software. Para consultas, por favor, escribir a etodorov@exa.unicen.edu.ar.

## ¿Estos medidores compiten con los medidores comerciales?

En este proyecto, de ninguna manera competimos con los medidores comerciales. De hecho, te invitamos a visitar [páginas de fabricantes de medidores en Argentina](https://www.argentina.gob.ar/ciencia/unidad-coronavirus/ventilar/produccion-local-medidores) para más información. Los medidores de este proyecto pueden complementar, llegando a los sectores de menos recursos donde las soluciones comenciales son inaccesibles, ayudando al objetivo fundamental de la prevención y la toma de consciencia sobre la vía de contagio por aerosoles. 

## Más Información

- Este artículo del diario "El País", de octubre de 2020, es insuperable desde el punto de vista comunicacional. https://elpais.com/ciencia/2020-10-24/un-salon-un-bar-y-una-clase-asi-contagia-el-coronavirus-en-el-aire.html

- [Importancia de la ventilación para la prevención de la COVID-19](https://youtu.be/fpMb1TzraGw).  Canal del Ministerio de Ciencia, Tecnología e Innovación, 26 marzo de 2021.   

- [Recomendaciones para la prevención de transmisión de COVID-19 por aerosoles](https://www.argentina.gob.ar/sites/default/files/covid-19-prevencion-de-transmision-por-aerosoles-2021.pdf) del Ministerio de Ciencia, Tecnología e Innovación y Ministerio de Salud.

- Campaña [Ventilar](https://www.argentina.gob.ar/ciencia/unidad-coronavirus/ventilar) para la prevención de la transmisión del COVID-19 por aerosoles, del Ministerio de Ciencia, Tecnología e Innovación, Argentina.

- [Guía de evaluación de la frecuencia de ventilación](https://schools.forhealth.org/ventilation-guide/). Programa "Healthy Buildings" de la Escuela de Salud Pública T.H. Chan, Universidad de Harvard. 

- [Guía del Consejo Superior de Investigaciones Científicas](https://www.csic.es/en/node/1259002) (España) sobre cómo se deben ventilar para reducir el riesgo de contagio por Covid-19. 

- [Estimación de riesgo de contagio simplificada](https://www.nationalgeographic.com/science/article/how-to-measure-risk-airborne-coronavirus-your-office-classroom-bus-ride-cvd) de National Geographic.

- Natalia Rubinstein, Ciclo Ciencia para ventilar: [Yo ventilo, ella ventila, vos ventilás. ¿Pero lo hacemos bien?](https://youtu.be/4q3odycwvYs).

- Sandra Cordo, Ciclo Ciencia para ventilar: [Transmisión de Covid-19 por aire. Principales medidas de prevención](https://youtu.be/4AUWS4Dpn0A).

- Alejandro Nadra, Juan Fernández Zaragoza, Ciclo Ciencia para ventilar: [¿Qué son las tecnologías libres y qué aportan en el contexto de pandemia?](https://youtu.be/301sSYFhCg8).

---

- [Jornada sobre Ventilación para bares y restaurantes](https://youtu.be/Q7DBwQ0G1Hc) en contexto de COVID 19, donde se presentaron [líneas de financiamiento](https://www.argentina.gob.ar/servicio/obtener-un-credito-para-mi-restaurante-o-bar) de la Secretaría de la Pequeña y Mediana Empresa y los Emprendedores de Argentina, hasta 31 de octubre de 2021.

---

- Proyectos afines y material técnico

	- Proyecto del [Dr. Jorge Aliaga]((http://www.jorgealiaga.com.ar/?page_id=2864)) de [Medición de CO2](https://github.com/jlaliaga/Medidor-de-CO2)  

	- Proyecto [Aireamos](https://www.aireamos.org), como coalición de grupos liderados por científicos en el área.

		- [Comparativas y guías](https://www.aireamos.org/medidores/) sobre medidores comerciales de CO2.
      
	- [Proyecto LibreCO2](https://github.com/danielbernalb/LibreCO2) de [Daniel Bernal](https://aireciudadano.com/guia-de-construccion-medidor-libreco2/) donde se propone el uso de tarjetas Arduino.
      
	- Un [medidor de CO2](https://emariete.com/medidor-casero-co2/) de eMariete.

		- [Ciudado con los sensores *fake*](https://youtu.be/dWYMuFZt8kk).

	- [Proyecto CODOS](https://github.com/miguelangelcasanova/codos).

	- [Comparativa de sensores](https://soloelectronicos.com/2020/11/01/monitor-de-co2-con-sensor-mh-z19/): Recomiendan el MH-Z19, lo mismo que Jorge Aliaga. Aunque la medida de temperatura que provee el propio sensor no es precisa, al tener esa medida junto con CO2, se puede controlar la habitabilidad de un espacio con un criterio CO2 y temperatura combinado.

	- ¿Cómo funcionan los sensores de tecnología NDIR? Explicación sencilla en [co2meter.com](https://www.co2meter.com/blogs/news/6010192-how-does-an-ndir-co2-sensor-work)
      
 - Artículos científicos

	- Zhe Peng and Jose L. Jimenez, "Exhaled CO2 as a COVID-19 Infection Risk Proxy for Different Indoor Environments and Activities", Environmental Science & Technology Letters, 2021, 8 (5), 392-397. https://pubs.acs.org/doi/10.1021/acs.estlett.1c00183

	- Greenhalgh T, Jimenez JL, Prather KA, Tufekci Z, Fisman D, Schooley R. "Ten scientific reasons in support of airborne transmission of SARS-CoV-2". Lancet. 2021; 397: 1603-1605. https://www.thelancet.com/journals/lancet/article/PIIS0140-6736(21)00869-2/fulltext

	- Liu, Y, Misztal, PK, Xiong, J, et al., "Detailed investigation of ventilation rates and airflow patterns in a northern California residence", Indoor Air, 2018; 28: 572–584. https://onlinelibrary.wiley.com/doi/10.1111/ina.12462

	- Johansson MA, Quandelacy TM, Kada S, et al. "SARS-CoV-2 transmission from people without COVID-19 symptoms". JAMA Netw Open 2021; 4: e2035057.

	- Lidia Morawska, Donald K Milton, "It Is Time to Address Airborne Transmission of Coronavirus Disease 2019 (COVID-19)", Clinical Infectious Diseases, Volume 71, Issue 9, 1 November 2020, Pages 2311–2313. https://academic.oup.com/cid/article/71/9/2311/5867798
		- Este paper fue suscrito por más de 200 científicos.

	- Hou YJ, et al. SARS-CoV-2 Reverse Genetics Reveals a Variable Infection Gradient in the Respiratory Tract. Cell. 2020 Jul 23;182(2):429-446. https://www.ncbi.nlm.nih.gov/pmc/articles/PMC7250779/pdf/main.pdf

	- Miller, SL, Nazaroff, WW, Jimenez, JL, et al. Transmission of SARS-CoV-2 by inhalation of respiratory aerosol in the Skagit Valley Chorale superspreading event. Indoor Air. 2021; 31: 314– 323. https://onlinelibrary.wiley.com/doi/epdf/10.1111/ina.12751

	- Shrey Trivedi, Savvas Gkantonas, Léo C. C. Mesquita, Salvatore Iavarone, Pedro M. de Oliveira, and Epaminondas Mastorakos, "Estimates of the stochasticity of droplet dispersion by a cough", Physics of Fluids 33, 115130 (2021) https://doi.org/10.1063/5.0070528 
		- Este paper fue citado en un [artículo periodístico](https://www.pagina12.com.ar/385125-coronavirus-advierten-que-con-el-distanciamiento-social-no-a) donde los investigadores concluyeron que el distanciamiento de dos metros no debe considerarse una medida de seguridad por sí sola, sino que para ser efectiva tiene que estar acompañada por la utilización de barbijos, la ventilación de espacios y la vacunación. “Todos estamos desesperados por ver la parte posterior de esta pandemia, pero recomendamos encarecidamente que las personas sigan usando máscaras", al menos en "espacios interiores", dijo Epaminondas Mastorakos, experto en mecánica de fluidos de la Universidad de Cambridge. "No hay ninguna buena razón para exponerse a este riesgo mientras el virus esté con nosotros”, añadió.

- El proyecto en los medios

	- En el canal de YouTube de El Taller De TD ![imagen](https://user-images.githubusercontent.com/30849839/136961650-8044a72a-d548-4286-9769-e5bc1738fc94.png) [explicaron el proyecto](https://youtu.be/7OHqT15nE9w![imagen](https://user-images.githubusercontent.com/30849839/136962101-3a85b2da-e438-4487-a09c-d2df5bf979d4.png)
), 11 OCT 2021. ¡Gracias!

	- Exactas Dialoga, 5 OCT 2021. [Medidor sencillo de dióxido de carbono.](https://www.mixcloud.com/Exactasdialoga/exactas-dialoga-e5-medidor-sencillo-de-di%C3%B3xido-de-carbono/)

	- Diario "ABC Hoy" de Tandil, 30 SEP 2021. [Alumnos de 5° Año del Colegio Nuestra Tierra diseñaron un detector de Dióxido de Carbono.](http://www.abchoy.com.ar/leernota.php?id=178465&titulo=alumnos-5-ano-del-colegio-nuestra-tierra-disenaron-un-detector-dioxido-carbono)

	- Diario "La Voz de Tandil", 29 SEP 2021. [Estudiantes del Colegio Nuestra Tierra construyeron un medidor de dióxido de carbono.](https://www.lavozdetandil.com.ar/2021/09/29/estudiantes-del-colegio-nuestra-tierra-construyeron-un-medidor-de-dioxido-de-carbono)

	- Portal "Enfoque de Negocios", 16 JUL 2021. [Se fabricarán medidores de Dióxido de Carbono en Tandil.](https://www.enfoquedenegocios.com.ar/medidores-de-dioxido-de-carbono/)

	- [Presentación del proyecto](https://youtu.be/HmPg_Kn5Qhk) en el Encuentro IoT MdP/Tandil 2021 en el marco del proyecto [NEON](https://www.project-neon.eu/) de la Unión Europea, 13 JUL 2021. 

	- Diario "El Eco" de Tandil, 3 JUL 2021. [Investigadores de Exactas inventaron un medidor de dióxido de carbono con un diseño económico y simple.](https://www.eleco.com.ar/la-ciudad/investigadores-de-exactas-inventaron-un-medidor-de-dioxido-de-carbono-con-un-diseno-economico-y-simple)

	- Boletín de la Facultad de Ciencias Exactas - Número 274 - Reunión con la Dirección Nacional de Desarrollo Tecnológico e Innovación del MinCYT, 28 JUN 2021. (En redes sociales de la Facultad de Ciencias Exactas, UNICEN)

	- Diario "ABC Hoy" de Tandil, 23 JUN 2021. [Medidor de dióxido de carbono para concientizar sobre ventilación para prevención del COVID-19.](http://www.abchoy.com.ar/leernota.php?id=175998&titulo=medidor_dioxido_carbono_para_concientizar_sobre_ventilacion_para_prevencion_del_covid_19)

	- Diario "El Eco" de Tandil, 23 JUN 2021. https://www.eleco.com.ar/la-ciudad/crean-un-medidor-de-dioxido-de-carbono-que-busca-concientizar-la-importancia-de-la-ventilacion-de-los-ambientes

	- Boletín de la Facultad de Ciencias Exactas - Número 273 - Crean un medidor de Dióxido de Carbono que busca concientizar la importancia de la ventilación de los ambientes para evitar la propagación del COVID-19, 22 JUN 2021. (En redes sociales de la Facultad de Ciencias Exactas, UNICEN)

¿Se puede usar esta información en otras publicaciones?

Si, se puede utilizar. De hecho, nosotros consultamos numerosas fuentes que tratamos de documentar en esta sección. No es necesario que te pongas en contacto con nosotros para pedirnos permiso. Sólo te pedimos que indiques el enlace a esta página como fuente de la información. Si es posible, incluir la fecha en la que viste la información. Si necesitas una lista de autores, sólo tienes que enumerar a las personas que contribuyen en este proyecto en la fecha que accediste.

Con respecto al código fuente, recuerda que tiene licencia GPL, lo cual permite usar, estudiar, compartir (copiar) y modificar el software, siempre que preserves estas libertades.
