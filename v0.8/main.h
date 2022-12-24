#include <gtk/gtk.h>
//------------- ENUMS ------------
enum tAction {
  TA_TOURNE,
  TA_DEPLACE
};

enum {
  L_PLI_M = 1,
  L_PLI_V,
  L_PLI_C,
  L_COUPE,
  L_LGT_M,
  L_LGT_V,
  L_LGT_C
};

enum {
  M_LANG_CREA0,
  M_LANG_CREA1,
  M_LANG_CREA2,
  M_LANG_SAUV
};

enum {
  ME_PIECE_DEPLACER,
  ME_PIECE_TOURNER,
  ME_PIECE_CHOISIR,
  ME_LANG_INVERSER,
  ME_FACE_RATTACHER,
  ME_FACE_DETACHER
};

enum {
  MODE_PIECE = 1,
  MODE_ARETE,
  MODE_LANG
};

enum {
  D_HAUT, D_HAUT5, D_HAUT20,
  D_BAS, D_BAS5, D_BAS20,
  D_GAUCHE, D_GAUCHE5, D_GAUCHE20,
  D_DROITE, D_DROITE5, D_DROITE20
};

enum {
  R_GAUCHE, R_GAUCHE5, R_GAUCHE45,
  R_DROITE, R_DROITE5, R_DROITE45
};
/*#define R_GAUCHE    5
#define R_GAUCHE45  25
#define R_DROITE    6
#define R_DROITE45  26*/

//---------- STRUCTURES ----------
typedef struct {
  char* nomIcone;
  int numConseil;
  char* nomAction;
  char* nomActionComplet;
  void* action;
  char* nomActionCTRL;
  void* actionCTRL;
  char* nomActionALT;
  void* actionALT;
} sBtns;

typedef struct { // menu
  int nSection;
  const char* nom;
  const char* texte;
  const char* com;
  void* action;
} lmenu;

typedef struct { // coordonnées 3d
  double x, y, z;
} Vector3d;

typedef struct { // coordonnées 2d
  double x, y;
} Vector2d;

typedef struct { // rect
  int id;
  int type;
  Vector2d p1;
  Vector2d p2;
} rectT;

typedef struct { // tris
  int id;
  Vector2d p1;
  Vector2d p2;
  Vector2d p3;
} triT;

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
  int nPP;    // id de la piece
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

typedef struct { // paramètres courants
  //char fichierOBJ[128];
  float echelle;
  int formatPage;
  int typeLang;
  //int premierTriangle;
  float tailleNums;
  int hauteurLang;

} DonneesParam;

typedef struct {
  int orig;
  int face;
  int mis;
} tB;

typedef struct { // données de dépliage
  char fichierOBJ[128];
  char fichierDAT[128];
  float echelle;
  float fz;
  int formatPage;
  int typeLang;
  int premierTriangle;
  float tailleNums;
  int hauteurLang;

  int nbFaces;
  int nbSommets;

  Vector3d(*v3d)[3];
  Vector2d(*v2d)[3];
  int(*faces)[4];
  Voisin(*voisins)[3];
  Cop* tCop;
  _Bool* dispo;
  Depliage* sD;
  int nbD;
  AN* lAN;
  int nbAN;
  NAff* lSNA;
  int nAff;
  int* page;
  int nbP;
  int nbPP;
  Ligne* lignes;
  int nbL;
  Lang* sLgt;
  int nbLgt;

  rectT* rects;
  int nbRects;

  triT* tris;
  int nbTris;

  int mode;
  int idCourant;
  int idRecherche;
  int pCourante;
  int pPrecedente;
  int nbPages;
  Vector2d coords;
  int id_action;

  cairo_surface_t* surface;
  cairo_t* cr;
  GApplication* app;
  GtkWidget* win;
  GtkWidget* boxV;
  //GtkWidget *doc;
  //GtkWidget **dpage;
  GtkWidget* dpage;
  GtkWidget** bPage;
  //GtkWidget **dlabel;
  GtkWidget* statut;

  GtkWidget* params[5];
} DonneesDep;

