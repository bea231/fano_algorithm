/* Sergeev Artemiy, 33601/2 (3057/2), 07.10.2013 */

#include <stdio.h>

/* Debug memory check support */
#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#  include <crtdbg.h>
#  define SetDbgMemHooks()                                         \
  _CrtSetDbgFlag(_CRTDBG_LEAK_CHECK_DF | _CRTDBG_CHECK_ALWAYS_DF | \
  _CRTDBG_ALLOC_MEM_DF | _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG))
#else
  #define SetDbgMemHooks() 0
#endif /* _DEBUG */

int main( void )
{
  SetDbgMemHooks();
  return 0;
}