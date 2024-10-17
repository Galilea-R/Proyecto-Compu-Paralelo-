Este es el primer proyecto de computo pararalelo y en la nube de Sebastian Yamil Castellanos Gómez 198090 y Galilea Resendis González 196811. El proyecto se basa en hacer un algoritmo DBSCAN de manera serial y paralelizada, como parte de las especificaciones se debe de alcanzar un speedup minimo de 1.5 con la versión paralelizada. 

El proyecto lo corrimos en una computadora asus zenbook pro duo con un procesador de 12ª generación intel core i9, con 32 de RAM y 20 nucleos. 

Para la versión serial tenemos como funciones principales a: 
1) region_query: Esta función busca todos los vecinos de un punto específico (point_idx) que están dentro de una distancia (epsilon). Encontrar los vecinos de un punto es fundamental para algoritmo DBSCAN, ya que así va a poder formar clústeres basados en la densidad de puntos cercanos.
2) expand_cluster: Esta función expande un clúster agregando puntos que tienen suficientes vecinos dentro del radio epsilon.
3) dbscan: El algoritmo dbscan encuentra clústeres en un conjunto de datos basándose en la densidad de puntos cercanos

Para la verisón paralalelizada tenemos las mismas funciones que en la serial, solo que en cada función le decidimos agregar una parte paralelizada. Lo hicimos considerando que acciones se podrían maximizar sin que ocurrieran errores o tardara más tiempo que el serial.Modificamos: 

1)En el algoritmo DBSCAN para garantizar la coherencia de los identificadores de los clústeres cuando se corren en paralelo, se utiliza #pragma omp atomic al incrementar cluster_id, evitando que lleguen a ocurrir condiciones de carrera entre hilos. No se paraleliza el bucle principal en sí, ya que la expansión de clústeres (expand_cluster) podría provocar problemas de sincronización complejos debido a la dependencia de los datos durante la expansión.

2)La función region_query busca vecinos dentro de un radio epsilon alrededor de un punto dado. Para acelerar esta operación que consideramos podría ser costosa, se utiliza OpenMP con #pragma omp parallel para dividir la tarea entre múltiples hilos, y #pragma omp for con un dynamic ajustable dependiendo del tamaño de chunk para equilibrar la carga de trabajo y fuera más eficiente. Cada hilo acumula resultados localmente en private_neighbors para no tener tantos recursos compartidos. Deespués, se combina con el vector compartido neighbors usando #pragma omp critical. Esta sección se utiliza para evitar condiciones de carrera durante la modificación del vector.

3) La función expand_cluster se realiza de manera secuencial, pero utiliza #pragma omp critical cuando se insertan nuevos vecinos en el vector compartido neighbors. Esta decisión se tomó para evitar condiciones de carrera al agregar puntos a un clúster, ya que varios hilos podrían estar trabajando simultáneamente en la expansión. Aunque no toda la función es paralela, region_query se ejecuta en paralelo dentro de expand_cluster, lo que permite que la búsqueda de vecinos se beneficie del paralelismo, mientras que la expansión del clúster se realiza de forma controlada.

Las pruebas realizadas las hicimos con 20000,40000,80000,120000,140000,160000,180000,200000 datos generados de DBSCAN_noise.ipynb. En el código serial lo corrimos 10 veces por cada dato(ej 10 veces 20000) y después sacamos el promedio del tiempo de ejecución y lo guardamos en el archivo llamado promedioSerial.csv en la carpeta de Resultados. Para el paralelo tambíen lo corrimos 10 veces por cada dato, pero con 1,10,20 y 40 hilos cada uno 10 veces(ej con 1 hilo 20000 datos 10 veces) y después sacamos el promedio del tiempo de ejecución. Los resultados fueron guardados en un archivo csv por cada número de hilos en la carpeta de Resulatdos. 

En la carpeta Resultados aparte de los csv tenemos una gráfica mostrando el tiempo de ejecución del codigo serial comparado con el tiempo de ejecución en paralelo y otra gráfica midiendo los speedups alcanzados por cada número de hilos diferentes que probamos. 

En conclusión vemos que alcanzamos un buen speedup cuando ya empezamos a correr con 10 hilos y 20 hilos, cosa que esperabamos obtener porque sabíamos que con 1 hilo y 40 hilos ibamos a tener overhead. 


