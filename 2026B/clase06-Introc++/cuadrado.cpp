// cuadrado.cpp - Programa con una funcion y entrada del usuario
// Compilar:  g++ cuadrado.cpp -o cuadrado
// Ejecutar:  ./cuadrado

#include <iostream>
using namespace std;

int cuadrado(int n) {
    return n * n;
}

int main() {
    int n;
    cout << "Escribe un numero entero: ";
    cin >> n;
    cout << "Su cuadrado es: " << cuadrado(n) << endl;
    return 0;
}
