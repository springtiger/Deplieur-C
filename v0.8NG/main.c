//---------- INCLUDES ----------
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <math.h>
#include <locale.h>

#ifdef WIN32
//#include "depl_txtW.h"
#include "cairo.h"
#include "cairo-pdf.h"
#define _USE_MATH_DEFINES
#else
//#include "depl_txt.h"
#include <cairo/cairo.h>
#include <cairo/cairo-pdf.h>
#endif

#include "structs.c"  // STRUCTURES

#include "main.h"

// TEXTES
char const * const textes[] = {
  "Nom fichier  :",												// 0
  "Échelle      :",												// 1
  "Erreur ouverture fichier",							// 2
  "Erreur fermeture fichier",							// 3
  "Format A(0..5) :",											// 4
  "points",																// 5
  "faces",																// 6
  "Taille des nombres (défaut 11) :",			// 7
  "Hauteur des languettes (défaut 10) :",	// 8
  "1er triangle :",												// 9
  "gabarit.pdf",													// 10
	"nb liens     :",												// 11
	"format page  :",												// 12
	"taille nums  :",												// 13
	"hauteur lang :",												// 14
	"Tourner des pages (O/N) ?",						// 15
	"N° de page (0 pour finir) ?",					// 16
	"page",																	// 17
	"Angle actuel",													// 18
	"Nouvel angle",													// 19
	"Créer fichier languettes (O/N) ?",			// 20
	"Type de languettes",										// 21
	" 0 : sans languettes",									// 22
	" 1 : 1 languette par paire",						// 23
	" 2 : 2 languettes par paire",					// 24
	"Votre choix (0,1,2): ",								// 25
	"Inverser une languette (O/N) ?",				// 26
	"Afficher le n° des faces (O/N) ?",			// 27
	"Pages",																// 28
	"Impossible de sauvegarder les données",// 29
	"n° languette à inverser ?", 						// 30
	"_Pièce",                               // 31
	"_Arête",                               // 32
	"_Lang.",                               // 33
	"%d non trouvé",                        // 34
	"Déplieur v0.8",                        // 35
	"Mode Pièce : sélectionner, déplacer, tourner une pièce\n[ALT+P]", // 36
	"Mode Arête : en rouge : couper, en bleu : associer\n[ALT+A]",     // 37
  "Mode Languette : cliquer pour basculer\n[ALT+L]",                 // 38
  "Entrer un n° pour le rechercher",        // 39
  "Zoomer",                                 // 40
  "Dézoomer",                               // 41
  "Tourner pièce courante vers la gauche",  // 42
  "Tourner pièce courante vers la droite",  // 43
  "Déplacer pièce courante vers le haut",   // 44
  "Déplacer pièce courante vers la gauche", // 45
  "Déplacer pièce courante vers la droite", // 46
  "Déplacer pièce courante vers le bas"     // 47
};

char const OUI = 'O';
char const NON = 'N';

//---------- CONSTANTES ----------
#define APPLICATION_ID "com.github.gilboonet.deplieur"

#ifndef WIN32
	#define max(a,b) (a>=b?a:b)
	#define min(a,b) (a<=b?a:b)
#endif

#define epsilon 0.0001
#define MAX_TAMPON 100
#define _CRT_SECURE_NO_WARNINGS

#define L_PLI_M 1
#define L_PLI_V 2
#define L_PLI_C	3
#define L_COUPE 4
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

#define D_HAUT       1
#define D_HAUT20    21
#define D_BAS        2
#define D_BAS20     22
#define D_GAUCHE     3
#define D_GAUCHE20  23
#define D_DROITE     4
#define D_DROITE20  24

#define R_GAUCHE    5
#define R_GAUCHE45  25
#define R_DROITE    6
#define R_DROITE45  26

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
//DonneesDep dd;

