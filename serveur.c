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

void pseudo(char buffer[]){
    snprintf(buffer, MAX_BUFFER, "\n     Entrer votre pseudo : ");
}

void introduction1(char tampon[],char * pseudo){
  snprintf(tampon, MAX_BUFFER, "     Bienvenue %s dans le jeu du pacman ! \n", pseudo);
}

void introduction2(char tampon[]){
  sleep(2);
  snprintf(tampon, MAX_BUFFER, "     L'objectif est de recuperer 20 etoiles en 1 minute\n     Vous deplacerez votre personnage avec les touches ZQSD\n");
}

void introduction3(char tampon[]){
  sleep(2);
  snprintf(tampon, MAX_BUFFER, "\n     Niveau 1 : Deux fantomes, pas de perte de points\n     Niveau 2 : Deux fantomes, perte de points / 15 secondes\n     Niveau 3 : Trois fantomes, perte de points / 15 secondes\n");
}

void intro_lvl(char tampon[]){
  sleep(1);
  snprintf(tampon, MAX_BUFFER,"\n     Saisir le niveau de difficulte : ");
}

void introduction4(char tampon[]){
  snprintf(tampon, MAX_BUFFER,"     Choisir parmi les niveaux de difficulte : ");
}

void introduction5(char tampon[]){
  snprintf(tampon, MAX_BUFFER,"     Le chronometre debute a l'affichage du jeu\n     Que le meilleur gagne!");
}

int testQuitter(char tampon[]) {
    return strcmp(tampon, EXIT) == 0;
}


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

            int lvl = 0;
	    char name[25];

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

                if (testQuitter(tampon)) {
                    break; // on quitte la boucle
                }
            }

            sleep(2);	    
            introduction1(tampon, name);
            send(fdSocketCommunication, tampon, strlen(tampon), 0);

            introduction2(tampon);
            send(fdSocketCommunication, tampon, strlen(tampon), 0);
	    
            introduction3(tampon);
	    send(fdSocketCommunication, tampon, strlen(tampon), 0);
            bool reset = true;
            do {
	      intro_lvl(tampon);
	      send(fdSocketCommunication, tampon, strlen(tampon), 0);

	      nbRecu = recv(fdSocketCommunication, tampon, MAX_BUFFER, 0);
	      if (nbRecu > 0) {
                tampon[nbRecu] = 0;
                printf("Recu de %s:%d : %s\n",
		       inet_ntoa(coordonneesAppelant.sin_addr),
		       ntohs(coordonneesAppelant.sin_port),
		       tampon);
		lvl = atoi(tampon);
		printf("lvl = %d\n",lvl);

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
	    printf("lvl = %d", lvl);
            introduction5(tampon);
	    send(fdSocketCommunication, tampon, strlen(tampon), 0);
            sleep(2);


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
