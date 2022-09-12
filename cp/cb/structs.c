#include <gtk/gtk.h>
//---------- STRUCTURES ----------
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
