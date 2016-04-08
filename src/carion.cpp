// Local
#include "arion.hpp"
#include "carion.h"

const char* ArionRun(const char* inputJson)
{
  Arion* arion = new Arion();
  
  std::string inputJsonString(inputJson);
  
  arion->run(inputJsonString);
  
  delete arion;
  
  return "done";
}
