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

// PROTOS
bool eqd(double d1, double d2);

// STRUCTURES
struct sVector3d { // coordonnées 3d
	double x, y, z;
};

struct sVector2d { // coordonnées 2d
	double x, y;
};

struct sCop {
	int nF, nV;	// n° face et voisin
	double cop; // valeur de la coplanéité (0 = coplanaire)
};

struct sVoisin {
	int nF;		// n° de la face liée
	int idx;	// index du 1er point lié
};

struct sLigne  {
	int id;
	struct sVector2d p1, p2; // points (p1 : petit, p2 : grand)
	int n1, i1; // relié à triangle + index (petit)
	int n2, i2;	// relié à triangle + index (grand)
	int nb;			// nb (1 = coupe, 2 = pli)
	int nP;		// id de la page
};

struct sNAff	{
	int nMin; // petit n°
	int nMax; // grand n°
	int a;	// Affichage
};

struct sCoul {
	double r, v, b;
	};
	
const struct sCoul 
	C_NOIR	= {0,0,0},
	C_ROUGE	= {1,0,0},
	C_VERT	= {0,1,0},
	C_BLEU	= {0,0,1},
	C_MARRON= {0.501960784313725, 0.0, 0.0};

struct sLigne sLigneNew (int iPage, int id, struct sVector2d p1, struct sVector2d p2, int n1, int i1, int n2, int i2) {
	struct sLigne l;
	
	l.nP = iPage;
	l.id = id;
	l.p1 = p1;
	l.p2 = p2;
	l.n1 = n1;
	l.i1 = i1;
	l.n2 = n2;
	l.i2 = i2;
	l.nb = 1;
	
	return l;
}

// DEFINITIONS
struct sVector2d sVector2dAdd (struct sVector2d v1, struct sVector2d v2) {
	struct sVector2d r;
	r.x = v1.x + v2.x;
	r.y = v1.y + v2.y;
	
	return r;
}
struct sVector2d sVector2dSub (struct sVector2d v1, struct sVector2d v2) {
	struct sVector2d r;
	r.x = v1.x - v2.x;
	r.y = v1.y - v2.y;
	
	return r;
}
struct sVector2d sVector2dDiv (struct sVector2d v, double d) {
	struct sVector2d r;
	r.x = v.x / d;
	r.y = v.y / d;
	
	return r;
}
struct sVector2d centroid(struct sVector2d* pts) {
	struct sVector2d r;
	
	r = sVector2dAdd(pts[0], sVector2dAdd(pts[1], pts[2]));
	r = sVector2dDiv(r, 3);
	
	return r;
}

struct sVector3d sVector3dSub (struct sVector3d v1, struct sVector3d v2) {
	struct sVector3d r;
	r.x = v1.x - v2.x;
	r.y = v1.y - v2.y;
	r.z = v1.z - v2.z;
	
	return r;
}

struct sVoisin sVoisinNew (int n, int i) {
	struct sVoisin v;
	v.nF = n;
	v.idx = i;
	
	return v;
}

struct sVector2d milieu (struct sVector2d p1, struct sVector2d p2) {
	struct sVector2d r = sVector2dDiv(sVector2dAdd(p1, p2), 2);
	
	return r;
}

void d2ize (struct sVector3d p[3], struct sVector2d P[3]) {
double
	x0 = p[0].x, y0 = p[0].y, z0 = p[0].z,
	x1 = p[1].x, y1 = p[1].y, z1 = p[1].z,
	x2 = p[2].x, y2 = p[2].y, z2 = p[2].z;

	P[0].x = 0;
	P[0].y = 0;
	P[1].x = sqrt((x1 - x0)*(x1 - x0) + (y1 - y0)*(y1 - y0) + (z1 - z0)*(z1 - z0));
	P[1].y = 0;
	P[2].x = ((x1 - x0) * (x2 - x0) + (y1 - y0) * (y2 - y0) + (z1 - z0) * (z2 - z0)) 
						/ P[1].x;
	P[2].y = sqrt((x2 - x0)*(x2 - x0) + (y2 - y0)*(y2 - y0) + (z2 - z0)*(z2 - z0) 
						- P[2].x * P[2].x);
}

