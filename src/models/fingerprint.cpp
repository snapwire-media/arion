//------------------------------------------------------------------------------
//
// Copyright (c) 2015-2016 Paul Filitchkin, Snapwire
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

#include "models/fingerprint.hpp"
#include "utils/utils.hpp"

#include <iostream>
#include <string>
#include <ostream>
#include <fstream>

// Boost
#include <boost/exception/info.hpp>
#include <boost/exception/error_info.hpp>
#include <boost/exception/all.hpp>
#include <boost/foreach.hpp>

// OpenCV
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>

// Exiv2
#include <exiv2/exiv2.hpp>

using boost::property_tree::ptree;
using namespace cv;
using namespace std;

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
Fingerprint::Fingerprint() :
    Operation(),
    mStatus(FingerprintStatusDidNotTry),
    mErrorMessage(),
    mType(FingerprintTypeInvalid),
    mpPixelMd5(0)
{
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
Fingerprint::~Fingerprint()
{
  if (mpPixelMd5)
  {
    // DELETE
  }
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void Fingerprint::setup(const ptree& params)
{
  // Make a copy from the const reference
  mParams = ptree(params);
  
  readType(params);
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
bool Fingerprint::getStatus() const
{
  return mStatus;
}

//------------------------------------------------------------------------------
// Manually set the fingerprint type
//------------------------------------------------------------------------------
void Fingerprint::setType(const std::string& type)
{
  decodeType(type);
}

//------------------------------------------------------------------------------
// Private helper for reading type from a parameter tree (from JSON)
//------------------------------------------------------------------------------
void Fingerprint::readType(const ptree& params)
{
  try
  {
    string type = params.get<std::string>("type");
    
    // Make sure it's lowercase
    transform(type.begin(), type.end(), type.begin(), ::tolower);
    
    decodeType(type);
    
  }
  catch (boost::exception& e)
  {
    // Required, but output error during run()
  }
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void Fingerprint::decodeType(const std::string& type)
{
  if (type == "md5")
  {
    mType = FingerprintTypeMD5;
  }
  else
  {
    // Invalid
    mType = FingerprintTypeInvalid;
  }
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
bool Fingerprint::run()
{
  mStatus = FingerprintStatusPending;
  
  if (mImage.empty())
  {
    return false;
  }
  
  if (mType == FingerprintTypeMD5)
  {
    //--------------------------------
    //      Compute Image MD5
    //--------------------------------
    mpPixelMd5 = Utils::computeMd5((char*)mImage.data, (int)mImage.step[0] * mImage.rows);

    if (mpPixelMd5)
    {
      mStatus = FingerprintStatusSuccess;
      return true;
    }
  }
  
  return false;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
#ifdef JSON_PRETTY_OUTPUT
void Fingerprint::serialize(rapidjson::PrettyWriter<rapidjson::StringBuffer>& writer) const
#else
void Fingerprint::serialize(rapidjson::Writer<rapidjson::StringBuffer>& writer) const
#endif
{
  writer.StartObject();

  // Result
  writer.String("type");
  writer.String("fingerprint");

  if (mStatus == FingerprintStatusSuccess)
  {
    // Result
    writer.String("result");
    writer.Bool(true);
    
    // md5 of pixels
    writer.String("md5");
    writer.String(mpPixelMd5);

  }
  else
  {
    // Result
    writer.String("result");
    writer.Bool(false);

    // Error message
    if ((mStatus == FingerprintStatusError) &&  !mErrorMessage.empty())
    {
      writer.String("error_message");
      writer.String(mErrorMessage);
    }
  }

  writer.EndObject();
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
bool Fingerprint::getJpeg(std::vector<unsigned char>& data)
{
  return false;
}