// IMPLEMENTATION
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
Vector2d Vector2dMul (Vector2d v, double d) {
  Vector2d r;

  r.x = v.x * d;
  r.y = v.y * d;

  return r;
}
Vector2d centroid(Vector2d* pts) {
	Vector2d r;

	r = Vector2dAdd(pts[0], Vector2dAdd(pts[1], pts[2]));
	r = Vector2dDiv(r, 3);

	return r;
}
Vector2d milieu (Vector2d p1, Vector2d p2) {
	return Vector2dDiv(Vector2dAdd(p1, p2), 2);
}
Vector2d vPetit(Vector2d p1, Vector2d p2) {
	return Vector2dNew(fmin(p1.x, p2.x), fmin(p1.y, p2.y));
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
Vector2d Vector2dPlusPetit(Vector2d p1, Vector2d p2) {
  if ((p1.x < p2.x) || (eqd(p1.x, p2.x) && (p1.y < p2.y)))
    return p1;
  else
    return p2;
}
Vector2d Vector2dPlusGrand(Vector2d p1, Vector2d p2) {
  if ((p1.x < p2.x) || (eqd(p1.x, p2.x) && (p1.y < p2.y)))
    return p2;
  else
    return p1;
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
Ligne LigneNew (int iPage, int iPiece, int id, Vector2d p1, Vector2d p2, int n1, int i1, int n2, int i2, int nA) {
	Ligne l;

	l.nP = iPage;
	l.nPP = iPiece;
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

  return (iMax1 == iMax2) ? iMin1 - iMin2 : iMax1 - iMax2;
}
int compAff (const void * el1, const void * el2) {
	NAff v1 = * (const NAff *) el1;
	NAff v2 = * (const NAff *) el2;

	return (v1.nMax != v2.nMax) ? v1.nMax - v2.nMax : v1.nMin - v2.nMin;
}
int compAffa (const void * el1, const void * el2) {
	NAff v1 = * (const NAff *) el1;
	NAff v2 = * (const NAff *) el2;

	return v1.a - v2.a;
}
int compLang (const void * el1, const void * el2) {
	Lang v1 = * (const Lang *) el1;
	Lang v2 = * (const Lang *) el2;

	return (v1.n1 != v2.n1) ? v1.n1 - v2.n1 : v1.n2 - v2.n2;
}
int compLangO (const void * el1, const void * el2) {
	Lang v1 = * (const Lang *) el1;
	Lang v2 = * (const Lang *) el2;

	return v1.o - v2.o;
}
int compPg (const void * el1, const void * el2) {
	Ligne v1 = * (const Ligne *) el1;
	Ligne v2 = * (const Ligne *) el2;

	//return v1.nP - v2.nP;
	return v1.nP * 1000 + v1.nPP
      - (v2.nP * 1000 + v2.nPP);

}
double degToRad(double degrees) {
	return degrees * pi / 180;
}
double radToDeg(double radians) {
  return (radians *180) / pi;
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
	for (int i = 0; i < nb; i++)
		if (l[i])
			n++;

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
	for (int i = 0; i < n; i++)
		for (int j = 0; j < 3; j++) {
			int vi = 0;
			_Bool ok = FALSE;
			do {
				if (vi != i) {
					for (int k = 0; (k < 3) && (!ok); k++)
						if ((f[vi][k] == f[i][suiv(j)]) && (f[vi][suiv(k)] == f[i][j])) {
							v[i][j]= VoisinNew(vi, suiv(k));
							ok = TRUE;
						}
					if (!ok)
						vi++;
				} else
					vi++;
			} while ((vi < n) && (!ok));
		}
}
void calculeCop(int n, Voisin voisins[][3], Cop* tCop, Vector3d v3d[][3]) {
	int nbCop = 0;
	for (int i = 0; i < n; i++)
		for (int j = 0; j < 3; j++) {
			int nV = voisins[i][j].nF;
			Vector3d p;

			if      (Vector3dEq(v3d[i], v3d[nV], 0))
				p = v3d[nV][0];
			else if (Vector3dEq(v3d[i], v3d[nV], 1))
				p = v3d[nV][1];
			else
				p = v3d[nV][2];

			double c = Vector3dIsCoplanar(v3d[i], p);
			tCop[nbCop].cop = c;
			tCop[nbCop].nF =  i;
			tCop[nbCop].nV = nV;
			nbCop++;
		}
}
int sauveLanguettes(Lang * sL, int nbL, int mode, char *nomFichier) {
  char * nomFichierDonnees = (nomFichier[0] == '\0') ? g_build_filename(g_get_tmp_dir(), "donnees.lng", (gchar *) 0) : nomFichier;

  int n = strlen(nomFichierDonnees)-4;
  memcpy(nomFichierDonnees +n, ".lng", 4);

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

	//puts("SAUVE LANGUETTES");

	return 0;
}
int sauveDonnees(DonneesDep *d) {
  char * nomFichierDonnees = (d->fichierDAT[0] == '\0')
    ? g_build_filename(g_get_tmp_dir(), "donnees.dep", (gchar *) 0) : d->fichierDAT;

	FILE * fd;
	int rc;

	fd = fopen(nomFichierDonnees, "w");
	if (!fd) {
		perror(textes[29]);
		return -1;
	}

	fprintf(fd, "%s\n",     d->fichierOBJ);
	fprintf(fd, "%5.2lf\n", d->echelle);
	fprintf(fd, "%2d\n",    d->formatPage);
	fprintf(fd, "%2d\n",    d->typeLang);
	fprintf(fd, "%4d\n",    d->premierTriangle);
	fprintf(fd, "%5.2f\n",  d->tailleNums);
	fprintf(fd, "%2d\n",    d->hauteurLang);

	for (int i = 0; i < d->nbD; i++)
		if (d->sD[i].orig < 0)
			fprintf(fd, "%4d %4d %4d %6.2lf %6.2lf\n", d->sD[i].orig, d->sD[i].face, d->sD[i].a, d->sD[i].d.x, d->sD[i].d.y);
		else
			fprintf(fd, "%4d %4d\n", d->sD[i].orig, d->sD[i].face);

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
	for (int i = 1; i < nbL; i++)
		if ((lignes[dOK].nP == lignes[i].nP)
        && ((Vector2dEq(lignes[dOK].p1, lignes[i].p1) && Vector2dEq(lignes[dOK].p2, lignes[i].p2))
        ||  (Vector2dEq(lignes[dOK].p1, lignes[i].p2) && Vector2dEq(lignes[dOK].p2, lignes[i].p1)))) {
			lignes[i].id = -1;
			lignes[dOK].nb++;
		} else
			dOK = i;
}
void afficheNum(DonneesDep *d, cairo_t *cr, int num, Vector2d p1, Vector2d p2, Coul c, int typeA) {
  cairo_text_extents_t te;
	char ch[10];
	Vector2d m;
	double fheight = -3, nx, ny, a;

	cairo_save(cr);
	snprintf(ch, 10, "%d", num);
	m = milieu(p1, p2);
	if ((d->mode != MODE_PIECE) && (typeA > 0)) {
    cairo_rectangle(cr, m.x-2, m.y-2, 5, 5);
    if (typeA == 1)
      cairo_set_source_rgb(cr, 0, 0, 1);
    else if (typeA == 2)
      cairo_set_source_rgb(cr, 1, 0, 0);
    cairo_fill(cr);
  }

	if (typeA == 0) {
    if (d->idRecherche == num)
      cairo_set_source_rgb(cr, 1,0,0);
    else
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
}
void faitLigne(DonneesDep *d, cairo_t *cr, Vector2d p1, Vector2d p2, int typeL) {
	int hLang = d->hauteurLang;

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
			trapeze(pts, p1, p2, hLang*d->fz, 0.45);
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
int chargeOBJ(DonneesDep *d) {
  //puts("DEBUT CHARGE OBJ");
  setlocale(LC_NUMERIC, "C");

	Vector3d* sommets = NULL;
	d->nbSommets = 0;
	int* faces0 = NULL;
	d->nbFaces = 0;

	FILE* fs;
	if (!(fs = fopen(d->fichierOBJ, "r"))) {
		printf("erreur ouverture %s\n", d->fichierOBJ);
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
			Vector3d s = { v[0] * d->echelle, v[1] * d->echelle, v[2] * d->echelle };
			Vector3d* tmp;
			tmp = (Vector3d*)realloc(sommets, sizeof(Vector3d) * ((size_t)d->nbSommets + 1));
			if (tmp) {
				sommets = tmp;
        sommets[d->nbSommets++] = s;
			}
		} else if ((tL0 == 'g') && (tL1 == ' ')) { // g = group (groupe)
			gc++;
		} else if ((tL0 == 'f') && (tL1 == ' ')) { // f = face
			char c[3][20];
			 sscanf(ligneLue, "f %s %s %s", c[0], c[1], c[2]);
			for (int i = 0; i < 3; i++) {
				size_t pos = strspn(c[i], "0123456789");
				char ch[20];
        strncpy(ch, c[i], pos);
        ch[pos] = '\0';
        fc[i] = atoi(ch);
			}
			fc[3] = gc;
			int* tmp = (int*)realloc(faces0, sizeof(int) * ((size_t)d->nbFaces + 1) * 4);
			if (tmp) {
				faces0 = tmp;
        for (int i = 0; i < 4; i++)
          faces0[d->nbFaces * 4 + i] = fc[i] - 1;
        d->nbFaces++;
			}
		}
	}
	if (fclose(fs)) {
		perror(textes[3]);
		exit(1);
	}

	g_free(d->faces);
	d->faces = calloc(d->nbFaces, sizeof * d->faces);
	int n = 0;
	for (int i = 0; i < d->nbFaces; i++) {
		for (int j = 0; j < 4; j++)
			d->faces[i][j] = faces0[n++];
	}
	g_free(faces0);

	//printf("CHARGE OBJ : %d faces lues\n", d->nbFaces);

	// VOISINS
	g_free(d->voisins);
	d->voisins = calloc(d->nbFaces, sizeof * d->voisins);
	calculeVoisinage(d->faces, d->nbFaces, d->voisins);

	// NUMEROS
  g_free(d->lSNA);
	d->lSNA = g_new(NAff, d->nbFaces * 3);
	d->nAff = 0;

	int nA = 0, nMin, nMax;
	for (int i = 0; i < d->nbFaces; i++){
    for (int j = 0; j < 3; j++) {
      int n = d->voisins[i][j].nF;
      nMin = min(i, n);
      nMax = max(i, n);
      NAff cleN;
      NAff* rechN;
      cleN.nMax = max(i, n);
      cleN.nMin = min(i, n);
      qsort(d->lSNA, d->nAff, sizeof(NAff), compAff);
      rechN = (NAff*)bsearch(&cleN, d->lSNA, d->nAff, sizeof(NAff), compAff);

      if (rechN == NULL) {
        d->lSNA[d->nAff].nMin = nMin;
        d->lSNA[d->nAff].nMax = nMax;
        d->lSNA[d->nAff].a = nA++;
        d->nAff++;
      }
    }
	}

	// V3D V2D
	g_free(d->v3d);
	d->v3d = calloc(d->nbFaces, sizeof * d->v3d);
	g_free(d->v2d);
	d->v2d = calloc(d->nbFaces, sizeof * d->v2d);
	for (int i = 0; i < d->nbFaces; i++) {
		for (int j = 0; j < 3; j++)
			d->v3d[i][j] = sommets[d->faces[i][j]];
		Vector3dD2ize(d->v3d[i], d->v2d[i]);
	}
	g_free(sommets);

	// estCOP
  g_free(d->tCop);
	d->tCop = calloc((size_t)d->nbFaces * 3, sizeof * d->tCop);
	calculeCop(d->nbFaces, d->voisins, d->tCop, d->v3d);

  //puts("FIN CHARGE OBJ");

	return EXIT_SUCCESS;
}
int retourneLigne(char *contenu, char *tampon, int dep) {
  int n = 0;
  while (TRUE) {
    char c = contenu[dep];
    if (c == '\n') {
      tampon[n] = 0;
      return dep + 1;
    } else
      tampon[n++] = c;
    dep++;
  }
}
int chargeDonnees(DonneesDep *d) {
  //puts("DEBUT CHARGE DONNEES");
  char * nomFichierDonnees = d->fichierDAT[0] == '\0'
    ? g_build_filename(g_get_tmp_dir(), "donnees.dep", (gchar *) 0)
    : d->fichierDAT;

	GFile *file = g_file_new_for_path(nomFichierDonnees);
	//printf("nom fichier donnees : %s\n", g_file_get_path(file));

  char *contents;
  gsize length;

	if (!g_file_load_contents(file, NULL, &contents, &length, NULL, NULL))
    perror("erreur lecture fichier donnees.dep");

	int fi = 0;
  char lu[60];

  fi = retourneLigne(contents, d->fichierOBJ, fi);
  fi = retourneLigne(contents, lu, fi);
  d->echelle = strtod(lu, NULL);

  fi = retourneLigne(contents, lu, fi);
  d->formatPage = atoi(lu);

  fi = retourneLigne(contents, lu, fi);
  d->typeLang = atoi(lu);

  fi = retourneLigne(contents, lu, fi);
  d->premierTriangle = atoi(lu);

  fi = retourneLigne(contents, lu, fi);
  d->tailleNums = atof(lu);

  fi = retourneLigne(contents, lu, fi);
  d->hauteurLang = atoi(lu);

  /*puts("=============================================");
  printf("LU fichier          = %s\n", d->fichierOBJ);
  printf("LU echelle          = %lf\n",d->echelle);
  printf("LU formatPage       = %d\n", d->formatPage);
  printf("LU typeLang         = %d\n", d->typeLang);
  printf("LU premierTriangle  = %d\n", d->premierTriangle);
  printf("LU tailleNums       = %f\n", d->tailleNums);
  printf("LU hauteurLang      = %d\n", d->hauteurLang);
*/
	g_free(d->sD);
	d->sD = NULL;
	d->nbD = 0;
	Depliage sD0;

	int d0, d1, d2;
	char *cdx, *cdy;
	Vector2d delta = {0, 0};
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
		Depliage* tmp = (Depliage*)realloc(d->sD, sizeof(Depliage) * ((size_t)d->nbD + 1));
		if (tmp) {
			d->sD = tmp;
			d->sD[d->nbD] = sD0;
			d->nbD++;
		}
  } while (fi < length);
  //printf("LU : Depliage %d etapes\n", d->nbD);

	// charge données languettes
	d->nbLgt = 0;
	g_free(d->sLgt);
	d->sLgt = NULL;
	Lang sLgt0;

	int d3;
	int nvl;
	//char* nomFichierLanguettes = "donnees.lng";
  //char * nomFichierLanguettes = g_build_filename(g_get_tmp_dir(), "donnees.lng", (gchar *) 0);
  char * nomFichierLanguettes = nomFichierDonnees;
  int n = strlen(nomFichierLanguettes) - 4;
  //strncpy(nomFichierDonnees + n, ".lng", 4);
  memcpy(nomFichierDonnees + n, ".lng", 4);

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
	    d->sLgt = g_realloc(d->sLgt, sizeof(Lang) * ((size_t)d->nbLgt + 1));
      d->sLgt[d->nbLgt] = sLgt0;
			d->nbLgt++;
		}
		if (fclose(fd) == EOF) {
			perror(textes[3]);
			exit(1);
		}
	}

  g_free(contents);
	g_object_unref(file);

  //printf("LU : Languettes %d \n", d->nbLgt);

	chargeOBJ(d);

	//puts("CHARGE DONNEES AVANT DEPLIAGE");

	// DEBUT DEPLIAGE

	int nbP = -1;
	int nbPP = -1;
  int dOrig = 9999;
	Vector2d* vMin = NULL;
	g_free(d->lignes);
	d->lignes = g_new(Ligne, d->nbFaces * 3);
	d->nbL = 0;
	for (int i = 0; i < d->nbD; i++) {
		if (d->sD[i].orig < 0) {
			nbPP++;
			if (d->sD[i].orig != dOrig) {
        dOrig = d->sD[i].orig;
        nbP++;
      }
			int f = d->sD[i].face;
			int a = d->sD[i].a;
			if (a != 0) {
				Vector2d m = centroid(d->v2d[f]);
				for (int vi = 0; vi < 3; vi++)
					d->v2d[f][vi] = Vector2dRotation(m, d->v2d[f][vi], degToRad(a));
			}
			Vector2d * tmp = (Vector2d*)realloc(vMin, sizeof(Vector2d) * ((size_t)nbPP + 1));
			if (tmp) {
				vMin = tmp;
				vMin[nbPP] = vPetit(d->v2d[f][0], vPetit(d->v2d[f][1], d->v2d[f][2]));
			}
		}
		d->sD[i].page = nbP;
		d->sD[i].piece= nbPP;

		if (d->sD[i].orig > -1) {
			int tc = d->sD[i].orig;
			int vc = d->sD[i].face;
			int vi = d->voisins[tc][0].nF == vc ? 0 : d->voisins[tc][1].nF == vc ? 1 : 2;
			Voisin v = d->voisins[tc][vi];

			// 1°) rapproche v2d[vc] de v2d[tc]
			Vector2d deltaV = Vector2dSub(d->v2d[tc][vi], d->v2d[vc][v.idx]);
			for (int n = 0; n < 3; n++)
				d->v2d[vc][n] = Vector2dAdd(d->v2d[vc][n], deltaV);
			// 2°) tourne v2d[vc]
			double a = calcAngle(d->v2d[tc][vi], d->v2d[tc][suiv(vi)], d->v2d[vc][prec(v.idx)]);
			for (int n = 0; n < 3; n++) {
				d->v2d[vc][n] = Vector2dRotation(d->v2d[tc][vi], d->v2d[vc][n], a);
				vMin[nbPP] = vPetit(vMin[nbPP], d->v2d[vc][n]);
			}
		}
	}

	//puts("CHARGE DONNEES APRES DEPLIAGE");

	for (int i = 0; i < d->nbD; i++) {
		int tc = d->sD[i].face;
		int ppc = d->sD[i].piece;
		for (int j = 0; j < 3; j++)
			d->v2d[tc][j] = Vector2dSub(d->v2d[tc][j], Vector2dSub(vMin[ppc], marge));
	}
	free(vMin);

	//puts("CHARGE DONNEES APRES DECALAGE");

  Vector2d dC = {0, 0};
	for (int i = 0; i < d->nbD; i++) {
		int tc = d->sD[i].face;
		int pc = d->sD[i].page;
		int ppc = d->sD[i].piece;
		if (d->sD[i].orig < 0)
      dC = d->sD[i].d;
		for (int j = 0; j < 3; j++) {
		  // rech nA
      NAff cleN;
      NAff* rechN;
      cleN.nMax = max(tc, d->voisins[tc][j].nF);
      cleN.nMin = min(tc, d->voisins[tc][j].nF);
      rechN = (NAff*)bsearch(&cleN, d->lSNA, d->nAff, sizeof(NAff), compAff);
		  int nA = rechN->a;

			d->lignes[d->nbL] = LigneNew(pc, ppc, d->nbL,
				Vector2dAdd(d->v2d[tc][j], dC), Vector2dAdd(d->v2d[tc][suiv(j)], dC),
				tc, j,
				d->voisins[tc][j].nF, d->voisins[tc][j].idx,
				nA);
			d->nbL++;
		}
	}

	supprimeDoublons(d->lignes, d->nbL);

	qsort(d->lignes, d->nbL, sizeof(Ligne), compPg);
	qsort(d->sLgt, d->nbLgt, sizeof(Lang), compLang);

  //puts("FIN CHARGE DONNEES");

	return EXIT_SUCCESS;
}
int init_cairo(DonneesDep *d) {
	d->surface = cairo_pdf_surface_create(textes[10], formats[d->formatPage].x, formats[d->formatPage].y);
	d->cr = cairo_create(d->surface);

	cairo_select_font_face(d->cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
	cairo_set_font_size(d->cr, d->tailleNums);
	cairo_set_line_width(d->cr, 1);

	return EXIT_SUCCESS;
}
void closeCairo(DonneesDep *d) {
	cairo_surface_destroy(d->surface);
	cairo_destroy(d->cr);
	cairo_debug_reset_static_data();
}

static void rotated (GtkGestureClick *gesture, int n_press, double x, double y, DonneesDep *d) {
  Vector2d pt = Vector2dNew(x, y);
  int nPage = gtk_notebook_get_current_page(GTK_NOTEBOOK(d->doc));
  double a = angle(d->coords, pt);
  _Bool OK = FALSE;
  int trouve = -1;

  if (d->idCourant > -1) {
    //printf("piece %d -- ROTATION : %6.2lf\n", d->idCourant, radToDeg(a));
    for (int i = 0; i < d->nbD; i++) {
      Depliage D = d->sD[i];
      if ((D.orig < 0) && (D.page == nPage) && (D.piece == d->idCourant)) {
        OK = TRUE;
        trouve = i;
        break;
      }
    }
    if (OK) {
      d->sD[trouve].a = d->sD[trouve].a - (int)radToDeg(a);
      sauveDonnees(d);
      d->idCourant = -1;
      rendu(d, nPage);
    }

    d->idCourant = -1;
    redessine_page_courante(d);
  }
}

int inversePiece(Depliage* SDi, tB* B, int nbB, int ln0, int ln3, int ln0orig) {
  // Init. du dépliage
  SDi[0].orig = ln0orig;
  SDi[0].face = B[0].face;
  int nbSDi = 1;

  bool bk = FALSE;
  int snbSDi = -1;
  while(nbB > 0){
    if (snbSDi == nbSDi) {
      break;
    } else
      snbSDi = nbSDi;

    for (int i = 0; i < nbB; i++) {
      if (B[i].mis == 1)
        continue;
      for (int j = nbSDi-1; j >= 0; j--) {
        if (SDi[j].face == B[i].face) {
          _Bool ok = TRUE;
          for (int k = 0; k < nbSDi; k++)
            if (SDi[k].face == B[i].orig) {
              ok = FALSE;
              break;
            }
          if (ok) {
            SDi[nbSDi].orig = B[i].face;
            SDi[nbSDi].face = B[i].orig;
            nbSDi++;
            B[i].mis = 1;
            bk = TRUE;
            break;
          }
        } else if (SDi[j].face == B[i].orig) {
          _Bool ok = TRUE;
          for (int k = 0; k < nbSDi; k++)
            if (SDi[k].face == B[i].face) {
              ok = FALSE;
              break;
            }
          if (ok) {
            SDi[nbSDi].orig = B[i].orig;
            SDi[nbSDi].face = B[i].face;
            nbSDi++;
            B[i].mis = 1;
            bk = TRUE;
            break;
          }
        }
      }

      if (bk) {
        bk = FALSE;
        break;
      }
    }
  }
  return nbSDi;
}

static void pressed (GtkGestureClick *gesture, int n_press, double x, double y, DonneesDep *d) {
  _Bool OK = FALSE;
  int nPage = gtk_notebook_get_current_page(GTK_NOTEBOOK(d->doc));
  d->pCourante = nPage;
  Vector2d pt = Vector2dNew(x, y);

  if (d->mode == MODE_PIECE) {
    if (d->nbRects > 0) {
      g_free(d->rects);
      d->rects = NULL;
      d->nbRects = 0;
    }
  } else {
    if (d->nbTris > 0) {
      g_free(d->tris);
      d->tris = NULL;
      d->nbTris = 0;
    }
  }

  int trouve = -1;

  if (d->mode != MODE_PIECE) {
    for (int i = 0; i < d->nbRects; i++) {
      rectT r = d->rects[i];
      if ((x >= r.p1.x) && (x <= r.p2.x) && (y >= r.p1.y) && (y <= r.p2.y)) {
        OK = TRUE;
        trouve = i;
        break;
      }
    }
    if (OK) {
      Ligne l = d->lignes[d->rects[trouve].id];
      int estArete = d->rects[trouve].type == 1;
      //printf("clic sur arête %d (%d <->  %d)\n", l.nA, l.n1, l.n2);
      int ln0 = -1, ln1 = -1, ln2 = -1, ln3 = -1, ln4 = -1;
      // recherche des lignes concernées
      int orig0;
      for (int i = 0; i < d->nbD; i++) {
        Depliage D = d->sD[i];
        if (D.face == l.n1)
          ln1 = i;
        if (D.face == l.n2)
          ln2 = i;
        if ((orig0 == 0) && (D.page == l.nP))
          orig0 = D.orig;
      }
      int face1 = d->sD[ln1].face;
      int face2 = d->sD[ln2].face;

      if ((d->mode == MODE_ARETE) && estArete) {
        int piece1 = d->sD[ln1].piece;
        int piece2 = d->sD[ln2].piece;
        int page1 = d->sD[ln1].page;
        if (d->sD[ln1].piece == piece2) { // Deplacement au sein d'une meme piece
          if (ln2 > ln1) {
            //puts("depl SIMPLE");
            d->sD[ln2].orig = face1;
          } else { // inverser ordre de la piece
            int nbB = 0;
            for (int i = 0; i < d->nbD; i++) {
              if (d->sD[i].piece == piece1) {
                nbB++;
                if (ln0 == -1)
                  ln0 = i;
                ln3 = i;
              }
            }
            //printf("ln1:%d ln0:%d ln2:%d ln3:%d\n", ln1, ln0, ln2, ln3);
            tB B[nbB];

            for (int i = 0; i < nbB; i++) {
              B[i].face = d->sD[ln3 - i].face;
              B[i].orig = (B[i].face == face2) ? face1 : d->sD[ln3 - i].orig;
              B[i].mis = 0;
            }
            Depliage SDi[nbB];
            inversePiece(SDi, B, nbB, ln0, ln3, d->sD[ln0].orig);

            for (int i = ln0; i <= ln3; i++){
              //if (SDi[i-ln0].face != SDi[i-ln0].orig) {
                d->sD[i].face = SDi[i-ln0].face;
                d->sD[i].orig = SDi[i-ln0].orig;
              //}
            }
          }
        } else {  // Deplacement entre deux pieces
          int nbB = 0;
          for (int i = 0; i < d->nbD; i++) {
            if (d->sD[i].piece == piece2) {
              nbB++;
              if (ln0 == -1)
                ln0 = i;
              ln3 = i;
            } else if (d->sD[i].piece == piece1) {
              ln4 = i;
            }
          }

          //printf("ln1:%d ln0:%d ln2:%d ln3:%d ln4:%d\n", ln1, ln0, ln2, ln3, ln4);
          //printf("piece1:%d piece2:%d\n", piece1, piece2);
          /*for (int i = 0; i < d->nbD; i++) {
            Depliage d = d->sD[i];
            if ((d.piece == piece1) || (d.piece == piece2))
              printf("%d : %d %d %d %d\n", i, d.page, d.piece, d.orig, d.face);
          }*/

          tB B[nbB];
          int ii = 1, in;
          for (int i = 0; i < nbB; i++) {
            if (i+ln0 == ln2)
              in = 0;
            else {
              in = ii;
              ii++;
            }
            B[in].orig = d->sD[i+ln0].orig;
            B[in].face = d->sD[i+ln0].face;
            B[in].mis = 0;
          }

          /*puts("-B-");
          for (int i = 0; i < nbB; i++) {
            printf("%d: %d %d\n", i, B[i].orig, B[i].face);
          }*/

          Depliage SDi[nbB];
          int nbSDi;

          if (ln1 < ln2) {
            if (ln1 != ln0) {
              nbSDi = inversePiece(SDi, B, nbB, ln0, ln3, d->sD[ln0].orig);
              SDi[0].orig = face1;

              /*puts("-sDi-");
              for (int i = 0; i < nbSDi; i++) {
                printf("%d: %d %d\n", i, SDi[i].orig, SDi[i].face);
              }*/
              for (int i = ln0-1, j = ln3; i > ln4; i--) {
                d->sD[j--] = d->sD[i];
              }

              for (int i = 0, j = ln4+1; i < nbSDi; i++)
                if (SDi[i].face >= 0) {
                  if (SDi[i].orig != SDi[i].face) {
                    d->sD[j].orig = SDi[i].orig;
                    d->sD[j].face = SDi[i].face;
                    d->sD[j].page = page1;
                    d->sD[j].piece = piece1;
                    j++;
                  }
                }
            }
          } else {
            if (ln2 != ln0) {
              nbSDi = inversePiece(SDi, B, nbB, ln0, ln3, d->sD[ln0].orig);
              SDi[0].orig = face1;
            } else {
              nbSDi = ln3 - ln0 +1;
              SDi[0].orig = face1;
              SDi[0].face = face2;
              for (int i = 1; i < nbSDi; i++){
                SDi[i].orig = d->sD[ln0 +i].orig;
                SDi[i].face = d->sD[ln0 +i].face;
              }
            }
            /*puts("-sDi-");
            for (int i = 0; i < nbSDi; i++) {
              printf("%d: %d %d\n", i, SDi[i].orig, SDi[i].face);
            }*/

            int nb = ln0;
            for (int i = ln3+1; i <= ln4; i++) {
              d->sD[nb++] = d->sD[i];
            }

            for (int i = 0; i < nbSDi; i++)
              if (SDi[i].face >= 0) {
                if (SDi[i].orig != SDi[i].face) {
                  d->sD[nb].orig = SDi[i].orig;
                  d->sD[nb].face = SDi[i].face;
                  d->sD[nb].page = page1;
                  d->sD[nb].piece = piece1;
                  nb++;
                }
              }
          }
        }

      } else if ((d->mode == MODE_ARETE) && !estArete) { // scission
        //printf("SCISSION: %d (%d) %d (%d)\n", ln1, face1, ln2, face2);
        int piece = d->sD[ln1].piece;
        int numPieces[d->nbD];
        numPieces[0] = 0;
        for (int i = 0; i < d->nbD; i++) numPieces[i] = 0;
        for (int i = 0; i < d->nbD; i++) numPieces[d->sD[i].piece] = 1;
        int nPLibre;
        for (nPLibre = 0; numPieces[nPLibre] == 1; nPLibre++);
        //printf("piece libre : %d\n", nPLibre);

        int nb = 0;
        for (int i = 0; i < d->nbD; i++) {
          if (d->sD[i].piece == piece) {
            if (i >= ln2) nb++;
            if (ln0 == -1) ln0 = i;
          }
        }
        Depliage D1[nb], D2[nb];
        int nb1 = 0;
        D2[0] = d->sD[ln2];
        D2[0].piece = nPLibre;
        D2[0].orig = d->sD[ln0].orig;
        int nb2 = 1;
        for (int i = ln2 +1 ; i < (ln2 +nb); i++) {
          Depliage dpl = d->sD[i];
          _Bool ok = false;
          for (int j = 0; j < nb2; j++) {
            if (dpl.orig == D2[j].face) {
              ok = true;
              break;
            }
          }
          if (ok) {
            D2[nb2] = dpl;
            D2[nb2].piece = nPLibre;
            nb2++;
          } else {
            D1[nb1++] = dpl;
          }
        }
        //puts("-D1-");
        for (int i = 0; i < nb1; i++) {
          Depliage dpl = D1[i];
          d->sD[ln2+i] = dpl;
          //printf("%d: %d %d %d %d\n", i, d.page, d.piece, d.orig, d.face);
        }
        //puts("-D2-");
        for (int i = 0; i < nb2; i++) {
          Depliage dpl = D2[i];
          d->sD[ln2+i+nb1] = dpl;
          //printf("%d: %d %d %d %d\n", i, d.page, d.piece, d.orig, d.face);
        }

        // FIN Scission
      } else if ((d->mode == MODE_LANG) && estArete) {
        //printf("NB LANG : %d\n", d->nbLgt);
        int L1 = -1, L2 = -1;
        for (int i = 0; i < d->nbLgt; i++) {
          Lang L = d->sLgt[i];
          if ((L.n1 == face1) && (L.n2 == face2)) {
            L1 = i;
          } else if ((L.n1 == face2) && (L.n2 == face1)) {
            L2 = i;
          }
        }

        //printf("LANG: %d %d\n", L1, L2);

        int etat1 = L1 > -1 ? d->sLgt[L1].v : L2 > -1 ? 1 - d->sLgt[L2].v : 0;
        if (etat1 == -1)
          etat1 = 0;

        if (L1 == -1) {
          Lang L = {face1, face2, l.nA, 1 - etat1};
          d->sLgt = g_realloc(d->sLgt, sizeof(Lang) * ((size_t)d->nbLgt + 1));
          d->sLgt[d->nbLgt] = L;
          d->nbLgt++;
        } else {
          d->sLgt[L1].v = 1 - etat1;
        }

        if (L2 == -1) {
          Lang L = {face2, face1, l.nA, etat1};
          d->sLgt = g_realloc(d->sLgt, sizeof(Lang) * ((size_t)d->nbLgt + 1));
          d->sLgt[d->nbLgt] = L;
          d->nbLgt++;
        } else {
          d->sLgt[L2].v = etat1;
        }
      }
      if (d->mode == MODE_LANG) {
        sauveLanguettes(d->sLgt, d->nbLgt, M_LANG_SAUV, d->fichierDAT);
      } else
        sauveDonnees(d);


      d->idCourant = -1;
      rendu(d, nPage);
      redessine_page_courante(d);
    }
  } else {
    for (int i = 0; i < d->nbTris; i++) {
      OK = dansTriangle(d->tris[i].p1, d->tris[i].p2, d->tris[i].p3, pt);
      if (OK) {
        trouve = i;
        break;
      }
    }
    if (OK) {
      d->idCourant = d->tris[trouve].id;
      d->coords = Vector2dNew(x, y);
      //printf("clic sur piece %d \n", d->tris[trouve].id);
    } else {
      if (d->idCourant != -1) {
        //printf("Deplacement : %6.2lf %6.2lf\n", d->coords.x - x, d->coords.y -y);
        OK = FALSE;
        for (int i = 0; i < d->nbD; i++) {
          Depliage dpl = d->sD[i];
          if ((dpl.orig < 0) && (dpl.page == nPage) && (dpl.piece == d->idCourant)) {
            //printf(" %d : orig %d face %d (%d %d) x:%6.2lf y:%6.5lf\n", i, d.orig, d.face, d.page, d.piece, d.d.x, d.d.y);
            OK = TRUE;
            trouve = i;
            break;
          }
        }
        if (OK) {
          Vector2d c = Vector2dNew(d->coords.x - x, d->coords.y -y);
          c = Vector2dDiv(c, d->fz);
          d->sD[trouve].d = Vector2dSub(d->sD[trouve].d, c);
          sauveDonnees(d);
          d->idCourant = -1;
          rendu(d, nPage);
        }
      }
      d->idCourant = -1;
    }
    redessine_page_courante(d);
  }
}

static void tournePiece (GtkEntry* self, gpointer user_data) {
  DonneesDep *d = (DonneesDep*)user_data;
  int sens = d->id_action;

  _Bool OK;
  int trouve;
  int nPage = gtk_notebook_get_current_page(GTK_NOTEBOOK(d->doc));

  if (d->idCourant > -1) {
    OK = FALSE;
    for (int i = 0; i < d->nbD; i++) {
      Depliage dpl = d->sD[i];
      if ((dpl.orig < 0) && (dpl.piece == d->idCourant)) {
        OK = TRUE;
        trouve = i;
        break;
      }
    }
    if (OK) {
      int sauveIdC = d->idCourant;
      int angle = 0;

      switch (sens) {
        case R_GAUCHE   : angle = 5; break;
        case R_GAUCHE45 : angle = 45; break;
        case R_DROITE   : angle = -5; break;
        case R_DROITE45 : angle = 45; break;
      }
      d->sD[trouve].a += angle;
      if (d->sD[trouve].a < -360)
        d->sD[trouve].a += 360;
      if (d->sD[trouve].a > 360)
        d->sD[trouve].a -= 360;

      sauveDonnees(d);
      d->idCourant = -1;
      rendu(d, nPage);
      d->idCourant = sauveIdC;
      redessine_page_courante(d);
    }
  }
}

static void deplacePiece (GtkEntry* self, gpointer user_data) {
  DonneesDep *d = (DonneesDep*)user_data;
  int sens = d->id_action;
  _Bool OK;
  int trouve;
  int nPage = gtk_notebook_get_current_page(GTK_NOTEBOOK(d->doc));

  if (d->idCourant > -1) {
    //printf("Deplacement : %6.2lf %6.2lf\n", d->coords.x - x, d->coords.y -y);
    OK = FALSE;
    for (int i = 0; i < d->nbD; i++) {
      Depliage dpl = d->sD[i];
      if ((dpl.orig < 0) && (dpl.piece == d->idCourant)) {
        //printf(" %d : orig %d face %d (%d %d) x:%6.2lf y:%6.5lf\n", i, d.orig, d.face, d.page, d.piece, d.d.x, d.d.y);
        OK = TRUE;
        trouve = i;
        break;
      }
    }
    if (OK) {
      Vector2d c = {0.0,0.0};

      switch (sens) {
        case D_DROITE : c.x = -5; break;
        case D_DROITE20 : c.x = -20; break;
        case D_GAUCHE : c.x = 5; break;
        case D_GAUCHE20 : c.x = 20; break;
        case D_BAS    : c.y = -5; break;
        case D_BAS20  : c.y = -20; break;
        case D_HAUT   : c.y = 5; break;
        case D_HAUT20 : c.y = 20; break;
      }
      //printf("DEPL : %d : %f %f\n", sens, c.x, c.y);
      d->sD[trouve].d = Vector2dSub(d->sD[trouve].d, c);
      d->coords = Vector2dSub(d->coords, c);
      sauveDonnees(d);
      int sauveIdC = d->idCourant;
      d->idCourant = -1;
      rendu(d, nPage);
      d->idCourant = sauveIdC;
      redessine_page_courante(d);
    }
  }
}

static void exporteDepliage (GtkDialog *dialog, int response, gpointer data) {
  if (response == GTK_RESPONSE_ACCEPT) {
    DonneesDep *d = (DonneesDep *)data;
    GtkFileChooser *chooser = GTK_FILE_CHOOSER (dialog);
    g_autoptr(GFile) file = gtk_file_chooser_get_file (chooser);
    sauveDepliage(d, file);
  }

  gtk_window_destroy (GTK_WINDOW (dialog));
}

static void exportePDF (GtkDialog *dialog, int response, gpointer data) {
  if (response == GTK_RESPONSE_ACCEPT) {
    DonneesDep *d = (DonneesDep *)data;
    GtkFileChooser *chooser = GTK_FILE_CHOOSER (dialog);
    g_autoptr(GFile) file = gtk_file_chooser_get_file (chooser);
    sauvePDF(d, file);
  }

  gtk_window_destroy (GTK_WINDOW (dialog));
}

static void sauveDepliage(DonneesDep *d, GFile * file) {
  strcpy(d->fichierDAT, g_file_get_path(file));
  sauveDonnees(d);
  sauveLanguettes(d->sLgt, d->nbLgt, M_LANG_SAUV, d->fichierDAT);

  guint id = gtk_statusbar_get_context_id(GTK_STATUSBAR(d->statut), "sauveDonnees");
  gtk_statusbar_push(GTK_STATUSBAR(d->statut), id, d->fichierDAT);
}

static void sauvePDF(DonneesDep *d, GFile * file) {
	cairo_surface_t * surface;
	cairo_t * cr;

  Vector2d pageDim = formats[d->formatPage];

  guint id = gtk_statusbar_get_context_id(GTK_STATUSBAR(d->statut), "exportePDF");
  gtk_statusbar_push(GTK_STATUSBAR(d->statut), id, g_file_get_path(file));

	surface = cairo_pdf_surface_create(g_file_get_path(file), pageDim.x, pageDim.y);
	cr = cairo_create(surface);

	cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
	cairo_set_font_size(cr, d->tailleNums);
	cairo_set_line_width(cr, 1);

  int typeL;
  int nPage = 0;
  d->idRecherche = -1;

  // compte nb Pieces
  int nbPieces = 0;
  for (int i = 0; i < d->nbL; i++)
    nbPieces = max(nbPieces, d->lignes[i].nPP);

  int maxI[nbPieces+2];
  for (int i = 0; i <= nbPieces+1; i++) maxI[i] = 0;

  int numsX[d->nbL];
  for (int i = 0; i < d->nbL; i++) numsX[i] = -1;
  //int nX = 0;
  int nAA;
  _Bool memePiece;
  for (int i = 0; i < d->nbL; i++) {
    Ligne l = d->lignes[i];
    if (l.id > -1) {
      if (l.nP > nPage){
        cairo_show_page(cr);
        nPage = l.nP;
      }
      double cop = d->tCop[(l.n1 * 3) + l.i1].cop;
      _Bool cop0 = fabs(cop) < 10e-7;
      if (l.nb == 1) {
        memePiece = FALSE;
        for (int li = 0; li < d->nbL; li++) {
          Ligne l2 = d->lignes[li];
          if ((l.n1 == l2.n2) && (l.n2 == l2.n1)) {
            if (l.nPP == l2.nPP) {
              memePiece = TRUE;
              break;
            }
          }
        }
        nAA = numsX[l.nA];
        if (nAA == -1) {
            nAA = maxI[memePiece ? l.nPP+1 : 0]++;
            numsX[l.nA] = nAA;
        }
        afficheNum(d, cr, nAA, l.p1, l.p2, memePiece ? C_BLEU : C_NOIR, 0);
        if (d->nbLgt == 0)
          typeL = L_COUPE;
        else {
          Lang * rL = NULL;
          for (int j = 0; !rL && (j < d->nbLgt); j++)
            if ((d->sLgt[j].n1 == l.n1) && (d->sLgt[j].n2 == l.n2))
              rL = &d->sLgt[j];
          if (!rL)
            typeL = L_COUPE;
          else
            typeL = rL->v == 0 ? L_COUPE : cop0 ? L_LGT_C : (cop < 0) ? L_LGT_M : L_LGT_V;
        }
      } else
        typeL = cop0 ? L_PLI_C : (cop < 0) ? L_PLI_M : L_PLI_V;
      faitLigne(d, cr, l.p1, l.p2, typeL);
      if ((typeL == L_COUPE) || (typeL == L_LGT_M) || (typeL == L_LGT_V)) {
        if (l.nb != 1) {
          nAA = numsX[l.nA];
          memePiece = FALSE;
          for (int li = 0; li < d->nbL; li++) {
            Ligne l2 = d->lignes[li];
            if ((l.n1 == l2.n2) && (l.n2 == l2.n1)) {
              if (l.nPP == l2.nPP) {
                memePiece = TRUE;
                break;
              }
            }
          }
          if (nAA == -1) {
              nAA = maxI[memePiece ? l.nPP+1 : 0]++;
              numsX[l.nA] = nAA;
          }
          afficheNum(d, cr, nAA, l.p1, l.p2, memePiece ? C_BLEU : C_NOIR, 0);
        }
      }
    }
  }
  cairo_surface_destroy(surface);
  cairo_destroy(cr);
}

static void dessinePage (GtkDrawingArea *area, cairo_t *cr, int width, int height, gpointer data) {
  DonneesDep *d = (DonneesDep*)data;
  gdk_cairo_set_source_rgba(cr,&c_BLANC);
  cairo_paint (cr);

  static const double dashCadre[] = {10.0, 10.0, 10.0};
  gdk_cairo_set_source_rgba(cr, &c_NOIR);
  cairo_set_line_width(cr, 3);
  cairo_set_dash(cr, dashCadre, 1, 0);
  cairo_rectangle(cr, 10, 10, (formats[d->formatPage].x -20) * d->fz, (formats[d->formatPage].y -20) * d->fz);
  cairo_stroke(cr);

  cairo_set_font_size(cr, d->tailleNums * d->fz);

  int typeL;

  if (d->tris) {
    g_free(d->tris);
    d->tris = NULL;
  }
  d->nbTris = 0;
  if (d->rects) {
    g_free(d->rects);
    d->rects = NULL;
  }
  d->nbRects = 0;

  //unsigned nPage = (unsigned)(uintptr_t) data;
  int nPage = d->pCourante;

  int *lidx = NULL;
  int nbLidx = 0;

  for (int i = 0; i < d->nbL; i++) {
    Ligne l = d->lignes[i];
    if ((l.id > -1) && (l.nP == nPage)) {
      lidx = g_realloc(lidx, sizeof(int) * (nbLidx+1));
      lidx[nbLidx++] = i;

      double cop = d->tCop[(l.n1 * 3) + l.i1].cop;
      _Bool cop0 = fabs(cop) < 10e-7;
      if (l.nb == 1) {
        afficheNum(d, cr, l.nA, Vector2dMul(l.p1, d->fz), Vector2dMul(l.p2, d->fz), C_NOIR, 0);
        if (d->nbLgt == 0)
          typeL = L_COUPE;
        else {
          Lang * rL = NULL;
          for (int j = 0; !rL && (j < d->nbLgt); j++)
            if ((d->sLgt[j].n1 == l.n1) && (d->sLgt[j].n2 == l.n2))
              rL = &d->sLgt[j];
          if (!rL)
            typeL = L_COUPE;
          else
            typeL = rL->v == 0 ? L_COUPE : cop0 ? L_LGT_C : (cop < 0) ? L_LGT_M : L_LGT_V;
        }
      } else
        typeL = cop0 ? L_PLI_C : (cop < 0) ? L_PLI_M : L_PLI_V;
      // affichage ligne
      if (d->idCourant > -1){
        gdk_cairo_set_source_rgba(cr, &c_ROUGE);
        Vector2d cZ = d->coords;
        cairo_move_to(cr, cZ.x, cZ.y);
        cairo_arc(cr, cZ.x, cZ.y, 3 * d->fz, 0, 2 * pi);
        cairo_stroke(cr);
      }
      cairo_set_line_width(cr, (d->idCourant == l.nPP ? 2 : 1) * d->fz);
      Vector2d l1z = Vector2dMul(l.p1, d->fz);
      Vector2d l2z = Vector2dMul(l.p2, d->fz);
      faitLigne(d, cr, l1z, l2z, typeL);

      if (d->mode != MODE_PIECE) {
        int ln1 = -1, ln2 = -1;
        int p1 = -1, p2 = -1;
        for (int di = 0; di < d->nbD; di++) {
          int fn = d->sD[di].face;
          if (fn == l.n1) {
            ln1 = di;
            p1 = d->sD[ln1].piece;
          } else if (fn == l.n2) {
            ln2 = di;
            p2 = d->sD[ln2].piece;
          }
          if ((ln1 > -1) && (ln2 > -1))
            break;
        }

        int type = 0;
        if (d->mode == MODE_ARETE) {
          if (typeL >=  L_COUPE) {
            if (p1 != p2)
              type = 1;
          } else if (typeL < L_COUPE) {
            type = 2;
          }
        } else { // MODE_LANG
            //type = ((typeL == L_COUPE) || (typeL >= L_LGT_M)) ? 1 : 0;
            type = typeL >= L_COUPE ? 1 : 0;
        }
        if ((type == 1 && d->mode ==  MODE_LANG) || (type > 0 && d->mode == MODE_ARETE)) {
          Vector2d m = milieu(l1z, l2z);
          rectT r;
          r.id = i;
          r.type = type;
          r.p1.x = m.x - 3;
          r.p1.y = m.y - 3;
          r.p2.x = m.x + 3;
          r.p2.y = m.y + 3;
          d->rects = g_realloc(d->rects, (size_t)(d->nbRects+1) * sizeof(rectT));
          d->rects[d->nbRects] = r;
          d->nbRects++;
          afficheNum(d, cr, l.nA, l1z, l2z, C_NOIR, r.type);
        }
      }
    }
  }

  Ligne LI, LJ, LK;
  for (int i = 0;  i < nbLidx; i++) {
    LI = d->lignes[lidx[i]];
    for (int j = 0; j < nbLidx; j++) {
      if (i != j) {
        LJ = d->lignes[lidx[j]];
        for (int k = 0; k < nbLidx; k++) {
          if ((k != i) && (k != j)) {
           LK = d->lignes[lidx[k]];
           if ((LI.nPP == LJ.nPP) && (LJ.nPP == LK.nPP))
           {
            if (Vector2dEq(LI.p1, LJ.p1) && Vector2dEq(LI.p2, LK.p1) && Vector2dEq(LJ.p2, LK.p2)) {
              ajouteTriangle(d, LI.nPP, LI.p1, LI.p2, LJ.p2);
              break;
            }
            else
            if (Vector2dEq(LI.p1, LJ.p2) && Vector2dEq(LI.p2, LK.p1) && Vector2dEq(LJ.p1, LK.p2)) {
              ajouteTriangle(d, LI.nPP, LI.p1, LI.p2, LJ.p1);
              break;
            }
            else
            if (Vector2dEq(LI.p1, LJ.p1) && Vector2dEq(LI.p2, LK.p2) && Vector2dEq(LJ.p2, LK.p1)) {
              ajouteTriangle(d, LI.nPP, LI.p1, LI.p2, LJ.p2);
              break;
            }
            else
            if (Vector2dEq(LI.p1, LJ.p2) && Vector2dEq(LI.p2, LK.p2) && Vector2dEq(LJ.p1, LK.p1)) {
              ajouteTriangle(d, LI.nPP, LI.p1, LI.p2, LJ.p1);
              break;
            }
            else
            if (Vector2dEq(LI.p1, LK.p1) && Vector2dEq(LI.p2, LJ.p1) && Vector2dEq(LJ.p2, LK.p2)) {
              ajouteTriangle(d, LI.nPP, LI.p1, LI.p2, LJ.p2);
              break;
            }
            else
            if (Vector2dEq(LI.p1, LK.p2) && Vector2dEq(LI.p2, LJ.p2) && Vector2dEq(LJ.p1, LK.p1)) {
              ajouteTriangle(d, LI.nPP, LI.p1, LI.p2, LJ.p1);
              break;
            }
            else
            if (Vector2dEq(LI.p1, LK.p1) && Vector2dEq(LI.p2, LJ.p2) && Vector2dEq(LJ.p1, LK.p2)) {
              ajouteTriangle(d, LI.nPP, LI.p1, LI.p2, LJ.p1);
              break;
            }
            else
            if (Vector2dEq(LI.p1, LK.p2) && Vector2dEq(LI.p2, LJ.p1) && Vector2dEq(LJ.p2, LK.p1)) {
              ajouteTriangle(d, LI.nPP, LI.p1, LI.p2, LJ.p2);
              break;
            }
           }
          }
        }
      }
    }
  }

  g_free(lidx);
}
void ajouteTriangle(DonneesDep *d, int id, Vector2d p1, Vector2d p2, Vector2d p3) {
  p1 = Vector2dMul(p1, d->fz);
  p2 = Vector2dMul(p2, d->fz);
  p3 = Vector2dMul(p3, d->fz);

  Vector2d P[3];
  P[0] = Vector2dPlusPetit(Vector2dPlusPetit(p1, p2), p3);
  P[2] = Vector2dPlusGrand(Vector2dPlusGrand(p1, p2), p3);

  if (!Vector2dEq(P[0], p1) && !Vector2dEq(P[2], p1))
    P[1] = Vector2dNew(p1.x, p1.y);
  else if (!Vector2dEq(P[0], p2) && !Vector2dEq(P[2], p2))
    P[1] = Vector2dNew(p2.x, p2.y);
  else
    P[1] = Vector2dNew(p3.x, p3.y);

  // rech si déjà présent
  _Bool ok = FALSE;
  for (int i = 0; (i < d->nbTris) && !ok; i++)
  {
    ok = Vector2dEq(P[0], d->tris[i].p1)
      && Vector2dEq(P[1], d->tris[i].p2)
      && Vector2dEq(P[2], d->tris[i].p3);
  }

  if (!ok) {
    d->tris = g_realloc(d->tris, sizeof(triT) * (d->nbTris +1));
    triT t = {id, P[0], P[1], P[2]};
    d->tris[d->nbTris++] = t;
  }
}

static void on_changePage (GtkDialog *dialog,  int response, gpointer user_data) {
  DonneesDep *d = (DonneesDep *)user_data;
  int nPage = gtk_notebook_get_current_page(GTK_NOTEBOOK(d->doc));

  Depliage *sauvePiece = NULL;
  int nsP;

  Depliage *sauveBloc = NULL;
  int nsB;
  int idxDeb = -1, idxFin = -1;
  int sauveOrig = -1;

  if (response == GTK_RESPONSE_YES)
  {
    //printf("deplacement piece %d page %d vers %d\n", d->idCourant, d->pCourante, nPage);

    if (nPage < d->pCourante) { // déplacement vers une page précédente
      // sauver piece
      nsP = 0;
      for (int i = 0; i < d->nbD; i++)
      {
        Depliage D = d->sD[i];

        if ((sauveOrig == -1) && (D.page == nPage) && (D.orig < 0))
          sauveOrig = i;

        if ((idxDeb == -1) && (D.page > nPage))
          idxDeb = i;

        if (D.piece == d->idCourant) {
          if (idxFin == -1)
            idxFin = i;
          sauvePiece = (Depliage *)g_realloc(sauvePiece, sizeof(Depliage) * (nsP+1));
          Depliage s;
          if (D.orig < 0) {
            s.a = D.a;
            s.d = Vector2dNew(D.d.x, D.d.y);
          }
          s.face = D.face;
          s.orig = D.orig;
          s.page = D.page;
          s.piece = D.piece;

          sauvePiece [nsP] = s;
          nsP++;
        }
      }

      //printf("sauveOrig : %d\n", sauveOrig);
      //printf("idxDeb %d idFin : %d\n", idxDeb, idxFin);

      // sauver bloc entre la nouvelle page et la page de la pièce
      nsB = 0;
      for (int i = idxDeb; i < idxFin; i++) {
        Depliage D = d->sD[i];
        sauveBloc = (Depliage *)g_realloc(sauveBloc, sizeof(Depliage) * (nsB+1));
        Depliage s;
        if (D.orig < 0) {
          s.a = D.a;
          s.d = Vector2dNew(D.d.x, D.d.y);
        }
        s.face = D.face;
        s.orig = D.orig;
        s.page = D.page;
        s.piece = D.piece;

        sauveBloc [nsB] = s;
        nsB++;
      }

      // déplacer piece
      for (int i = 0; i < nsP; i++) {
        Depliage D = sauvePiece[i];
        int j = i + idxDeb;

        if (D.orig < 0) {
          d->sD[j].a = D.a;
          d->sD[j].d = Vector2dNew(D.d.x, D.d.y);
          d->sD[j].orig = d->sD[sauveOrig].orig;
        }
        else
          d->sD[j].orig = D.orig;
        d->sD[j].face = D.face;
        d->sD[j].page = d->sD[sauveOrig].page;
        d->sD[j].piece = D.piece;
      }

      // déplacer bloc
      for (int i = 0; i < nsB; i++) {
        Depliage D = sauveBloc[i];
        int j = idxDeb + i + nsP;
        if (D.orig < 0) {
          d->sD[j].a = D.a;
          d->sD[j].d = Vector2dNew(D.d.x, D.d.y);
        }
        d->sD[j].orig = D.orig;
        d->sD[j].face = D.face;
        //d->sD[j].page = d->sD[sauveOrig].page;
        d->sD[j].page = D.page;
        d->sD[j].piece = D.piece;
      }
    } else {
      // Déplacement vers une page suivante
      nsP = 0;
      // sauver pièce
      for (int i = 0; i < d->nbD; i++) {
        Depliage D = d->sD[i];

        if ((idxFin == -1) && (D.page == nPage)) {
          idxFin = i;
          sauveOrig = i;
        }

        if (D.piece == d->idCourant) {
          if (idxDeb == -1)
            idxDeb = i;

          Depliage s;
          if (D.orig < 0) {
            s.a = D.a;
            s.d = Vector2dNew(D.d.x, D.d.y);
          }
          s.face = D.face;
          s.orig = D.orig;
          s.page = D.page;
          s.piece = D.piece;

          sauvePiece = (Depliage *) g_realloc(sauvePiece, sizeof(Depliage) * (nsP+1));
          sauvePiece [nsP] = s;
          nsP++;
        }
      }
      // déplacer bloc
      idxDeb += nsP;
      for (int i = idxDeb; i < idxFin; i++) {
        int j = i - nsP;
        Depliage D = d->sD[i];
        if (D.orig < 0) {
          d->sD[j].a = D.a;
          d->sD[j].d = Vector2dNew(D.d.x, D.d.y);
        }
        d->sD[j].orig = D.orig;
        d->sD[j].face = D.face;
        d->sD[j].page = D.page;
        d->sD[j].piece = D.piece;
      }
      // déplacer pièce
      for (int i = 0; i < nsP; i++) {
        Depliage D = sauvePiece[i];
        int j = idxFin - nsP + i;

        if (D.orig < 0) {
          d->sD[j].a = D.a;
          d->sD[j].d = Vector2dNew(D.d.x, D.d.y);
          d->sD[j].orig = d->sD[sauveOrig].orig;
        }
        else
          d->sD[j].orig = D.orig;
        d->sD[j].face = D.face;
        d->sD[j].page = d->sD[sauveOrig].page;
        d->sD[j].piece = D.piece;
      }
    }

    sauveDonnees(d);
    d->idCourant = -1;
    rendu(d, nPage);
    d->pCourante = nPage;
    redessine_page_courante(d);
  }

  d->idCourant = -1;
  d->pCourante = nPage;

  g_free(sauvePiece);
  g_free(sauveBloc);

  gtk_window_destroy(GTK_WINDOW(dialog));
}

void changePage (GtkNotebook* self, GtkWidget* page, guint page_num, gpointer user_data) {
  GtkWidget *dialog;
  DonneesDep *d = (DonneesDep *)user_data;

  GtkWindow *win = GTK_WINDOW(d->win);

  if ((d->idCourant > -1) && (d->pCourante != page_num)) {
    dialog = gtk_message_dialog_new(
      win, GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_MODAL,
      GTK_MESSAGE_INFO,
      GTK_BUTTONS_YES_NO,
      "Deplacer la piece ?"
    );

    g_signal_connect(dialog, "response", G_CALLBACK(on_changePage), d);
    gtk_widget_show(dialog);
  }
  else
    d->pCourante = page_num;
}

void rendu(DonneesDep *d, int pc) {
  chargeDonnees(d);
  // PREMIERE PAGE DE RENDU
  GtkNotebook * n = GTK_NOTEBOOK(d->doc);
  GtkGesture *Gpress;
  GtkGesture *Grotate;

  // Suppression des pages existantes
  while (gtk_notebook_get_n_pages(n) > 0)
    gtk_notebook_remove_page(n, -1);
  if (d->dpage) g_free(d->dpage);
  if (d->dlabel) g_free(d->dlabel);

  int nbP = d->lignes[d->nbL-1].nP +1;

  d->dpage = g_new(GtkWidget*, nbP);
  d->dlabel= g_new(GtkWidget*, nbP);

  for (unsigned i = 0; i < nbP; i++) {
    GtkWidget *frame = gtk_scrolled_window_new();
    d->dpage[i] = gtk_drawing_area_new();
    GtkDrawingArea * p = GTK_DRAWING_AREA(d->dpage[i]);
    gtk_widget_set_cursor(d->dpage[i], gdk_cursor_new_from_name("crosshair", NULL));

    Gpress = gtk_gesture_click_new();
    gtk_gesture_single_set_button (GTK_GESTURE_SINGLE (Gpress), GDK_BUTTON_PRIMARY);
    gtk_widget_add_controller (d->dpage[i], GTK_EVENT_CONTROLLER (Gpress));
    g_signal_connect (Gpress, "pressed", G_CALLBACK (pressed), d);

    Grotate = gtk_gesture_click_new();
    gtk_gesture_single_set_button (GTK_GESTURE_SINGLE (Grotate), GDK_BUTTON_SECONDARY);
    gtk_widget_add_controller (d->dpage[i], GTK_EVENT_CONTROLLER (Grotate));
    g_signal_connect (Grotate, "pressed", G_CALLBACK (rotated), d);

    int W = 595 * d->fz;
    int H = 842 * d->fz;
    gtk_drawing_area_set_content_width(p, W);
    gtk_drawing_area_set_content_height(p, H);
    char tampon[10];
    snprintf(tampon, 10, "page %d", i+1);
    d->dlabel[i] = gtk_label_new(tampon);

    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(frame), d->dpage[i]);
    gtk_widget_set_vexpand(frame, TRUE);
    gtk_widget_set_hexpand(frame, TRUE);
    gtk_notebook_append_page(n, frame, d->dlabel[i]);

    gtk_drawing_area_set_draw_func (p, dessinePage, d, NULL);
    d->idCourant = -1;
    //redessine_page_courante();
  }

  if (pc > -1)
    gtk_notebook_set_current_page(n, pc);

}
void deplier(DonneesDep *d) { // DEPLIAGE
  if (d->echelle == 0) { // si 0 : valeurs par défaut
    d->echelle = 1;
    d->formatPage = 4; // A4
    //d->formatPage = 6; // Cr30
    d->hauteurLang = 15;
    d->tailleNums = 15.0;
    d->typeLang = 1;
    d->fz = 1;
  }

  chargeOBJ(d);

  //char * tampon = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(cbFormat));
  //d->formatPage = formatPageID(tampon);
  //d->formatPage = 4;
  Vector2d limitePage;
  if (d->fz != 1) {
    limitePage = Vector2dSub(Vector2dMul(formats[d->formatPage], d->fz), Vector2dMul(marge, d->fz));
  } else {
    limitePage = Vector2dSub(formats[d->formatPage], marge);
  }

  //char * tampon = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(cbLang));
  //d->typeLang = tampon[0] == '1' ? 1 : tampon[0] == '2' ? 2 : 0;
  //d->typeLang = 1;
  //d->typeLang = 0;

	//d->tailleNums = atof(
  //  gtk_entry_buffer_get_text (gtk_entry_get_buffer(GTK_ENTRY(enTailleN))));
  //d->tailleNums = 15;

  //d->hauteurLang = atoi(
  //  gtk_entry_buffer_get_text (gtk_entry_get_buffer(GTK_ENTRY(enLangH))));
  //d->hauteurLang = 15;

  d->premierTriangle = 0;

/*  printf("fichier     : %s\n", d->fichierOBJ);
  printf("echelle     : %5.2f\n", d->echelle);
  printf("format page : %d\n",  d->formatPage);
  printf("-->limites  : %5.0lf %5.0lf\n", limitePage.x, limitePage.y);
  printf("taille nums : %5.2f\n", d->tailleNums);
  printf("type lang   : %d\n", d->typeLang);
  printf("haut. lang. : %d\n", d->hauteurLang);
  printf("nb sommets  : %d\n", d->nbSommets);
  printf("nb faces    : %d\n", d->nbFaces);
*/

  // DEBUT DU DEPLIAGE
	d->dispo = g_new(_Bool, d->nbFaces);
	for (int i = 0; i < d->nbFaces; i++)
		d->dispo[i] = 1;

	d->lSNA = g_new(NAff, d->nbFaces * 3);
	d->nAff = 0;

	d->page = g_new(int, d->nbFaces);
	d->nbP = 0;
	d->nbPP = 0;
	d->sD = g_new(Depliage, d->nbFaces); // depliage
	d->nbD = 0; // nb de faces dépliées
	d->lignes = g_new(Ligne, d->nbFaces * 3);
	d->nbL = 0;

	int gc;
	int tc = d->premierTriangle;
	do {
		gc = d->faces[tc][3];
		int nbTp = 0;
		int tcn = 0;
		d->page[nbTp++] = tc;
		Vector2d delta = {0.0, 0.0};
		Depliage sdC = { d->nbP, d->nbPP,tc, -1*(d->nbP+1),  0, delta};
		d->sD[d->nbD++] = sdC;
		d->dispo[tc] = 0;
		_Bool ok;
		do {
			for (int vi = 0; vi < 3; vi++) {
				Voisin v = d->voisins[tc][vi];
				int vc = v.nF;
				ok = d->dispo[vc] && (d->faces[vc][3] == gc);
				if (ok)
					for (int i = 0; (i < nbTp) && ok; i++)
						if (d->page[i] == vc)
              ok = FALSE;
				if (ok) {
					Vector2d sauveV2dVoisin[3];	// Sauve v2d du voisin
					for (int i = 0; i < 3; i++)
						sauveV2dVoisin[i] = d->v2d[vc][i];
					// Rapproche v2d[vc] de v2d[tc]
					Vector2d deltaV = Vector2dSub(d->v2d[tc][vi], d->v2d[vc][v.idx]);
					for (int i = 0; i < 3; i++)
						d->v2d[vc][i] = Vector2dAdd(d->v2d[vc][i], deltaV);
					// Tourne
					double a = calcAngle(d->v2d[tc][vi],
						d->v2d[tc][suiv(vi)], d->v2d[vc][prec(v.idx)]);
					for (int i = 0; i < 3; i++)
						d->v2d[vc][i] = Vector2dRotation(d->v2d[tc][vi], d->v2d[vc][i], a);
					// Teste dépassement page
					int nbTV = (nbTp + 1) * 3;
					Vector2d(*tmp) = g_new(Vector2d, nbTV);
					for (int i = 0; i < nbTp; i++)
						for (int j = 0; j < 3; j++)
							tmp[i * 3 + j] = d->v2d[d->page[i]][j];
					for (int j = 0; j < 3; j++)
						tmp[nbTp * 3 + j] = d->v2d[vc][j];
					Vector2d vb[2];
					calcBoiteEnglobante(vb, tmp, nbTV);
					g_free(tmp);
					Vector2d dV = Vector2dSub(vb[1], vb[0]);
					if ((dV.x > limitePage.x) || (dV.y > limitePage.y))
						ok = FALSE;
					if (ok) // Teste collision avec la pièce
						for (int i = 0; (i < nbTp) && ok; i++)
							if (overlap(d->v2d[d->page[i]], d->v2d[vc]))
								ok = FALSE;
					if (ok) {
						d->page[nbTp++] = vc;
						Depliage sdC = { d->nbP, d->nbP, vc, tc, 0 };
						d->sD[d->nbD++] = sdC;
						d->dispo[vc] = 0;
					} else
            for (int i = 0; i < 3; i++)
							d->v2d[vc][i] = sauveV2dVoisin[i];
				}
			}
			// Recherche prochaine face à déplier
			tcn++;
			if (tcn < nbTp) {
				tc = d->page[tcn];
				ok = TRUE;
			} else
				ok = FALSE;
		} while (ok);

		// ajustement en bas à gauche
		Vector2d(*tmp) = g_new(Vector2d, nbTp * 3);
		for (int i = 0, k = 0; i < nbTp; i++)
			for (int j = 0; j < 3; j++)
				tmp[k++] = d->v2d[d->page[i]][j];
		Vector2d b[2];
		calcBoiteEnglobante(b, tmp, nbTp * 3);
		g_free(tmp);
		for (int i = 0; i < nbTp; i++)
			for (int j = 0; j < 3; j++)
				d->v2d[d->page[i]][j] =
					Vector2dSub(d->v2d[d->page[i]][j], Vector2dSub(b[0], marge));

		// répartition en lignes
		int nA;
		for (int i = 0; i < nbTp; i++)
			for (int j = 0; j < 3; j++) {
        NAff cleN;
				NAff* rechN;
				cleN.nMax = max(d->page[i], d->voisins[d->page[i]][j].nF);
				cleN.nMin = min(d->page[i], d->voisins[d->page[i]][j].nF);
				rechN = (NAff*)bsearch(&cleN, d->lSNA, d->nAff, sizeof(NAff), compAff);
				if (rechN != NULL)
					nA = rechN->a;
				else {
					cleN.a = d->nAff;
					nA = d->nAff;
					d->lSNA[d->nAff++] = cleN;
					//qsort(d->lSNA, d->nAff, sizeof(NAff), compAff);
				}

				d->lignes[d->nbL] = LigneNew(
					d->nbP,
					d->nbP,
					d->nbL,
					d->v2d[d->page[i]][j],
					d->v2d[d->page[i]][suiv(j)],
					d->page[i],
					j,
					d->voisins[d->page[i]][j].nF,
					d->voisins[d->page[i]][j].idx,
					nA);
        d->nbL++;
			}

		supprimeDoublons(d->lignes, d->nbL);
		tc = premDispo(d->dispo, d->nbFaces);
		d->nbP++;
	} while (tc > -1);

  qsort(d->lSNA, d->nAff, sizeof(NAff), compAff);
	qsort(d->lignes, d->nbL, sizeof(Ligne), compPg);

	d->lAN = g_new(AN, d->nbFaces);
	d->nbAN = 0;

	sauveDonnees(d);

	// init. languettes
  d->sLgt = NULL;
  d->nbLgt = 0;

	for (int i =0; i < d->nbL; i++) {
    Ligne l = d->lignes[i];
    if (l.nb == 1) {
      Lang l0 = { .n1 = l.n1, .n2 = l.n2, .v = l.n1 < l.n2 ? 1 : 0};
      d->sLgt = g_realloc(d->sLgt, sizeof(Lang) * ((size_t)d->nbLgt + 1));
      d->sLgt[d->nbLgt] = l0;
			d->nbLgt++;
    }
  }

  for (int i = 0; i < d->nbLgt; i++) {
    Lang L = d->sLgt[i];
    _Bool OK = TRUE;
    for(int j = 0; (j < d->nAff) && OK; j++) {
      NAff A = d->lSNA[j];
      if ((min(L.n1, L.n2) == A.nMin) && (max(L.n1, L.n2) == A.nMax))
      {
        d->sLgt[i].o = A.a;
        OK = FALSE;
      }
    }
  }

 	qsort(d->sLgt, d->nbLgt, sizeof(Lang), compLang);
  sauveLanguettes(d->sLgt, d->nbLgt, d->typeLang, d->fichierDAT);
}
float sign (Vector2d p1, Vector2d p2, Vector2d p3) {
  return (p1.x - p3.x) * (p2.y - p3.y) - (p2.x - p3.x) * (p1.y - p3.y);
}
_Bool dansTriangle (Vector2d v1, Vector2d v2, Vector2d v3, Vector2d pt) {
  float d1, d2, d3;
  _Bool has_neg, has_pos;

  d1 = sign(pt, v1, v2);
  d2 = sign(pt, v2, v3);
  d3 = sign(pt, v3, v1);

  has_neg = (d1 < 0) || (d2 < 0) || (d3 < 0);
  has_pos = (d1 > 0) || (d2 > 0) || (d3 > 0);

  return !(has_neg && has_pos);
}
//---------- UI ----------
static void redessine_page_courante(DonneesDep *d) {
  gtk_widget_queue_draw(d->dpage[gtk_notebook_get_current_page(GTK_NOTEBOOK(d->doc))]);
}
static void on_open_response (GtkDialog *dialog, int response, gpointer data) { // SELECTION DU FICHIER .OBJ
  DonneesDep *d = (DonneesDep *)data;

  if (response == GTK_RESPONSE_ACCEPT) {
    GtkFileChooser *chooser = GTK_FILE_CHOOSER (dialog);
    g_autoptr(GFile) file = gtk_file_chooser_get_file (chooser);
    char *ch = g_new(char, 100);
    snprintf(ch, 100, "[%s]", g_file_get_basename(file));
    gtk_window_set_title(GTK_WINDOW(d->win), ch);
    g_free(ch);
    snprintf(d->fichierOBJ, 128, "%s", g_file_get_path(file));
    for (int i =0; i < strlen(d->fichierOBJ); i++) {
      if (d->fichierOBJ[i] < ' ')
        d->fichierOBJ[i] = 0;
    }

    deplier(d);
    rendu(d, -1);

    //gtk_widget_set_sensitive(btnVoirOBJ, TRUE);
    //gtk_widget_set_sensitive(btnDeplier, TRUE);
  }

  gtk_window_destroy (GTK_WINDOW (dialog));
}
static void quit_activated(GSimpleAction *action, GVariant *parameter, gpointer user_data) {
  DonneesDep *d = (DonneesDep *)user_data;
  GApplication *app = d->app;

  g_application_quit (app);
}
static void new_activated(GSimpleAction *action, GVariant *parameter, gpointer data) {
// CREATION DE L'INTERFACE PERMETTANT DE CHOISIR LE FICHIER OBJ
  DonneesDep *d = (DonneesDep *)data;
  GtkWidget *dialog;

  dialog = gtk_file_chooser_dialog_new ("Ouvre fichier .OBJ",
    GTK_WINDOW(d->win),
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

static void ouvreDepliage (GtkDialog *dialog, int response, gpointer data) {
  DonneesDep *d = (DonneesDep *)data;

  if (response == GTK_RESPONSE_ACCEPT)
    {
      GtkFileChooser *chooser = GTK_FILE_CHOOSER (dialog);
      g_autoptr(GFile) file = gtk_file_chooser_get_file (chooser);

      strcpy(d->fichierDAT, g_file_get_path(file));
      d->fz = 1;
      rendu(d, -1);
    }

  gtk_window_destroy (GTK_WINDOW (dialog));
}

static void open_activated(GSimpleAction *action, GVariant *parameter, gpointer user_data) {
  DonneesDep *d = (DonneesDep *)user_data;
  GtkWidget *dialog;
  GtkFileChooserAction actionFC = GTK_FILE_CHOOSER_ACTION_OPEN;

  dialog = gtk_file_chooser_dialog_new ("Ouvrir depliage",
                                        GTK_WINDOW(d->win),
                                        actionFC,
                                        "_Cancel", GTK_RESPONSE_CANCEL,
                                        "_Open", GTK_RESPONSE_ACCEPT,
                                        NULL);

  GtkFileFilter *filtre = gtk_file_filter_new ();
  gtk_file_filter_add_suffix (filtre, "dat");
  gtk_file_filter_set_name (filtre, "DAT Donnees Depliage");
  gtk_file_chooser_add_filter (GTK_FILE_CHOOSER(dialog), filtre);

  gtk_widget_show (dialog);

  g_signal_connect(dialog, "response", G_CALLBACK (ouvreDepliage), user_data);
}

static void ropen_activated(GSimpleAction *action, GVariant *parameter, gpointer user_data) {
  //puts("*** RECHARGE ***");
  DonneesDep *d = (DonneesDep *)user_data;

  d->fz = 1;
  rendu(d, -1);
}
//static void lobj_activated(GSimpleAction *action, GVariant *parameter, gpointer user_data) {}

static void on_edpa_response (GtkDialog *dialog, int response, gpointer data) { // EDITION PARAMETRES
  DonneesDep *d = (DonneesDep *)data;

  if (response == GTK_RESPONSE_ACCEPT) {
    float ancienneEchelle = d->echelle;
    int ancienTypeLang = d->typeLang;
    char tmp[20];

    strcpy(tmp, gtk_entry_buffer_get_text(gtk_entry_get_buffer(GTK_ENTRY(d->params[0]))));
    if (strlen(tmp) > 0) {
      d->echelle = strtof(tmp, NULL);
      //printf("ECHELLE : %f\n", d->echelle);
    }

    strcpy(tmp, gtk_entry_buffer_get_text(gtk_entry_get_buffer(GTK_ENTRY(d->params[1]))));
    if (strlen(tmp) > 0) {
      d->tailleNums = strtof(tmp, NULL);
      //printf("TAILLE NUMS : %f\n", d->tailleNums);
    }

    strcpy(tmp, gtk_entry_buffer_get_text(gtk_entry_get_buffer(GTK_ENTRY(d->params[3]))));
    if (strlen(tmp) > 0) {
      d->hauteurLang = atoi(tmp);
      //printf("HAUTEUR LANG : %d\n", d->hauteurLang);
    }

    strcpy(tmp, gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(d->params[2])));
    if (strlen(tmp) > 0) {
      d->typeLang = (int)tmp[0] - 48;
      //printf("LANGUETTES : %d\n", d->typeLang);
    }

    sauveDonnees(d);

    if ((fabs(ancienneEchelle - d->echelle) > epsilon) || (ancienTypeLang != d->typeLang))
      deplier(d);

    rendu(d, -1);
    redessine_page_courante(d);
  }

  gtk_window_destroy (GTK_WINDOW (dialog));
}

static void cala_activated(GSimpleAction *action, GVariant *parameter, gpointer user_data) {

}
static void edpa_activated(GSimpleAction *action, GVariant *parameter, gpointer user_data) {
  DonneesDep *d = (DonneesDep *)user_data;

  GtkWidget *dialog, *content_area, *grille;
  GtkWidget *edits[5];
  GtkWidget *labels[5];
  GtkDialogFlags flags;
  char tmp[10];

  flags = GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_MODAL;
  dialog = gtk_dialog_new_with_buttons("Paramètres",
                                        GTK_WINDOW(d->win), flags,
                                        "_OK", GTK_RESPONSE_ACCEPT,
                                        "_Annuler", GTK_RESPONSE_CANCEL,
                                        NULL);
  g_signal_connect (dialog, "response", G_CALLBACK (on_edpa_response), user_data);

  content_area = gtk_dialog_get_content_area (GTK_DIALOG (dialog));
  grille = gtk_grid_new();
  gtk_widget_set_hexpand (grille, TRUE);
  gtk_widget_set_vexpand (grille, TRUE);
  gtk_widget_set_halign (grille, GTK_ALIGN_CENTER);
  gtk_widget_set_valign (grille, GTK_ALIGN_CENTER);
  gtk_box_append (GTK_BOX (content_area), grille);
  gtk_grid_set_row_spacing (GTK_GRID (grille), 6);
  gtk_grid_set_column_spacing (GTK_GRID (grille), 6);

  labels[0] = gtk_label_new ("Echelle");
  gtk_grid_attach (GTK_GRID (grille), labels[0], 0, 0, 1, 1);
  edits[0] = gtk_entry_new();
  snprintf(tmp, 10, "%6.2f", d->echelle);
  gtk_entry_buffer_set_text(gtk_entry_get_buffer(GTK_ENTRY(edits[0])), tmp, -1);
  gtk_grid_attach (GTK_GRID (grille), edits[0], 3, 0, 1, 1);

  labels[1] = gtk_label_new ("Taille Nums");
  gtk_grid_attach (GTK_GRID (grille), labels[1], 0, 1, 1, 1);
  edits[1] = gtk_entry_new();
  snprintf(tmp, 10, "%6.2f", d->tailleNums);
  gtk_entry_buffer_set_text(gtk_entry_get_buffer(GTK_ENTRY(edits[1])), tmp, -1);
  gtk_grid_attach (GTK_GRID (grille), edits[1], 3, 1, 1, 1);

  labels[2] = gtk_label_new ("Languettes");
  gtk_grid_attach (GTK_GRID (grille), labels[2], 0, 2, 1, 1);

  edits[2] = gtk_combo_box_text_new();
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(edits[2]), NULL, "0 - Aucune");
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(edits[2]), NULL, "1 par paire");
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(edits[2]), NULL, "2 par paire");
  gtk_combo_box_set_active(GTK_COMBO_BOX(edits[2]), d->typeLang);
  gtk_grid_attach (GTK_GRID (grille), edits[2], 3, 2, 1, 1);

  labels[3] = gtk_label_new ("Haut. lang.");
  gtk_grid_attach (GTK_GRID (grille), labels[3], 0, 3, 1, 1);
  edits[3] = gtk_entry_new();
  snprintf(tmp, 10, "%d", d->hauteurLang);
  gtk_entry_buffer_set_text(gtk_entry_get_buffer(GTK_ENTRY(edits[3])), tmp, -1);
  gtk_grid_attach (GTK_GRID (grille), edits[3], 3, 3, 1, 1);

  edits[4] = gtk_check_button_new_with_label ("Nums internes en couleur ?");
  gtk_grid_attach (GTK_GRID (grille), edits[4], 0, 4, 1, 1);

  d->params[0] = edits[0];
  d->params[1] = edits[1];
  d->params[2] = edits[2];
  d->params[3] = edits[3];
  d->params[4] = edits[4];

  gtk_widget_show (dialog);
}
static void save_activated(GSimpleAction *action, GVariant *parameter, gpointer user_data) {
  DonneesDep *d = (DonneesDep *)user_data;

  GtkWidget *dialog;
  GtkFileChooser *chooser;
  GtkFileChooserAction Caction = GTK_FILE_CHOOSER_ACTION_SAVE;

  dialog = gtk_file_chooser_dialog_new ("Sauver Depliage",
                                        GTK_WINDOW(d->win),
                                        Caction,
                                        "_Cancel", GTK_RESPONSE_CANCEL,
                                        "_Save", GTK_RESPONSE_ACCEPT,
                                        NULL);
  chooser = GTK_FILE_CHOOSER (dialog);

  GtkFileFilter *filtre = gtk_file_filter_new ();
  gtk_file_filter_add_suffix (filtre, "dat");
  gtk_file_filter_set_name (filtre, "DAT Donnees Depliage");
  gtk_file_chooser_add_filter (GTK_FILE_CHOOSER(dialog), filtre);

  gtk_file_chooser_set_current_name (chooser, "donnees.dat");

  gtk_widget_show (dialog);
  g_signal_connect (dialog, "response", G_CALLBACK (exporteDepliage), user_data);
}
//static void sava_activated(GSimpleAction *action, GVariant *parameter, gpointer user_data) {}
static void expg_activated(GSimpleAction *action, GVariant *parameter, gpointer user_data) {
  DonneesDep *d = (DonneesDep *)user_data;

  GtkWidget *dialog;
  GtkFileChooser *chooser;
  GtkFileChooserAction Caction = GTK_FILE_CHOOSER_ACTION_SAVE;

  dialog = gtk_file_chooser_dialog_new ("Exporter PDF",
                                        GTK_WINDOW(d->win),
                                        Caction,
                                        "_Cancel", GTK_RESPONSE_CANCEL,
                                        "_Save", GTK_RESPONSE_ACCEPT,
                                        NULL);
  chooser = GTK_FILE_CHOOSER (dialog);

  GtkFileFilter *filtre = gtk_file_filter_new ();
  gtk_file_filter_add_suffix (filtre, "pdf");
  gtk_file_filter_set_name (filtre, "PDF Portable Document Format");
  gtk_file_chooser_add_filter (GTK_FILE_CHOOSER(dialog), filtre);

  gtk_file_chooser_set_current_name (chooser, "export.pdf");

  gtk_widget_show (dialog);
  g_signal_connect (dialog, "response", G_CALLBACK (exportePDF), user_data);
}
static void modeB_toggled(GtkToggleButton *bouton, gpointer data) {
  DonneesDep *d = (DonneesDep *)data;

  if (gtk_toggle_button_get_active(bouton)) {
    const char *tmp = gtk_button_get_label(GTK_BUTTON(bouton));
    d->mode = tmp[1] == 'P' ? MODE_PIECE
            : tmp[1] == 'A' ? MODE_ARETE
            : MODE_LANG;

    redessine_page_courante(d);
  }
}
static void zoom_in_clicked (GtkEntry* self, gpointer user_data) {
  DonneesDep *d = (DonneesDep *)user_data;

  d->fz = d->fz * 1.25;
  rendu(d, gtk_notebook_get_current_page(GTK_NOTEBOOK(d->doc)));
}
static void zoom_out_clicked (GtkEntry* self, gpointer user_data) {
  DonneesDep *d = (DonneesDep *)user_data;

  d->fz = d->fz / 1.25;
  rendu(d, gtk_notebook_get_current_page(GTK_NOTEBOOK(d->doc)));
}
static void recherche_activated (GtkEntry* self, gpointer user_data) {
  DonneesDep *d = (DonneesDep *)user_data;

  GtkEntryBuffer *buf = gtk_entry_get_buffer(GTK_ENTRY(self));
  const gchar *text   = gtk_entry_buffer_get_text(buf);

  int r;
  char * ptr;
  strtol(text, &ptr, 10);
  if (ptr == text) {
    r = -1;
  } else {
    r = atoi(text);
  }
  d->idRecherche = r;

  char chR[20], chaine[60]= "", chaineR[100];
  _Bool ok = FALSE;

  if (r > -1) {
    for (int i = 0; i < d->nbL; i++) {
      Ligne L = d->lignes[i];
      if (L.nA == r) {
        snprintf(chR, 20, " %d", L.nP+1);
        strcat(chaine, chR);
        ok = TRUE;
      }
    }

    if (ok)
      snprintf(chaineR, 100, "%d dans pages : %s", r, chaine);
    else
      snprintf(chaineR, 100, gtexte(34), r);

    //puts(chaineR);

    guint id = gtk_statusbar_get_context_id(GTK_STATUSBAR(d->statut), "recherche");
    gtk_statusbar_push(GTK_STATUSBAR(d->statut), id, chaineR);
  }

  redessine_page_courante(d);
}
static void appConfigure (GApplication *app, gpointer user_data) {
  GtkWidget *header;
  GtkWidget *bModeP, *bModeF, *bModeL;
  GtkWidget *box;
  GtkWidget *eRech;

  DonneesDep *d = (DonneesDep *)user_data;

  GtkWidget *win = gtk_application_window_new (GTK_APPLICATION (app));
  d->app = app;
  d->win = win;
  gtk_window_set_title (GTK_WINDOW (win), "[Dplr]");
  gtk_window_set_default_size (GTK_WINDOW (win), 670, 1000); // A4
  //gtk_window_set_default_size (GTK_WINDOW (win), 1000, 1000); // Cr30
  gtk_window_set_resizable(GTK_WINDOW (win), TRUE);

  GtkWidget *bv = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_window_set_child(GTK_WINDOW(win), bv);

  #define NB_SECTIONS  5
  #define NB_ITEMS   8
  GMenu *menu     = g_menu_new ();
  GMenu *section[NB_SECTIONS];
  GSimpleAction *act[NB_ITEMS];
  //GSimpleAction *actB;
  GMenuItem *menu_item[NB_ITEMS];
  lmenu lm[NB_ITEMS] = {
    {0, "new",  "Nouveau dépliage", "win.new", new_activated},
    {0, "open", "Ouvrir dépliage", "win.open", open_activated},
    {0, "yyyy", "Ré-ouvrir dépliage", "win.yyyy", ropen_activated},
    //{1, "lobj", "Charger modèle OBJ", "win.lobj", lobj_activated},
    {1, "edpa", "Editer paramètres", "win.edpa", edpa_activated},
    {1, "cala", "Calculer languettes", "win.cala", cala_activated},
    {2, "save", "Sauver", "win.save", save_activated},
    //{2, "xxxx", "Sauver sous", "win.xxxx", sava_activated},
    {3, "expg", "Exporter gabarit", "win.expg", expg_activated},
    {4, "quit", "Quitter", "app.quit", quit_activated}
  };

void creeBouton(char *nomIcone, int numConseil, char *nomAction,
  char *nomActionComplet, void *action, char *nomActionALT, void *actionALT) {
  GtkWidget *btn = gtk_button_new_from_icon_name(nomIcone);
  gtk_box_append (GTK_BOX (box), btn);
  gtk_widget_set_tooltip_text(btn, gtexte(numConseil));
  gtk_actionable_set_detailed_action_name (GTK_ACTIONABLE (btn), nomActionComplet);
  GSimpleAction *actB = g_simple_action_new(nomAction, NULL);
  g_signal_connect (actB, "activate", G_CALLBACK (action), d);
  g_action_map_add_action(G_ACTION_MAP(win), G_ACTION(actB));
  g_simple_action_set_enabled(actB, TRUE);

  if (actionALT != NULL) {
    actB = g_simple_action_new(nomActionALT, NULL);
    g_signal_connect (actB, "activate", G_CALLBACK (actionALT), d);
    g_action_map_add_action(G_ACTION_MAP(win), G_ACTION(actB));
    g_simple_action_set_enabled(actB, TRUE);
  }
}

  for (int i = 0; i < NB_SECTIONS; i++)
    section[i] = g_menu_new();
  for (int i = 0; i < NB_ITEMS; i++) {
    act[i] = g_simple_action_new (lm[i].nom, NULL);
    menu_item[i] = g_menu_item_new(lm[i].texte, lm[i].com);
    //g_signal_connect (act[i], "activate", G_CALLBACK (lm[i].action),  app);
    g_signal_connect (act[i], "activate", G_CALLBACK (lm[i].action),  d);
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

  d->doc = gtk_notebook_new();
  gtk_notebook_set_tab_pos(GTK_NOTEBOOK(d->doc), GTK_POS_LEFT);
  gtk_notebook_set_scrollable(GTK_NOTEBOOK(d->doc), TRUE);
  gtk_notebook_set_show_border(GTK_NOTEBOOK(d->doc), TRUE);
  gtk_box_append (GTK_BOX (bv), d->doc);
  g_signal_connect(GTK_NOTEBOOK(d->doc), "switch-page", G_CALLBACK(changePage), d);

  header = gtk_header_bar_new ();
  gtk_header_bar_pack_end (GTK_HEADER_BAR (header), mb);

  box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
  gtk_widget_add_css_class (box, "linked");

  // Mode Pièce
  bModeP = gtk_toggle_button_new_with_mnemonic(gtexte(31));
  gtk_widget_set_tooltip_text(bModeP, gtexte(36));
  gtk_box_append (GTK_BOX (box), bModeP);

  // Mode Arête
  bModeF = gtk_toggle_button_new_with_mnemonic(gtexte(32));
  gtk_widget_set_tooltip_text(bModeF, gtexte(37));
  gtk_box_append (GTK_BOX (box), bModeF);

  // Mode Languette
  bModeL = gtk_toggle_button_new_with_mnemonic(gtexte(33));
  gtk_widget_set_tooltip_text(bModeL, gtexte(38));
  gtk_box_append (GTK_BOX (box), bModeL);

  // Groupement des 3 modes
  gtk_toggle_button_set_group(GTK_TOGGLE_BUTTON(bModeP), GTK_TOGGLE_BUTTON(bModeF));
  gtk_toggle_button_set_group(GTK_TOGGLE_BUTTON(bModeL), GTK_TOGGLE_BUTTON(bModeF));
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(bModeP), TRUE);
  d->mode = MODE_PIECE;

  // Recherche
  eRech = gtk_entry_new();
  gtk_entry_set_placeholder_text(GTK_ENTRY(eRech), "123");
  gtk_editable_set_width_chars(GTK_EDITABLE(eRech), 3);
  gtk_widget_set_tooltip_text(eRech, gtexte(39));
  gtk_box_append (GTK_BOX (box), eRech);

  // ZOOM
  creeBouton("zoom-in-symbolic", 40, "zoomIn", "win.zoomIn", zoom_in_clicked, "", NULL);
  creeBouton("zoom-out-symbolic", 41, "zoomOut", "win.zoomOut", zoom_out_clicked, "", NULL);

  // ROTATION
  creeBouton("object-rotate-left-symbolic", 42, "rotateLeft", "win.rotateLeft", rotG_activated, "rotateLeft45", rotG45_activated);
  creeBouton("object-rotate-right-symbolic", 43, "rotateRight", "win.rotateRight", rotD_activated, "rotateRight45", rotD45_activated);

  // DEPLACEMENT
  creeBouton("go-up-symbolic", 44, "moveUp", "win.moveUp", moveH_activated, "moveUp20", moveH20_activated);
  creeBouton("go-previous-symbolic", 45, "moveLeft", "win.moveLeft", moveG_activated, "moveLeft20", moveG20_activated);
  creeBouton("go-next-symbolic", 46, "moveRight", "win.moveRight", moveD_activated, "moveRight20", moveD20_activated);
  creeBouton("go-down-symbolic", 47, "moveDown", "win.moveDown", moveB_activated, "moveDown20", moveB20_activated);

  g_signal_connect(eRech, "activate", G_CALLBACK(recherche_activated), d);
  d->idRecherche = -1;

  g_signal_connect(bModeP, "toggled", G_CALLBACK(modeB_toggled), d);
  g_signal_connect(bModeF, "toggled", G_CALLBACK(modeB_toggled), d);
  g_signal_connect(bModeL, "toggled", G_CALLBACK(modeB_toggled), d);

  gtk_header_bar_pack_start (GTK_HEADER_BAR (header), box);

  d->statut = gtk_statusbar_new();
  gtk_box_append (GTK_BOX (bv), d->statut);

  strcpy(d->fichierDAT, "");

  guint stid = gtk_statusbar_get_context_id(GTK_STATUSBAR(d->statut), "...");
  gtk_statusbar_push(GTK_STATUSBAR(d->statut), stid, gtexte(35));

  struct {
    const char *action_and_target;
    const char *accelerators[2];
  } accels[] = {
    { "win.rotateLeft", {"Page_Up", NULL } },
    { "win.rotateLeft45", {"<Control>Page_Up", NULL } },
    { "win.rotateRight", {"Page_Down", NULL } },
    { "win.rotateRight45", {"<Control>Page_Down", NULL } },
    { "win.moveUp", {"Up", NULL } },
    { "win.moveUp20", {"<Control>Up", NULL } },
    { "win.moveLeft", {"Left", NULL } },
    { "win.moveLeft20", {"<Control>Left", NULL } },
    { "win.moveRight", {"Right", NULL } },
    { "win.moveRight20", {"<Control>Right", NULL } },
    { "win.moveDown", {"Down", NULL } },
    { "win.moveDown20", {"<Control>Down", NULL } },
    { "win.zoomIn", {"KP_Add", NULL } },
    { "win.zoomOut", {"KP_Subtract", NULL } }
  };

  for (int i = 0; i < G_N_ELEMENTS (accels); i++)
    gtk_application_set_accels_for_action (GTK_APPLICATION(app), accels[i].action_and_target, accels[i].accelerators);

  gtk_window_set_titlebar (GTK_WINDOW (win), header);

  gtk_window_present(GTK_WINDOW(win));
}

