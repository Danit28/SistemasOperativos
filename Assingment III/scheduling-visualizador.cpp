#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <string>
#include <limits>
#include <iomanip>
#include <fstream> // Necesario para escribir en archivos

// --- Constantes del Disco ---
const int MAX_CYLINDER = 4999;
const int MIN_CYLINDER = 0;
const int REQUEST_COUNT = 1000;

//Estructura para almacenar los resultados de un algoritmo
struct SchedulingResult {
    int movement = 0;
    std::vector<int> path;
};

// Clase con todos los algoritmos.

class DiskScheduler {
private:
    //Calcula el movimiento total a partir de una ruta dada
    int calculateMovementFromPath(const std::vector<int>& path) const {
        int movement = 0;
        for (size_t i = 1; i < path.size(); ++i) {
            movement += std::abs(path[i] - path[i - 1]);
        }
        return movement;
    }

public:
    DiskScheduler() {}

    //Algoritmo FCFS
    SchedulingResult calculateFCFS(int initialPos, std::vector<int> requests) const {
        SchedulingResult result;
        result.path.push_back(initialPos);
        result.path.insert(result.path.end(), requests.begin(), requests.end());
        result.movement = calculateMovementFromPath(result.path);
        return result;
    }

    // Algoritmo SCAN
    SchedulingResult calculateSCAN(int initialPos, std::vector<int> requests) const {
        SchedulingResult result;
        result.path.push_back(initialPos);
        
        std::sort(requests.begin(), requests.end());
        auto it = std::lower_bound(requests.begin(), requests.end(), initialPos);
        int splitIndex = std::distance(requests.begin(), it);

        // 1. Moverse "arriba"
        for (int i = splitIndex; i < requests.size(); ++i) {
            result.path.push_back(requests[i]);
        }

        // Si hay solicitudes "abajo"
        if (splitIndex > 0) {
            result.path.push_back(MAX_CYLINDER); // Ir al final
            // 2. Moverse "abajo"
            for (int i = splitIndex - 1; i >= 0; --i) {
                result.path.push_back(requests[i]);
            }
        }
        
        result.movement = calculateMovementFromPath(result.path);
        return result;
    }

    // Algoritmo C-SCAN
    SchedulingResult calculateCSCAN(int initialPos, std::vector<int> requests) const {
        SchedulingResult result;
        result.path.push_back(initialPos);
        
        std::sort(requests.begin(), requests.end());
        auto it = std::lower_bound(requests.begin(), requests.end(), initialPos);
        int splitIndex = std::distance(requests.begin(), it);

        // 1. Moverse "arriba"
        for (int i = splitIndex; i < requests.size(); ++i) {
            result.path.push_back(requests[i]);
        }

        // Si hay solicitudes "abajo"
        if (splitIndex > 0) {
            result.path.push_back(MAX_CYLINDER); // Ir al final
            result.path.push_back(MIN_CYLINDER); // Saltar al inicio
            
            // 2. Continuar "arriba" desde el inicio
            for (int i = 0; i < splitIndex; ++i) {
                result.path.push_back(requests[i]);
            }
        }

        result.movement = calculateMovementFromPath(result.path);
        return result;
    }
};

//Guarda un vector en un archivo de texto
void savePathToFile(const std::string& filename, const std::vector<int>& path) {
    std::ofstream outFile(filename);
    if (!outFile.is_open()) {
        std::cerr << "Error: No se pudo abrir el archivo para escribir: " << filename << std::endl;
        return;
    }
    for (size_t i = 0; i < path.size(); ++i) {
        outFile << path[i] << (i == path.size() - 1 ? "" : " ");
    }
    outFile.close();
}


int main() {
    int initialHeadPos;

    // 1. Leer la posición inicial desde std::cin
    std::cin >> initialHeadPos;
    
    // 2. Generar solicitudes
    std::srand(static_cast<unsigned int>(std::time(nullptr)));
    std::vector<int> requests;
    for (int i = 0; i < REQUEST_COUNT; ++i) {
        requests.push_back(std::rand() % (MAX_CYLINDER + 1));
    }

    // 3. Crear instancia y ejecutar
    DiskScheduler scheduler;

    SchedulingResult fcfs = scheduler.calculateFCFS(initialHeadPos, requests);
    SchedulingResult scan = scheduler.calculateSCAN(initialHeadPos, requests);
    SchedulingResult cscan = scheduler.calculateCSCAN(initialHeadPos, requests);

    // 4. Imprimir resultados en formato de texto plano para Python

    std::cout << "\nSimulador de Planificacion de Disco" << std::endl;
    std::cout << "------------------------------------" << std::endl;
    std::cout << "Cilindros: " << MIN_CYLINDER << " a " << MAX_CYLINDER << std::endl;
    std::cout << "Solicitudes generadas: " << REQUEST_COUNT << std::endl;
    std::cout << "Posicion inicial del cabezal: " << initialHeadPos << std::endl;
    std::cout << "------------------------------------" << std::endl;

    // FCFS
    std::cout << "FCFS   | Movimiento Total: " << fcfs.movement << std::endl;

    // SCAN
    std::cout << "SCAN   | Movimiento Total: " << scan.movement << std::endl;

    // C-SCAN
    std::cout << "C-SCAN | Movimiento Total: " << cscan.movement << std::endl;
    

    // Rutas completas (a archivos .txt)
    savePathToFile("fcfs_path.txt", fcfs.path);
    savePathToFile("scan_path.txt", scan.path);
    savePathToFile("cscan_path.txt", cscan.path);

    return 0;
}