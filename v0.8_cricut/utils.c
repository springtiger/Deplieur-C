// Depliage : structures et fonctions

#define epsilon 0.0001

#define MAX_TAMPON 100
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>


#ifdef WIN32
#include "depl_txtW.h"
#include "cairo.h"
#include "cairo-pdf.h"
#define _USE_MATH_DEFINES
#else
#include "depl_txt.h"
#include <cairo/cairo.h>
#include <cairo/cairo-pdf.h>
#endif

#include <math.h>
#include <locale.h>

static const double tiret[] = {20.0};
static const double tpoint[] = {8.0,2.0,2.0,2.0};

// STRUCTURES

typedef struct { // menu
  int nSection;
  const char *nom;
  const char *texte;
  const char *com;
  void *action;
} lmenu;

typedef struct { // coordonnées 3d
	double x, y, z;
} Vector3d;

typedef struct { // coordonnées 2d
	double x, y;
} Vector2d;

typedef struct { // rect
  int id;
  Vector2d p1;
  Vector2d p2;
} rectT;

typedef struct { // coplanarité
	int nF, nV;	// n° face et voisin
	double cop; // valeur de la coplanarité (0 = coplanaire)
} Cop;

typedef struct { // voisinage
	int nF;		// n° de la face liée
	int idx;	// index du 1er point lié
} Voisin;

typedef struct { // ligne d'affichage
	int id;
	Vector2d p1, p2; // points (p1 : petit, p2 : grand)
	int n1, i1; // relié à triangle + index (petit)
	int n2, i2;	// relié à triangle + index (grand)
	int nb;			// nb (1 = coupe, 2 = pli)
	int nP;			// id de la page
	int nA;     // n° d'affichage
} Ligne;

typedef struct {	// numérotation des paires d'arêtes
	int nMin; // petit n°
	int nMax; // grand n°
	int a;		// Affichage
} NAff;

typedef struct {	// couleur
	double r, v, b;
} Coul;

typedef struct { // dépliage
	int page;
	int piece;
	int face;
	int orig;
	int a;      // angle
	Vector2d d; // coordonnées Décalage
} Depliage;

typedef struct { // numérotation d'affichage (doublon?)
	int n;
	Vector2d p1, p2;
} AN;

typedef struct { // languette
	int n1;
	int n2;
	int o;
	int v;
} Lang;

typedef struct { // données d'une pièce
  int page;
  int id;
  Vector2d c;
} Piece;

typedef struct { // données de dépliage
	char fichierOBJ[128];
	float echelle;
	int formatPage;
	int typeLang;
	int premierTriangle;
	float tailleNums;
	int hauteurLang;

	int nbFaces;
	int nbSommets;

	Vector3d (* v3d)[3];
	Vector2d (* v2d)[3];
	int (*faces)[4];
	Voisin (* voisins)[3];
	Cop *tCop;
	_Bool *dispo;
	Depliage * sD;
	int nbD;
	AN *lAN;
	int nbAN;
	NAff *lSNA;
	int nAff;
	int *page;
	int nbP;
	int nbPP;
	Ligne *lignes;
	int nbL;
  Lang *sLgt;
  int nbLgt;
  rectT *rects;
  int nbRects;
  int mode;
  int idCourant;

	cairo_surface_t* surface;
	cairo_t* cr;
  GtkWidget *doc;
  GtkWidget **dpage;
} DonneesDep;

#ifndef WIN32
	#define max(a,b) (a>=b?a:b)
	#define min(a,b) (a<=b?a:b)
#endif

#define L_COUPE 1
#define L_PLI_M 2
#define L_PLI_V 3
#define L_PLI_C	4
#define L_LGT_M	5
#define L_LGT_V	6
#define L_LGT_C	7

#define M_LANG_SAUV		3
#define M_LANG_CREA0	0
#define M_LANG_CREA1	1
#define M_LANG_CREA2	2

#define ME_PIECE_DEPLACER 1
#define ME_PIECE_TOURNER  2
#define ME_PIECE_CHOISIR  3
#define ME_LANG_INVERSER  4
#define ME_FACE_RATTACHER 5
#define ME_FACE_DETACHER  6

#define MODE_PIECE  1
#define MODE_ARETE  2
#define MODE_LANG   3

GdkRGBA c_ROUGE   = { 1,  0,  0,  1};
GdkRGBA c_BLEU    = { 0,  0,  1,  1};
GdkRGBA c_VERT    = { 0,  1,  0,  1};
GdkRGBA c_BLANC   = { 1,  1,  1,  1};
GdkRGBA c_NOIR    = { 0,  0,  0,  1};
GdkRGBA c_MARRON  = { 0.5,0,  0,  1};

