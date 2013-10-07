/* Sergeev Artemiy, 33601/2 (3057/2), 07.10.2013 */

#include <ctype.h>
#include <setjmp.h>
#include <stdlib.h>
#include "bitrw.h"
#include "fano.h"

#define _ABS(x) ((x) > 0 ? (x) : -(x))


static unsigned int s_getMedian( probability_char_t *probTable, unsigned int begin, unsigned int end )
{
  double       sumBegin = probTable[begin].probability,
               sumEnd = probTable[end].probability,
               delta;
  unsigned int median = end, i;

  for (i = begin + 1; i < end; i++)
    sumBegin += probTable[i].probability;

  do
  {
    delta = _ABS(sumBegin - sumEnd);
    --median;
    sumBegin -= probTable[median].probability;
    sumEnd   += probTable[median].probability;
  } while (delta > _ABS(sumBegin - sumEnd));
  return median;
}

static void s_fanoAlgorithm( probability_char_t *probTable, binary_code_t *codes, unsigned int begin, unsigned int end )
{
  if (end > begin)
  {
    unsigned int median = s_getMedian(probTable, begin, end), i;

    for (i = begin; i <= end; i++)
      codes[probTable[i].character].code |= ((i > median) << (codes[probTable[i].character].length++));

    s_fanoAlgorithm(probTable, codes, begin, median);
    s_fanoAlgorithm(probTable, codes, median + 1, end);
  }
}

static void s_Swap( probability_char_t *char1, probability_char_t *char2 )
{
  probability_char_t char3;

  char3.probability = char1->probability;
  char3.character = char1->character;
  char1->probability = char2->probability;
  char1->character = char2->character;
  char2->probability = char3.probability;
  char2->character = char3.character;
}

static void s_QuickSort ( probability_char_t *probTable, int size )
{
  int i, j;
  double x;

  if (size < 2)
    return;
  x = probTable[size / 2].probability;
  i = 0;
  j = size - 1;
  while (i <= j)
  {
    while (probTable[i].probability > x)
      i++;
    while (probTable[j].probability < x)
      j--;
    if(i <= j)
    {
      if(i != j)
        s_Swap(&probTable[i], &probTable[j]);
      i++;
      j--;
    }
  }

  s_QuickSort(probTable, j + 1);
  s_QuickSort(probTable + i, size - i);
}

static error_code_t s_initProbabilityTable( probability_char_t *probTable, FILE *codesFile )
{
  int          character;
  unsigned int numOfCharacters  = 0, i;
  double       totalProbability = 0, probability;
  char         buf[50], *tmp;

  while ((character = fgetc(codesFile)) != EOF)
  {
    if (character == '\r' && fgetc(codesFile) == '\n')
    {
      /* Handle a binary representation of '\n' */
      character = '\n';
    }
    else if (character == '\\' && fgetc(codesFile) == 'x')
    {
      /* Read a hex code of character */
      char code[2] = {0};

      if (fgets(code, 2, codesFile) == NULL)
        return ERROR_FANO_CODE_INVALID_FILE_FORMAT;
      
      character = strtoul(code, &tmp, 16);

      if (fgetc(codesFile) != ' ' || tmp == code || character > 255)
        return ERROR_FANO_CODE_INVALID_FILE_FORMAT;
    }

    /* Read a probability of character */
    if ((fscanf(codesFile, "%50s", buf)) <= 0 || ferror(codesFile))
      return ERROR_FANO_CODE_INVALID_FILE_FORMAT;

    if (probTable[character].probability == 0)
       numOfCharacters++;
    else
      totalProbability -= probTable[character].probability;

    probTable[character].character = character;
    totalProbability += probTable[character].probability = strtod(buf, &tmp);

    if (tmp == buf)
      return ERROR_FANO_CODE_INVALID_FILE_FORMAT;
    while (isspace(character = fgetc(codesFile)) && character != '\n' && character != EOF)
      if (ferror(codesFile))
        return ERROR_FANO_CODE_INVALID_FILE_FORMAT;
  }

  if (numOfCharacters < MAX_CHARACTERS_NUM - 1)
  {
    probability = (1.0 - totalProbability) / (MAX_CHARACTERS_NUM - numOfCharacters - 1);
    for (i = 0; i < MAX_CHARACTERS_NUM; i++)
    {
      if (probTable[i].probability == 0)
      {
        probTable[i].character   = i;
        probTable[i].probability = probability;
      }
    }
  }
  return ERROR_SUCCESS;
}

error_code_t fanoEncode( alg_parameters_t parameters )
{
  jmp_buf            jmpBuf;
  error_code_t       errorJumpCode;
  unsigned int       i;
  int                character;
  FILE               *inputFile                    = NULL,
                     *outputFile                   = NULL, 
                     *codesFile                    = NULL;
  probability_char_t probTable[MAX_CHARACTERS_NUM] = {0};
  binary_code_t      codes[MAX_CHARACTERS_NUM]     = {0};
  unsigned long      bitsCount                     = 0;
  

  /* Error handler */
  if (errorJumpCode = setjmp(jmpBuf))
  {  
    if (inputFile)
      fclose(inputFile), inputFile = NULL;
    if (outputFile)
      fclose(outputFile), outputFile = NULL;
    if (codesFile)
      fclose(codesFile), codesFile = NULL;
    return errorJumpCode;
  }

  /* Open some files */
  if ((inputFile = fopen(parameters.inputFileName, "rb")) == NULL)
    longjmp(jmpBuf, ERROR_FANO_FILE_NOT_FOUND);
  if ((outputFile = fopen(parameters.outputFileName, "wb")) == NULL)
    longjmp(jmpBuf, ERROR_FANO_FILE_NOT_FOUND);
  if ((codesFile = fopen(parameters.codingFileName, "rb")) == NULL)
      longjmp(jmpBuf, ERROR_FANO_FILE_NOT_FOUND);

  if ((errorJumpCode = s_initProbabilityTable(probTable, codesFile)) != ERROR_SUCCESS)
    longjmp(jmpBuf, errorJumpCode);
  s_QuickSort(probTable, MAX_CHARACTERS_NUM);
  s_fanoAlgorithm(probTable, codes, 0, MAX_CHARACTERS_NUM - 1);


  bitWriteInit(outputFile);
  while((character = fgetc(inputFile)) != EOF)
  {
    for (i = 0; i < codes[character].length; i++)
    {
      bitWrite((codes[character].code & (1 << i)) > 0 ? 1 : 0);
      bitsCount++;
    }
  }
  bitWriteClose();
  fwrite(&bitsCount, sizeof(unsigned long), 1, outputFile);
  longjmp(jmpBuf, ERROR_SUCCESS);
}

error_code_t fanoDecode( alg_parameters_t parameters )
{
  return ERROR_SUCCESS;
}