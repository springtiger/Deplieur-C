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

	float ech = 1;
	//printf("Echelle :");
	//scanf("%f", &ech);

	printf("Chargement : %s\n", OBJ);
	struct sVector3d * sommets = NULL;
	int nbSommets = 0;
	
	int gc = 0; // groupe courant
	
	//int faces
	
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
		char tL0 = tampon[0];
		char tL1 = tampon[1];
		if ((tL0 == 'v')&&(tL1 == ' ')) { // v = vector (sommet)
			double v[3];
			sscanf(ligneLue, "v %lf %lf %lf", &v[0], &v[1], &v[2]);
			struct sVector3d s = { v[0]*ech, v[1]*ech, v[2]*ech };
			sommets = (struct sVector3d *)realloc(sommets, sizeof(struct sVector3d) * (nbSommets+1));
			sommets[nbSommets++] = s;
			printf("sommet : %lf %lf %lf\n", s.x, s.y, s.z);
		} else if ((tL0 == 'g')&&(tL1 == ' ')) { // g = group (groupe)
			gc++;
		} else if ((tL0 == 'f')&&(tL1 == ' ')) { // f = face
			char c[3][20];
			//c[0] = malloc(sizeof(char)*30);
			//c[1] = malloc(sizeof(char)*30);
			//c[2] = malloc(sizeof(char)*30);
			sscanf(ligneLue, "f %s %s %s", c[0], c[1], c[2]);
			printf("face : %s %s %s\n", c[0], c[1], c[2]);
		}
		
		//printf("%s", tampon);
	} while (ligneLue != NULL);
	if (fclose(fs))
		perror("erreur fermeture fichier");
		
	free(tampon);
	free(sommets);
}
