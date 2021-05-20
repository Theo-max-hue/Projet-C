#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <stdbool.h>

#define PORT 6000
#define MAX_BUFFER 1000
#define NBETOILES 20
#define NBLIGNES 6
#define NBCOLONNES 10

const char *EXIT = "exit";

int testQuitter(char tampon[]);
void input(char tampon[]);
int niveau(char tampon[]);
void affichageMatrice(int coordonnees[][2], int * nbmechants);



int main(int argc , char const *argv[]) {
  int fdSocket;
  int nbRecu;
  struct sockaddr_in coordonneesServeur;
  int longueurAdresse;
  char tampon[MAX_BUFFER];

  fdSocket = socket(AF_INET, SOCK_STREAM, 0);

  if (fdSocket < 0) {
    printf("socket incorrecte\n");
    exit(EXIT_FAILURE);
  }

  // On prépare les coordonnées du serveur
  longueurAdresse = sizeof(struct sockaddr_in);
  memset(&coordonneesServeur, 0x00, longueurAdresse);

  coordonneesServeur.sin_family = PF_INET;
  // adresse du serveur
  inet_aton("127.0.0.1", &coordonneesServeur.sin_addr);
  // toutes les interfaces locales disponibles
  coordonneesServeur.sin_port = htons(PORT);

  if (connect(fdSocket, (struct sockaddr *) &coordonneesServeur, sizeof(coordonneesServeur)) == -1) {
    printf("connexion impossible\n");
    exit(EXIT_FAILURE);
  }

  printf("La connexion est etablie.\n");

  //while (1) {
  int i = 0;
  int lvl;
  int nbmechants = 2;
  int score = 0;
  bool reset = true;
  bool perdu = false;
  clock_t start, end, middle;
      
  /*
    nbRecu = recv(fdSocket, tampon, MAX_BUFFER, 0);

    if (nbRecu > 0) {
    tampon[nbRecu] = 0;
    printf("%s\n", tampon);

    if (testQuitter(tampon)) {
    // break; // on quitte la boucle
    }
    }
      
    input(tampon);
    send(fdSocket, tampon, strlen(tampon), 0);

    while (i!=4){
    nbRecu = recv(fdSocket, tampon, MAX_BUFFER, 0);
    if (nbRecu > 0) {
    tampon[nbRecu] = 0;
    printf("%s\n", tampon);

    if (testQuitter(tampon)) {
    //break; // on quitte la boucle
    }
    }
    i++;
    }*/


      
  // A DELETE
  nbRecu = recv(fdSocket, tampon, MAX_BUFFER, 0);

  if (nbRecu > 0) {
    tampon[nbRecu] = 0;
    printf("%s\n", tampon);

    if (testQuitter(tampon)) {
      // break; // on quitte la boucle
    }
  }
  // A DELETE

  do{
	  
    lvl = niveau(tampon);
	  
    if (lvl == 3){
      nbmechants = 3;
    }
	  
    send(fdSocket, tampon, strlen(tampon), 0);
    nbRecu = recv(fdSocket, tampon, MAX_BUFFER, 0);
	  
    if (nbRecu > 0) {
      tampon[nbRecu] = 0;
      if (testQuitter(tampon)) { }
    }
	  
    if (strcmp(tampon,"\n     Choisir parmi les niveaux de difficulte !\n     Saisir le niveau de difficulte :") != 0){
      reset = false;
    } else {
      printf("%s\n", tampon);
    }
  } while (reset != false);

  printf("%s\n", tampon);
      
  int coordonnees[1 + nbmechants + NBETOILES][2];
  
  do{
    // Affichage score
    nbRecu = recv(fdSocket, tampon, MAX_BUFFER, 0);

    if (nbRecu > 0) {
      tampon[nbRecu] = 0;
      printf("%s\n", tampon);

      if (testQuitter(tampon)) {
	// break; // on quitte la boucle
      }
    }
    
    // Affichage grille
    for (i = 0; i < (1 + nbmechants + NBETOILES); i++) {
      coordonnees[i][0] = (recv(fdSocket, tampon, MAX_BUFFER, 0))-1;
      testQuitter(tampon);
    }

    for (i = 0; i < (1 + nbmechants + NBETOILES); i++) {
      coordonnees[i][1] = (recv(fdSocket, tampon, MAX_BUFFER, 0))-1;
      testQuitter(tampon);
    }
    
    affichageMatrice(coordonnees, &nbmechants);

    // Réception demande déplacement
    nbRecu = recv(fdSocket, tampon, MAX_BUFFER, 0);
    if (nbRecu > 0) {
      tampon[nbRecu] = 0;
      printf("%s\n", tampon);
      testQuitter(tampon);
    }

    // Envoi déplacement
    do{
      reset = true;
      input(tampon);
      if ((strcmp(tampon,"z") == 0) || (strcmp(tampon,"q") == 0) || (strcmp(tampon,"s") == 0) || (strcmp(tampon,"d") == 0)){
	  reset = false;
	} else {
	printf("     Saisie incorrecte\n     Ou veux-tu te deplacer ?");
      }
      testQuitter(tampon);
    } while (reset == true);
	
    send(fdSocket, tampon, strlen(tampon), 0);
    
    // Reception réponse
    
    nbRecu = recv(fdSocket, tampon, MAX_BUFFER, 0);

    if (nbRecu > 0) {
      tampon[nbRecu] = 0;

      if (testQuitter(tampon)) {
	// break; // on quitte la boucle
      }
    }

    // Analyse réponse
    
    if ( strcmp(tampon,"suite") != 0){
      printf("%s\n", tampon);
      perdu = true;
    }

    
    
  } while ((perdu == false) && (score != 20));

  // Affichage grille finale
  for (i = 0; i < (1 + nbmechants + NBETOILES); i++) {
    coordonnees[i][0] = (recv(fdSocket, tampon, MAX_BUFFER, 0))-1;
    testQuitter(tampon);
  }

  for (i = 0; i < (1 + nbmechants + NBETOILES); i++) {
    coordonnees[i][1] = (recv(fdSocket, tampon, MAX_BUFFER, 0))-1;
    testQuitter(tampon);
  }
    
  affichageMatrice(coordonnees, &nbmechants);

  
      

  /*while ((perdu == false) && (score != 20)){

    nbRecu = recv(fdSocket, tampon, MAX_BUFFER, 0);
    if (nbRecu > 0) {
      tampon[nbRecu] = 0;
      printf("%s\n", tampon);
      testQuitter(tampon);
    }
	
    
    do{
      input(tampon);
      if (strlen(tampon) != 1 ){
	printf("     Saisie incorrecte\n     Ou veux-tu te deplacer ?");
	testQuitter(tampon);
      } 
      } while (strlen(tampon) != 1);
	
    send(fdSocket, tampon, strlen(tampon), 0);
    
    printf("avt recptn\n");
    nbRecu = recv(fdSocket, tampon, MAX_BUFFER, 0);
    printf("apr recptn\n");
    if (nbRecu > 0) {
      tampon[nbRecu] = 0;
      printf("%s\n", tampon);
      testQuitter(tampon);
    }
    printf("apr affi\n");
	
    }*/

  // } end_while(1)
	
  close(fdSocket);
  
  printf("Fin du programme.\n");
  return EXIT_SUCCESS;
}





