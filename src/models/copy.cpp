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

#include "models/copy.hpp"
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
Copy::Copy(const ptree& params, string inputFile) :
    Operation(params),
    mOperationTime(0),
    mStatus(CopyStatusDidNotTry),
    mErrorMessage(),
    mInputFile(inputFile)
{

  try
  {
    string outputUrl = params.get<string>("output_url");

    int pos = outputUrl.find(Utils::FILE_SOURCE);

    if (pos != string::npos)
    {
      mOutputFile = Utils::getStringTail(outputUrl, pos + Utils::FILE_SOURCE.length());
    }
  }
  catch (boost::exception& e)
  {
    // TODO: better error handling
    cerr << "ERROR: Could not read the copy operation output url" << endl;
    return;
  }
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
Copy::~Copy()
{
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
bool Copy::getStatus() const
{
  return mStatus;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
bool Copy::run()
{
  boost::timer::cpu_timer timer;

  mStatus = CopyStatusPending;

  std::ifstream src(mInputFile.c_str(),  std::ios::binary);
  std::ofstream dst(mOutputFile.c_str(), std::ios::binary);

  dst << src.rdbuf();

  //--------------------------------
  //  Inherit EXIF data if needed
  //--------------------------------
  if (mpExifData || mpXmpData || mpIptcData)
  {
    try
    {
      // NOTE: writing metadata is split out into separate data types for future
      //       functionality where we may want to inject certain input data into
      //       these formats
      Exiv2::Image::AutoPtr outputExivImage = Exiv2::ImageFactory::open(mOutputFile.c_str());

      if (outputExivImage.get() != 0)
      {
        if (mpExifData)
        {
          // Output image inherits input EXIF data
          outputExivImage->setExifData(*mpExifData);
        }

        if (mpXmpData)
        {
          // Output image inherits input XMP data
          outputExivImage->setXmpData(*mpXmpData);
        }

        if (mpIptcData)
        {
          // Output image inherits input IPTC data
          outputExivImage->setIptcData(*mpIptcData);
        }
      }

      outputExivImage->writeMetadata();

    }
    catch (Exiv2::AnyError& e)
    {
      mStatus = CopyStatusError;
      mErrorMessage = e.what();
      return false;
    }
  }

  mStatus = CopyStatusSuccess;

  typedef boost::chrono::duration<double> sec; // seconds, stored with a double
  sec seconds = boost::chrono::nanoseconds(timer.elapsed().user + timer.elapsed().system);

  mOperationTime = seconds.count();

  return true;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
#ifdef JSON_PRETTY_OUTPUT
void Copy::serialize(rapidjson::PrettyWriter<rapidjson::StringBuffer>& writer) const
#else
void Copy::serialize(rapidjson::Writer<rapidjson::StringBuffer>& writer) const
#endif
{
  writer.StartObject();

  // Result
  writer.String("type");
  writer.String("copy");

  // Output URL
  writer.String("output_url");
  writer.String("file://" + mOutputFile);

  if (mStatus == CopyStatusSuccess)
  {
    // Result
    writer.String("result");
    writer.Bool(true);

    // Time
    writer.String("time");
    writer.Double(mOperationTime);

  }
  else
  {
    // Result
    writer.String("result");
    writer.Bool(false);

    // Error message
    if ((mStatus == CopyStatusError) &&  !mErrorMessage.empty())
    {
      writer.String("error_message");
      writer.String(mErrorMessage);
    }
  }

  writer.EndObject();
}
