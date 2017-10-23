#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "traduccion.h"
#include "util.h"


/*
  Convierte una direccion de Internet y un puerto de servicio
  (ambos cadena de caracteres) a valores numericos para poder
  ser utilizados en otras funciones, como bind y connect.
  La informacion tambien se imprimira por pantalla.
  Parametros de entrada:
  - maquina - cadena de caracteres con la direccion de Internet
  - puerto - cadena de caracteres con el puerto de servicio
  - tipo - SOCKET_UDP, SOCKET_TCP o SOCKET_TCP_PASIVO (escucha)
  Parametros de salida:
  - info - estructura addrinfo con el primer valor encontrado
  Devuelve:
  - Verdadero, si ha tenido exito.
*/
int traduce_a_direccion(const char *maquina, const char *puerto,
                        int tipo, struct addrinfo *info)
{
  struct addrinfo hints;        /* Estructura utilizada para afinar la
                                   busqueda */
  struct addrinfo *result, *rp; /*rp, variable usada para recorrer
                                   la lista de direcciones 
                                   encontradas */
  int error = 0;

  /* Obtiene las direcciones que coincidan con maquina/puerto */

  /* Ponemos a 0 la estructura hints */
  memset(&hints, 0, sizeof(struct addrinfo));
  /*Inicializamos la estructura */
  hints.ai_family = AF_INET;    /* AF_UNSPEC Permite IPv4 o IPv6
                                   AF_INET solo IPv4 */
  if (SOCKET_UDP == tipo)
    hints.ai_socktype = SOCK_DGRAM;     /* Socket de datagramas */
  else
    hints.ai_socktype = SOCK_STREAM;    /* Socket de flujo */
  hints.ai_protocol = 0;        /* Cualquier protocolo */
  if (SOCKET_TCP_PASIVO == tipo)
    hints.ai_flags |= AI_PASSIVE;       /* Cualquier direccion IP */


  /*Llamamos a la funcion de busqueda de nombres */
  error = getaddrinfo(maquina, puerto, &hints, &result);
  if (error != 0)
    {
      //Mostramos informacion del error usando la funcion gai_strerror
      fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(error));
    }
  else
    {

      /* getaddrinfo() devuelve una lista de estructuras addrinfo.
         Vamos a imprimirlas todas, aunque solo devolveremos la primera */
      printf("Resultado de la resolucion de nombre:\n");
      for (rp = result; rp != NULL; rp = rp->ai_next)
        {
          printf("-> ");
          imprime_extremo_conexion(rp->ai_addr, rp->ai_addrlen, tipo);
          printf("\n");
        }

      if (result == NULL)
        {                       /* No se ha devuelto ninguna direccion */
          fprintf(stderr, "No se han encontrado direcciones.\n");
          error = 1;            //Hay error
        }
      else
        {
          //Copiamos solo los campos del primer resultado que interesan.
          info->ai_family = result->ai_family;
          info->ai_socktype = result->ai_socktype;
          info->ai_protocol = result->ai_protocol;
          *info->ai_addr = *result->ai_addr;    //Copiamos contenido del puntero
          info->ai_addrlen = result->ai_addrlen;
        }

      freeaddrinfo(result);     /* Liberamos los datos */

    }
  return !error;
}

/*
  Funcion que imprime el nombre de la maquina asociada a una
  direccion de internet y el puerto de una conexion.
  Hace uso de la funcion getnameinfo.
  Parametros de entrada:
  - direccion - estructura sockaddr con informacion de un extremo del socket.
  - len - longitud de la estructura direccion
  - tipo - SOCKET_UDP o SOCKET_TCP
  Devuelve:
  - Nada
*/
void imprime_extremo_conexion(const struct sockaddr *direccion, socklen_t len,
                              int tipo)
{
  char hbufnum[NI_MAXHOST];     //cadena de la maquina (numerico)
  char hbufnombre[NI_MAXHOST];  //cadena de la maquina (nombre)
  char sbuf[NI_MAXSERV];        //cadena del servicio
  int opciones = NI_NUMERICHOST | NI_NUMERICSERV;       //Opciones para getnameinfo
  int error = 0;

  if (tipo == SOCKET_UDP)
    {
      opciones |= NI_DGRAM;
    }
  //Convertimos a cadena de caracteres
  error = getnameinfo(direccion, len, hbufnum, sizeof(hbufnum), sbuf,
                      sizeof(sbuf), opciones);
  if (error == 0)
    {
      //Obtenemos tambien el nombre asociado a esa direccion IP
      if (tipo == SOCKET_TCP_PASIVO)
        printf("Escuchando en ");
      else if (getnameinfo(direccion, len, hbufnombre, sizeof(hbufnombre),
                           NULL, 0, NI_NAMEREQD))
        //Error obteniendo el nombre
        printf("Maquina=(desconocida) ");
      else
        printf("Maquina=%s ", hbufnombre);

      //Imprimimos valores numericos.
      printf("(%s), Puerto=%s", hbufnum, sbuf);
      if (tipo == SOCKET_UDP)
        printf(" UDP");
      else
        printf(" TCP");

    }
  else
    //Mostramos informacion del error usando la funcion gai_strerror
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(error));
}

