/* nomarch 1.0 - extract old `.arc' archives.
 * Copyright (C) 2001 Russell Marks. See unarc.c for license details.
 *
 * readhuff.c - read RLE+Huffman-compressed files (the CP/M `SQ' scheme).
 */

/* I was originally going to adapt some old GPL'd Huffman code,
 * but it turns out the format pretty much forces an array-based
 * implementation. Not that I mind that :-), it just makes it
 * a bit unusual. So this is from scratch (though it wasn't too hard).
 */

#include "stdio2.h"
#include <stdlib.h>
#include "readrle.h"

#include "readhuff.h"


struct huff_node_tag
  {
  /* data is stored as a negative `pointer' */
  int kids[2];
  };

#define READ_WORD(x,y) (x)=rawinput(y),(x)|=(rawinput(y)<<8)
#define VALUE_CONV(x) ((x)^0xffff)

#define HUFF_EOF	256


struct bits {
 int bitbox,bitsleft;
};


static int rawinput(struct data_in_out *io)
{
if(io->data_in_point<io->data_in_max)
  return(*io->data_in_point++);
return(-1);
}

static void rawoutput(int byte, struct data_in_out *io)
{
if(io->data_out_point<io->data_out_max)
  *io->data_out_point++=byte;
}


static void bit_init(struct bits *bits)
{
  bits->bitbox=0; bits->bitsleft=0;
}

static int bit_input(struct bits *bits, struct data_in_out *io)
{
  if(bits->bitsleft==0)
  {
    bits->bitbox=rawinput(io);
    if(bits->bitbox==-1) return(-1);
    bits->bitsleft=8;
  }

  bits->bitsleft--;
  return((bits->bitbox&(1<<(7-bits->bitsleft)))?1:0);
}


unsigned char *convert_huff(unsigned char *data_in,
                            unsigned long in_len,
                            unsigned long orig_len)
{
unsigned char *data_out;
struct huff_node_tag *nodearr;
int nodes,f,b;
struct bits bits;
struct rledata rd;
struct data_in_out io;

if((data_out=malloc(orig_len))==NULL)
  fprintf(stderr,"nomarch: out of memory!\n"),exit(1);

io.data_in_point=data_in; io.data_in_max=data_in+in_len;
io.data_out_point=data_out; io.data_out_max=data_out+orig_len;

READ_WORD(nodes,&io);

if(!nodes)
  {
  free(data_out);
  return(NULL);
  }

if((nodearr=malloc(sizeof(struct huff_node_tag)*nodes))==NULL)
  fprintf(stderr,"nomarch: out of memory!\n"),exit(1);

/* apparently the tree can be empty (zero-length file?), so
 * there's a preset entry which is required. In the context of
 * .arc I'm sure this is cruft which we don't actually need,
 * but just in case...
 */
nodearr[0].kids[0]=nodearr[0].kids[1]=VALUE_CONV(HUFF_EOF);

for(f=0;f<nodes;f++)
  {
  READ_WORD(nodearr[f].kids[0],&io);
  READ_WORD(nodearr[f].kids[1],&io);
  }

/* after the table, we get the codes to interpret; this is
 * a bitstream, with EOF marked by the code HUFF_EOF.
 */
bit_init(&bits);
outputrle(-1,NULL,&rd,&io);

do
  {
  f=0;
  while((f&0x8000)==0)
    {
    if(f>=nodes)
      {
      /* must be corrupt */
      free(nodearr);
      free(data_out);
      return(NULL);
      }

    /* it seems we can't rely on getting the EOF code (even though we
     * do >95% of the time!), so check for `real' EOF too.
     * (worth checking in case of corrupt file too, I guess.)
     */
    if((b=bit_input(&bits,&io))==-1)
      {
      f=VALUE_CONV(HUFF_EOF);
      break;
      }
    
    f=nodearr[f].kids[b];
    }
  
  f=VALUE_CONV(f);
  if(f!=HUFF_EOF)
    outputrle(f,rawoutput,&rd,&io);
  }
while(f!=HUFF_EOF);

free(nodearr);

return(data_out);
}
