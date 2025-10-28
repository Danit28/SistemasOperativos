#include <iostream> 
#include <stdio.h>
#include <cmath>

using namespace std;

int main() {
    int filas = 5;
    int columnas = 4;

    // Creamos primeramente las filas con new
    int** matriz = new int*[filas];

    //Luego creamos las columnas para cada fila
    for (int i = 0; i < filas; ++i) {
        matriz[i] = new int[columnas];
    }

    // Llenamos la matriz con algunos valores
    for (int i = 0; i < filas; ++i) {
        for (int j = 0; j < columnas; ++j) {
            matriz[i][j] = pow(i,2) + pow(j,2);
        }
    }

    // Imprimimos la matriz 
    cout << "Contenido de la matriz:" << endl;
    for (int i = 0; i < filas; ++i) {
        for (int j = 0; j < columnas; ++j) {
            cout << matriz[i][j] << " ";
        }
        cout << endl;
    }


    //Liberamos la matriz 
    cout << "\nLiberando la memoria..." << endl;

    for (int i = 0; i < filas; ++i) {
        delete[] matriz[i]; 
    }

    delete[] matriz; 

}