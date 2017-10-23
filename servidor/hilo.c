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

#include "traduccion.h"
#include "util.h"
#include "prlib.h"


void atiende_cliente(int s_cliente,DATOS *info);
void imprimeTablero(char *tab);

// Constantes
#define PLAZO        60         /* plazo de espera lectura/escritura */
#define PLAZO_CONEX  60         /* plazo de espera conexion */


/*
  Funcion principal
*/
void *hilo(void *datos)
{
  int s_servidor;               /* socket del servidor */
  int s_cliente;                /* socket del cliente */

  DATOS *info=(DATOS*)datos;
  char puerto[5];
  strcpy(puerto, info->puerto);

  /* Ignoramos la senial SIGPIPE que se produce cuando se intenta escribir
    en un socket cerrado por el otro extremo cuando ambos extremos están
    en la misma maquina. Si no se ignorara el programa
    se cerraría inmediatamente al ocurrir esto. Al ignorarlo, write devuelve
    un error que podemos tratar en el codigo */
  signal (SIGPIPE, SIG_IGN);

  //Inicializacion del socket de escucha
  s_servidor = inicia_socket_servidor(puerto);

 
  printf("Esperando conexion...\n");
  //Espera a que llegue cliente el tiempo establecido
  if (espera_recepcion(s_servidor, PLAZO_CONEX) == 1)
    {
      //Acepta una nueva conexion
      s_cliente = acepta_cliente(s_servidor);
      if (s_cliente >= 0)
	{
	  atiende_cliente(s_cliente,info);       //Atiende al cliente
	  //Cerramos conexion con el cliente
	  if (close(s_cliente) < 0)
	    fprintf(stderr,"Error cerrando socket de cliente\n");
	  
	  printf("Conexion con el cliente cerrada hilo\n");
	}
    }     


  if (close(s_servidor) < 0)    //Cierra socket de escucha
    error_fatal("close");

  pthread_exit(NULL);
}



