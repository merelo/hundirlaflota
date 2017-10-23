/***************************************************************
 *
 *   Funciones comunes del servicio invierte
 *
 ****************************************************************/

#include <stdio.h>
#include <sys/select.h>
#include <unistd.h>
#include <stdlib.h>

#include "util.h"

/* 
   Muestra por la salida de errores informacion del ultimo error
   producido y sale del programa.
   Parametros de entrada:
   - mens - cadena de texto que se agrega al mensaje de error
   Devuelve:
   - Nada
*/
void error_fatal(char *mens)
{
  perror(mens);
  exit(EXIT_ERROR);
}

/* 
   Bloquea el programa hasta que no haya algo que leer del descriptor 
   pasado como parametro s (puede ser un socket o un fichero) o hasta que
   transcurran un plazo (segundos) sin recibir nada.
   Parametros de entrada:
   - descriptor - descriptor del fichero o socket
   - segundos - tiempo maximo de espera en segundos
   Devuelve:
   - 1 si se puede leer, 0 si ha vencido el plazo.
*/
int espera_recepcion(int descriptor, int segundos)
{
  
  struct timeval plazo = { segundos, 0L };      /* plazo de recepcion */

  fd_set fds;                   //Conjunto de descriptores a monitorizar
  FD_ZERO(&fds);                //Limpiamos fds
  FD_SET(descriptor, &fds);     //Insertamos descriptor en el conjunto

  if(segundos==0)
    return (select
	    (descriptor + 1, &fds, (fd_set *) NULL, (fd_set *) NULL, NULL));
  else
    return (select
	    (descriptor + 1, &fds, (fd_set *) NULL, (fd_set *) NULL, &plazo));
}


/* 
   Bloquea el programa hasta que no se pueda escribir en el descriptor 
   pasado como parametro s (puede ser un socket o un fichero) o hasta que
   transcurran un plazo (segundos) sin poder escribir.
   Parametros de entrada:
   - descriptor - descriptor del fichero o socket
   - segundos - tiempo maximo de espera en segundos
   Devuelve:
   - 1 si se puede escribir, 0 si ha vencido el plazo.
*/
int espera_envio(int descriptor, int segundos)
{

  struct timeval plazo = { segundos, 0L };      /* plazo para poder escribir */

  fd_set fds;                   //Conjunto de descriptores a monitorizar
  FD_ZERO(&fds);                //Limpiamos fds
  FD_SET(descriptor, &fds);     //Insertamos descriptor en el conjunto
  return (select
          (descriptor + 1, (fd_set *) NULL, &fds, (fd_set *) NULL, &plazo));

}

/* 
   Funcion que lee del descriptor (socket o fichero) el numero de bytes indicados
   por parametro y los almacena en el buffer pasado.
   Tiene en cuenta errores de fin de fichero o cierre de socket y que en una
   lectura puede que no se lean todos los bytes pedidos. Tambien deja de leer
   si entre lecturas pasa mas de un determinado tiempo.
   Parametros de entrada:
   - s - descriptor del fichero o socket
   - buffer - buffer donde se guardaran los bytes leidos
   - longitud - numero de bytes que se quieren leer
   - segundos - tiempo maximo de espera en segundos entre lecturas
   Devuelve:
   - Verdadero si ha conseguido leer todos los bytes pedidos
*/
int lee(int s, char *buffer, int longitud, int segundos)
{
  int leidos_total = 0;         //Datos leidos hasta el momento
  int leidos_actual = 0;        //Datos leidos en la ultima peticion de lectura
  int error = FALSE;
  while (leidos_total < longitud && !error)
    {
      //Esperamos a que haya datos disponibles
      if (espera_recepcion(s, segundos) == 1)
        {
          leidos_actual =
            read(s, buffer + leidos_total, longitud - leidos_total);
          if (leidos_actual == 0)       //Indica fin de fichero o cierre del socket
            error = TRUE;
          else if (leidos_actual < 0)   //Ha habido un error
            {
              perror("read");
              error = TRUE;
            }
          else
            leidos_total += leidos_actual;
        }
      else
        {
          fprintf(stderr, "La lectura tarda demasiado\n");
          error = TRUE;
        }
    }

  return (leidos_total == longitud);
}

