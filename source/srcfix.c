/******************************************************************************
 *  SRCFIX.c  --- Source Beautifier for DB/C Source Files                      *
 *  Version 3.10 -- 5/9/94 - REC - Subject, Wills, & Company                   *
 ******************************************************************************/

# include "stdio.h"
# include "ctype.h"
# include "stdlib.h"
# include "string.h"

#ifdef DOS
# define ACCESS _access
# define MKTEMP _mktemp
# define PUTENV _putenv
char *MASK="sfXXXXXX";
# include "io.h"
char SLASH = '\\';
#else
# define ACCESS access
# define MKTEMP tmpnam
# define PUTENV putenv
char  *MASK="sfxxx";
char SLASH = '/';
#endif

char    infilename[1000],outfilename[1000],envfname[1000],srcpath[1000],esrcpath[1000];
char    tab,spc,inputline[1000],outline[1000];
char    wkfname[1000],*xargv[1000],verb[1000],xverb[1000];
char	   line1[1000],line2[1000],line3[1000],line4[1000];
int     envopt,loopswt,lst_spc,lst_tab,splitswt,spcswt,lblsplit,tabstop,xswt,verbswt,xargc;
int     verbose,kill_dbg,ntabs,ovrwrit,disp,cont,cmpdir,indepth,ininc,up_in_place, no_tabs;
long  linecnt,ibytes,obytes;
char  a,b,c,d,e,f,*x,*y,*z,*w,cb_buff[4096];
int   i,j,k,l,m,n,o,p,ok,done;
FILE  *inf,*outf;
char     name[1000],*fname;
char  *dbcsrcpath;

int  carriage_ret = 0x0d;
int  line_feed = 0x0a;

void  getinp();
void  getout();
int   getopts();
#ifdef __STDC__
int   nomore(char *zzz);
#else
int   nomore();
#endif
void  getline();
void  putline();
int  process();
#ifdef __STDC__
void  evalverb(char *xx);
#else
void  evalverb();
#endif
void  copy_temp_back();
int  get_srcpath();
#ifdef __STDC__
void searchpath(char *, char *, char *);
#else
void searchpath();
#endif

#ifdef __STDC__
void main(int argc,char *argv[])
#else
void main(argc,argv)
int argc;
char *argv[];
#endif

