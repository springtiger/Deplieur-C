// PROTOTYPES
//---------- VECTOR2D ----------
Vector2d Vector2dNew(double x, double y);
Vector2d Vector2dAdd (Vector2d v1, Vector2d v2);
Vector2d Vector2dSub (Vector2d v1, Vector2d v2);
Vector2d Vector2dDiv (Vector2d v, double d);
Vector2d Vector2dMul (Vector2d v, double d);
Vector2d centroid(Vector2d* pts);
Vector2d milieu (Vector2d p1, Vector2d p2);
Vector2d vPetit(Vector2d p1, Vector2d p2);
double Vector2dDistance (Vector2d p1, Vector2d p2);
_Bool Vector2dEq (Vector2d p1, Vector2d p2);
Vector2d Vector2dRotation (Vector2d c, Vector2d p, double angle);

//---------- VECTOR3D ----------
Vector3d Vector3dSub (Vector3d v1, Vector3d v2);
double Vector3dDistance (Vector3d p1, Vector3d p2);
_Bool Vector3dEq (Vector3d t1[3], Vector3d t2[3], int n);
void Vector3dD2ize (Vector3d p[3], Vector2d P[3]);
double Vector3dIsCoplanar (Vector3d t[3], Vector3d p);
Vector2d Vector2dPlusPetit(Vector2d p1, Vector2d p2);
Vector2d Vector2dPlusPetit(Vector2d p1, Vector2d p2);

//---------- LIGNE ----------
Ligne LigneNew (int iPage, int iPiece, int id, Vector2d p1, Vector2d p2, int n1, int i1, int n2, int i2, int nA);

//---------- VOISIN ----------
Voisin VoisinNew (int n, int i);

//---------- FONCTIONS ----------
int comp (const void * el1, const void * el2);
int compAff (const void * el1, const void * el2);
int compAffa (const void * el1, const void * el2);
int compLang (const void * el1, const void * el2);
int compLangO (const void * el1, const void * el2);
int compPg (const void * el1, const void * el2);
double degToRad(double degrees);
double radToDeg(double radians);
double direction (Vector2d p1, Vector2d p2);
int prec (int n);
int suiv (int n);
_Bool eqd(double d1, double d2);
double calcAngle (Vector2d a, Vector2d b, Vector2d c);
double angle (Vector2d p1, Vector2d p2);
char* gtexte(int n);

//---------- FONCTIONS APPLICATIVES ----------
void trapeze (Vector2d * P, Vector2d p1, Vector2d p2, double s, double dt);
_Bool li (Vector2d l1S, Vector2d l1E, Vector2d l2S, Vector2d l2E);
_Bool overlap (Vector2d *t1, Vector2d *t2);
void calcBoiteEnglobante(Vector2d b[2], Vector2d *pts, int nbP);
int compteDispo(_Bool *l, int nb);
int premDispo(_Bool *l, int nb);
void calculeVoisinage(int(*f)[4], int n, Voisin v[][3]);
void calculeCop(int n, Voisin voisins[][3], Cop* tCop, Vector3d v3d[][3]);
int sauveLanguettes(Lang * sL, int nbL, int mode);
int sauveDonnees(DonneesDep d);
void supprimeDoublons(Ligne * lignes, int nbL);
void afficheNum(cairo_t *cr, int num, Vector2d p1, Vector2d p2, Coul c);
void afficheNumsPage(cairo_t *cr, AN *lAN, int nbAN, Vector2d v[][3]);
void faitLigne(cairo_t *cr, Vector2d p1, Vector2d p2, int typeL, int hLang);
DonneesDep chargeOBJ(DonneesDep dd);
int retourneLigne(char *contenu, char *tampon, int dep);
DonneesDep chargeDonnees(DonneesDep dd);
DonneesDep init_cairo(DonneesDep d);
void closeCairo(DonneesDep d);
static void pressed (GtkGestureClick *gesture, int n_press, double x, double y, GtkWidget *area);
static void rotated (GtkGestureClick *gesture, int n_press, double x, double y, GtkWidget *area);
static void dessinePage (GtkDrawingArea *area, cairo_t *cr, int width, int height, gpointer data);
void rendu(int);
void deplier();
static void exportePDF();
void ajouteTriangle(int id, Vector2d p1, Vector2d p2, Vector2d p3);
float sign (Vector2d, Vector2d, Vector2d);
_Bool dansTriangle(Vector2d, Vector2d, Vector2d, Vector2d);

//---------- UI ----------
static void appConfigure (GApplication *app, gpointer user_data);
static void redessine_page_courante();
static void on_open_response (GtkDialog *dialog, int response, gpointer data);
static void quit_activated(GSimpleAction *action, GVariant *parameter, gpointer user_data);
static void new_activated(GSimpleAction *action, GVariant *parameter, gpointer data);
static void open_activated(GSimpleAction *action, GVariant *parameter, gpointer user_data);
static void ropen_activated(GSimpleAction *action, GVariant *parameter, gpointer user_data);
static void lobj_activated(GSimpleAction *action, GVariant *parameter, gpointer user_data);
static void edpa_activated(GSimpleAction *action, GVariant *parameter, gpointer user_data);
static void save_activated(GSimpleAction *action, GVariant *parameter, gpointer user_data);
static void sava_activated(GSimpleAction *action, GVariant *parameter, gpointer user_data);
static void expg_activated(GSimpleAction *action, GVariant *parameter, gpointer user_data);
static void modeB_toggled(GtkToggleButton *bouton);
static void recherche_activated (GtkEntry* self, gpointer user_data);
static void zoom_in_clicked (GtkEntry* self, gpointer user_data);
static void zoom_out_clicked (GtkEntry* self, gpointer user_data);


int main (int argc, char **argv);