double isCoplanar (struct sVector3d t[3], struct sVector3d p) {
// Function to find equation of plane.
// https://www.geeksforgeeks.org/program-to-check-whether-4-points-in-a-3-d-plane-are-coplanar/
struct sVector3d 
	v1 = { t[1].x - t[0].x, t[1].y - t[0].y, t[1].z - t[0].z },
	v2 = { t[2].x - t[0].x, t[2].y - t[0].y, t[2].z - t[0].z };

double
	a = v1.y * v2.z - v2.y * v1.z,
	b = v2.x * v1.z - v1.x * v2.z,
	c = v1.x * v2.y - v1.y * v2.x,
	d = (- a * t[0].x) - (b * t[0].y) - (c * t[0].z);

// equation of plane is: a*x + b*y + c*z = 0
// checking if the 4th point satisfies  
// the above equation  
	return (a * p.x) + (b * p.y) + (c * p.z) + d;
}

int suiv (int n) {
	return n < 2 ? n + 1 : 0;
}

int prec (int n) {
	return n > 0 ? n -1 : 2;
}

double distance2d (struct sVector2d p1, struct sVector2d p2) {
	struct sVector2d d = sVector2dSub(p2, p1);
	return sqrt((d.x * d.x) + (d.y * d.y));
}

double distance3d (struct sVector3d p1, struct sVector3d p2) {
	struct sVector3d d = sVector3dSub(p2, p1);
	return sqrt((d.x * d.x) + (d.y * d.y) + (d.z * d.z));
}

bool eqd(double d1, double d2) {
	return fabs(d1 - d2) < epsilon;
}

bool eq (struct sVector2d p1, struct sVector2d p2) {
	return distance2d(p1, p2) < epsilon;
}

bool eq3 (struct sVector3d t1[3], struct sVector3d t2[3], int n) {
	return (distance3d(t2[n], t1[0]) >= epsilon)
			&& (distance3d(t2[n], t1[1]) >= epsilon)
			&& (distance3d(t2[n], t1[2]) >= epsilon);
}

double calcAngle (struct sVector2d a, struct sVector2d b, struct sVector2d c) {
	struct sVector2d ab = sVector2dSub(b, a);
	struct sVector2d ac = sVector2dSub(c, a);
				
  double rot_ab_ac = atan2(ac.y*ab.x - ac.x*ab.y, ac.x*ab.x + ac.y*ab.y);
  
  return rot_ab_ac;
}

struct sVector2d rotation (struct sVector2d c, struct sVector2d p, double angle) {
	double 
		lcos = cos(angle),
		lsin = sin(angle);

	struct sVector2d r = {
		(lcos * (p.x - c.x)) + (lsin * (p.y - c.y)) + c.x,
		(lcos * (p.y - c.y)) - (lsin * (p.x - c.x)) + c.y };
		
	return r;
}

/*function direction (p1, p2) {
	return Math.atan2(p2[1] - p1[1], p2[0] - p1[0])
}*/

double angle (struct sVector2d p1, struct sVector2d p2) {
	//double r = atan2(p1.y - p2.y, p1.x - p2.x);
	double r = atan2(p2.y - p1.y, p2.x - p1.x);
	return r;
}

int compPg (const void * el1, const void *el2) {
	struct sLigne v1 = * (const struct sLigne *) el1;
	struct sLigne v2 = * (const struct sLigne *) el2;
	
	int r = v1.nP - v2.nP;
	
	return r;
}

int compAff (const void * el1, const void *el2) {
	struct sNAff v1 = * (const struct sNAff *) el1;
	struct sNAff v2 = * (const struct sNAff *) el2;

	int r = (v1.nMax != v2.nMax) ? v1.nMax - v2.nMax : v1.nMin - v2.nMin;

	return r;
}
	
int comp (const void * el1, const void * el2) {
	struct sLigne v1 = * (const struct sLigne *) el1;
	struct sLigne v2 = * (const struct sLigne *) el2;

	int iMin1 = min(v1.n1, v1.n2);
	int iMax1 = max(v1.n1, v1.n2);
	int iMin2 = min(v2.n1, v2.n2);
	int iMax2 = max(v2.n1, v2.n2);
	
	if (iMax1 == iMax2)
		return iMin1 - iMin2;
	else
		return iMax1 - iMax2;
}

bool li (struct sVector2d l1S, struct sVector2d l1E, 
				 struct sVector2d l2S, struct sVector2d l2E) {
	// true if the lines intersect
	if (eq(l1S, l2S) || eq(l1S, l2E) || eq(l1E, l2S) || eq(l1E, l2E)) {
		return false;
	}

	double denominator = ((l2E.y - l2S.y) * (l1E.x - l1S.x))
										 - ((l2E.x - l2S.x) * (l1E.y - l1S.y));

	if (denominator == 0) {
		return false;
	}

	double 
		a = l1S.y - l2S.y,
		b = l1S.x - l2S.x,
		numerator1 = ((l2E.x - l2S.x) * a) - ((l2E.y - l2S.y) * b),
		numerator2 = ((l1E.x - l1S.x) * a) - ((l1E.y - l1S.y) * b);
	a = numerator1 / denominator;
	b = numerator2 / denominator;

	if ((a > 0) && (a < 1) && (b > 0) && (b < 1)) {
		return true;
	} else {
		return false;
	}
}

