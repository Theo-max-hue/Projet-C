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
#define MAX_CLIENTS 5
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
void coord(int coordonnees[][2], int * nbmechants);
void deplacement(char tampon[]);
void pacman(char tampon[], int coordonnees[][2]);
void fantome(int coordonnees[][2], int *nbmechants);
void etat(char tampon[], double *temps, bool *perdu);
void continuer(char tampon[]);
void timer(char tampon[]);
void lose(char tampon[]);
void resultat(char tampon[], bool *perdu, int *lvl, int* score, double temps);
void record(char tampon[], FILE *file, int *lvl);
void move(char tampon[], int coordonnees[][2], int *nbmechants, int *lvl, bool *perdu);
void new_score(FILE *file,int *lvl, int *score, char *name);



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

  longueurAdresse = sizeof(struct sockaddr_in);
  memset(&coordonneesServeur, 0x00, longueurAdresse);

  coordonneesServeur.sin_family = PF_INET;
  coordonneesServeur.sin_addr.s_addr = htonl(INADDR_ANY);
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

    printf("Client connectÃ© - %s:%d\n",
	   inet_ntoa(coordonneesAppelant.sin_addr),
	   ntohs(coordonneesAppelant.sin_port));

    if ((pid = fork()) == 0) {
      close(fdSocketAttente);

      // Initialisation

      FILE* file = NULL;

      char name[25];
      char press;

      int i;
      int nbmechants;
      int score = 0;
      int lvl = 0;

      double temps;
      
      bool reset = true;
      bool perdu = false;
      bool hard;
      
      struct timeval start, middle;
      srand(time(NULL));


      // Introduction
      pseudo(tampon);
      send(fdSocketCommunication, tampon, strlen(tampon), 0);
	    
      nbRecu = recv(fdSocketCommunication, tampon, MAX_BUFFER, 0);	    
      if (nbRecu > 0) {
      tampon[nbRecu] = 0;
      printf("Recu de %s:%d : %s\n",
            inet_ntoa(coordonneesAppelant.sin_addr),
            ntohs(coordonneesAppelant.sin_port),
            tampon);
      snprintf(name,MAX_BUFFER,tampon);

        if (testQuitter(tampon))
          break;
      }
      
      introduction1(tampon, name);
      send(fdSocketCommunication, tampon, strlen(tampon), 0);

      introduction2(tampon);
      send(fdSocketCommunication, tampon, strlen(tampon), 0);
	    
      introduction3(tampon);
      send(fdSocketCommunication, tampon, strlen(tampon), 0);

      nbRecu = recv(fdSocketCommunication, tampon, MAX_BUFFER, 0);
      if (nbRecu > 0) {
	tampon[nbRecu] = 0;
	lvl = atoi(tampon);
	testQuitter(tampon);
      }
      
      record(tampon, file, &lvl);
      send(fdSocketCommunication, tampon, strlen(tampon), 0);
      
      introduction4(tampon);
      send(fdSocketCommunication, tampon, strlen(tampon), 0);

      if ((lvl == 3) || (lvl == 5)){
        hard = true;
	nbmechants = 3;
      } else {
        hard = false;
        nbmechants = 2;
      }

      int coordonnees[NBETOILES + nbmechants][2];
      
      if (coordonnees == NULL) {
	printf("L'allocation tableau des coordonnees a echouee!\n");
      }

      coord(coordonnees, &nbmechants);

      // DÃ©but du jeu

      sleep(2);
      gettimeofday (&start, NULL);

      // Envoi grille;
      for (i = 0; i < (1 + nbmechants + NBETOILES); i++) {
	usleep(100);
	(send(fdSocketCommunication, coordonnees[i], coordonnees[i][0]+1, 0));
	usleep(100);
	(send(fdSocketCommunication, coordonnees[i], coordonnees[i][1]+1, 0));
      }
      
      do{
	// Demande dÃ©placement
	deplacement(tampon);
	send(fdSocketCommunication, tampon, strlen(tampon), 0);

	// Reception dÃ©placement
	nbRecu = recv(fdSocketCommunication, tampon, MAX_BUFFER, 0);
	if (nbRecu > 0) {
	  tampon[nbRecu] = 0;
	  testQuitter(tampon);
	}

	gettimeofday (&middle, NULL);
	temps = (middle.tv_sec - start.tv_sec) + ((middle.tv_usec - start.tv_usec)/1000000000.0);

	move(tampon, coordonnees, &nbmechants, &lvl, &perdu);

	etat(tampon, &temps, &perdu);
	send(fdSocketCommunication, tampon, strlen(tampon), 0);
	
        
	// Envoi grille
	for (i = 0; i < (1 + nbmechants); i++) {
	  usleep(100);
	  (send(fdSocketCommunication, coordonnees[i], coordonnees[i][0]+1, 0));
	  usleep(100);
	  (send(fdSocketCommunication, coordonnees[i], coordonnees[i][1]+1, 0));
	}
	
	nbRecu = recv(fdSocketCommunication, tampon, MAX_BUFFER, 0);
	if (nbRecu > 0) {
	  tampon[nbRecu] = 0;
	  if (strcmp(tampon,"0") == 0){
	    score = 20;
	  }
	  testQuitter(tampon);
	}
      } while ((perdu == false) && (score != 20));

      usleep(5000);
      
      // RÃ©ception score
      nbRecu = recv(fdSocketCommunication, tampon, MAX_BUFFER, 0);
      if (nbRecu > 0) {
	tampon[nbRecu] = 0;
	score = atoi(tampon);

	testQuitter(tampon);
      }

      // Envoi rÃ©sultat
      resultat(tampon, &perdu, &lvl, &score, temps);
      send(fdSocketCommunication, tampon, strlen(tampon), 0);

      // Save score
      new_score(file, &lvl, &score, name);
      
      printf("Client dÃ©connectÃ© - %s:%d\n",
	   inet_ntoa(coordonneesAppelant.sin_addr),
	   ntohs(coordonneesAppelant.sin_port));
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
  snprintf(tampon, MAX_BUFFER, "     L'objectif est de recupÃ©rer 20 Ã©toiles en 1 minute\n     Vous dÃ©placerez votre personnage avec les touches ZQSD\n");
}

