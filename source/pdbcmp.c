/******************************************************************************
*  pdbcmp - DB/C Precompiler                                                  *
*  Version 1.20000  -- 5/26/96 - REC - Subject, Wills, & Company              *
******************************************************************************/

char *VERSION="1.30008";

# include "stdio.h" 
# include "ctype.h"
# include "stdlib.h"
# include "string.h"
//

#ifdef DOS
# define ACCESS  _access
# define MKTEMP _mktemp
# define PUTENV _putenv
char *MASK="pdXXXXXX";
# include "io.h"
char SLASH = '\\';
#else
# define ACCESS access
# define MKTEMP tmpnam
# define PUTENV putenv
char  *MASK="pdxxx";
char SLASH = '/';
#endif

#define INCLUDE 1
#define DATAVAR 2
#define UVERB 3
#define EXTLAB 4
#define ENDER 5
#define IFDEF 6
#define IFNDEF 7
#define IFLAB 8
#define IFNLAB 9
#define ENDIF 10
#define ELSE 11
#define ROUT 12
#define BRANCH 13
#define GOTO 14
#define TRAP 15
#define CALL 16
#define RETURN 17
#define BGNINDENT 18
#define ENDINDENT 19
#define MOVELAB 20
#define DEBUG 21
#define EQUATE 22
#define IFZ 23
#define IFNZ 24
#define XIF 25
#define LROUT 26
#define ENDROUT 27

#define MAX_HASH 199
typedef struct hlist {
	char *item;
	unsigned char *next;
	} HLIST;
#define hlist_sz sizeof(hlist)

HLIST *variables[MAX_HASH],*proglabels[MAX_HASH],*lrefer[MAX_HASH],*equ_nz[MAX_HASH];
HLIST *entpts[MAX_HASH],*actlabels[MAX_HASH],*uverb[MAX_HASH];
HLIST *uverb_labs[MAX_HASH],*dbcvol[MAX_HASH];
long varcount = 0;
long labelcount = 0;
long lrefercount = 0;
long entcount = 0;
long actlabelcount = 0;

#ifdef __STDC__
void format();
int  get_srcpath();
void searchpath(char *, char *, char *);
void getline(FILE *fp);
void process_src1(char *xx,char *ext);
void process_src2();
void save_lrefer(char *l1,char *l2);
void process_line();
void fixname(char *name);
void process_line1();
void process_line2();
void write_output();
void comment_out(int ii, int kk);
void proc_uverb1(char *xx,char *yy);
void proc_uverb2(char *xx,char *yy);
void trace_labels();
void do_trace(char *);
void check_pcompdir();
void proc_branch(char *zz);
void chk_proc2_cont(char *xx);
void newEntry(char *xx);
int search_hash(HLIST *table[],char *ent,int addflg);
void proc_opt(char *opt);
#else
void format();
int  get_srcpath();
void searchpath();
void getline();
void process_src1();
void process_src2();
void save_lrefer();
void process_line();
void fixname();
void process_line1();
void process_line2();
void write_output();
void comment_out();
void proc_uverb1();
void proc_uverb2();
void trace_labels();
void do_trace();
void check_pcompdir();
void proc_branch();
void chk_proc2_cont();
int search_hash();
void proc_opt();
#endif

int i,cont,branch_cont,uverb_cont,uverb_cont2,proc2_cont;
#define MAX_PARM 100
#define MAX_LABEL 100
#define MAX_VERB 100
char label[MAX_LABEL],verb[MAX_VERB],parm[MAX_PARM];
char lastlabel[MAX_LABEL],inputline[1000],srcpath[1000],envfname[100],wkfname[100];
char outfname[100],lastchar;
char buffer[1000],name[400],name2[400],*fname,reflab[MAX_PARM],sav_uverb[MAX_LABEL];
char bufferx[1000],buffer2[1000];
int cd_flg,indents,verbclass,inside_unused_lroutine;
#define tab 9
FILE *outf,*inf;

int commentout;
int namecase;
int verbose;
int commentopt;
int passopt;
int killdbg;
int ii;

#ifdef __STDC__
void main(int argc,char *argv[])
#else
void main(argc,argv)
int argc;
char *argv[];
#endif

{
char *ptr;

#ifndef __DATE__
#define __DATE__ ""
#endif
printf("pdbcmp - DB/C Language Precompiler\n");
printf("Subject, Wills & Company -- Version %s --- %s\n",VERSION,__DATE__);

namecase = 0;
verbose = 0;
commentopt = 0;
passopt = 0;
killdbg = 0;
wkfname[0] = '\0';
outfname[0] = '\0';

if ((ptr = getenv("DBC_NAMECASE")) != 0)
	{
	if (!(strcmp(ptr,"UPPER"))) namecase = 1;
	if (!(strcmp(ptr,"LOWER"))) namecase = 2;
	}

cont = 0;
cd_flg = 0;
proc2_cont = 0;
indents = 0;
branch_cont = 0;
uverb_cont = 0;
uverb_cont2 = 0;
inside_unused_lroutine = 0;
for(i=0;i<MAX_HASH;i++)
	{
	variables[i] = '\0';
	proglabels[i] = '\0';
	lrefer[i] = '\0';
	entpts[i] = '\0';
	actlabels[i] = '\0';
	uverb[i] = '\0';
	uverb_labs[i] = '\0';
	dbcvol[i] = '\0';
	equ_nz[i] = '\0';
	}

if (argc < 2) format();
ii = 2;
while (ii < argc)
	{
	proc_opt(argv[ii]);
	ii++;
	}

i = get_srcpath();
switch(i)
	{
	case 0 :
		if (verbose) printf("No Source Path\n");
		strcpy(srcpath,".");
		break;
	case 1 :
		if (verbose) printf("Using DBC_SRCPATH from Environment : %s\n",srcpath);
		break;
	case 2 :
		if (verbose) printf("Using DBC_FILEPATH from Environment : %s\n",srcpath);
		break;
	case 3 :
		if (verbose) printf("Using DBC_SRCPATH from Environment file : %s\n",srcpath);
		break;
	case 4 :
		if (verbose) printf("Using DBC_FILEPATH from Environment file : %s\n",srcpath);
		break;
	}

if (passopt)
	{
	if (wkfname[0] != '\0')
		strcpy(buffer,wkfname);
	else
		{
		strcpy(buffer,argv[1]);
		ptr = buffer;
		while ((*ptr) && (*ptr != '.'))
		ptr++;
		strcpy(ptr,".PDB");
		strcpy(wkfname,buffer);
		searchpath(wkfname,srcpath,buffer);
		}
	if (buffer[0] != '\0')
		strcpy(wkfname,buffer);
	}
else {
	if (wkfname[0] == '\0')
		{
		strcpy(name,MASK);
		fname = MKTEMP(name);
		strcpy(wkfname,fname);
		}
	}
fixname(wkfname);
if (verbose) printf("Creating Pass 1 File : %s\n",wkfname);
if ((outf = (fopen(wkfname,"w"))) == NULL)
    	{
     printf("\nUnable to Create File : %s\n\n",wkfname);
	exit(1);
     }

strcpy(lastlabel,"*MAINLINE");
entcount++;
search_hash(entpts,lastlabel,1);
process_src1(argv[1],"PRG");
if (verbose)
	{
	printf("%li Total Variables\n",varcount);
	printf("%li Total Labels\n",labelcount);
	printf("%li Total Entry Points\n",entcount);
	printf("%li Total Label References\n",lrefercount);
	}
fclose(outf);

if (passopt) exit(0);

if (verbose) printf("Tracing labels\n");
trace_labels();

if (verbose) printf("%li Labels Actually used\n",actlabelcount);

if (verbose) printf("Re-Opening Pass 1 File : %s\n",wkfname);
if ((inf = (fopen(wkfname,"r"))) == NULL)
    	{
     printf("\nUnable to Open File : %s\n\n",wkfname);
	exit(1);
     }

if (outfname[0] == '\0')
	{
	strcpy(buffer,argv[1]);
	ptr = buffer;
	while ((*ptr) && (*ptr != '.'))
		ptr++;
	strcpy(ptr,".PDB");
	strcpy(outfname,buffer);
	}
else
	{
	ptr = outfname;
	while ((*ptr) && (*ptr != '.')) ptr++;
	if (*ptr != '.')
		strcpy(ptr,".PDB");
	}

searchpath(outfname,srcpath,buffer);
if (buffer[0] != '\0')
	strcpy(outfname,buffer);
fixname(outfname);
if (verbose) printf("Creating Pass 2 File : %s\n",outfname);
if ((outf = (fopen(outfname,"w"))) == NULL)
    	{
     printf("\nUnable to Create File : %s\n\n",outfname);
	exit(1);
     }

process_src2();
fclose(outf);
fclose(inf);

if (verbose) printf("Deleting Pass 1 File\n");
if (remove(wkfname) == -1)
        {
        printf("Error Deleting Pass 1 File [%s]\n",wkfname);
        exit(1);
        }
}

