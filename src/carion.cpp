// Local
#include "arion.hpp"
#include "models/resize.hpp"
#include "carion.h"
#include <stdio.h>
#include <string.h>

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
const char* returnChars(const std::string& string)
{
  const char* localOutputJson = string.c_str();
  
  // Create on the heap
  char* outputJson = (char*)malloc(strlen(localOutputJson));
  
  strcpy(outputJson, localOutputJson);
  
  return (const char*)outputJson;
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

  return returnChars(arion.getJson());
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
const char* ArionResize(struct ArionInputOptions inputOptions,
                        struct ArionResizeOptions resizeOptions)
{  

  
  Arion arion;
  std::string inputUrl = std::string(inputOptions.inputUrl);
  arion.setInputUrl(inputUrl);
  arion.setCorrectOrientation(true);
  
  arion.addResizeOperation(resizeOptions);

  arion.run();
  
  //resize.getJpeg();
  
  return (const char*)0;
}