// PROTOTYPES
//---------- VECTOR2D ----------
Vector2d Vector2dNew(double x, double y);
Vector2d Vector2dAdd(Vector2d v1, Vector2d v2);
Vector2d Vector2dSub(Vector2d v1, Vector2d v2);
Vector2d Vector2dDiv(Vector2d v, double d);
Vector2d Vector2dMul(Vector2d v, double d);
Vector2d centroid(Vector2d* pts);
Vector2d milieu(Vector2d p1, Vector2d p2);
Vector2d vPetit(Vector2d p1, Vector2d p2);
double Vector2dDistance(Vector2d p1, Vector2d p2);
_Bool Vector2dEq(Vector2d p1, Vector2d p2);
Vector2d Vector2dRotation(Vector2d c, Vector2d p, double angle);

//---------- VECTOR3D ----------
Vector3d Vector3dSub(Vector3d v1, Vector3d v2);
double Vector3dDistance(Vector3d p1, Vector3d p2);
_Bool Vector3dEq(Vector3d t1[3], Vector3d t2[3], int n);
void Vector3dD2ize(Vector3d p[3], Vector2d P[3]);
double Vector3dIsCoplanar(Vector3d t[3], Vector3d p);
Vector2d Vector2dPlusPetit(Vector2d p1, Vector2d p2);

//---------- LIGNE ----------
Ligne LigneNew(int iPage, int iPiece, int id, Vector2d p1, Vector2d p2, int n1, int i1, int n2, int i2, int nA);

//---------- VOISIN ----------
Voisin VoisinNew(int n, int i);

//---------- FONCTIONS ----------
int comp(const void* el1, const void* el2);
int compAff(const void* el1, const void* el2);
int compAffa(const void* el1, const void* el2);
int compLang(const void* el1, const void* el2);
int compLangO(const void* el1, const void* el2);
int compPg(const void* el1, const void* el2);
double degToRad(double degrees);
double radToDeg(double radians);
double direction(Vector2d p1, Vector2d p2);
int prec(int n);
int suiv(int n);
_Bool eqd(double d1, double d2);
double calcAngle(Vector2d a, Vector2d b, Vector2d c);
double angle(Vector2d p1, Vector2d p2);
//char* gtexte(int n);
char* VerifNomFichier(char* nom);

//---------- FONCTIONS APPLICATIVES ----------
void trapeze(Vector2d* P, Vector2d p1, Vector2d p2, double s, double dt);
_Bool li(Vector2d l1S, Vector2d l1E, Vector2d l2S, Vector2d l2E);
_Bool overlap(Vector2d* t1, Vector2d* t2);
void calcBoiteEnglobante(Vector2d b[2], Vector2d* pts, int nbP);
int compteDispo(_Bool* l, int nb);
int premDispo(_Bool* l, int nb);
void calculeVoisinage(int(*f)[4], int n, Voisin v[][3]);
void calculeCop(int n, Voisin voisins[][3], Cop* tCop, Vector3d v3d[][3]);
int sauveLanguettes(Lang* sL, int nbL, int mode, char* nomFichier);
int sauveDonnees(DonneesDep* d);
void supprimeDoublons(Ligne* lignes, int nbL);
void afficheNum(DonneesDep* d, cairo_t* cr, int num, Vector2d p1, Vector2d p2, Coul c, int typeA);
void faitLigne(DonneesDep* d, cairo_t* cr, Vector2d p1, Vector2d p2, int typeL);
int chargeOBJ(DonneesDep* d);
int retourneLigne(char* contenu, char* tampon, int dep);
int chargeDonnees(DonneesDep* d);
static void pressed(GtkGestureClick* gesture, int n_press, double x, double y, DonneesDep* d);
static void rotated(GtkGestureClick* gesture, int n_press, double x, double y, DonneesDep* d);
static void dessinePage(GtkDrawingArea* area, cairo_t* cr, int width, int height, gpointer data);
void rendu(DonneesDep* d, int pc);
void deplier();
static void sauvePDF(DonneesDep* d, GFile* file);
static void sauveDepliage(DonneesDep* d, GFile* file);
int inversePiece(Depliage* SDi, tB* B, int nbB, int ln0, int ln3, int ln0orig);

