//---------- INCLUDES ----------
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <math.h>
#include <locale.h>

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

#include "structs.c"  // STRUCTURES

//---------- CONSTANTES ----------
#define APPLICATION_ID "com.github.gilboonet.deplieur"

#ifndef WIN32
	#define max(a,b) (a>=b?a:b)
	#define min(a,b) (a<=b?a:b)
#endif

#define epsilon 0.0001
#define MAX_TAMPON 100
#define _CRT_SECURE_NO_WARNINGS

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

static const double tiret[] = {20.0};
static const double tpoint[] = {8.0,2.0,2.0,2.0};

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

const Vector2d marge = { 10, 10 };
const Vector2d formats[8] = { // formats de page en pt
	{2380,	3368},	// A0
	{1684,	2380},	// A1
	{1190,	1684},	// A2
	{ 842,	1190},	// A3
	{ 595,   842},	// A4
	{ 421,   595},	// A5
	{ 849,   849},  // CR1
	{ 849,  1698},  // CR2
};

/*const char * const formatsPage[][3] = {
  {"A0", "841", "1189"},
  {"A1", "594", "841"},
  {"A2", "402", "594"},
  {"A3", "297", "402"},
  {"A4", "210", "297"},
  {"A5", "148", "210"},
  {"Cr1", "300", "300"},
  {"Cr2", "300", "600"}
};*/

//---------- VARIABLES GLOBALES ----------
GtkWidget *win;
DonneesDep dd;

//---------- VECTOR2D ----------
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

double Vector2dDistance (Vector2d p1, Vector2d p2) {
	Vector2d d = Vector2dSub(p2, p1);

	return sqrt((d.x * d.x) + (d.y * d.y));
}

_Bool Vector2dEq (Vector2d p1, Vector2d p2) {
	return Vector2dDistance(p1, p2) < epsilon;
}

Vector2d Vector2dRotation (Vector2d c, Vector2d p, double angle) {
	double
		lcos = cos(angle),
		lsin = sin(angle);

	Vector2d r = {
		(lcos * (p.x - c.x)) + (lsin * (p.y - c.y)) + c.x,
		(lcos * (p.y - c.y)) - (lsin * (p.x - c.x)) + c.y };

	return r;
}

//---------- VECTOR3D ----------
Vector3d Vector3dSub (Vector3d v1, Vector3d v2) {
	Vector3d r;

	r.x = v1.x - v2.x;
	r.y = v1.y - v2.y;
	r.z = v1.z - v2.z;

	return r;
}

double Vector3dDistance (Vector3d p1, Vector3d p2) {
	Vector3d d = Vector3dSub(p2, p1);

	return sqrt((d.x * d.x) + (d.y * d.y) + (d.z * d.z));
}

_Bool Vector3dEq (Vector3d t1[3], Vector3d t2[3], int n) {
	return (Vector3dDistance(t2[n], t1[0]) >= epsilon)
			&& (Vector3dDistance(t2[n], t1[1]) >= epsilon)
			&& (Vector3dDistance(t2[n], t1[2]) >= epsilon);
}