/*
  Realiza el proceso de inicializacion estandar de un socket TCP activo.
  En caso de error provoca el fin del programa.
  Parametros de entrada:
  - maquina - cadena de texto con el nombre o direccion IP de la maquina.
  - puerto - cadena de texto con el puerto a usar. Puede ser un numero
             o un nombre de servicio existente en el fichero services
  Devuelve:
  - Nuevo socket.
*/
int inicia_socket_cliente(const char *maquina, const char *puerto)
{
  int s;
  struct addrinfo infoserv;     /* informacion extremo local */
  struct sockaddr dirserv;      /* direccion internet socket servidor */

  /* Ponemos a 0 la estructura infoserv */
  memset(&infoserv, 0, sizeof(struct addrinfo));
  infoserv.ai_addr = &dirserv;  //aqui se guardara la direccion
  /* Obtenemos datos del extremo al que queremos llamar.
     Si tiene exito, utilizaremos los datos almacenados en infoserv 
     para crear el socket. */
  if (!traduce_a_direccion(maquina, puerto, SOCKET_TCP, &infoserv))
    exit(EXIT_ERROR); //EXIT_ERROR es una constante definida en util.h

  /* Crea socket de TCP */
  if ((s = socket(infoserv.ai_family, infoserv.ai_socktype,
                  infoserv.ai_protocol)) < 0)
    error_fatal("socket");

  /* Conectamos el socket */
  if (connect(s, &dirserv, sizeof(dirserv)) < 0)
    {
      close(s);
      error_fatal("connect");
    }
  return s;
}


/*
  Acepta una nueva conexion a partir del socket de escucha pasado como 
  parametro. 
  Parametros de entrada:
  - s_escucha - socket que esta escuchando del servidor
  Devuelve:
  - Nuevo socket de cliente, <0 si hay error.
*/
int acepta_cliente(int s_escucha)
{
  struct sockaddr dir_cliente;       /* direccion socket del cliente */
  socklen_t len_dir = sizeof(dir_cliente);
  /* len_dir es un parametro de entrada/salida en la funcion accept */
  int s_nuevo;

  s_nuevo = accept(s_escucha, &dir_cliente, &len_dir);
  if (s_nuevo < 0)
    perror("accept");           //Mostramos error, conexion no valida
  else
    {
      //Conexion correcta
      //Imprimimos informacion de la nueva conexion
      printf("Nueva conexion: ");
      imprime_extremo_conexion(&dir_cliente, len_dir,
                               SOCKET_TCP);
      printf("\n");
    }
  return s_nuevo;
}



/*
  Realiza el proceso de inicializacion estandar de un socket de escucha.
  En caso de error provoca el fin del programa.
  Parametros de entrada:
  - puerto - cadena de texto con el puerto a usar. Puede ser un numero
  o un nombre de servicio existente en el fichero services
  Devuelve:
  - Nuevo socket de escucha.
*/
int inicia_socket_servidor(const char *puerto)
{
  int s;
  struct addrinfo infoserv;     /* informacion extremo local */
  struct sockaddr dirserv;      /* direccion internet socket servidor */
  int on = 1;

  /* Calcula "nombre local" */
  /* Ponemos a 0 la estructura infoserv */
  memset(&infoserv, 0, sizeof(struct addrinfo));
  infoserv.ai_addr = &dirserv;  //aqui se guardara la direccion
  /* Al indicar SOCKET_TCP_PASIVO, indicamos que queremos escuchar 
     en todas las interfaces de la maquina. Si tiene exito, utilizaremos
     los datos almacenados en infoserv para crear el socket. */
  if (!traduce_a_direccion(NULL, puerto, SOCKET_TCP_PASIVO, &infoserv))
    exit(EXIT_ERROR);

  /* Crea socket de TCP */
  if ((s = socket(infoserv.ai_family, infoserv.ai_socktype,
                  infoserv.ai_protocol)) < 0)
    error_fatal("socket");

  /* Reusar direccion si hay conexiones activas pero no escuchando. */
  if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0)
    {
      close(s);
      error_fatal("setsockopt");
    }
  /* Asigna nombre local al socket */
  if (bind(s, &dirserv, sizeof(dirserv)) < 0)
    {
      close(s);
      error_fatal("bind");
    }

  /* Prepara al servidor para aceptar conexiones */
  if (listen(s, 1) < 0)   //Maximo 1 cliente esperando a ser atendido
    {
      close(s);
      error_fatal("listen");

    }
  return s;
}
