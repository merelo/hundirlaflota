#ifndef PRLIB_H
#define PRLIB_H

#define AGUA	0
#define TOCADO	1
#define HUNDIDO	2
#define DISPARADO	3
#define ERROR	-1

#define LONG_COL 10
#define LONG_LIN 10

typedef struct{
  int idPartida;
  int idJugador;
  char puerto[5];
  int *n_partidas; //variable para controlar maximo de partidas
  int *partidas_disp;
}DATOS;

struct mymsgbuf{ 
   long mtype; 
   char mtext[5]; 
}; 

int anade_coordenada(char * mapa, char * pos);

int partida_sigue(char * mPropio, char * mEnemigo);

void cambia_mapa(char * mapa, char * mapaux);

void libera_todo(int npartida);

int busca_barco(char * mapa, char letra);

int busca_error(char * cad);

int introduce_puerto(int qid,struct mymsgbuf *qbuf);

void anade_puerto(char *puerto);

#endif
