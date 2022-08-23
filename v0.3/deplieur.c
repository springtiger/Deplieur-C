/* depliage en C
 v0
- charge fichier .obj
- extrait points et faces
- trouve les voisins de chaque arête
- calcule les coplaneités
- demande le fichier 2d(.obj)
- demande l'échelle (1 = 100%)
- demande le format de sortie (de 0 à 5 pour A0 à A5)
- deplie dans un PDF avec autant de pages que nécessaire
- traits de coupe en rouge
- les plis montagne en - marron, plis vallée en -. vert
- numérote uniquement les paires d'arêtes à relier
- paires d'arêtes internes à la pièce en bleu
- paires d'arêtes entre deux pièces en noir
- sauve les données dans un fichier .dep
*/

#include "deputils.c"

int main(void)
{
	DonneesDep dd;
	int nvl;
	
	printf("%s", textes[0]); // Saisie nom fichier OBJ
	nvl = scanf(" %49s", dd.fichierOBJ);

	printf("%s", textes[1]); // Saisie échelle
	nvl = scanf("%f", &(dd.echelle));

	dd = chargeOBJ(dd);

	printf("%s", textes[4]); // Saisie format page
	nvl = scanf("%d", &(dd.formatPage));

	Vector2d limitePage = Vector2dSub(formats[dd.formatPage], marge);

	printf("\n%d %s - %d %s\n", 
		dd.nbSommets, textes[5], dd.nbFaces, textes[6]);

	// DEBUT DEPLIAGE
	printf("%s", textes[7]);
	if ((nvl = scanf("%f", &(dd.tailleNums))) < 1)
		dd.tailleNums = 11.0;

	printf("%s", textes[8]); // hauteur
	if ((nvl = scanf("%d", &(dd.hauteurLang))) < 1)
		dd.hauteurLang = 10;

	dd.dispo = calloc(dd.nbFaces, sizeof * dd.dispo);
	for (int i = 0; i < dd.nbFaces; i++)
		dd.dispo[i] = 1;

	dd.lSNA = calloc((size_t)dd.nbFaces * 3, sizeof * dd.lSNA);
	dd.nAff = 0;

	dd.page = calloc(dd.nbFaces, sizeof * dd.page);
	dd.nbP = 0;
	dd.sD = calloc(dd.nbFaces, sizeof * dd.sD); // depliage
	dd.nbD = 0; // nb de faces dépliées
	dd.lignes = calloc((size_t)dd.nbFaces * 3, sizeof * dd.lignes);
	dd.nbL = 0;

	printf("%s", textes[9]); // 1er triangle
	nvl = scanf("%d", &(dd.premierTriangle));
	int tc = dd.premierTriangle;

	
	int gc;
	do
	{
		gc = dd.faces[tc][3];
		int nbTp = 0;
		int tcn = 0;
		dd.page[nbTp++] = tc;
		Depliage sdC = { dd.nbP, tc, -1 };
		dd.sD[dd.nbD++] = sdC;
		dd.dispo[tc] = 0;
		_Bool ok;
		do
		{
			for (int vi = 0; vi < 3; vi++)
			{
				Voisin v = dd.voisins[tc][vi];
				int vc = v.nF;
				ok = dd.dispo[vc] && (dd.faces[vc][3] == gc);
				if (ok)
				{
					for (int i = 0; (i < nbTp) && ok; i++)
					{
						if (dd.page[i] == vc) ok = 0;
					}
				}
				if (ok)
				{	// sauve v2d du voisin
					Vector2d sauveV2dVoisin[3];
					for (int i = 0; i < 3; i++)
						sauveV2dVoisin[i] = dd.v2d[vc][i];
					// 1°) rapproche v2d[vc] de v2d[tc]
					Vector2d deltaV = Vector2dSub(dd.v2d[tc][vi], dd.v2d[vc][v.idx]);
					for (int i = 0; i < 3; i++)
						dd.v2d[vc][i] = Vector2dAdd(dd.v2d[vc][i], deltaV);
					// 2°) tourne
					double a = calcAngle(dd.v2d[tc][vi],
						dd.v2d[tc][suiv(vi)], dd.v2d[vc][prec(v.idx)]);
					for (int i = 0; i < 3; i++)
						dd.v2d[vc][i] = rotation(dd.v2d[tc][vi], dd.v2d[vc][i], a);
					// 3°) test dépassement page
					int nbTV = (nbTp + 1) * 3;
					Vector2d(*tmp) = calloc(nbTV, sizeof * tmp);
					for (int i = 0; i < nbTp; i++)
					{
						for (int j = 0; j < 3; j++)
							tmp[i * 3 + j] = dd.v2d[dd.page[i]][j];
					}
					for (int j = 0; j < 3; j++)
						tmp[nbTp * 3 + j] = dd.v2d[vc][j];
					Vector2d vb[2];
					calcBoiteEnglobante(vb, tmp, nbTV);
					free(tmp);
					Vector2d dV = Vector2dSub(vb[1], vb[0]);
					if ((dV.x > limitePage.x) || (dV.y > limitePage.y))
						ok = 0;
					
					if (ok) // 4°) test collision avec la pièce
					{
						for (int i = 0; (i < nbTp) && ok; i++)
						{
							if (overlap(dd.v2d[dd.page[i]], dd.v2d[vc]))
								ok = 0;
						}
					}
					if (ok) {
						dd.page[nbTp++] = vc;
						Depliage sdC = { dd.nbP, vc, tc, 0 };
						dd.sD[dd.nbD++] = sdC;
						dd.dispo[vc] = 0;
					}
					else
					{
						for (int i = 0; i < 3; i++)
							dd.v2d[vc][i] = sauveV2dVoisin[i];
					}
				}
			}
			// rech prochaine face à déplier
			tcn++;
			if (tcn < nbTp)
			{
				tc = dd.page[tcn];
				ok = 1;
			}
			else
				ok = 0;
		} while (ok);

		Vector2d(*tmp) = calloc((size_t)nbTp * 3, sizeof * tmp);
		for (int i = 0, k = 0; i < nbTp; i++)
		{
			for (int j = 0; j < 3; j++)
			{
				tmp[k++] = dd.v2d[dd.page[i]][j];
			}
		}

		Vector2d b[2];
		calcBoiteEnglobante(b, tmp, nbTp * 3);
		free(tmp);
		// ajustement en bas à gauche
		for (int i = 0; i < nbTp; i++)
		{
			for (int j = 0; j < 3; j++)
				dd.v2d[dd.page[i]][j] =
					Vector2dSub(dd.v2d[dd.page[i]][j], Vector2dSub(b[0], marge));
		}
		// répartition en lignes
		for (int i = 0; i < nbTp; i++)
		{
			for (int j = 0; j < 3; j++)
			{
				dd.lignes[dd.nbL] = LigneNew(
					dd.nbP,
					dd.nbL,
					dd.v2d[dd.page[i]][j],
					dd.v2d[dd.page[i]][suiv(j)],
					dd.page[i],
					j,
					dd.voisins[dd.page[i]][j].nF,
					dd.voisins[dd.page[i]][j].idx);
				dd.nbL++;
			}
		}

		supprimeDoublons(dd.lignes, dd.nbL);

		tc = premDispo(dd.dispo, dd.nbFaces);
		dd.nbP++;
	} while (tc > -1);

	qsort(dd.lSNA, dd.nAff, sizeof(struct sNAff), compAff);
	qsort(dd.lignes, dd.nbL, sizeof(struct sLigne), compPg);

	int lc = 0;
	int nA;
	int typeL;
	int ppc = 0;

	dd.lAN = calloc(dd.nbFaces, sizeof * dd.lAN);
	dd.nbAN = 0;

	dd = init_cairo(dd);
	
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
							Vector2d m = centroid(dd.v2d[sdt.face]);
							afficheNum(dd.cr, sdt.face, m, m, C_BLEU);
						}
					}
				}
				dd.nbAN = 0;
				cairo_show_page(dd.cr);
				ppc++;
			}
			if (l.nb == 1)
				typeL = L_COUPE;
			else {
				double c = dd.tCop[(l.n1 * 3) + l.i1].cop;
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
				faitLigne(dd.cr, l.p1, l.p2, typeL, dd.hauteurLang);
			if (l.nb == 1)
			{
				NAff cleN;
				NAff* rechN;
				cleN.nMax = max(l.n1, l.n2);
				cleN.nMin = min(l.n1, l.n2);
				rechN = (NAff*)bsearch(&cleN, dd.lSNA, dd.nAff, sizeof(NAff), compAff);
				if (rechN != NULL)
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

	sauveDonnees(dd);

	if (dd.nbAN > 0)
	{
		afficheNumsPage(dd.cr, dd.lAN, dd.nbAN, dd.v2d);

		for (int sdi = 0; sdi < dd.nbD; sdi++)
		{
			Depliage sdt = dd.sD[sdi];
			if (sdt.page == ppc)
			{
				Vector2d m = centroid(dd.v2d[sdt.face]);
				afficheNum(dd.cr, sdt.face, m, m, C_BLEU);
			}
		}
	}
	
	// FIN
	closeCairo(dd);
	free(dd.faces);
	free(dd.voisins);
	free(dd.v2d);
	free(dd.v3d);
	free(dd.tCop);
	free(dd.sD);
	free(dd.dispo);
	free(dd.lAN);
	free(dd.lSNA);
	free(dd.page);
	free(dd.lignes);

	return 0;
}
