#ifndef TRADUCCION_H
#define TRADUCCION_H

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>


//Constantes usadas en las funciones
#define SOCKET_UDP 0
#define SOCKET_TCP 1
#define SOCKET_TCP_PASIVO 2

//Funciones
int traduce_a_direccion(const char *maquina, const char *puerto,
                        int tipo, struct addrinfo *info);
void imprime_extremo_conexion(const struct sockaddr *direccion,
                              socklen_t len, int tipo);
int inicia_socket_cliente(const char *maquina, const char *puerto);
int inicia_socket_servidor(const char *puerto);
int acepta_cliente(int s_escucha);

#endif