{
	memset(wkfname,'\0',1000);
	memset(line1,'\0',1000);
	memset(line2,'\0',1000);
	memset(line3,'\0',1000);
	memset(line4,'\0',1000);
	verbose = 0;
	loopswt = 0;
	lblsplit = 0;
	xswt = 0;
	ininc = 5;
	spcswt = 1;
	cmpdir = 0;
	tabstop = -1;
	ntabs = 3;
	up_in_place = 0;
	kill_dbg = 0;
	envopt = 0;
	no_tabs = 0;

	xargc=argc;

	for (i=0;i<argc;i++)
	{
		xargv[i] = argv[i];
	}

	if ((x = getenv("SRCFIX_OPTS")) != 0)
	{
		envopt = 1;
		y = x;
		j = 0;
		while(*y)
		{
			if (*y == ' ')
			{
				inputline[j] = '\0';
				xargc++;
				xargv[i] = (char *)malloc((strlen(inputline) + 1));
				strcpy(xargv[i],inputline);
				i++;
				j = 0;
			}
			else
			{
				inputline[j] = *y;
				j++;
			}
			y++;
		}
		inputline[j] = '\0';
		xargc++;
		xargv[i] = (char *)malloc((strlen(inputline) + 1));
		strcpy(xargv[i],inputline);
	}



	printf("SRCFIX -- Source Code Beautifier For DB/C Source Code\n");
#ifdef __STDC__
	printf("Subject, Wills & Company -- Version 6.00004 --- %s\n",__DATE__);
#else
	printf("Subject, Wills & Company -- Version 6.00004 --- \n");
#endif

	if (argc < 2)
	{
		printf("\nFormat: srcfix infile outfile <options>\n\n");
		printf("Current Supported Options Are As Follows:\n\n");
		printf(" -O      ---  Overwite Output File if it Exists\n");
		printf(" -In     ---  Spaces per indention (default=5)\n");
		printf(" -W      ---  Wide, insert a tab after the verb instead of a space\n");
		printf(" -C      ---  Indent #IF type compiler directives\n");

		printf(" -L      ---  Turn off Label / Code line splitting\n");
		printf(" -2      ---  Loop style 2, Line up loop control verbs with corresponding loops\n");
		printf(" -Tn     ---  Size of tabstops (default = same as spaces per indent setting)\n");
		printf(" -Dn     ---  Tabstops between label and data declaration or verb,(default = 3)\n");
		printf(" -U      ---  Update in place\n");
		printf(" -Ffile  ---  Temp file name, use with /U\n");
		printf(" -K      ---  kill DEBUG statements\n");
		printf(" -KV     ---  kill DEBUG statements - Verbose\n");
		printf(" -V      ---  Verbose\n");
		printf(" -X      ---  No Tabs all spaces");
		printf("\n Options Can also be specified in the Environment Variable 'SRCFIX_OPTS'\n");
		printf("\n  for example ; SET SRCFIX_OPTS=-U -K\n\n");
		printf("\n");
		exit(1);
	}
	else {
		ovrwrit = 0;
		disp = 0;

		i = getopts();
		if (i) exit(i);

		if (tabstop == -1) tabstop = ininc;
		tab = 9;
		spc = 32;
		done = 0;
		linecnt = 0;

		i = get_srcpath();
		switch(i)
		{
		case 0 :
			printf("No Source Path\n");
			break;
		case 1 :
			printf("Using DBC_SRCPATH from Environment : %s\n",srcpath);
			break;
		case 2 :
			printf("Using DBC_FILEPATH from Environment : %s\n",srcpath);
			break;
		case 3 :
			printf("Using DBC_SRCPATH from Environment file : %s\n",srcpath);
			break;
		case 4 :
			printf("Using DBC_FILEPATH from Environment file : %s\n",srcpath);
			break;
		}
		sprintf(esrcpath,"DBC_SRCPATH=%s",srcpath);
		if (PUTENV(esrcpath) != 0)
		{
			printf("OUT OF ENVIRONMENT SPACE\n");
		}

		while (done == 0)
		{
			ok = 1;
			getinp();
			getout();
			ibytes = 0;
			obytes = 0;
			linecnt = 0;
			cont = 0;
			if (!(ok)) exit(1);
			while ((ok) && (!(feof(inf))))
			{
				getline();
				if (!(feof(inf)) || (inputline[0]))
				{
					if (process() == 1) putline();
					if (kill_dbg == 2) {
						strcpy(line1,line2);
						strcpy(line2,line3);
						strcpy(line3,line4);
						strcpy(line4,inputline);
					}
				}
			}
			ibytes--;
			ibytes--;
			if (ibytes > 0)
			{
				printf("%li -- Lines Processed\n",linecnt);
				printf("%li -- Bytes Read in\n",ibytes);
				printf("%li -- Bytes Written out\n",obytes);
			}
			fclose(outf);
			fclose(inf);
			if (up_in_place == 1)
				copy_temp_back();
			done = 1;
		}
	}
	exit(0);
}

void copy_temp_back()

{
	int     cb_bytes;

	printf("Copying Back Over Original Source File\n");
	if ((inf = fopen(wkfname,"rb")) == NULL)
	{
		printf("\nFatal Error : Unable to re-open temporary file : %s \n",wkfname);
		exit(1);
	}
	if ((outf = fopen(infilename,"wb")) == NULL)
	{
		printf("\nFatal Error : Unable to re-open Original Source file : %s \n",xargv[1]);
		exit(1);
	}
	printf("Outputing to : %s\n",infilename);
	for(;;)
	{
		cb_bytes = fread(cb_buff,1,4096,inf);
		if (cb_bytes) fwrite(cb_buff,1,cb_bytes,outf);
		if (cb_bytes != 4096) break;
	}
	fclose(inf);
	fclose(outf);
	printf("Copy back Complete\n");
	printf("Deleting Temporary File\n");
	if (remove(wkfname) == -1)
	{
		printf("Error Deleting Temp File\n");
		exit(1);
	}
}