bool overlap (struct sVector2d *t1, struct sVector2d *t2) {
	bool r = 
		 li(t1[0], t1[1], t2[0], t2[1])
	|| li(t1[0], t1[1], t2[1], t2[2])
	|| li(t1[0], t1[1], t2[2], t2[0])
	|| li(t1[1], t1[2], t2[0], t2[1])
	|| li(t1[1], t1[2], t2[1], t2[2])
	|| li(t1[1], t1[2], t2[2], t2[0])
	|| li(t1[2], t1[0], t2[0], t2[1])
	|| li(t1[2], t1[0], t2[1], t2[2])
	|| li(t1[2], t1[0], t2[2], t2[0]);

	return r;
}

void afficheNum(cairo_t *cr, 
	int num, 
	struct sVector2d p1, 
	struct sVector2d p2, 
	struct sCoul c) {
  
  cairo_text_extents_t te;
	char ch[10];
	struct sVector2d m;
	double fheight = -3, nx, ny, a;
	
	cairo_set_source_rgb(cr, c.r, c.v, c.b);
	cairo_save(cr);
	sprintf(ch, "%d", num);
	m = milieu(p1, p2);
	cairo_text_extents (cr, ch, &te);
  nx = -te.width / 2.0;
  ny = fheight / 2;
  a = angle(p1, p2) - M_PI;
	cairo_translate(cr, m.x, m.y);
	cairo_rotate(cr, a);
	cairo_translate(cr, nx, ny);
	cairo_move_to(cr, 0,0);
	cairo_show_text(cr, ch);
	cairo_restore(cr);
}

char *litFichierTexte (const char *nomFichier) {
	char *texte;
	FILE *textfile;
	long numbytes;

	textfile = fopen(nomFichier, "r");
	if (textfile == NULL)
		return NULL;
    
	fseek(textfile, 0L, SEEK_END);
	numbytes = ftell(textfile);
	fseek(textfile, 0L, SEEK_SET);	

	texte = (char*)calloc(numbytes, sizeof(char));	
	if (texte == NULL)
		return NULL;

	fread(texte, sizeof(char), numbytes, textfile);
	fclose(textfile);
	
	return texte;
}

void calcBoiteEnglobante(struct sVector2d b[2], struct sVector2d *pts, int nbP) {
	// calcul de la boite englobante
	b[0] = (struct sVector2d) {10000,10000};
	b[1] = (struct sVector2d) {-10000,-10000};
	for (int i = 0; i < nbP; i++){
		struct sVector2d p = pts[i];
		if (b[0].x > p.x)b[0].x = p.x;
		if (b[0].y > p.y)b[0].y = p.y;
		if (b[1].x < p.x)b[1].x = p.x;
		if (b[1].y < p.y)b[1].y = p.y;
	}
}

int compteDispo(bool *l, int nb) {
	int n = 0;
	for (int i = 0; i < nb; i++) {
		if (l[i])
			n++;
	}
return n;	
}

int premDispo(bool *l, int nb) {
int n = -1, i = 0;
do {
	if (l[i])
		n = i;
	i++;	
} while ((i < nb) &&  (n == -1));

return n;
}


void faitLigne(cairo_t *cr, struct sVector2d p1, struct sVector2d p2, int typeL)
{
	if (typeL != L_PLI_C) { // pas de ligne si pli coplanaire
		struct sCoul c;
		static const double tiret[] = {10.0};
		static const double tpoint[] = {8.0,2.0,2.0,2.0};

		printf("faitLigne : %d\n", typeL);
		if (typeL == L_COUPE) {
			c = C_ROUGE;
			cairo_set_dash(cr, tiret, 0, 0);
		} else if (typeL == L_PLI_M) {
			c = C_MARRON;
			cairo_set_dash(cr, tiret, 1, 0);
		} else {
			c = C_VERT;
			cairo_set_dash(cr, tpoint, 4, 0);
		}
	
		cairo_set_source_rgb(cr, c.r, c.v, c.b);
 
		cairo_move_to(cr, p1.x, p1.y);
		cairo_line_to(cr, p2.x, p2.y);
		cairo_stroke(cr);
	}
}