#ifdef __STDC__
void proc_opt(char *opt)
#else
void proc_opt(opt)
char *opt;
#endif

{
char *xx;

strcpy(buffer,opt);
xx = buffer;
while((*xx) && (*xx != '='))
	{
	*xx = tolower(*xx);
	xx++;
	}
if (!(strcmp(buffer,"-k")))
	{
	printf("Kill DEBUGS\n");
	killdbg = 1;
	return;
	}
if (!(strcmp(buffer,"-v")))
	{
	printf("Verbose Mode\n");
	verbose = 1;
	return;
	}
if (!(strcmp(buffer,"-1")))
	{
	printf("Pass 1 Mode Only\n");
	passopt = 1;
	return;
	}
if (!(strcmp(buffer,"-c1")))
	{
	printf("Comment Option 1, Retain Original Comments\n");
	commentopt = 1;
	return;
	}
if (!(strcmp(buffer,"-c2")))
	{
	printf("Comment Option 2, Retain All Original Source Lines\n");
	commentopt = 2;
	return;
	}
if (!(strncmp(buffer,"-w=",3)))
	{
	xx = buffer;
	xx++;
	xx++;
	xx++;
	strcpy(wkfname,xx);
	return;
	}
if (!(strncmp(buffer,"-o=",3)))
	{
	xx = buffer;
	xx++;
	xx++;
	xx++;
	strcpy(outfname,xx);
	return;
	}
if (!(strncmp(buffer,"-e=",3)))
	{
	xx = buffer;
	xx++;
	xx++;
	xx++;
	search_hash(variables,xx,1);
	search_hash(equ_nz,xx,1);
	printf("%s EQUATED\n",xx);
	return;
	}
printf("\nInvalid Option : %s\n",buffer);
exit(1);
}

void trace_labels()

{
int ptr,ret;
HLIST *cur;

for (ptr=0;ptr<MAX_HASH;ptr++)
	{
	cur = entpts[ptr];
	while (cur)
		{
		do_trace(cur->item);
		cur = (HLIST *)cur->next;
		}
	}
}

#ifdef __STDC__
void do_trace(char *tracex)
#else
void do_trace(tracex)
char *tracex;
#endif

{
int ret;
long htot;
int hptr,slen;
char *ptr;
char buffer[100];
HLIST *cur;

ret = search_hash(actlabels,tracex,1);
if (ret == 2) 
	actlabelcount++;
else return;

if (verbose) printf("Label Reachable : %s\n",tracex);
sprintf(buffer,"%-5s*",tracex);
ptr = buffer;
while(*ptr)
	{
	if (*ptr == ' ') *ptr = '*';
	ptr++;
	}

htot = 0;
ptr = buffer;
for(hptr=0;hptr<5;hptr++)
	if (*ptr != '\0')
		{
		htot = htot + (hptr + 1) * (int)(*ptr);
		ptr++;
		}
hptr = htot % MAX_HASH;

cur = lrefer[hptr];
slen = strlen(tracex);
if (slen < 5) slen = 5;
slen++;
while(cur)
	{
	if (!(strncmp(buffer,cur->item,slen)))
		{
		ptr = cur->item;
		ptr++;
		while (*ptr != '*') ptr++;
		while (*ptr == '*') ptr++;
		do_trace(ptr);
		}
	cur=(HLIST *)cur->next;
	}
}

int get_srcpath()