void getinp()

{
	searchpath(xargv[1],"DBC_SRCPATH",infilename);
	if (infilename[0] == '\0') strcpy(infilename,xargv[1]);
	if ((inf = fopen(infilename,"r")) == NULL)
	{
		printf("\nUnable to open input file : %s\n\n",infilename);
		exit(1);
	}
	printf("Input File : %s\n",infilename);
}

void  getout()

{
	char *xxx;

	if (up_in_place != 1)
	{
		xxx = xargv[2];
		if (*xxx == '-')
		{
			printf("No Output file Indicated !\n");
			exit(1);
		}
		searchpath(xargv[2],"DBC_SRCPATH",outfilename);
		if (*outfilename == '\0') strcpy(outfilename,xargv[2]);
		if (!(ovrwrit))
		{
			if ((outf = fopen(outfilename,"r")) != NULL)
			{
				printf("\nOutput File : %s \nAlready Exists !! \n\n",outfilename);
				fclose(outf);
				exit(1);
			}
		}
		if ((outf= (fopen(outfilename,"w"))) == NULL)
		{
			printf("\nUnable to Create Output File : %s\n\n",outfilename);
			ok = 0;
			exit(1);
		}
		printf("Output file : %s\n",outfilename);
	}
	else
	{
		if (wkfname[0])
			fname = wkfname;
		else
		{
			strcpy(name,MASK);
			fname = MKTEMP(name);
			strcpy(wkfname,fname);
		}
		printf("Creating Temporary File : %s\n",wkfname);
		if ((outf= (fopen(wkfname,"w"))) == NULL)
		{
			printf("\nUnable to Create Output File : %s\n\n",wkfname);
			exit(1);
		}
	}
}


#ifdef __STDC__
void searchpath(char *fn, char *envvar, char *outpath)
#else
void searchpath(fn, envvar, outpath)
char *fn, *envvar, *outpath;
#endif

{
	char *xpath,*xx,*yy;

	xpath = getenv(envvar);
	xx = xpath;
	while(*xx)
	{
		yy = inputline;
		while((*xx) && (*xx != ';'))
		{
			*yy = *xx;
			xx++;
			yy++;
		}
		yy--;
		if (*yy != SLASH)
		{
			yy++;
			*yy = SLASH;
		}
		yy++;
		*yy = '\0';
		strcat(inputline,fn);
		if (ACCESS(inputline,0) == 0)
		{
			strcpy(outpath,inputline);
			return;
		}
		if (*xx == ';') xx++;
	}
	*outpath = '\0';
	return;
}


int   getopts()

