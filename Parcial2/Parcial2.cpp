#include <iostream>
#include <vector>
#include <string>
#include <iomanip>
#include <limits>
#include <list>
#include <sstream>
#include <map>

using namespace std;

// ============================================================================
// CLASE: MemoryBlock
// Estructura para representar un bloque de memoria individual
// ============================================================================
class MemoryBlock {
public:
    int id;              // Identificador único del bloque
    int size;            // Tamaño del bloque en unidades de memoria
    bool is_free;        // true = bloque libre, false = bloque ocupado
    string process_id;   // ID del proceso que ocupa el bloque (vacío si está libre)

    // Constructor: inicializa un bloque de memoria
    MemoryBlock(int i, int s, bool free = true, const string& p_id = "")
        : id(i), size(s), is_free(free), process_id(p_id) {}
};


// CLASE: MemoryManager
// Gestiona toda la lógica de asignación y liberación de memoria

class MemoryManager {
private:
    // --- Estructuras de datos para almacenar la memoria ---
    list<MemoryBlock> dynamic_memory_;    // Lista enlazada para particionamiento dinámico
    vector<MemoryBlock> fixed_memory_;    // Vector para particionamiento fijo
    
    // --- Variables de configuración ---
    map<string, int> process_sizes_;      // Mapa: process_id -> tamaño real del proceso
    string partition_scheme_str;          // "DYNAMIC" o "FIXED"
    int total_memory_size_;               // Tamaño total de la memoria disponible
    int next_block_id_;                   // ID para el siguiente bloque (usado en dinámico)
    int allocation_algorithm_;            // 1: First Fit, 2: Best Fit, 3: Worst Fit

    // ========================================================================
    // MÉTODO: mergeFreeBlocks
    // Fusiona bloques de memoria libres contiguos en particionamiento dinámico
    // Esto ayuda a reducir la fragmentación externa
    // ========================================================================
    void mergeFreeBlocks() {
        // Si hay menos de 2 bloques, no hay nada que fusionar
        if (dynamic_memory_.size() < 2) return;
        
        // Iterar sobre la lista de bloques
        list<MemoryBlock>::iterator it = dynamic_memory_.begin();
        while (it != dynamic_memory_.end() && next(it) != dynamic_memory_.end()) {
            list<MemoryBlock>::iterator next_it = next(it);
            
            // Si el bloque actual y el siguiente están libres, fusionarlos
            if (it->is_free && next_it->is_free) {
                it->size += next_it->size;  // Sumar el tamaño del siguiente al actual
                dynamic_memory_.erase(next_it);  // Eliminar el siguiente bloque
            } else {
                ++it;  // Avanzar al siguiente bloque
            }
        }
    }

    // ========================================================================
    // MÉTODO: printShellFormat
    // Muestra el mapa de memoria en formato visual
    // Ejemplo: [Libre:50][P1:30][P2:20]
    // ========================================================================
    void printShellFormat() {
        cout << "# Mapa de Memoria" << endl;
        
        if (partition_scheme_str == "DYNAMIC") {
            // Imprimir bloques de memoria dinámica
            for (const MemoryBlock& block : dynamic_memory_) {
                if (block.size > 0) {
                    if (block.is_free)
                        cout << "[Libre:" << block.size << "]";
                    else
                        cout << "[" + block.process_id + ":" << process_sizes_[block.process_id] << "]";
                }
            }
        } else { // FIXED
            // Imprimir particiones fijas
            for (const MemoryBlock& block : fixed_memory_) {
                if (block.is_free)
                    cout << "[Libre:" << block.size << "]";
                else
                    cout << "[" + block.process_id + ":" << process_sizes_[block.process_id] << "]";
            }
        }
        cout << endl;
    }

