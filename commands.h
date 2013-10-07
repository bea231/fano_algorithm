/* Sergeev Artemiy, 33601/2 (3057/2), 07.10.2013 */

#ifndef _COMMANDS_H_
#define _COMMANDS_H_

#include "fano.h"
#include "errors.h"

/* Command representation type */
typedef struct tag_command
{
  char *name;                                              // Command name

  // Pointer to command's execution function
  error_code_t (*commandFunc)( alg_parameters_t parameters );

  char *help;                                              // Comment to command
} command_t;

/* Option representation type */
typedef struct tag_option
{
  char *name;                                                           // Command name

  // Pointer to command's execution function
  int (*optionFunc)( int argc, char *argv[], alg_parameters_t *parameters );

  char *help;                                                           // Comment to command
} option_t;

error_code_t commandHelp( alg_parameters_t parameters );
error_code_t commandExit( alg_parameters_t parameters );
error_code_t commandEncode( alg_parameters_t parameters );
error_code_t commandDecode( alg_parameters_t parameters );

int optionT( int argc, char *argv[], alg_parameters_t *parameters );

error_code_t parseArguments( int argc, char *argv[] );


#endif /* _COMMANDS_H_ */