{
int ptype;  /* 0 = none, 1 = env srcpath, 2 = env filepath, 3 = envfile srcpath,
               4 = envfile filepath */
char *ptr;
FILE *envf;

ptype = 0;
memset(srcpath,'\0',1000);
if ((ptr = getenv("DBC_SRCPATH")) != 0)
	{
	strcpy(srcpath,ptr);
	ptype = 1;
	}

if (((ptr = getenv("DBC_FILEPATH")) != 0) && (ptype == 0))
	{
	strcpy(srcpath,ptr);
	ptype = 2;
	}

if ((ptr = getenv("DBC_ENVFILE")) == 0)
	return(ptype);

strcpy(envfname,ptr);
if ((envf = fopen(envfname,"r")) == NULL)
        {
        printf("\nError : Unable to open dbc environment file : %s \n",envfname);
        return(ptype);
        }

while (!(feof(envf)))
        {
        getline(envf);
        if (!(feof(envf)))
              	{ 
 /*printf("%s\n",inputline);*/
			ptr = inputline;
			while (*ptr == ' ') ptr++;
			if ((!(strncmp(ptr,"DBC_FILEPATH",12))) && (ptype == 0))
				{
				while((*ptr != '=') && (*ptr != '\0')) ptr++;
				if (*ptr != '\0') ptr++;
				strcpy(srcpath,ptr);
				ptype = 4;
				}
			if ((!(strncmp(ptr,"DBC_SRCPATH",11))) && 
				(ptype != 1))
				{
				while((*ptr != '=') && (*ptr != '\0')) ptr++;
				if (*ptr != '\0') ptr++;
				strcpy(srcpath,ptr);
				ptype = 3;
				}
			if (!(strncmp(ptr,"DBCVOL_",7)))
				{
				while (*ptr != '_') ptr++;
				ptr++;
				strcpy(buffer,ptr);
				while ((*ptr != '=') && (*ptr != '\0')) ptr++;
				if (*ptr == '=')
					{
					ptr++;
					strcpy(buffer2,ptr);
					ptr = buffer;
					while (*ptr != '=') ptr++;
					*ptr = '\0';
					sprintf(bufferx,"%-5s*%s",buffer,buffer2);
					ptr = bufferx;
					while(*ptr)
						{
						if (*ptr == ' ') *ptr= '*';
						ptr++;
						}
					search_hash(dbcvol,bufferx,1);
					}
				}
			if (namecase == 0)
				{
				if (!(strncmp(ptr,"DBC_NAMECASE=UPPER",20))) namecase = 1;
				if (!(strncmp(ptr,"DBC_NAMECASE=LOWER",20))) namecase = 2;
				}
               }
        }
fclose(envf);
return(ptype);
}

#ifdef __STDC__
void process_src1(char *sname, char *defext)
#else
void process_src1(sname,defext)
char *sname, *defext;
#endif

{
FILE *srcf;
char *ptr,*cptr,bufx[100];
int ext,colon;


ptr = sname;
ext = 0;
colon = -1;
ii = 0;

while (*ptr)
	{
	if ((*ptr == ' ') || (*ptr == tab))
		{
		*ptr = '\0';
		break;
		}
	if (*ptr == ':') 
		{
		colon = ii;
		cptr = ptr;
		}
	if (*ptr == '.')
		ext = 1;	
	else
		if (ext == 1)
			ext = 2;
	ptr++;
	ii++;
	}

if (ext == 2)
	sprintf(name,"%s",sname);
else
	if (colon < 2)
		sprintf(name,"%s.%s",sname,defext);
	else
		{
		strcpy(bufx,cptr);
		*cptr = '\0';
		sprintf(name,"%s.%s%s",sname,defext,bufx);
		}

fixname(name);
searchpath(name,srcpath,name2);

if (name2[0] == '\0')
	{
	printf("PDBCMP ERROR - SOURCE FILE : %s NOT FOUND\n\n",name);
	fprintf(outf,"*------------------------------------------------------------------------------\n");
	fprintf(outf,"* pdbcmp exception, include file not found\n");
	fprintf(outf," INC %s\n",name);
	fprintf(outf,"*------------------------------------------------------------------------------\n");
	return;
	}

if (verbose) printf("Processing Source File : %s\n",name2);

if ((srcf = fopen(name2,"r")) == NULL)
	{
	printf("\nUnable to Open Source File : %s\n\n",name2);
	exit(1);
	}

if (commentopt == 2)
	{
	fprintf(outf,"*------------------------------------------------------------------------------\n");
	fprintf(outf,"* pdbcmp => source file : %s\n",name2);
	fprintf(outf,"*------------------------------------------------------------------------------\n");
	}

while (!(feof(srcf)))
	{
	getline(srcf);
	if (!(feof(srcf)) || (inputline[0]))
		 process_line1();
	}

fclose(srcf);
if (commentopt == 2)
	{
	fprintf(outf,"*------------------------------------------------------------------------------\n");
	fprintf(outf,"* pdbcmp => end of source\n");
	fprintf(outf,"*------------------------------------------------------------------------------\n");
	}
}

void process_src2()

{
char *ptr;

strcpy(lastlabel,"*MAINLINE");
while (!(feof(inf)))
	{
	getline(inf);
	if (!(feof(inf)) || (inputline[0]))
		 process_line2();
	}
}


void process_line1()

