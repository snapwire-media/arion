//------------------------------------------------------------------------------
//
// Copyright (c) 2015 Paul Filitchkin, Snapwire
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//    * Redistributions of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//
//    * Redistributions in binary form must reproduce the above copyright
//      notice, this list of conditions and the following disclaimer in
//      the documentation and/or other materials provided with the
//      distribution.
//
//    * Neither the name of the organization nor the names of its contributors
//      may be used to endorse or promote products derived from this software
//      without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
// OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
// WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
// OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
// ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//------------------------------------------------------------------------------

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
  // "type" is not optional, throws exception if missing or unknown
  string type = pt.get<std::string>("type");

  if (type == "resize")
  {
    mType = OperationTypeResize;
  }
  else if (type == "readmeta")
  {
    mType = OperationTypeReadmeta;
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
