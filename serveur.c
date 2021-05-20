#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <stdbool.h>

#define PORT 6000
#define MAX_BUFFER 1000
#define MAX_CLIENTS 3
#define NBETOILES 20
#define NBLIGNES 6
#define NBCOLONNES 10

struct personnage{
  int x;
  int y;
};

const char *EXIT = "exit";

int testQuitter(char tampon[]);
void pseudo(char buffer[]);
void introduction1(char tampon[],char * pseudo);
void introduction2(char tampon[]);
void introduction3(char tampon[]);
void introduction4(char tampon[]);
void introduction5(char tampon[]);
void intro_lvl(char tampon[]);
void coord(int coordonnees[][2], int * nbmechants);
void deplacement(char tampon[]);
void pacman(char tampon[], int coordonnees[][2], bool *perdu);
void fantome(int coordonnees[][2], bool *perdu, int *nbmechants, int * score);
void send_score(char tampon[], int *score);
void lose(char tampon[]);
void resultat(bool *perdu, double temps, int *lvl, int* score);



int main(int argc, char const *argv[]) {
  int fdSocketAttente;
  int fdSocketCommunication;
  struct sockaddr_in coordonneesServeur;
  struct sockaddr_in coordonneesAppelant;
  char tampon[MAX_BUFFER];
  int nbRecu;
  int longueurAdresse;
  int pid;

  fdSocketAttente = socket(PF_INET, SOCK_STREAM, 0);

  if (fdSocketAttente < 0) {
    printf("socket incorrecte\n");
    exit(EXIT_FAILURE);
  }

  // On prépare l’adresse d’attachement locale
  longueurAdresse = sizeof(struct sockaddr_in);
  memset(&coordonneesServeur, 0x00, longueurAdresse);

  coordonneesServeur.sin_family = PF_INET;
  // toutes les interfaces locales disponibles
  coordonneesServeur.sin_addr.s_addr = htonl(INADDR_ANY);
  // toutes les interfaces locales disponibles
  coordonneesServeur.sin_port = htons(PORT);

  if (bind(fdSocketAttente, (struct sockaddr *) &coordonneesServeur, sizeof(coordonneesServeur)) == -1) {
    printf("erreur de bind\n");
    exit(EXIT_FAILURE);
  }

  if (listen(fdSocketAttente, 5) == -1) {
    printf("erreur de listen\n");
    exit(EXIT_FAILURE);
  }

  socklen_t tailleCoord = sizeof(coordonneesAppelant);

  int nbClients = 0;

  while (nbClients < MAX_CLIENTS) {
    if ((fdSocketCommunication = accept(fdSocketAttente, (struct sockaddr *) &coordonneesAppelant,
					&tailleCoord)) == -1) {
      printf("erreur de accept\n");
      exit(EXIT_FAILURE);
    }

    printf("Client connecté - %s:%d\n",
	   inet_ntoa(coordonneesAppelant.sin_addr),
	   ntohs(coordonneesAppelant.sin_port));

    if ((pid = fork()) == 0) {
      close(fdSocketAttente);

      // Initialisation

      int i = 0;
      bool reset = true;
      int nbmechants;
      char press;
      char name[25];
      int score = 0;
      int lvl = 0;
      bool perdu = false;
      bool hard;
      clock_t start, end, middle;
      double temps;
      srand(time(NULL));


      // Introduction
      /* pseudo(tampon);
      send(fdSocketCommunication, tampon, strlen(tampon), 0);
	    
      nbRecu = recv(fdSocketCommunication, tampon, MAX_BUFFER, 0);	    
      if (nbRecu > 0) {
	tampon[nbRecu] = 0;
	printf("Recu de %s:%d : %s\n",
	       inet_ntoa(coordonneesAppelant.sin_addr),
	       ntohs(coordonneesAppelant.sin_port),
	       tampon);
	snprintf(name,MAX_BUFFER,tampon);

	if (testQuitter(tampon)) {
	  break; // on quitte la boucle
	}
      }
	    
      introduction1(tampon, name);
      send(fdSocketCommunication, tampon, strlen(tampon), 0);

      introduction2(tampon);
      send(fdSocketCommunication, tampon, strlen(tampon), 0);
	    
      introduction3(tampon);
      send(fdSocketCommunication, tampon, strlen(tampon), 0);*/
            
            
      intro_lvl(tampon);
      send(fdSocketCommunication, tampon, strlen(tampon), 0);
	      
      do {
	nbRecu = recv(fdSocketCommunication, tampon, MAX_BUFFER, 0);
	if (nbRecu > 0) {
	  tampon[nbRecu] = 0;
	  printf("Recu de %s:%d : %s\n",
		 inet_ntoa(coordonneesAppelant.sin_addr),
		 ntohs(coordonneesAppelant.sin_port),
		 tampon);
	  lvl = atoi(tampon);

	  if (testQuitter(tampon)) {
	    break; // on quitte la boucle
	  }
	}
	if ((lvl == 1) || (lvl == 2) || (lvl == 3)){
	  reset = false;
	} else {
	  introduction4(tampon);
	  send(fdSocketCommunication, tampon, strlen(tampon), 0);
	}
      } while (reset == true);
      
      introduction5(tampon);
      send(fdSocketCommunication, tampon, strlen(tampon), 0);

      if (lvl != 3){
        hard = false;
	nbmechants = 2;
      } else {
        hard = true;
        nbmechants = 3;
      }

      int coordonnees[NBETOILES + nbmechants][2];
      
      if (coordonnees == NULL) {
	printf("L'allocation tableau des coordonnees a echouee!\n");
      }

      coord(coordonnees, &nbmechants);

      // Début du jeu

      start = clock();

      do{
	// Envoi score
	send_score(tampon, &score);
	send(fdSocketCommunication, tampon, strlen(tampon), 0);
	
	// Envoi grille
	for (i = 0; i < (1 + nbmechants + NBETOILES); i++) {
	  usleep(10);
	  (send(fdSocketCommunication, coordonnees[i], coordonnees[i][0]+1, 0));
	}
	for (i = 0; i < (1 + nbmechants + NBETOILES); i++) {
	  usleep(10);
	  (send(fdSocketCommunication, coordonnees[i], coordonnees[i][1]+1, 0));
	}

	// Demande déplacement
      
	deplacement(tampon);
	send(fdSocketCommunication, tampon, strlen(tampon), 0);

	// Reception déplacement
	nbRecu = recv(fdSocketCommunication, tampon, MAX_BUFFER, 0);

	if (nbRecu > 0) {
	  tampon[nbRecu] = 0;
	  printf("Recu de %s:%d : %s\n",
		 inet_ntoa(coordonneesAppelant.sin_addr),
		 ntohs(coordonneesAppelant.sin_port),
		 tampon);
	  testQuitter(tampon);
	}
	
	pacman(tampon, coordonnees, &perdu);

	for (int i=1; i<= nbmechants; i++){
	  if ((coordonnees[0][0] == coordonnees[i][0]) && (coordonnees[0][1] == coordonnees[i][1])){
	    perdu = true;
	  }
	}
	
	if (perdu == false){
	  fantome(coordonnees, &perdu, &nbmechants, &score);
	  
	  if (perdu == false){
	    usleep(10);
	    snprintf(tampon, MAX_BUFFER,"suite");
	    send(fdSocketCommunication, tampon, strlen(tampon), 0);
	  } else {
	    usleep(10);
	    lose(tampon);
	    send(fdSocketCommunication, tampon, strlen(tampon), 0);
	  }
        } else if (perdu == true){
	  // Sorti de grille
	  usleep(10);
	  lose(tampon);
	  send(fdSocketCommunication, tampon, strlen(tampon), 0);
	}
	
	temps = ((double)middle - start) / CLOCKS_PER_SEC;
	if (temps > 60){
	  perdu = true;
	  usleep(10);
	  snprintf(tampon, MAX_BUFFER, "\n     Le temps est ecoule ! ");
	  send(fdSocketCommunication, tampon, strlen(tampon), 0);
        }
	
      } while ((perdu == false) && (score != 20));
      
      end = clock();
      temps = ((double)end - start) / CLOCKS_PER_SEC;

      // Envoi grille finale
      for (i = 0; i < (1 + nbmechants + NBETOILES); i++) {
	usleep(10);
	(send(fdSocketCommunication, coordonnees[i], coordonnees[i][0]+1, 0));
      }
      for (i = 0; i < (1 + nbmechants + NBETOILES); i++) {
	usleep(10);
	(send(fdSocketCommunication, coordonnees[i], coordonnees[i][1]+1, 0));
      }

      if (perdu == true){
	//send(fdSocketCommunication, tampon, strlen(tampon), 0);
      } else {
	printf("fin du game bb ");
      }

      //resultat(perdu, temps, lvl, score);


      exit(EXIT_SUCCESS);
    }

    nbClients++;
  }

  close(fdSocketCommunication);
  close(fdSocketAttente);

  for (int i = 0; i < MAX_CLIENTS; i++) {
    wait(NULL);
  }

  printf("Fin du programme.\n");
  return EXIT_SUCCESS;
}





