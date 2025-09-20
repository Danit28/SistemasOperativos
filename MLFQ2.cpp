#include <iostream>
#include <fstream>
#include <queue>
#include <vector>
#include <sstream>
#include <algorithm>
#include <string>
using namespace std;

class proceso {
public:
    string etiqueta;  // Etiqueta del proceso
    int arrival_time; // Arrival Time
    int burst_time; // Burst Time
    int priority;   // Prioridad del proceso
    int ComTim;     // Completion Time
    int TuArTi;     // Turnaround Time
    int WaiTim;     // Waiting Time
    int ResTim;     // Response Time
    int tiempoRestante; // Tiempo restante para que termine un proceso
    int nivel;          // Nivel de la cola en MLFQ
    bool primeraVez;    // Indica si es la primera vez que se ejecuta el proceso
    int cola;           // Cola a la que pertenece el proceso

    // Constructor de la clase proceso
    proceso(string et, int BT, int AT, int q, int p)
        : etiqueta(et), burst_time(BT), arrival_time(AT), priority(p),
          ComTim(0), TuArTi(0), WaiTim(0), ResTim(0), cola(q),
          tiempoRestante(BT), nivel(0), primeraVez(true) {}
};

class Cola {
public:
    queue<proceso> procesos; // Cola de procesos
    int quantum;         // Quantum de tiempo para la cola
    string tipo;       // Tipo de la cola (RR, FCFS, etc.)
    int nivel;        // Nivel de la cola

    // Constructor de la clase Cola
    Cola(int lvl, string pol, int tq = 0)
        : nivel(lvl), tipo(pol), quantum(tq) {}

    // Método para añadir un proceso a la cola
    void add_process(const proceso& p) {
        procesos.push(p);
    }

    // Método para verificar si la cola está vacía
    bool is_empty() const {
        return procesos.empty();
    }
};

class Scheduler {
public:
    // Vector de tipos de planificacion (colas)
    vector<Cola> queues;

    // Constructor de la clase Scheduler
    Scheduler() {};

    // Método para añadir una nueva cola al scheduler
    void add_queue(int level, const string& policy, int time_quantum) {
        queues.emplace_back(level, policy, time_quantum);
    }
};

// Funcion para imprimir los resultados del WT, CT, TAT, RT
void printResultados(vector<proceso>& resp) {

    string filename = "salida_MLFQ.txt";
    // Ordenar por etiqueta para salida ordenada
    sort(resp.begin(), resp.end(),
        [](proceso &a, proceso &b) {
            return a.etiqueta < b.etiqueta;
        });

    int proc = resp.size();
    vector<double> avgs(4, 0);

    // Imprimir resultados
    for (auto& p : resp) {
        avgs[0] += p.ComTim;
        avgs[1] += p.TuArTi;
        avgs[2] += p.WaiTim;
        avgs[3] += p.ResTim;
    }
    for (int i = 0; i < 4; i++) avgs[i] /= proc;

    ofstream out(filename);
    if (!out.is_open()) {
        cerr << "Error al abrir archivo de salida\n";
        return;
    }
   
    // Guardar resultados en archivo

    out << "# archivo: " << filename << "\n";
    out << "# etiqueta; BT; AT; Q; Pr; WT; CT; RT; TAT\n";

    for (auto& p : resp) {
        out << p.etiqueta << ";"
            << p.burst_time << ";"
            << p.arrival_time << ";"
            << p.cola << ";"   
            << p.priority << ";"
            << p.WaiTim << ";"
            << p.ComTim << ";"
            << p.ResTim << ";"
            << p.TuArTi << "\n";
    }

    out << "WT=" << avgs[2]
        << "; CT=" << avgs[0]
        << "; RT=" << avgs[3]
        << "; TAT=" << avgs[1] << ";\n";

    out.close();
}

