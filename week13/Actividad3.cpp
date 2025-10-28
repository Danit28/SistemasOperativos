#include <iostream> 
#include <stdio.h>

using namespace std;

int main() {
    //Declarar un array de números enteros
    int numeros[9] = {1, 1, 2, 3, 5, 8, 13, 21, 34};

    cout << "Valores del Array" << endl;
    // Imprimimos el array usando un bucle normal
    for (int i = 0; i < 9; ++i) {
        cout << "numeros[" << i << "] = " << numeros[i] << endl;
    }
    
    // Declaramos un puntero a entero (int*).
    // Lo inicializamos para que apunte al inicio del array.
    int* ptr = numeros;

    // Modificar elementos del array usando el puntero
    *(ptr + 1) = 2;
    *(ptr + 2) = 3;
    *(ptr + 3) = 5;
    *(ptr + 4) = 8;
    *(ptr + 5) = 13;
    *(ptr + 6) = 21;
    *(ptr + 7) = 34;
    *(ptr + 8) = 55;


    cout << "Nuevos valores del array" << endl;
    for (int i = 0; i < 9; ++i) {
        cout << "Posicion " << i << ":  " << "Valor:" << numeros[i];
        cout << "    Direccion: " << &numeros[i] << endl;
    }

    cout << "Direccion guardada en el puntero: " << ptr << endl;
    cout << "Direccion del propio puntero : " << &ptr << endl;

    return 0;
}