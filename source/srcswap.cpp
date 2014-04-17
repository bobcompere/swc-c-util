# include "stdio.h"
# include "ctype.h"
# include "stdlib.h"
# include "string.h"

#define BUF_SIZE 1024

char *VERSION="2.00001";

FILE     *inf,*outf,*tran;
char     *xargv[10],name[30],buf[BUF_SIZE];
char     markchar;
int      noquote,comm,lit,disp,ovrwrit,nomark,xargc,fopn;
char     a,b,c,d,e,f,*x,*y,*z,*w;
int      i,j,k,l,m,n,o,p,db,ok,done;

struct  xl         {
                   char      type;
                   char      instring[30];
                   char      outstring[30];
                   int       len;
                   struct    xl *next;
                   }    *top,*cur,*next,*nuevo;


void getopt(int ii);
void  get_xl();
void get_xl_file();
void add_xl_to_list();
void get_xl_line();
void srt_xl();
void xl_swap(struct xl *z1,struct xl *z2);
void process();
void getinp();
void getout();
void getline();
void mk_nuevo();
void format();

int main(int argc,char *argv[])

{

printf("SRCSWAP - Source code mark/replace utility Version 1.0000 %s",__DATE__);
markchar='~';
ovrwrit = 0;
disp=0;
nomark=0;
noquote=0;
xargc=argc;

for (i=0;i<argc;i++) 
	{
	xargv[i] = argv[i];
	}

if (argc < 4) format();

i = 4;
while (i < argc) 
         {
         getopt(i);
         i++;
         }

getinp();
getout();

top=NULL;
cur=NULL;
 
get_xl();
srt_xl();
done=0;

printf("\nProcessing...");
while (!(done))
         {
         getline();
         process();
         }
fclose(outf);
return(0);
}


void getopt(int ii)

{
x = xargv[ii];

if (*x != '/')
         format();
x++;
switch (*x)
         {
         case 'o':
         case 'O': ovrwrit = 1;
                   printf("\nOverwrite Enabled");
                   break;
         case 'N' :
         case 'n' : printf("\nNo Mark Option on Swapped Strings");
                   nomark=1;
                   break;
         case 'c' :
         case 'C' : x++;
                    if (*x != '=') format();
                    x++;
                    if (!(*x)) format();
                    markchar=*x;
                    printf("\nMark Character set to %c",markchar);
                    break;
         case 'd' :
         case 'D' : printf("\nDisplay translations");
                    disp=1;
                    break;
	    case 'q' :
	    case 'Q' :
			     printf("\nIngnore Quotation Marks");
			     noquote = 1;
				break;
         default  : format();
         }
}




void  get_xl()

{
x = xargv[3];
fopn = 0;
while (*x)
         {
         get_xl_file();
         printf("\ntranslation file : %s",&name[0]);
         add_xl_to_list();
         }
}



void get_xl_file()

{
i = 0;
while ((*x) && (*x != ','))
         {
         name[i]=*x;
         x++;
         i++;
         }
if (*x) x++;
name[i] = '\0';

if (fopn)
         fclose(tran);

if ((tran = fopen(&name[0],"r"))==NULL)
         {
	printf("\nUnable to open translation file : %s\n\n",&name[0]);
         format();
         }
}




void add_xl_to_list()

{
int      len,ptr,ptr2;
int      err;

while (!(feof(tran)))
         {
         err=0;
         get_xl_line();
         switch(buf[0])
         {
         case 's' :
         case 'S' :  mk_nuevo();
                     nuevo->type='S';
                     ptr=1;
                     if (buf[ptr] != ',') err=1;
                     ptr++;
                     ptr2=0;
                     nuevo->len=0;
                     while ((!(err)) && (buf[ptr]) && (buf[ptr] != ','))
                             {         
                             nuevo->instring[ptr2]=buf[ptr];
                             ptr++;
                             ptr2++;
                             nuevo->len++;
                             }
                     if ((ptr2 != 0) && (!(err)) && (buf[ptr] == ',')) 
                        {
                        nuevo->instring[ptr2]='\0';
                        ptr++;
                        }
                        else err=1;
                     ptr2=0;
                     while ((!(err)) && (buf[ptr]))
                             {
                             nuevo->outstring[ptr2] = buf[ptr];
                             ptr++;
                             ptr2++;
                             }
                     if (!(err))
                       {
                       nuevo->outstring[ptr2]='\0';
                       if (disp) printf("\nSwap : %s WITH %s",nuevo->instring,nuevo->outstring);
                       }
                     break;
         case 'm' :
         case 'M' :  mk_nuevo();
                     nuevo->type='M';
                     ptr=1;
                     if (buf[ptr] != ',') err=1;
                     ptr++;
                     ptr2=0;
                     nuevo->len=0;
                     while ((!(err)) && (buf[ptr]))
                             {         
                             nuevo->instring[ptr2]=buf[ptr];
                             nuevo->len++;
                             ptr++;
                             ptr2++;
                             }
                     if (ptr2 == 0) err=1;
                     nuevo->instring[ptr2]='\0';
                     strcpy(nuevo->outstring,"*");
                     if (!(err))
                        if (disp) printf("\nMark : %s",nuevo->instring);
                     break ;
         case '\0':  break;
         default  :  err = 1;
         }
         if (err)  {
                   printf("\nError in input file : %s : %s\n",&name[0],&buf[0]);
                   exit(1);
                   }
         }
}
            
                                                           