#define TA_TOURNE  1
#define TA_DEPLACE 2
static void lanceAction(gpointer data, int typeAction, int id) {
  DonneesDep *d = (DonneesDep *)data;
  d->id_action = id;
  switch(typeAction) {
    case TA_TOURNE : tournePiece(NULL, data); break;
    case TA_DEPLACE : deplacePiece(NULL, data); break;
  }
}
static void rotG_activated (GSimpleAction *action, GVariant *parameter, gpointer user_data) {
  lanceAction(user_data, TA_TOURNE, R_GAUCHE);
}

static void rotG45_activated (GSimpleAction *action, GVariant *parameter, gpointer user_data) {
  lanceAction(user_data, TA_TOURNE, R_GAUCHE45);
}

static void rotD_activated (GSimpleAction *action, GVariant *parameter, gpointer user_data) {
  lanceAction(user_data, TA_TOURNE, R_DROITE);
}

static void rotD45_activated (GSimpleAction *action, GVariant *parameter, gpointer user_data) {
  lanceAction(user_data, TA_TOURNE, R_DROITE45);
}

static void moveH_activated (GSimpleAction *action, GVariant *parameter, gpointer user_data) {
  lanceAction(user_data, TA_DEPLACE, D_HAUT);
}

static void moveH20_activated (GSimpleAction *action, GVariant *parameter, gpointer user_data) {
  lanceAction(user_data, TA_DEPLACE, D_HAUT20);
}

