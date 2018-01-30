/*
 * Copyright (C) 2018 ykonoclast
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "uprollutils.h"

int lancer(int p_nbdl, int p_nbdg, bool p_nexpl)
{
    int total = 0;
    if(p_nbdl >= 0 && p_nbdg >= 0)
    {
	//inversion des dés lancés et gardés si nécessaire
	if(p_nbdl < p_nbdg)
	{
	    int temp = p_nbdl;
	    p_nbdl = p_nbdg;
	    p_nbdg = temp;
	}
	total = (*engine)(p_nbdl, p_nbdg, p_nexpl);
    }
    else
    {
	fprintf(stderr, "uprollutils.c - lancer: paramétre aberrant lancés=%d gardés=%d", p_nbdl, p_nbdg);
	exit(5);
    }
    return total;
}

int dieroll(bool p_nexpl)
{
    int result = ((int) (arc4random_uniform(10) + 1));

    while(!p_nexpl && result == 10)
    {//gestion de l'explosion
	result += ((int) (arc4random_uniform(10) + 1));
    }
    return result;
}

int lt_comp(const void* a, const void* b)
{
    return(*((int*) b) - *((int*) a));
}

int wqsort(int p_nbdl, int p_nbdg, bool p_nexpl)
{
    int total = 0;
    int* tabl = calloc(p_nbdl, sizeof(tabl));

    for(int i = 0; i < (p_nbdl); ++i)
    {
	tabl[i] = dieroll(p_nexpl);
    }
    qsort(tabl, p_nbdl, sizeof(*tabl), &lt_comp);
    for(int i = 0; i < (p_nbdg); ++i)
    {
	total += tabl[i];
    }
    free(tabl);
    return total;
}

int wnosort(int p_nbdl, int p_nbdg, bool p_nexpl)
{
    int total = 0;
    int* tabg = calloc(p_nbdg, sizeof(tabg));

    //lancement de chaque dé
    for(int i = 0; i < (p_nbdl); ++i)
    {
	int alea = dieroll(p_nexpl);
	//vérification de si le dé doit être gardé
	int j = 0;
	while(j < p_nbdg && alea > tabg[j])
	{
	    if((j - 1) >= 0)//ce n'est pas le premier dé à garder : on inverse le dé actuellement considéré avec son prédécesseur
	    {
		tabg[j - 1] = tabg[j];
	    }
	    tabg[j] = alea;
	    ++j;
	}
    }
    //somme du jet
    for(int i = 0; i < p_nbdg; i++)
    {
	total += tabg[i];
    }
    free(tabg);
    return total;
}