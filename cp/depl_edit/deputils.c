// Depliage : structures et fonctions

// STRUCTURES
struct sVector3d { // coordonnées 3d
	double x, y, z;
};

struct sVector2d { // coordonnées 2d
	double x, y;
};

struct sCop { // coplanarité
	int nF, nV;	// n° face et voisin
	double cop; // valeur de la coplanarité (0 = coplanaire)
};

struct sVoisin { // voisinage
	int nF;		// n° de la face liée
	int idx;	// index du 1er point lié
};

struct sLigne  { // ligne d'affichage
	int id;
	struct sVector2d p1, p2; // points (p1 : petit, p2 : grand)
	int n1, i1; // relié à triangle + index (petit)
	int n2, i2;	// relié à triangle + index (grand)
	int nb;			// nb (1 = coupe, 2 = pli)
	int nP;		// id de la page
};

struct sNAff	{	// numérotation des paires d'arêtes
	int nMin; // petit n°
	int nMax; // grand n°
	int a;	// Affichage
};

struct sCoul {	// couleurs
	double r, v, b;
	};
	
const struct sCoul 
	C_NOIR	= {0, 0, 0},
	C_ROUGE	= {1, 0, 0},
	C_VERT	= {0, 1, 0},
	C_BLEU	= {0, 0, 1},
	C_MARRON= {0.5, 0, 0};

struct sDepliage {
	int page;
	int face;
	int orig;
	int a;
};

struct sAN {
	int n;
	struct sVector2d p1, p2;
};

struct sLang {
	int n1;
	int n2;
	int v;
};

//#define max(a,b) (a>=b?a:b)
//#define min(a,b) (a<=b?a:b)

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

const double pi = 3.14159265358979323846;

// FONCTIONS
_Bool eqd(double d1, double d2) {
	return fabs(d1 - d2) < epsilon;
}

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

struct sVector2d sVector2dNew(double x, double y) {
	struct sVector2d r;
	r.x = x;
	r.y = y;
	
	return r;
}

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

struct sVector2d vPetit(struct sVector2d p1, struct sVector2d p2) {
	struct sVector2d r;
	r.x = fmin(p1.x, p2.x);
	r.y = fmin(p1.y, p2.y);

	return r;
}

double direction (struct sVector2d p1, struct sVector2d p2) {
	return atan2(p2.y - p1.y, p2.x - p1.x);
}

double degToRad(double degrees) { return degrees * pi / 180; }