    // ========================================================================
    // MÉTODO: calculateAndShowFragmentation
    // Calcula y muestra la fragmentación interna y externa
    // ========================================================================
    void calculateAndShowFragmentation() {
        cout << "\n# Ficha de Fragmentacion" << endl;
        
        if (partition_scheme_str == "DYNAMIC") {
            // En particionamiento dinámico, la fragmentación interna es 0
            // porque los bloques se ajustan al tamaño del proceso
            int total_free_memory = 0;
            
            // Sumar toda la memoria libre (fragmentación externa)
            for (const MemoryBlock& block : dynamic_memory_) {
                if (block.is_free) {
                    total_free_memory += block.size;
                }
            }
            
            int external_fragmentation = total_free_memory;
            cout << "  - Fragmentacion Interna: 0" << endl;
            cout << "  - Fragmentacion Externa:" << external_fragmentation << endl;
            
        } else { // FIXED
            int total_internal_fragmentation = 0;
            int total_free_memory_fix = 0;
            
            // Calcular fragmentación interna y externa
            for (const MemoryBlock& partition : fixed_memory_) {
                // Fragmentación interna: espacio desperdiciado en particiones ocupadas
                if (!partition.is_free && process_sizes_.count(partition.process_id)) {
                    total_internal_fragmentation += (partition.size - process_sizes_.at(partition.process_id));
                }
                // Fragmentación externa: memoria libre total
                if (partition.is_free) {
                    total_free_memory_fix += partition.size;
                }
            }
            
            int external_fragmentation = total_free_memory_fix;
            cout << "  - Fragmentacion Interna Total: " << total_internal_fragmentation  << endl;
            cout << "  - Fragmentacion Externa: " << external_fragmentation << endl;
        }
    }

    // ========================================================================
    // MÉTODO: allocate
    // Punto de entrada para asignar memoria a un proceso
    // Delega a allocateDynamic o allocateFixed según el esquema
    // ========================================================================
    void allocate(const string& process_id, int process_size) {
        if (partition_scheme_str == "DYNAMIC") {
            allocateDynamic(process_id, process_size);
        } else {
            allocateFixed(process_id, process_size);
        }
    }

    // ========================================================================
    // MÉTODO: liberate
    // Punto de entrada para liberar memoria de un proceso
    // Delega a liberateDynamic o liberateFixed según el esquema
    // ========================================================================
    void liberate(const string& process_id) {
        if (partition_scheme_str == "DYNAMIC") {
            liberateDynamic(process_id);
        } else {
            liberateFixed(process_id);
        }
    }

    // ========================================================================
    // MÉTODO: allocateDynamic
    // Asigna memoria a un proceso en particionamiento dinámico
    // Implementa los algoritmos First Fit, Best Fit y Worst Fit
    // ========================================================================
    void allocateDynamic(const string& process_id, int process_size) {
        list<MemoryBlock>::iterator block_to_use = dynamic_memory_.end();
        
        // --- FIRST FIT: Usar el primer bloque que sea suficientemente grande ---
        if (allocation_algorithm_ == 1) {
            for (auto it = dynamic_memory_.begin(); it != dynamic_memory_.end(); ++it) {
                if (it->is_free && it->size >= process_size) {
                    block_to_use = it;
                    break;  // Usar el primer bloque encontrado
                }
            }
        } 
        // --- BEST FIT: Usar el bloque más pequeño que sea suficientemente grande ---
        else if (allocation_algorithm_ == 2) {
            int min_diff = numeric_limits<int>::max();  // Diferencia mínima inicializada al máximo
            
            for (auto it = dynamic_memory_.begin(); it != dynamic_memory_.end(); ++it) {
                if (it->is_free && it->size >= process_size && (it->size - process_size < min_diff)) {
                    min_diff = it->size - process_size;  // Actualizar diferencia mínima
                    block_to_use = it;
                }
            }
        } 
        // --- WORST FIT: Usar el bloque más grande disponible ---
        else if (allocation_algorithm_ == 3) {
            int max_size = -1;  // Tamaño máximo inicializado a -1
            
            for (auto it = dynamic_memory_.begin(); it != dynamic_memory_.end(); ++it) {
                if (it->is_free && it->size >= process_size && it->size > max_size) {
                    max_size = it->size;  // Actualizar tamaño máximo
                    block_to_use = it;
                }
            }
        }

        // Si se encontró un bloque adecuado, asignar el proceso
        if (block_to_use != dynamic_memory_.end()) {
            process_sizes_[process_id] = process_size;  // Guardar tamaño real del proceso
            
            // Si el bloque es mayor que el proceso, dividirlo
            if (block_to_use->size > process_size) {
                // Crear un nuevo bloque libre con el espacio restante
                MemoryBlock new_free_block(next_block_id_++, block_to_use->size - process_size, true);
                block_to_use->size = process_size;  // Ajustar tamaño del bloque actual
                dynamic_memory_.insert(next(block_to_use), new_free_block);  // Insertar nuevo bloque
            }
            
            // Marcar el bloque como ocupado
            block_to_use->is_free = false;
            block_to_use->process_id = process_id;
            cout << " -> Proceso " << process_id << " asignado." << endl;
        } else {
            // No se encontró un bloque suficientemente grande
            cout << "Error: No hay espacio suficiente para el proceso " << process_id << "." << endl;
        }
    }
    
