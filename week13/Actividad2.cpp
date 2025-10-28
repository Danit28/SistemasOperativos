#include <iostream> 
#include <stdio.h>

using namespace std;

int main() {
    // Declarar la variable original
    int variable = 69;

    cout << "Valor de la variable: " << variable << endl;

    // creamos un puntero que apunta a la variable anteriormente creada
    int* puntero = &variable;
    
    // Modificamos el de la variable con el puntero
    *puntero = 100;

    cout << "Nuevo valor de la variable: " << variable << endl;
    
    // Creamos una referencia a nuestra variable original
    int& referencia = variable;

    // Modificamos el valor usando la referencia.
    referencia = 500;

    cout << "Nuevo valor de la variable: " << variable << endl;
    cout << "Direccion de  la variable: " << &variable << endl;
    cout << "Direccion del puntero :  " << &puntero << endl;
    cout << "Direccion de la referencia:  " << &referencia << endl;

    return 0;
}