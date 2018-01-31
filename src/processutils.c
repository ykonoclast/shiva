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

void workfunc(int p_c2p, int p_id, int p_nbchld, int p_mincol, int p_maxcol, int p_step, int p_nbroll, bool p_verb, bool p_abso, bool p_ronly)
{
    if(p_verb)
    {
	printf("    ==worker%d==>activé PID=%d\n", p_id, getpid());
    }
    if(fcntl(p_c2p, F_SETFL, O_NONBLOCK) < 0)
    {
	fprintf(stderr, "échec d'un fcntl");
	return(2);
    }

    int offset = (p_id * p_step) + p_mincol;
    while(offset <= p_maxcol)//on parcours les ND affectés à ce worker
    {
	if(p_verb)
	{
	    printf("    ==worker%d==>calculs pour ND=%d\n", p_id, offset);
	}
	for(int trait = 1; trait <= 7; ++trait)//pour chaque rang de trait de 1 à 7
	{
	    for(int domaine = 1; domaine <= 7; ++domaine)//pour chaque rang de domaine de 1 à 7
	    {
		for(int comp = 0; comp <= domaine; ++comp)//pour chaque rang de compétence de 0 jusqu'au rang de domaine gouvernant
		{
		    int result = 0;
		    int reussis = 0;
		    int somme = 0;
		    for(int i = 0; i < p_nbroll; ++i)
		    {
			int roll = lancer(domaine + comp, trait, false);
			if(!p_abso)
			{
			    if(comp >= 3)
			    {
				roll += 5;
			    }
			}
			somme += roll;
			if(roll >= offset)
			{
			    ++reussis;
			}
		    }
		    if(!p_ronly)
		    {
			result = (reussis * 100) / p_nbroll;
		    }
		    else
		    {
			result = somme / p_nbroll;
		    }
		    char str[50];
		    sprintf(str, "fils n°%d ND=%d result=%d\n", p_id, offset, result);
		    write(p_c2p, str, sizeof(str));
		}
	    }
	}
	offset += (p_nbchld * p_step);
    }
    close(p_c2p);
}