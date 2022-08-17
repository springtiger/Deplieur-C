// edition d'un d‚pliage (fichier .dep) en C
// v0.1
// fait :
//	- charge fichier .dep, puis fichier .obj li‚
//  - extrait points et faces
//	- trouve les voisins de chaque arˆte
//  - calcule les coplaneit‚s
//	- cr‚e le fichier des languettes (.lng) si besoin
//	- edite les languettes
//	- cr‚e le gabarit (.pdf)

#define epsilon 0.0001

#define L_COUPE 1
#define L_PLI_M 2
#define L_PLI_V 3
#define L_PLI_C	4

#define MAX_TAMPON 100
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#ifdef WIN32
#include "cairo.h"
#include "cairo-pdf.h"
#define _USE_MATH_DEFINES
#else
#include <cairo/cairo.h>
#include <cairo/cairo-pdf.h>
#endif

#include <math.h>
#include "deputils.c"

int main(void) {
	// charge donn‚es
	int nbVarsLues;
	char* nomFichierDonnees = "donnees.dep";
	FILE* fd;
	int rc;
	fd = fopen(nomFichierDonnees, "r");
	if (fd == NULL) {
		return -1;
	}

	char OBJ[50] = { 0 };
	double ech;
	int fc;
	int tc0;
	float fontSize;
	int hLang;

	nbVarsLues = fscanf(fd, "%s", OBJ);
	nbVarsLues = fscanf(fd, "%lf", &ech);
	nbVarsLues = fscanf(fd, "%2d", &fc);
	nbVarsLues = fscanf(fd, "%4d", &tc0);
	nbVarsLues = fscanf(fd, "%f", &fontSize);
	nbVarsLues = fscanf(fd, "%2d", &hLang);
	int nbD = 0;
	struct sDepliage* sD = NULL;
	struct sDepliage sD0;

	int d0, d1, d2;
	while ((nbVarsLues = fscanf(fd, "%d", &d0)) > 0) {
		if (d0 == -1) {
			nbVarsLues = fscanf(fd, "%d", &d1);
			nbVarsLues = fscanf(fd, "%d", &d2);
		}
		else {
			nbVarsLues = fscanf(fd, "%d", &d1);
			d2 = 0;
		}
		sD0.orig = d0;
		sD0.face = d1;
		sD0.a = d2;
		struct sDepliage* tmp = (struct sDepliage*)realloc(sD, sizeof(struct sDepliage) * (nbD + 1));
		if (tmp != NULL)
		{
			sD = tmp;
			sD[nbD] = sD0;
			nbD++;
		}
	}
	rc = fclose(fd);
	if (rc == EOF) {
		return -1;
	}

	// charge donn‚es languettes
	int nbLgt = 0;
	struct sLang* sLgt = NULL;

	int nbLgtB = 0;
	struct sLang* sLgtB = NULL;

	struct sLang sLgt0;

	char* nomFichierLanguettes = "donnees.lng";
	fd = fopen(nomFichierLanguettes, "r");
	if (fd != NULL) {
		while (nbVarsLues = fscanf(fd, "%d", &d0) > 0) {
			nbVarsLues = fscanf(fd, "%d", &d1);
			nbVarsLues = fscanf(fd, "%d", &d2);
			sLgt0.n1 = d0;
			sLgt0.n2 = d1;
			sLgt0.v = d2;
			struct sLang* tmp = (struct sLang*)realloc(sLgt, sizeof(struct sLang) * (nbLgt + 1));
			if (tmp != NULL)
			{
				sLgt = tmp;
				sLgt[nbLgt] = sLgt0;
				nbLgt++;
			}
		}
		rc = fclose(fd);
		if (rc == EOF) {
			return -1;
		}
	}

	printf("nb liens     : %d\n", nbD);
	printf("Nom fichier  :%s\n", OBJ);
	printf("Echelle      : %lf\n", ech);
	printf("format page  : A%d\n", fc);
	printf("taille nums  : %f\n", fontSize);
	printf("hauteur lang : %d\n", hLang);
	printf("1er triangle : %d\n", tc0);
	printf("--------- Chargement : %s ---------\n", OBJ);
	struct sVector3d* sommets = NULL;
	int nbSommets = 0;

	int gc = 0; // groupe courant

	int* faces0 = NULL;
	int nbFaces = 0;
	size_t pos;

	FILE* fs;
	fs = fopen(OBJ, "r");
	if (fs == NULL) {
		perror("Erreur ouverture fichier");
		exit(0);
	}

	char* tampon;
	char* ligneLue;
	do {
		int fc[4];
		tampon = malloc(sizeof(char) * MAX_TAMPON);
		ligneLue = fgets(tampon, MAX_TAMPON, fs);
		char tL0 = tampon[0];
		char tL1 = tampon[1];
		if ((tL0 == 'v') && (tL1 == ' ')) { // v = vector (sommet)
			double v[3];
			nbVarsLues = sscanf(ligneLue, "v %lf %lf %lf", &v[0], &v[1], &v[2]);
			struct sVector3d s = { v[0] * ech, v[1] * ech, v[2] * ech };
			struct sVector3d* tmp;
			tmp = (struct sVector3d*)realloc(sommets, sizeof(struct sVector3d) * (nbSommets + 1));
			if (tmp != NULL)
			{
				sommets = tmp;
				sommets[nbSommets++] = s;
			}
		}
		else if ((tL0 == 'g') && (tL1 == ' ')) { // g = group (groupe)
			gc++;
		}
		else if ((tL0 == 'f') && (tL1 == ' ')) { // f = face
			char c[3][20];
			nbVarsLues = sscanf(ligneLue, "f %s %s %s", c[0], c[1], c[2]);
			for (int i = 0; i < 3; i++) {
				pos = strspn(c[i], "0123456789");
				char* ch = malloc(pos + 1);
				if (ch != NULL)
				{
					strncpy(ch, c[i], pos);
					ch[pos] = '\0';
					fc[i] = atoi(ch);
				}
				free(ch);
			}
			fc[3] = gc;
			size_t n = (nbFaces + 1) * 4;
			int* tmp = (int*)realloc(faces0, (size_t)sizeof(int) * n);
			if (tmp != NULL)
			{
				faces0 = tmp;
				for (int i = 0; i < 4; i++) {
					faces0[nbFaces * 4 + i] = fc[i] - 1;
				}
				nbFaces++;
			}
		}
	} while (ligneLue != NULL);
	if (fclose(fs))
		perror("erreur fermeture fichier");

	free(tampon);

	int(*faces)[4];
	faces = calloc(nbFaces, sizeof * faces);
	int n = 0;
	for (int i = 0; i < nbFaces; i++) {
		for (int j = 0; j < 4; j++)
			faces[i][j] = faces0[n++];
	}
	free(faces0);

	// VOISINS
	struct sVoisin(*voisins)[3] = calloc(nbFaces, sizeof * voisins);
	calculeVoisinage(faces, nbFaces, voisins);

	// V3D V2D
	struct sVector3d(*v3d)[3] = calloc(nbFaces, sizeof * v3d);
	struct sVector2d(*v2d)[3] = calloc(nbFaces, sizeof * v2d);
	for (int i = 0; i < nbFaces; i++) {
		for (int j = 0; j < 3; j++) {
			v3d[i][j] = sommets[faces[i][j]];
		}
		d2ize(v3d[i], v2d[i]);
	}
	free(sommets);

	// estCOP
	struct sCop(*tCop) = calloc(nbFaces * 3, sizeof * tCop);
	calculeCop(nbFaces, voisins, tCop, v3d);
	// fin chargement	

	struct sVector2d formats[6] = {
		{2380,	3368},	// A0
		{1684,	2380},	// A1
		{1190,	1684},	// A2
		{ 842,	1190},	// A3
		{ 595,   842},	// A4
		{ 421,   595},	// A5
	};
	struct sVector2d marge = { 10, 10 };
	//struct sVector2d limitePage = sVector2dSub(formats[fc], marge);

	// nb elements
	printf("%d points - %d faces\n", nbSommets, nbFaces);

	// DEBUT DEPLIAGE

	puts("Afficher le nø des faces (O/N) ?");
	char chAffNumFace;
	_Bool bAffNumFace;
	nbVarsLues = scanf(" %c", &chAffNumFace);
	if (nbVarsLues == 1) {
		bAffNumFace = toupper(chAffNumFace) == 'O';
	}

	// INITs PDF
	cairo_surface_t* surface;
	cairo_t* cr;

	surface = cairo_pdf_surface_create("gabarit.pdf", formats[fc].x, formats[fc].y);
	cr = cairo_create(surface);

	cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL,
		CAIRO_FONT_WEIGHT_NORMAL);
	cairo_set_font_size(cr, fontSize);
	cairo_set_line_width(cr, 1);

	struct sNAff(*lSNA) = calloc(nbFaces * 3, sizeof * lSNA);
	int nAff = 0;

	int nbP = -1;
	struct sVector2d* vMin = NULL;
	struct sLigne(*lignes) = calloc(nbFaces * 3, sizeof * lignes);
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
			struct sVector2d * tmp = (struct sVector2d*)realloc(vMin, sizeof(struct sVector2d) * (nbP + 1));
			if (tmp != NULL)
			{
				vMin = tmp;
				vMin[nbP] = vPetit(v2d[f][0], vPetit(v2d[f][1], v2d[f][2]));
			}
		}
		sD[i].page = nbP;

		if (sD[i].orig > -1) {
			int tc = sD[i].orig;
			int vc = sD[i].face;
			int vi = voisins[tc][0].nF == vc ? 0 :
				voisins[tc][1].nF == vc ? 1 : 2;
			struct sVoisin v = voisins[tc][vi];

			// 1ø) rapproche v2d[vc] de v2d[tc]
			struct sVector2d deltaV = sVector2dSub(v2d[tc][vi], v2d[vc][v.idx]);
			for (int n = 0; n < 3; n++)
				v2d[vc][n] = sVector2dAdd(v2d[vc][n], deltaV);
			// 2ø) tourne v2d[vc]
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

	struct sAN(*lAN) = calloc(nbFaces, sizeof * lAN);
	int nbAN = 0;
	int ppc = 0;

	for (int i = 0; i < nbL; i++) {
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

			double c = tCop[(l.n1 * 3) + l.i1].cop;
			if (l.nb == 1) {
				struct sLang l0 = { .n1 = l.n1, .n2 = l.n2 };
				struct sLang* rL;
				if (nbLgt > 0) {
					rL = (struct sLang*)bsearch(&l0, sLgt, nbLgt,
						sizeof(struct sLang), compLang);
					if (rL == NULL) {
						typeL = L_COUPE;
					}
					else if (rL->v == 0) {
						typeL = L_COUPE;
					}
					else {
						if (fabs(c) < 10e-7) {
							typeL = L_LGT_C;
						}
						else {
							typeL = c < 0 ? L_LGT_M : L_LGT_V;
						}
					}
				}
				else {
					typeL = L_COUPE;
				}

				l0.v = l.n1 < l.n2 ? 1 : 0;
				struct sLang* tmp = (struct sLang*)realloc(sLgtB, sizeof(struct sLang) * (nbLgtB + 1));
				if (tmp != NULL)
				{
					sLgtB = tmp;
					sLgtB[nbLgtB] = l0;
					nbLgtB++;
				}
			}
			else {
				if (fabs(c) < 10e-7) {
					typeL = L_PLI_C;
				}
				else {
					typeL = c < 0 ? L_PLI_M : L_PLI_V;
				}
			}
			if (typeL != L_PLI_C) {
				faitLigne(cr, l.p1, l.p2, typeL, hLang);
			}
			if (l.nb == 1) {
				struct sNAff cleN;
				struct sNAff* rechN;
				cleN.nMax = max(l.n1, l.n2);
				cleN.nMin = min(l.n1, l.n2);
				rechN = (struct sNAff*)bsearch(&cleN, lSNA, nAff,
					sizeof(struct sNAff), compAff);
				if (rechN != NULL)
					nA = rechN->a;
				else {
					cleN.a = nAff;
					nA = nAff;
					lSNA[nAff++] = cleN;
					qsort(lSNA, nAff, sizeof(struct sNAff), compAff);
				}
				struct sAN sAN0 = { nA, l.p1, l.p2 };
				lAN[nbAN++] = sAN0;
			}
		}
	}

	txtPage = (char*)malloc(20 * sizeof(char));
	if (txtPage != NULL)
	{
		sprintf(txtPage, "page %d (%d)", lc + 1, faces[lignes[nbL - 1].n1][3]);
		printf("pages: %s\n", txtPage);
		free(txtPage);
	}

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

	puts("Cr‚er fichier languettes (O/N) ?");
	char repLng;

	nbVarsLues = scanf(" %c", &repLng);
	if (nbVarsLues == 1) {
		if (toupper(repLng) == 'O') {
			puts("Type de languettes");
			puts(" 0 : sans languettes");
			puts(" 1 : 1 languette par paire");
			puts(" 2 : 2 languettes par paire");
			int repTLng;
			nbVarsLues = scanf("%d", &repTLng);
			if ((repTLng >= 0) && (repTLng <= 2))
				sauveLanguettes(sLgtB, nbLgtB, repTLng);
		}
	}

	qsort(lSNA, nAff, sizeof(struct sNAff), compAffa);

	int numL;
	do {
		puts("Inverser languette (nø sinon -1) ?");
		nbVarsLues = scanf("%d", &numL);
		if (nbVarsLues == 1) {
			if (numL > -1) {
				struct sNAff cleN;
				struct sNAff* rechN;
				cleN.a = numL;
				rechN = (struct sNAff*)bsearch(&cleN, lSNA, nAff,
					sizeof(struct sNAff), compAffa);
				if (rechN != NULL) {
					struct sLang cleL;
					struct sLang* rechL;
					cleL.n1 = rechN->nMax;
					cleL.n2 = rechN->nMin;
					rechL = (struct sLang*)bsearch(&cleL, sLgt, nbLgt,
						sizeof(struct sLang), compLang);
					if (rechL != NULL) {
						rechL->v = 1 - rechL->v;
					}
					cleL.n1 = rechN->nMin;
					cleL.n2 = rechN->nMax;
					rechL = (struct sLang*)bsearch(&cleL, sLgt, nbLgt,
						sizeof(struct sLang), compLang);
					if (rechL != NULL) {
						rechL->v = 1 - rechL->v;
						sauveLanguettes(sLgt, nbLgt, M_LANG_SAUV);
					}
				}
			}
		}
	} while (numL > -1);

	cairo_surface_destroy(surface);
	cairo_destroy(cr);

	free(sD);
	free(sLgt);
	free(sLgtB);
	free(faces);
	free(v3d);
	free(v2d);
	free(tCop);
	free(lSNA);
	free(lignes);
	free(lAN);

	return 0;
}