{
	int      retval=0,jj;

	if (envopt != 0)
		printf("SRCFIX_OPTS will be used in addition to command line options\n");

	for (i=2;i<xargc;i++)
	{
		x = xargv[i];
		if (*x == '-')
		{
			x++;
			a = toupper(*x);
			switch(a)
			{
			case 'O' :
			ovrwrit = 1;
			printf("Overwrite Enabled\n");
			break ;
			case 'W' :
				spcswt = 0; 
				printf("Tabs will be inserted following verbs\n");
				break;
			case 'C' :
				cmpdir = 1;
				printf("Compiler directives will be indented\n");
				break;
			case 'U' :
				if (up_in_place == -1)
				{
					printf("Inconsistant Options, Update in place / output file\n\n");
					exit(1);
				}
				else
				{
					printf("Update In Place\n");
					up_in_place = 1;
				}
				break;
			case '2' :
				loopswt = 1;
				printf("Loop style 2\n");
				break;
			case 'L' :
				printf("Labels will not be split from code \n");
				lblsplit = 1;
				break;
			case 'I' :
				x++;
				jj= atoi(x);
				if (jj<1)
				{
					printf("\n\nInvalid format for /I option\n\n");
					retval=1;
				}
				else
				{
					printf("%3i Spaces per Indentation\n",jj);
					ininc = jj;
				}
				break;
			case 'T' :
				x++;
				jj = atoi(x);
				if (jj<1)
				{
					printf("\n\nInvalid format for /T option\n\n");
					retval=1;
				}
				else
				{
					printf("%3i Spaces per Tabstop\n",jj);
					tabstop = jj;
				}
				break;
			case 'F' :
				x++;
				strcpy(wkfname,x);
				printf("Work file name : %s \n",wkfname);
				break;
			case 'D' :
				x++;
				j = atoi(x);
				if (jj<1)
				{
					printf("\n\nInvalid format for /D option\n\n");
					retval=1;
				}
				else
				{
					printf("%3i Tabstops between label and data declaration or verb\n",jj);
					ntabs = jj;
				}
				break;
			case 'K' :
				x++;
				a = toupper(*x);
				if (a != 'V') {
					printf("Kill DEBUG statements\n");
					kill_dbg = 1;
				}
				else {
					printf("Kill DEBUG statements - VERBOSE\n");
					kill_dbg = 2;
				}
				break;
			case 'V' :
				printf("Verbose Mode");
				verbose = 1;
				break;
			case 'X' :
				no_tabs = 1;
				break;
			default  :
				printf("\n\nOption /%s  -  is not defined\n\n",x);
				retval=1;
			}
		}
		else
		{
			if (i != 2)
			{
				printf("\n\nOption /%s  -  is not defined\n\n",x);
				retval=1;
			}
			else up_in_place = -1;
		}
	}
	return(retval);
}



void getline()

{

	int     ptr,eol,eof;
	char    aa;

	ptr = 0;
	eol = 0;
	eof = 0;

	ibytes++;
	while (!((eol) || (eof) || (ptr == 1000)))
	{
		aa = getc(inf);
		ibytes++;
		if (feof(inf)) eof = 1;
		else
		{
			if ((aa == '\n') || (aa == line_feed))
			{
				eol = 1;
				inputline[ptr] = '\0';
			}
			else if (aa != carriage_ret) {
				inputline[ptr] = aa;
				ptr++;
			}
		}
	}
	inputline[ptr] = '\0';
	if (verbose) printf("Line in : [%s]\n",inputline);
	linecnt++;
}



void putline()

{
	int     ptr;

	ptr = 0;

	if (verbose) printf("Line Out: [%s]\n",outline);
	while ((ptr < 1000) && (outline[ptr]))
	{
		putc(outline[ptr],outf);
		obytes++;
		ptr++;
	}
	putc('\n',outf);
	obytes++;
	obytes++;
}



int process()

