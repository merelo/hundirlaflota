/*
  Funciones que va a usar el hilo
*/
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <string.h>
#include <sys/msg.h> 
#include <signal.h>

#include "prlib.h"

/*
  Funcion: anade_coordenada
  Funcion que añade tiro al mapa y devuelve 0 1 2 o 3
  Recibe:
  mapa: mapa donde añadir el tiro
  cad: cadena con la coordenada de tiro, p.e. (A1)
  Devuelve:
  0 si el tiro cae en agua (caracter '.')
  1 si el tiro cae en un barco y no esta hundido (siguen quedando otras letras del mismo barco)
  2 si el tiro cae en un barco y barco hundido (no hay mas letras de ese barco)
  3 si el tiro cae en una coordenada que ya ha sido disparada (caracter '*' o caracter '~')
*/
int anade_coordenada(char * mapa, char * pos) {
  //si el tiro da en agua, se sustituye la coordenada por '~'
  //si el tiro cae en barco, se sustituye la coordenada por '*'
  int res;
  int col;
  int fil;
  char casilla;

  if (busca_error(pos))
    res = ERROR;
  else {
    fil = (int) (pos[0] - 'A');
    col = (int) (pos[1] - '0');

    casilla = *(mapa + (fil * LONG_LIN) + col);

    switch (casilla) {
    case 'A':
    case 'B':
    case 'C':
    case 'D':
    case 'E':
      *(mapa + (fil * LONG_LIN) + col) = '*';
      res = TOCADO;
      if (busca_barco(mapa, casilla) == 0)
	res = HUNDIDO;
      break;
    case '.':
      *(mapa + (fil * LONG_LIN) + col) = '~';
      res = AGUA;
      break;
    default:
      res = DISPARADO;
    }
  }

  return res;
}

/*
  Funcion: partida_sigue
  Funcion que recibe dos mapas y comprueba si la partida puede seguir
  Recibe:
  mPropio: mapa del cliente de este hilo
  mEnemigo: mapa del enemigo de este hilo
  Devuelve:
  0 si la partida sigue
  1 si el mapaPropio gana (que en mEnemigo no haya 'A','B','C','D' ni 'E')
  2 si el mapaEnemigo gana (que en mPropio no haya 'A','B','C','D' ni 'E')
  3 si el mapaEnemigo no tiene letra o *
*/
int partida_sigue(char * mPropio, char * mEnemigo) {

  int n;
  int i;

  n = 1;
  for (i = 0; i < 5; i++)
    if (busca_barco(mEnemigo, (char) 'A' + i))
      n = 0;

  if (n == 0) {
    n = 2;
    for (i = 0; i < 5; i++)
      if (busca_barco(mPropio, (char) 'A' + i))
	n = 0;
  }else{
    if(!busca_barco(mEnemigo,(char)'*'))
      n=3;
  }

  return n;
}

