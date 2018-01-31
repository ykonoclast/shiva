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

void workfunc(int p_c2p, int id, int nbchld, int mincol, int maxcol, int step)
{
    //TODO commentaire de création si verbose
    if(fcntl(p_c2p, F_SETFL, O_NONBLOCK) < 0)
    {
	fprintf(stderr, "échec d'un fcntl");
	return(2);
    }

    srand(time(NULL)*(id + 1));

    int offset = (id * step) + mincol;
    while(offset <= maxcol)//on parcours les ND affectés à ce worker
    {
	printf("    ==worker%d==>calculs pour ND=%d\n", id, offset); //TODO seulement si verbose
	for(int trait = 1; trait <= 7; ++trait)//pour chaque rang de trait de 1 à 7
	{
	    for(int domaine = 1; domaine <= 7; ++domaine)//pour chaque rang de domaine de 1 à 7
	    {

		for(int comp = 0; comp <= domaine; ++comp)//pour chaque rang de compétence de 0 jusqu'au rang de domaine gouvernant
		{
		    int result = 0;
		    for(int i = 0; i < 1000000; ++i)//TODO valeur de nbdicerolls
		    {
			result += lancer(domaine + comp, trait, false);

		    }
		    result = result / 1000000; //TODO valeur de nbdicerolls

		    char str[50];
		    sprintf(str, "fils n°%d ND=%d result=%d\n", id, offset, result);
		    write(p_c2p, str, sizeof(str));
		}
	    }
	}
	offset += (nbchld * step);
    }
    close(p_c2p);
}