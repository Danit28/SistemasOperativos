#include <iostream> 
#include <stdio.h>

using namespace std;

int main() {

    // Variable entera
    int entero = 33;

    // Direccion de memoria de la variable entera
    cout << "Direccion de la variable entera: " << &entero << endl;

    //Creamos un puntero que apunta a la variable entera
    int* puntero = &entero;

    // Modificamos el valor de la variable a través del puntero
    *puntero = 420;

    // Presentamos el valor y la direccion de la variable
    cout << "Nuevo valor de la variable: " << entero << endl;
    cout << "Direccion de memoria de la variable: " << &entero << endl;

    return 0;
}