/* 
   Funcion que atiende a un cliente. Lee la peticion que llega, la procesa
   y responde al cliente.
   Parametros de entrada:
   - s_cliente - socket del cliente
   Devuelve:
   - Nada
*/
void atiende_cliente(int s_cliente,DATOS *info)
{
  DATOS *sInfo=(DATOS *)info;
  char *datos = NULL;
  int longitud = 0;
  char mapas[212];
  int len=0;
  int i=0;
  int error=0;
  int fin=0;
  char *tablaE=NULL;
  int result=0;

  //variables de info
  int *n_partidas=sInfo->n_partidas;
  int *partidas_disp=sInfo->partidas_disp;
  int idJugador=0;
  if(sInfo->idJugador==1)
    idJugador=1;
  int idPartida=0;
  char puerto[5];
  strcpy(puerto,sInfo->puerto);

  //semaforo para hablar con proceso padre
  sem_t * sem=NULL;
  sem=sem_open("semaforo",O_CREAT,0600,0);
  //variables zona memoria
  key_t claveP;
  key_t claveE;
  int shmidP;
  int shmidE;
  char *mapaPropio;
  char *mapaEnemigo;
  //abro el semaforo que controla acceso a memoria
  char nomSem[10];
  sprintf(nomSem,"%d",sInfo->idPartida);
  idPartida=atoi(nomSem);
  sem_t *semA=NULL;
  sem_t *semB=NULL;
  sprintf(nomSem,"semA%d",idPartida);
  semA=sem_open(nomSem,0);
  sprintf(nomSem,"semB%d",idPartida);
  semB=sem_open(nomSem,0);

  sem_post(sem);
  sem_close(sem);
  //primer jugador
  if(idJugador==0){
    //espera a semaforo del otro cliente
    sem_wait(semA);
    
    //abro zonas de memoria
    claveP=ftok(".",'a'+idPartida);
    claveE=ftok(".",'A'+idPartida);
    if((shmidP=shmget(claveP,sizeof(char *)*LONG_LIN*LONG_COL,0))==-1||(shmidE=shmget(claveE,sizeof(char *)*LONG_LIN*LONG_COL,0))==-1) {
      printf("Error al abrir el segmento\n"); 
      error=1;
    }
    else{
      if((mapaPropio=shmat(shmidP,NULL,0))==(char *)-1||(mapaEnemigo=shmat(shmidE,NULL,0))==(char *)-1){
	printf("Error al mapear el segmento\n");
	error=1;
      }
    }
  }  
  //caso de que el jugador sea el numero 2
  else{
    //subo semaforo A
    sem_post(semA);
    
    //abro zonas de memoria
    claveE=ftok(".",'a'+idPartida);
    claveP=ftok(".",'A'+idPartida);
    if((shmidP=shmget(claveP,sizeof(char *)*LONG_LIN*LONG_COL,0))==-1||(shmidE=shmget(claveE,sizeof(char *)*LONG_LIN*LONG_COL,0))==-1) {
      printf("Error al abrir el segmento\n"); 
      error=1;
    }
    else{
      if((mapaPropio=shmat(shmidP,NULL,0))==(char *)-1||(mapaEnemigo=shmat(shmidE,NULL,0))==(char *)-1){
	printf("Error al mapear el segmento\n");
	error=1;
      }
    }
  }

  if(error==0){
    //pido datos de mapa
    envia_mensaje(s_cliente,"Empieza",7,PLAZO);
    //recibo el mapa
    datos = lee_mensaje(s_cliente, PLAZO, &longitud);
    if(datos!=NULL){
      strcpy(mapaPropio,datos);
    }else{
      fin=1;
    }
    //si el jugador es el 1 subo semaforo para comenzar turno
    if(idJugador==1){
      sem_wait(semA);
      sem_post(semB);
      sleep(1);
    }else{
      sem_post(semA);
    }
    if(fin==1){
      sem_wait(semB);
      sem_post(semB);
    }
    result=0;
    while(fin==0){
      //espera de nuevo el turno, bajo semaforo B
      if(result==0||result==3)
	sem_wait(semB);

      if(mapaPropio[0]=='Z'|| mapaEnemigo[0]=='Z')
	fin=3;
      else
	fin=partida_sigue(mapaPropio, mapaEnemigo);
      
      if(fin==1){
	envia_mensaje(s_cliente,"Victoria",8,PLAZO);
	for(i=0;i<100;i++){
	  mapas[i]=mapaPropio[i];
	  mapas[i+100]=mapaEnemigo[i];
	}
	mapas[200]='\0';
	len=strlen(mapas);
	envia_mensaje(s_cliente,mapas,len,PLAZO);
      }else if(fin==2){
	envia_mensaje(s_cliente,"Derrota",7,PLAZO);
	for(i=0;i<100;i++){
	  mapas[i]=mapaPropio[i];
	  mapas[i+100]=mapaEnemigo[i];
	}
	mapas[200]='\0';
	len=strlen(mapas);
	envia_mensaje(s_cliente,mapas,len,PLAZO);
      }else if(fin==3){
	envia_mensaje(s_cliente,"Error",5,PLAZO);
      }else{
	envia_mensaje(s_cliente,"SIGUE",5,PLAZO);
	//si la partida sigue, envio mapas
	//cambio los barcos por '.' para no dar las coordenadas
	tablaE=(char *)malloc(sizeof(char *)*LONG_LIN*LONG_COL);
	cambia_mapa(mapaEnemigo,tablaE);
	for(i=0;i<100;i++){
	  mapas[i]=mapaPropio[i];
	  mapas[i+100]=tablaE[i];
	}
	mapas[200]='\0';
	len=strlen(mapas);
	//envio mapas
	envia_mensaje(s_cliente,mapas,len,PLAZO);
	free(tablaE);
	tablaE=NULL;
	//recibo coordenadas de tiro
	datos = lee_mensaje(s_cliente, PLAZO, &longitud);

	if (NULL != datos)
	  {
	    datos[longitud]='\0';
	    /* Añade coordenadas al mapa enemigo */
	    result=anade_coordenada(mapaEnemigo, datos);
	    
	    //cambio los barcos por '.' para no dar las coordenadas
	    tablaE=(char *)malloc(sizeof(char *)*LONG_LIN*LONG_COL);
	    cambia_mapa(mapaEnemigo,tablaE);
	    for(i=0;i<100;i++){
	      mapas[i+100]=tablaE[i];
	    }
	    mapas[200]='\0';
	    switch (result){
	    case 0:
	      strcat(mapas,"Agua");
	      mapas[204]='\0';
	      break;
	    case 1:
	      strcat(mapas,"Tocado");
	      mapas[206]='\0';
	      break;
	    case 2:
	      strcat(mapas,"Hundido");
	      mapas[207]='\0';
	      break;
	    case 3:
	      strcat(mapas,"Ya disparado");
	      mapas[212]='\0';
	      break;
	    }
	    len=strlen(mapas);
	    //envio resultado del tiro
	    envia_mensaje(s_cliente,mapas,len,PLAZO);
	    /* Libera memoria */
	    free(datos);
	    datos=NULL;
	    free(tablaE);
	    tablaE=NULL;
	  }
	else{
	  fin=3;
	  mapaPropio[0]='Z';
	  mapaEnemigo[0]='Z';
	}
      }
      //subo semaforo B
      if(result==0||result==3||fin==1||fin==2||fin==3)
	sem_post(semB);
      sleep(1); 
    }
  }else{
    printf("Error de apertura de zonas de memoria en partida %d\n",idPartida);
    envia_mensaje(s_cliente,"Error",5,PLAZO);
  }

    //liberamos memoria
  if(idJugador==0){
    sem_wait(semA);
    anade_puerto(puerto);
    //libera la memoria
    sem_close(semA);
    sem_close(semB);
    mapaPropio[0]='Z';
    mapaEnemigo[0]='Z';
    shmdt(mapaPropio);
    shmdt(mapaEnemigo);

    libera_todo(idPartida);
    //sumamos en 1 el numero de partidas disponibles
    partidas_disp[idPartida]=0;
    *n_partidas=(*n_partidas)-1;
  }else{
    shmdt(mapaPropio);
    shmdt(mapaEnemigo);
    anade_puerto(puerto);
    sem_post(semA);
    sem_close(semA);
    sem_close(semB);
  }
}

void imprimeTablero(char *tab){
  int i=0;
  int j=0;
  printf("    0 1 2 3 4 5 6 7 8 9    \n\n");
  for(i=0;i<LONG_LIN;i++){
    printf("%c   ",i+'A');
    for(j=0;j<LONG_COL;j++){
      printf("%c ",*(tab+i*LONG_LIN+j));
    }
    printf("  %c\n",i+'A');
  }
  printf("\n    0 1 2 3 4 5 6 7 8 9     \n\n");
}
