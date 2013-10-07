/* Sergeev Artemiy, 33601/2 (3057/2), 07.10.2013 */

#include "bitrw.h"

#define BIT_BUF_SIZE 7

/* Bit read/write variables */
static unsigned char s_BitWRAccum;
static int s_BitWRPos;
static FILE *s_BitWRF;

void bitWriteInit( FILE *F )
{
  s_BitWRPos = BIT_BUF_SIZE;
  s_BitWRAccum = 0;
  s_BitWRF = F;
}

void bitWrite( int bit )
{
  if (bit)
    s_BitWRAccum |= (1 << s_BitWRPos);
  if (--s_BitWRPos < 0)
  {
    fputc(s_BitWRAccum, s_BitWRF);
    s_BitWRPos = BIT_BUF_SIZE;
    s_BitWRAccum = 0;
  }
}

void bitsWrite( int Bits, int Len )
{
  while (Len-- > 0)
    bitWrite((Bits >> Len) & 1);
}

int  bitWriteClose( void )
{
  if (s_BitWRPos != BIT_BUF_SIZE)
    fputc(s_BitWRAccum, s_BitWRF);
  return BIT_BUF_SIZE - s_BitWRPos;
}

void bitReadInit( FILE *F )
{
  s_BitWRPos = -1;
  s_BitWRF = F;
}


int bitRead( void )
{
  int b;

  if (s_BitWRPos < 0)
  {
    if ((b = fgetc(s_BitWRF)) == EOF)
      return EOF;
    s_BitWRAccum = b;
    s_BitWRPos = BIT_BUF_SIZE;
  }
  return (s_BitWRAccum >> s_BitWRPos--) & 1;
} 

int bitsRead( int len )
{
  int a = 0, b;

  while (len-- > 0)
  {
    if ((b = bitRead()) == EOF)
      return EOF;
    a |= b << len;
  }
  return a;
}

