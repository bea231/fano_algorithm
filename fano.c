/* Sergeev Artemiy, 33601/2 (3057/2), 07.10.2013 */

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

error_code_t fanoEncode( alg_parameters_t parameters )
{
  return ERROR_SUCCESS;
}

error_code_t fanoDecode( alg_parameters_t parameters )
{
  return ERROR_SUCCESS;
}