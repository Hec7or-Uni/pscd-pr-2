# include <iostream>
# include <string>
# include <fstream>
# include <thread>
# include <stdlib.h>
# include <atomic>

using namespace std;

// VARIABLES CTEs
const int N = 512;
const int N_BUSC = 8;               // Num de procesos buscadores

typedef int VectInt[N];             // "VectInt" es un "alias" para vector de int de dim. N
bool cargado            = false;    // Indicador de que el proceso coordinador ha cargado el vector
bool he_acabado[N_BUSC] = {false};  // Indicador de que los procesos buscadores han acabado

void wait(thread b[]) {
    for(int i = 0; i < N_BUSC; i++) {
        while(!he_acabado[i]);
        b[i].join();
     }
}

//Pre: 0 <= i <= d <= N-1
//Post: maxVeces sera el mayor result encontrado hasta el momento
//      indMin e indMax sera el intervalo en donde se encontro value maxVeces.
void search(const VectInt v,const int i, const int d,const int value, int& maxVeces, int& indMin, int& indMax, atomic_flag& tas) {
    while(!cargado);
    int result = 0;
    for (int iter = i; iter <= d; iter++) {     // Busqueda
        if (v[iter] == value) {
            result++;
            if (result > maxVeces) {
                //Protocolo de entrada
                while(tas.test_and_set()) {}
                //SC
                if(result > maxVeces) {
                    maxVeces = result;
                    indMin = i;
                    indMax = d;
                }
                //Protocolo de salida
                tas.clear();
            }
        }
    }
    // Informo al coordinador que he terminado --> he_acabdo[i] = true
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
        cerr << "ERROR >> No se pudo abrir el fichero: " + nombreFichero + "\n";
        exit(1);
    }
}

void coordinator(const string nombreFichero, VectInt v, int& n, thread b[], int& maxVeces, int& indMin, int& indMax) {
    load(nombreFichero, v, n);
    cargado = true;
    wait(b);
    cout << "maxVeces: " + to_string(maxVeces) + "\n";
    cout << "indMin: " + to_string(indMin) + "\n";
    cout << "indMax: " + to_string(indMax) + "\n";
}

int main() { 
    VectInt v;
    int value;
    string nombreFichero = "fichero_datos.txt";
    int maxVeces = 0;
    int indMin   = 0;
    int indMax   = 0;
    atomic_flag tas = ATOMIC_FLAG_INIT;  

    cout << "\nElige el valor a buscar en el " + nombreFichero + ": ";
    cin  >> value;

    int n = 0;  // iterador para los datos a guardar
    thread P[N_BUSC];
    thread coord = thread(coordinator, nombreFichero, v, ref(n),P, ref(maxVeces), ref(indMin), ref(indMax));

    int i;
 	if(N % 2 == 0) {
		for(i = 0; i < N_BUSC; i++)
		    P[i] = thread(search, v, (N/N_BUSC) * i, (N/N_BUSC) * (i+1) - 1, value, ref(maxVeces), ref(indMin), ref(indMax), ref(tas));
	} else {
		for(i = 0; i < N_BUSC - 1; i++)
			P[i] = thread(search, v, (N/N_BUSC) * i, (N/N_BUSC) * (i+1) - 1, value, ref(maxVeces), ref(indMin), ref(indMax), ref(tas));
		P[i] = thread(search, v, (N/N_BUSC) * i, (N/N_BUSC) * (i+1), value, ref(maxVeces), ref(indMin), ref(indMax), ref(tas));
	}
    coord.join();
    
    return 0;
}