{
	int     ii,kk,jj,ptri,ptro,vptr,org,k2;
	char    parms[1000],*xx,*yy;

	ptri = 0;
	ptro = 0;

	outline[0] = '\0';

	switch(inputline[0])
	{
	case '*' :
	case '+' :
	case '\0' :
	case '.' :
		strcpy(outline,inputline);
		return(1) ;
	}

	xx = inputline;
	while(*xx)      /* convert tabs to spaces */
	{
		if (*xx == tab) *xx = spc;
		xx++;
	}

	if (cont)
	{
		cont = 0;
		for (i=0;i<=lst_tab;i++)
		{
			if (no_tabs == 0) {
				outline[ptro] = tab;
				ptro++;
			}
			else {
				for (k=0;k<tabstop;k++)
							{
								outline[ptro] = ' ';
								ptro++;
							}
			}
		}
		xx = inputline;
		while ((*xx) && (*xx == spc)) xx++;
		strcpy(&outline[ptro],xx);

		ptro = strlen(outline) - 1;
		while ((ptro > 0) && (outline[ptro] == spc)) ptro--;
		if (outline[ptro] == ':')
		{
			cont = 1;
		}
		return(1);
	}

	ii = 0;
	if (isgraph(inputline[0]))   /* pull label if there */
	{
		while(isgraph(inputline[ptri]))
		{
			outline[ptro] = inputline[ptri];
			ptro++;
			ptri++;
		}
		outline[ptro] = '\0';
	}

	if (nomore(&inputline[ptri])) return(1);

	while (inputline[ptri] == ' ') ptri++;

	verb[0] = '\0';
	ii = 0;
	while(isgraph(inputline[ptri]))
	{
		verb[ii] = inputline[ptri];
		ii++;
		ptri++;
	}
	verb[ii] = '\0';

	parms[0] = '\0';

	if (!(nomore(&inputline[ptri])))
	{
		while (inputline[ptri] == ' ') ptri++;
		strcpy(parms,&inputline[ptri]);
		ii = strlen(parms) - 1;
		while (parms[ii] == ' ')
		{
			parms[ii] = '\0';
			ii--;
		}
	}

	evalverb(verb);

	if ((kill_dbg != 0) && (verbswt == 7)) {
		if (kill_dbg == 2) {
			printf("------\n");
			printf("Debug removed @ line %i\n",linecnt);
			if (line1[0]) printf("%s\n",line1);
			if (line2[0]) printf("%s\n",line2);
			if (line3[0]) printf("%s\n",line3);
			if (line4[0]) printf("%s\n",line4);
			printf("%s\n",inputline);
		}
		return(0);
	}

	switch(verbswt)
	{
	case 3 :
	case 2 :  indepth--;
	break;
	case 5 :  indepth = indepth - 2;
	}
	if (indepth < 0) indepth = 0;

	org = 0;
	if ((splitswt == 0) && (ptro != 0) && (verbswt != 6))   /* with label and split */
	{
		lst_tab = indepth + 1;
		lst_spc = strlen(verb) + 1;
		outline[ptro] = '\n';
		ptro++;
		if (no_tabs == 0) {
			outline[ptro] = tab;
			ptro++;
		} else {
			for (k=0;k<tabstop;k++)
			{
				outline[ptro] = ' ';
				ptro++;
			}
		}

		for(i=0;i<indepth;i++) {
			if (ininc == tabstop && no_tabs == 0)
			{
				outline[ptro] = tab;
				ptro++;
			}
			else
				for (k=0;k<tabstop;k++)
				{
					outline[ptro] = ' ';
					ptro++;
				}
		}
	}
	else if ((ptro == 0) && (verbswt != 6))    /*  no label  */
	{
		lst_tab = indepth + 1;
		lst_spc = strlen(verb) + 1;
		if (no_tabs == 0) {
			outline[ptro] = tab;
			ptro++;
		}
		else {
			for (k=0;k<tabstop;k++)
			{
				outline[ptro] = ' ';
				ptro++;
			}
		}
		for(i=0;i<indepth;i++)
			if (ininc == tabstop && no_tabs == 0)
			{
				outline[ptro] = tab;
				ptro++;
			}
			else {
				for (k=0;k<tabstop;k++)
				{
					outline[ptro] = ' ';
					ptro++;
				}
			}
	}
	else
		if (verbswt != 6)
		{             /*  label , no split  */
			lst_tab = ntabs;
			lst_spc = strlen(verb) + 1;
			switch(splitswt)
			{
			case 1 :
				ii = ntabs * tabstop - 1;
				break;
			case 2 :
				ii = tabstop - 1;
			}
			if (ptro > ii)
			{
				outline[ptro] = ' ';
				ptro++;
			}
			else
			{
				i = ((ii + 1) - ptro) / tabstop;
				if (((ii + 1) - ptro) % tabstop) i++;
				for(k=0;k<i;k++)
				{
					if (no_tabs == 0) {
						outline[ptro] = tab;
						ptro++;
					}
					else {
						for (k2=0;k2<tabstop;k2++)
									{
										outline[ptro] = ' ';
										ptro++;
									}
					}
				}
			}
		}
		else       /* verbswt == 6   */
		{

			i = ntabs;
			for(k=0;k<i;k++)
			{
				if (no_tabs == 0) {
					outline[ptro] = tab;
					ptro++;
				} else {
					for(k2=0;k2<tabstop;k2++)
					{
						outline[ptro] = ' ';
						ptro++;
					}
				}
			}
		}


	strcpy(&outline[ptro],verb);
	while (outline[ptro]) ptro++;

	outline[ptro] = '\0';
	if (parms[0])
	{
		if (spcswt == 1)
			outline[ptro] = ' ';
		else
			outline[ptro] = tab;
		ptro++;
		strcpy(&outline[ptro],parms);
		while (outline[ptro]) ptro++;
	}


	if (indepth < 0)
		indepth = 0;

	outline[ptro] = '\0';

	cont = 0;

	ptro--;

	while ((ptro > 0) && (outline[ptro] == spc)) ptro--;
	if (outline[ptro] == ':')
		cont = 1;

	switch(verbswt)
	{
	case 1 :
	case 3 :
		indepth++;
		break;
	case 4 :
		indepth = indepth + 2;
	}
	return(1);
}



