/* Sergeev Artemiy, 33601/2 (3057/2), 07.10.2013 */

#include <string.h>
#include "commands.h"

#define COMMAND(a, b, c) {a, b, c},
static command_t s_CommandsTable[] = 
{
  #include "commands.txt"
};
#undef COMMAND

#define OPTION(a, b, c) {a, b, c},
static option_t s_OptionsTable[] = 
{
  #include "options.txt"
};
#undef OPTION

/*** Commands ***/

error_code_t commandHelp( alg_parameters_t parameters )
{
  return ERROR_SUCCESS;
}

error_code_t commandExit( alg_parameters_t parameters )
{
  return ERROR_SUCCESS;
}

error_code_t commandEncode( alg_parameters_t parameters )
{
  return ERROR_SUCCESS;
}

error_code_t commandDecode( alg_parameters_t parameters )
{
  return ERROR_SUCCESS;
}

/*** Options ***/

int optionT( int argc, char *argv[], alg_parameters_t *parameters )
{
  parameters->codingFileName = argv[0];
  return 1;
}


/*** Parsing functions ***/

static error_code_t s_parseOption( char *name, option_t **option )
{
  unsigned int i;
  size_t       currentLength, 
               tableSize = sizeof(s_OptionsTable) / sizeof(s_OptionsTable[0]);

  for (i = 0; i < tableSize; i++)
  {
    currentLength = strlen(s_OptionsTable[i].name);
    if (strncmp(s_OptionsTable[i].name, name, currentLength) == 0)
    {
      *option = &s_OptionsTable[i];
      return ERROR_SUCCESS;
    }
  }
  *option = NULL;
  return ERROR_OPT_NOT_FOUND;
}

static error_code_t s_parseCommand( char *name, command_t **command )
{
  unsigned int i;
  size_t       currentLength, 
               tableSize = sizeof(s_CommandsTable) / sizeof(s_CommandsTable[0]);

  for (i = 0; i < tableSize; i++)
  {
    currentLength = strlen(s_CommandsTable[i].name);
    if (strncmp(s_CommandsTable[i].name, name, currentLength) == 0)
    {
      *command = &s_CommandsTable[i];
      return ERROR_SUCCESS;
    }
  }
  *command = NULL;
  return ERROR_COMMAND_NOT_FOUND;
}

error_code_t parseArguments( int argc, char *argv[] )
{
  command_t        *command;
  option_t         *option;
  unsigned int     argumentIndex = 1;
  alg_parameters_t parameters = {0};

  if (argc < 3 || argc > 6)
    return ERROR_COMMAND_INVALID_PARAMETERS_COUNT;

  /* Try to determine command */
  if (s_parseCommand(argv[argumentIndex++], &command) != ERROR_SUCCESS)
    return ERROR_COMMAND_NOT_FOUND;
  /* Try to determine and proceed option */
  if (s_parseOption(argv[argumentIndex], &option) == ERROR_SUCCESS)
  {
    ++argumentIndex;
    if (option->optionFunc)
      argumentIndex += option->optionFunc(argc - argumentIndex, argv + argumentIndex, &parameters);
  }
  /* Try to proceed parameters */
  if (argc - argumentIndex == 2)
  {
    parameters.inputFileName  = argv[argumentIndex++];
    parameters.outputFileName = argv[argumentIndex];
  }
  else 
    return ERROR_COMMAND_INVALID_PARAMETERS_COUNT;

  return command->commandFunc(parameters);
}
