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

static bool abso = false;
static bool verb = false;
static bool ronly = false;
int (*engine)(int p_nbdl, int p_nbdg, bool p_nexpl) = wqsort;
static int nbworkers = 4;
static int ndmin = 5;
static int ndmax = 55;
static int step = 5;
static char* ofile = "shivresult.csv";

static int parse_opt(int key, char *arg, struct argp_state *state)
{
    switch(key)
    {
	case 'a': abso = true;
	    break;
	case 'v': verb = true;
	    break;
	case 'r': ronly = true;
	    break;
	case 'n': engine = &wnosort;
	    break;
	case 'w': nbworkers = atoi(arg);
	    break; //TODO tester validité
	case 'm': ndmin = atoi(arg);
	    break; //TODO tester validité
	case 'M': ndmax = atoi(arg);
	    break; //TODO tester validité
	case 's': step = atoi(arg);
	    break; //TODO tester validité
	case 'o': ofile = arg;
	    break;
    }
    return 0;
}

int main(int argc, char **argv)//et le nombre de lancers?
{
    //lecture des paramétres
    struct argp_option options[] = {
	{ "absolute", 'a', 0, 0, "ne pas appliquer le bonus de rang aux jets de compétence"},
	{ "nosort", 'n', 0, 0, "utiliser l'algorithme nosort (plus lent, uniquement pour comparaison avec les anciennes versions)"},
	{ "verbose", 'v', 0, 0, "afficher des informations d'avancement du traitement"},
	{ "result-only", 'r', 0, 0, "créée un tableau de résultats moyens plutôt que de chances de réussite"},
	{ "workers", 'w', "NUM", 0, "nombre de workers à créer(par défaut: 4)"},
	{ "ndmin", 'm', "NUM", 0, "ND minimum (par défaut: 5)"},
	{ "ndmax", 'M', "NUM", 0, "ND maximum (par défaut: 55)"},
	{ "step", 's', "NUM", 0, "écart entre chaque ND calculé, le ND max est ignoré si il est dépassé (par défaut: 5)"},
	{ "output", 'o', "STR", 0, "nom du fichier de sortie (par défaut: \"shivresult.csv\")"},
	{ 0}
    };

    struct argp argp = {options, parse_opt};

    argp_parse(&argp, argc, argv, 0, 0, 0);

    //TODO implémenter les options verbose, aboslute et result-only



    int* commtab = malloc(nbworkers * sizeof(commtab));


    //création des processus workers
    for(int i = 0; i < nbworkers; ++i)
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
		workfunc(commelt[1], i, nbworkers, ndmin, ndmax, step);
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
    int nbcol = ((ndmax - ndmin - (ndmax - ndmin) % step) / step) + 1;
    int (*tabresult)[nbcol] = malloc(sizeof(int[NBLINES][nbcol]));

    bool* opnd_strm = malloc(nbworkers * sizeof(opnd_strm)); //tableau des pipes ouverts
    for(int i = 0; i < nbworkers; ++i)
    {
	opnd_strm[i] = true; //TODO vérifier si on ne peut pas optimiser ça
    }
    int lnumber[nbworkers];
    for(int i = 0; i < nbworkers; ++i)
    {
	lnumber[i] = 0;
    }
    bool over = false;
    while(!over)
    {
	over = true;
	for(int i = 0; i < nbworkers; ++i)
	{
	    if(opnd_strm[i])
	    {
		/*int buf;
		switch(read(commtab[i], &buf, sizeof(buf)))*/
		char buf[50];
		int idchld;
		int ND;
		int result;
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
			sscanf(buf, "fils n°%d ND=%d result=%d\n", &idchld, &ND, &result);
			over = false;
			int curcol = (ND - ndmin) / step;
			tabresult[lnumber[i]][curcol] = result;
			printf("fils n°%d result=%d inscrit=%d lnumber=%d curcol=%d\n", idchld, result, tabresult[lnumber[i]][curcol], lnumber[i], curcol);
			if(lnumber[i] < (NBLINES - 1))
			{
			    lnumber[i] = lnumber[i] + 1;
			}
			else
			{
			    lnumber[i] = 0;
			}
		}
	    }
	}
    }



    //écriture du résultat
    FILE * fp;
    fp = fopen(ofile, "w");
    fprintf(fp, "X");
    for(int c = 1; c <= nbcol; ++c)
    {
	fprintf(fp, ",ND%d", ndmin + ((c - 1) * step));
    }
    fprintf(fp, "\n");

    int t = 1;
    int d = 1;
    int c = 0;
    for(int l = 0; l < NBLINES; ++l)
    {
	fprintf(fp, "t%d d%d c%d", t, d, c);
	for(int c = 0; c < nbcol; ++c)
	{
	    fprintf(fp, ",%d", tabresult[l][c]);
	}
	fprintf(fp, "\n");
	if(c < d)
	{
	    ++c;
	}
	else
	{
	    c = 0;
	    if(d < 7)
	    {
		++d;
	    }
	    else
	    {
		d = 1;
		++t;
	    }
	}
    }

    free(opnd_strm);
    free(commtab);
    free(tabresult);

    //récupération des fils morts
    pid_t wpid;
    int status;

    while((wpid = wait(&status)) > 0)
    {//TODO si verbose
	printf("Child PID%d exited with status %d (%s)\n", (int) wpid, status, (status == 0) ? "nominal" : "error");
    }
    return(EXIT_SUCCESS);
}