{
int inc,ptr,ptr2,stop,no_cd;
char *xx;

no_cd = 0;
commentout = 0;

switch(inputline[0])
	{
	case '*' :
		check_pcompdir();
     case '+' :
     case '\0' :
     case '.' : 
		if (commentopt) write_output();
		return;
	}

process_line();

inc = 0;

if (branch_cont)
	{
	branch_cont = 0;
	proc_branch(verb);
	}
if (uverb_cont)
	{
	uverb_cont = 0;
	proc_uverb1(sav_uverb,verb);
	proc_uverb1(sav_uverb,parm);
	no_cd = 1;
	}
if (uverb_cont2)
	{
	uverb_cont2 = 0;
	proc_uverb2(sav_uverb,verb);
	proc_uverb2(sav_uverb,parm);
	no_cd = 1;
	}
if (cont == 1) no_cd = 1;
if (cd_flg == 0)
	{
	if (verbclass == 1) /* include statements */
		{
		comment_out(1,2);
		inc = 1;
		}
	if (verbclass == DEBUG)
		if (killdbg == 1)
			comment_out(1,8);
	
	if ((verbclass == DATAVAR) || (verbclass == EQUATE))
		{
		stop = search_hash(variables,label,1);
		if (stop == 2) varcount++;
		if (verbclass == EQUATE)
			{
			stop = atoi(parm);
			if (stop != 0) search_hash(equ_nz,label,1);
			}
		}
	else		
		if (verbclass == UVERB)
			{
			stop = search_hash(uverb,label,1);
			strcpy(sav_uverb,label);
			proc_uverb1(label,parm);
			}
		else
			if (label[0] != '\0')
				if (verbclass != EXTLAB)
					{	
					stop = search_hash(proglabels,label,1);
				/*	printf("label: %s\n",label); */
					if (stop == 2) labelcount++;
					if (lastlabel[0] != '\0')
						save_lrefer(lastlabel,label);
					strcpy(lastlabel,label);
					}
				else
					{	
					stop = search_hash(proglabels,label,1);
					if (stop == 2) labelcount++;
					}
			else
				if ((no_cd == 0) && (verbclass != ENDER && verbclass != ENDROUT) &&
				    (lastlabel[0] == '\0') && (verb[0] != '#'))
						comment_out(1,1);
		if (verbclass == 6)
			{
			comment_out(1,3);
			stop = search_hash(variables,parm,0);
			if (stop == 0) cd_flg = 1;
			}
		if (verbclass == 7)
			{
			comment_out(1,4);
			stop = search_hash(variables,parm,0);
			if (stop == 1) cd_flg = 1;
			}
		if (verbclass == IFLAB)
			{
			comment_out(1,5);
			stop = search_hash(proglabels,parm,0);
			if (stop == 0) 
				{
			/* out 7/8/99	stop = search_hash(variables,parm,0); */
				if (stop == 0) cd_flg = 1;
				}
			}
		if (verbclass == IFNLAB)
			{
			comment_out(1,6);
			stop = search_hash(proglabels,parm,0);
		/* out 7/8/99	if (stop != 1) 
				stop = search_hash(variables,parm,0); */
			if (stop == 1) cd_flg = 1; 
			}
		if (verbclass == IFZ)
			{
			comment_out(1,6);
			stop = search_hash(equ_nz,parm,0);
			if (stop == 1) cd_flg = 1;
			}
		if (verbclass == IFNZ)
			{
			comment_out(1,6);
			stop = search_hash(equ_nz,parm,0);
			if (stop != 1) cd_flg = 1;
			}
		if (verbclass == 10)
			comment_out(1,7);
		if (verbclass == 11)
			{
			comment_out(1,7);
			cd_flg = 1;
			}
		if (verbclass == ROUT)
			{
			stop = search_hash(entpts,label,1);
			if (stop == 2) newEntry(label);
			}
		if (verbclass == BRANCH)
			{
			xx = parm;
			while ((*xx != ' ') && (*xx != ',') && (*xx != tab) && (*xx != '\0'))
				xx++;
			if (*xx == ',')
				xx++;
			else
				{
				xx++;
				while ((*xx == ' ') || (*xx ==  tab)) xx++;
				while ((*xx != ' ') && (*xx != ',') && (*xx != tab) && (*xx != '\0'))
					xx++;
				while ((*xx == ' ') || (*xx ==  tab)) xx++;
				}
			proc_branch(xx);
			}
		if (verbclass == MOVELAB)
			{
			strcpy(reflab,parm);
			xx = reflab;
			while ((*xx != ' ') && (*xx != tab) && (*xx != '\0')) xx++;
			*xx = '\0';
			stop = search_hash(entpts,reflab,1);
			if (stop == 2) newEntry(reflab);
			
	
			}
		if ((verbclass == 14) ||
		    (verbclass == 15) ||
		    (verbclass == CALL))
			{
			strcpy(reflab,parm);
			ptr = 0;
			while (reflab[ptr])
				switch(reflab[ptr])
				{
				case ' ':
				case tab:
				case ',':
					reflab[ptr] = '\0';
					break;
				default:
					ptr++;
				}

			save_lrefer(lastlabel,reflab);
			if (verbclass == 14)
				{
				if (indents == 0)
					{
					ptr2=ptr;
					while(parm[ptr2])
						{
						parm[ptr2] = toupper(parm[ptr2]);
						ptr2++;
						}
					stop = 0;
					while((parm[ptr]) && (!(stop)))
						switch(parm[ptr])
							{
							case ' ':
							case tab :
								ptr++;
								break;
							default :
								if ((parm[ptr] == 'I') &&
								    (parm[ptr + 1] == 'F') &&
								    ((parm[ptr + 2] == ' ') || (parm[ptr + 2] == tab)))
									stop = 1;
								else stop = 2;
							}
					if (stop != 1)
						{
						strcpy(lastlabel,"");
						}
					}
				}
				xx = parm;
				while ((*xx != ' ') && (*xx != tab) && (*xx != '\0')) xx++;
				if (*xx != '\0') {
					while ((*xx != ' ') && (*xx != tab) && (*xx != '\0')) xx++;
						if (*xx != '\0') proc_branch(xx);
				}
			}
		if ((verbclass == 17) && (indents == 0))
			if ((parm[0] != 'I') ||
			    (parm[1] != 'F') ||
			    ((parm[2] != ' ') && (parm[2] != tab)))
				{
				strcpy(lastlabel,"");
				}
		if (search_hash(uverb,verb,0) == 1)
			{
			strcpy(sav_uverb,verb);
			proc_uverb2(verb,parm);
			if (lastlabel[0] != '\0')
				save_lrefer(lastlabel,verb);
			}
		if (verbclass == 18) indents++;
		if ((verbclass == 19) && (indents > 0)) indents--;
		}
else
	{
	if (verbclass == 11)
		if (cd_flg == 1) cd_flg = 0;
	if (verbclass == 10)
		cd_flg--;
	if (verbclass == 6)
		cd_flg++;
	if (verbclass == 7)
		cd_flg++;
	if (verbclass == IFLAB)
		cd_flg++;
	if (verbclass == IFNLAB)
		cd_flg++;
	if (verbclass == IFNZ)
		cd_flg++;
	if (verbclass == XIF)
		cd_flg = 0;
	comment_out(1,8);
	}

cont = 0;
if (commentout != 0) {
	uverb_cont = 0;
	uverb_cont2 = 0;
}
if ((commentout == 0) || (commentopt == 2)) 
	{
	write_output();
	if (lastchar == ':')
		cont = 1;
	}		
if (inc == 1)
	process_src1(parm,"TXT");
return;
}


void process_line()

