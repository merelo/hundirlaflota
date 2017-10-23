#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "funcCliente.h"

/*
  Funcion mete_datos: pide al usuario las coordenadas, verifica que sean correctas y las introduce en el tablero

  Recibe:
         tablero: puntero a la direccion del tablero
  Devuelve:
	 nada
 */
void mete_datos(char *tablero){
  int nBarcoA=1;
  int lBarcoA=1;
  char barcoA='A';
  
  int nBarcoB=1;
  int lBarcoB=2;
  char barcoB='B';

  int nBarcoC=1;
  int lBarcoC=3;
  char barcoC='C';
  
  int nBarcoD=1;
  int lBarcoD=4;
  char barcoD='D';

  int nBarcoE=1;
  int lBarcoE=5;
  char barcoE='E';

  int error=1;
  char cad[100];


  //pide datos para el barco E, tamaño 5
  while(nBarcoE>0){
    while(error==1){
      imprimeTablero(tablero);
      printf("Introduzca barco de tamaño 5, con el formato: \"Coordenada1 Coordenada2 Coordenada3 Coordenada4 Coordenada5\"\n");
      fgets(cad,100,stdin);
      
      system("clear");
      if(strlen(cad)!=0)
	error=compruebaBarco(tablero,cad,lBarcoE,barcoE);
      else
	printf("Por favor, introduzca alguna coordenada\n");
    }
    error=1;
    nBarcoE--;
  }

  //pide datos para el barco D, tamaño 4
  while(nBarcoD>0){
    while(error==1){
      imprimeTablero(tablero);
      printf("Introduzca barco de tamaño 4, con el formato: \"Coordenada1 Coordenada2 Coordenada3 Coordenada4\"\n");
      fgets(cad,100,stdin);
      
      system("clear");
      if(strlen(cad)!=0)
	error=compruebaBarco(tablero,cad,lBarcoD,barcoD);
      else
	printf("Por favor, introduzca alguna coordenada\n");
    }
    error=1;
    nBarcoD--;
  }

  //pide datos para el barco C, tamaño 3
  while(nBarcoC>0){
    while(error==1){
      imprimeTablero(tablero);
      printf("Introduzca barco de tamaño 3, con el formato: \"Coordenada1 Coordenada2 Coordenada3\"\n");
      fgets(cad,100,stdin);
      
      system("clear");
      if(strlen(cad)!=0)
	error=compruebaBarco(tablero,cad,lBarcoC,barcoC);
      else
	printf("Por favor, introduzca alguna coordenada\n");
    }
    error=1;
    nBarcoC--;
  }

  //pide datos para el barco B, tamaño 2
  while(nBarcoB>0){
    while(error==1){
      imprimeTablero(tablero);
      printf("Introduzca barco de tamaño 2, con el formato: \"Coordenada1 Coordenada2\"\n");
      fgets(cad,100,stdin);

      system("clear");
      if(strlen(cad)!=0)
	error=compruebaBarco(tablero,cad,lBarcoB,barcoB);
      else
	printf("Por favor, introduzca alguna coordenada\n");
    }
    error=1;
    nBarcoB--;
  }
  
  //pide datos para el barco A, tamaño 1
  while(nBarcoA>0){
    while(error==1){
      imprimeTablero(tablero);
      printf("Introduzca barco de tamaño 1, con el formato: \"Coordenada1\"\n");
      fgets(cad,100,stdin);        
      
      system("clear");
      if(strlen(cad)!=0)
	error=compruebaBarco(tablero,cad,lBarcoA,barcoA);
      else
	printf("Por favor, introduzca alguna coordenada\n");
    }
    error=1;
    nBarcoA--;
  }
}


/*
  Funcion crearTablero: crea un tablero de LONG_LIN por LONG_COL en la direccion pasada

  Recibe:
         tablero: puntero a la direccion donde se almacenara el tablero
  Devuelve:
	 nada
 */
void crearTablero(char *tab){
  int i=0;
  int j=0;

  for(i=0;i<LONG_LIN;i++){
    for(j=0;j<LONG_COL;j++){
      tab[i*LONG_LIN+j]='.';
    }
  }
}

/*
  Funcion compruebaBarco: comprueba que las coordenadas esten correctas y las añade tablero

  Recibe:
         tablero: puntero al tablero
	 cad: cadena de posiciones que da el jugador
	 lBarco: longitud del barco
	 barco: caracter identificador del barco
  Devuelve:
	 0 si las coordenadas estan correctas
	 1 si las coordenadas son erroneas
 */
