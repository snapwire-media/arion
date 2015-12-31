//------------------------------------------------------------------------------
//
// Arion
//
// Extract metadata and create beautiful thumbnails of your images.
//
// ------------
//  arion.cpp
// ------------
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

// Local
#include "models/operation.hpp"
#include "models/resize.hpp"
#include "models/readmeta.hpp"
#include "utils/utils.hpp"
#include "arion.hpp"

// Local Third party
#include "thirdparty/rapidjson/writer.h"
#include "thirdparty/rapidjson/prettywriter.h"
#include "thirdparty/rapidjson/stringbuffer.h"

// Boost
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/exception/info.hpp>
#include <boost/exception/error_info.hpp>
#include <boost/exception/all.hpp>
#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>
#include <boost/program_options.hpp>
#include <boost/thread.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/timer/timer.hpp>

// OpenCV
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>

// Exiv2
#include <exiv2/exiv2.hpp>

// Stdlib
#include <iostream>
#include <string>

using namespace boost::program_options;
using namespace boost::filesystem;
using boost::property_tree::ptree;
using namespace rapidjson;
using namespace std;

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
Arion::Arion()
{
}

//------------------------------------------------------------------------------
// Return true if image was rotated, false otherwise
//------------------------------------------------------------------------------
bool Arion::handleOrientation(Exiv2::ExifData& exifData, cv::Mat& image)
{

  Exiv2::ExifKey key("Exif.Image.Orientation");

  Exiv2::ExifData::iterator pos = exifData.findKey(key);

  if (pos == exifData.end())
  {
    return false;
  }

  long orientation = pos->toLong();

  switch(orientation)
  {
    case 1: // normal (do nothing)
      break;

    case 2: // horizontal flip
    {
      cv::flip(image, image, 1);

      return true;
    }

    case 3: // 180 rotate left (two left rotations)
    {
      // TODO: even though this is rare it needs to be more efficient
      cv::transpose(image, image);
      cv::flip(image, image, 0);
      cv::transpose(image, image);
      cv::flip(image, image, 0);

      return true;
    }

    case 4: // vertical flip
    {
      cv::flip(image, image, 0);

      return true;
    }

    case 5: // vertical flip + 90 rotate right
    {
      cv::flip(image, image, 0);
      cv::transpose(image, image);
      cv::flip(image, image, 1);

      return true;
    }

    case 6: // 90 rotate right x
    {
      cv::transpose(image, image);
      cv::flip(image, image, 1);

      return true;
    }

    case 7: // horizontal flip + 90 rotate right
    {
      cv::flip(image, image, 1);
      cv::transpose(image, image);
      cv::flip(image, image, 1);

      return true;
    }

    case 8: // 90 rotate left
    {
      cv::transpose(image, image);
      cv::flip(image, image, 0);

      return true;
    }

    default:
      break;
  }

  return false;

}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void Arion::run(const string& inputJson)
{
  boost::timer::cpu_timer timer;

  vector<Operation> operations;
  bool hasValidExif = false;
  bool hasValidXmp = false;
  bool hasValidIptc = false;
  
  //----------------------------------
  //       Parse JSON Input
  //----------------------------------
  std::stringstream ss(inputJson);

  ptree pt;

  boost::property_tree::read_json(ss, pt);

  bool correctOrientation = false;
  string imageFilePath;
  cv::Mat sourceImage;
  char* md5;

  Exiv2::Image::AutoPtr exivImage;

  try
  {
    //--------------------------------
    //         Extract Image
    //--------------------------------
    // We always operate on a single input image
    string imageUrl = pt.get<std::string>("input_url");

    int pos = imageUrl.find(Utils::FILE_SOURCE);

    if (pos != string::npos)
    {
      imageFilePath = Utils::getStringTail(imageUrl, pos + Utils::FILE_SOURCE.length());
    }
    else
    {
      Utils::exitWithError("Unsupported input source. Use 'file://' prefix");
    }

    sourceImage = cv::imread(imageFilePath);

    if (sourceImage.empty())
    {
      Utils::exitWithError("Could not read source image");
    }

    //--------------------------------
    //      Compute Image MD5
    //--------------------------------

    md5 = Utils::computeMd5((char*)sourceImage.data, (int)sourceImage.step[0] * sourceImage.rows);

    //--------------------------------
    //   Correct orientation flag
    //--------------------------------
    try
    {
      correctOrientation = pt.get<bool>("correct_rotation");
    }
    catch (boost::exception& e)
    {
      // Not required
    }

    //--------------------------------
    //    Extract Image Meta Data
    //--------------------------------

    try
    {
      exivImage = Exiv2::ImageFactory::open(imageFilePath.c_str());

      if (exivImage.get() != 0)
      {
        exivImage->readMetadata();

        Exiv2::ExifData& exifData = exivImage->exifData();

        if (!exifData.empty())
        {
          // Got this far so we have at least some valid EXIF data...
          hasValidExif = true;

#if DEBUG
          Utils::exifDebug(exifData);
#endif

          if (correctOrientation)
          {
            handleOrientation(exifData, sourceImage);
          }
        }

        Exiv2::XmpData& xmpData = exivImage->xmpData();

        if (!xmpData.empty())
        {

          hasValidXmp = true;

#if DEBUG
          Utils::xmpDebug(xmpData);
#endif
        }
        
        Exiv2::IptcData& iptcData = exivImage->iptcData();

        if (!iptcData.empty())
        {
          hasValidIptc = true;

#if DEBUG
          Utils::iptcDebug(iptcData);
#endif
        }
      }
    }
    catch (Exiv2::AnyError& e)
    {
      // Not the end of the world if reading EXIF data failed
      //cerr << "ERROR: Caught Exiv2 exception '" << e.what() << "'\n";
      //return -1;
    }
  }
  catch (boost::exception& e)
  {
    stringstream ss;

#if DEBUG
    ss << "Error reading image " << diagnostic_information(e);
#endif

    Utils::exitWithError(ss.str());
  }

  int operationParseCount = 0;

  // Make sure we understand all operations before proceeding
  BOOST_FOREACH (const ptree::value_type& node, pt.get_child("operations"))
  {
    try
    {
      const ptree& operationProps = node.second;

      Operation newOperation(operationProps);

      operations.push_back(newOperation);

      operationParseCount++;

    }
    catch (std::exception& e)
    {

      stringstream ss;

      ss << "Count not parse operation " << (operationParseCount+1) << " : " << e.what();

      Utils::exitWithError(ss.str());

    }
  }

  //----------------------------------
  //       Execute operations
  //----------------------------------
  StringBuffer s;
    
  #ifdef JSON_PRETTY_OUTPUT
    PrettyWriter<StringBuffer> writer(s);
  #else
    Writer<StringBuffer> writer(s);
  #endif

  writer.StartObject();

  // Dimensions
  writer.String("height");
  writer.Uint(sourceImage.rows);
  writer.String("width");
  writer.Uint(sourceImage.cols);
  
  // md5 of pixels
  writer.String("md5");
  writer.String(md5);
  
  int total_operations = 0;
  int failed_operations = 0;
  
  writer.String("info");
  writer.StartArray();
  
  BOOST_FOREACH (Operation& operation, operations)
  {
    try
    {
      switch (operation.getType())
      {
        case OperationTypeResize:
        {
          Resize r(operation.getParams());

          if (hasValidExif)
          {
            r.setExifData(&exivImage->exifData());
          }

          if (hasValidXmp)
          {
            r.setXmpData(&exivImage->xmpData());
          }
          
          if (hasValidIptc)
          {
            r.setIptcData(&exivImage->iptcData());
          }

          if (!r.run(sourceImage))
          {
            failed_operations++;
          }
          
          total_operations++;
 
          r.Serialize(writer);

          break;
        }
        case OperationTypeReadmeta:
        {
          Readmeta r(operation.getParams());

          if (hasValidExif)
          {
            r.setExifData(&exivImage->exifData());
          }

          if (hasValidXmp)
          {
            r.setXmpData(&exivImage->xmpData());
          }
          
          if (hasValidIptc)
          {
            r.setIptcData(&exivImage->iptcData());
          }

          if (!r.run())
          {
            failed_operations++;
          }
          
          total_operations++;
          
          r.Serialize(writer);

          break;
        }
        default:
          // This should never actually occur since an invalid
          // operation would throw an exception earlier...
          //cout << "Unknown operation... skipping" << endl;
          break;
      }
    }
    catch (std::exception& e)
    {
      // Break out of the standard output and exit with error
      Utils::exitWithError(e.what());
    }
  }
  
  writer.EndArray();

  typedef boost::chrono::duration<double> sec; // seconds, stored with a double
  sec seconds = boost::chrono::nanoseconds(timer.elapsed().user + timer.elapsed().system);
  
  // Result of command (all operations must succeed to get true)
  writer.String("result");
  
  if (failed_operations == 0)
  {
    writer.Bool(true);
  }
  else
  {
    writer.Bool(false);
  }
  
  // Operation stats
  writer.String("total_operations");
  writer.Uint(total_operations);
  
  writer.String("failed_operations");
  writer.Uint(failed_operations);

  // Time
  writer.String("time");
  writer.Double(seconds.count());  
  writer.EndObject();
  
  // Final successful output
  cout << s.GetString() << endl;
  
  exit(0);
}