void viderBuffer(){
  int c = 0;
  while (c != '\n' && c != EOF){
    c = getchar();
  }
}

int testQuitter(char tampon[]) {
  return strcmp(tampon, EXIT) == 0;
}

void input(char tampon[]){
  printf("     ");
  fgets(tampon, MAX_BUFFER, stdin);
  strtok(tampon, "\n");
}

int niveau(char tampon[]){
  printf("     ");
  fgets(tampon, MAX_BUFFER, stdin);
  strtok(tampon, "\n");
  return atoi(tampon);
}

void affichageMatrice(int coordonnees[][2], int * nbmechants) {
  for (int i = 0; i < NBLIGNES; i++) {
    printf("   ");
    for (int j = 0; j < NBCOLONNES; j++) {
      if ((i == coordonnees[0][0]) && (j == coordonnees[0][1])){
	printf("| C ");
      } else{
	printf("|   ");
      }
      for (int k=(*nbmechants+1); k<(*nbmechants+NBETOILES+1); k++){
	if((i == coordonnees[k][0]) && (j == coordonnees[k][1])){
	  if((coordonnees[0][0] == coordonnees[k][0]) && (coordonnees[0][1] == coordonnees[k][1])){
	    printf("\b\b\b\b| C ");
	  } else {
	    printf("\b\b\b\b| * ");
	  }
	}
      }
      for (int k=1; k<(*nbmechants+1); k++){
	if((i == coordonnees[k][0]) && (j == coordonnees[k][1])){
	  printf("\b\b\b\b| X ");
	  if((coordonnees[0][0] == coordonnees[k][0]) && (coordonnees[0][1] == coordonnees[k][1])){
	    printf("\b\b\b\b|XXX");
	  }
	}
      }
    }
    printf("|\n");
  }
}