{
int ptr,ptr2,stop,inQuotes;

ptr = 0;
ptr2 = 0;
stop = 0;
inQuotes = 0;

while ((inputline[ptr]) && (!(stop)))
	{
	switch(inputline[ptr])
		{
		case ' ' :
		case tab :
			stop = 1;
			ptr++;
			break;
		default:
			label[ptr2] = inputline[ptr];
			ptr++;
			ptr2++;
		}
	if (ptr2 == MAX_LABEL)
		{
		stop = 1;
		ptr2--;
		}
	}
label[ptr2] = '\0';

stop = 0;
inQuotes = 0;
while ((inputline[ptr]) && (!(stop)))
	{
	switch(inputline[ptr])
		{
		case ' ' :
		case tab :
			ptr++;
			break;
		default:
			stop = 1;
		}
	}

ptr2 = 0;
stop = 0;
while ((inputline[ptr]) && (!(stop)))
	{
	switch(inputline[ptr])
		{
		case '"' :
			if (inQuotes) inQuotes = 0;
			else inQuotes = 1;
			verb[ptr2] = inputline[ptr];
			ptr++;
			ptr2++;
			break;
		case ' ' :
		case tab :
			if (inQuotes == 0) {
				stop = 1;
				ptr++;
				break;
			}
		default:
			verb[ptr2] = inputline[ptr];
			ptr++;
			ptr2++;
		}
	if (ptr2 == MAX_VERB)
		{
		stop = 1;
		ptr2--;
		}
	}
verb[ptr2] = '\0';

stop = 0;
while ((inputline[ptr]) && (!(stop)))
	{
	switch(inputline[ptr])
		{
		case ' ' :
		case tab :
			ptr++;
			break;
		default:
			stop = 1;
		}
	}

ptr2 = 0;
stop = 0;
while ((inputline[ptr]) && (!(stop)))
	{
	parm[ptr2] = inputline[ptr];
	ptr++;
	ptr2++;
	if (ptr2 == MAX_PARM) 
		{
		ptr2--;
		stop = 1;
		}
	}
parm[ptr2] = '\0';
while((ptr2 != 0) && ((parm[ptr2] == ' ') || (parm[ptr2] == tab) || (parm[ptr2] == '\0')))
	{
	parm[ptr2] = '\0';
	ptr2--;
	}


ptr = 0;
while (verb[ptr])
	{
	verb[ptr] = toupper(verb[ptr]);
	ptr++;
	}

verbclass = -1;

switch(verb[0])
	{
	case 'A' :
		if (!(strcmp(verb,"AFILE"))) 
			{
			verbclass = DATAVAR;
			return;
			}
		break;
	case 'B' :
		if (!(strcmp(verb,"BRANCH")))
			{
			verbclass = BRANCH;
			return;
			}
		break;
	case 'C' :
		if (!(strcmp(verb,"CALL")))
			{
			verbclass = CALL;
			return;
			}
		if (!(strcmp(verb,"CHAR"))) 
			{
			verbclass = DATAVAR;
			return;
			}
		if (!(strcmp(verb,"CHARACTER"))) 
			{
			verbclass = DATAVAR;
			return;
			}
		if (!(strcmp(verb,"CLASS"))) 
			{
			verbclass = DATAVAR;
			return;
			}
		if (!(strcmp(verb,"COMFILE"))) 
			{
			verbclass = DATAVAR;
			return;
			}
		if (!(strcmp(verb,"CVERB"))) 
			{
			verbclass = UVERB;
			return;
			}
		break;
	case 'D' :
		if (!(strcmp(verb,"DIM"))) 
			{
			verbclass = DATAVAR;
			return;
			}
		if (!(strcmp(verb,"DEFINE"))) 
			{
			verbclass = DATAVAR;
			return;
			}
		if (!(strcmp(verb,"DEVICE"))) 
			{
			verbclass = DATAVAR;
			return;
			}
		if (!(strcmp(verb,"DEBUG"))) 
			{
			verbclass = DEBUG;
			return;
			}
		break;
	case 'E' :
		if (!(strcmp(verb,"ENDIF")))
			{
			verbclass = ENDINDENT;
			return;	
			}
		if (!(strcmp(verb,"EQU")))
			{
			verbclass = EQUATE;
			break;
			}
		if (!(strcmp(verb,"EQUATE")))
			{
			verbclass = EQUATE;
			break;
			}
		if (!(strcmp(verb,"EXTERNAL")))
			{
			verbclass = EXTLAB;
			break;
			}
		if (!(strcmp(verb,"ENDSWITCH")))
			{
			verbclass = ENDINDENT;
			return;	
			}
		if (!(strcmp(verb,"ENDROUTINE")))
			{
			verbclass = ENDROUT;
			break;
			}
		if (!(strcmp(verb,"ENDCLASS")))
			{
			verbclass = ENDER;
			break;
			}
		break;
	case 'F' :
    		if (!(strcmp(verb,"FORM"))) 
			{
			verbclass = DATAVAR;
			return;
			}
		if (!(strcmp(verb,"FLOAT"))) 
			{
			verbclass = DATAVAR;
			return;
			}
		if (!(strcmp(verb,"FILE"))) 
			{
			verbclass = DATAVAR;
			return;
			}
		if (!(strcmp(verb,"FOR")))
			{
			verbclass = BGNINDENT;
			return;	
			}
		break;
	case 'G' :
		if (!(strcmp(verb,"GOTO")))
			{
			verbclass = GOTO;
			return;
			}
		if (!(strcmp(verb,"GDIM"))) 
			{
			verbclass = DATAVAR;
			return;
			}
    		if (!(strcmp(verb,"GFORM"))) 
			{
			verbclass = DATAVAR;
			return;
			}
		if (!(strcmp(verb,"GLIST"))) 
			{
			verbclass = DATAVAR;
			return;
			}
		if (!(strcmp(verb,"GVARLIST"))) 
			{
			verbclass = DATAVAR;
			return;
			}
		if (!(strcmp(verb,"GCHAR"))) 
			{
			verbclass = DATAVAR;
			return;
			}
		if (!(strcmp(verb,"GCHARACTER"))) 
			{
			verbclass = DATAVAR;
			return;
			}
		if (!(strcmp(verb,"GINIT"))) 
			{
			verbclass = DATAVAR;
			return;
			}
		if (!(strcmp(verb,"GNUM"))) 
			{
			verbclass = DATAVAR;
			return;
			}
		if (!(strcmp(verb,"GVAR"))) 
			{
			verbclass = DATAVAR;
			return;
			}
		if (!(strcmp(verb,"GINTEGER"))) 
			{
			verbclass = DATAVAR;
			return;
			}
		if (!(strcmp(verb,"GFLOAT"))) 
			{
			verbclass = DATAVAR;
			return;
			}
		if (!(strcmp(verb,"GNUMBER"))) 
			{
			verbclass = DATAVAR;
			return;
			}
		if (!(strcmp(verb,"GINT"))) 
			{
			verbclass = DATAVAR;
			return;
			}
		if (!(strcmp(verb,"GRECORD"))) 
			{
			verbclass = DATAVAR;
			return;
			}
		if (!(strcmp(verb,"GFILE"))) 
			{
			verbclass = DATAVAR;
			return;
			}
		if (!(strcmp(verb,"GIFILE"))) 
			{
			verbclass = DATAVAR;
			return;
			}
		if (!(strcmp(verb,"GLABEL"))) 
			{
			verbclass = DATAVAR;
			return;
			}
		if (!(strcmp(verb,"GAFILE"))) 
			{
			verbclass = DATAVAR;
			return;
			}
		if (!(strcmp(verb,"GPFILE"))) 
			{
			verbclass = DATAVAR;
			return;
			}
		if (!(strcmp(verb,"GCOMFILE"))) 
			{
			verbclass = DATAVAR;
			return;
			}
		if (!(strcmp(verb,"GDEVICE"))) 
			{
			verbclass = DATAVAR;
			return;
			}
		if (!(strcmp(verb,"GRESOURCE"))) 
			{
			verbclass = DATAVAR;
			return;
			}
		if (!(strcmp(verb,"GIMAGE"))) 
			{
			verbclass = DATAVAR;
			return;
			}
		if (!(strcmp(verb,"GQUEUE")))
			{
			verbclass = DATAVAR;
			return;
			}
		break;
	case 'H' :
		break;
	case 'I' :
		if (!(strcmp(verb,"IF")))
			{
			verbclass = BGNINDENT;
			return;	
			}
		if ((!(strcmp(verb,"INC"))) || (!(strcmp(verb,"INCLUDE"))))
			{
			verbclass = INCLUDE;
			return;
			}
		if (!(strcmp(verb,"INIT"))) 
			{
			verbclass = DATAVAR;
			return;
			}
		if (!(strcmp(verb,"INTEGER"))) 
			{
			verbclass = DATAVAR;
			return;
			}
		if (!(strcmp(verb,"INT"))) 
			{
			verbclass = DATAVAR;
			return;
			}
		if (!(strcmp(verb,"IFILE"))) 
			{
			verbclass = DATAVAR;
			return;
			}
		if (!(strcmp(verb,"IMAGE"))) 
			{
			verbclass = DATAVAR;
			return;
			}
		break;
	case 'J' :
		break;
	case 'K' :
		break;
	case 'L' :
		if (!(strcmp(verb,"LOOP")))
			{
			verbclass = BGNINDENT;
			return;	
			}
		if (!(strcmp(verb,"LIST"))) 
			{
			verbclass = DATAVAR;
			return;
			}
		if (!(strcmp(verb,"LISTEND"))) 
			{
			verbclass = ENDER;
			return;
			}
		if (!(strcmp(verb,"LROUTINE")))
			{
			verbclass = LROUT;
			return;
			}
		if (!(strcmp(verb,"LOADLABEL"))) {
			verbclass = BRANCH;
			return;
			}
		if (!(strcmp(verb,"LABEL"))) 
			{
			verbclass = DATAVAR;
			return;
			}
		break;
	case 'M' :
		if (!(strcmp(verb,"MOVELABEL")))
			{
			verbclass = MOVELAB;
			return;
			}
		if (!(strcmp(verb,"MOVELV")))
			{
			verbclass = MOVELAB;
			return;
			}
		break;
	case 'N' :
		if (!(strcmp(verb,"NUMBER"))) 
			{
			verbclass = DATAVAR;
			return;
			}
		if (!(strcmp(verb,"NUM"))) 
			{
			verbclass = DATAVAR;
			return;
			}
		break;
	case 'O' :
		if (!(strcmp(verb,"OBJECT"))) 
			{
			verbclass = DATAVAR;
			return;
			}
		break;
	case 'P' :
		if (!(strcmp(verb,"PFILE"))) 
			{
			verbclass = DATAVAR;
			return;
			}
		if (!(strcmp(verb,"PERFORM")))
			{
			verbclass = BRANCH;
			return;
			}
		break;
	case 'Q' :
		if (!(strcmp(verb,"QUEUE"))) 
			{
			verbclass = DATAVAR;
			return;
			}
		break;
	case 'R' :
		if (!(strcmp(verb,"RETURN")))
			{
			verbclass = RETURN;
			return;
			}
		if (!(strcmp(verb,"REPEAT")))
			{
			verbclass = ENDINDENT;
			return;	
			}
		if (!(strcmp(verb,"RECORD"))) 
			{
			verbclass = DATAVAR;
			return;
			}
		if (!(strcmp(verb,"RESOURCE"))) 
			{
			verbclass = DATAVAR;
			return;
			}
		if (!(strcmp(verb,"ROUTINE")))
			{
			verbclass = ROUT;
			return;
			}

		if (!(strcmp(verb,"RECORDEND"))) 
			{
			verbclass = ENDER;
			return;
			}
		break;
	case 'S' :
		if (!(strcmp(verb,"SWITCH")))
			{
			verbclass = BGNINDENT;
			return;	
			}
		break;
	case 'T' :
		if (!(strcmp(verb,"TRAP")))
			{
			verbclass = TRAP;
			return;
			}
		break;
	case 'U' :
		break;
	case 'V' :
		if (!(strcmp(verb,"VARLIST"))) 
			{
			verbclass = DATAVAR;
			return;
			}
		if (!(strcmp(verb,"VAR"))) 
			{
			verbclass = DATAVAR;
			return;
			}
		if (!(strcmp(verb,"VERB"))) 
			{
			verbclass = UVERB;
			return;
			}
		break;
	case 'W' :
		break;
	case 'X' :
		break;
	case 'Y' :
		break;
	case 'Z' :
		break;
	case '#' :
		if (!(strcmp(verb,"#IFDEF")))
			{
			verbclass = IFDEF;
			return;
			}
		if (!(strcmp(verb,"#IFNDEF")))
			{
			verbclass = IFNDEF;
			return;
			}
		if (!(strcmp(verb,"#IFLABEL")))
			{
			verbclass = IFLAB;
			return;
			}
		if (!(strcmp(verb,"#IFNLABEL")))
			{
			verbclass = IFNLAB;
			return;
			}
		if (!(strcmp(verb,"#ENDIF")))
			{
			verbclass = ENDIF;
			return;
			}
		if (!(strcmp(verb,"#ELSE")))
			{
			verbclass = ELSE;
			return;
			}
		if (!(strcmp(verb,"#IFZ")))
			{
			verbclass = IFZ;
			return;
			}
		if (!(strcmp(verb,"#IFNZ")))
			{
			verbclass = IFNZ;
			return;
			}
		if (!(strcmp(verb,"#XIF")))
			{
			verbclass = XIF;
			return;
			}
		printf("Unknown Compiler Directive [%s]\n",inputline);
		break;
	case '%' :
		if (!(strcmp(verb,"%IFDEF")))
			{
			verbclass = IFDEF;
			return;
			}
		if (!(strcmp(verb,"%IFNDEF")))
			{
			verbclass = IFNDEF;
			return;
			}
		if (!(strcmp(verb,"%IFLABEL")))
			{
			verbclass = IFLAB;
			return;
			}
		if (!(strcmp(verb,"%IFNLABEL")))
			{
			verbclass = IFNLAB;
			return;
			}
		if (!(strcmp(verb,"%ENDIF")))
			{
			verbclass = ENDIF;
			return;
			}
		if (!(strcmp(verb,"%ELSE")))
			{
			verbclass = ELSE;
			return;
			}
		printf("Unknown Compiler Directive [%s]\n",inputline);
		break;
	}
//	printf("[%s][%s][%s]\n",inputline,verb,parm);
}


