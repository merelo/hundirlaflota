#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <signal.h>

#include "funcCliente.h"
#include "traduccion.h"
#include "util.h"


//Constantes
#define PLAZO         120        /* plazo de espera lectura/escritura */
#define PUERTO       "20000"

int main(int argc, char **argv)
{
  int s_cliente;                /* socket del cliente */
  int continuar = TRUE;         /* Indica cuando hay que salir del programa */
  int error = 0;                /* Indica si ha habido error */
  char linea[20];    /* Linea leida de la entrada estandar */
  int len = 0;                  /* Longitud de la linea a enviar*/
  int len_recibido = 0;          /* Longitud del mensaje recibido */
  char *respuesta = NULL;       /* Datos leidos en la respuesta del servidor */
  char *puerto=NULL;
  int s_conec;
  char *tablero=NULL;
  char *mapas=NULL;
  char mEnemigo[100];
  char mPropio[100];
  char *mensaje=NULL;


  int i=0;

  /* Ignoramos la senial SIGPIPE que se produce cuando se intenta escribir
    en un socket cerrado por el otro extremo cuando ambos extremos están
    en la misma maquina. Si no se ignorara el programa
    se cerraría inmediatamente al ocurrir esto. Al ignorarlo, write devuelve
    un error que podemos tratar en el codigo */
  signal (SIGPIPE, SIG_IGN);
  

  system("clear");
  /* Comprueba numero de argumentos */
  if (argc != 2)
    {
      fprintf(stderr, "Uso: a.out IP\n");
      error=1;
    }
  else
    {
      //Inicializacion del socket del cliente. Si falla sale del programa.
      s_conec = inicia_socket_cliente(argv[1], PUERTO);
      printf("Conexion creada. \n");


      respuesta = lee_mensaje(s_conec, PLAZO, &len_recibido);
      if (NULL == respuesta){
	printf("Respuesta incorrecta del servidor\n");
	error_fatal("close");
      }
      if (close(s_conec) < 0)     //Cierra el socket
        error_fatal("close");
      sleep(2);
      puerto=(char *)calloc(len_recibido,sizeof(char));
      for(i=0;i<len_recibido;i++){
	puerto[i]=respuesta[i];
      }
      //free(respuesta);
      //respuesta=NULL;
      system("clear");
      if(len_recibido>5){
	printf("El servidor esta atendiendo el numero maximo de partidas, vuelve a intentarlo mas tarde\n");
	free(puerto);
	puerto=NULL;
      }else{
	//Inicializacion del socket del cliente. Si falla sale del programa.
	s_cliente = inicia_socket_cliente(argv[1], puerto);
	printf("Conexion creada. \n"
	       "Esperando rival\n\n");

	free(puerto);
	puerto=NULL;

	//espero confirmacion de partida
	respuesta = lee_mensaje(s_cliente, PLAZO, &len_recibido);
	system("clear");
	if (len_recibido > 0 &&respuesta!=NULL && strcmp(respuesta,"Error")!=0){
	  printf("Rival encontrado\n");
	  tablero=(char *)calloc(LONG_LIN*LONG_COL,sizeof(char));
	  crearTablero(tablero);
	  mete_datos(tablero);
	  len=strlen(tablero);
	  envia_mensaje(s_cliente, tablero, len, PLAZO);
	  printf("Esperando turno\n");
	}else{
	  free(respuesta);
	  respuesta=NULL;
	  printf("No hay rival\n");
	  error_fatal("close");
	}
	//	free(respuesta);
	//respuesta=NULL;

	while (continuar)
	  {
	    continuar=FALSE;  //Por defecto, a menos que todo vaya bien
	    len = 0;  //Valor inicial

	    //esperamos a nuestro turno
	    respuesta = lee_mensaje(s_conec, PLAZO, &len_recibido);
	    if(respuesta==NULL||len_recibido<=0){
	      printf("Error en el servidor\n");
	      error_fatal("close");
	    }
	    respuesta[len_recibido]='\0';
	    system("clear");
	    if(strcmp(respuesta,"Victoria")==0){
	      mapas = lee_mensaje(s_conec, PLAZO, &len_recibido);

	      //Enviamos peticion si el mensaje no esta vacio
	      if (len_recibido > 0&&NULL!=mapas)
		{
		  mapas[len_recibido]='\0';
		  for(i=0;i<100;i++){
		    mEnemigo[i]=mapas[i+100];
		    mPropio[i]=mapas[i];
		  }
		  /*for(i=200;i<(int)strlen(mapas);i++)
		    mensaje[i-200]=mapas[i];*/
		//  free(mapas);
		 // mapas=NULL;
		 // free(respuesta);
		  imprime2tablero(mPropio,mEnemigo);
		}
	      printf("Enhorabuena, has ganado\n");
	    }else if(strcmp(respuesta,"Derrota")==0){
	      mapas = lee_mensaje(s_conec, PLAZO, &len_recibido);
	      //Enviamos peticion si el mensaje no esta vacio
	      if (len_recibido > 0&&NULL!=mapas)
		{
		  mapas[len_recibido]='\0';
		  for(i=0;i<100;i++){
		    mEnemigo[i]=mapas[i+100];
		    mPropio[i]=mapas[i];
		  }
		//  free(mapas);
		//  mapas=NULL;
		//  free(respuesta);
		  imprime2tablero(mPropio,mEnemigo);
		}
	      printf("Lo siento, has perdido\n");
	    }else if(strcmp(respuesta,"Error")==0){
	      error=1;
	      printf("Hubo un error en el servidor\n");
	      free(respuesta);
	    }else{
	      //  free(respuesta);
	      // respuesta=NULL;
	      //recibe mapas
	      mapas = lee_mensaje(s_conec, PLAZO, &len_recibido);
	      //Enviamos peticion si el mensaje no esta vacio
	      if (len_recibido > 0&&NULL!=mapas)
		{
		  mapas[len_recibido]='\0';
		  for(i=0;i<100;i++){
		    mEnemigo[i]=mapas[i+100];
		    mPropio[i]=mapas[i];
		  }
		  //free(mapas);
		  //mapas=NULL;

		  i=1;
		  system("clear");
		  while(i==1){
		    //imprime los dos mapas
		    imprime2tablero(mPropio,mEnemigo);
		    
		    //pide coordenada
		    printf("Introduce una coordenada de tiro\n");
		    fgets(linea,20,stdin);

		    linea[0]=toupper(linea[0]);
		    if(error_coord(linea)==0){
		      i=0;
		    }else{
		      system("clear");
		      printf("Coordenada erronea\n");
		    }
		  }
		  //envia coordenada
		  len=2;
		  linea[len]='\0';
		  envia_mensaje(s_conec, linea, len, PLAZO);

		  //MIRAR AQUI ERROR
		  //recibe mapas y almacena
		  mapas = lee_mensaje(s_conec, PLAZO, &len_recibido);
		  if (NULL != mapas && len_recibido>0)
		    {
		      mapas[len_recibido]='\0';
		      for(i=0;i<100;i++){
			mEnemigo[i]=mapas[i+100];
			mPropio[i]=mapas[i];
		      }
		      mensaje=(char *)calloc(len_recibido,sizeof(char));
		      for(i=200;i<len_recibido;i++){
			mensaje[i-200]=mapas[i];
		      }

		      system("clear");
		      //Imprimimos los dos mapas
		      imprime2tablero(mPropio,mEnemigo);		      
		      printf("mensaje: %s\n",mensaje);
		      if(strcmp(mensaje,"Tocado")!=0||strcmp(mensaje,"Hundido")!=0)
			printf("Esperando turno\n");
		      free(mensaje);
		      mensaje=NULL;
		      continuar=TRUE;  //seguimos
		      free(mapas);
		      mapas=NULL;
		    }
		  else
		    printf("Respuesta incorrecta\n");
		}
	    }
	  }

	if (close(s_cliente) < 0)     //Cierra el socket
	  error_fatal("close");
	
	free(puerto);
	free(tablero);
	tablero=NULL;
      }
    }
  return error;
}
