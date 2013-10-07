/* Sergeev Artemiy, 33601/2 (3057/2), 07.10.2013 */

#include "errors.h"

#define ERROR(a, b) b,
static char *s_Errors[] = 
{
  #include "errors.txt"
};
#undef ERROR

void errorPrint( FILE *outStream, error_code_t code )
{
  char *outStr = "";

  if (code >= 0 && code < sizeof(s_Errors) / sizeof(s_Errors[0]))
    outStr = s_Errors[code];
  fprintf(outStream, outStr);
}