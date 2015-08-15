/*

This code is Copyright 2005-2006 by Michael Kohn

This package is licensed under the LGPL. You are free to use this library
in both commercial and non-commercial applications as long as you dynamically
link to it. If you statically link this library you must also release your
software under the LGPL. If you need more flexibility in the license email
me and we can work something out. 

Michael Kohn <mike@mikekohn.net>

*/
#include "stdio2.h"
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include "common.h"
#include "depacker.h"
#include "inflate.h"
#include "crc32.h"

struct zip_file_header
{
  unsigned int signature;
  int version;
  int general_purpose_bit_flag;
  int compression_method;
  int last_mod_file_time;
  int last_mod_file_date;
  unsigned int crc_32;
  int compressed_size;
  int uncompressed_size;
  int file_name_length;
  int extra_field_length;
  char *file_name;
  unsigned char *extra_field;
};

#define QUIET

#define read_int(x) read32l(x)
#define read_word(x) read16l(x)

/*-------------------------- fileio.c ---------------------------*/


static int read_chars(FILE *in, char *s, int count)
{
int t;

  for (t=0; t<count; t++)
  {
    s[t]=getc(in);
  }

  s[t]=0;

  return 0;
}

static int read_buffer(FILE *in, unsigned char *buffer, int len)
{
int t;

  t=0;
  while (t<len)
  {
    t=t+fread(buffer+t,1,len-t,in);
  }

  return t;
}

static int write_buffer(FILE *out, unsigned char *buffer, int len)
{
int t;

  t=0;
  while (t<len)
  {
    t=t+fwrite(buffer+t,1,len-t,out);
  }

  return t;
}

/*----------------------- end of fileio.c -----------------------*/

#define BUFFER_SIZE 16738


static unsigned int copy_file(FILE *in, FILE *out, int len, struct inflate_data *data)
{
unsigned char buffer[BUFFER_SIZE];
unsigned int checksum;
int t,r;

  checksum=0xffffffff;

  t=0;

  while(t<len)
  {
    if (t+BUFFER_SIZE<len)
    { r=BUFFER_SIZE; }
      else
    { r=len-t; }

    read_buffer(in,buffer,r);
    write_buffer(out,buffer,r);
    checksum=crc32_A2(buffer,r,checksum);
    t=t+r;
  }

  return checksum^0xffffffff;
}

static int read_zip_header(FILE *in, struct zip_file_header *header)
{
  header->signature=read_int(in);
  if (header->signature!=0x04034b50) return -1;

  header->version=read_word(in);
  header->general_purpose_bit_flag=read_word(in);
  header->compression_method=read_word(in);
  header->last_mod_file_time=read_word(in);
  header->last_mod_file_date=read_word(in);
  header->crc_32=read_int(in);
  header->compressed_size=read_int(in);
  header->uncompressed_size=read_int(in);
  header->file_name_length=read_word(in);
  header->extra_field_length=read_word(in);

  return 0;
}


/* For xmp:
 * pass an array of patterns containing files we want to exclude from
 * our search (such as README, *.nfo, etc)
 */
static int kunzip_file_with_name(FILE *in, FILE *out)
{
struct zip_file_header header;
int ret_code;
uint32 checksum=0;
long marker;
struct inflate_data data;

  ret_code=0;

  if (read_zip_header(in,&header)==-1) return -1;

  header.file_name=(char *)malloc(header.file_name_length+1);
  if (header.file_name == NULL)
    goto err;

  header.extra_field=(unsigned char *)malloc(header.extra_field_length+1);
  if (header.extra_field == NULL)
    goto err2;

  read_chars(in,header.file_name,header.file_name_length);
  read_chars(in,(char *)header.extra_field,header.extra_field_length);

  marker=ftell(in);

  crc32_init_A();

  if (header.uncompressed_size!=0)
  {
    if (header.compression_method==0)
    {
      checksum=copy_file(in,out,header.uncompressed_size,&data);
    }
    else
    {
      if (inflate(in, out, &checksum, 1) < 0)
	goto err3;
    }

    /* fclose(out); */

    if (checksum!=header.crc_32)
    {
      /* fprintf(stderr, "unzip: crc error: %d %d\n",checksum,header.crc_32); */
      ret_code=-4;
    }
  }

  free(header.file_name);
  free(header.extra_field);

  fseek(in,marker+header.compressed_size,SEEK_SET);

  if ((header.general_purpose_bit_flag&8)!=0)
  {
    read_int(in);
    read_int(in);
    read_int(in);
  }

  return ret_code;

 err3:
  free(header.extra_field);
 err2:
  free(header.file_name);
 err:
  return -1;
}

static int kunzip_get_offset_excluding(FILE *in)
{
struct zip_file_header header;
int i=0,curr;
int name_size;
long marker;
char name[1024];

  while(1)
  {
    curr=ftell(in);
    i=read_zip_header(in,&header);
    if (i==-1) break;

    /*if (skip_offset<0 || curr>skip_offset)*/
    {
      marker=ftell(in);  /* nasty code.. please make it nice later */

      name_size = header.file_name_length;
      if (name_size > 1023) {
	name_size = 1023;
      }
      read_chars(in,name,name_size);
      name[name_size]=0;

      fseek(in,marker,SEEK_SET); /* and part 2 of nasty code */

      if (!exclude_match(name)) {
        break;
      }
    }

    fseek(in,header.compressed_size+
             header.file_name_length+
             header.extra_field_length,SEEK_CUR);
  }

  if (i!=-1)
  { return curr; }
    else
  { return -1; }
}

static int test_zip(unsigned char *b)
{
	return b[0] == 'P' && b[1] == 'K' &&
		((b[2] == 3 && b[3] == 4) || (b[2] == '0' && b[3] == '0' &&
		b[4] == 'P' && b[5] == 'K' && b[6] == 3 && b[7] == 4));
}

static int decrunch_zip(FILE *in, FILE *out)
{
  int offset;

  offset = kunzip_get_offset_excluding(in);
  if (offset < 0)
    return -1;

  fseek(in, offset, SEEK_SET);

  if (kunzip_file_with_name(in,out) < 0)
     return -1;

  return 0;
}

struct depacker zip_depacker = {
	test_zip,
	decrunch_zip
};
