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

#include "processutils.h"

void workfunc(int p_c2p, int id, int nbchld, int mincol, int maxcol)
{
    //TODO commentaire de création si verbose
    int offset = id + mincol;
    while(offset <= maxcol)//on parcours les ND affectés à ce worker
    {
	for(int trait = 1; trait <= 7; ++trait)//pour chaque rang de trait de 1 à 7
	{
	    for(int domaine = 1; domaine <= 7; ++domaine)//pour chaque rang de domaine de 1 à 7
	    {
		for(int comp = 0; comp <= domaine; ++comp)//pour chaque rang de compétence de 0 jusqu'au rang de domaine gouvernant
		{
		    //int result = offset + comp + domaine + trait/*lancer((int) (arc4random_uniform(10) + 1), (int) (arc4random_uniform(5) + 1), (int) arc4random_uniform(2))*/;
		    //write(p_c2p, &result, sizeof(result));
		    char str[50];
		    sprintf(str, "fils n°%d ND=%d trait=%d domaine=%d comp=%d\n", id, offset, trait, domaine, comp);
		    write(p_c2p, str, sizeof(str));
#ifdef DEBUG
		    printf("DANS FILS:%s\n", str);
#endif
		}
	    }
	}



	offset += nbchld;
    }
    close(p_c2p);
}