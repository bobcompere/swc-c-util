/*

convert pcx files to pcl files

*/

#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "ctype.h"

typedef  struct  pcx_hdr{
        unsigned  char zsoft;
        unsigned  char version;
        unsigned  char encoding;
        unsigned  char bits_per_pix;
        short     xmin;
        short     ymin;
        short     xmax;
        short     ymax;
        short     dpi_x;
        short     dpi_y;
        unsigned char pallet[16][3];
        unsigned char resv;
        unsigned char planes;
        short     bytes_per_line;
        short     hdr_pal_int;
        short     vid_x;
        short     vid_y;
       } PCX_HDR ;

void     load_pcx_to_screen(char *fname,short x, short y);
void	    convert_pcx_to_pcl(char *fname, char *outf, short swt);                   

unsigned char	bitmask[8] = {
				        0x01,
				        0x02,
				        0x04,
				        0x08,
				        0x10,
				        0x20,
				        0x40,
				        0x80
				        };


char *VERSION="1.00002";

int main(int argc, char *argv[])

{
short swt; 

printf("PCXtoPCL\n");
printf("Subject, Wills & Company -- Version %s --- %s\n",VERSION,__DATE__);

if (argc < 3) 
	{
	printf("\n\nformat : C>pcxtopcl pclfile pcxfile\n\n");
	exit(1);
	}
swt = -1;
if (argc == 4) {
	if (!(strcmp("0",argv[3])))
		swt = 0;
	if (!(strcmp("1",argv[3])))
		swt = 1;
}
convert_pcx_to_pcl(argv[1],argv[2],swt);
}

void     convert_pcx_to_pcl(char *filename,char *outf, short fswt)