#ifdef __STDC__
int nomore(char *xxx)
#else
int nomore(xxx)
char *xxx;
#endif

{
	while (*xxx == ' ')  xxx++;
	if (*xxx) return(0);
	return(1);
}


int get_srcpath()

{
	int ptype;  /* 0 = none, 1 = env srcpath, 2 = env filepath, 3 = envfile srcpath,
               4 = envfile filepath */
	char *ptr;

	ptype = 0;
	memset(srcpath,'\0',1000);
	if ((ptr = getenv("DBC_SRCPATH")) != 0)
	{
		strcpy(srcpath,ptr);
		return(1);
	}

	if ((ptr = getenv("DBC_FILEPATH")) != 0)
	{
		strcpy(srcpath,ptr);
		ptype = 2;
	}

	if ((ptr = getenv("DBC_ENVFILE")) == 0)
		return(ptype);

	strcpy(envfname,ptr);
	if ((inf = fopen(envfname,"r")) == NULL)
	{
		printf("\nError : Unable to open dbc environment file : %s \n",envfname);
		return(ptype);
	}

	while ((ptype != 3) && (!(feof(inf))))
	{
		getline();
		if (!(feof(inf)) || (inputline[0]))
		{
			ptr = inputline;
			while (*ptr == ' ') ptr++;
			if ((!(strncmp(ptr,"DBC_FILEPATH",12))) && (ptype != 2))
			{
				while((*ptr != '=') && (*ptr != '\0')) ptr++;
				if (*ptr != '\0') ptr++;
				strcpy(srcpath,ptr);
				ptype = 4;
			}
			if (!(strncmp(ptr,"DBC_SRCPATH",11)))
			{
				while((*ptr != '=') && (*ptr != '\0')) ptr++;
				if (*ptr != '\0') ptr++;
				strcpy(srcpath,ptr);
				ptype = 3;
			}
		}
	}
	fclose(inf);
	return(ptype);
}



#ifdef __STDC__
void  evalverb(char *xx)
#else
void  evalverb(xx)
char *xx;
#endif

