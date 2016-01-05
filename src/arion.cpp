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
#include "models/read_meta.hpp"
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
#include <boost/optional/optional.hpp>

// OpenCV
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>

// Exiv2
#include <exiv2/exiv2.hpp>

// Stdlib
#include <iostream>
#include <string>
#include <bits/stl_vector.h>

using namespace boost::program_options;
using namespace boost::filesystem;
using boost::property_tree::ptree;
using namespace rapidjson;
using namespace std;

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
Arion::Arion() : 
  mCorrectOrientation(false),
  mpExifData(0),
  mpXmpData(0),
  mpIptcData(0)
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
void Arion::overrideMeta(const ptree& pt)
{
  boost::optional< const ptree& > optionalTree = pt.get_child_optional("write_meta");
  
  if (!optionalTree)
  {
    // child node is missing
    return;
  }
  
  
  const ptree& writemetaTree = optionalTree.get();
  
  if (!mpIptcData)
  {
    mpIptcData = new Exiv2::IptcData();
  }
  
  try
  {
    string caption = writemetaTree.get<string>("caption");
    
    (*mpIptcData)["Iptc.Application2.Caption"] = caption;
  }
  catch (boost::exception& e)
  {
    // Optional
  }
  
  try
  {
    string copyright = writemetaTree.get<string>("copyright");
    
    (*mpIptcData)["Iptc.Application2.Copyright"] = copyright;
  }
  catch (boost::exception& e)
  {
    // Optional
  }
  
  try
  {
    string province_state = writemetaTree.get<string>("province_state");
    
    (*mpIptcData)["Iptc.Application2.ProvinceState"] = province_state;
  }
  catch (boost::exception& e)
  {
    // Optional
  }
  
  try
  {
    string city = writemetaTree.get<string>("city");
    
    (*mpIptcData)["Iptc.Application2.City"] = city;
  }
  catch (boost::exception& e)
  {
    // Optional
  }
  
  try
  {
    string country_name = writemetaTree.get<string>("country_name");
    
    (*mpIptcData)["Iptc.Application2.CountryName"] = country_name;
  }
  catch (boost::exception& e)
  {
    // Optional
  }
}

//------------------------------------------------------------------------------
// Given each input operation do the following:
//  1. Get its type and parameters
//  2. Create the corresponding operation object and place it in the queue
//  3. Provide any additional data to the operation
//------------------------------------------------------------------------------
void Arion::parseOperations(const ptree& pt)
{
  int operationParseCount = 0;

  // Prep all operations before running them
  BOOST_FOREACH (const ptree::value_type& node, pt.get_child("operations"))
  {
    try
    {
      const ptree& operationTree = node.second;
      
      // "type" is not optional, throws exception if missing or unknown
      string type = operationTree.get<std::string>("type");
      
      // Get all of the params for this operation
      // "params" is not optional, throws exception if missing
      const ptree& paramsTree = operationTree.get_child("params");
      
      Operation* operation;

      if (type == "resize")
      {
        // This is a resize operation so create the corresponding object
        operation = new Resize(paramsTree, mSourceImage);
      }
      else if (type == "read_meta")
      {
        // This is a resize operation so create the corresponding object
        operation = new Read_meta(paramsTree);
      }
      else
      {
        stringstream ss;
        ss << "Operation " << type << " not supported";
        
        Utils::exitWithError(ss.str());
      }
      
      // Add to operation queue
      mOperations.push_back(operation);

      if (mpExifData)
      {
        operation->setExifData(mpExifData);
      }

      if (mpXmpData)
      {
        operation->setXmpData(mpXmpData);
      }

      if (mpIptcData)
      {
        operation->setIptcData(mpIptcData);
      }
      
      operationParseCount++;

    }
    catch (std::exception& e)
    {

      stringstream ss;

      ss << "Count not parse operation " << (operationParseCount+1) << " : " << e.what();

      Utils::exitWithError(ss.str());

    }
  }
}

//--------------------------------
//   Extract Image Meta Data
//--------------------------------
void Arion::extractMetadata(const string& imageFilePath)
{
  try
  {
    mExivImage = Exiv2::ImageFactory::open(imageFilePath.c_str());

    if (mExivImage.get() != 0)
    {
      mExivImage->readMetadata();

      Exiv2::ExifData& exifData = mExivImage->exifData();

      if (!exifData.empty())
      {
        mpExifData = &exifData;

#if DEBUG
        Utils::exifDebug(exifData);
#endif

        if (mCorrectOrientation)
        {
          handleOrientation(exifData, mSourceImage);
        }
      }

      Exiv2::XmpData& xmpData = mExivImage->xmpData();

      if (!xmpData.empty())
      {
        mpXmpData = &xmpData;

#if DEBUG
        Utils::xmpDebug(xmpData);
#endif
      }

      Exiv2::IptcData& iptcData = mExivImage->iptcData();

      if (!iptcData.empty())
      {
        mpIptcData = &iptcData;

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

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void Arion::extractImage(const string& imageFilePath)
{
  try
  {
    mSourceImage = cv::imread(imageFilePath);

    if (mSourceImage.empty())
    {
      Utils::exitWithError("Could not read source image");
    }

    //--------------------------------
    //      Compute Image MD5
    //--------------------------------

    // We need this until the very end, so memory deallocation will be handled
    // when the program exits and the heap is deallocated
    mpPixelMd5 = Utils::computeMd5((char*)mSourceImage.data, (int)mSourceImage.step[0] * mSourceImage.rows);

  }
  catch (boost::exception& e)
  {
    stringstream ss;

#if DEBUG
    ss << "Error reading image " << diagnostic_information(e);
#endif

    Utils::exitWithError(ss.str());
  }
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void Arion::run(const string& inputJson)
{
  boost::timer::cpu_timer timer;
  
  //----------------------------------
  //       Parse JSON Input
  //----------------------------------
  std::stringstream ss(inputJson);

  ptree inputTree;

  boost::property_tree::read_json(ss, inputTree);

  string imageFilePath;

  // We always operate on a single input image
  string imageUrl = inputTree.get<std::string>("input_url");

  int pos = imageUrl.find(Utils::FILE_SOURCE);

  if (pos != string::npos)
  {
    imageFilePath = Utils::getStringTail(imageUrl, pos + Utils::FILE_SOURCE.length());
  }
  else
  {
    Utils::exitWithError("Unsupported input source. Use 'file://' prefix");
  }
  
  //--------------------------------
  //   Correct orientation flag
  //--------------------------------
  try
  {
    mCorrectOrientation = inputTree.get<bool>("correct_rotation");
  }
  catch (boost::exception& e)
  {
    // Not required
  }

  //----------------------------------
  //        Preprocessing
  //----------------------------------
  extractImage(imageFilePath);
  
  extractMetadata(imageFilePath);

  parseOperations(inputTree);
  
  overrideMeta(inputTree);
  
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
  writer.Uint(mSourceImage.rows);
  
  writer.String("width");
  writer.Uint(mSourceImage.cols);
  
  // md5 of pixels
  writer.String("md5");
  writer.String(mpPixelMd5);
  
  int total_operations = 0;
  int failed_operations = 0;
  
  writer.String("info");
  writer.StartArray();
    
  BOOST_FOREACH (Operation* operation, mOperations)
  {
    try
    {
      if (!operation->run())
      {
        failed_operations++;
      }
      
      total_operations++;
 
      operation->serialize(writer);
      
      delete operation;
      
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