void get_xl_line()

{
int	ptr,eol,eof;
char	aa;

ptr = 0;
eol = 0;
eof = 0;

while (!((eol) || (eof) || (ptr >= BUF_SIZE)))
	{
	aa = getc(tran);
	if (feof(tran)) eof = 1;
	else
          switch (aa)
            {
            case '\n' : eol = 1;
                        buf[ptr] = '\0';
                        ptr++;
                        break;
            case ' '  : break;
            default   : buf[ptr]=aa;
                        ptr++;
            }
	}
buf[ptr] = '\0';
}


void srt_xl()


{
struct   xl    *x1,*x2;
printf("\nSorting......");
x1  =  top;
while (x1)
         {
         x2 = x1->next;
         while (x2)
                   {
                   if (disp) printf("\n%10s(%2i)  %10s(%2i)",x1->instring,x1->len,
                                                             x2->instring,x2->len);
                   if (x1->len < x2->len) xl_swap(x1,x2);
                   x2 = x2->next;
                   }
         x1 = x1->next;
         }
printf("done!");
if       (disp)
         x1 = top;
         while(x1)
                   {
                   switch(x1->type)
                   {
                    case 'S' : 
                             printf("\n(%2i) swap %s WITH %s",x1->len,x1->instring,x1->outstring);
                             break;
                    case 'M' : 
                             printf("\n(%2i) mark %s",x1->len,x1->instring);
                   }
                   x1=x1->next;
                   }
}

void xl_swap(struct xl *z1,struct xl *z2)

{
char     temptype,tempin[30],tempout[30];
int      templen;

strcpy(&tempin[0],z1->instring);
strcpy(&tempout[0],z1->outstring);
templen = z1->len;
temptype = z1->type;

strcpy(z1->instring,z2->instring);
strcpy(z1->outstring,z2->outstring);
z1->len = z2->len;
z1->type= z2->type;

strcpy(z2->instring,&tempin[0]);
strcpy(z2->outstring,&tempout[0]);
z2->len=templen;
z2->type=temptype;
}

void process()

{
int      ii,kk,jj,xl;

ii = 0;
switch (buf[ii])
         {
         case '+' :
         case '*' :
         case '.' :  comm=1;
                     break;
         default  :  comm=0;
         }
lit = 0;
while (buf[ii])
         {
         cur=top;
         xl=0;
         while ((!(comm)) && (!(lit)) && (cur) && (!(xl)))
                   {
                   kk=0;
                   jj=ii;
                   while ((cur->instring[kk]) && (cur->instring[kk] == buf[jj]))
                             {
                             jj++;
                             kk++;
                             }
                   if (!(cur->instring[kk]))
                             {
                             xl=1;
                             switch (cur->type)
                             {
                             case 'M' : putc(markchar,outf);
                                        fprintf(outf,"%s",cur->instring);
                                        ii=jj;
                                        break;
                             case 'S' : if (!(nomark)) putc(markchar,outf);
                                        fprintf(outf,"%s",cur->outstring);
                                        ii=jj;
                                        break;
                             }
                             }
                   cur=cur->next;
                   }
         if (!(xl)) 
                   {
                   putc(buf[ii],outf);
                   if ((buf[ii] == '"') && (noquote == 0))
                             if (lit == 1) lit = 0;
                             else lit = 1;
                   ii++;
                   }
         }
putc('\n',outf);         
}



void getinp()

{
if ((inf = fopen(xargv[1],"r")) == NULL)
	{
	printf("\nUnable to open input file : %s\n\n",xargv[1]);
         format();
	}
printf("\nInput File : %s",xargv[1]);
return;
}



void getout()

{
if (!(ovrwrit))
	{
	if ((outf = fopen(xargv[2],"r")) != NULL)
		{
		printf("\nOutput File : %s \nAlready Exists !! \n\n",xargv[2]);
                   format();
		}
	}
if ((outf= (fopen(xargv[2],"w"))) == NULL)
	{
	printf("\nUnable to Create Output File : %s\n\n",xargv[2]);
         format();
	}
printf("\nOutput file : %s",xargv[2]);
}


void getline()


{
int	ii;

ii  = 0;
a = ' ';
while ((!(feof(inf))) && (a != '\n')) 
	{
	a = getc(inf);
	if ((a != '\n') && (!(feof(inf)))) 
		{
//		buf[ii] = toupper(a);
		buf[ii] = a;
		ii++;
		}
	}
buf[ii] = '\0';
if (feof(inf)) done = 1;
}

void mk_nuevo()

{
if ((nuevo = (struct xl*)malloc(sizeof(struct xl))) == NULL)
         {          
	printf("\nMEMORY ALLOCATION ERROR !!\n\n");
	exit(1);
         }
if (cur == NULL)
         top=nuevo;
else
         cur->next=nuevo;
cur=nuevo;
nuevo->next=NULL;
}
         




void format()


{
printf("\nFormat c>srcswap infile outfile tranfile1,tranfile2...   <opts>\n");
printf("\nOptions : \n\n");
printf("\n/O - Overwrite Option");
printf("\n/N - No Marks on Swapped Strings");
printf("\n/C=x Set Mark Character to 'x', default is ~");
printf("\n/Q - Ignore Quotation Marks");
printf("\n\nTranfile Format:");
printf("\nMark Text : 'M,text'");
printf("\nSwap Text : 'S,in_text,out_text'");
printf("\n\n");
exit(1);
}
