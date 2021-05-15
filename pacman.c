#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>
#include <stdbool.h>
//#include<json-c/json.h>

#define NBMECHANTS 2
#define NBETOILES 20
#define NBLIGNES 6
#define NBCOLONNES 10

struct personnage{
    int x;
    int y;
};

void introduction(char *name, int *lvl){
    printf("\n     Entrer votre pseudo : ");
    scanf("%s",&name);
    printf("\n     Bienvenue %s dans le jeu du pacman !\n", &name);
    sleep(2);
    printf("\n     L'objectif est de recuperer 20 etoiles en 1 minute\n");
    printf("     Vous deplacerez votre personnage avec les touches ZQSD\n");
    sleep(3);
    printf("\n     Niveau 1 : Deux fantomes, pas de perte de points \n");
    printf("     Niveau 2 : Deux fantomes, perte de points / 15 secondes \n");
    printf("     Niveau 3 : Trois fantomes, perte de points / 15 secondes \n");
    int a;
    bool reset = true;
    do{
        printf("\n     Saisir le niveau de difficulte : ");
        scanf("%d",&a);
        if ((a == 1) || (a == 2) || (a == 3)){
            printf("\n     Le chronometre debute a l'affichage du jeu\n");
            printf("     Que le meilleur gagne !\n\n");
            reset = false;
        } else {
            printf("\n     Choisir parmi les niveaux de difficultes");
        }
    } while (reset == true);
    *lvl = a;
    sleep(2);
}


int *constructionTableau(const int taille) {
    int *tableau = (int *) malloc(taille * sizeof(int));

    if (tableau == NULL) {
        printf("Allocation echouée\n");
    }

    for (int i = 0; i < taille; i++) {
        tableau[i] = 0;
    }

    return tableau;
}

int **constructionMatrice(const int nbLignes, const int nbColonnes) {
    int **matrice = (int **) malloc(nbLignes * sizeof(int *));

    if (matrice == NULL) {
        printf("L'allocation a échouée!\n");
    }

    for (int i = 0; i < nbLignes; i++) {
        matrice[i] = constructionTableau(nbColonnes);
    }

    return matrice;
}

void affichageMatrice(int coordonnees[][2],int * pacman, int *score, int * nbmechants) {
    for (int i = 0; i < NBLIGNES; i++) {
        for (int j = 0; j < NBCOLONNES; j++) {
            if ((i == pacman[0]) && (j == pacman[1])){
                printf("| C ");
            } else{
                printf("|   ");
            }
            for (int k=*nbmechants; k<(*nbmechants+NBETOILES); k++){
                if((i == coordonnees[k][0]) && (j == coordonnees[k][1])){
                    if((pacman[0] == coordonnees[k][0]) && (pacman[1] == coordonnees[k][1])){
                        printf("\b\b\b\b| C ");
                        coordonnees[k][0] = -1;
                        coordonnees[k][1] = -1;
                        *score += 1;
                    } else {
                        printf("\b\b\b\b| * ");
                    }
                }
            }
            for (int k=0; k<*nbmechants; k++){
                if((i == coordonnees[k][0]) && (j == coordonnees[k][1])){
                    printf("\b\b\b\b| X ");
                }
            }
        }
        printf("|\n");
    }
    printf("\n");
    printf("          Votre score est : %d\n",*score);
}

int *coord_pacman(){
    int *coord_pacman = (int *) malloc(sizeof(int)* 2);

    if (coord_pacman == NULL) {
        printf("L'allocation tableau des coordonnees du pacman a echouee!\n");
    }

    struct personnage pacman;
    pacman.x = 4;
    pacman.y = 5;
    for (int i = 0; i < 1; i++) {
        coord_pacman[i] = pacman.x;
        coord_pacman[i+1] = pacman.y;
    }

    return coord_pacman;
}

void coord(int coordonnees[][2],int * pacman, int * nbmechants){
    struct personnage perso[NBETOILES + *nbmechants];
    for (int i=0; i<(NBETOILES + *nbmechants); i++){
        perso[i].x = rand()%(NBLIGNES);
        perso[i].y = rand()%(NBCOLONNES);
        for (int j=0; j<i; j++){
            if (((coordonnees[j][0] == perso[i].x) && (coordonnees[j][1] == perso[i].y)) || ((perso[i].x == pacman[0]) || (perso[i].y == pacman[1]))){
                i--;
            }
        }
        coordonnees[i][0] = perso[i].x;
        coordonnees[i][1] = perso[i].y;
    }
}


void keypress (char *press){
    do{
        printf("\nOu veux-tu te deplacer ? ");
        scanf("%s", press);
        if (strlen(press) != 1 ){
            printf("Saisie incorrecte");
        }
    } while (strlen(press) != 1);
}