void introduction3(char tampon[]){
  sleep(2);
  snprintf(tampon, MAX_BUFFER, "     Niveau 1 : Deux fantomes, pas de perte de points\n     Niveau 2 : Deux fantomes, perte de points / 15 secondes\n     Niveau 3 : Trois fantomes, perte de points / 15 secondes\n     Niveau 4 : Deux fantomes evoluÃ©s, perte de points / 15 secondes\n     Niveau 5 : Ultime\n");
}

void introduction4(char tampon[]){
  sleep(2);
  snprintf(tampon, MAX_BUFFER,"     Le chronometre debutera a l'affichage du jeu\n     Que le meilleur gagne!\n");
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

void pacman(char tampon[], int coordonnees[][2]){
  if (strcmp(tampon,"q") == 0){
    if (coordonnees[0][1] - 1 >=0){
      coordonnees[0][1] -= 1;
    }
  } else if (strcmp(tampon,"d") == 0){
    if (coordonnees[0][1] + 1 < NBCOLONNES){
      coordonnees[0][1] += 1;
    }
  } else if (strcmp(tampon,"z") == 0){
    if (coordonnees[0][0] - 1 >=0){
      coordonnees[0][0] -= 1;
    }
  } else if (strcmp(tampon,"s") == 0){
    if (coordonnees[0][0] + 1 < NBLIGNES){
      coordonnees[0][0] += 1;
    }
  }
}

void fantome(int coordonnees[][2], int *nbmechants){
  bool reset;  
  for (int i=1; i<(*nbmechants+1);i++){
    int move = rand()%2;
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
  } 
}

void move(char tampon[], int coordonnees[][2], int *nbmechants, int *lvl, bool *perdu){
  if(*lvl < 4){
    fantome(coordonnees, nbmechants);
    pacman(tampon, coordonnees);
  } else {
    pacman(tampon, coordonnees);
    fantome(coordonnees, nbmechants);
  }

	
  // Mechant kill Pacman
  for (int i=1; i<(*nbmechants+1);i++){
    if ((coordonnees[i][0] == coordonnees[0][0]) && (coordonnees[i][1] == coordonnees[0][1])){
      *perdu = true;
    }
  }
}

void etat(char tampon[], double *temps, bool *perdu){
  if ((*perdu == false) && (*temps <= 60)){
    continuer(tampon);
  } else {
    if (*temps > 60){
      *perdu = true;
      timer(tampon);
    } else {
      lose(tampon);
    }
  }
}

void continuer(char tampon[]){
  usleep(100);
  snprintf(tampon, MAX_BUFFER,"0");
}

void timer(char tampon[]){
  usleep(100);
  snprintf(tampon, MAX_BUFFER, "\n\n           Limite de temps depassee !\n          ----------------------------");
}    

void lose(char tampon[]){
  usleep(100);
  snprintf(tampon, MAX_BUFFER,"                *************\n                * Game over *\n                *************\n");
}

void resultat(char tampon[], bool *perdu, int *lvl, int* score, double temps){
  if (*lvl != 1){
    int a = temps/15;
    *score -=  a;
    if (*score < 0){
      *score = 0;
    }
  }

  if (*perdu == true){
    usleep(10);
    snprintf(tampon, MAX_BUFFER,"\n\n          Votre temps est %.1f secondes.\n            Votre score final est : %d\n\n            Dommage, vous avez perdu !\n\n", temps, *score);
  } else {
    usleep(10);
    snprintf(tampon, MAX_BUFFER,"\n\n          Votre temps est %.1f secondes.\n                 Score final : %d\n\n      (|___/)      Felicitations       (|___/)\n      (='.'=)        -   -   -         (='.'=)\n     ('')_('')    Vous avez gagne !   ('')_('')\n\n", temps, *score);
    sleep(5);
  }
}


void record(char tampon[], FILE *file, int *lvl){
  char *fonction = NULL;
  char *separateur ={" "};
  char niveau[5];
  char joueur[30];
  char raw[MAX_BUFFER];
  int score_bis;
  int score = 0;

  snprintf(niveau, 5, "%d",*lvl);
  
  file = fopen("historique.txt", "r");
  if (file != NULL){
       // tant qu'il n'a pas parcouru l'intÃ©gralitÃ© du file
    while (fgets(raw, MAX_BUFFER, file) != NULL){
      fonction = strtok(raw, separateur);
            
      // SÃ©lection du niveau
      if (strcmp(fonction,niveau) == 0){
	fonction = strtok(NULL, separateur);
	score_bis = atoi(fonction);

	// score max ?
	if (score_bis > score){
	  score = score_bis;
	  fonction = strtok(NULL, separateur);
	  strcpy(joueur,fonction);
	}
      }
    }
    fclose(file);
    }
  
  if (score == 0){
    usleep(100);
    snprintf(tampon, MAX_BUFFER,"\n     -->  Niveau %s: Aucun record  <-- \n\n", niveau);
      } else {
    usleep(100);
    snprintf(tampon, MAX_BUFFER,"\n\n        -->  Niveau %s: Record detenu par %s  %d!  <-- \n\n", niveau, joueur, score);
  }
}

void new_score(FILE *file,int *lvl, int *score, char *name){
  file = fopen("historique.txt", "a");
  if (file != NULL){
    fprintf(file,"%d %d %s \n", *lvl, *score, name);
    fclose(file);
  }
}
