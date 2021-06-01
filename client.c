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

void connexion(int *fdSocket);
int testQuitter(char tampon[]);
void input(char tampon[]);
int niveau(char tampon[]);
void deplacement(char tampon[]);
void affichageMatrice(int coordonnees[][2], int * nbmechants, int * score);
void resultat(char tampon[], int *score);



int main(int argc , char const *argv[]) {

  char tampon[MAX_BUFFER];
  
  int fdSocket;
  int nbRecu;
  int i = 0;
  int lvl;
  int nbmechants = 2;
  int score = 0;

  bool reset = true;
  bool perdu = false;

  connexion(&fdSocket);   
  
  nbRecu = recv(fdSocket, tampon, MAX_BUFFER, 0);
  if (nbRecu > 0) {
    tampon[nbRecu] = 0;
    printf("%s\n", tampon);
    testQuitter(tampon);
  }
      
  input(tampon);
  send(fdSocket, tampon, strlen(tampon), 0);

  while (i!=3){
    nbRecu = recv(fdSocket, tampon, MAX_BUFFER, 0);
    if (nbRecu > 0) {
      tampon[nbRecu] = 0;
      printf("%s\n", tampon);

      testQuitter(tampon);
    }
    i++;
  }
  sleep(1);

  lvl = niveau(tampon);
  send(fdSocket, tampon, strlen(tampon), 0);

  if ((lvl == 3 || lvl == 5)){
    nbmechants = 3;
  }
  
  // Record
  nbRecu = recv(fdSocket, tampon, MAX_BUFFER, 0);
  if (nbRecu > 0) {
    tampon[nbRecu] = 0;
    printf("%s\n", tampon);
    testQuitter(tampon);
    }

  // Start
  nbRecu = recv(fdSocket, tampon, MAX_BUFFER, 0);
  if (nbRecu > 0) {
    tampon[nbRecu] = 0;
    printf("%s\n", tampon);
    testQuitter(tampon);
  }
  
      
  int coordonnees[1 + nbmechants + NBETOILES][2];

    
  // Affichage grille
  for (i = 0; i < (1 + nbmechants + NBETOILES); i++) {
    coordonnees[i][0] = (recv(fdSocket, tampon, MAX_BUFFER, 0))-1;
    testQuitter(tampon);
    coordonnees[i][1] = (recv(fdSocket, tampon, MAX_BUFFER, 0))-1;
    testQuitter(tampon);
  }
    
  affichageMatrice(coordonnees, &nbmechants, &score);
  
  do{
    // RÃ©ception demande dÃ©placement
    nbRecu = recv(fdSocket, tampon, MAX_BUFFER, 0);
    if (nbRecu > 0) {
      tampon[nbRecu] = 0;
      printf("%s\n", tampon);
      testQuitter(tampon);
    }

    deplacement(tampon);
    send(fdSocket, tampon, strlen(tampon), 0);
    
    // Reception rÃ©ponse

    nbRecu = recv(fdSocket, tampon, MAX_BUFFER, 0);

    if (nbRecu > 0) {
      tampon[nbRecu] = 0;
      testQuitter(tampon);
    }

    // Analyse rÃ©ponse
    
    if (strcmp(tampon,"0") != 0){
      printf("%s\n", tampon);
      perdu = true;
    }

    
    // Affichage grille
    for (i = 0; i < (1 + nbmechants); i++) {
      coordonnees[i][0] = (recv(fdSocket, tampon, MAX_BUFFER, 0))-1;
      testQuitter(tampon);
      coordonnees[i][1] = (recv(fdSocket, tampon, MAX_BUFFER, 0))-1;
      testQuitter(tampon);
    }
    
    affichageMatrice(coordonnees, &nbmechants, &score);
    printf("\n               Votre score est : %d\n", score);

    resultat(tampon, &score);
    send(fdSocket, tampon, strlen(tampon), 0);
  } while ((perdu == false) && (score != 20));

  usleep(5000);

  char a[2];
  sprintf(a, "%d", score);
  snprintf(tampon, MAX_BUFFER,"%s\n",a);
  send(fdSocket, tampon, strlen(tampon), 0);

  
  // RÃ©cup rÃ©sultat
  nbRecu = recv(fdSocket, tampon, MAX_BUFFER, 0);
  if (nbRecu > 0) {
    tampon[nbRecu] = 0;
    printf("%s\n", tampon);

    testQuitter(tampon);
  }
	
  close(fdSocket);
  
  printf("Fin du programme.\n");
  return EXIT_SUCCESS;
}




void connexion(int *fdSocket){
  struct sockaddr_in coordonneesServeur;
  int longueurAdresse;

  *fdSocket = socket(AF_INET, SOCK_STREAM, 0);

  if (*fdSocket < 0) {
    printf("socket incorrecte\n");
    exit(EXIT_FAILURE);
  }

  longueurAdresse = sizeof(struct sockaddr_in);
  memset(&coordonneesServeur, 0x00, longueurAdresse);

  coordonneesServeur.sin_family = PF_INET;
  inet_aton("127.0.0.1", &coordonneesServeur.sin_addr);
  coordonneesServeur.sin_port = htons(PORT);

  if (connect(*fdSocket, (struct sockaddr *) &coordonneesServeur, sizeof(coordonneesServeur)) == -1) {
    printf("connexion impossible\n");
    exit(EXIT_FAILURE);
  }

  printf("La connexion est etablie.\n");
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
  bool reset = true;
  int lvl;
  do{
    printf("     Saisir le niveau de difficulte : ");
    fgets(tampon, MAX_BUFFER, stdin);
    strtok(tampon, "\n");
    lvl = atoi(tampon);
    if ((lvl >= 1) && (lvl <= 5)){
      reset = false;
    } else {
      printf("     Choisir parmi les niveaux de difficulte !\n\n");
    }
    testQuitter(tampon);
  } while (reset == true);
  return lvl;
}



void deplacement(char tampon[]){
  bool reset = true;
  do{
    printf("     ");
    fgets(tampon, MAX_BUFFER, stdin);
    strtok(tampon, "\n");
    if ((strcmp(tampon,"z") == 0) || (strcmp(tampon,"q") == 0) || (strcmp(tampon,"s") == 0) || (strcmp(tampon,"d") == 0)){
      reset = false;
    } else {
      printf("     Saisie incorrecte\n     Ou veux-tu te deplacer ?");
    }
    testQuitter(tampon);
  } while (reset == true);
}



void affichageMatrice(int coordonnees[][2], int * nbmechants, int * score) {
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
	    // Pacman eat
	    printf("\b\b\b\b| C ");
	    coordonnees[k][0] = 100;
	    coordonnees[k][1] = 100;
	    *score += 1;
	  } else {
	    printf("\b\b\b\b| * ");
	  }
	}
      }
      for (int k=1; k<(*nbmechants+1); k++){
        if((i == coordonnees[k][0]) && (j == coordonnees[k][1])){
          printf("\b\b\b\b| X ");
          if((i != 1) && (coordonnees[k][0] == coordonnees[k-1][0]) && (coordonnees[k][1] == coordonnees[k-1][1])){
              printf("\b\b\b\b|X X");
            }
          if((coordonnees[0][0] == coordonnees[k][0]) && (coordonnees[0][1] == coordonnees[k][1])){
            printf("\b\b\b\b|XXX");
          }
        }
      }
    }
    printf("|\n");
  }
}

void resultat(char tampon[], int *score){
  if(*score == 20){
    snprintf(tampon, MAX_BUFFER,"0");
  } else {
    snprintf(tampon, MAX_BUFFER,"1");
  }
}
