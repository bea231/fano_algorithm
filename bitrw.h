/* Sergeev Artemiy, 33601/2 (3057/2), 07.10.2013 */

#ifndef _BITRW_H_
#define _BITRW_H_

#include <stdio.h>

void bitWInit( FILE *F );
void bitWrite( int bit );
void bitsWrite( int Bits, int Len );
int  bitWriteClose( void );
void bitReadInit( FILE *F );
int bitRead( void );
int bitsRead( int len );
#endif /* _BITRW_H_ */