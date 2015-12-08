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
    mCaption(""),
    mCopyright("")
{
  try
  {
    mReadInfo = params.get<bool>("read_info");
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
//------------------------------------------------------------------------------
void Readmeta::readIptc()
{
  if (mpIptcData == 0)
  {
    return;
  }
  
  if (!mReadInfo)
  {
    return;
  }
  
  const Exiv2::IptcData &iptcData = *mpIptcData;
  
  Exiv2::IptcData::const_iterator end = iptcData.end();
  
  for (Exiv2::IptcData::const_iterator md = iptcData.begin(); md != end; ++md)
  {
    if (md->key() == "Iptc.Application2.Keywords")
    {
      string keyword = md->toString();
      
      if (!keyword.empty())
      {
        mKeywords.push_back(keyword);
      }
    }
    
    if (md->key() == "Iptc.Application2.Caption")
    {
      string caption = md->toString();
      
      if (!caption.empty())
      {
        mCaption = caption;
      }
    }
    
    if (md->key() == "Iptc.Application2.Copyright")
    {
      string copyright = md->toString();
      
      if (!copyright.empty())
      {
        mCopyright= copyright;
      }
    }
    
    if (md->key() == "Iptc.Application2.SpecialInstructions")
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
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void Readmeta::outputStatus(ostream& s, unsigned indent) const
{
  string p = string(indent, ' ');

  s << p << "{" << endl;
  s << p << "  \"type\": \"metadata\"," << endl;

  if (mStatus == ReadmetaStatusSuccess)
  {
    s << p << "  \"result\": true," << endl;
    s << p << "  \"time\": " << mOperationTime;
    
    if (mReadInfo)
    {
      s << "," << endl;
      s << p << "  \"model_released\": " << (mModelReleased ? "true" : "false") << "," << endl;
      s << p << "  \"property_released\": " << (mPropertyReleased ? "true" : "false") << "," << endl;
      s << p << "  \"copyright\": \"" << mCopyright << "\"," << endl;
      s << p << "  \"caption\": \"" << mCaption << "\"";

      if (!mKeywords.empty())
      {
        int keyword_count = 0;

        s << "," << endl;
        s << p << "  \"keywords\": [";

        BOOST_FOREACH (const string& keyword, mKeywords)
        {
          if (keyword_count > 0)
          {
            s << ',';
          }

          s << "\"" << keyword << "\"";

          keyword_count++;
        }

        s << "]" << endl;
      }
      else
      {
        s << endl;
      }
    }
    else
    {
      s << endl;
    }
  }
  else
  {
    s << p << "  \"result\": false," << endl;

    if ((mStatus == ReadmetaStatusError) && !mErrorMessage.empty())
    {
      s << p << "  \"error_message\": \"" << mErrorMessage << "\"," << endl;
    }
  }

  s << p << "}";

  // NOTE: Tried doing property tree -> JSON, but this doesn't quite work...
  //       In addition BOOST outputs everything as a string

}