int* move_pacman(char press, int *pacman,bool *move, bool *perdu){
    switch (press){
    case 'q':
        pacman[1] -= 1;
        break;
    case 'd':
        pacman[1] += 1;
        break;
    case 'z':
        pacman[0] -= 1;
        break;
    case 's':
        pacman[0] += 1;
        break;
    default:
        *move = false;
        printf("Veuillez rentrer une autre touche \n");
        break;
    }
    if ((pacman[1] < 0) || (pacman[1] >= NBCOLONNES) || (pacman[0] < 0) || (pacman[0] >= NBLIGNES)){
        printf("\nVous etes sorti de la grille\n");
        *perdu = true;
    }

    return pacman;
}

void move_mechant(int coordonnees[][2], int* pacman, bool *perdu, int *nbmechants){
    bool reset;
    for (int i=0; i<*nbmechants;i++){
        do{
            reset = false;
            //? abscisse ou ordonnée
            int move = rand()%2;
            if (coordonnees[i][move] > pacman[move]){
                coordonnees[i][move] -= 1;
            } else if (coordonnees[i][move] < pacman[move]){
                coordonnees[i][move] += 1;
            } else {
                if (move == 0){
                    if (coordonnees[i][1] > pacman[1]){
                        coordonnees[i][1] -= 1;
                    } else {
                        coordonnees[i][1] += 1;
                    }
                } else {
                    if (coordonnees[i][0] > pacman[0]){
                        coordonnees[i][0] -= 1;
                    } else {
                        coordonnees[i][0] += 1;
                    }
                }
            }
            if (i != 0){
                if((coordonnees[i][0] == coordonnees[i-1][0]) && (coordonnees[i][1] == coordonnees[i-1][1])){
                    reset = true;
                }
            }
        } while (reset == true);
    }

    // Mechant kill Pacman
    for (int i=0; i<*nbmechants;i++){
        if ((coordonnees[i][0] == pacman[0]) && (coordonnees[i][1] == pacman[1])){
            printf("\n       Le mechant a devore Pacman !\n");
            *perdu = true;
        }
    }
}

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
        printf("       Dommage, vous avez perdu !");
    } else {
        printf("\n   (|___/)      Felicitations       (|___/)");
        printf("\n   (='.'=)        -   -   -         (='.'=)");
        printf("\n  ('')_('')    Vous avez gagne !   ('')_('')");
    }
    printf("\n\n");
    sleep(5);
}



int main(int argc, char const **argv[]) {

    // FILE *fp;
	// char buffer[1024];
	// struct json_object *parsed_json;
	// struct json_object *pseudo;
	// struct json_object *final_score;
    // size_t i;

    // fp = fopen("sauvegarde.json","r");
	// fread(buffer, 1024, 1, fp);
	// fclose(fp);

    // parsed_json = json_tokener_parse(buffer);

    // json_object_object_get_ex(parsed_json, "Pseudo", &pseudo);
	// json_object_object_get_ex(parsed_json, "Score", &final_score);

    // printf("Pseudo: %s\n", json_object_get_string(pseudo));
	// printf("Score: %d\n", json_object_get_int(final_score));

    int lvl=0;
    int nbmechants = 2;
    int *pacman;
    int coordonnees[NBETOILES + nbmechants][2];
    char press;
    char name[25];
    int score = 0;
    bool perdu = false;
    bool hard;
    clock_t start, end, middle;
    double temps;
    srand(time(NULL));

    if (coordonnees == NULL) {
        printf("L'allocation tableau des coordonnees a echouee!\n");
    }

    pacman = coord_pacman();
    introduction(name, &lvl);
    if (lvl != 3){
        hard = false;
    } else {
        hard = true;
        nbmechants = 3;
    }

    coord(coordonnees, pacman, &nbmechants);
    affichageMatrice(coordonnees, pacman, &score, &nbmechants);
    start = clock();
    
    while ((perdu == false) && (score != 20)){
        bool move = true;
        keypress(&press);
        if (hard == true){
            nbmechants = 3;
        } else {
            nbmechants = 2;
        }
        pacman = move_pacman(press, pacman, &move, &perdu);
        if ((move == 1) && (perdu == false)){
            move_mechant(coordonnees, pacman, &perdu, &nbmechants);
            affichageMatrice(coordonnees, pacman, &score, &nbmechants);
        }
        middle = clock();
        temps = ((double)middle - start) / CLOCKS_PER_SEC;
        if (temps > 60){
            perdu = true;
            printf("       Le temps est ecoule");
        }
    }
    end = clock();
    temps = ((double)end - start) / CLOCKS_PER_SEC;
     
    resultat(&perdu, temps, &lvl, &score);

    free(pacman);
    return 0;
}