int testQuitter(char tampon[]) {
  return strcmp(tampon, EXIT) == 0;
}

void pseudo(char buffer[]){
  usleep(10);
  snprintf(buffer, MAX_BUFFER, "\n     Entrer votre pseudo : ");
}

void introduction1(char tampon[],char * pseudo){
  usleep(500000);
  snprintf(tampon, MAX_BUFFER, "     Bienvenue %s dans le jeu du pacman ! \n", pseudo);
}

void introduction2(char tampon[]){
  sleep(1);
  snprintf(tampon, MAX_BUFFER, "     L'objectif est de recuperer 20 etoiles en 1 minute\n     Vous deplacerez votre personnage avec les touches ZQSD\n");
}

void introduction3(char tampon[]){
  sleep(2);
  snprintf(tampon, MAX_BUFFER, "\n     Niveau 1 : Deux fantomes, pas de perte de points\n     Niveau 2 : Deux fantomes, perte de points / 15 secondes\n     Niveau 3 : Trois fantomes, perte de points / 15 secondes\n");
}

void intro_lvl(char tampon[]){
 usleep(500000);
  snprintf(tampon, MAX_BUFFER,"\n     Saisir le niveau de difficulte : ");
}

void introduction4(char tampon[]){
  sleep(1);
  snprintf(tampon, MAX_BUFFER,"\n     Choisir parmi les niveaux de difficulte !\n     Saisir le niveau de difficulte :");
}