int compruebaBarco(char *tablero,char *cad,int lBarco,char barco){
  int error=0;
  char *token=NULL;
  char *c1=NULL;
  char *c2=NULL;
  char *c3=NULL;
  char *c4=NULL;
  char *c5=NULL;
  int largAux=lBarco;
  int i=0;
  char **coord[5]={&c1,&c2,&c3,&c4,&c5};

  //separamos la cadena en trozos, largAux evita que sigamos si hay mas de 5 coordenadas y genere core
  token=strtok(cad," ");
  while(token!=NULL&&largAux>0){
    *coord[i]=(char*)malloc(2*sizeof(char));
    strcpy(*coord[i],token);
    i++;
    token = strtok(NULL, " ");
    largAux--;
  }

  if(largAux!=0){
    error=1;
    printf("El numero de coordenadas necesarias son %d\n\n",lBarco);
  }else{
    for(i=0;i<lBarco&&error==0;i++){
      if((*coord[i])[1]>'9'||(*coord[i])[1]<'0'){
	error=1;
      }else{
	//traducimos la coordenada primera a numero
	switch(tolower((*coord[i])[0])){
	case 'a':
	case 'b': 
	case 'c':
	case 'd':
	case 'e':
	case 'f':
	case 'g':
	case 'h':
	case 'i':
	case 'j':
	  (*coord[i])[0]=tolower((*coord[i])[0])-49;
	  break;
	default:
	  error=1;
	  break;
	}
      }
    }

    
    if(error!=1&&lBarco>1){
      //comprobamos que las coordenadas esten ordenadas
      error=ordenados(coord,lBarco);
      
      if(error==1){
	printf("Error, las coordenadas deben ir consecutivas\n\n");
      }else{
	if((error=posiciones_ocupadas(tablero,coord,lBarco))==1){
	  printf("Ha seleccionado posiciones que ya están ocupadas por otros barcos\n\n");
	}else{
	  for(i=0;i<lBarco;i++)
	    rellenar_mapa(tablero,(int)toupper((*coord[i])[0]-48),(int)toupper((*coord[i])[1]-48),barco);
	}
      }
    }else if(error==1){
      printf("Error, las coordenadas son A-J y 0-9\n\n");
    }else{
      if((error=posiciones_ocupadas(tablero,coord,lBarco))==1){
	printf("Ha seleccionado posiciones que ya están ocupadas por otros barcos\n\n");
      }else{
	rellenar_mapa(tablero,(int)toupper((*coord[0])[0]-48),(int)toupper((*coord[0])[1]-48),barco);
      }
    }
  }

  //liberamos memoria
  free(c1);
  c1=NULL;
  if(lBarco>1){
    free(c2);
    c2=NULL;
  }
  if(lBarco>2){
    free(c3);
    c3=NULL;
  }
  if(lBarco>3){
    free(c4);
    c4=NULL;
  }
  if(lBarco==5){
    free(c5);
    c5=NULL;
  }

  return error;
}

/*
  Funcion rellenar_mapa: añade posicion al tablero

  Recibe:
         tablero: puntero al tablero
	 x: posicion x
	 y: posicion y
	 barco: caracter que se imprimira en el tablero
  Devuelve:
	 nada
 */
void rellenar_mapa(char *tablero,int x,int y, char barco){
  tablero[x*LONG_LIN+y]=barco;
}


/*
  Funcion posiciones_ocupadas: comprueba que las coordenadas no esten ya ocupadas

  Recibe:
         tablero: puntero al tablero
  Devuelve:
	 0 si las posiciones estan libres
	 1 si alguna posicion esta ocupada
 */
int posiciones_ocupadas(char *tablero,char **coordenada[5],int lBarco){
  int i=0;
  int x=0;
  int y=0;
  int error=0;

  for(i=0;i<lBarco&&error==0;i++){
    x=(int)((*coordenada[i])[0]-48);
    y=(int)((*coordenada[i])[1]-48);
    if(tablero[x*LONG_LIN+y]=='A'||tablero[x*LONG_LIN+y]=='B'||tablero[x*LONG_LIN+y]=='C'||tablero[x*LONG_LIN+y]=='D'){
      error=1;
    }
  }

  return error;
}


/*
  Funcion imprimeTablero: imprime el tablero que se le pasa por parametro

  Recibe:
         tab: puntero al tablero
  Devuelve:
	 nada
 */
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

/*
  Funcion ordenados: comprueba que las coordenadas que se quieren introducir sean consecutivas

  Recibe: 
         coordenada: Tabla de punteros a cadena de tamaño 5
	 lBarco: longitud del barco
  Devuelve:
         0 si las coordenadas son consecutivas
	 1 si las coordenadas no son consecutivas
 */