{
FILE     *pcxfile,*pclfile;
PCX_HDR  *hdr;
char     *ptr;
int    	 ccmax,cnt,size,i;
long     imagesize,pixset;
short    alt,xx,yy,dx,dy,lastcolor,c[8];
char     aa,disp[50];
unsigned cc;
char	    *bitmap;
int	    stripe,strcnt;
int	    bp;
unsigned char outbyte;
int skipper,extra ;

printf("preparing to convert\n");
if ((pcxfile = fopen(filename,"rb")) == NULL)
         {
         printf("Unable to open PCX file : %s\n\n",filename);
         exit(1);
         }
if ((pclfile = fopen(outf,"wb")) == NULL)
         {
         printf("Unable to create PCL file : %s\n\n",outf);
         exit(1);
         }
size = sizeof(PCX_HDR);
if ((hdr = (PCX_HDR *)malloc(size)) == NULL)
         {
         printf("Unable to allocate buffer for PCX header\n\n");
         exit(1);
         }
printf("Input file openned\n");
ptr = (char *)hdr;
for(i=0;i<size;i++)
         {
         *ptr = getc(pcxfile);
         ptr = ptr + 1;
         }
printf("Header read in\n");
dx = hdr->xmax - hdr->xmin + 1;
dy = hdr->ymax - hdr->ymin + 1;
imagesize = (long)dx * (long)dy;

printf("Zsoft : %i\n",hdr->zsoft);
printf("Version : %i\n",hdr->version);
printf("Encoding : %i\n",hdr->encoding);
printf("Bits per Pixel : %i\n",hdr->bits_per_pix);
printf("X min : %i\n",hdr->xmin);
printf("Y min : %i\n",hdr->ymin);
printf("X max : %i\n",hdr->xmax);
printf("Y max : %i\n",hdr->ymax);
printf("dx    : %i\n",hdr->xmax);
printf("dy    : %i\n",hdr->ymax);
printf("DPI X : %i\n",hdr->dpi_x);
printf("DPI Y : %i\n",hdr->dpi_y);
printf("Planes : %i\n",hdr->planes);
printf("Bytes per line : %i\n",hdr->bytes_per_line);
printf("Video X : %i\n",hdr->vid_x);
printf("Video Y : %i\n",hdr->vid_y);
printf("Image size : %li\n\n",imagesize);

extra = (8 * hdr->bytes_per_line) - (hdr->bits_per_pix * hdr->xmax);
printf("extra : %i\n",extra);
//if ((extra == 0) || (extra > 7))
if (fswt == -1)	{
	if (extra > 7)
		skipper = 1;
	else
		skipper = 0;
	printf("Skipper   : %i\n",skipper);
}
else	{
	skipper = fswt;
	printf("Skipper : %i ---- forced\n ",skipper);
}

/* init */

putc(0x00,pclfile);
putc(0x00,pclfile);

/* set res to 300dpi */

putc(0x1b,pclfile);
putc(0x2a,pclfile);
putc(0x74,pclfile);
putc(0x33,pclfile);
putc(0x30,pclfile);
putc(0x30,pclfile);
putc(0x52,pclfile);

/* begin at current position */

putc(0x1b,pclfile);
putc(0x2a,pclfile);
putc(0x72,pclfile);
putc(0x31,pclfile);
putc(0x41,pclfile);

xx = 0;
yy = 0;

stripe = dx / 8;
if (dx % 8) stripe++;
bp = 7;
strcnt = stripe;
outbyte = 0x00;

/*  write 'stripe' bytes */

putc(0x1b,pclfile);
putc(0x2a,pclfile);
putc(0x62,pclfile);
fprintf(pclfile,"%iW",stripe);

fseek(pcxfile,128,SEEK_SET);
pixset = 0;
lastcolor = -1;

while(pixset < imagesize)
         {
         cc = getc(pcxfile);
         if ((cc & 128) && (cc & 64))   
                   {
                   cnt = (cc & 63);
                   cc = getc(pcxfile);
			    switch (hdr->bits_per_pix)
					{
					case 8:
					  ccmax = 1;
					  c[0] = (cc & 255);
					  break;
					case 4:
					  ccmax = 2;
                   		  c[0] = (cc & 240);
                   		  c[1] = (cc & 15);
					  break;
					case 1:
				       ccmax = 8;
					  c[0] = cc & 128;
					  c[1] = cc & 64;
					  c[2] = cc & 32;
					  c[3] = cc & 16;
					  c[4] = cc & 8;
					  c[5] = cc & 4;
					  c[6] = cc & 2;
					  c[7] = cc & 1;
					}
                   }
         else
                   {
			    switch (hdr->bits_per_pix)
					{
					case 8:
					  ccmax = 1;
					  c[0] = (cc & 255);
					  break;
					case 4:
					  ccmax = 2;
                   		  c[0] = (cc & 240);
                   		  c[1] = (cc & 15);
					  break;
					case 1:
				       ccmax = 8;
					  c[0] = cc & 128;
					  c[1] = cc & 64;
					  c[2] = cc & 32;
					  c[3] = cc & 16;
					  c[4] = cc & 8;
					  c[5] = cc & 4;
					  c[6] = cc & 2;
					  c[7] = cc & 1;
					}
                   cnt = 1;
                   }
         alt = 0;
         while((xx < dx) && (cnt > 0))
                   {
			    if (c[alt] == 0) outbyte = outbyte | bitmask[bp];
		         bp--;
		         if (bp == -1)
	                 {
	                 putc(outbyte,pclfile);
	                 bp = 7;
		            outbyte = 0x00;
	                 }
                   pixset++;
                   if (alt == (ccmax - 1))
                             {
                             cnt--;
                             alt = 0;
                             }
                   else  alt++;
                   xx++;
                   }
		
         if (xx == dx)
                   {
                   xx = 0;
			    if (bp != 7)
	                 putc(outbyte,pclfile);
//				else
//					{
//	printf("\ndid it");
         		    if (skipper)   cc = getc(pcxfile);
//					}
			    bp = 7;
			    alt = 0;
	              outbyte = 0x00;
			    if (imagesize != pixset)
	              	  { 
				  putc(0x1b,pclfile);
                      putc(0x2a,pclfile);
                      putc(0x62,pclfile);
	                 fprintf(pclfile,"%iW",stripe);
				  }
                   }
         }
if (bp != 7)
      putc(outbyte,pclfile);
putc(0x1b,pclfile);
putc(0x2a,pclfile);
putc(0x72,pclfile);
putc(0x42,pclfile);
fclose(pclfile);
fclose(pcxfile);
}