void introduction5(char tampon[]){
  usleep(10);
  snprintf(tampon, MAX_BUFFER,"     Le chronometre debute a l'affichage du jeu\n     Que le meilleur gagne!\n");
}

void coord(int coordonnees[][2], int * nbmechants){
  struct personnage perso[NBETOILES + *nbmechants + 1];
  for (int i=0; i<(NBETOILES + *nbmechants + 1); i++){
    if (i != 0){
      perso[i].x = rand()%(NBLIGNES);
      perso[i].y = rand()%(NBCOLONNES);
    } else {
      perso[i].x = 4;
      perso[i].y = 5;
    }
    for (int j=0; j<i; j++){
      if (i != 0){
	if (((coordonnees[j][0] == perso[i].x) && (coordonnees[j][1] == perso[i].y)) || ((perso[i].x == coordonnees[0][0]) || (perso[i].y == coordonnees[0][1]))){
	  i--;
	}
      }
    }
    coordonnees[i][0] = perso[i].x;
    coordonnees[i][1] = perso[i].y;
  }
}

void deplacement(char tampon[]){
  usleep(10);
  snprintf(tampon, MAX_BUFFER,"\n     Ou veux-tu te deplacer ?");
}

void pacman(char tampon[], int coordonnees[][2], bool *perdu){
  if (strcmp(tampon,"q") == 0){
    coordonnees[0][1] -= 1;
  } else if (strcmp(tampon,"d") == 0){
    coordonnees[0][1] += 1;
  } else if (strcmp(tampon,"z") == 0){
    coordonnees[0][0] -= 1;
  } else if (strcmp(tampon,"s") == 0){
    coordonnees[0][0] += 1;
  }
  
  if ((coordonnees[0][1] < 0) || (coordonnees[0][1] >= NBCOLONNES) || (coordonnees[0][0] < 0) || (coordonnees[0][0] >= NBLIGNES)){
    usleep(10);
    snprintf(tampon, MAX_BUFFER,"\n     Vous etes sorti de la grille\n");
    *perdu = true;
  }
}

