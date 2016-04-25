// Local
#include "arion.hpp"
#include "models/resize.hpp"
#include "carion.h"
#include <stdio.h>
#include <string.h>

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
char* returnChars(const std::string& string)
{
  const char* localOutputJson = string.c_str();
  
  // Create on the heap
  char* outputJson = (char*)malloc(strlen(localOutputJson));
  
  strcpy(outputJson, localOutputJson);
  
  return outputJson;
}

//------------------------------------------------------------------------------
// C wrapper for running Arion. The input is a JSON formatted string
// WARNING: Don't forget to free the output memory!
//------------------------------------------------------------------------------
const char* ArionRunJson(const char* inputJsonChar)
{
  std::string inputJson(inputJsonChar);
  
  Arion arion;
  
  if (!arion.setup(inputJson))
  {
    return returnChars(arion.getJson());
  }
  
  arion.run();

  return (const char*)returnChars(arion.getJson());
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
struct ArionResult ArionResize(struct ArionInputOptions inputOptions,
                               struct ArionResizeOptions resizeOptions)
{
  struct ArionResult result;
  std::string inputUrl = std::string(inputOptions.inputUrl);
  std::vector<unsigned char> buffer;
  
  Arion arion;
  arion.setInputUrl(inputUrl);
  arion.setCorrectOrientation(true);
  arion.addResizeOperation(resizeOptions);
  arion.run();
  arion.getJpeg(buffer);
  
  result.outputData = (unsigned char *)malloc(buffer.size());
  result.resultJson = returnChars(arion.getJson());
  
  // TODO: is there a way without this memcpy?
  memcpy(result.outputData, &buffer[0], buffer.size());
  
  result.outputSize = buffer.size();
  
  return result;
}
