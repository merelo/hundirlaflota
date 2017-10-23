#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <signal.h>

#include "traduccion.h"
#include "util.h"
#include "prlib.h"
#include "hilo.h"

#include <sys/msg.h> 
#include "semaphore.h"
#include "fcntl.h"
#include <sys/ipc.h> 
#include <sys/shm.h> 
#include <ctype.h> 

#define MAX_SEND_SIZE 80 

#define LONG_LIN 10
#define LONG_COL 10
#define MAX_PARTIDAS 25

int peticion(int sc, int *modo);
void atiende_cliente(int s_cliente,char *tabla);
void procesa(char *datos, char *tabla);
int mete_puerto(int qid,struct mymsgbuf *qbuf); 
int saca_puerto(int qid,long type,struct mymsgbuf *qbuf); 
void crea_memoria(int idPartida);
int numPartida(int *huecos);

// Constantes
#define PLAZO        60         /* plazo de espera lectura/escritura */
#define PLAZO_CONEX  0         /* plazo de espera conexion */
#define PUERTO       "20000"      /* nombre del servicio
                                   debe estar en el fichero services */


int mete_puerto(int qid,struct mymsgbuf *qbuf) 
{ 
  int resultado;
   
  resultado=msgsnd(qid,qbuf,5,0);
           
  return (resultado);

} 

int saca_puerto(int qid,long type,struct mymsgbuf *qbuf) 
{ 
  int resultado;
   
  resultado=msgrcv(qid,qbuf,5,type,IPC_NOWAIT); 
  
  return (resultado); 
} 





/*
  Funcion principal
*/
int main()
{
  int s_servidor;               /* socket del servidor */
  int s_cliente;                /* socket del cliente */
  int continuar = TRUE;         /* Indica cuando hay que salir del programa */
  int i = 0;

  //gestion numero de partidas
  int n_partidas=0;
  int partidas_disp[MAX_PARTIDAS];
  
//variables para colas	
  key_t clave; 
  int msgqueue_id; 
  long tipo=1; 
  struct mymsgbuf qbuffer; 
  int puertoint=20000;
  char puerto[6];
  int control=0;
  DATOS datos;
  int idPartida=0;
  //mensajes
  char * max="El numero de partidas es maximo";

  //semaforo entre hilo con id 0 y proceso principal
  sem_t * sem=NULL;
  sem=sem_open("semaforo",O_CREAT,0600,0);
  //para creacion de hilos
  pthread_t th;
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
  //creacion cola con puertos
  clave=ftok(".",'1');
  
  for(i=0;i<MAX_PARTIDAS;i++)
    partidas_disp[i]=0;

  if ((msgqueue_id=msgget(clave,IPC_CREAT|0660))==-1) 
    { 
      printf("Error al iniciar la cola\n"); 
    } 
  else 
    {
        
      for(i=1;i<=(MAX_PARTIDAS*2);i++){//relleno cola de puertos
	puertoint=20000+i;
	sprintf(puerto,"%d",puertoint);	
        qbuffer.mtype = tipo;
	strcpy(qbuffer.mtext,puerto);
        mete_puerto(msgqueue_id,&qbuffer);//Introduce puerto en la cola
        
      }
    }


  
  /* Ignoramos la senial SIGPIPE que se produce cuando se intenta escribir
    en un socket cerrado por el otro extremo cuando ambos extremos están
    en la misma maquina. Si no se ignorara el programa
    se cerraría inmediatamente al ocurrir esto. Al ignorarlo, write devuelve
    un error que podemos tratar en el codigo */
  signal (SIGPIPE, SIG_IGN);

  //Inicializacion del socket de escucha
  s_servidor = inicia_socket_servidor(PUERTO);

  while (continuar)
    {
      printf("Esperando conexiones...\n");
      //Espera a que llegue cliente el tiempo establecido
      if (espera_recepcion(s_servidor, PLAZO_CONEX) == 1)
        {
          
          //Acepta una nueva conexion
         
          s_cliente = acepta_cliente(s_servidor);
          printf("CLIENTE ACEPTADO\n");
          if (s_cliente >= 0)
            {
              //atiende_cliente(s_cliente,&tabla[0][0]);       //Atiende al cliente
             
	      if((control==0&&numPartida(partidas_disp)==-1)||saca_puerto(msgqueue_id,tipo,&qbuffer)==-1) {//extraigo elemento de la cola
		printf("El numero de partidas es maximo\n");	
		
		if (!envia_mensaje(s_cliente, max, strlen(max), PLAZO))//envio mensaje maximo partidas
		  fprintf(stderr, "Fallo al enviar la respuesta\n");
		
	      }else{
		if(control==0){
		  idPartida=numPartida(partidas_disp);
		  partidas_disp[idPartida]=1;
		  crea_memoria(idPartida);
		}
		//creamos el hilo
		datos.idPartida=idPartida;
		datos.idJugador=control;
		datos.n_partidas=&n_partidas;
		datos.partidas_disp=partidas_disp;
		
		printf("idJugador: %d\n",control);
		qbuffer.mtext[5]='\0';
		strcpy(datos.puerto,qbuffer.mtext);		
		pthread_create(&th, &attr, hilo, &datos);
	
		if (!envia_mensaje(s_cliente, datos.puerto, strlen(datos.puerto), PLAZO))//envio mensaje con numero de puerto
		  fprintf(stderr, "Fallo al enviar el numero de puerto\n");

                sem_wait(sem);	
		control=1-control;
              }
	      
              //Cerramos conexion con el cliente
              if (close(s_cliente) < 0)
                fprintf(stderr,"Error cerrando socket de cliente\n");
	      
              printf("Conexion con el cliente cerrada\n");
            }
        }
      else
        continuar = FALSE;      //Falso        
    }
  fprintf(stderr, "*** no oigo nada ***\n");

  if (close(s_servidor) < 0)    //Cierra socket de escucha
    error_fatal("close");

  return 0;
}

void crea_memoria(int idPartida){
    char semaforo[10];
    sem_t * sem_A = NULL;
    sem_t * sem_B=NULL;
    key_t clave; 
    int shmid; 
    char *seg=NULL;
    
    sprintf(semaforo,"semA%d",idPartida);
    sem_A=sem_open(semaforo,O_CREAT,0600,0);
    sprintf(semaforo,"semB%d",idPartida);
    sem_B=sem_open(semaforo,O_CREAT,0600,0);

      
    clave=ftok(".",'a'+idPartida);
    
    if((shmid=shmget(clave,sizeof(char *)*LONG_LIN*LONG_COL,IPC_CREAT|IPC_EXCL|0660))==-1){
      printf("La memoria ya existe\n");
    }else{
      if((seg=shmat(shmid,NULL,0))==(char *)-1){    
	printf("Error al mapear el segmento\n");
      }      
       shmdt(seg);
    }  
  
    
    clave=ftok(".",'A'+idPartida);
    if((shmid=shmget(clave,sizeof(char *)*LONG_LIN*LONG_COL,IPC_CREAT|IPC_EXCL|0660))==-1){
      printf("La memoria ya existe\n");
    }else{
      if((seg=shmat(shmid,NULL,0))==(char *)-1){
	printf("Error al mapear el segmento\n");
      }
      shmdt(seg);
    }
    sem_close(sem_A);
    sem_close(sem_B);
}


int numPartida(int *huecos){
  int idPartida=-1;
  int i=0;
  for(i=0;i<MAX_PARTIDAS&&idPartida==-1;i++){
    if(huecos[i]==0)
      idPartida=i;
  }

  return idPartida;
}
