#include "models/operation.hpp"

#include <iostream>
#include <string>
#include <ostream>

#include <boost/exception/info.hpp>
#include <boost/exception/error_info.hpp>
#include <boost/exception/all.hpp>
#include <boost/foreach.hpp>

using boost::property_tree::ptree;
using namespace std;

class OperationTypeException: public std::exception
{
  virtual const char* what() const throw()
  {
    return "Invalid operation type";
  }
} OperationTypeException;

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
Operation::Operation(const ptree& pt) : mType()
{
  // "type" is not optional, throws execption if missing
  string type = pt.get<std::string>("type");

  if (type == "resize")
  {
    mType = OperationTypeResize;
  }
  else
  {
    // Type not supported...
    throw OperationTypeException;
  }

  // Get all of the params for this operation
  // "params" is not optional, throws execption if missing
  const ptree& paramsNode = pt.get_child("params");

  // Make a copy from the const reference
  mParams = ptree(paramsNode);
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
const unsigned Operation::getType() const
{
  return mType;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
ptree Operation::getParams() const
{
  return mParams;
}