void fantome(int coordonnees[][2], bool *perdu, int *nbmechants, int * score){
  bool reset;  
  printf("pacman (%d,%d)\n",coordonnees[0][0],coordonnees[0][1]);
  for (int i=1; i<(*nbmechants+1);i++){
    do{
      reset = false;
      //? abscisse ou ordonnée
      int move = rand()%2;
      int last = coordonnees[i][move];
      if (coordonnees[i][move] > coordonnees[0][move]){
	coordonnees[i][move] -= 1;
      } else if (coordonnees[i][move] < coordonnees[0][move]){
	coordonnees[i][move] += 1;
      } else {
	if (move == 0){
	  if (coordonnees[i][1] > coordonnees[0][1]){
	    coordonnees[i][1] -= 1;
	  } else {
	    coordonnees[i][1] += 1;
	  }
	} else {
	  if (coordonnees[i][0] > coordonnees[0][0]){
	    coordonnees[i][0] -= 1;
	  } else {
	    coordonnees[i][0] += 1;
	  }
	}
      }
      if ((i > 1) && (i <= *nbmechants)){
	if((coordonnees[i][0] == coordonnees[i-1][0]) && (coordonnees[i][1] == coordonnees[i-1][1])){
	  printf("fant[%d] (%d,%d) = fant[%d] (%d,%d)\n",i, coordonnees[i][0],coordonnees[i][1], i-1,coordonnees[i-1][0],coordonnees[i-1][1]);
	  coordonnees[i][move] = last;
	  reset = true;
	}
      }
      // printf("fant[%d] = (%d,%d)\n",i,coordonnees[i][0],coordonnees[i][1]);
    } while (reset == true);
  }

  // Mechant kill Pacman
  for (int i=+1; i<(*nbmechants+1);i++){
    if ((coordonnees[i][0] == coordonnees[0][0]) && (coordonnees[i][1] == coordonnees[0][1])){
      *perdu = true;
      printf("pacman[%d] (%d,%d) = fant[%d] (%d,%d)\n",i, coordonnees[0][0],coordonnees[0][1], i,coordonnees[i][0],coordonnees[i][1]);
    }
  }

  // Pacman eat star
  for (int i = (*nbmechants+1); i < (1+(*nbmechants)+NBETOILES) ;i++){
    if ((coordonnees[i][0] == coordonnees[0][0]) && (coordonnees[i][1] == coordonnees[0][1])){
      //printf("\n       Pacman mange une etoile !\n");
      coordonnees[i][0] = 100;
      coordonnees[i][1] = 100;
      *score += 1;
    }
  }
}

void send_score(char tampon[], int *score){
  usleep(10);
  char a[2];
  sprintf(a, "%d", *score);
  snprintf(tampon, MAX_BUFFER,"\n           Votre score est : %s\n",a);
}

void lose(char tampon[]){
  usleep(10);
  snprintf(tampon, MAX_BUFFER,"                  Game over\n");
}
/*
void resultat(bool *perdu, double temps, int *lvl, int* score){
    printf("\n\n       Votre temps est %.1f secondes.\n", temps);
    if (*lvl != 1){
        for (int i=0;i<20;i++){
            if (temps > 15){
                temps -= 15;
                *score -=1;
            } else {
                break;
            }
        }
        if (*score < 0){
            *score = 0;
        }
        printf("       Votre score final est : %d\n", *score);
    }

    if (*perdu == true){
      usleep(10);
      snprintf(tampon, MAX_BUFFER,"           Dommage, vous avez perdu !");
    } else {
      usleep(10);
      snprintf(tampon, MAX_BUFFER,"\n   (|___/)      Felicitations       (|___/)\n   (='.'=)        -   -   -         (='.'=)\n  ('')_('')    Vous avez gagne !   ('')_('')\n\n");
    sleep(5);
}
*/
