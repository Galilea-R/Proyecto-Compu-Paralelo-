Este es el primer proyecto de computo pararalelo y en la nube de Sebastian Yamil Castellanos Gómez 198090 y Galilea Resendis González 196811. El proyecto se basa en hacer un algoritmo DBSCAN de manera serial y paralelizada, como parte de las especificaciones se debe de alcanzar un speed up minimo de 1.5 con la versión paralelizada. 

Para la versión serial tenemos como funciones principales a: 
1) region_query: Esta función busca todos los vecinos de un punto específico (point_idx) que están dentro de una distancia (epsilon). Encontrar los vecinos de un punto es fundamental para algoritmo DBSCAN, ya que así va a poder formar clústeres basados en la densidad de puntos cercanos.
2) expand_cluster: Esta función expande un clúster agregando puntos que tienen suficientes vecinos dentro del radio epsilon.
3) dbscan: El algoritmo dbscan encuentra clústeres en un conjunto de datos basándose en la densidad de puntos cercanos

Para la verisón paralalelizada tenemos las mismas funciones que en la serial, solo que en cada función le decidimos agregar una parte paralelizada. Lo hicimos considerando que acciones se podrían maximizar sin que ocurrieran errores o tardara más tiempo que el serial.
Usamos: 

1)#pragma omp atomic: lo usamos para incrementar el contador cluster_id de forma segura. Esto asegura que cada clúster reciba un identificador único, incluso cuando múltiples hilos intenten actualizarlo simultáneamente asegurando evitar una condición de carrera. 

2)#pragma omp critical: lo usamos para proteger el acceso a la estructura de datos neighbors. Esto previene que múltiples hilos modifiquen la lista de vecinos de un punto al mismo tiempo, garantizando la integridad de los datos compartidos.

3)#pragma omp for nowait schedule(dynamic, 5): lo utilizamos para crear equipos de hilos que trabajarán en tareas diferentes, como buscar vecinos o expandir clústeres. Esto nos permite aprovechar la capacidad de procesamiento de múltiples núcleos del procesador para acelerar las tareas computacionalmente intensivas.

Las pruebas realizadas las hicimos con 20000,40000,80000,120000,140000,160000,180000,200000 datos generados de DBSCAN_noise.ipynb. En el código serial lo corrimos 10 veces por cada dato(ej 10 veces 20000) y después sacamos el promedio del tiempo de ejecución y lo guardamos en el archivo llamado promedio.csv en la carpeta de version_serial. Para el paralelo tambíen lo corrimos 10 veces por cada dato, pero con 1,10,20 y 40 hilos cada uno 10 veces(ej con 1 hilo 20000 datos 10 veces) y después sacamos el promedio del tiempo de ejecución. Los resultados fueron guardados en un archivo csv por cada número de hilos en la carpeta de version_paralelo. 