const Coul
	C_NOIR	= {0, 0, 0},
	C_ROUGE	= {1, 0, 0},
	C_VERT	= {0, 1, 0},
	C_BLEU	= {0, 0, 1},
	C_MARRON= {0.5, 0, 0};

const double pi = 3.14159265358979323846;

// FONCTIONS
_Bool eqd(double d1, double d2) {
	return fabs(d1 - d2) < epsilon;
}

Ligne LigneNew (int iPage, int id, Vector2d p1, Vector2d p2,
	int n1, int i1, int n2, int i2, int nA) {
	Ligne l;

	l.nP = iPage;
	l.id = id;
	l.p1 = p1;
	l.p2 = p2;
	l.n1 = n1;
	l.i1 = i1;
	l.n2 = n2;
	l.i2 = i2;
	l.nb = 1;
	l.nA = nA;

	return l;
}

Vector2d Vector2dNew(double x, double y) {
	Vector2d r;

	r.x = x;
	r.y = y;

	return r;
}

Vector2d Vector2dAdd (Vector2d v1, Vector2d v2) {
	Vector2d r;

	r.x = v1.x + v2.x;
	r.y = v1.y + v2.y;

	return r;
}

Vector2d Vector2dSub (Vector2d v1, Vector2d v2) {
	Vector2d r;

	r.x = v1.x - v2.x;
	r.y = v1.y - v2.y;

	return r;
}

Vector2d Vector2dDiv (Vector2d v, double d) {
	Vector2d r;

	r.x = v.x / d;
	r.y = v.y / d;

	return r;
}

Vector2d centroid(Vector2d* pts) {
	Vector2d r;

	r = Vector2dAdd(pts[0], Vector2dAdd(pts[1], pts[2]));
	r = Vector2dDiv(r, 3);

	return r;
}

Vector3d Vector3dSub (Vector3d v1, Vector3d v2) {
	Vector3d r;

	r.x = v1.x - v2.x;
	r.y = v1.y - v2.y;
	r.z = v1.z - v2.z;

	return r;
}

Voisin VoisinNew (int n, int i) {
	Voisin v;

	v.nF = n;
	v.idx = i;

	return v;
}

Vector2d milieu (Vector2d p1, Vector2d p2) {
	Vector2d r = Vector2dDiv(Vector2dAdd(p1, p2), 2);

	return r;
}

Vector2d vPetit(Vector2d p1, Vector2d p2) {
	Vector2d r;

	r.x = fmin(p1.x, p2.x);
	r.y = fmin(p1.y, p2.y);

	return r;
}

double direction (Vector2d p1, Vector2d p2) {
	return atan2(p2.y - p1.y, p2.x - p1.x);
}

double degToRad(double degrees) {
	return degrees * pi / 180;
}

void d2ize (Vector3d p[3], Vector2d P[3]) {
	Vector3d
		d1 = Vector3dSub(p[1], p[0]),
		d2 = Vector3dSub(p[2], p[0]);

	P[0].x = 0;
	P[0].y = 0;
	P[1].x = sqrt((d1.x*d1.x) + (d1.y * d1.y) + (d1.z * d1.z));
	P[1].y = 0;
	P[2].x = ((d1.x*d2.x) + (d1.y*d2.y) + (d1.z*d2.z))/ P[1].x;
	P[2].y = sqrt((d2.x*d2.x) + (d2.y*d2.y) + (d2.z*d2.z) - (P[2].x*P[2].x));
}