void check_pcompdir()

{
char *ptr,*ptr2;
int ret;

strcpy(buffer,inputline);
ptr = buffer;
while ((*ptr) && (*ptr != '=') && (*ptr != ' '))
	{
	*ptr = toupper(*ptr);
	ptr++;
	}
if (strncmp(buffer,"*#PDBCMP=",9)) return;
ptr++;
ptr2 = ptr;
while ((*ptr2) && (*ptr2 != ' ') && (*ptr2 != tab))
	ptr2++;
*ptr2 = '\0';
if (strlen(ptr) > 0) 
	{
	ret = search_hash(entpts,ptr,1);
	if (ret == 2) newEntry(ptr);
	}
}

void write_output()

{
int ptr,xx;
ptr = 0;
while ((ptr < 1000) && (inputline[ptr])) 
	{
	xx = putc(inputline[ptr],outf);
	if (xx == EOF)
		{
		printf("Error Writing Output file \n");
		exit(1);
		}
	ptr++;
	}
xx = putc('\n',outf);
if (xx == EOF)
	{
	printf("Error Writing Output file \n");
	exit(1);
	 } 
}


#ifdef __STDC__
void proc_uverb1(char *xverb, char *parms)
#else
void proc_uverb1(xverb,parms)
char *xverb, *parms;
#endif

