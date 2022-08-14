// depliage en C
// v0
// fait :
//	- charge fichier .obj
//  - extrait points et faces
//	- trouve les voisins de chaque arête
//  - calcule les coplaneités
//  - demande le fichier 2d(.obj)
//	- demande l'échelle (1 = 100%)
//  - demande le format de sortie (de 0 à 5 pour A0 à A5)
//	- deplie dans un PDF avec autant de pages que nécessaire
//	- traits de coupe en rouge
//	- les plis montagne en - marron, plis vallée en -. vert
//	- numérote uniquement les paires d'arêtes à relier
//	- paires d'arêtes internes à la pièce en bleu
//	- paires d'arêtes entre deux pièces en noir
//	- sauve les données dans un fichier .dep

#define epsilon 0.0001
#define max(a,b) (a>=b?a:b)
#define min(a,b) (a<=b?a:b)

#define L_COUPE 1
#define L_PLI_M 2
#define L_PLI_V 3
#define L_PLI_C	4

#define MAX_TAMPON 100

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include <string.h>
//#include <cairo/cairo.h>
//#include <cairo/cairo-pdf.h>
#include "deputils.c"


int main(void)
{
	char OBJ[50] = "bb";
	printf("Nom fichier :");
	scanf("%s", OBJ);

	//float ech;
	//printf("Echelle :");
	//scanf("%f", &ech);

	printf("--------- Chargement : %s ---------\n", OBJ);
	
	struct sVector3d * sommets = NULL;
	int nbSommets = 0;
	
	FILE * fs;
	fs = fopen(OBJ, "r");
	if (fs == NULL) {
		perror("Erreur ouverture fichier");
		exit(0);
	}
	
	char *tampon;
	char *ligneLue;
	do {
		tampon = malloc(sizeof(char) * MAX_TAMPON);
		ligneLue = fgets(tampon, MAX_TAMPON, fs);
		char typeLigne[2];
		strncpy(typeLigne, tampon, 2);
		
		//printf("%c\n", typeLigne[0]);
		
		double v0, v1, v2;
		if ((typeLigne[0] == 'v')&&(typeLigne[1] == ' ')) { // v = vector (sommet)
			sscanf(ligneLue, "v %lf %lf %lf", &v0, &v1, &v2);
			printf("sommet : %lf %lf %lf\n", v0, v1, v2);
		}
		
		//printf("%s", tampon);
	} while (ligneLue != NULL);
	if (fclose(fs))
		perror("erreur fermeture fichier");
		
	free(tampon);
	free(sommets);
}