int main(void) {
	char OBJ[32];
	printf("Nom fichier :");
	scanf("%s", OBJ);
	puts("");

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
	double f[4];											// point courant
  
  int nbF = 0;											// nb de faces
  int* faces0 = NULL;								// tableau des faces
  int t[3];													// face courante
    
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
			faces0 = (int*)realloc(faces0, (nbF+1)*3*sizeof(int));
			for (int i = 0; i < 3; i++) {
				faces0[nbF*3+i] = t[i];
			}
			nbF++;
		}
	}
	free(tok);
	
	int faces[nbF][3], n = 0;
	for (int i = 0; i < nbF; i++){
		for (int j = 0; j < 3; j++){
			faces[i][j] = faces0[n++];
		}
	}
	free(faces0);
	
	// VOISINS
	struct sVoisin voisins[nbF][3];
	for (int i = 0; i < nbF; i++) {
		for (int j = 0; j < 3; j++) {		
			int vi = 0;
			bool ok = false;
			do {
				if (vi != i) {
					for (int k = 0; (k < 3) && (!ok); k++) {
						if ( (faces[vi][k] == faces[i][suiv(j)]) 
							&& (faces[vi][suiv(k)] == faces[i][j])) {
							voisins[i][j]= sVoisinNew(vi, suiv(k));
							ok = true;
						}
					}
					if (!ok)
						vi++;
				} else
					vi++;
			} while ((vi < nbF) && (!ok));
		}
	}
	
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
	// estCop < 0 ? "maroon" : "green" : null
	struct sCop tCop[nbF*3];
	int nbCop = 0;
	for (int i = 0; i < nbF; i++){
		for (int j = 0; j < 3; j++){
			int nV = voisins[i][j].nF;
			struct sVector3d p;
			
			if (eq3(v3d[i], v3d[nV], 0)) {
				p = v3d[nV][0];
			} 
			else if (eq3(v3d[i], v3d[nV], 1)) {
				p =  v3d[nV][1];
			}
			else {
				p =  v3d[nV][2];
			}
				
			double c = isCoplanar(v3d[i], p);
			tCop[nbCop].cop = c;
			tCop[nbCop].nF =  i;
			tCop[nbCop].nV = nV;
			nbCop++;
		}
	}
	
  struct sVector2d marge = {10, 10};
  struct sVector2d formats[6] = {
		{2380,	3368},	// A0
		{1684,	2380},	// A1
		{1190,	1684},	// A2
		{ 842,	1190},	// A3
		{ 595,   842},	// A4
		{ 421,   595},	// A5
	};

	printf("Format A (0..5) :");
  int fc = 3;
  scanf("%d", &fc);
  
  struct sVector2d limitePage = sVector2dSub(formats[fc], marge);
	
	//float lt = 2; // largeur triangle

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
	
	int tc = 0; // triangle courant
	//printf("1er triangle:");
	//scanf("%d", &tc);
	
	int nbP = 0;
	struct sLigne lignes[nbF *3];
	int nbL = 0;

do {	
	int nbTp =0;
	int tcn = 0;
	page[nbTp++] = tc;
	dispo[tc] = false;
	bool ok;
	do {
		for (int vi =0; vi < 3; vi++){
			struct sVoisin v = voisins[tc][vi];
			int vc = v.nF;
			ok = dispo[vc];
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
	
	// tri
	qsort(lignes, nbL, sizeof(struct sLigne), comp);
	// renum
	for (int i = 0; i < nbL; i++)
		lignes[i].id = i;
	// suppression doublons
	int dOK = 0;
	for (int i = 1; i < nbL; i++){
		if ((lignes[dOK].nP == lignes[i].nP) 
		&& ((eq(lignes[dOK].p1, lignes[i].p1) && eq(lignes[dOK].p2, lignes[i].p2))
		    || (eq(lignes[dOK].p1, lignes[i].p2) && eq(lignes[dOK].p2, lignes[i].p1)))
		) {
			lignes[i].id = -1;
			lignes[dOK].nb++;		
		}
		else
			dOK = i;
	}
	
	tc = premDispo(dispo, nbF);
	nbP++;
}while(tc > -1);

	qsort(lSNA, nAff, sizeof(struct sNAff), compAff);
	qsort(lignes, nbL, sizeof(struct sLigne), compPg);

	int lc = 0;
	int nA;
	int typeL;
	for (int i = 0; i < nbL; i++){
		struct sLigne l = lignes[i];
		if (l.id > -1) {
			if (lc != l.nP) {
				lc = l.nP;
				cairo_show_page(cr);
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
				printf("%d : %f\n", typeL, c);
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
				afficheNum(cr, nA, l.p1, l.p2, C_NOIR);
			}
		}
	}
	
  cairo_surface_destroy(surface);
  cairo_destroy(cr);

	return 0;
}