{
char buffer[1000],buffer2[1000];
char *xx,*yy,*zz;
int cc;

xx = parms;
while(*xx)
	{
	strcpy(buffer,xx);
	cc = 0;
	while((*xx != ',') && (*xx != ' ') && (*xx != '\0') && (*xx != ':'))
		{
		xx++;
		cc++;
		}
	buffer[cc] = '\0';
	yy = buffer;
	cc = 0;
	while ((*yy) && (*yy != '=')) yy++;
	if (*yy == '=')
		{
		*yy = '\0';
		yy++;
		while ((*yy == ' ') || (*yy == tab)) yy++;
		zz = yy;		
		while(*zz)
			{
			*zz = toupper(*zz);
			zz++;
			}
		if (!(strcmp(yy,"LABEL")))
			{
			zz = buffer;
			while ((*zz != '\0') && (*zz != ' ') && (*zz != tab))
				zz++;
			*zz = '\0';
			sprintf(buffer2,"%s*%s",xverb,buffer);
			search_hash(uverb_labs,buffer2,1);
			}
		}
	while ((*xx == ',') || (*xx == ' ')) xx++;
	if (*xx == ':')
		{
		*xx = '\0';
		uverb_cont = 1;
		return;
		}
	}
}


#ifdef __STDC__
void proc_uverb2(char *xverb, char *parms)
#else
void proc_uverb2(xverb,parms)
char *xverb, *parms;
#endif

{
char buffer[1000],buffer2[1000];
char *xx,*yy,*zz;
int cc,inQuotes;

xx = parms;
while(*xx)
	{
	strcpy(buffer,xx);
	cc = 0;
	inQuotes = 0;
	while (*xx != '\0')
		{
		if (inQuotes == 0 && (*xx == ',' || *xx == ':' || *xx == ' ')) break;
		if (*xx == '"') {
			if (inQuotes) inQuotes = 0;
			else inQuotes = 1;
		}
		xx++;
		cc++;
		}
	buffer[cc] = '\0';
	yy = buffer;
	cc = 0;
	while ((*yy) && (*yy != '=')) yy++;
	if (*yy == '=')
		{
		*yy = '\0';
		yy++;
//		printf("checking[%s][%s]\n",xverb,buffer);
		sprintf(buffer2,"%s*%s",xverb,buffer);
		cc = search_hash(uverb_labs,buffer2,0);
		if (cc == 1)
			{
			cc = search_hash(entpts,yy,1);
			if (cc == 2) newEntry(yy);
			}
		}
	while ((*xx == ',') || (*xx == ' ')) xx++;
	if (*xx == ':')
		{
		*xx = '\0';
		uverb_cont2 = 1;
		return;
		}
	}
}


#ifdef __STDC__
void newEntry(char *zz)
#else
void newEntry(zz)
char *zz;
#endif
{
	entcount++;
	if (verbose) printf("Adding Entry Point : %s\n",zz);
}

#ifdef __STDC__
void proc_branch(char *zz)
#else
void proc_branch(zz)
char *zz;
#endif

{
char buffer[1000];
char *xx,*yy;
int cc;

xx = zz;
while(*xx)
	{
	strcpy(buffer,xx);
	cc = 0;
	while((*xx != ',') && (*xx != ' ') && (*xx != '\0') && (*xx != ':'))
		{
		xx++;
		cc++;
		}
	buffer[cc] = '\0';
	yy = buffer;
	if (cc > 0) {
		if (buffer[0] == '~') yy++;
	}
	save_lrefer(lastlabel,yy);
	while ((*xx == ',') || (*xx == ' ')) xx++;
	if (*xx == ':')
		{
		*xx = '\0';
		branch_cont = 1;
		return;
		}
	}
}

void process_line2()

{
int inc,ptr,ptr2,stop,ret;

commentout = 0;
switch(inputline[0])
	{
	case '*' :
     case '+' :
     case '\0' :
     case '.' : 
		write_output();
		return;
	}

if (proc2_cont)
	{
	proc2_cont = 0;
	if (parm[0] == '\0')
		chk_proc2_cont(verb);	
	else
		chk_proc2_cont(parm);	
	}

ptr = 0;
ptr2 = 0;
stop = 0;

process_line();

if (label[0] == '\0') {
	if (lastlabel[0] == '\0') {
		if ((cont == 0) && (verbclass != ENDER) && (proc2_cont == 0)) {
			if (verbclass == ENDROUT) {
				if (inside_unused_lroutine) {
					inside_unused_lroutine = 0;
					comment_out(2,0);
				}
			}
			else {
				comment_out(2,1);
			}
		}
	}
}
else {
	if ((verbclass == DATAVAR) || (verbclass == 3) || (verbclass == EXTLAB) ||
			(verbclass == ENDER) || (verbclass == ENDROUT) || (verbclass == EQUATE)) {
		if (verbclass == DATAVAR && inside_unused_lroutine) comment_out(2,3);
		else chk_proc2_cont(parm);
	}
	else {
		ret = search_hash(actlabels,label,0);
		if (ret == 0) {
			strcpy(lastlabel,"");
			if (verbclass == LROUT) inside_unused_lroutine = 1;
			if (cont == 0) comment_out(2,2);
		}
		else {
			strcpy(lastlabel,label);
			inside_unused_lroutine = 0;
		}
	}
}
cont = 0;
if ((commentout == 0) || (commentopt == 2)) {
	write_output();
	if ((lastchar == ':') && (commentout == 0)) cont = 1;
	}
}

#ifdef __STDC__
void comment_out(int pass, int ident)
#else
void comment_out(pass,ident)
int pass, ident;
#endif

{
commentout = 1;
if (commentopt == 2) fprintf(outf,"*pdbcmp*<%i><%i> ",pass,ident);
}

#ifdef __STDC__
void chk_proc2_cont(char *zz)
#else
void chk_proc2_cont(zz)
char *zz;
#endif

{
char *xx;
int first;

first = 0;
xx = zz;
while(*xx)
	{	
	first++;
	xx++;
	}
if (first)
	{
	xx--;
	if (*xx == ':') proc2_cont = 1;
	}
}

#ifdef __STDC__
int search_hash(HLIST *table[],char *item, int addflg)
#else
int search_hash(table,item,addflg)
HLIST *table[];
char *item;
int addflg;
#endif

