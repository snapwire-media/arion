// Local
#include "arion.hpp"
#include "carion.h"
#include <stdio.h>
#include <string.h>

//------------------------------------------------------------------------------
// C wrapper for running Arion. The input is a JSON formatted string
// WARNING: Don't forget to free the output memory!
//------------------------------------------------------------------------------
const char* ArionRun(const char* inputJsonChar)
{
  std::string inputJson(inputJsonChar);
  
  Arion arion;

  arion.run(inputJson);
  
  const char* localOutputJson = arion.getJson().c_str();
  
  // Create on the heap
  char* outputJson = (char*)malloc(strlen(localOutputJson));
  
  strcpy(outputJson, localOutputJson);
  
  return outputJson;
}
