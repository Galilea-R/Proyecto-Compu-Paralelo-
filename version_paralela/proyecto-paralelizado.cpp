#include <omp.h>
#include <iostream>
#include <string>
#include <fstream>
#include <cmath>
#include <vector>
#include <sstream>  // Para usar stringstream y dividir por comas
#include <chrono>

using namespace std;
using namespace std::chrono;

// Carga el archivo CSV con los puntos
void load_CSV(string file_name, float** points, long long int size) {
    ifstream in(file_name);  // Abrir archivo CSV para lectura
    if (!in) {
        cerr << "Couldn't read file: " << file_name << "\n";
        return;
    }
    
    long long int point_number = 0;
    string line;

    while (getline(in, line) && point_number < size) {
        stringstream ss(line);
        string x_str, y_str;
        
        // Leer las coordenadas separadas por comas
        if (getline(ss, x_str, ',') && getline(ss, y_str, ',')) {
            points[point_number][0] = stof(x_str);  // Convertir string a float para X
            points[point_number][1] = stof(y_str);  // Convertir string a float para Y
            points[point_number][2] = 0;  // Inicializar la tercera columna (ID del cluster)
            point_number++;
        }
    }
    in.close();  // Cerrar archivo al terminar
}

// Función para guardar el archivo CSV con los resultados
void save_to_csv(string file_name, float** points, long long int size) {
    ofstream out(file_name);  // Abrir archivo CSV para escribir
    if (!out) {
        cerr << "Couldn't write to file: " << file_name << "\n";
        return;
    }
    for (long long int i = 0; i < size; i++) {
        // Escribir las coordenadas originales x, y, y el cluster ID (o ruido)
        out << points[i][0] << "," << points[i][1] << "," << points[i][2] << "\n";
    }
    out.close();  // Cerrar archivo al terminar
}

// DBSCAN functions
vector<int> region_query(float** points, long long int point_idx, float epsilon, long long int size) {
    vector<int> neighbors;
    #pragma omp parallel
    {
        vector<int> private_neighbors;
        #pragma omp for nowait schedule(dynamic, 5)
        for (long long int i = 0; i < size; i++) {
            float distance = sqrt(pow(points[point_idx][0] - points[i][0], 2) + pow(points[point_idx][1] - points[i][1], 2));
            if (distance <= epsilon) {
                private_neighbors.push_back(i);
            }
        }
        #pragma omp critical
        neighbors.insert(neighbors.end(), private_neighbors.begin(), private_neighbors.end());
    }
    return neighbors;
}

// Función para expandir un clúster de manera concurrente
void expand_cluster(float** points, long long int point_idx, vector<int>& neighbors, int cluster_id, float epsilon, int min_samples, long long int size) {
    points[point_idx][2] = cluster_id;  // Etiquetamos como parte del clúster

    for (size_t i = 0; i < neighbors.size(); i++) {
        int neighbor_idx = neighbors[i];
        if (points[neighbor_idx][2] == 0) {  // Si no ha sido visitado
            points[neighbor_idx][2] = cluster_id;  // Etiquetamos como parte del clúster
            vector<int> new_neighbors = region_query(points, neighbor_idx, epsilon, size);
            if (new_neighbors.size() >= min_samples) {
                #pragma omp critical
                neighbors.insert(neighbors.end(), new_neighbors.begin(), new_neighbors.end());  // Expandimos el clúster
            }
        }
    }
}

// Versión paralelizada de DBSCAN
void dbscan(float** points, float epsilon, int min_samples, long long int size) {
    int cluster_id = 0;  // Comenzamos en 1 ya que 0 es para ruido
    
    for (long long int i = 0; i < size; i++) {
        if (points[i][2] == 0) {  // Si no ha sido visitado
            vector<int> neighbors = region_query(points, i, epsilon, size);
            if (neighbors.size() < min_samples) {
                points[i][2] = -1;  // Marcamos como ruido
            } else {
                #pragma omp atomic
                cluster_id++;  // Incrementamos el ID del clúster de manera atómica
                points[i][2] = cluster_id;
                expand_cluster(points, i, neighbors, cluster_id, epsilon, min_samples, size);
            }
        }
    }
}

int main() {
    // Ajusta el numero de threads para que coincida con todos los hilos logicos posibles del equipo
    omp_set_num_threads(omp_get_max_threads());
    long long int size = 20000;  // Cambia esta variable al tamaño de los datos que estés usando
    float epsilon = 0.03;
    int min_samples = 10;

    // Construcción dinámica de los nombres de los archivos
    string input_file = to_string(size) + "_data.csv";     // Ejemplo: "1000_data.csv"
    string output_file = to_string(size) + "_resultsP2.csv"; // Ejemplo: "1000_results.csv"

    // Inicialización de la matriz para almacenar los puntos
    float** points = new float*[size];
    for (long long int i = 0; i < size; i++) {
        points[i] = new float[3];  // Dos coordenadas (x, y) y una columna para el cluster ID
    }

    auto start = high_resolution_clock::now();

    load_CSV(input_file, points, size);  // Cargar los puntos desde el CSV

    dbscan(points, epsilon, min_samples, size);  // Ejecutar DBSCAN

    save_to_csv(output_file, points, size);  // Guardar los resultados

    auto end = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end - start);

    // Mostrar el tiempo de ejecución en milisegundos
    std::cout << "Tiempo de ejecución (Paralelo): " << duration.count() << " milisegundos\n";

    // Liberar memoria
    for (long long int i = 0; i < size; i++) {
        delete[] points[i];
    }
    delete[] points;

    return 0;
}