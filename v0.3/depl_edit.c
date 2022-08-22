/* edition d'un dépliage (fichier .dep) en C
v0.2
- charge fichier .dep
- charge fichier .obj lié, extrait points et faces
- trouve les voisins de chaque arête
- calcule les coplaneités
- tourne les pages
- crée le fichier des languettes (.lng)
- edite les languettes
- crée le gabarit (.pdf)
*/

#include "deputils.c"

Lang * inverseLanguette(Lang * sLgt, int nbLgt){
	int nvl, nO, saisie, sOK;
	do
	{
		printf("%s", textes[26]); // Inverser languette
		do {
			saisie = toupper(getchar());
			sOK = (saisie == OUI) || (saisie == NON);
		} while (! sOK);
		if (saisie == OUI)
		{
			printf("%s", textes[30]);
			if ((nvl = scanf(" %d", &nO)) == 1)
			{
				Lang L;
				for (int i = 0; i < nbLgt; i++)
				{
					L = sLgt[i];
					if (L.o == nO){
						printf("%d : %d %d %d = %d\n", i, L.o, L.n1, L.n2, L.v); 
						sLgt[i].v = 1 - L.v;
					}
				}
				sauveLanguettes(sLgt, nbLgt, M_LANG_SAUV);
			}
		}
	} while (saisie == OUI);
	return sLgt;
}

DonneesDep TournerPage(DonneesDep dd)
{
	int *sDP = NULL;
	int nPsD = 0;
	for (int i = 0; i < dd.nbD; i++)
	{
		if (dd.sD[i].orig == -1)
		{
			int * tmp = (int *) realloc(sDP, (nPsD+1) * (sizeof * sDP) );
			if (tmp)
			{
				sDP = tmp;
				sDP[nPsD++] = i;
			}
		}
	}

	char repLng;
	int numPaT, nvl;
	int nbChgt = 0;
	printf("%s", textes[15]); // Tourner des pages ?
	if ((nvl = scanf(" %c", &repLng)) == 1)
	{
		if (toupper(repLng) == OUI)
		{
			do
			{
				printf("%s", textes[16]);
				if ((nvl = scanf(" %d", &numPaT)) == 1)
				{
					if ((numPaT > 0) && (numPaT <= nPsD))
					{
						int apage = dd.sD[sDP[numPaT-1]].a;
						printf("%s %d %s : %d\n%s ?", 
						textes[17], numPaT, textes[18], apage, textes[19]);
						if ((nvl = scanf(" %d", &apage)) == 1)
						{
							dd.sD[sDP[numPaT-1]].a = apage;
							nbChgt++;
						}
					}
				}
			} while (numPaT > 0);
		}
	}
	free(sDP);	
	if (nbChgt > 0)
	{	
		sauveDonnees(dd);
	}
	return dd;
}

