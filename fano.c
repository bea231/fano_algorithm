/* Sergeev Artemiy, 33601/2 (3057/2), 07.10.2013 */

#include <ctype.h>
#include <setjmp.h>
#include <stdlib.h>
#include "bitrw.h"
#include "fano.h"

#define _ABS(x) ((x) > 0 ? (x) : -(x))
#define EPSILON 1e-8

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
    unsigned int median = s_getMedian(probTable, begin, end), i, length;

    for (i = begin; i <= end; i++)
    { 
      length = codes[probTable[i].character].length;

      codes[probTable[i].character].code[length / 8] |= ((i > median) << (length % 8));
      ++codes[probTable[i].character].length;
    }

    s_fanoAlgorithm(probTable, codes, begin, median);
    s_fanoAlgorithm(probTable, codes, median + 1, end);
  }
}

static void s_fanoBuildMedians( probability_char_t *probTable, int *medians, int medianIndex, int begin, int end )
{
  if (end > begin)
  {
    medians[medianIndex] = s_getMedian(probTable, begin, end);

    s_fanoBuildMedians(probTable, medians, 2 * medianIndex + 1, begin, medians[medianIndex]);
    s_fanoBuildMedians(probTable, medians, 2 * medianIndex + 2, medians[medianIndex] + 1, end);
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

    if (probTable[character].character == 0)
       numOfCharacters++;
    else
      totalProbability -= probTable[character].probability;

    probTable[character].character = character;
    totalProbability += probTable[character].probability = strtod(buf, &tmp);

    if (tmp == buf || probTable[character].probability <= 0 || probTable[character].probability > 1 + EPSILON)
      return ERROR_FANO_CODE_INVALID_FILE_FORMAT;
    while (isspace(character = fgetc(codesFile)) && character != '\n' && character != EOF)
      if (ferror(codesFile))
        return ERROR_FANO_CODE_INVALID_FILE_FORMAT;
  }

  if (totalProbability > 1 + EPSILON || totalProbability < 1 - EPSILON)
    return ERROR_FANO_CODE_INVALID_FILE_FORMAT;

  if (numOfCharacters < MAX_CHARACTERS_NUM - 1)
  {
    probability = (1.0 - totalProbability) / (MAX_CHARACTERS_NUM - numOfCharacters - 1);
    for (i = 1; i < MAX_CHARACTERS_NUM; i++)
    {
      if (probTable[i].character == 0)
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
  s_QuickSort(probTable + 1, MAX_CHARACTERS_NUM);
  s_fanoAlgorithm(probTable, codes, 1, MAX_CHARACTERS_NUM - 1);


  bitWriteInit(outputFile);
  while((character = fgetc(inputFile)) != EOF)
  {
    for (i = 0; i < codes[character].length; i++)
    {
      bitWrite((codes[character].code[i / 8] & (1 << (i % 8))) > 0 ? 1 : 0);
      bitsCount++;
    }
  }
  bitWriteClose();
  fwrite(&bitsCount, sizeof(unsigned long), 1, outputFile);
  longjmp(jmpBuf, ERROR_SUCCESS);
}

error_code_t fanoDecode( alg_parameters_t parameters )
{
  jmp_buf            jmpBuf;
  error_code_t       errorJumpCode;
  unsigned int       i;
  int                character,
                     curCode, curMedian, maxCode,
                     medians[MAX_TREE_LEVEL * MAX_CHARACTERS_NUM] = {0};
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
  s_QuickSort(probTable + 1, MAX_CHARACTERS_NUM);
  s_fanoBuildMedians(probTable, medians, 0, 1, MAX_CHARACTERS_NUM - 1);

  fseek(inputFile, -(long)sizeof(unsigned long), SEEK_END);
  fread(&bitsCount, sizeof(unsigned long), 1, inputFile);
  if (ferror(inputFile))
    longjmp(jmpBuf, ERROR_FANO_DECODE_INVALID_FILE_FORMAT);
  fseek(inputFile, 0, SEEK_SET);

  bitReadInit(inputFile);
  curMedian = 0;
  curCode = 1;
  maxCode = MAX_CHARACTERS_NUM - 1;
  for (i = 0; i < bitsCount; i++)
  {
    character = bitRead();
    if (character == 1)
    {
      curCode   = medians[curMedian] + 1;
      curMedian = 2 * curMedian + 2;
    }
    else if (character == 0)
    {
      maxCode   = medians[curMedian];
      curMedian = 2 * curMedian + 1;
    }
    else
      longjmp(jmpBuf, ERROR_FANO_DECODE_INVALID_FILE_FORMAT);

    if (maxCode <= curCode)
    {
      fputc(probTable[curCode].character, outputFile);
      curMedian = 0;
      curCode = 1;
      maxCode = MAX_CHARACTERS_NUM - 1;
      continue;
    }

    if (curMedian > sizeof(medians) / sizeof(medians[0]) || ferror(inputFile))
      longjmp(jmpBuf, ERROR_FANO_DECODE_INVALID_FILE_FORMAT);
  }

  longjmp(jmpBuf, ERROR_SUCCESS);
  return ERROR_SUCCESS;
}