/*
  Funcion: cambia_mapa
  Funcion que sustituye los 'A', 'B', 'C', 'D', 'E' por '.'
  Recibe:
  mapa: 		mapa a cambiar
  mapaux: 	nuevo mapa cambiado
*/
void cambia_mapa(char * mapa, char * mapaux) {
  int fil, col;
  char nmapa[LONG_LIN * LONG_COL];
  char casilla;

  for (fil = 0; fil < LONG_LIN; fil++) {
    for (col = 0; col < LONG_COL; col++) {

      casilla = *(mapa + (fil * LONG_LIN) + col);

      switch (casilla) {
      case 'A':
      case 'B':
      case 'C':
      case 'D':
      case 'E':
	*(nmapa + (fil * LONG_LIN) + col) = '.';
	break;
      default:
	*(nmapa + (fil * LONG_LIN) + col) = casilla;
	break;
      }

    }
  }
  // Se copia el nuevo mapa en el dado por parametro.
  strncpy(mapaux, nmapa, LONG_LIN * LONG_COL);
}
/*
  Funcion: libera_todo
  Funcion que libera y borra zonas de memoria compartida y semaforos
  Recibe:
  npartida: numero de la partida para definir semaforos y memoria
  Devuelve:
  nada
*/
void libera_todo(int npartida) {
  //libera zona de memoria con clave 'a'+npartida y 'A'+npartida
  //borra semaforos con clave ("semA%s",npartida) y ("semB%s",npartida)
  //si la partida es 1, las claves son "semA1" y "semB1"
  char semaforoA[10];
  char semaforoB[10];
  int shmid;
  size_t tam_seg = LONG_LIN * LONG_COL * sizeof(char);
  key_t clave1;
  key_t clave2;

  //Marcado de la memoria compartida para borrado.
  clave1 = ftok(".", 'a' + npartida);
  clave2 = ftok(".", 'A' + npartida);

  if ((shmid = shmget(clave1, tam_seg, 0)) == -1) {
    fprintf(stderr, "El segmento de memoria compartida no existe\n");
  } else {
    shmctl(shmid, IPC_RMID, NULL);
    printf("Segmento de memoria marcado para borrar\n");
  }
  if ((shmid = shmget(clave2, tam_seg, 0)) == -1) {
    fprintf(stderr, "El segmento de memoria compartida no existe\n");
  } else {
    shmctl(shmid, IPC_RMID, NULL);
    printf("Segmento de memoria marcado para borrar\n");
  }

  //Eliminacion de los semaforos.
  sprintf(semaforoA, "semA%d", npartida);
  sprintf(semaforoB, "semB%d", npartida);

  sem_unlink(semaforoA);
  sem_unlink(semaforoB);
  printf("\n\nborro todo\n\n");
}

/*
 * Función: busca_barco
 * Función que cuenta el numero de veces que aparece determinado
 * caracter en el tablero.
 * Recibe:
 * 1- mapa: Contiene el mapa.
 * 2- letra: Caracter a buscar en el mapa.
 * Devuelve:
 * -1 	si hay error.
 * 0 	si no hay error.
 * */
int busca_barco(char * mapa, char letra) {

  int encontrado = 0;
  int i;
  int j;

  for (j = 0; j < LONG_LIN; j++) {
    for (i = 0; i < LONG_COL; i++) {
      if (*(mapa + (i * LONG_LIN) + j) == letra)
	encontrado++;
    }
  }

  return encontrado;
}

/*
 * Función: busca_error
 * Función que comprueba que la coordenada dada (p.e. A1)
 * es correcta.
 * Recibe:
 * 1- cad: string que contiene la coordenada mediante dos caracteres.
 * Devuelve:
 * -1 	si hay error.
 * 0 	si no hay error.
 * */
int busca_error(char * cad) {

  int res = 0;
  int fila = (int) cad[1] - '0';
  int col = (int) tolower(cad[0]) - 'a';

  if ((col < 0) || (col > (LONG_COL - 1)))
    res = ERROR;
  if ((fila < 0) || (fila > LONG_LIN-1))
    res = ERROR;

  return res;
}


int introduce_puerto(int qid,struct mymsgbuf *qbuf) 
{ 
  int resultado;
   
  resultado=msgsnd(qid,qbuf,5,0);
           
  return (resultado);

} 


// funcion que anade el puerto pasado por parametro a la cola de mensajes
void anade_puerto(char * puerto){
  key_t clave;
  struct mymsgbuf qbuffer; 
  int msgqueue_id;
  long tipo=1;

  clave=ftok(".",'1');
  if ((msgqueue_id=msgget(clave,IPC_CREAT|0660))==-1)
    {
      printf("Error al iniciar la cola\n");
    }
  else
    {
      puerto[5]='\0';
      qbuffer.mtype=tipo; //tipo mensaje
      strcpy(qbuffer.mtext,puerto); 
      printf("metere: %s\n",qbuffer.mtext);
      if(introduce_puerto(msgqueue_id,&qbuffer)==-1)
	printf("error peta todo\n");
    }
}