int main(void)
{	// charge données
	int nvl;
	char* nomFichierDonnees = "donnees.dep";
	FILE* fd;
	if (!(fd = fopen(nomFichierDonnees, "r")))
	{
		perror(textes[2]);
		exit(1);
	}

	DonneesDep dd;
	fscanf(fd, "%s", dd.fichierOBJ);
	fscanf(fd, "%f", &(dd.echelle));
	fscanf(fd, "%2d", &(dd.formatPage));
	fscanf(fd, "%4d", &(dd.premierTriangle));
	fscanf(fd, "%f", &(dd.tailleNums));
	fscanf(fd, "%2d", &(dd.hauteurLang));
	dd.sD = NULL;
	dd.nbD = 0;
	Depliage sD0;

	int d0, d1, d2, d3;
	while ((nvl = fscanf(fd, "%d", &d0)) > 0)
	{
		if (d0 == -1)
		{
			fscanf(fd, "%d", &d1);
			fscanf(fd, "%d", &d2);
		}
		else
		{
			fscanf(fd, "%d", &d1);
			d2 = 0;
		}
		sD0.orig = d0;
		sD0.face = d1;
		sD0.a = d2;
		Depliage* tmp = (Depliage*)realloc(dd.sD, sizeof(Depliage) * (dd.nbD + 1));
		if (tmp)
		{
			dd.sD = tmp;
			dd.sD[dd.nbD] = sD0;
			dd.nbD++;
		}
	}
	if (fclose(fd) == EOF)
	{
		perror(textes[3]);
		exit(1);
	}

	// charge données languettes
	int nbLgt = 0;
	Lang* sLgt = NULL;
	int nbLgtB = 0;
	Lang* sLgtB = NULL;
	Lang sLgt0;

	char* nomFichierLanguettes = "donnees.lng";
	if ((fd = fopen(nomFichierLanguettes, "r")))
	{
		while ((nvl = fscanf(fd, "%d", &d0)) > 0)
		{
			fscanf(fd, "%d", &d1);
			fscanf(fd, "%d", &d2);
			fscanf(fd, "%d", &d3);
			sLgt0.o  = d0;
			sLgt0.n1 = d1;
			sLgt0.n2 = d2;
			sLgt0.v  = d3;
			Lang* tmp = (Lang*)realloc(sLgt, sizeof(Lang) * (nbLgt + 1));
			if (tmp)
			{
				sLgt = tmp;
				sLgt[nbLgt] = sLgt0;
				nbLgt++;
			}
		}
		if (fclose(fd) == EOF)
		{
			perror(textes[3]);
			exit(1);
		}
	}

	printf("%s %d\n", textes[11], dd.nbD);
	printf("%s %s\n", textes[0], dd.fichierOBJ);
	printf("%s %lf\n",textes[1], dd.echelle);
	printf("%s A%d\n",textes[12], dd.formatPage);
	printf("%s %f\n", textes[13], dd.tailleNums);
	printf("%s %d\n", textes[14], dd.hauteurLang);
	printf("%s %d\n", textes[9], dd.premierTriangle);
	
	dd = chargeOBJ(dd);
	printf("\n%d %s - %d %s\n", dd.nbSommets, textes[5], dd.nbFaces, textes[6]);
	
	dd = TournerPage(dd);
	sLgt = inverseLanguette(sLgt, nbLgt);

	printf("%s", textes[27]);
	char chAffNumFace;
	_Bool bAffNumFace;
	if ((nvl = scanf(" %c", &chAffNumFace))== 1)
	{
		bAffNumFace = toupper(chAffNumFace) == 'O';
	}

	dd = init_cairo(dd);
	
	// DEBUT DEPLIAGE
	dd.lSNA = calloc(dd.nbFaces * 3, sizeof * dd.lSNA);
	dd.nAff = 0;

	int nbP = -1;
	Vector2d* vMin = NULL;
	dd.lignes = calloc(dd.nbFaces * 3, sizeof * dd.lignes);
	dd.nbL = 0;
	for (int i = 0; i < dd.nbD; i++)
	{
		if (dd.sD[i].orig == -1)
		{
			nbP++;
			int f = dd.sD[i].face;
			int a = dd.sD[i].a;
			if (a != 0)
			{
				Vector2d m = centroid(dd.v2d[f]);
				for (int vi = 0; vi < 3; vi++)
					dd.v2d[f][vi] = rotation(m, dd.v2d[f][vi], degToRad(a));
			}
			Vector2d * tmp = (Vector2d*)realloc(vMin, sizeof(Vector2d) * (nbP + 1));
			if (tmp)
			{
				vMin = tmp;
				vMin[nbP] = vPetit(dd.v2d[f][0], vPetit(dd.v2d[f][1], dd.v2d[f][2]));
			}
		}
		dd.sD[i].page = nbP;

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
				vMin[nbP] = vPetit(vMin[nbP], dd.v2d[vc][n]);
			}
		}
	}

	for (int i = 0; i < dd.nbD; i++)
	{
		int tc = dd.sD[i].face;
		int pc = dd.sD[i].page;
		for (int j = 0; j < 3; j++)
		{
			dd.v2d[tc][j] = Vector2dSub(dd.v2d[tc][j], Vector2dSub(vMin[pc], marge));
		}
	}
	free(vMin);

	for (int i = 0; i < dd.nbD; i++)
	{
		int tc = dd.sD[i].face;
		int pc = dd.sD[i].page;
		for (int j = 0; j < 3; j++)
		{
			dd.lignes[dd.nbL] = LigneNew(pc, dd.nbL,
				dd.v2d[tc][j], dd.v2d[tc][suiv(j)],
				tc, j,
				dd.voisins[tc][j].nF, dd.voisins[tc][j].idx);
			dd.nbL++;
		}
	}

	supprimeDoublons(dd.lignes, dd.nbL);

	qsort(dd.lSNA, dd.nAff, sizeof(NAff), compAff);
	qsort(dd.lignes, dd.nbL, sizeof(Ligne), compPg);
	qsort(sLgt, nbLgt, sizeof(Lang), compLang);

	int lc = 0;
	int nA;
	int typeL;

	dd.lAN = calloc(dd.nbFaces, sizeof * dd.lAN);
	dd.nbAN = 0;
	int ppc = 0;

	for (int i = 0; i < dd.nbL; i++)
	{
		Ligne l = dd.lignes[i];
		if (l.id > -1)
		{
			if (lc != l.nP)
			{
				lc = l.nP;
				if (dd.nbAN > 0)
				{
					afficheNumsPage(dd.cr, dd.lAN, dd.nbAN, dd.v2d);
					for (int sdi = 0; sdi < dd.nbD; sdi++)
					{
						Depliage sdt = dd.sD[sdi];
						if (sdt.page == ppc)
						{
							if (bAffNumFace || (dd.sD[sdi].orig == -1))
							{
								Vector2d m = centroid(dd.v2d[sdt.face]);
								afficheNum(dd.cr, sdt.face, m, m, C_BLEU);
							}
						}
					}
				}
				dd.nbAN = 0;
				cairo_show_page(dd.cr);
				ppc++;
			}

			double c = dd.tCop[(l.n1 * 3) + l.i1].cop;
			if (l.nb == 1)
			{
				Lang l0 = { .n1 = l.n1, .n2 = l.n2 };
				Lang* rL;
				if (nbLgt > 0)
				{
					rL = (Lang*)bsearch(&l0, sLgt, nbLgt, sizeof(Lang), compLang);
					if (!rL)
					{
						typeL = L_COUPE;
					}
					else if (rL->v == 0)
					{
						typeL = L_COUPE;
					}
					else
					{
						if (fabs(c) < 10e-7)
						{
							typeL = L_LGT_C;
						}
						else
						{
							typeL = c < 0 ? L_LGT_M : L_LGT_V;
						}
					}
				}
				else
				{
					typeL = L_COUPE;
				}

				l0.v = l.n1 < l.n2 ? 1 : 0;
				Lang* tmp = (Lang*)realloc(sLgtB, sizeof(Lang) * (nbLgtB + 1));
				if (tmp)
				{
					sLgtB = tmp;
					sLgtB[nbLgtB] = l0;
					nbLgtB++;
				}
			}
			else
			{
				if (fabs(c) < 10e-7)
				{
					typeL = L_PLI_C;
				}
				else
				{
					typeL = c < 0 ? L_PLI_M : L_PLI_V;
				}
			}
			if (typeL != L_PLI_C)
			{
				faitLigne(dd.cr, l.p1, l.p2, typeL, dd.hauteurLang);
			}
			if (l.nb == 1)
			{
				NAff cleN;
				NAff* rechN;
				cleN.nMax = max(l.n1, l.n2);
				cleN.nMin = min(l.n1, l.n2);
				rechN = (NAff*)bsearch(&cleN, dd.lSNA, dd.nAff, sizeof(NAff), compAff);
				if (rechN)
					nA = rechN->a;
				else
				{
					cleN.a = dd.nAff;
					nA = dd.nAff;
					dd.lSNA[dd.nAff++] = cleN;
					qsort(dd.lSNA, dd.nAff, sizeof(NAff), compAff);
				}
				AN sAN0 = { nA, l.p1, l.p2 };
				dd.lAN[dd.nbAN++] = sAN0;
			}
		}
	}

	if (dd.nbAN > 0) 
	{
		afficheNumsPage(dd.cr, dd.lAN, dd.nbAN, dd.v2d);
		for (int sdi = 0; sdi < dd.nbD; sdi++)
		{
			Depliage sdt = dd.sD[sdi];
			if (sdt.page == ppc)
			{
				if (bAffNumFace || (dd.sD[sdi].orig == -1))
				{
					Vector2d m = centroid(dd.v2d[sdt.face]);
					afficheNum(dd.cr, sdt.face, m, m, C_BLEU);
				}
			}
		}
	}
	
	printf("%s", textes[20]); // fichier languettes
	char repLng;
	if ((nvl = scanf(" %c", &repLng)) == 1)
	{
		if (toupper(repLng) == 'O')
		{
			puts(textes[21]);
			puts(textes[22]);
			puts(textes[23]);
			puts(textes[24]);
			printf("%s", textes[25]);
			int repTLng;
			nvl = scanf(" %d", &repTLng);
			if ((repTLng >= 0) && (repTLng <= 2))
			{
				sauveLanguettes(sLgtB, nbLgtB, repTLng);
			}
		}
	}
	
	free(dd.faces);
	free(dd.voisins);
	free(dd.v2d);
	free(dd.v3d);
	free(dd.tCop);
	free(dd.sD);
	free(dd.lAN);
	free(dd.lSNA);
	free(dd.lignes);

	free(sLgt);
	free(sLgtB);
	
	closeCairo(dd);

	return 0;
}
