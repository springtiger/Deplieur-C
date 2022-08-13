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

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <cairo/cairo.h>
#include <cairo/cairo-pdf.h>
#include "deputils.c"

int main(void) {
// charge données
	char * nomFichierDonnees = "donnees.dep";
	FILE * fd;
	int rc;
	fd = fopen(nomFichierDonnees, "r");
	if (fd == NULL) {
		return -1;
	}

	char OBJ[50];
	double ech;
	int fc;
	int tc0;
	
	fscanf(fd, "%s", OBJ);
	fscanf(fd, "%lf", &ech);
	fscanf(fd, "%2d", &fc);
	fscanf(fd, "%4d", &tc0);
	int nbD = 0;
	struct sDepliage * sD = NULL;
	struct sDepliage sD0;
	
  int d0, d1, d2;
  while (fscanf(fd, "%d", &d0) > 0) {
 		if (d0 == -1) {
			fscanf(fd, "%d", &d1);
			fscanf(fd, "%d", &d2);
		} else {
			fscanf(fd, "%d", &d1);
			d2 = 0;			
		}
 		sD0.orig = d0;
 		sD0.face = d1;
 		sD0.a		 = d2;
		sD = (struct sDepliage *) realloc(sD, sizeof(struct sDepliage)*(nbD+1));
		sD[nbD] = sD0;
		nbD++;
 	}	
	rc = fclose(fd);
	if (rc == EOF) {
		return -1;
	}

// charge données languettes
	int nbLgt = 0;
	struct sLang * sLgt = NULL;

	int nbLgtB = 0;
	struct sLang * sLgtB = NULL;

	struct sLang sLgt0;
	
	char * nomFichierLanguettes = "donnees.lng";
	fd = fopen(nomFichierLanguettes, "r");	
	if (fd != NULL) {
		while (fscanf(fd, "%d", &d0) > 0) {
			fscanf(fd, "%d", &d1);
			fscanf(fd, "%d", &d2);
			sLgt0.n1  = d0;
			sLgt0.n2  = d1;
			sLgt0.v  = d2;
			sLgt = (struct sLang *) realloc(sLgt, sizeof(struct sLang)*(nbLgt+1));
			sLgt[nbLgt] = sLgt0;
			nbLgt++;
		}
		rc = fclose(fd);
		if (rc == EOF) {
			return -1;
		}
	}
		
	printf("nb liens : %d\n", nbD);	
	printf("Nom fichier :%s\n", OBJ);
	printf("Echelle : %lf\n", ech);
	printf("format page : A%d\n", fc);
	printf("1er triangle : %d\n", tc0);
	printf("--------- Chargement : %s ---------\n", OBJ);
	char* donneesOBJ = litFichierTexte(OBJ);
	if (donneesOBJ == NULL)
	{
		printf("Erreur Lecture\n");
		return 1;
	}
	// extrait les lignes
  char* d = strdup(donneesOBJ);
  free(donneesOBJ);
  const char *sep = "\n";
  char* tok;
	char type[2];

  int nbV = 0;											// nb de points
  struct sVector3d* vertices = NULL;// tableau des points (3d)
	double f[3];											// point courant
  
  int nbF = 0;											// nb de faces
  int* faces0 = NULL;								// tableau des faces
  int t[4];													// face courante
  int gc = 0;
  
  while ((tok = strtok_r(d, sep, &d))) {
		strncpy(type, tok, 2);
		if (strcmp(type, "v ") == 0) {			// POINT (vertex)
			char* vd = strdup(tok);
			char* vtok;
			int n = 0;
			while ((n < 3) && (vtok = strtok_r(vd, " ", &vd))) {
				if (strcmp(vtok, "v") != 0) {
					f[n++] = atof(vtok);
				}
			}
			struct sVector3d v = { f[0]*ech, f[1]*ech, f[2]*ech };
		  vertices = (struct sVector3d*) realloc(vertices, (nbV+1) * sizeof(struct sVector3d));
			vertices[nbV] = v;
			nbV++;
		}
		else if (strcmp(type, "g ") == 0) {	// GROUPE
			gc++;
		}
		else if (strcmp(type, "f ") == 0) {	// FACE
			char* fd = strdup(tok);
			char* ftok;
			int n = 0;
			while ((n < 3) && (ftok = strtok_r(fd, " ", &fd))) {
				if (strcmp(ftok, "f") != 0) {
					int t0 = atoi(ftok);
					if (t0 == 0) {
						int pos = strcspn(ftok, "/");
						char tmp[10];
						strncpy(tmp, ftok, pos-1); 
						t0 = atoi(tmp);
					}
					t[n++] = t0;
				}
			}
			t[3] = gc;
			faces0 = (int*)realloc(faces0, (nbF+1)*4*sizeof(int));
			for (int i = 0; i < 4; i++) {
				faces0[nbF*4+i] = t[i]-1;
			}
			nbF++;
		}
	}
	free(tok);
	
	int faces[nbF][4];
	int n = 0;
	for (int i = 0; i < nbF; i++){
		for (int j = 0; j < 4; j++){
			faces[i][j] = faces0[n++];
		}
	}
	free(faces0);
	
	// VOISINS
	struct sVoisin voisins[nbF][3];
	calculeVoisinage(faces, nbF, voisins);
		
	// V3D V2D
	struct sVector3d v3d[nbF][3];
	struct sVector2d v2d[nbF][3];
	for (int i = 0; i < nbF; i++) {
		for (int j = 0; j < 3; j++) {
			v3d[i][j] = vertices[faces[i][j]];
		}
		d2ize(v3d[i], v2d[i]);
	}
	free(vertices);
	
	// estCOP
	struct sCop tCop[nbF*3];
	calculeCop(nbF, voisins, tCop, v3d);
	
  struct sVector2d formats[6] = {
		{2380,	3368},	// A0
		{1684,	2380},	// A1
		{1190,	1684},	// A2
		{ 842,	1190},	// A3
		{ 595,   842},	// A4
		{ 421,   595},	// A5
	};
  struct sVector2d marge = {10, 10};
  //struct sVector2d limitePage = sVector2dSub(formats[fc], marge);
	
	// nb elements
	printf("%d points - %d faces\n", nbV, nbF);
  
	// DEBUT DEPLIAGE
	
	puts("Afficher le n° des faces (O/N) ?");
	char chAffNumFace; 
	bool bAffNumFace;
	int r = scanf(" %c", &chAffNumFace);
	if (r == 1) {
		bAffNumFace = toupper(chAffNumFace) == 'O';
	}

  // INITs PDF
  cairo_surface_t *surface;
  cairo_t *cr;

  surface = cairo_pdf_surface_create("affiche.pdf", formats[fc].x, formats[fc].y);
  cr = cairo_create(surface);
  
  cairo_select_font_face (cr, "Sans", CAIRO_FONT_SLANT_NORMAL,
		CAIRO_FONT_WEIGHT_NORMAL);
  cairo_set_font_size (cr, 14.0);
  cairo_set_line_width(cr, 1);

	struct sNAff lSNA [nbF*3];
	int nAff = 0;

	int nbP = -1;
	struct sVector2d *vMin = NULL;
	struct sLigne lignes[nbF *3];
	int nbL = 0;
	
	for (int i = 0; i < nbD; i++) {
		if (sD[i].orig == -1) {
			nbP++;
			int f = sD[i].face;
			int a = sD[i].a;
			if (a != 0) {
				struct sVector2d m = centroid(v2d[f]);
				for (int vi = 0; vi < 3; vi++)
					v2d[f][vi] = rotation(m, v2d[f][vi], a);
			}
			vMin = (struct sVector2d*) realloc(vMin, sizeof(struct sVector2d) * (nbP+1));
			vMin[nbP] = vPetit(v2d[f][0], vPetit(v2d[f][1],v2d[f][2]));
		}
		sD[i].page = nbP;
		
		if (sD[i].orig > -1) {
			int tc = sD[i].orig;
			int vc = sD[i].face;
			int vi =	voisins[tc][0].nF == vc ? 0 :
								voisins[tc][1].nF == vc ? 1 : 2;
			struct sVoisin v = voisins[tc][vi];

			// 1°) rapproche v2d[vc] de v2d[tc]
			struct sVector2d deltaV = sVector2dSub(v2d[tc][vi], v2d[vc][v.idx]);
			for (int n = 0; n < 3; n++)
				v2d[vc][n] = sVector2dAdd(v2d[vc][n], deltaV);
			// 2°) tourne v2d[vc]
			double a = calcAngle(v2d[tc][vi], v2d[tc][suiv(vi)], v2d[vc][prec(v.idx)]);
			for (int n = 0; n < 3; n++) {
				v2d[vc][n] = rotation(v2d[tc][vi], v2d[vc][n], a);
				vMin[nbP] = vPetit(vMin[nbP], v2d[vc][n]);
			}
		}
	}
	
	for (int i = 0; i < nbD; i++) {
		int tc = sD[i].face;
		int pc = sD[i].page;
		for (int j = 0; j < 3; j++) {
			v2d[tc][j] = sVector2dSub(v2d[tc][j],
				sVector2dSub(vMin[pc], marge));
		}
	}

	for (int i = 0; i < nbD; i++) {
		int tc = sD[i].face;
		int pc = sD[i].page;
		for (int j = 0; j < 3; j++) {
			lignes[nbL] = sLigneNew(pc, nbL,
				v2d[tc][j], v2d[tc][suiv(j)],
				tc, j,
				voisins[tc][j].nF, voisins[tc][j].idx);
			nbL++;
		}
	}
	
	supprimeDoublons(lignes, nbL);

	qsort(lSNA, nAff, sizeof(struct sNAff), compAff);
	qsort(lignes, nbL, sizeof(struct sLigne), compPg);

	int lc = 0;
	int nA;
	int typeL;
	char* txtPage = NULL;

	struct sAN lAN[nbF];
	int nbAN = 0;
	int ppc = 0;
	
	for (int i = 0; i < nbL; i++){
		struct sLigne l = lignes[i];
		if (l.id > -1) {
			if (lc != l.nP) {
				lc = l.nP;
				if (nbAN > 0) {
					afficheNumsPage(cr, lAN, nbAN, v2d);
					for (int sdi = 0; sdi < nbD; sdi++) {
						struct sDepliage sdt = sD[sdi];
						if (sdt.page == ppc) {
							if (bAffNumFace) {
								struct sVector2d m = centroid(v2d[sdt.face]);
								afficheNum(cr, sdt.face, m, m, C_BLEU);
							}
						}
					}
				}
				nbAN = 0;
				cairo_show_page(cr);
				ppc++;
			}	

			double c = tCop[(l.n1*3)+l.i1].cop;
			if (l.nb == 1) {
				struct sLang l0 = {.n1 = l.n1, .n2 = l.n2};
				struct sLang * rL;
				if (nbLgt > 0) {
					rL = (struct sLang *)bsearch(&l0, sLgt, nbLgt,
					sizeof(struct sLang), compLang);
					if(rL == NULL) {
						typeL = L_COUPE;
					} else if (rL->v == 0) {
						typeL = L_COUPE;
					}	else {	
						if (fabs(c) < 10e-7) {
							typeL = L_LGT_C;
						} else {
							typeL = c < 0 ? L_LGT_M : L_LGT_V;
						}
					}
				} else {
					typeL = L_COUPE;
				}
				
				l0.v = l.n1 < l.n2 ? 1 : 0;
				sLgtB = (struct sLang *) realloc(sLgtB, sizeof(struct sLang) * (nbLgtB+1));
				sLgtB[nbLgtB] = l0;
				nbLgtB++;
			} else {
				if (fabs(c) < 10e-7) {
					typeL = L_PLI_C;
				} else {
					typeL = c < 0 ? L_PLI_M : L_PLI_V;
				}
			}
			if (typeL != L_PLI_C) {
				faitLigne(cr, l.p1, l.p2, typeL);
			}
			if (l.nb == 1) {	
				struct sNAff cleN;
				struct sNAff *rechN;
				cleN.nMax = max(l.n1, l.n2);
				cleN.nMin = min(l.n1, l.n2);
				rechN = (struct sNAff *)bsearch(&cleN, lSNA, nAff,
				sizeof(struct sNAff), compAff);
				if (rechN != NULL)
					nA = rechN->a;
				else {
					cleN.a = nAff;
					nA = nAff;
					lSNA[nAff++] = cleN;
					qsort(lSNA, nAff, sizeof(struct sNAff), compAff);
				}
				struct sAN sAN0 = {nA, l.p1, l.p2};
				lAN[nbAN++] = sAN0;
			}
		}
	}
	
	txtPage = (char *)malloc(20 * sizeof(char));
	sprintf(txtPage, "page %d (%d)", lc+1, faces[lignes[nbL-1].n1][3]);
	printf("pages: %s\n", txtPage);
	free(txtPage);
	
	if (nbAN > 0) {
		afficheNumsPage(cr, lAN, nbAN, v2d);
		
		for (int sdi = 0; sdi < nbD; sdi++) {
			struct sDepliage sdt = sD[sdi];
			if (sdt.page == ppc) {
				if (bAffNumFace) {
					struct sVector2d m = centroid(v2d[sdt.face]);
					afficheNum(cr, sdt.face, m, m, C_BLEU);
				}
			}
		}
	}
	
	puts("Créer fichier languettes (O/N) ?");
	char repLng;
	
	r = scanf(" %c", & repLng);
	if (r == 1) {
		if (toupper(repLng) == 'O') {
			puts("Type de languettes");
			puts(" 0 : sans languettes");
			puts(" 1 : 1 languette par paire");
			puts(" 2 : 2 languettes par paire");
			int repTLng;
			scanf("%d", & repTLng);
			if ((repTLng >= 0) && (repTLng <= 2))
				sauveLanguettes(sLgtB, nbLgtB, repTLng);
		}
	}

	qsort(lSNA, nAff, sizeof(struct sNAff), compAffa);	
	
	int numL;
	do {
		puts("Inverser languette (n° sinon -1) ?");
		int r;
		r = scanf("%d", &numL);
		if (r == 1) {
			if (numL > -1) {
				struct sNAff cleN;
				struct sNAff * rechN;
				cleN.a = numL;
				rechN = (struct sNAff *) bsearch(&cleN, lSNA, nAff,
								sizeof(struct sNAff), compAffa);
				if(rechN != NULL) {
					struct sLang cleL;
					struct sLang * rechL;
					cleL.n1 = rechN->nMax;
					cleL.n2 = rechN->nMin;
					rechL = (struct sLang *) bsearch(&cleL, sLgt, nbLgt,
									sizeof(struct sLang), compLang);
					if (rechL != NULL) {
						rechL->v = 1 - rechL->v;
					}
					cleL.n1 = rechN->nMin;
					cleL.n2 = rechN->nMax;
					rechL = (struct sLang *) bsearch(&cleL, sLgt, nbLgt,
									sizeof(struct sLang), compLang);
					if (rechL != NULL) {
						rechL->v = 1 - rechL->v;
						sauveLanguettes(sLgt, nbLgt, M_LANG_SAUV);
					}
				}
			}
		}
	} while(numL > -1);	
	
  cairo_surface_destroy(surface);
  cairo_destroy(cr);

	free(sD);
	free(sLgt);
	free(sLgtB);
	return 0;

}
