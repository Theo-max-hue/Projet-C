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

const char *EXIT = "exit";

void pseudo(char tampon[]){
  printf("     ");
  fgets(tampon, MAX_BUFFER, stdin);
  strtok(tampon, "\n");
}

void niveau(char tampon[]){
  printf("     ");
  fgets(tampon, MAX_BUFFER, stdin);
  strtok(tampon, "\n");
}

int testQuitter(char tampon[]) {
    return strcmp(tampon, EXIT) == 0;
}


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

    while (1) {
      int i = 0;
        // on attend la réponse du serveur
        nbRecu = recv(fdSocket, tampon, MAX_BUFFER, 0);

	if (nbRecu > 0) {
            tampon[nbRecu] = 0;
            printf("%s\n", tampon);

            if (testQuitter(tampon)) {
                break; // on quitte la boucle
            }
        }

        pseudo(tampon);
        send(fdSocket, tampon, strlen(tampon), 0);

	while (i!=4){
	  printf("i=%d",i);
	  nbRecu = recv(fdSocket, tampon, MAX_BUFFER, 0);
	  if (nbRecu > 0) {
	    tampon[nbRecu] = 0;
	    printf("%s\n", tampon);

	    if (testQuitter(tampon)) {
	      break; // on quitte la boucle
	    }
	  }
	  i++;
	}

	niveau(tampon);
        send(fdSocket, tampon, strlen(tampon), 0);

	



        
    }

    close(fdSocket);

    return EXIT_SUCCESS;
}