{
long htot;
int hptr,stop,depth;
char *ptr;
HLIST *cur,*last,*putspot;


strcpy(bufferx,item);
ptr = bufferx;
while(*ptr)
	switch (*ptr)
		{
		case ' ':
		case tab :
			*ptr = '\0';
			break;
		default :
			ptr++;
		}
htot = 0;
ptr = bufferx;
for(hptr=0;hptr<5;hptr++)
	if (*ptr != '\0')
		{
		htot = htot + (hptr + 1) * (int)(*ptr);
		ptr++;
		}
hptr = htot % MAX_HASH;


cur = table[hptr];
depth = 0;
stop = 0;
while (!(stop))
	if (cur == '\0')
		{
		stop = 1;
		}
	else
		if (!(strcmp(bufferx,cur->item)))
			{
			return(1);
			}
		else
			{
			last = cur;
			cur = (HLIST *)cur->next;
			depth++;
			}
if (addflg)
	{
	if ((putspot=(HLIST *)malloc(sizeof (HLIST)))==NULL)
		{
		printf("\nOut of Memory Adding HLIST\n\n");
		exit(1);
		}
	putspot->next = '\0';
	if ((putspot->item=(char *)malloc(strlen(bufferx) + 1)) == NULL)
		{
		printf("\nOut of Memory Adding HLIST Item\n\n");
		exit(1);
		}
	strcpy(putspot->item,bufferx);
	if (depth == 0) table[hptr] = putspot;
	else last->next=(unsigned char *)putspot;
	return(2);
	}
else
	return(0);
}
	


#ifdef __STDC__
void searchpath(char *fn, char *envvar, char *outpath)
#else
void searchpath(fn,envvar,outpath)
char *fn, *envvar, *outpath;
#endif

{
char *xpath,*xx,*yy;

xx = fn;
while ((*xx) && (*xx != SLASH)) xx++;
if (*xx == SLASH)
	{
	strcpy(outpath,fn);
	return;
	}

xpath = envvar;
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
        
#ifdef __STDC__
void save_lrefer(char *lab1,char *lab2)
#else
void save_lrefer(lab1,lab2)
char *lab1, *lab2;
#endif

{
char *ptr;
int ret;
char buffer[1000];

sprintf(buffer,"%-5s*%s",lab1,lab2);
ptr = buffer;
while(*ptr)
	{
	if (*ptr == ' ') *ptr= '*';
	ptr++;
	}
ret = search_hash(lrefer,buffer,1);
if (ret == 2) 
	lrefercount++;
}


#ifdef __STDC__
void getline(FILE *fp)
#else
void getline(fp)
FILE *fp;
#endif

{

int     ptr,eol,eof;
char    aa;

ptr = 0;
eol = 0;
eof = 0;

while (!((eol) || (eof) || (ptr == 1000)))
        {
        aa = getc(fp);
        if (feof(fp)) eof = 1;
        else
         {
         if (aa == '\n')
                {
                eol = 1;
                inputline[ptr] = '\0';
                }
         else inputline[ptr] = aa;
         ptr++;
         }
        }
inputline[ptr] = '\0';
while ((ptr > -1) && 
	((inputline[ptr] == '\0') || (inputline[ptr] == ' ') || (inputline[ptr] == tab)))
	{
	inputline[ptr] = '\0';
	ptr--;
	}
if (ptr > -1) lastchar = inputline[ptr];
else lastchar = '\0';
}

#ifdef __STDC__
void fixname(char *name)
#else
void fixname(name)
char *name;
#endif

{
char *ptr,*ptr2,bufx[200],bufx2[200];
int ii,volst,slen;
long htot,hptr;
HLIST *cur;


ptr = name;
ii = 0;
volst = -1;
while (*ptr)
	{
	ptr++;
	ii++;
	}
if (ii == 0) return;
ii--;
ptr--;
while ((ii > -1) && (*ptr != SLASH))
	{
	switch(namecase)
		{
		case 1 :
			*ptr = toupper(*ptr);
			break;
		case 2 :
			*ptr = tolower(*ptr);
			break;
		}
	if (*ptr == ':') volst = ii;
	ii--;
	ptr--;
	}
if ((volst > 0) && (volst != 1))
	{
	ptr = name;
	ii = 0;
	while (ii != volst)
		{
		ii++;
		ptr++;
		}
	ptr++;
	strcpy(bufx2,ptr);
	sprintf(bufx,"DBCVOL_%s",ptr);
	if ((ptr = getenv(bufx)) != 0)
		{
		strcpy(bufx,ptr);
		ptr2 = bufx;
		while(*ptr2) ptr2++;
		ptr2--;
		if (*ptr2 == SLASH)
			ptr2++;
		else
			{
			ptr2++;
			*ptr2=SLASH;
			ptr2++;
			*ptr2='\0';
			}
		strcpy(bufx2,name);
		sprintf(name,"%s%s",bufx,bufx2);
		ptr = name;
		while (*ptr) ptr++;
		while (*ptr != ':') ptr--;
		*ptr = '\0';
		}
	else
		{
		sprintf(bufx,"%-5s*",bufx2);
		ptr = bufx;
		while(*ptr)
			{
			if (*ptr == ' ') *ptr = '*';
			ptr++;
			}
		htot = 0;
		ptr = bufx;
		for(hptr=0;hptr<5;hptr++)
			if (*ptr != '\0')
				{
				htot = htot + (hptr + 1) * (int)(*ptr);
				ptr++;
				}
		hptr = htot % MAX_HASH;
		cur = dbcvol[hptr];
		slen = strlen(bufx2);
		if (slen < 5) slen = 5;
		slen++;
		ptr = 0;
		while(cur)
			{
			if (!(strncmp(bufx,cur->item,slen)))
				{
				ptr = cur->item;
				ptr++;
				while (*ptr != '*') ptr++;
				while (*ptr == '*') ptr++;
				cur = 0;
				}
			else cur=(HLIST *)cur->next;
			}
		if (ptr)
			{
			strcpy(bufx,ptr);
			ptr2 = bufx;
			while(*ptr2) ptr2++;
			ptr2--;
			if (*ptr2 == SLASH)
				ptr2++;
			else
				{
				ptr2++;
				*ptr2=SLASH;
				ptr2++;
				*ptr2 = '\0';
				}
			strcpy(bufx2,name);
			sprintf(name,"%s%s",bufx,bufx2);
			ptr = name;
			while (*ptr) ptr++;
			while (*ptr != ':') ptr--;
			*ptr = '\0';
			}
		}
	}
}
		


void format()

{
printf("\nFormat : pdbcmp file[.prg] [opts]\n");
printf("Output file will be 'file.pdb\n\n");
printf("\nOptions\n");
printf("-v Verbose Mode\n");
printf("-1 Pass 1 Only\n");
printf("-c1 Comment Option 1, retain all original comments\n");
printf("-c2 Comment Option 2, retain all original source lines with\n");
printf("    those removed commented out\n");
printf("-o=filename, to specify output file\n");
printf("-w=filename, to specify pass 1 workfile\n");
printf("-e=VAR, to set an equate\n");
printf("-k  kill debug statements\n");
exit(1);
}

