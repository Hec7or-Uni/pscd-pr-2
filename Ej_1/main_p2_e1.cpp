# include <iostream>
# include <string>
# include <fstream>
# include <thread>
# include <stdlib.h>

using namespace std;

// VARIABLES CTEs
const int N = 512;
const int N_BUSC = 8;               // Num de procesos buscadores

typedef int VectInt[N];             // "VectInt" es un "alias" para vector de int de dim. N
bool cargado            = false;    // Indicador de que el proceso coordinador ha cargado el vector
bool he_acabado[N_BUSC] = {false};  // Indicador de que los procesos buscadores han acabado
int  resul[N_BUSC]      = {0};      // Vector donde guardaremos los resultados de cada proceso buscador

void wait(thread b[]) {
    for(int i = 0; i < N_BUSC; i++) {
        while(!he_acabado[i]);
        b[i].join();
     }
}

// Pre: 0 <= i <= d <= N-1
// Post: result será el número de veces que value aparece entre las componentes i y d de v
void search(const VectInt v,const int i, const int d,const int value, int& result) {
    while(!cargado);
    for (int iter = i; iter <= d; iter++)   // Busqueda
        if (v[iter] == value) result++;
    int iD = i;
    int decod = N/N_BUSC;                   
    iD = iD/decod;
    he_acabado[iD] = true;
}

//Pre: -
//Post: Almacena los datos de un fichero <nombreFichero> en un vector <v>
void load(const string nombreFichero, VectInt v, int& n) {
    ifstream f(nombreFichero);
    if (f.is_open()) {
        f >> v[n];
        n+=1;
        while (!f.eof()) {
            f >> v[n];
            n++;
        }
        f.close();
    } else {
        cerr << "ERROR >> No se pudo abrir el fichero" + nombreFichero + "\n";
        exit(1);
    }
}

void coordinator(const string nombreFichero, VectInt v, int& n, thread b[]) {
    load(nombreFichero, v, n);
    cargado = true;
    wait(b);
    int resulTotal = 0;
    for(int i = 0; i < N_BUSC; i++) // Procesado de resultados
        resulTotal += resul[i];
    cout << "Resultado: " +  to_string(resulTotal) + "\n";
}

int main() { 
    VectInt v;
    int value;
    string nombreFichero = "fichero_datos.txt";

    cout << "\nElige el valor a buscar en el " + nombreFichero + ": ";
    cin  >> value;

    int n = 0;  // iterador para los datos a guardar
    thread P[N_BUSC];
    thread coord = thread(coordinator, nombreFichero, v, ref(n),P);

    int i;
 	if (N % 2 == 0) {
		for(i = 0; i < N_BUSC; i++)
		    P[i] = thread(search, v, (N/N_BUSC) * i, (N/N_BUSC) * (i+1) - 1, value, ref(resul[i]));
	} else {
		for(i = 0; i < N_BUSC - 1; i++)
			P[i] = thread(search, v, (N/N_BUSC) * i, (N/N_BUSC) * (i+1) - 1, value, ref(resul[i]));
		P[i] = thread(search, v, (N/N_BUSC) * i, (N/N_BUSC) * (i+1), value, ref(resul[i]));
	}
    coord.join();

    return 0;
}