void Vector3dD2ize (Vector3d p[3], Vector2d P[3]) { // retourne le vecteur 3d p en vecteur 2d (P)
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

double Vector3dIsCoplanar (Vector3d t[3], Vector3d p) {
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

//---------- LIGNE ----------
Ligne LigneNew (int iPage, int id, Vector2d p1, Vector2d p2, int n1, int i1, int n2, int i2, int nA) {
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

//---------- VOISIN ----------
Voisin VoisinNew (int n, int i) {
	Voisin v;

	v.nF = n;
	v.idx = i;

	return v;
}

//---------- FONCTIONS ----------
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

int compPg (const void * el1, const void * el2) {
	Ligne v1 = * (const Ligne *) el1;
	Ligne v2 = * (const Ligne *) el2;

	int r = v1.nP - v2.nP;

	return r;
}

double degToRad(double degrees) {
	return degrees * pi / 180;
}

double direction (Vector2d p1, Vector2d p2) {
	return atan2(p2.y - p1.y, p2.x - p1.x);
}

int prec (int n) {	// précédent dans triplet 0,1,2
	return n > 0 ? n -1 : 2;
}

int suiv (int n) {	// suivant dans triplet 0,1,2
	return n < 2 ? n + 1 : 0;
}

_Bool eqd(double d1, double d2) { // équivalence à epsilon près entre 2 double
	return fabs(d1 - d2) < epsilon;
}

double calcAngle (Vector2d a, Vector2d b, Vector2d c) {
	Vector2d ab = Vector2dSub(b, a);
	Vector2d ac = Vector2dSub(c, a);

  double rot_ab_ac = atan2(ac.y*ab.x - ac.x*ab.y, ac.x*ab.x + ac.y*ab.y);

  return rot_ab_ac;
}

double angle (Vector2d p1, Vector2d p2) {
	return atan2(p2.y - p1.y, p2.x - p1.x);
}

//---------- FONCTIONS APPLICATIVES ----------
void trapeze (Vector2d * P, Vector2d p1, Vector2d p2, double s, double dt) {
	double d = Vector2dDistance(p1, p2);
	double a = degToRad(90) - direction(p1, p2);

	if (d > 50) dt = dt/2;

	P[0] = Vector2dRotation(p1, p1, a);
	P[1] = Vector2dRotation(p1, Vector2dAdd(p1, Vector2dNew(s, d*dt)), a);
	P[2] = Vector2dRotation(p1, Vector2dAdd(p1, Vector2dNew(s, d*(1-dt))), a);
	P[3] = Vector2dRotation(p1, Vector2dAdd(p1, Vector2dNew(0, d)), a);
}

_Bool li (Vector2d l1S, Vector2d l1E, Vector2d l2S, Vector2d l2E) {
	// true if the lines intersect
	if (Vector2dEq(l1S, l2S) || Vector2dEq(l1S, l2E) || Vector2dEq(l1E, l2S) || Vector2dEq(l1E, l2E)) {
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
	for (int i = 0; i < n; i++)
		for (int j = 0; j < 3; j++) {
			int nV = voisins[i][j].nF;
			Vector3d p;

			if (Vector3dEq(v3d[i], v3d[nV], 0))
				p = v3d[nV][0];
			else if (Vector3dEq(v3d[i], v3d[nV], 1))
				p =  v3d[nV][1];
			else
				p =  v3d[nV][2];

			double c = Vector3dIsCoplanar(v3d[i], p);
			tCop[nbCop].cop = c;
			tCop[nbCop].nF =  i;
			tCop[nbCop].nV = nV;
			nbCop++;
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
		if      (mode == M_LANG_SAUV)
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

	for (int i = 0; i < d.nbD; i++)
		if (d.sD[i].orig < 0)
			fprintf(fd, "%4d %4d %4d %6.2lf %6.2lf\n", d.sD[i].orig, d.sD[i].face, d.sD[i].a, d.sD[i].d.x, d.sD[i].d.y);
		else
			fprintf(fd, "%4d %4d\n", d.sD[i].orig, d.sD[i].face);


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
        && ((Vector2dEq(lignes[dOK].p1, lignes[i].p1) && Vector2dEq(lignes[dOK].p2, lignes[i].p2))
        ||  (Vector2dEq(lignes[dOK].p1, lignes[i].p2) && Vector2dEq(lignes[dOK].p2, lignes[i].p1)))) {
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
	if (dd.mode != MODE_PIECE) {
    cairo_rectangle(cr, m.x-2, m.y-2, 5, 5);
    cairo_set_source_rgb(cr, 0, 0, 1);
    cairo_fill(cr);
  }
	cairo_set_source_rgb(cr, c.r, c.v, c.b);
	cairo_text_extents (cr, ch, &te);
  nx = -te.width / 2.0;
  ny = fheight / 2;
	cairo_translate(cr, m.x, m.y);
  if (!Vector2dEq(p1, p2)) {
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
		if ( ((iAN > 0) && (lANc.n == lAN[iAN-1].n)) || ((iAN < nbAN-1) && (lANc.n == lAN[iAN+1].n)))
			lC = C_VERT;
		else
			lC = C_NOIR;
		afficheNum(cr, lANc.n, lANc.p1, lANc.p2, lC);
	}
}

void faitLigne(cairo_t *cr, Vector2d p1, Vector2d p2, int typeL, int hLang) {
	if (typeL != L_PLI_C) {	// pas de ligne si pli coplanaire
		Coul c;

		if ((typeL == L_COUPE) || (typeL == L_LGT_C) || (typeL == L_LGT_M) || (typeL == L_LGT_V)) {
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

DonneesDep chargeOBJ(DonneesDep dd){
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

	char ligneLue [MAX_TAMPON];
	int gc = 0; // groupe courant
	while (fgets(ligneLue, MAX_TAMPON, fs)) {
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
        for (int i = 0; i < 4; i++)
          faces0[dd.nbFaces * 4 + i] = fc[i] - 1;
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
		for (int j = 0; j < 3; j++)
			dd.v3d[i][j] = sommets[dd.faces[i][j]];
		Vector3dD2ize(dd.v3d[i], dd.v2d[i]);
	}
	g_free(sommets);

	// estCOP
	dd.tCop = calloc((size_t)dd.nbFaces * 3, sizeof * dd.tCop);
	calculeCop(dd.nbFaces, dd.voisins, dd.tCop, dd.v3d);

	return dd;
}

int retourneLigne(char *contenu, char *tampon, int dep){
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

DonneesDep chargeDonnees(DonneesDep dd) {
	char* nomFichierDonnees = "donnees.dep";

	GFile *file = g_file_new_for_path(nomFichierDonnees);
	printf("nom fichier donnees : %s\n", g_file_get_path(file));

  char *contents;
  gsize length;

	if (!g_file_load_contents(file, NULL, &contents, &length, NULL, NULL))
    perror("erreur lecture fichier donnees.dep");

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
	do {
    fi = retourneLigne(contents, lu, fi);
    if (g_str_has_prefix(g_strstrip(lu), "-")) {
      cdx = (char *)malloc(10);
      cdy = (char *)malloc(10);
      sscanf(lu, " %d %d %d %s %s", &d0, &d1, &d2, cdx, cdy);
      delta.x = strtod(cdx, NULL);
      delta.y = strtod(cdy, NULL);
      free(cdx);
      free(cdy);
    } else {
      sscanf(lu, " %d %d", &d0, &d1);
      d2 = 0;
    }
		sD0.orig = d0;
		sD0.face = d1;
		sD0.a = d2;
		if (d0 < 0)
      sD0.d = delta;
		Depliage* tmp = (Depliage*)realloc(dd.sD, sizeof(Depliage) * ((size_t)dd.nbD + 1));
		if (tmp) {
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
	if (fd) {
		while ((nvl = fscanf(fd, "%d", &d0)) > 0) {
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
		if (fclose(fd) == EOF) {
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
	for (int i = 0; i < dd.nbD; i++) {
		if (dd.sD[i].orig < 0) {
			nbPP++;
			if (dd.sD[i].orig != dOrig) {
        dOrig = dd.sD[i].orig;
        nbP++;
      }
			int f = dd.sD[i].face;
			int a = dd.sD[i].a;
			if (a != 0) {
				Vector2d m = centroid(dd.v2d[f]);
				for (int vi = 0; vi < 3; vi++)
					dd.v2d[f][vi] = Vector2dRotation(m, dd.v2d[f][vi], degToRad(a));
			}
			Vector2d * tmp = (Vector2d*)realloc(vMin, sizeof(Vector2d) * ((size_t)nbPP + 1));
			if (tmp) {
				vMin = tmp;
				vMin[nbPP] = vPetit(dd.v2d[f][0], vPetit(dd.v2d[f][1], dd.v2d[f][2]));
			}
		}
		dd.sD[i].page = nbP;
		dd.sD[i].piece= nbPP;

		if (dd.sD[i].orig > -1) {
			int tc = dd.sD[i].orig;
			int vc = dd.sD[i].face;
			int vi = dd.voisins[tc][0].nF == vc ? 0 : dd.voisins[tc][1].nF == vc ? 1 : 2;
			Voisin v = dd.voisins[tc][vi];

			// 1°) rapproche v2d[vc] de v2d[tc]
			Vector2d deltaV = Vector2dSub(dd.v2d[tc][vi], dd.v2d[vc][v.idx]);
			for (int n = 0; n < 3; n++)
				dd.v2d[vc][n] = Vector2dAdd(dd.v2d[vc][n], deltaV);
			// 2°) tourne v2d[vc]
			double a = calcAngle(dd.v2d[tc][vi], dd.v2d[tc][suiv(vi)], dd.v2d[vc][prec(v.idx)]);
			for (int n = 0; n < 3; n++) {
				dd.v2d[vc][n] = Vector2dRotation(dd.v2d[tc][vi], dd.v2d[vc][n], a);
				vMin[nbPP] = vPetit(vMin[nbPP], dd.v2d[vc][n]);
			}
		}
	}

	for (int i = 0; i < dd.nbD; i++) {
		int tc = dd.sD[i].face;
		int pc = dd.sD[i].piece;
		for (int j = 0; j < 3; j++)
			dd.v2d[tc][j] = Vector2dSub(dd.v2d[tc][j], Vector2dSub(vMin[pc], marge));
	}
	free(vMin);

	Vector2d dC;
	for (int i = 0; i < dd.nbD; i++) {
		int tc = dd.sD[i].face;
		int pc = dd.sD[i].page;
		if (dd.sD[i].orig < 0)
      dC = dd.sD[i].d;
		for (int j = 0; j < 3; j++) {
		  // rech nA
		  int nA = -1;
		  int nMin = min(tc, dd.voisins[tc][j].nF);
		  int nMax = max(tc, dd.voisins[tc][j].nF);
		  for (int iA = 0; (nA == -1) && (iA < dd.nAff); iA ++) {
        NAff nA0 = dd.lSNA[iA];
        if ((nA0.nMin == nMin) && (nA0.nMax == nMax))
          nA = nA0.a;
      }
      if (nA == -1) { // nouveau n°
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

	qsort(dd.lignes, dd.nbL, sizeof(Ligne), compPg);
	qsort(dd.sLgt, dd.nbLgt, sizeof(Lang), compLang);

	return dd;
}

DonneesDep init_cairo(DonneesDep d) {
	d.surface = cairo_pdf_surface_create(textes[10], formats[d.formatPage].x, formats[d.formatPage].y);
	d.cr = cairo_create(d.surface);

	cairo_select_font_face(d.cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
	cairo_set_font_size(d.cr, d.tailleNums);
	cairo_set_line_width(d.cr, 1);

	return d;
}

void closeCairo(DonneesDep d) {
	cairo_surface_destroy(d.surface);
	cairo_destroy(d.cr);
	cairo_debug_reset_static_data();
}

static void pressed (GtkGestureClick *gesture, int n_press, double x, double y, GtkWidget *area){
  _Bool OK = FALSE;
  int nPage = gtk_notebook_get_current_page(GTK_NOTEBOOK(dd.doc));

  printf("mode: %d clic: %lf %lf (page %d) %d rect\n", dd.mode, x, y, nPage, dd.nbRects);

  int i;

  if (dd.mode != MODE_PIECE)
    for (i = 0; i < dd.nbRects; i++) {
      rectT r = dd.rects[i];
      if ( (x >= r.p1.x) && (x <= r.p2.x) && (y >= r.p1.y) && (y <= r.p2.y)) {
        OK = TRUE;
        break;
      }
    }

  if (OK) {
    Ligne l = dd.lignes[dd.rects[i].id];
    printf("clic sur arête %d (%d <->  %d)\n", l.nA, l.n1, l.n2);
    for (i = 0; i < dd.nbD; i++) {
      Depliage d = dd.sD[i];
      if (d.page == nPage)
        printf("%d : %d, %d\n", i, d.orig, d.face);
    }
  }
}

static void dessinePage (GtkDrawingArea *area, cairo_t *cr, int width, int height, gpointer data) {
  // DESSIN PAR DEFAUT DE PAGE
  cairo_rectangle(cr, 1, 1, 594, 841);
  cairo_set_font_size(cr, dd.tailleNums);
  gdk_cairo_set_source_rgba (cr, &c_BLANC);
  cairo_fill (cr);

  int typeL;

  if (dd.rects) {
    g_free(dd.rects);
    dd.rects = NULL;
  }
  dd.nbRects = 0;

  unsigned nPage = (unsigned)(uintptr_t) data;

  for (int i = 0; i < dd.nbL; i++) {
    Ligne l = dd.lignes[i];
    if ((l.id > -1) && (l.nP == nPage)) {
      double cop = dd.tCop[(l.n1 * 3) + l.i1].cop;
      _Bool cop0 = fabs(cop) < 10e-7;
      if (l.nb == 1) {
        afficheNum(cr, l.nA, l.p1, l.p2, C_NOIR);
        if (dd.nbLgt == 0)
          typeL = L_COUPE;
        else {
          Lang * rL = NULL;
          for (int j = 0; !rL && (j < dd.nbLgt); j++)
            if ((dd.sLgt[j].n1 == l.n1) && (dd.sLgt[j].n2 == l.n2))
              rL = &dd.sLgt[j];
          if (!rL)
            typeL = L_COUPE;
          else
            typeL = rL->v == 0 ? L_COUPE : cop0 ? L_LGT_C : (cop < 0) ? L_LGT_M : L_LGT_V;
        }
      } else
        typeL = cop0 ? L_PLI_C : (cop < 0) ? L_PLI_M : L_PLI_V;
      // affichage ligne
      faitLigne(cr, l.p1, l.p2, typeL, dd.hauteurLang);
      if ((typeL == L_COUPE) || (typeL == L_LGT_M) || (typeL == L_LGT_V)) {
        afficheNum(cr, l.nA, l.p1, l.p2, C_NOIR);
        dd.rects = g_realloc(dd.rects, (size_t)(dd.nbRects+1) * sizeof(rectT));
        Vector2d m = milieu(l.p1, l.p2);
        rectT r;
        r.id = i;
        r.p1.x = m.x - 3;
        r.p1.y = m.y - 3;
        r.p2.x = m.x + 3;
        r.p2.y = m.y + 3;
        dd.rects[dd.nbRects] = r;
        dd.nbRects++;
      }
    }
  }
}

void rendu() {
  dd = chargeDonnees(dd);
  // PREMIERE PAGE DE RENDU
  GtkNotebook * d = GTK_NOTEBOOK(dd.doc);
  GtkGesture *press;
  //GtkGesture *drag;
  // Suppression des pages existantes
  while (gtk_notebook_get_n_pages(d) > 0)
    gtk_notebook_remove_page(d, -1);
  if (dd.dpage)
    g_free(dd.dpage);

  int nbP = dd.lignes[dd.nbL-1].nP +1;

  dd.dpage = g_new(GtkWidget*, nbP);
  for (unsigned i = 0; i < nbP; i++) {
    dd.dpage[i] = gtk_drawing_area_new();
    GtkDrawingArea * p = GTK_DRAWING_AREA(dd.dpage[i]);
    gtk_widget_set_cursor(dd.dpage[i], gdk_cursor_new_from_name("crosshair", NULL));

    press = gtk_gesture_click_new();
    gtk_gesture_single_set_button (GTK_GESTURE_SINGLE (press), GDK_BUTTON_PRIMARY);
    gtk_widget_add_controller (dd.dpage[i], GTK_EVENT_CONTROLLER (press));
    g_signal_connect (press, "pressed", G_CALLBACK (pressed), dd.dpage[i]);

    /*drag = gtk_gesture_drag_new();
    gtk_gesture_single_set_button (GTK_GESTURE_SINGLE (drag), GDK_BUTTON_PRIMARY);
    gtk_widget_add_controller (dpage[i], GTK_EVENT_CONTROLLER (drag));
    g_signal_connect (drag, "drag-begin", G_CALLBACK (on_drag_begin), dpage[i]);
    g_signal_connect (drag, "drag-end", G_CALLBACK (on_drag_end), dpage[i]);
    gtk_gesture_group(press, drag);*/

    gtk_drawing_area_set_content_width(p, 600);
    gtk_drawing_area_set_content_height(p, 841);
    gtk_notebook_append_page(d, dd.dpage[i], NULL);

    gtk_drawing_area_set_draw_func (p, dessinePage, (gpointer)(uintptr_t)i, NULL);
    gtk_widget_queue_draw(dd.dpage[i]);
  }
}

void deplier() { // DEPLIAGE
	//dd.echelle = atof(
  //  gtk_entry_buffer_get_text (gtk_entry_get_buffer(GTK_ENTRY(enEchelle))));

  dd.echelle = 8;
  dd = chargeOBJ(dd);

  //char * tampon = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(cbFormat));
  //dd.formatPage = formatPageID(tampon);
  dd.formatPage = 4;
	Vector2d limitePage = Vector2dSub(formats[dd.formatPage], marge);

  //tampon = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(cbLang));
  //dd.typeLang = tampon[0] == '1' ? 1 : tampon[0] == '2' ? 2 : 0;
  dd.typeLang = 1;

	//dd.tailleNums = atof(
  //  gtk_entry_buffer_get_text (gtk_entry_get_buffer(GTK_ENTRY(enTailleN))));
  dd.tailleNums = 15;

  //dd.hauteurLang = atoi(
  //  gtk_entry_buffer_get_text (gtk_entry_get_buffer(GTK_ENTRY(enLangH))));
  dd.hauteurLang = 15;

  dd.premierTriangle = 0;

  printf("fichier     : %s\n", dd.fichierOBJ);
  printf("echelle     : %5.2f\n", dd.echelle);
  printf("format page : %d\n",  dd.formatPage);
  printf("-->limites  : %5.0lf %5.0lf\n", limitePage.x, limitePage.y);
  printf("taille nums : %5.2f\n", dd.tailleNums);
  printf("type lang   : %d\n", dd.typeLang);
  printf("haut. lang. : %d\n", dd.hauteurLang);
  printf("nb sommets  : %d\n", dd.nbSommets);
  printf("nb faces    : %d\n", dd.nbFaces);

  // DEBUT DU DEPLIAGE
	dd.dispo = g_new(_Bool, dd.nbFaces);
	for (int i = 0; i < dd.nbFaces; i++)
		dd.dispo[i] = 1;

	dd.lSNA = g_new(NAff, dd.nbFaces * 3);
	dd.nAff = 0;

	dd.page = g_new(int, dd.nbFaces);
	dd.nbP = 0;
	dd.nbPP = 0;
	dd.sD = g_new(Depliage, dd.nbFaces); // depliage
	dd.nbD = 0; // nb de faces dépliées
	dd.lignes = g_new(Ligne, dd.nbFaces * 3);
	dd.nbL = 0;

	int gc;
	int tc = dd.premierTriangle;
	do {
		gc = dd.faces[tc][3];
		int nbTp = 0;
		int tcn = 0;
		dd.page[nbTp++] = tc;
		Vector2d delta = {0.0,0.0};
		Depliage sdC = { dd.nbP, tc, -1*(dd.nbP+1),  dd.nbPP, 0, delta};
		dd.sD[dd.nbD++] = sdC;
		dd.dispo[tc] = 0;
		_Bool ok;
		do {
			for (int vi = 0; vi < 3; vi++) {
				Voisin v = dd.voisins[tc][vi];
				int vc = v.nF;
				ok = dd.dispo[vc] && (dd.faces[vc][3] == gc);
				if (ok)
					for (int i = 0; (i < nbTp) && ok; i++)
						if (dd.page[i] == vc)
              ok = FALSE;
				if (ok) {
					Vector2d sauveV2dVoisin[3];	// Sauve v2d du voisin
					for (int i = 0; i < 3; i++)
						sauveV2dVoisin[i] = dd.v2d[vc][i];
					// Rapproche v2d[vc] de v2d[tc]
					Vector2d deltaV = Vector2dSub(dd.v2d[tc][vi], dd.v2d[vc][v.idx]);
					for (int i = 0; i < 3; i++)
						dd.v2d[vc][i] = Vector2dAdd(dd.v2d[vc][i], deltaV);
					// Tourne
					double a = calcAngle(dd.v2d[tc][vi],
						dd.v2d[tc][suiv(vi)], dd.v2d[vc][prec(v.idx)]);
					for (int i = 0; i < 3; i++)
						dd.v2d[vc][i] = Vector2dRotation(dd.v2d[tc][vi], dd.v2d[vc][i], a);
					// Teste dépassement page
					int nbTV = (nbTp + 1) * 3;
					Vector2d(*tmp) = g_new(Vector2d, nbTV);
					for (int i = 0; i < nbTp; i++)
						for (int j = 0; j < 3; j++)
							tmp[i * 3 + j] = dd.v2d[dd.page[i]][j];
					for (int j = 0; j < 3; j++)
						tmp[nbTp * 3 + j] = dd.v2d[vc][j];
					Vector2d vb[2];
					calcBoiteEnglobante(vb, tmp, nbTV);
					g_free(tmp);
					Vector2d dV = Vector2dSub(vb[1], vb[0]);
					if ((dV.x > limitePage.x) || (dV.y > limitePage.y))
						ok = FALSE;
					if (ok) // Teste collision avec la pièce
						for (int i = 0; (i < nbTp) && ok; i++)
							if (overlap(dd.v2d[dd.page[i]], dd.v2d[vc]))
								ok = FALSE;
					if (ok) {
						dd.page[nbTp++] = vc;
						Depliage sdC = { dd.nbP, vc, tc, 0 };
						dd.sD[dd.nbD++] = sdC;
						dd.dispo[vc] = 0;
					} else
            for (int i = 0; i < 3; i++)
							dd.v2d[vc][i] = sauveV2dVoisin[i];
				}
			}
			// Recherche prochaine face à déplier
			tcn++;
			if (tcn < nbTp) {
				tc = dd.page[tcn];
				ok = TRUE;
			} else
				ok = FALSE;
		} while (ok);

		// ajustement en bas à gauche
		Vector2d(*tmp) = g_new(Vector2d, nbTp * 3);
		for (int i = 0, k = 0; i < nbTp; i++)
			for (int j = 0; j < 3; j++)
				tmp[k++] = dd.v2d[dd.page[i]][j];
		Vector2d b[2];
		calcBoiteEnglobante(b, tmp, nbTp * 3);
		g_free(tmp);
		for (int i = 0; i < nbTp; i++)
			for (int j = 0; j < 3; j++)
				dd.v2d[dd.page[i]][j] =
					Vector2dSub(dd.v2d[dd.page[i]][j], Vector2dSub(b[0], marge));

		// répartition en lignes
		int nA;
		for (int i = 0; i < nbTp; i++)
			for (int j = 0; j < 3; j++) {
        NAff cleN;
				NAff* rechN;
				cleN.nMax = max(dd.page[i], dd.voisins[dd.page[i]][j].nF);
				cleN.nMin = min(dd.page[i], dd.voisins[dd.page[i]][j].nF);
				rechN = (NAff*)bsearch(&cleN, dd.lSNA, dd.nAff, sizeof(NAff), compAff);
				if (rechN != NULL)
					nA = rechN->a;
				else {
					cleN.a = dd.nAff;
					nA = dd.nAff;
					dd.lSNA[dd.nAff++] = cleN;
					qsort(dd.lSNA, dd.nAff, sizeof(NAff), compAff);
				}

				dd.lignes[dd.nbL] = LigneNew(
					dd.nbP,
					dd.nbL,
					dd.v2d[dd.page[i]][j],
					dd.v2d[dd.page[i]][suiv(j)],
					dd.page[i],
					j,
					dd.voisins[dd.page[i]][j].nF,
					dd.voisins[dd.page[i]][j].idx,
					nA);
        dd.nbL++;
			}

		supprimeDoublons(dd.lignes, dd.nbL);
		tc = premDispo(dd.dispo, dd.nbFaces);
		dd.nbP++;
	} while (tc > -1);

  qsort(dd.lSNA, dd.nAff, sizeof(NAff), compAff);
	qsort(dd.lignes, dd.nbL, sizeof(Ligne), compPg);

	dd.lAN = g_new(AN, dd.nbFaces);
	dd.nbAN = 0;

	sauveDonnees(dd);

	// init. languettes
  dd.sLgt = NULL;
  dd.nbLgt = 0;

	for (int i =0; i < dd.nbL; i++) {
    Ligne l = dd.lignes[i];
    if (l.nb == 1) {
      Lang l0 = { .n1 = l.n1, .n2 = l.n2, .v = l.n1 < l.n2 ? 1 : 0};
      dd.sLgt = g_realloc(dd.sLgt, sizeof(Lang) * ((size_t)dd.nbLgt + 1));
      dd.sLgt[dd.nbLgt] = l0;
			dd.nbLgt++;
    }
  }

  for (int i = 0; i < dd.nbLgt; i++) {
    Lang L = dd.sLgt[i];
    _Bool OK = TRUE;
    for(int j = 0; (j < dd.nAff) && OK; j++) {
      NAff A = dd.lSNA[j];
      if ((min(L.n1, L.n2) == A.nMin) && (max(L.n1, L.n2) == A.nMax))
      {
        dd.sLgt[i].o = A.a;
        OK = FALSE;
      }
    }
  }

 	qsort(dd.sLgt, dd.nbLgt, sizeof(Lang), compLang);
  sauveLanguettes(dd.sLgt, dd.nbLgt, dd.typeLang);
}

//---------- UI ----------
static void redessine_page_courante() {
  gtk_widget_queue_draw(dd.dpage[gtk_notebook_get_current_page(GTK_NOTEBOOK(dd.doc))]);
}

static void on_open_response (GtkDialog *dialog, int response, gpointer data) { // SELECTION DU FICHIER .OBJ
  if (response == GTK_RESPONSE_ACCEPT) {
    GtkFileChooser *chooser = GTK_FILE_CHOOSER (dialog);
    g_autoptr(GFile) file = gtk_file_chooser_get_file (chooser);
    char *ch = g_new(char, 100);
    sprintf(ch, "[deplieur UI:%s]", g_file_get_basename(file));
    gtk_window_set_title(GTK_WINDOW(win), ch);
    g_free(ch);
    sprintf(dd.fichierOBJ, "%s", g_file_get_path(file));

    deplier();
    rendu();

    //gtk_widget_set_sensitive(btnVoirOBJ, TRUE);
    //gtk_widget_set_sensitive(btnDeplier, TRUE);
  }

  gtk_window_destroy (GTK_WINDOW (dialog));
}

static void quit_activated(GSimpleAction *action, GVariant *parameter, gpointer user_data) {
  GApplication *app = G_APPLICATION (user_data);

  g_application_quit (app);
}

static void new_activated(GSimpleAction *action, GVariant *parameter, gpointer data) {
// CREATION DE L'INTERFACE PERMETTANT DE CHOISIR LE FICHIER OBJ
  GtkWidget *dialog;


  dialog = gtk_file_chooser_dialog_new ("Ouvre fichier .OBJ",
    GTK_WINDOW(win),
    GTK_FILE_CHOOSER_ACTION_OPEN,
    "_Cancel", GTK_RESPONSE_CANCEL,
    "_Open", GTK_RESPONSE_ACCEPT,
    NULL);

  GtkFileFilter *filtre = gtk_file_filter_new ();
  gtk_file_filter_add_suffix (filtre, "obj");
  gtk_file_filter_set_name (filtre, "OBJ Wavefront");
  gtk_file_chooser_add_filter (GTK_FILE_CHOOSER(dialog), filtre);

  gtk_widget_show (dialog);

  g_signal_connect (dialog, "response", G_CALLBACK (on_open_response), data);
}

static void open_activated(GSimpleAction *action, GVariant *parameter, gpointer user_data) {
}

static void ropen_activated(GSimpleAction *action, GVariant *parameter, gpointer user_data) {
  puts("*** RECHARGE ***");
  rendu();
}

static void lobj_activated(GSimpleAction *action, GVariant *parameter, gpointer user_data) {
}

static void edpa_activated(GSimpleAction *action, GVariant *parameter, gpointer user_data) {
}

static void save_activated(GSimpleAction *action, GVariant *parameter, gpointer user_data) {
}

static void sava_activated(GSimpleAction *action, GVariant *parameter, gpointer user_data) {
}

static void expg_activated(GSimpleAction *action, GVariant *parameter, gpointer user_data) {
}

static void modeB_toggled(GtkToggleButton *bouton) {
  if (gtk_toggle_button_get_active(bouton)) {
    const char *tmp = gtk_button_get_label(GTK_BUTTON(bouton));
    dd.mode = tmp[0] == 'P' ? MODE_PIECE
            : tmp[0] == 'A' ? MODE_ARETE
            : MODE_LANG;

    redessine_page_courante();
  }
}

static void appConfigure (GApplication *app, gpointer user_data) {
  GtkWidget *header;
  GtkWidget *bModeP, *bModeF, *bModeL;
  GtkWidget *box;

  win = gtk_application_window_new (GTK_APPLICATION (app));
  gtk_window_set_title (GTK_WINDOW (win), "[Deplieur UI]");
  gtk_window_set_default_size (GTK_WINDOW (win), 640, 1000);

  GtkWidget *bv = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_window_set_child(GTK_WINDOW(win), bv);

  #define NB_SECTIONS  5
  #define NB_ITEMS   9
  GMenu *menu     = g_menu_new ();
  GMenu *section[NB_SECTIONS];
  GSimpleAction *act[NB_ITEMS];
  GMenuItem *menu_item[NB_ITEMS];
  lmenu lm[NB_ITEMS] = {
    {0, "new",  "Nouveau dépliage", "win.new", new_activated},
    {0, "open", "Ouvrir dépliage", "win.open", open_activated},
    {0, "yyyy", "Réouvrir dépliage", "win.yyyy", ropen_activated},
    {1, "lobj", "charger modèle OBJ", "win.lobj", lobj_activated},
    {1, "edpa", "Editer paramètres", "win.edpa", edpa_activated},
    {2, "save", "Sauver", "win.save", save_activated},
    {2, "xxxx", "Sauver sous", "win.xxxx", sava_activated},
    {3, "expg", "Exporter gabarit", "win.expg", expg_activated},
    {4, "quit", "Quitter", "app.quit", quit_activated}
  };

  for (int i = 0; i < NB_SECTIONS; i++)
    section[i] = g_menu_new();
  for (int i = 0; i < NB_ITEMS; i++) {
    act[i] = g_simple_action_new (lm[i].nom, NULL);
    menu_item[i] = g_menu_item_new(lm[i].texte, lm[i].com);
    g_signal_connect (act[i], "activate", G_CALLBACK (lm[i].action),  app);
    g_action_map_add_action(i< NB_ITEMS-1 ? G_ACTION_MAP(win): G_ACTION_MAP(app), G_ACTION(act[i]));
    g_menu_append_item (section[lm[i].nSection], menu_item[i]);
    g_simple_action_set_enabled(act[i], TRUE);
    g_object_unref (menu_item[i]);
  }

  for (int i = 0; i < NB_SECTIONS; i++) {
    g_menu_append_section (menu, NULL, G_MENU_MODEL (section[i]));
    g_object_unref (section[i]);
  }

  GtkWidget *mb = gtk_menu_button_new();
  gtk_menu_button_set_icon_name(GTK_MENU_BUTTON(mb), "open-menu-symbolic");
  gtk_menu_button_set_menu_model(GTK_MENU_BUTTON(mb), G_MENU_MODEL(menu));

  dd.doc = gtk_notebook_new();
  gtk_notebook_set_tab_pos(GTK_NOTEBOOK(dd.doc), GTK_POS_RIGHT);
  gtk_notebook_set_scrollable(GTK_NOTEBOOK(dd.doc), TRUE);
  gtk_notebook_set_show_border(GTK_NOTEBOOK(dd.doc), TRUE);
  gtk_box_append (GTK_BOX (bv), dd.doc);

  header = gtk_header_bar_new ();
  gtk_header_bar_pack_end (GTK_HEADER_BAR (header), mb);

  box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
  gtk_widget_add_css_class (box, "linked");
  bModeP = gtk_toggle_button_new_with_label("Pièce");
  gtk_box_append (GTK_BOX (box), bModeP);
  bModeF = gtk_toggle_button_new_with_label("Arête");
  gtk_box_append (GTK_BOX (box), bModeF);
  bModeL = gtk_toggle_button_new_with_label("Lang.");
  gtk_box_append (GTK_BOX (box), bModeL);
  gtk_toggle_button_set_group(GTK_TOGGLE_BUTTON(bModeP), GTK_TOGGLE_BUTTON(bModeF));
  gtk_toggle_button_set_group(GTK_TOGGLE_BUTTON(bModeL), GTK_TOGGLE_BUTTON(bModeF));
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(bModeP), TRUE);
  dd.mode = MODE_PIECE;

  g_signal_connect(bModeP, "toggled", G_CALLBACK(modeB_toggled), bModeP);
  g_signal_connect(bModeF, "toggled", G_CALLBACK(modeB_toggled), bModeF);
  g_signal_connect(bModeL, "toggled", G_CALLBACK(modeB_toggled), bModeL);

  gtk_header_bar_pack_start (GTK_HEADER_BAR (header), box);

  gtk_window_set_titlebar (GTK_WINDOW (win), header);

  gtk_window_present(GTK_WINDOW(win));
}

int main (int argc, char **argv) {
  GtkApplication *app;
  int stat;

  setlocale(LC_ALL, "C");
  app = gtk_application_new (APPLICATION_ID, G_APPLICATION_FLAGS_NONE);
  g_signal_connect (app, "activate", G_CALLBACK (appConfigure), NULL);

  stat = g_application_run (G_APPLICATION (app), argc, argv);
  g_free (dd.faces);
  g_free (dd.voisins);
  g_free (dd.v2d);
  g_free (dd.v3d);
  g_free (dd.tCop);
  g_free (dd.sD);
  g_free (dd.dispo);
  g_free (dd.lAN);
  g_free (dd.lSNA);
  g_free (dd.page);
  g_free (dd.lignes);
  g_free (dd.rects);
  g_object_unref (app);
  return stat;
}
//fin deplieur
