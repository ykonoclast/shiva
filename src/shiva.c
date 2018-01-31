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

//valeurs des options par défaut
static bool abso = false;
static bool verb = false;
static bool ronly = false;
int (*engine)(int p_nbdl, int p_nbdg, bool p_nexpl) = wnosort;
static int nbworkers = 4;
static int ndmin = 5;
static int ndmax = 55;
static int step = 5;
static int nbdiceroll = 100000;
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
	case 'q': engine = &wqsort;
	    break;
	case 'd': nbdiceroll = atoi(arg);
	    break;
	case 'w': nbworkers = atoi(arg);
	    break;
	case 'm': ndmin = atoi(arg);
	    break;
	case 'M': ndmax = atoi(arg);
	    break;
	case 's': step = atoi(arg);
	    break;
	case 'o': ofile = arg;
	    break;
    }
    return 0;
}

int main(int argc, char **argv)
{
    clock_t start = clock(), diff;
    //lecture des paramétres
    struct argp_option options[] = {
	{ "absolute", 'a', 0, 0, "ne pas appliquer le bonus de rang aux jets de compétence"},
	{ "qsort", 'q', 0, 0, "utiliser l'algorithme qsort (généralement plus lent)"},
	{ "verbose", 'v', 0, 0, "afficher des informations d'avancement du traitement"},
	{ "result-only", 'r', 0, 0, "créée un tableau de résultats moyens plutôt que de chances de réussite"},
	{ "workers", 'w', "NUM", 0, "nombre de workers à créer(par défaut: 4, ignoré avec result-only)"},
	{ "dicerolls", 'd', "NUM", 0, "nombre de jets par élément statistique (par défaut: 100 000)"},
	{ "ndmin", 'm', "NUM", 0, "ND minimum (par défaut: 5, ignoré avec result-only)"},
	{ "ndmax", 'M', "NUM", 0, "ND maximum (par défaut: 55, ignoré avec result-only)"},
	{ "step", 's', "NUM", 0, "écart entre chaque ND calculé, le ND max est ignoré si il est dépassé (par défaut: 5, ignoré avec result-only)"},
	{ "output", 'o', "STR", 0, "nom du fichier de sortie (par défaut: \"shivresult.csv\")"},
	{ 0}
    };
    struct argp argp = {options, parse_opt};
    argp_parse(&argp, argc, argv, 0, 0, 0);

    if(nbdiceroll < 1 || nbworkers < 1 || ndmin < 1 || ndmax < 1 || step < 1)
    {
	fprintf(stderr, "valeur de paramétre inférieure à 1");
	return(1);
    }

    if(ronly)
    {
	nbworkers = 1;
	ndmax = 1;
	ndmin = 1;
	step = 1;
    }
    if(verb)
    {
	printf("[main]__démarrage: NDmin=%d, NDmax=%d, step=%d, fichier de sortie=%s, nb workers=%d, nb jets par test=%d, %s, %s, %s\n", ndmin, ndmax, step, ofile, nbworkers, nbdiceroll, abso ? "jets absolus" : "jets de compétence", ronly ? "résultats moyens" : "chances de réussite", (engine == &wqsort) ? "qsort" : "nosort");
    }

    //création des processus workers
    if(verb)
    {
	printf("[main]__création des workers\n");
    }

    int* restrict commtab = malloc(nbworkers * sizeof(commtab));
    for(int i = 0; i < nbworkers; ++i)
    {
	//création d'un pipe à lecture non-bloquante
	int commelt[2];
	if(pipe(commelt) < 0)
	{
	    fprintf(stderr, "échec d'un pipe");
	    return(2);
	}
	if(fcntl(commelt[0], F_SETFL, O_NONBLOCK) < 0)
	{
	    fprintf(stderr, "échec d'un fcntl");
	    return(3);
	}

	//création d'un worker
	switch(fork())
	{
	    case 0://fils
		close(commelt[0]); //on ferme la lecture du pipe
		workfunc(commelt[1], i, nbworkers, ndmin, ndmax, step, nbdiceroll, verb, abso, ronly);
		return(EXIT_SUCCESS); //afin de ne pas exécuter le code du père en revenant de workfunc

	    case -1://fork error
		fprintf(stderr, "échec d'un fork");
		return(4);

	    default://père
		close(commelt[1]); //on ferme l'écriture du pipe
		commtab[i] = commelt[0];
	}
    }

    //suite du code du père
    //communication avec les fils et remplissage du tableau de résultats
    int nbcol = ((ndmax - ndmin - (ndmax - ndmin) % step) / step) + 1;
    int (*tabresult)[nbcol] = malloc(sizeof(int[NBLINES][nbcol])); //tableau à double entree pour les résultats
    int* restrict lnumber = calloc(nbworkers, sizeof(lnumber)); //tableau d'avancement en lignes par worker
    bool * restrict opnd_strm = malloc(nbworkers * sizeof(opnd_strm)); //tableau des pipes encore ouverts
    for(int i = 0; i < nbworkers; ++i)
    {
	opnd_strm[i] = true;
    }

    bool over = false;
    while(!over)//tant que tous les workers n'ont pas fermé leur pipe
    {
	over = true; //on part du principe qu'il n'y a rien à lire, si on a tort, on repassera over à false
	for(int i = 0; i < nbworkers; ++i)
	{//parcourt du tableau des pipes
	    if(opnd_strm[i])
	    {//si ce pipe est ouvert
		char buf[50];
		int idchld;
		int ND;
		int result;
		switch(read(commtab[i], &buf, sizeof(buf)))
		{
		    case -1://pipe vide : rien à lire, mais il n'est pas fermé ce n'est donc pas terminé
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

		    case 0://pipe fermé, on ferme la lecture du côté père et on indique de ne plus lire ce pipe
			close(commtab[i]);
			opnd_strm[i] = false;
			break;

		    default://on lit le message et on refait un tour à l'issue
			sscanf(buf, "fils n°%d ND=%d result=%d\n", &idchld, &ND, &result);
			over = false;
			int curcol = (ND - ndmin) / step;
			tabresult[lnumber[i]][curcol] = result;
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
    if(verb)
    {
	printf("[main]__calculs terminés, écriture du fichier de résultats\n");
    }
    FILE * restrict fp;
    fp = fopen(ofile, "w");
    //Ligne d'en-tête
    fprintf(fp, "X");

    if(ronly)
    {
	fprintf(fp, ",Résultat moyen\n");
    }
    else
    {
	for(int c = 1; c <= nbcol; ++c)
	{
	    fprintf(fp, ",ND%d", ndmin + ((c - 1) * step));
	}
	fprintf(fp, "\n");
    }
    //lignes de résultat avec en-têtes de lignes indiquant les valeurs de caracs
    int t = 1;
    int d = 1;
    int c = 0;
    for(int l = 0; l < NBLINES; ++l)
    {
	fprintf(fp, "trait=%d domaine=%d comp=%d", t, d, c);
	for(int c = 0; c < nbcol; ++c)
	{
	    fprintf(fp, ",%d", tabresult[l][c]);
	    if(!ronly)
	    {
		fprintf(fp, "%%");
	    }
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

    //un peu de ménage
    free(opnd_strm);
    free(commtab);
    free(tabresult);
    free(lnumber);

    //récupération des fils morts
    if(verb)
    {
	printf("[main]__récupération des workers\n");
    }
    pid_t wpid;
    int status;
    while((wpid = wait(&status)) > 0)
    {
	if(verb)
	{
	    printf("[main]__worker PID%d terminé sur statut %d (%s)\n", (int) wpid, status, (status == 0) ? "nominal" : "erreur");
	}
    }
    diff = clock() - start;
    int msec = diff * 1000 / CLOCKS_PER_SEC;
    int sec = msec / 1000;
    if(verb)
    {
	printf("[main]__temps total d'exécution:%ds\n", sec);
    }

    return(EXIT_SUCCESS);
}


