// Local
#include "arion.hpp"
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
//  std::string inputJson(inputJsonChar);
//  
//  Arion arion;
//
//  arion.run();
//  
//  const char* localOutputJson = arion.getJson().c_str();
//  
//  // Create on the heap
//  char* outputJson = (char*)malloc(strlen(localOutputJson));
//  
//  strcpy(outputJson, localOutputJson);
//  
//  return outputJson;
  
  return (const char*)0;
}