    // ========================================================================
    // MÉTODO: allocateFixed
    // Asigna memoria a un proceso en particionamiento fijo
    // Implementa los algoritmos First Fit, Best Fit y Worst Fit
    // ========================================================================
    void allocateFixed(const string& process_id, int process_size) {
        int partition_index_to_use = -1;  // Índice de la partición a usar (-1 = no encontrada)

        // --- FIRST FIT: Usar la primera partición que sea suficientemente grande ---
        if (allocation_algorithm_ == 1) {
            for (int i = 0; i < fixed_memory_.size(); ++i) {
                if (fixed_memory_[i].is_free && fixed_memory_[i].size >= process_size) {
                    partition_index_to_use = i;
                    break;  // Usar la primera partición encontrada
                }
            }
        } 
        // --- BEST FIT: Usar la partición más pequeña que sea suficientemente grande ---
        else if (allocation_algorithm_ == 2) {
            int min_diff = numeric_limits<int>::max();  // Diferencia mínima
            
            for (int i = 0; i < fixed_memory_.size(); ++i) {
                if (fixed_memory_[i].is_free && fixed_memory_[i].size >= process_size) {
                    if (fixed_memory_[i].size - process_size < min_diff) {
                        min_diff = fixed_memory_[i].size - process_size;
                        partition_index_to_use = i;
                    }
                }
            }
        } 
        // --- WORST FIT: Usar la partición más grande disponible ---
        else if (allocation_algorithm_ == 3) {
            int max_diff = -1;  // Diferencia máxima
            
            for (int i = 0; i < fixed_memory_.size(); ++i) {
                if (fixed_memory_[i].is_free && fixed_memory_[i].size >= process_size) {
                    if (fixed_memory_[i].size - process_size > max_diff) {
                        max_diff = fixed_memory_[i].size - process_size;
                        partition_index_to_use = i;
                    }
                }
            }
        }
        
        // Si se encontró una partición adecuada, asignar el proceso
        if (partition_index_to_use != -1) {
            fixed_memory_[partition_index_to_use].is_free = false;  // Marcar como ocupada
            fixed_memory_[partition_index_to_use].process_id = process_id;  // Asignar proceso
            process_sizes_[process_id] = process_size;  // Guardar tamaño real para calcular fragmentación
            
            cout << " -> Proceso " << process_id << " asignado a la particion " << partition_index_to_use << endl;
        } else {
            // No se encontró una partición adecuada
            cout << "Error: No hay una particion libre o suficientemente grande para el proceso " << process_id << "." << endl;
        }
    }

    // ========================================================================
    // MÉTODO: liberateDynamic
    // Libera la memoria ocupada por un proceso en particionamiento dinámico
    // ========================================================================
    void liberateDynamic(const string& process_id) {
        bool found = false;
        
        // Buscar y liberar todos los bloques del proceso
        for (auto& block : dynamic_memory_) {
            if (!block.is_free && block.process_id == process_id) {
                block.is_free = true;        // Marcar como libre
                block.process_id = "";       // Limpiar ID del proceso
                process_sizes_.erase(process_id);  // Eliminar del registro de tamaños
                found = true;
            }
        }
        
        // Si se liberó el proceso, fusionar bloques libres contiguos
        if (found)
            mergeFreeBlocks();
        else
            cout << "Error: Proceso " << process_id << " no encontrado." << endl;
    }

