/* Sergeev Artemiy, 33601/2 (3057/2), 07.10.2013 */

#ifndef _FANO_H_
#define _FANO_H_

#include "errors.h"

#define MAX_CHARACTERS_NUM 256
#define MAX_TREE_LEVEL     10

typedef struct tag_alg_parameters
{
  char *codingFileName,
       *inputFileName,
       *outputFileName;
} alg_parameters_t;

typedef struct tag_binary_code
{
  unsigned char code[32];
  unsigned char length;
} binary_code_t;

typedef struct tag_probability_char
{
  unsigned char character;
  double        probability;
} probability_char_t;

error_code_t fanoEncode( alg_parameters_t parameters );
error_code_t fanoDecode( alg_parameters_t parameters );

#endif /* _FANO_H_ */