void ajouteTriangle(DonneesDep* d, int id, Vector2d p1, Vector2d p2, Vector2d p3);
float sign(Vector2d, Vector2d, Vector2d);
_Bool dansTriangle(Vector2d, Vector2d, Vector2d, Vector2d);

//---------- UI ----------
void creeBouton(sBtns B, GtkWidget *boxH, DonneesDep *d);
static void appConfigure(GApplication* app, gpointer data);
static void redessine_page_courante(DonneesDep* d);
static void on_changePage(GObject* source, GAsyncResult* result, void* data);

static void activated_new(GSimpleAction* action, GVariant* parameter, gpointer data);
static void nouveauDepliage(GObject* source, GAsyncResult* result, void* data);

static void activated_open(GSimpleAction *, GVariant *, gpointer);
static void ouvreDepliage(GObject* source, GAsyncResult* result, void* data);

static void activated_save(GSimpleAction* action, GVariant* parameter, gpointer data);
static void exporteDepliage(GObject* source, GAsyncResult* result, void* data);

static void activated_expg(GSimpleAction* action, GVariant* parameter, gpointer data);
static void exportePDF(GObject *source, GAsyncResult *result, void *data);

static void activated_cala(GSimpleAction* action, GVariant* parameter, gpointer data);
//static void activated_edpa(GSimpleAction* action, GVariant* parameter, gpointer data);
static void activated_quit(GSimpleAction* action, GVariant* parameter, gpointer data);

static void activated_rech(GtkEntry* self, gpointer data);

static void toggleB(GtkToggleButton* source, gpointer data);
static void modeB_toggled(GtkToggleButton* bouton, gpointer data);

static void deplacePiece(GtkEntry* self, gpointer data);
static void tournePiece(GtkEntry* self, gpointer data);
static void lanceAction(gpointer data, enum tAction typeAction, int id);

static void zoom_in_clicked(GSimpleAction*, GVariant*, gpointer);
static void zoom_out_clicked(GSimpleAction*, GVariant*, gpointer);

static void rotG(GSimpleAction*, GVariant*, gpointer);
static void rotG5(GSimpleAction*, GVariant*, gpointer);
static void rotG45(GSimpleAction*, GVariant*, gpointer);

static void rotD(GSimpleAction*, GVariant*, gpointer);
static void rotD5(GSimpleAction*, GVariant*, gpointer);
static void rotD45(GSimpleAction*, GVariant*, gpointer);

static void moveH(GSimpleAction*, GVariant*, gpointer);
static void moveH5(GSimpleAction* a, GVariant* p, gpointer d);
static void moveH20(GSimpleAction* a, GVariant* p, gpointer d);

static void moveB(GSimpleAction* a, GVariant* p, gpointer d);
static void moveB5(GSimpleAction* a, GVariant* p, gpointer d);
static void moveB20(GSimpleAction* a, GVariant* p, gpointer d);

static void moveG(GSimpleAction* a, GVariant* p, gpointer d);
static void moveG5(GSimpleAction* a, GVariant* p, gpointer d);
static void moveG20(GSimpleAction* a, GVariant* p, gpointer d);

static void moveD(GSimpleAction* a, GVariant* p, gpointer d);
static void moveD5(GSimpleAction* a, GVariant* p, gpointer d);
static void moveD20(GSimpleAction* a, GVariant* p, gpointer d);

int main(int argc, char** argv);
