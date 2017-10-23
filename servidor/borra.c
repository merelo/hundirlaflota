#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/msg.h>

#include <ctype.h>
#include <semaphore.h>
#include <fcntl.h>
#include <unistd.h>

#define TAM_BUFFER 100

int main(){
  int shmid;
  int i=0;
  key_t clave=ftok(".",'a');
  int msgqueue_id;
  if(sem_unlink("semA0")==-1){
    printf("Error eliminando semaforo 'huecos'\n");
  }
  if(sem_unlink("semB0")==-1){
    printf("Error eliminando semaforo 'elementos'\n");
  }


  for(i=0;i<26;i++){
    clave=ftok(".",'a'+i);
    if((shmid=shmget(clave,sizeof(char*)*(TAM_BUFFER),IPC_CREAT|0660))==-1){
      printf("Error al abrir el segmento\n");
    }else{
      if(shmctl(shmid,IPC_RMID,0)==-1){
	printf("Error eliminando el segmento\n");
      }
    }
    clave=ftok(".",'A'+i);
    if((shmid=shmget(clave,sizeof(char*)*(TAM_BUFFER),IPC_CREAT|0660))==-1){
      printf("Error al abrir el segmento\n");
    }else{
      if(shmctl(shmid,IPC_RMID,0)==-1){
	printf("Error eliminando el segmento\n");
      }
    }
  }

  clave=ftok(".",'1');
  if ((msgqueue_id=msgget(clave,IPC_CREAT|0660))==-1)
    {
      printf("Error al iniciar la cola\n");
    }
  else
    { 
      if(-1==msgctl(msgqueue_id, IPC_RMID, 0))
	printf("ERROR BORRANDO COLA\n");
    }
  return 0;
}