static void moveB_activated (GSimpleAction *action, GVariant *parameter, gpointer user_data) {
  lanceAction(user_data, TA_DEPLACE, D_BAS);
}

static void moveB20_activated (GSimpleAction *action, GVariant *parameter, gpointer user_data) {
  lanceAction(user_data, TA_DEPLACE, D_BAS20);
}

static void moveG_activated (GSimpleAction *action, GVariant *parameter, gpointer user_data) {
  lanceAction(user_data, TA_DEPLACE, D_GAUCHE);
}
static void moveG20_activated (GSimpleAction *action, GVariant *parameter, gpointer user_data) {
  lanceAction(user_data, TA_DEPLACE, D_GAUCHE20);
}

static void moveD_activated (GSimpleAction *action, GVariant *parameter, gpointer user_data) {
  lanceAction(user_data, TA_DEPLACE, D_DROITE);
}

static void moveD20_activated (GSimpleAction *action, GVariant *parameter, gpointer user_data) {
  lanceAction(user_data, TA_DEPLACE, D_DROITE20);
}

char* gtexte(int n) {
  return (char*)g_locale_from_utf8((gchar*)textes[n], strlen(textes[n]), NULL, NULL, NULL);
}

int main (int argc, char **argv) {
  GtkApplication *app;
  DonneesDep *d = malloc(sizeof *d);

  setlocale(LC_ALL, "C");
  app = gtk_application_new (APPLICATION_ID, G_APPLICATION_DEFAULT_FLAGS);
  g_signal_connect (app, "activate", G_CALLBACK (appConfigure), d);

  g_application_run (G_APPLICATION (app), argc, argv);
  g_free (d->faces);
  g_free (d->voisins);
  g_free (d->v2d);
  g_free (d->v3d);
  g_free (d->tCop);
  g_free (d->sD);
  g_free (d->dispo);
  g_free (d->lAN);
  g_free (d->lSNA);
  g_free (d->page);
  g_free (d->lignes);
  g_free (d->rects);
  g_free (d->tris);
  g_free (d);
  g_object_unref (app);

  return EXIT_SUCCESS;
}
//fin deplieur
