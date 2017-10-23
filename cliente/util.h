#ifndef UTIL_H
#define UTIL_H

//Constantes
#define TRUE 1
#define FALSE 0
#define EXIT_ERROR 1

//Funciones (ver descripcion en fichero .c)
void error_fatal(char *mens);
int espera_recepcion(int descriptor, int segundos);
int espera_envio(int descriptor, int segundos);
int lee(int s, char *buffer, int longitud, int segundos);
int escribe(int s, const char *buffer, int longitud, int segundos);
int envia_mensaje(int s_cliente, char *datos, int len, int segundos);
char *lee_mensaje(int s_cliente, int segundos, int *out_longitud);

#endif

