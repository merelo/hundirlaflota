#ifndef FUNCCLIENTE_H
#define FUNCCLIENTE_H

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>


//Constantes usadas en las funciones
#define LONG_LIN 10
#define LONG_COL 10

//Funciones
void crearTablero(char *tab);
void mete_datos(char *tablero);
int compruebaBarco(char *tablero,char *cad,int lBarco,char barco);
int ordenados(char **coordenada[5], int lBarco);
void rellenar_mapa(char *tablero,int x,int y, char barco);
int posiciones_ocupadas(char *tablero,char **coordenada[5],int lBarco);
void imprimeTablero(char *tab);
int error_coord(char *cad);
void imprime2tablero(char *mPropio, char *mEnemigo);

#endif