int ordenados(char **coordenada[5],int lBarco){
  int i=0;
  int j=0;
  char *temp=NULL;
  int error=0;
  int opcion=0;


  //comprobamos que una de las coordenadas tenga el mismo valor en todas las posiciones
  switch(lBarco){
  case 5:
    if((*coordenada[0])[0]==(*coordenada[1])[0]&&(*coordenada[2])[0]==(*coordenada[1])[0]&&(*coordenada[2])[0]==(*coordenada[3])[0]&&(*coordenada[3])[0]==(*coordenada[4])[0])
      opcion=1;
    else if((*coordenada[0])[1]==(*coordenada[1])[1]&&(*coordenada[2])[1]==(*coordenada[1])[1]&&(*coordenada[2])[1]==(*coordenada[3])[1]&&(*coordenada[3])[1]==(*coordenada[4])[1])
      opcion=2;
    else
      error=1;
    break;
  case 4:
    if((*coordenada[0])[0]==(*coordenada[1])[0]&&(*coordenada[2])[0]==(*coordenada[1])[0]&&(*coordenada[2])[0]==(*coordenada[3])[0])
      opcion=1;
    else if((*coordenada[0])[1]==(*coordenada[1])[1]&&(*coordenada[2])[1]==(*coordenada[1])[1]&&(*coordenada[2])[1]==(*coordenada[3])[1])
      opcion=2;
    else
      error=1;
    break;
  case 3:
    if((*coordenada[0])[0]==(*coordenada[1])[0]&&(*coordenada[2])[0]==(*coordenada[1])[0])
      opcion=1;
    else if((*coordenada[0])[1]==(*coordenada[1])[1]&&(*coordenada[2])[1]==(*coordenada[1])[1])
      opcion=2;
    else
      error=1;
    break;
  case 2:
    if((*coordenada[0])[0]==(*coordenada[1])[0])
      opcion=1;
    else if((*coordenada[0])[1]==(*coordenada[1])[1])
      opcion=2;
    else
      error=1;
    break;
  }


  //si una de las coordenadas es igual en todas las posiciones, continuamos
  if(error==0){

    //ordenamos las posiciones, ordenamos segun la coordenada que es distinta
    if(opcion==1){
      for (i=1; i<lBarco; i++){
	for (j=0 ; j<lBarco-1; j++){
	  if ((*coordenada[j])[1] > (*coordenada[j+1])[1]){
	    temp = (*coordenada[j]);
	    (*coordenada[j]) = (*coordenada[j+1]);
	    (*coordenada[j+1]) = temp;
	  }
	}
      }
    }else if(opcion==2){
      for (i=1; i<lBarco; i++){
	for (j=0 ; j<lBarco-1; j++){
	  if ((*coordenada[j])[0] > (*coordenada[j+1])[0]){
	    temp = (*coordenada[j]);
	    (*coordenada[j]) = (*coordenada[j+1]);
	    (*coordenada[j+1]) = temp;
	  }
	}
      }
    }
    

    //comprobamos que las coordenadas sean consecutivas
    if(lBarco==5){
      if((abs((*coordenada[0])[0]-(*coordenada[1])[0])+abs((*coordenada[1])[0]-(*coordenada[2])[0])+abs((*coordenada[2])[0]-(*coordenada[3])[0])+abs((*coordenada[3])[0]-(*coordenada[4])[0])+abs((*coordenada[0])[1]-(*coordenada[1])[1])+abs((*coordenada[1])[1]-(*coordenada[2])[1])+abs((*coordenada[2])[1]-(*coordenada[3])[1])+abs((*coordenada[3])[1]-(*coordenada[4])[1]))!=4)
	error=1;
    }else if(lBarco==4){
      if((abs((*coordenada[0])[0]-(*coordenada[1])[0])+abs((*coordenada[1])[0]-(*coordenada[2])[0])+abs((*coordenada[2])[0]-(*coordenada[3])[0])+abs((*coordenada[0])[1]-(*coordenada[1])[1])+abs((*coordenada[1])[1]-(*coordenada[2])[1])+abs((*coordenada[2])[1]-(*coordenada[3])[1]))!=3)
	error=1;
    }else if(lBarco==3){
      if((abs((*coordenada[0])[0]-(*coordenada[1])[0])+abs((*coordenada[1])[0]-(*coordenada[2])[0])+abs((*coordenada[0])[1]-(*coordenada[1])[1])+abs((*coordenada[1])[1]-(*coordenada[2])[1]))!=2)
	error=1;
    }else{
      if((abs((*coordenada[0])[0]-(*coordenada[1])[0])+abs((*coordenada[0])[1]-(*coordenada[1])[1]))!=1)
	error=1;
    }
  }
  return error;
}

int error_coord(char *cad){
  int error=0;
  if(cad[0]<'A'||cad[0]>'J'){
    error=1;
  }else if(cad[1]<'0'||cad[1]>'9'){
    error=1;
  }
  return error;
}


void imprime2tablero(char *mPropio, char *mEnemigo){
  int i=0;
  int j=0;
  char *cad="    0 1 2 3 4 5 6 7 8 9    \t    0 1 2 3 4 5 6 7 8 9    \n\n";
  printf("       MAPA PROPIO        \t       MAPA ENEMIGO       \n");
  printf("%s",cad);
  for(i=0;i<LONG_LIN;i++){
    printf("%c   ",i+'A');
    for(j=0;j<LONG_COL;j++){
      printf("%c ",*(mPropio+i*LONG_LIN+j));
    }
    printf("  %c\t%c   ",i+'A',i+'A');
    for(j=0;j<LONG_COL;j++){
      printf("%c ",*(mEnemigo+i*LONG_LIN+j));
    }
    printf("  %c\n",i+'A');
  }
  printf("\n\n%s",cad);
}