    // ========================================================================
    // MÉTODO: liberateFixed
    // Libera la memoria ocupada por un proceso en particionamiento fijo
    // ========================================================================
    void liberateFixed(const string& process_id) {
        // Buscar y liberar la partición del proceso
        for (auto& partition : fixed_memory_) {
            if (!partition.is_free && partition.process_id == process_id) {
                partition.is_free = true;     // Marcar como libre
                partition.process_id = "";    // Limpiar ID del proceso
                process_sizes_.erase(process_id);  // Eliminar del registro de tamaños
                return;  // Salir después de encontrar y liberar
            }
        }
        
        // Si llegamos aquí, el proceso no fue encontrado
        cout << "Error: Proceso " << process_id << " no encontrado." << endl;
    }

public:
    // ========================================================================
    // CONSTRUCTOR
    // Inicializa las variables del gestor de memoria
    // ========================================================================
    MemoryManager() : total_memory_size_(0), next_block_id_(1), allocation_algorithm_(0) {}

    // ========================================================================
    // MÉTODO: run
    // Flujo principal del simulador
    // Configura el sistema y procesa comandos del usuario
    // ========================================================================
    void run() {
        cout << "===== Simulador de Gestion de Memoria =====\n";
        
        // --- PASO 1: Configurar tamaño total de memoria ---
        cout << "Ingrese el tamano total de la memoria (ej. 100): ";
        cin >> total_memory_size_;
        if (total_memory_size_ <= 0) {
            cout << "El tamano debe ser positivo. Saliendo.\n";
            return;
        }

        // --- PASO 2: Elegir esquema de particionamiento ---
        int scheme_choice;
        cout << "\nElija un esquema de particionamiento:\n  1. Fijo\n  2. Dinamico\nOpcion: ";
        cin >> scheme_choice;

        if (scheme_choice == 1) {
            // --- CONFIGURAR PARTICIONAMIENTO FIJO ---
            partition_scheme_str = "FIXED";
            int num_partitions;
            cout << "\n--- Configurando Particionamiento Fijo ---\n";
            cout << "Ingrese el numero de particiones: ";
            cin >> num_partitions;
            
            int current_total_size = 0;
            // Leer el tamaño de cada partición
            for (int i = 0; i < num_partitions; ++i) {
                int p_size;
                cout << "Tamano para particion " << i << ": ";
                cin >> p_size;
                fixed_memory_.emplace_back(i, p_size, true);  // Crear partición libre
                current_total_size += p_size;
            }
            
            // Validar que las particiones no excedan la memoria total
            if(current_total_size > total_memory_size_){
                cout << "Error: La suma de las particiones excede la memoria total. Saliendo.\n";
                return;
            }
        } 
        else if (scheme_choice == 2) {
            // --- CONFIGURAR PARTICIONAMIENTO DINÁMICO ---
            partition_scheme_str = "DYNAMIC";
            cout << "\n--- Configurando Particionamiento Dinamico ---\n";
            // Iniciar con un solo bloque libre del tamaño total de memoria
            dynamic_memory_.emplace_back(0, total_memory_size_, true);
        } 
        else {
            cout << "Opcion no valida. Saliendo.\n";
            return;
        }

        // --- PASO 3: Elegir algoritmo de asignación ---
        cout << "\nElija un algoritmo de asignacion:\n";
        cout << "  1. First Fit\n  2. Best Fit\n  3. Worst Fit\nOpcion: ";
        cin >> allocation_algorithm_;

        // --- PASO 4: Procesar comandos ---
        cout << "\nMemoria inicializada. Ingrese los comandos " << endl;
        cout << "# entrada" << endl;
        
        string line, command, process_id;
        int process_size;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');  // Limpiar buffer

        // Bucle principal de procesamiento de comandos
        while (getline(cin, line)) {
            if (line.empty()) continue;  // Ignorar líneas vacías
            
            stringstream ss(line);
            ss >> command;

            // Comando A: Asignar memoria a un proceso
            if (command == "A" || command == "a") {
                ss >> process_id >> process_size;
                allocate(process_id, process_size);
            } 
            // Comando L: Liberar memoria de un proceso
            else if (command == "L" || command == "l") {
                ss >> process_id;
                liberate(process_id);
            } 
            // Comando M: Mostrar mapa de memoria y fragmentación, luego salir
            else if (command == "M" || command == "m") {
                cout << "\n# salida" << endl;
                printShellFormat();
                calculateAndShowFragmentation();
                break;  // Terminar el programa
            }
        }
    }
};

// ============================================================================
// FUNCIÓN MAIN
// Punto de entrada del programa
// ============================================================================
int main() {
    MemoryManager simulator;  // Crear instancia del simulador
    simulator.run();          // Ejecutar el simulador
    return 0;
}