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

	struct sVector3d * sommets = NULL;
	int nbSommets = 0;
	
	int gc = 0; // groupe courant
	
	int * faces0 = NULL;
	int nbFaces = 0;
	int pos;
	
	FILE * fs;
	fs = fopen(OBJ, "r");
	if (fs == NULL) {
		perror("Erreur ouverture fichier");
		exit(0);
	}
	
	char *tampon;
	char *ligneLue;
	do {
		int fc[4];
		tampon = malloc(sizeof(char) * MAX_TAMPON);
		ligneLue = fgets(tampon, MAX_TAMPON, fs);
		char tL0 = tampon[0];
		char tL1 = tampon[1];
		if ((tL0 == 'v')&&(tL1 == ' ')) { // v = vector (sommet)
			double v[3];
			sscanf(ligneLue, "v %lf %lf %lf", &v[0], &v[1], &v[2]);
			struct sVector3d s = { v[0]*ech, v[1]*ech, v[2]*ech };
			sommets = (struct sVector3d *) realloc(sommets, sizeof(struct sVector3d) * (nbSommets+1));
			sommets[nbSommets++] = s;
		} else if ((tL0 == 'g')&&(tL1 == ' ')) { // g = group (groupe)
			gc++;
		} else if ((tL0 == 'f')&&(tL1 == ' ')) { // f = face
			char c[3][20];
			sscanf(ligneLue, "f %s %s %s", c[0], c[1], c[2]);
			for (int i = 0; i < 3; i++) {
				pos = strspn(c[i], "0123456789");
				char ch[pos+1];
				strncpy(ch, c[i], pos);
				ch[pos] = '\0';
				fc[i] = atoi(ch);
			}
			fc[3] = gc;
			faces0 = (int *) realloc(faces0, sizeof(int) * (nbFaces+1)*4);
			for (int i = 0; i < 4; i++) {
				faces0[nbFaces*4+i] = fc[i]-1;
			}			
			nbFaces++;
		}
	} while (ligneLue != NULL);
	if (fclose(fs))
		perror("erreur fermeture fichier");
		
	free(tampon);
	
	int faces[nbFaces][4];
	int n = 0;
	for (int i = 0; i < nbFaces; i++) {
		for( int j = 0; j < 4; j++)
			faces[i][j] = faces0[n++];
	}
	free(faces0);
	
	for (int i = 0; i < nbFaces; i++)
		printf("face %d : %d %d %d (%d)\n", i, faces[i][0], faces[i][1], faces[i][2], faces[i][3]);
	

	// VOISINS
	struct sVoisin voisins[nbFaces][3];	
	calculeVoisinage(faces, nbFaces, voisins);
	
	for (int i = 0; i < nbFaces; i++) {
		printf("\n %d : voisins: ", i);
		for (int j = 0; j < 3; j++)
			printf(" %d", voisins[i][j].nF);
	}
	
	// V3D V2D
	struct sVector3d v3d[nbFaces][3];
	struct sVector2d v2d[nbFaces][3];
	for (int i = 0; i < nbFaces; i++) {
		for (int j = 0; j < 3; j++) {
			v3d[i][j] = sommets[faces[i][j]];
		}
		d2ize(v3d[i], v2d[i]);
	}
	free(sommets);
	
	// estCOP
	struct sCop tCop[nbFaces * 3];
	calculeCop(nbFaces, voisins, tCop, v3d);
	
  struct sVector2d marge = {10, 10};
  struct sVector2d formats[6] = {
		{2380,	3368},	// A0
		{1684,	2380},	// A1
		{1190,	1684},	// A2
		{ 842,	1190},	// A3
		{ 595,   842},	// A4
		{ 421,   595},	// A5
	};

	//printf("Format A(0..5) :");
  int fc = 4;
  //scanf("%d", &fc);
  
  struct sVector2d limitePage = sVector2dSub(formats[fc], marge);
	
	puts("");
	// nb elements
	printf("%d points\n%d faces\n", nbSommets, nbFaces);
  
	// DEBUT DEPLIAGE
	
}