/* 
   Funcion que escribe en el descriptor (socket o fichero) el numero de bytes 
   indicados por parametro del buffer tambien pasado.
   Tiene en cuenta errores de fin de fichero o cierre de socket y que en una
   escritura puede que no se escriban todos los bytes pedidos. Tambien deja de
   escribir si entre escrituras pasa mas de un determinado tiempo.
   Parametros de entrada:
   - s - descriptor del fichero o socket
   - buffer - buffer que contiene los bytes a escribir
   - longitud - numero de bytes que se quieren escribir
   - segundos - tiempo maximo de espera en segundos entre escrituras
   Devuelve:
   - Verdadero si ha conseguido escribir todos los bytes pedidos
*/
int escribe(int s, const char *buffer, int longitud, int segundos)
{
  int escritos_total = 0;       //Datos escritos hasta el momento
  int escritos_actual = 0;      //Datos escritos en la ultima peticion
  int error = FALSE;
  while (escritos_total < longitud && !error)
    {
      //Esperamos a que podamos enviar datos
      if (espera_envio(s, segundos) == 1)
        {
          escritos_actual =
            write(s, buffer + escritos_total, longitud - escritos_total);
          if (escritos_actual == 0)     //Indica fin de fichero o cierre del socket
            error = TRUE;
          else if (escritos_actual < 0) //Ha habido un error
            {
              perror("write");
              error = TRUE;
            }
          else
            escritos_total += escritos_actual;
        }
      else
        {
          fprintf(stderr, "La escritura tarda demasiado\n");
          error = TRUE;
        }
    }
  return (escritos_total == longitud);
}

/* 
   Lee un mensaje de un socket cliente. Espera recibir un entero (4 bytes)
   codificado como esta en memoria con la longitud de los datos, seguido de 
   los datos. Tiene en cuenta el tiempo maximo de espera. 
   Devuelve los datos leidos (memoria dinamica que
   hay que liberar tras su uso) y la longitud de estos (out_longitud)
   Parametros de entrada:
   - s_cliente - socket del cliente
   - segundos - tiempo maximo de espera en segundos entre lecturas
   Parametros de salida
   - out_longitud - longitud de los datos leidos
   Devuelve:
   - Datos leidos. Hay que liberar la memoria despues de su uso.
   NULL en caso de error.
*/
char *lee_mensaje(int s_cliente, int segundos, int *out_longitud)
{
  int longitud = 0;             //Longitud de los datos que vamos a recibir
  char *buf = NULL;             //Buffer para almacenar los datos leidos

  /* Leemos longitud: pasamos a la funcion lee la direccion de memoria de la
     variable longitud y su tamanio para que los bytes se guarden ahi */
  if (lee(s_cliente, (char *) &longitud, sizeof(longitud), segundos)
      && longitud > 0)
    {                           //Longitud recibida y mayor que cero
      //Creamos buffer con memoria dinamica de ese tamanio
      *out_longitud = longitud;
      buf = (char *) malloc(longitud);
      if (NULL != buf)          //Hay memoria
        {
          //Leemos datos
          if (!lee(s_cliente, buf, longitud, segundos))
            {
              fprintf(stderr, "Error recibiendo datos.\n");
              //Liberamos memoria
              free(buf);
              buf = NULL;
            }
        }
      else
        fprintf(stderr, "No hay memoria.\n");
    }

  return buf;
}

/* 
   Envia un mensaje. Envia un entero (4 bytes)
   codificado como esta en memoria con la longitud de los datos, seguido de 
   los datos. Tiene en cuenta el tiempo maximo de espera. 
   Parametros de entrada:
   - s_cliente - socket del cliente
   - datos - datos a enviar
   - len - longitud de los datos a enviar
   - segundos - tiempo maximo de espera en segundos entre escrituras
   Devuelve:
   - Verdadero si se han enviado todos los bytes.
*/
int envia_mensaje(int s_cliente, char *datos, int len, int segundos)
{
  int correcto = TRUE;

  //Enviamos longitud
  if (escribe(s_cliente, (char *) &len, sizeof(len), segundos))
    {
      //Enviamos datos
      if (!escribe(s_cliente, datos, len, segundos))
        {
          fprintf(stderr, "Error enviando datos.\n");
          correcto = FALSE;
        }
    }
  else
    {
      fprintf(stderr, "Error enviando longitud.\n");
      correcto = FALSE;
    }

  return correcto;
}