void d2ize (struct sVector3d p[3], struct sVector2d P[3]) {
	struct sVector3d
		d1 = sVector3dSub(p[1], p[0]),
		d2 = sVector3dSub(p[2], p[0]);
		
		P[0].x = 0;
		P[0].y = 0;
		P[1].x = sqrt((d1.x*d1.x) + (d1.y * d1.y) + (d1.z * d1.z));
		P[1].y = 0;
		P[2].x = ((d1.x*d2.x) + (d1.y*d2.y) + (d1.z*d2.z))/ P[1].x;
		P[2].y = sqrt((d2.x*d2.x) + (d2.y*d2.y) + (d2.z*d2.z) - (P[2].x*P[2].x));
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

double diff (struct sVector2d p1, struct sVector2d p2) {
	double r;
	
	if (eqd(p1.x, p2.x))
		r = p2.y - p1.y;
	else
		r = p2.x - p1.x;
		
	return r;
}

double distance2d (struct sVector2d p1, struct sVector2d p2) {
	struct sVector2d d = sVector2dSub(p2, p1);
	return sqrt((d.x * d.x) + (d.y * d.y));
}

double distance3d (struct sVector3d p1, struct sVector3d p2) {
	struct sVector3d d = sVector3dSub(p2, p1);
	return sqrt((d.x * d.x) + (d.y * d.y) + (d.z * d.z));
}

_Bool eq (struct sVector2d p1, struct sVector2d p2) {
	return distance2d(p1, p2) < epsilon;
}

_Bool eq3 (struct sVector3d t1[3], struct sVector3d t2[3], int n) {
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

double angle (struct sVector2d p1, struct sVector2d p2) {
	//double r = atan2(p1.y - p2.y, p1.x - p2.x);
	double r = atan2(p2.y - p1.y, p2.x - p1.x);
	return r;
}

int compPg (const void * el1, const void * el2) {
	struct sLigne v1 = * (const struct sLigne *) el1;
	struct sLigne v2 = * (const struct sLigne *) el2;
	
	int r = v1.nP - v2.nP;
	
	return r;
}

int compAff (const void * el1, const void * el2) {
	struct sNAff v1 = * (const struct sNAff *) el1;
	struct sNAff v2 = * (const struct sNAff *) el2;

	int r = (v1.nMax != v2.nMax) ? v1.nMax - v2.nMax : v1.nMin - v2.nMin;

	return r;
}

int compAffa (const void * el1, const void * el2) {
	struct sNAff v1 = * (const struct sNAff *) el1;
	struct sNAff v2 = * (const struct sNAff *) el2;

	int r = v1.a - v2.a;

	return r;
}

int compLang (const void * el1, const void * el2) {
	struct sLang v1 = * (const struct sLang *) el1;
	struct sLang v2 = * (const struct sLang *) el2;
	
	int r = (v1.n1 - v2.n1) * 2000 + (v1.n2 - v2.n2);
	
	return r;
}

void trapeze (struct sVector2d * P, struct sVector2d p1, 
	struct sVector2d p2, double s, double dt) {
	double d = distance2d(p1, p2);
	double a = degToRad(90) - direction(p1, p2);
	if (d > 50) dt = dt/2;
	//if (d > 100) dt = dt/2;

	P[0] = rotation(p1, p1, a);
	P[1] = rotation(p1, sVector2dAdd(p1, sVector2dNew(s, d*dt)), a);
	P[2] = rotation(p1, sVector2dAdd(p1, sVector2dNew(s, d*(1-dt))), a);
	P[3] = rotation(p1, sVector2dAdd(p1, sVector2dNew(0, d)), a);
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

_Bool li (struct sVector2d l1S, struct sVector2d l1E, 
				 struct sVector2d l2S, struct sVector2d l2E) {
	// true if the lines intersect
	if (eq(l1S, l2S) || eq(l1S, l2E) || eq(l1E, l2S) || eq(l1E, l2E)) {
		return 0;
	}

	double denominator = ((l2E.y - l2S.y) * (l1E.x - l1S.x))
										 - ((l2E.x - l2S.x) * (l1E.y - l1S.y));

	if (denominator == 0) {
		return 0;
	}

	double 
		a = l1S.y - l2S.y,
		b = l1S.x - l2S.x,
		numerator1 = ((l2E.x - l2S.x) * a) - ((l2E.y - l2S.y) * b),
		numerator2 = ((l1E.x - l1S.x) * a) - ((l1E.y - l1S.y) * b);
	a = numerator1 / denominator;
	b = numerator2 / denominator;

	if ((a > 0) && (a < 1) && (b > 0) && (b < 1)) {
		return 1;
	} else {
		return 0;
	}
}

_Bool overlap (struct sVector2d *t1, struct sVector2d *t2) {
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

int compteDispo(_Bool *l, int nb) {
	int n = 0;
	for (int i = 0; i < nb; i++) {
		if (l[i])
			n++;
	}
return n;	
}

int premDispo(_Bool *l, int nb) {
int n = -1, i = 0;
do {
	if (l[i])
		n = i;
	i++;	
} while ((i < nb) &&  (n == -1));

return n;
}

void calculeVoisinage(int f[][4], int n, struct sVoisin v[][3]) {
	for (int i = 0; i < n; i++) {
		for (int j = 0; j < 3; j++) {		
			int vi = 0;
			_Bool ok = 0;
			do {
				if (vi != i) {
					for (int k = 0; (k < 3) && (!ok); k++) {
						if ( (f[vi][k] == f[i][suiv(j)]) 
							&& (f[vi][suiv(k)] == f[i][j])) {
							v[i][j]= sVoisinNew(vi, suiv(k));
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

void calculeCop(int n, struct sVoisin voisins[][3], struct sCop* tCop, struct sVector3d v3d[][3]) {
	int nbCop = 0;
	for (int i = 0; i < n; i++){
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
}

int sauveLanguettes(struct sLang * sL, int nbL, int mode) {
	char * nomFichierDonnees = "donnees.lng";
	FILE * fichierDonnees;
	int rc;
	
	qsort(sL, nbL, sizeof(struct sLang), compLang);

	fichierDonnees = fopen(nomFichierDonnees, "w");
	if (fichierDonnees == NULL) {
		printf("Impossible de sauvegarder les données\n");
		return -1;
	}

	int v;
	for (int i = 0; i < nbL; i++) {
		if (mode == M_LANG_SAUV)
			v = sL[i].v;
		else if (mode == M_LANG_CREA0)
			v = 0;
		else if (mode == M_LANG_CREA1)
			v = sL[i].n1 > sL[i].n2 ? 0 : 1;
		else //if (mode == M_LANG_CREA2)
			v = 1;
		fprintf(fichierDonnees, "%4d %4d %d\n", sL[i].n1, sL[i].n2, v);
	}
	rc = fclose(fichierDonnees);
	if (rc == EOF ) {
		fprintf(stderr, "Impossible de fermer le fichier\n");
		printf("Impossible de fermer le fichier\n");
		return -1;
	}
	
	return 0;
}


int sauveDonnees(char *OBJ, double ech, int fc, int tc0, float fsize, int hL,
		struct sDepliage * sD, int nbD) {
	char * nomFichierDonnees = "donnees.dep";
	FILE * fichierDonnees;
	int rc;

	fichierDonnees = fopen(nomFichierDonnees, "w");
	if (fichierDonnees == NULL) {
		printf("Impossible de sauvegarder les données\n");
		return -1;
	}

	fprintf(fichierDonnees, "%s\n", OBJ);
	fprintf(fichierDonnees, "%5.2lf\n", ech);
	fprintf(fichierDonnees, "%2d\n", fc);
	fprintf(fichierDonnees, "%4d\n", tc0);
	fprintf(fichierDonnees, "%5.2f\n", fsize);
	fprintf(fichierDonnees, "%2d\n", hL);
	for (int i = 0; i < nbD; i++) {
		if (sD[i].orig == -1) {
			fprintf(fichierDonnees, "%4d %4d %4d\n", sD[i].orig, sD[i].face, sD[i].a);
		} else {
			fprintf(fichierDonnees, "%4d %4d\n", sD[i].orig, sD[i].face);
		}
	}
	rc = fclose(fichierDonnees);
	if (rc == EOF ) {
		fprintf(stderr, "Impossible de fermer le fichier\n");
		printf("Impossible de fermer le fichier\n");
		return -1;
	}
	
	return 0;
}

void supprimeDoublons(struct sLigne * lignes, int nbL) {
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
}

void afficheNum(cairo_t *cr, int num, struct sVector2d p1,
			struct sVector2d p2, struct sCoul c) {
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

void afficheNumsPage(cairo_t *cr, struct sAN *lAN, int nbAN, struct sVector2d v[][3]) {
	for (int iAN = 0; iAN < nbAN; iAN++){
		struct sAN lANc = lAN[iAN];
		struct sCoul lC;
		if ( ((iAN > 0) 			&& (lANc.n == lAN[iAN-1].n))
			|| ((iAN < nbAN-1) && (lANc.n == lAN[iAN+1].n)))
			lC = C_VERT;
		else
			lC = C_NOIR;
		afficheNum(cr, lANc.n, lANc.p1, lANc.p2, lC);
	}
}

void faitLigne(cairo_t *cr, struct sVector2d p1, struct sVector2d p2, int typeL, int hLang) {
	if (typeL != L_PLI_C) { // pas de ligne si pli coplanaire
		struct sCoul c;
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
			struct sVector2d pts[4];
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

