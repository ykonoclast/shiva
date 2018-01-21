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

#include "shiva.h"

int (*engine)(int p_nbdl, int p_nbdg, bool p_nexpl) = &T_SORT;

int main(void)
{
    //création des processus workers
    for(size_t i = 0; i < NBCHILDREN; ++i)
    {
	//création d'un pipe à lecture non-bloquante
	int commelt[2];
	if(pipe(commelt) < 0)
	{
	    fprintf(stderr, "échec d'un pipe");
	    return(1);
	}
	if(fcntl(commelt[0], F_SETFL, O_NONBLOCK) < 0)
	{
	    fprintf(stderr, "échec d'un fcntl");
	    return(2);
	}

	//crétion d'un worker
	switch(fork())
	{
	case 0://fils
	    close(commelt[0]); //on ferme la lecture du pipe
	    workfunc(commelt[1], i, NBCHILDREN, 5, 10);
	    return(EXIT_SUCCESS); //afin de ne pas exécuter le code du père en revenant de workfunc

	case -1://fork error
	    fprintf(stderr, "échec d'un fork");
	    return(3);

	default://père
	    close(commelt[1]); //on ferme l'écriture du pipe
	    commtab[i] = commelt[0];
	}
    }

    //suite du code du père
    //communication avec les fils et remplissage du tableau de résultats
    bool* opnd_strm = malloc(NBCHILDREN * sizeof(opnd_strm));
    for(int i = 0; i < NBCHILDREN; ++i)
    {
	opnd_strm[i] = true;
    }
    int itern[NBCHILDREN];
    for(int i = 0; i < NBCHILDREN; ++i)
    {
	itern[i] = 0;
    }
    bool over = false;
    while(!over)
    {
	over = true;
	for(int i = 0; i < NBCHILDREN; ++i)
	{
	    if(opnd_strm[i])
	    {
		/*int buf;
		switch(read(commtab[i], &buf, sizeof(buf)))*/
		char buf[50];
		int idchld;
		int ND;
		int trait;
		int domaine;
		int comp;
		switch(read(commtab[i], &buf, sizeof(buf)))
		{
		case -1://pipe vide
		    over = false;
		    if(errno == EAGAIN)
		    {
			break;
		    }
		    else
		    {
			perror("read");
			return(4);
		    }

		case 0://pipe fermé
		    close(commtab[i]);
		    opnd_strm[i] = false;
		    break;

		default://on lit le message
		    sscanf(buf, "fils n°%d ND=%d trait=%d domaine=%d comp=%d", &idchld, &ND, &trait, &domaine, &comp);
		    over = false;
		    itern[i] = itern[i] + 1;
#ifdef DEBUG
		    printf("Message du fils n°%d(%d) sur itération n°%d: ND:%d, trait:%d, domaine:%d, comp:%d\n", i, idchld, itern[i], ND, trait, domaine, comp);
#endif
		}
	    }
	}
    }

    //récupération des fils morts
    pid_t wpid;
    int status;

    while((wpid = wait(&status)) > 0)
    {//TODO si verbose
	printf("Child PID%d exited with status %d (%s)\n", (int) wpid, status, (status == 0) ? "nominal" : "error");
    }
    return(EXIT_SUCCESS);
}