void MLFQ(vector<proceso> &procesos, Scheduler& scheduler) {
    int tiempo = 0, completados = 0;
    int n = procesos.size();
    vector<proceso> resp;

    // Ordenar procesos por tiempo de llegada
    sort(procesos.begin(), procesos.end(),
         [](proceso &a, proceso &b) {
             return a.arrival_time < b.arrival_time;
         });

    int idx = 0; // procesos que van llegando

    // Se ejecuta hasta que todos los procesos se completen
    while (completados < n) {
        // Encolar los procesos que llegan
        while (idx < n && procesos[idx].arrival_time <= tiempo) {
            scheduler.queues[0].add_process(procesos[idx]);
            idx++;
        }

        bool ejecutado = false;
        // Revisar las colas en orden de prioridad
        for (int i = 0; i < scheduler.queues.size(); i++) {
            // La cola se ejecuta hasta que se vacíe o llegue un proceso de mayor prioridad
            while (!scheduler.queues[i].is_empty()) {
                proceso current = scheduler.queues[i].procesos.front();
                scheduler.queues[i].procesos.pop();

                // Si se llega un proceso por primera vez, registrar su tiempo de respuesta
                if (current.primeraVez) {
                    current.ResTim = tiempo;
                    current.primeraVez = false;
                }

                //Calcular el quantum a usar ya que es round robin
                int tiempoPedazo = min(current.tiempoRestante, scheduler.queues[i].quantum);
                tiempo += tiempoPedazo;
                current.tiempoRestante -= tiempoPedazo;

                // Encolar nuevos procesos que llegaron en este tiempo
                while (idx < n && procesos[idx].arrival_time <= tiempo) {
                    scheduler.queues[0].add_process(procesos[idx]);
                    idx++;
                }

                // Si el proceso se completa calcular métricas y añadir a resultados
                if (current.tiempoRestante == 0) {
                    current.ComTim = tiempo;
                    current.TuArTi = current.ComTim - current.arrival_time;
                    current.WaiTim = current.TuArTi - current.burst_time;
                    resp.push_back(current);
                    completados++;
                } else {
                    // Si no se completa, bajar de nivel si es posible y reencolar
                    if (current.nivel < scheduler.queues.size() - 1) {
                        current.nivel++;
                    }
                    scheduler.queues[current.nivel].add_process(current);
                }

                ejecutado = true;
                // Si llego un proceso a la cola 0, interrumpir y atenderlo
                if (!scheduler.queues[0].is_empty() && i != 0) {
                    break;
                }
                
            }
        }

        // Si no se ejecutó ningún proceso, avanzar el tiempo
        if (!ejecutado) tiempo++; // Si no hay nada, avanza el tiempo
    }

    // Imprimir resultados
    printResultados(resp);
}

int main(int argc, char* argv[]) {
    // Verificar argumentos
    if (argc < 2) {
        cerr << "Uso: " << argv[0] << " <archivo_entrada>" << endl;
        return 1;
    }

    ifstream archivo(argv[1]);
    if (!archivo.is_open()) {
        cerr << "Error: no se pudo abrir " << argv[1] << endl;
        return 1;
    }

    // Leer procesos del archivo y guardarlos todos en un vector
    vector<proceso> procesos;
    string linea;
    while (getline(archivo, linea)) {
        if (linea.empty() || linea[0] == '#') continue;
        stringstream ss(linea);
        string campo;

        getline(ss, campo, ';'); string et = campo;
        getline(ss, campo, ';'); int BT = stoi(campo);
        getline(ss, campo, ';'); int AT = stoi(campo);
        getline(ss, campo, ';'); int q = stoi(campo);
        getline(ss, campo, ';'); int p = stoi(campo);

        procesos.emplace_back(et, BT, AT, q, p);
    }

    // Crear el scheduler y añadir las colas con sus políticas y quantums
    Scheduler scheduler;
    scheduler.add_queue(0, "RR", 3);
    scheduler.add_queue(1, "RR", 5);
    scheduler.add_queue(2, "RR", 6);
    scheduler.add_queue(3, "RR", 20);

    // Ejecutar el algoritmo MLFQ
    MLFQ(procesos, scheduler);
    return 0;
}
