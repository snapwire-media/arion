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

#include "models/readmeta.hpp"
#include "utils/utils.hpp"

#include <iostream>
#include <string>
#include <ostream>

#include <sys/time.h>

// Boost
#include <boost/exception/info.hpp>
#include <boost/exception/error_info.hpp>
#include <boost/exception/all.hpp>
#include <boost/foreach.hpp>
#include <boost/timer/timer.hpp>
#include <boost/algorithm/string.hpp>    

// OpenCV
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>

// Exiv2
#include <exiv2/exiv2.hpp>

using boost::property_tree::ptree;
using namespace cv;
using namespace std;
using namespace boost::algorithm;

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
#define MODEL_RELEASED "model released (mr)"
#define PROPERTY_RELEASED "property released (pr)"

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
Readmeta::Readmeta(const ptree& params) :
    mpExifData(0),
    mpXmpData(0),
    mpIptcData(0),
    mPropertyReleased(false),
    mModelReleased(false),
    mReadInfo(false),
    mCaption(""),
    mCopyright("")
{
  try
  {
    mReadInfo = params.get<bool>("info");
  }
  catch (boost::exception& e)
  {
    // Not required
  }
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
bool Readmeta::getStatus() const
{
  return mStatus;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void Readmeta::setExifData(const Exiv2::ExifData* exifData)
{
  mpExifData = exifData;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void Readmeta::setXmpData(const Exiv2::XmpData* xmpData)
{
  mpXmpData = xmpData;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void Readmeta::setIptcData(const Exiv2::IptcData* iptcData)
{
  mpIptcData = iptcData;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
bool Readmeta::run()
{

  boost::timer::cpu_timer timer;

  mStatus = ReadmetaStatusPending;

  readIptc();
  
  mStatus = ReadmetaStatusSuccess;
  
  typedef boost::chrono::duration<double> sec; // seconds, stored with a double
  sec seconds = boost::chrono::nanoseconds(timer.elapsed().user + timer.elapsed().system);

  mOperationTime = seconds.count();
  
  return true;

}

//------------------------------------------------------------------------------
// Sets 
//------------------------------------------------------------------------------
void Readmeta::readIptcStringByKey(Exiv2::IptcData::const_iterator md, 
                                   const string& key,
                                   string* value)
{
  if (md->key() == key)
  {
    string v = md->toString();

    if (!v.empty())
    {
      *value = v;
    }
  }
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void Readmeta::readIptc()
{
  if (mpIptcData == 0)
  {
    return;
  }
  
  // Currently just the read info parameter
  if (!mReadInfo)
  {
    return;
  }

  const string iptcCaptionKey       = "Iptc.Application2.Caption";
  const string iptcKeywordsKey      = "Iptc.Application2.Keywords";
  const string iptcCopyrightKey     = "Iptc.Application2.Copyright";
  const string iptcInstructionsKey  = "Iptc.Application2.SpecialInstructions";
  const string iptcCityKey          = "Iptc.Application2.City";
  const string iptcProvinceStateKey = "Iptc.Application2.ProvinceState";
  const string iptcCountryNameKey   = "Iptc.Application2.CountryName";
  const string iptcCountryCodeKey   = "Iptc.Application2.CountryCode";

  const Exiv2::IptcData &iptcData = *mpIptcData;
  
  Exiv2::IptcData::const_iterator end = iptcData.end();
  
  for (Exiv2::IptcData::const_iterator md = iptcData.begin(); md != end; ++md)
  {
    
    readIptcStringByKey(md, iptcCaptionKey,       &mCaption);
    readIptcStringByKey(md, iptcCopyrightKey,     &mCopyright);
    readIptcStringByKey(md, iptcCityKey,          &mCity);
    readIptcStringByKey(md, iptcProvinceStateKey, &mProvinceState);
    readIptcStringByKey(md, iptcCountryNameKey,   &mCountryName);
    readIptcStringByKey(md, iptcCountryCodeKey,   &mCountryCode);
    
    if (md->key() == iptcKeywordsKey)
    {
      string keyword = md->toString();
      
      if (!keyword.empty())
      {
        mKeywords.push_back(keyword);
      }
    }

    if (md->key() == iptcInstructionsKey)
    {
      string instructions = md->toString();
      
      if (!instructions.empty())
      {
        string instructions_lower = to_lower_copy(instructions);
        
        std::size_t found;
        
        found = instructions_lower.find(MODEL_RELEASED);
        
        if (found != std::string::npos)
        {
          mModelReleased = true;
        }
        
        found = instructions_lower.find(PROPERTY_RELEASED);
        
        if (found != std::string::npos)
        {
          mPropertyReleased = true;
        }
      }
    }
  }
  
//Xmp.photoshop.City                           XmpText     3  Bol
//Xmp.photoshop.State                          XmpText    30  Splitsko-dalmatinska županija
//Xmp.photoshop.Country                        XmpText     7  Croatia
}