double isCoplanar (Vector3d t[3], Vector3d p) {
	// Function to find equation of plane.
	// https://www.geeksforgeeks.org/program-to-check-whether-4-points-in-a-3-d-plane-are-coplanar/
Vector3d
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

int suiv (int n) {	// suivant dans triplet 0,1,2
	return n < 2 ? n + 1 : 0;
}

int prec (int n) {	// précédent dans triplet 0,1,2
	return n > 0 ? n -1 : 2;
}

double diff (Vector2d p1, Vector2d p2) {
	double r;

	if (eqd(p1.x, p2.x))
		r = p2.y - p1.y;
	else
		r = p2.x - p1.x;

	return r;
}

double distance2d (Vector2d p1, Vector2d p2) {
	Vector2d d = Vector2dSub(p2, p1);

	return sqrt((d.x * d.x) + (d.y * d.y));
}

double distance3d (Vector3d p1, Vector3d p2) {
	Vector3d d = Vector3dSub(p2, p1);

	return sqrt((d.x * d.x) + (d.y * d.y) + (d.z * d.z));
}

_Bool eq (Vector2d p1, Vector2d p2) {
	return distance2d(p1, p2) < epsilon;
}

_Bool eq3 (Vector3d t1[3], Vector3d t2[3], int n) {
	return (distance3d(t2[n], t1[0]) >= epsilon)
			&& (distance3d(t2[n], t1[1]) >= epsilon)
			&& (distance3d(t2[n], t1[2]) >= epsilon);
}

double calcAngle (Vector2d a, Vector2d b, Vector2d c) {
	Vector2d ab = Vector2dSub(b, a);
	Vector2d ac = Vector2dSub(c, a);

  double rot_ab_ac = atan2(ac.y*ab.x - ac.x*ab.y, ac.x*ab.x + ac.y*ab.y);

  return rot_ab_ac;
}

Vector2d rotation (Vector2d c, Vector2d p, double angle) {
	double
		lcos = cos(angle),
		lsin = sin(angle);

	Vector2d r = {
		(lcos * (p.x - c.x)) + (lsin * (p.y - c.y)) + c.x,
		(lcos * (p.y - c.y)) - (lsin * (p.x - c.x)) + c.y };

	return r;
}

double angle (Vector2d p1, Vector2d p2) {
	double r = atan2(p2.y - p1.y, p2.x - p1.x);

	return r;
}

int compPg (const void * el1, const void * el2) {
	Ligne v1 = * (const Ligne *) el1;
	Ligne v2 = * (const Ligne *) el2;

	int r = v1.nP - v2.nP;

	return r;
}

int compAff (const void * el1, const void * el2) {
	NAff v1 = * (const NAff *) el1;
	NAff v2 = * (const NAff *) el2;

	int r = (v1.nMax != v2.nMax) ? v1.nMax - v2.nMax : v1.nMin - v2.nMin;

	return r;
}

int compAffa (const void * el1, const void * el2) {
	NAff v1 = * (const NAff *) el1;
	NAff v2 = * (const NAff *) el2;

	int r = v1.a - v2.a;

	return r;
}

int compLang (const void * el1, const void * el2) {
	Lang v1 = * (const Lang *) el1;
	Lang v2 = * (const Lang *) el2;

	//int r = (v1.n1 - v2.n1) * 2000 + (v1.n2 - v2.n2);
	int r = (v1.n1 != v2.n1) ? v1.n1 - v2.n1 : v1.n2 - v2.n2;

	return r;
}

int compLangO (const void * el1, const void * el2) {
	Lang v1 = * (const Lang *) el1;
	Lang v2 = * (const Lang *) el2;

	int r = v1.o - v2.o;

	return r;
}

void trapeze (Vector2d * P, Vector2d p1, Vector2d p2, double s, double dt) {
	double d = distance2d(p1, p2);
	double a = degToRad(90) - direction(p1, p2);

	if (d > 50) dt = dt/2;

	P[0] = rotation(p1, p1, a);
	P[1] = rotation(p1, Vector2dAdd(p1, Vector2dNew(s, d*dt)), a);
	P[2] = rotation(p1, Vector2dAdd(p1, Vector2dNew(s, d*(1-dt))), a);
	P[3] = rotation(p1, Vector2dAdd(p1, Vector2dNew(0, d)), a);
}

int comp (const void * el1, const void * el2) {
	Ligne v1 = * (const Ligne *) el1;
	Ligne v2 = * (const Ligne *) el2;

	int iMin1 = min(v1.n1, v1.n2);
	int iMax1 = max(v1.n1, v1.n2);
	int iMin2 = min(v2.n1, v2.n2);
	int iMax2 = max(v2.n1, v2.n2);

	if (iMax1 == iMax2)
		return iMin1 - iMin2;
	else
		return iMax1 - iMax2;
}

_Bool li (Vector2d l1S, Vector2d l1E, Vector2d l2S, Vector2d l2E) {
	// true if the lines intersect
	if (eq(l1S, l2S) || eq(l1S, l2E) || eq(l1E, l2S) || eq(l1E, l2E)) {
		return 0;
	}

	double denominator = ((l2E.y - l2S.y) * (l1E.x - l1S.x))
										 - ((l2E.x - l2S.x) * (l1E.y - l1S.y));

	if (denominator == 0)
		return 0;

	double
		a = l1S.y - l2S.y,
		b = l1S.x - l2S.x,
		numerator1 = ((l2E.x - l2S.x) * a) - ((l2E.y - l2S.y) * b),
		numerator2 = ((l1E.x - l1S.x) * a) - ((l1E.y - l1S.y) * b);
	a = numerator1 / denominator;
	b = numerator2 / denominator;

	if ((a > 0) && (a < 1) && (b > 0) && (b < 1))
		return 1;
	else
		return 0;
}

_Bool overlap (Vector2d *t1, Vector2d *t2) {
	_Bool r =
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

void calcBoiteEnglobante(Vector2d b[2], Vector2d *pts, int nbP) {
	// calcul de la boite englobante
	b[0] = (Vector2d) {10000, 10000};
	b[1] = (Vector2d) {-10000, -10000};
	for (int i = 0; i < nbP; i++) {
		Vector2d p = pts[i];
		if (b[0].x > p.x)b[0].x = p.x;
		if (b[0].y > p.y)b[0].y = p.y;
		if (b[1].x < p.x)b[1].x = p.x;
		if (b[1].y < p.y)b[1].y = p.y;
	}
}

int compteDispo(_Bool *l, int nb) {
	int n = 0;
	for (int i = 0; i < nb; i++) {
		if (l[i])
			n++;
	}

	return n;
}

int premDispo(_Bool *l, int nb) {
	int n = -1,
			i = 0;
	do {
		if (l[i])
			n = i;
		i++;
	} while ((i < nb) &&  (n == -1));

	return n;
}

void calculeVoisinage(int(*f)[4], int n, Voisin v[][3]) {
	for (int i = 0; i < n; i++) {
		for (int j = 0; j < 3; j++) {
			int vi = 0;
			_Bool ok = 0;
			do {
				if (vi != i) {
					for (int k = 0; (k < 3) && (!ok); k++) {
						if ((f[vi][k] == f[i][suiv(j)]) && (f[vi][suiv(k)] == f[i][j])) {
							v[i][j]= VoisinNew(vi, suiv(k));
							ok = 1;
						}
					}
					if (!ok)
						vi++;
				} else
					vi++;
			} while ((vi < n) && (!ok));
		}
	}
}

void calculeCop(int n, Voisin voisins[][3], Cop* tCop, Vector3d v3d[][3]) {
	int nbCop = 0;
	for (int i = 0; i < n; i++) {
		for (int j = 0; j < 3; j++) {
			int nV = voisins[i][j].nF;
			Vector3d p;

			if (eq3(v3d[i], v3d[nV], 0)) {
				p = v3d[nV][0];
			} else if (eq3(v3d[i], v3d[nV], 1)) {
				p =  v3d[nV][1];
			} else {
				p =  v3d[nV][2];
			}

			double c = isCoplanar(v3d[i], p);
			tCop[nbCop].cop = c;
			tCop[nbCop].nF =  i;
			tCop[nbCop].nV = nV;
			nbCop++;
		}
	}
}

int sauveLanguettes(Lang * sL, int nbL, int mode) {
	char * nomFichierDonnees = "donnees.lng";
	FILE * fichierDonnees;
	if (!(fichierDonnees = fopen(nomFichierDonnees, "w"))) {
		perror(textes[29]);
		return -1;
	}

	qsort(sL, nbL, sizeof(Lang), compLangO);
	int v;
	for (int i = 0; i < nbL; i++)	{
		if (mode == M_LANG_SAUV)
			v = sL[i].v;
		else if (mode == M_LANG_CREA0)
			v = 0;
		else if (mode == M_LANG_CREA1)
			v = sL[i].n1 > sL[i].n2 ? 0 : 1;
		else if (mode == M_LANG_CREA2)
			v = 1;
    else
      v = 0;
		fprintf(fichierDonnees, "%d %d %d %d\n", sL[i].o, sL[i].n1, sL[i].n2, v);
	}
	if (fclose(fichierDonnees) == EOF ) {
		perror(textes[29]);
		return -1;
	}

	return 0;
}

int sauveDonnees(DonneesDep d) {
	char * nomFichierDonnees = "donnees.dep";
	FILE * fd;
	int rc;

	fd = fopen(nomFichierDonnees, "w");
	if (!fd) {
		perror(textes[29]);
		return -1;
	}

	fprintf(fd, "%s\n", d.fichierOBJ);
	fprintf(fd, "%5.2lf\n", d.echelle);
	fprintf(fd, "%2d\n", d.formatPage);
	fprintf(fd, "%2d\n", d.typeLang);
	fprintf(fd, "%4d\n", d.premierTriangle);
	fprintf(fd, "%5.2f\n", d.tailleNums);
	fprintf(fd, "%2d\n", d.hauteurLang);
	for (int i = 0; i < d.nbD; i++) {
		if (d.sD[i].orig < 0) {
			fprintf(fd, "%4d %4d %4d %6.2lf %6.2lf\n",
				d.sD[i].orig, d.sD[i].face, d.sD[i].a, d.sD[i].d.x, d.sD[i].d.y);
		} else {
			fprintf(fd, "%4d %4d\n", d.sD[i].orig, d.sD[i].face);
		}
	}
	if ((rc = fclose(fd)) == EOF ) {
		perror(textes[29]);
		return -1;
	}

	return 0;
}

void supprimeDoublons(Ligne * lignes, int nbL) {
	// tri
	qsort(lignes, nbL, sizeof(Ligne), comp);
	// renum
	for (int i = 0; i < nbL; i++)
		lignes[i].id = i;
	// suppression doublons
	int dOK = 0;
	for (int i = 1; i < nbL; i++) {
		if ((lignes[dOK].nP == lignes[i].nP)
		&& ((eq(lignes[dOK].p1, lignes[i].p1) && eq(lignes[dOK].p2, lignes[i].p2))
		    || (eq(lignes[dOK].p1, lignes[i].p2) && eq(lignes[dOK].p2, lignes[i].p1)))
		) {
			lignes[i].id = -1;
			lignes[dOK].nb++;
		} else
			dOK = i;
	}
}

void afficheNum(cairo_t *cr, int num, Vector2d p1, Vector2d p2, Coul c) {
  cairo_text_extents_t te;
	char ch[10];
	Vector2d m;
	double fheight = -3, nx, ny, a;

	cairo_save(cr);
	sprintf(ch, "%d", num);
	m = milieu(p1, p2);
	cairo_rectangle(cr, m.x-2, m.y-2, 5, 5);
	cairo_set_source_rgb(cr, 0, 0, 1);
	cairo_fill(cr);
	cairo_set_source_rgb(cr, c.r, c.v, c.b);
	cairo_text_extents (cr, ch, &te);
  nx = -te.width / 2.0;
  ny = fheight / 2;
	cairo_translate(cr, m.x, m.y);
  if (!eq(p1, p2)) {
  	a = angle(p1, p2) - pi;
		cairo_rotate(cr, a);
	}
	cairo_translate(cr, nx, ny);
	cairo_move_to(cr, 0,0);
	cairo_show_text(cr, ch);
	cairo_restore(cr);
}

void afficheNumsPage(cairo_t *cr, AN *lAN, int nbAN, Vector2d v[][3]) {
	for (int iAN = 0; iAN < nbAN; iAN++) {
		AN lANc = lAN[iAN];
		Coul lC;
		if ( ((iAN > 0) 		 && (lANc.n == lAN[iAN-1].n))
			|| ((iAN < nbAN-1) && (lANc.n == lAN[iAN+1].n)))
			lC = C_VERT;
		else
			lC = C_NOIR;
		afficheNum(cr, lANc.n, lANc.p1, lANc.p2, lC);
	}
}

void faitLigne(cairo_t *cr, Vector2d p1, Vector2d p2, int typeL, int hLang) {
	if (typeL != L_PLI_C) {	// pas de ligne si pli coplanaire
		Coul c;
		static const double tiret[] = {10.0};
		static const double tpoint[] = {8.0,2.0,2.0,2.0};

		if ((typeL == L_COUPE) || (typeL == L_LGT_C) || (typeL == L_LGT_M)
		 || (typeL == L_LGT_V)) {
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

		if ((typeL == L_LGT_C) || (typeL == L_LGT_M) || (typeL == L_LGT_V)) {
			Vector2d pts[4];
			trapeze(pts, p1, p2, hLang, 0.45);
			cairo_move_to(cr, pts[0].x, pts[0].y);
			cairo_line_to(cr, pts[1].x, pts[1].y);
			cairo_line_to(cr, pts[2].x, pts[2].y);
			cairo_line_to(cr, pts[3].x, pts[3].y);
			cairo_stroke(cr);
			if (typeL == L_LGT_M) {
				c = C_MARRON;
				cairo_set_source_rgb(cr, c.r, c.v, c.b);
				cairo_set_dash(cr, tiret, 1, 0);
				cairo_move_to(cr, p1.x, p1.y);
				cairo_line_to(cr, p2.x, p2.y);
				cairo_stroke(cr);
			} else if (typeL == L_LGT_V) {
				c = C_VERT;
				cairo_set_source_rgb(cr, c.r, c.v, c.b);
				cairo_set_dash(cr, tpoint, 4, 0);
				cairo_move_to(cr, p1.x, p1.y);
				cairo_line_to(cr, p2.x, p2.y);
				cairo_stroke(cr);
			}
		} else {
			cairo_move_to(cr, p1.x, p1.y);
			cairo_line_to(cr, p2.x, p2.y);
			cairo_stroke(cr);
		}
	}
}

Vector2d marge = { 10, 10 };
Vector2d formats[8] = { // formats de page en pt
	{2380,	3368},	// A0
	{1684,	2380},	// A1
	{1190,	1684},	// A2
	{ 842,	1190},	// A3
	{ 595,   842},	// A4
	{ 421,   595},	// A5
	{ 849,   849},  // CR1
	{ 849,  1698},  // CR2
};

const char * const formatsPage[][3] = {
  {"A0", "841", "1189"},
  {"A1", "594", "841"},
  {"A2", "402", "594"},
  {"A3", "297", "402"},
  {"A4", "210", "297"},
  {"A5", "148", "210"},
  {"Cr1", "300", "300"},
  {"Cr2", "300", "600"}
};

DonneesDep chargeOBJ(DonneesDep dd)
{
  setlocale(LC_NUMERIC, "C");

	Vector3d* sommets = NULL;
	dd.nbSommets = 0;
	int* faces0 = NULL;
	dd.nbFaces = 0;

	FILE* fs;
	if (!(fs = fopen(dd.fichierOBJ, "r"))) {
		//perror(textes[2]);
		printf("erreur ouverture %s\n", dd.fichierOBJ);
		exit(0);
	}

	//printf("LECTURE fichier : %s\n", dd.fichierOBJ);

	char ligneLue [MAX_TAMPON];
	int gc = 0; // groupe courant
	while (fgets(ligneLue, MAX_TAMPON, fs)){
		int fc[4];
		char tL0 = ligneLue[0];
		char tL1 = ligneLue[1];
		if ((tL0 == 'v') && isspace(tL1)) { // v = vector (sommet)
      double v[3];
      sscanf(ligneLue, "v %lf %lf %lf", &v[0], &v[1], &v[2]);
			Vector3d s = { v[0] * dd.echelle, v[1] * dd.echelle, v[2] * dd.echelle };
			Vector3d* tmp;
			tmp = (Vector3d*)realloc(sommets, sizeof(Vector3d) * ((size_t)dd.nbSommets + 1));
			if (tmp) {
				sommets = tmp;
        sommets[dd.nbSommets++] = s;
			}
		} else if ((tL0 == 'g') && (tL1 == ' ')) { // g = group (groupe)
			gc++;
		} else if ((tL0 == 'f') && (tL1 == ' ')) { // f = face
			char c[3][20];
			 sscanf(ligneLue, "f %s %s %s", c[0], c[1], c[2]);
			for (int i = 0; i < 3; i++) {
				size_t pos = strspn(c[i], "0123456789");
				char ch[20];
				if (ch != NULL) {
					strncpy(ch, c[i], pos);
					ch[pos] = '\0';
					fc[i] = atoi(ch);
				}
			}
			fc[3] = gc;
			int* tmp = (int*)realloc(faces0, sizeof(int) * ((size_t)dd.nbFaces + 1) * 4);
			if (tmp) {
				faces0 = tmp;
        for (int i = 0; i < 4; i++) {
          faces0[dd.nbFaces * 4 + i] = fc[i] - 1;
        }
        dd.nbFaces++;
			}
		}
	}
	if (fclose(fs)) {
		perror(textes[3]);
		exit(1);
	}

	dd.faces = calloc(dd.nbFaces, sizeof * dd.faces);
	int n = 0;
	for (int i = 0; i < dd.nbFaces; i++) {
		for (int j = 0; j < 4; j++)
			dd.faces[i][j] = faces0[n++];
	}
	g_free(faces0);

	// VOISINS
	dd.voisins = calloc(dd.nbFaces, sizeof * dd.voisins);
	calculeVoisinage(dd.faces, dd.nbFaces, dd.voisins);

	// V3D V2D
	dd.v3d = calloc(dd.nbFaces, sizeof * dd.v3d);
	dd.v2d = calloc(dd.nbFaces, sizeof * dd.v2d);
	for (int i = 0; i < dd.nbFaces; i++) {
		for (int j = 0; j < 3; j++) {
			dd.v3d[i][j] = sommets[dd.faces[i][j]];
		}
		d2ize(dd.v3d[i], dd.v2d[i]);
	}
	g_free(sommets);

	// estCOP
	dd.tCop = calloc((size_t)dd.nbFaces * 3, sizeof * dd.tCop);
	calculeCop(dd.nbFaces, dd.voisins, dd.tCop, dd.v3d);

	return dd;
}

int retourneLigne(char *contenu, char *tampon, int dep)
{
  int n = 0;
  while (TRUE) {
    char c = contenu[dep];
    if (c == '\n')
    {
      tampon[n] = 0;
      return dep + 1;
    }
    else
      tampon[n++] = c;
    dep++;
  }
}
DonneesDep chargeDonnees(DonneesDep dd)
{
	char* nomFichierDonnees = "donnees.dep";

	GFile *file = g_file_new_for_path(nomFichierDonnees);
	printf("nom fichier donnees : %s\n", g_file_get_path(file));

  char *contents;
  gsize length;

	if (!g_file_load_contents(file, NULL, &contents, &length, NULL, NULL))
  {
    perror("erreur lecture fichier donnees.dep");
  }

	int fi = 0;
  char lu[60];

  fi = retourneLigne(contents, dd.fichierOBJ, fi);
  fi = retourneLigne(contents, lu, fi);
  dd.echelle = strtod(lu, NULL);

  fi = retourneLigne(contents, lu, fi);
  dd.formatPage = atoi(lu);

  fi = retourneLigne(contents, lu, fi);
  dd.typeLang = atoi(lu);

  fi = retourneLigne(contents, lu, fi);
  dd.premierTriangle = atoi(lu);

  fi = retourneLigne(contents, lu, fi);
  dd.tailleNums = atof(lu);

  fi = retourneLigne(contents, lu, fi);
  dd.hauteurLang = atoi(lu);

/*  puts("=============================================");
  printf("LU fichier          = %s\n", dd.fichierOBJ);
  printf("LU echelle          = %lf\n", dd.echelle);
  printf("LU formatPage       = %d\n", dd.formatPage);
  printf("LU typeLang         = %d\n", dd.typeLang);
  printf("LU premierTriangle  = %d\n", dd.premierTriangle);
  printf("LU tailleNums       = %f\n", dd.tailleNums);
  printf("LU hauteurLang      = %d\n", dd.hauteurLang);
*/
	g_free(dd.sD);
	dd.sD = NULL;
	dd.nbD = 0;
	Depliage sD0;

	int d0, d1, d2;
	char *cdx, *cdy;
	Vector2d delta;
	do
  {
    fi = retourneLigne(contents, lu, fi);
    if (g_str_has_prefix(g_strstrip(lu), "-"))
    {
      cdx = (char *)malloc(10);
      cdy = (char *)malloc(10);
      sscanf(lu, " %d %d %d %s %s", &d0, &d1, &d2, cdx, cdy);
      delta.x = strtod(cdx, NULL);
      delta.y = strtod(cdy, NULL);
      free(cdx);
      free(cdy);
    }
    else
    {
      sscanf(lu, " %d %d", &d0, &d1);
      d2 = 0;
    }
		sD0.orig = d0;
		sD0.face = d1;
		sD0.a = d2;
		if (d0 < 0)
      sD0.d = delta;
		Depliage* tmp = (Depliage*)realloc(dd.sD, sizeof(Depliage) * ((size_t)dd.nbD + 1));
		if (tmp)
		{
			dd.sD = tmp;
			dd.sD[dd.nbD] = sD0;
			dd.nbD++;
		}
  } while (fi < length);
  printf("LU : Dépliage %d étapes\n", dd.nbD);

	// charge données languettes
	dd.nbLgt = 0;
	g_free(dd.sLgt);
	dd.sLgt = NULL;
	Lang sLgt0;

	int d3;
	int nvl;
	char* nomFichierLanguettes = "donnees.lng";
	FILE * fd = fopen(nomFichierLanguettes, "r");
	if (fd)
	{
		while ((nvl = fscanf(fd, "%d", &d0)) > 0)
		{
			nvl = fscanf(fd, "%d", &d1);
			nvl = fscanf(fd, "%d", &d2);
			nvl = fscanf(fd, "%d", &d3);
			sLgt0.o  = d0;
			sLgt0.n1 = d1;
			sLgt0.n2 = d2;
			sLgt0.v  = d3;
	    dd.sLgt = g_realloc(dd.sLgt, sizeof(Lang) * ((size_t)dd.nbLgt + 1));
      dd.sLgt[dd.nbLgt] = sLgt0;
			dd.nbLgt++;
		}
		if (fclose(fd) == EOF)
		{
			perror(textes[3]);
			exit(1);
		}
	}

  g_free(contents);
	g_object_unref(file);

  printf("LU : Languettes %d \n", dd.nbLgt);

	dd = chargeOBJ(dd);

	// DEBUT DEPLIAGE
	g_free(dd.lSNA);
	dd.lSNA = g_new(NAff, dd.nbFaces * 3);
	dd.nAff = 0;

	int nbP = -1;
	int nbPP = -1;
  int dOrig = 0;
	Vector2d* vMin = NULL;
	g_free(dd.lignes);
	dd.lignes = g_new(Ligne, dd.nbFaces * 3);
	dd.nbL = 0;
	for (int i = 0; i < dd.nbD; i++)
	{
		if (dd.sD[i].orig < 0)
		{
			nbPP++;
			if (dd.sD[i].orig != dOrig)
      {
        dOrig = dd.sD[i].orig;
        nbP++;
      }
			int f = dd.sD[i].face;
			int a = dd.sD[i].a;
			if (a != 0)
			{
				Vector2d m = centroid(dd.v2d[f]);
				for (int vi = 0; vi < 3; vi++)
					dd.v2d[f][vi] = rotation(m, dd.v2d[f][vi], degToRad(a));
			}
			//Vector2d * tmp = (Vector2d*)realloc(vMin, sizeof(Vector2d) * ((size_t)nbP + 1));
			Vector2d * tmp = (Vector2d*)realloc(vMin, sizeof(Vector2d) * ((size_t)nbPP + 1));
			if (tmp)
			{
				vMin = tmp;
				vMin[nbPP] = vPetit(dd.v2d[f][0], vPetit(dd.v2d[f][1], dd.v2d[f][2]));
			}
		}
		dd.sD[i].page = nbP;
		dd.sD[i].piece= nbPP;

		if (dd.sD[i].orig > -1)
		{
			int tc = dd.sD[i].orig;
			int vc = dd.sD[i].face;
			int vi = dd.voisins[tc][0].nF == vc ? 0 :
				dd.voisins[tc][1].nF == vc ? 1 : 2;
			Voisin v = dd.voisins[tc][vi];

			// 1°) rapproche v2d[vc] de v2d[tc]
			Vector2d deltaV = Vector2dSub(dd.v2d[tc][vi], dd.v2d[vc][v.idx]);
			for (int n = 0; n < 3; n++)
				dd.v2d[vc][n] = Vector2dAdd(dd.v2d[vc][n], deltaV);
			// 2°) tourne v2d[vc]
			double a = calcAngle(dd.v2d[tc][vi], dd.v2d[tc][suiv(vi)], dd.v2d[vc][prec(v.idx)]);
			for (int n = 0; n < 3; n++)
			{
				dd.v2d[vc][n] = rotation(dd.v2d[tc][vi], dd.v2d[vc][n], a);
				vMin[nbPP] = vPetit(vMin[nbPP], dd.v2d[vc][n]);
			}
		}
	}

	for (int i = 0; i < dd.nbD; i++)
	{
		int tc = dd.sD[i].face;
		//int pc = dd.sD[i].page;
		int pc = dd.sD[i].piece;
		for (int j = 0; j < 3; j++)
		{
			dd.v2d[tc][j] = Vector2dSub(dd.v2d[tc][j], Vector2dSub(vMin[pc], marge));
		}
	}
	free(vMin);

	Vector2d dC;
	for (int i = 0; i < dd.nbD; i++)
	{
		int tc = dd.sD[i].face;
		int pc = dd.sD[i].page;
		if (dd.sD[i].orig < 0)
      dC = dd.sD[i].d;
		for (int j = 0; j < 3; j++)
		{
		  // rech nA
		  int nA = -1;
		  int nMin = min(tc, dd.voisins[tc][j].nF);
		  int nMax = max(tc, dd.voisins[tc][j].nF);
		  for (int iA = 0; (nA == -1) && (iA < dd.nAff); iA ++)
      {
        NAff nA0 = dd.lSNA[iA];
        if ((nA0.nMin == nMin) && (nA0.nMax == nMax))
          nA = nA0.a;
      }
      if (nA == -1) // nouveau n°
      {
        nA = dd.nAff;
        NAff nA0 = {nMin, nMax, nA};
        dd.lSNA = g_realloc(dd.lSNA, sizeof(NAff) * (nA+1));
        dd.lSNA[nA] = nA0;
        dd.nAff++;
      }

			dd.lignes[dd.nbL] = LigneNew(pc, dd.nbL,
				Vector2dAdd(dd.v2d[tc][j], dC), Vector2dAdd(dd.v2d[tc][suiv(j)], dC),
				tc, j,
				dd.voisins[tc][j].nF, dd.voisins[tc][j].idx,
				nA);
			dd.nbL++;
		}
	}

	supprimeDoublons(dd.lignes, dd.nbL);

	//qsort(dd.lSNA, dd.nAff, sizeof(NAff), compAff);
	qsort(dd.lignes, dd.nbL, sizeof(Ligne), compPg);
	qsort(dd.sLgt, dd.nbLgt, sizeof(Lang), compLang);

	return dd;
}

DonneesDep init_cairo(DonneesDep d) {
	d.surface = cairo_pdf_surface_create(textes[10],
		formats[d.formatPage].x, formats[d.formatPage].y);
	d.cr = cairo_create(d.surface);

	cairo_select_font_face(d.cr, "Sans", CAIRO_FONT_SLANT_NORMAL,
		CAIRO_FONT_WEIGHT_NORMAL);
	cairo_set_font_size(d.cr, d.tailleNums);
	cairo_set_line_width(d.cr, 1);

	return d;
}

void closeCairo(DonneesDep d) {
	cairo_surface_destroy(d.surface);
	cairo_destroy(d.cr);
	cairo_debug_reset_static_data();
}
