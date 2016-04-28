// Local
#include "arion.hpp"
#include "models/resize.hpp"
#include "carion.h"
#include <stdio.h>
#include <string.h>

//------------------------------------------------------------------------------
// Helper function for generating c string on heap
// WARNING: Don't forget to free this memory!
//------------------------------------------------------------------------------
char* getChars(const std::string& string)
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
    return getChars(arion.getJson());
  }
  
  arion.run();

  return (const char*)getChars(arion.getJson());
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
struct ArionResizeResult ArionResize(struct ArionInputOptions inputOptions,
                                     struct ArionResizeOptions resizeOptions)
{
  struct ArionResizeResult result;

  std::vector<unsigned char> buffer;
  
  Arion arion;
    
  std::string inputUrl = std::string(inputOptions.inputUrl);
  
  if (!arion.setInputUrl(inputUrl))
  {
    result.outputData = 0;
    result.outputSize = 0;
    result.resultJson = getChars(arion.getJson());
    result.returnCode = -1;
  }

  arion.setCorrectOrientation(true);
  arion.addResizeOperation(resizeOptions);
  
  // We just passed in one operation, use the 0th index
  int operation = 0;
    
  if (!arion.run())
  {
    result.outputData = 0;
    result.outputSize = 0;
    result.resultJson = getChars(arion.getJson());
    result.returnCode = -1;
    return result;
  }
  
  result.resultJson = getChars(arion.getJson());

  if (!arion.getJpeg(operation, buffer))
  {
    result.outputData = 0;
    result.outputSize = 0;
    result.resultJson = getChars(arion.getJson());
    result.returnCode = -1;
    return result;
  }
  
  result.outputSize = buffer.size();
  result.outputData = (unsigned char *)malloc(buffer.size());
  
  // Get our data onto the heap
  // TODO: is there a way without this memcpy?
  memcpy(result.outputData, &buffer[0], buffer.size());
  
  result.returnCode = 0;
  
  return result;
}