{
	char    *yy;
	int     ptr;

	yy=xx;
	ptr=0;
	while (*yy)
	{
		xverb[ptr] = toupper(*yy);
		ptr++;
		yy++;
		if (ptr == 1000)
		{
			printf("Problem Evaluating line  :: %li :: %s\n",linecnt,inputline);
			splitswt = 1;
			verbswt = 0;
		}
	}
	xverb[ptr] = '\0';

	verbswt=0;
	splitswt = lblsplit;
	yy = xverb;

	if (*yy == 'G') yy++;

	if (!(strcmp(yy,"EQU"))) splitswt = 1;
	if (!(strcmp(yy,"EQUATE"))) splitswt = 1;
	if (!(strcmp(yy,"DEFINE"))) splitswt = 1;
	if (!(strcmp(yy,"LIST"))) splitswt = 1;
	if (!(strcmp(yy,"VARLIST"))) splitswt = 1;
	if (!(strcmp(yy,"VERB"))) splitswt = 1;
	if (!(strcmp(yy,"CVERB"))) splitswt = 1;
	if (!(strcmp(yy,"CHAR"))) splitswt = 1;
	if (!(strcmp(yy,"DIM"))) splitswt = 1;
	if (!(strcmp(yy,"CHARACTER"))) splitswt = 1;
	if (!(strcmp(yy,"INIT"))) splitswt = 1;
	if (!(strcmp(yy,"NUM"))) splitswt = 1;
	if (!(strcmp(yy,"VAR"))) splitswt = 1;
	if (!(strcmp(yy,"INTEGER"))) splitswt = 1;
	if (!(strcmp(yy,"FLOAT"))) splitswt = 1;
	if (!(strcmp(yy,"FORM"))) splitswt = 1;
	if (!(strcmp(yy,"NUMBER"))) splitswt = 1;
	if (!(strcmp(yy,"INT"))) splitswt = 1;
	if (!(strcmp(yy,"RECORD"))) splitswt = 1;
	if (!(strcmp(yy,"FILE"))) splitswt = 1;
	if (!(strcmp(yy,"IFILE"))) splitswt = 1;
	if (!(strcmp(yy,"AFILE"))) splitswt = 1;
	if (!(strcmp(yy,"PFILE"))) splitswt = 1;
	if (!(strcmp(yy,"COMFILE"))) splitswt = 1;
	if (!(strcmp(yy,"DEVICE"))) splitswt = 1;
	if (!(strcmp(yy,"RESOURCE"))) splitswt = 1;
	if (!(strcmp(yy,"IMAGE"))) splitswt = 1;
	if (!(strcmp(yy,"QUEUE"))) splitswt = 1;
	if (!(strcmp(yy,"LABEL"))) splitswt = 1;
	if (!(strcmp(yy,"OBJECT"))) splitswt = 1;
	if (!(strcmp(yy,"METHOD"))) splitswt = 1;
	if (!(strcmp(yy,"CLASS"))) splitswt = 1;

	if (!(strcmp(yy,"EXTERNAL"))) splitswt = 1;
	if (!(strcmp(yy,"ROUTINE"))) splitswt = 2;
	if (!(strcmp(yy,"LROUTINE"))) splitswt = 2;


	if (!(strcmp(xverb,"IF"))) verbswt=1;
	if (!(strcmp(xverb,"ELSE"))) verbswt=3;
	if (!(strcmp(xverb,"ENDIF"))) verbswt=2;
	if (!(strcmp(xverb,"LOOP"))) verbswt=1;
	if (!(strcmp(xverb,"REPEAT"))) verbswt=2;
	if (loopswt == 1)
	{
		if (!(strcmp(xverb,"WHILE"))) verbswt=3;
		if (!(strcmp(xverb,"UNTIL"))) verbswt=3;
		if (!(strcmp(xverb,"CONTINUE"))) verbswt=3;
		if (!(strcmp(xverb,"BREAK"))) verbswt=3;
	}

	if (!(strcmp(xverb,"LISTEND"))) verbswt=6;
	if (!(strcmp(xverb,"RECORDEND"))) verbswt=6;
	if (!(strcmp(xverb,"CASE"))) verbswt=3;
	if (!(strcmp(xverb,"DEFAULT"))) verbswt=3;
	if (!(strcmp(xverb,"SWITCH"))) verbswt=4;
	if (!(strcmp(xverb,"ENDSWITCH"))) verbswt=5;
	if (!(strcmp(xverb,"FOR"))) verbswt=1;
	if (!(strcmp(xverb,"DEBUG"))) verbswt=7;
	if (cmpdir)
	{
		if (!(strcmp(&xverb[0],"#ENDIF"))) verbswt=2;
		if (!(strcmp(&xverb[0],"#ELSE"))) verbswt=3;
		if (!(strncmp(&xverb[0],"#IF",3))) verbswt=1;
	}
	if (verbose) printf("Verb[%s][%i][%i]\n",xverb,verbswt,splitswt);

}
