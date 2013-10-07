/* Sergeev Artemiy, 33601/2 (3057/2), 07.10.2013 */

#ifndef _ERRORS_H_
#define _ERRORS_H_

#include <stdio.h>

#define ERROR(a, b) a,
typedef enum tag_error_code_t
{
  #include "errors.txt"
} error_code_t;
#undef  ERROR

void errorPrint( FILE *outStream, error_code_t code );

#endif /* _ERRORS_H_ */
