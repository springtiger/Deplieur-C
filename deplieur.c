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

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include <string.h>
#include <cairo/cairo.h>
#include <cairo/cairo-pdf.h>
#include "deputils.c"

int main(void) {
	char OBJ[32] = "bb";
	printf("Nom fichier :");
	scanf("%s", OBJ);

	float ech;
	printf("Echelle :");
	scanf("%f", &ech);

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
	
  struct sVector2d marge = {10, 10};
  struct sVector2d formats[6] = {
		{2380,	3368},	// A0
		{1684,	2380},	// A1
		{1190,	1684},	// A2
		{ 842,	1190},	// A3
		{ 595,   842},	// A4
		{ 421,   595},	// A5
	};

	printf("Format A(0..5) :");
  int fc;
  scanf("%d", &fc);
  
  struct sVector2d limitePage = sVector2dSub(formats[fc], marge);
	
	// nb elements
	printf("%d points\n%d faces\n", nbV, nbF);
  
	// DEBUT DEPLIAGE

  // INITs PDF
  cairo_surface_t *surface;
  cairo_t *cr;

  surface = cairo_pdf_surface_create("output.pdf", formats[fc].x, formats[fc].y);
  cr = cairo_create(surface);
  
  cairo_select_font_face (cr, "Sans", CAIRO_FONT_SLANT_NORMAL,
		CAIRO_FONT_WEIGHT_NORMAL);
  cairo_set_font_size (cr, 11.0);
  cairo_set_line_width(cr, 1);

	bool dispo[nbF];
	for (int i = 0; i < nbF; i++)dispo[i] = true;
	
	struct sNAff lSNA [nbF*3];
	int nAff = 0;

	int page[nbF];
	struct sDepliage sD[nbF]; // depliage
	int nbD = 0; // nb de faces dépliées
	
	int tc0; // triangle courant
	printf("1er triangle:");
	scanf("%d", &tc0);
	
	int tc = tc0;
	int nbP = 0;
	struct sLigne lignes[nbF *3];
	int nbL = 0;

do {
	gc = faces[tc][3];	
	int nbTp =0;
	int tcn = 0;
	page[nbTp++] = tc;
	struct sDepliage sdC = {nbP, tc, -1};
	sD[nbD++] = sdC;
	dispo[tc] = false;
	bool ok;
	do {
		for (int vi =0; vi < 3; vi++){
			struct sVoisin v = voisins[tc][vi];
			int vc = v.nF;
			ok = dispo[vc] && (faces[vc][3] == gc);
			if (ok) {
				for (int i = 0; (i < nbTp) && ok; i++){
					if (page[i] == vc) ok = false;
				}
			}
			if (ok) {
				// 1°) rapproche v2d[vc] de v2d[tc]
				struct sVector2d deltaV = sVector2dSub(v2d[tc][vi], v2d[vc][v.idx]);
				for (int i = 0; i < 3; i++)
					v2d[vc][i] = sVector2dAdd(v2d[vc][i], deltaV);
				// 2°) tourne v2d[vc]
				double a = calcAngle(v2d[tc][vi], v2d[tc][suiv(vi)], v2d[vc][prec(v.idx)]);
				for (int i = 0; i < 3; i++)
					v2d[vc][i] = rotation(v2d[tc][vi], v2d[vc][i], a);
				// 3°) vérifie qu'ajouter v2d[vc] à la pièce tient dans la page
				int nbTV = (nbTp+1) * 3;
				struct sVector2d tmp[nbTV];
				for (int i = 0; i < nbTp; i++){
					for (int j = 0; j < 3; j++)
						tmp[i*3+j] = v2d[page[i]][j];
				}
				for (int j = 0; j < 3; j++)
					tmp[nbTp*3 +j] = v2d[vc][j];
				struct sVector2d vb[2];
				calcBoiteEnglobante(vb, tmp, nbTV);
				struct sVector2d dV = sVector2dSub(vb[1], vb[0]);
				if ((dV.x > limitePage.x) || (dV.y > limitePage.y))
					ok = false;
				// 4°) vérifie que v2d[vc] n'est pas en collision avec la pièce
				if (ok) {
					for (int i = 0; (i < nbTp) && ok; i++) {
						if (overlap(v2d[page[i]], v2d[vc]))
							ok = false;
					}
				}
				// 5°) OK
				if (ok) {
					page[nbTp++] = vc;
					struct sDepliage sdC = {nbP, vc, tc, 0};
					sD[nbD++] = sdC;
					dispo[vc] = false;
				}
			}
		}
		// rech prochaine face à déplier
		tcn++;
		if (tcn < nbTp){
			tc = page[tcn];
			ok = true;
		}
		else
			ok = false;
	} while (ok);
	
	struct sVector2d tmp[nbTp*3];
	for (int i = 0, k = 0; i < nbTp; i++){
		for (int j = 0; j < 3; j++){
			tmp[k++] = v2d[page[i]][j];
		}
	}
	
	struct sVector2d b[2];
	calcBoiteEnglobante(b, tmp, nbTp*3);	
	// ajustement en bas à gauche
	for (int i = 0; i < nbTp; i++){
		for (int j = 0; j < 3; j++)
			v2d[page[i]][j] = sVector2dSub(v2d[page[i]][j], 
				sVector2dSub(b[0], marge));
	}
	// répartition en lignes
	for (int i = 0; i < nbTp; i++){
		for (int j = 0; j < 3; j++){		
			lignes[nbL] = sLigneNew(
			nbP, nbL, 
				v2d[page[i]][j], v2d[page[i]][suiv(j)],
				page[i], j, 
				voisins[page[i]][j].nF, voisins[page[i]][j].idx);

			nbL++;
		}
	}
	
	supprimeDoublons(lignes, nbL);

	tc = premDispo(dispo, nbF);
	nbP++;
}while(tc > -1);

	qsort(lSNA, nAff, sizeof(struct sNAff), compAff);
	qsort(lignes, nbL, sizeof(struct sLigne), compPg);
	
	sauveDonnees(OBJ, ech, fc, tc0, sD, nbD);

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
				txtPage = (char *)malloc(20 * sizeof(char));
				cairo_move_to(cr, 0, limitePage.y-20);
				sprintf(txtPage, "page %d (%d)", lc, faces[l.n1][3]);
				cairo_set_source_rgb(cr, C_NOIR.r, C_NOIR.v, C_NOIR.b);
				cairo_show_text(cr, txtPage);
				free(txtPage);
				
				if (nbAN > 0) {
					afficheNumsPage(cr, lAN, nbAN, v2d);
					for (int sdi = 0; sdi < nbD; sdi++) {
						struct sDepliage sdt = sD[sdi];
						if (sdt.page == ppc) {
							struct sVector2d m = centroid(v2d[sdt.face]);
							afficheNum(cr, sdt.face, m, m, C_VERT);
						}
					}
				}
				nbAN = 0;
				cairo_show_page(cr);
				ppc++;
			}	
			if (l.nb == 1)
				typeL = L_COUPE;
			else {
				double c = tCop[(l.n1*3)+l.i1].cop;
				if (fabs(c) < 10e-7) {
					typeL = L_PLI_C;
				} else {
					typeL = c < 0 ? L_PLI_M : L_PLI_V;
				}
			}
			if (typeL != L_PLI_C)
				faitLigne(cr, l.p1, l.p2, typeL);
			if (l.nb == 1) {
				struct sNAff cleN;
				struct sNAff *rechN;
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
				struct sAN sAN0 = {nA, l.p1, l.p2};
				lAN[nbAN++] = sAN0;
			}
		}
	}
	
	txtPage = (char *)malloc(20 * sizeof(char));
	cairo_move_to(cr, 0, limitePage.y-20);
	sprintf(txtPage, "page %d (%d)", lc+1, faces[lignes[nbL-1].n1][3]);
	printf("pages: %s\n", txtPage);
	cairo_set_source_rgb(cr, C_NOIR.r, C_NOIR.v, C_NOIR.b);
	cairo_show_text(cr, txtPage);
	free(txtPage);
	
	if (nbAN > 0) {
		afficheNumsPage(cr, lAN, nbAN, v2d);
		
		for (int sdi = 0; sdi < nbD; sdi++) {
			struct sDepliage sdt = sD[sdi];
			if (sdt.page == ppc) {
				struct sVector2d m = centroid(v2d[sdt.face]);
				afficheNum(cr, sdt.face, m, m, C_VERT);
			}
		}
	}
  cairo_surface_destroy(surface);
  cairo_destroy(cr);

	return 0;
}
