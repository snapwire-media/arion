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

// Local
#include "models/operation.hpp"
#include "models/resize.hpp"
#include "models/read_meta.hpp"
#include "models/copy.hpp"
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
#include <boost/optional/optional.hpp>

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
// Exceptions
//------------------------------------------------------------------------------
class ArionImageExtractException: public exception
{
  virtual const char* what() const throw()
  {
    return "Failed to extract image";
  }
} extractException;

class ArionOperationNotSupportedException: public exception
{
  virtual const char* what() const throw()
  {
    return "Operation not supported";
  }
} operationNotSupportedException;

class ArionInputSourceException: public exception
{
  virtual const char* what() const throw()
  {
    return "Unsupported input source. Use 'file://' prefix";
  }
} inputSourceException;

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
Arion::Arion() : 
  mCorrectOrientation(false),
  mpExifData(0),
  mpXmpData(0),
  mpIptcData(0),
  mInputFile(),
  mTotalOperations(0),
  mFailedOperations(0),
  mResult(false)
{
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
Arion::~Arion() 
{
  mpExifData = 0;
  mpXmpData = 0;
  mpIptcData = 0;
  
  mOperations.release();
  
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
bool Arion::setup(const string& inputJson) 
{
  //----------------------------------
  //       Parse JSON Input
  //----------------------------------
  std::stringstream ss(inputJson);

  boost::property_tree::read_json(ss, mInputTree);
  
  try
  {
    // We always operate on a single input image
    string inputUrl = mInputTree.get<std::string>("input_url");
    
    parseInputUrl(inputUrl);
    
  }
  catch (boost::exception& e)
  {
    // Ignore this for now... The input may be set as bytes
  }
  catch (exception& e)
  {
    mResult = false;
    mErrorMessage = e.what();
    
    constructErrorJson();

    return false;
  }
  
  //----------------------------------
  //        Parse operations
  //----------------------------------
  if (!parseOperations(mInputTree))
  {
    mResult = false;
    constructErrorJson();

    return false;
  }
  
  //--------------------------------
  //   Correct orientation flag
  //--------------------------------
  try
  {
    mCorrectOrientation = mInputTree.get<bool>("correct_rotation");
  }
  catch (boost::exception& e)
  {
    // Not required
  }
  
  return true;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void Arion::setSourceImage(cv::Mat& sourceImage)
{
  mSourceImage = sourceImage;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
cv::Mat& Arion::getSourceImage()
{
  return mSourceImage;
}

//------------------------------------------------------------------------------
//  Manually pass in an input URL rather than reading it from JSON
//------------------------------------------------------------------------------
bool Arion::setInputUrl(const string& inputUrl)
{
  try
  {
    parseInputUrl(inputUrl);
  }
  catch (exception& e)
  {
    mResult = false;
    mErrorMessage = e.what();
    
    constructErrorJson();

    return false;
  }
  
  return true;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void Arion::setCorrectOrientation(bool correctOrientation)
{
  mCorrectOrientation = correctOrientation;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void Arion::addResizeOperation(struct ArionResizeOptions options)
{
  // This is a resize operation so create the corresponding object
  Resize* resize = new Resize();

  if (options.algo) 
  {
    string type(options.algo);
    resize->setType(type);
  }

  resize->setHeight(options.height);
  resize->setWidth(options.width);
  
  // Gravity
  if (options.gravity)
  {
    string gravity(options.gravity);
    resize->setGravity(gravity);
  }
  
  // Quality
  resize->setQuality(options.quality);
  
  // Preserve meta data
  if (options.preserveMeta > 0)
  {
    resize->setPreserveMeta(true);
  }
  else
  {
    resize->setPreserveMeta(false);
  }
  
  // Sharpening
  resize->setSharpenAmount(options.sharpenAmount);
  resize->setSharpenRadius(options.sharpenRadius);
  
  // Watermark
  if (options.watermarkUrl)
  {
    string watermarkUrl(options.watermarkUrl);
    resize->setWatermarkUrl(watermarkUrl);
    resize->setWatermarkAmount(options.watermarkAmount);
  }
  
  // Output Url
  if (options.outputUrl)
  {
    string outputUrl = std::string(options.outputUrl);
    resize->setOutputUrl(outputUrl);
  }
  
  // Add to operation queue
  mOperations.push_back(resize);
}

//------------------------------------------------------------------------------
// Helper method for parsing the input URL
// 
// We use the input URL convention here to future proof this method.
// For instance we could have a URL that is not a local file. It could
// be a URL for another service (e.g. S3)
//------------------------------------------------------------------------------
void Arion::parseInputUrl(std::string inputUrl)
{
  int pos = inputUrl.find(Utils::FILE_SOURCE);

  if (pos != string::npos)
  {
    mInputFile = Utils::getStringTail(inputUrl, pos + Utils::FILE_SOURCE.length());
  }
  else
  {
    throw inputSourceException;
  }
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
    // The write_meta object is no present, so skip this step...
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
  
  try
  {
    string country_code = writemetaTree.get<string>("country_code");
    
    (*mpIptcData)["Iptc.Application2.CountryCode"] = country_code;
  }
  catch (boost::exception& e)
  {
    // Optional
  }
  
  try
  {
    string subject = writemetaTree.get<string>("subject");
    
    (*mpIptcData)["Iptc.Application2.Subject"] = subject;
  }
  catch (boost::exception& e)
  {
    // Optional
  }
  
  try
  {
    string special_instructions = writemetaTree.get<string>("instructions");
    
    (*mpIptcData)["Iptc.Application2.SpecialInstructions"] = special_instructions;
  }
  catch (boost::exception& e)
  {
    // Optional
  }
  
  //-------------------------------------
  //  Add keywords if any are included
  //-------------------------------------
  boost::optional< const ptree& > keywordTreeOptional = writemetaTree.get_child_optional("keywords");
  
  if (keywordTreeOptional)
  {
    Exiv2::IptcKey key = Exiv2::IptcKey("Iptc.Application2.Keywords");
    
    Exiv2::IptcData::iterator pos = mpIptcData->findKey(key);
    
    if (pos != mpIptcData->end())
    {
      mpIptcData->erase(pos);
    }
    
    const ptree& keywordTree = keywordTreeOptional.get();
    
    BOOST_FOREACH (const ptree::value_type& node, keywordTree)
    {
      try
      {
        Exiv2::Value::AutoPtr v = Exiv2::Value::create(Exiv2::string);
        v->read(node.second.get_value<std::string>());
        
        mpIptcData->add(key, v.get());
      }
      catch (boost::exception& e)
      {
        // Ignore issues
      }
    }
  }
}

//------------------------------------------------------------------------------
// Given each input operation do the following:
//  1. Get its type and parameters
//  2. Create the corresponding operation object and place it in the queue
//  3. Provide any additional data to the operation
//------------------------------------------------------------------------------
bool Arion::parseOperations(const ptree& pt)
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
        operation = new Resize();
        operation->setup(paramsTree);
      }
      else if (type == "read_meta")
      {
        // This is a read_meta operation so create the corresponding object
        operation = new Read_meta();
        operation->setup(paramsTree);
      }
      else if (type == "copy")
      {
        // This is a copy operation so create the corresponding object
        operation = new Copy(mInputFile);
        operation->setup(paramsTree);
      }
      else
      {
        throw operationNotSupportedException;
      }
      
      // Add to operation queue
      mOperations.push_back(operation);

      operationParseCount++;

    }
    catch (std::exception& e)
    {
      
      stringstream ss;

      ss << "Count not parse operation " << (operationParseCount+1) << " - " << e.what();
      
      mErrorMessage = ss.str();
      constructErrorJson();
      
      return false;

    }
  }
  
  return true;
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
  }
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void Arion::extractImage(const string& imageFilePath)
{
  mSourceImage = cv::imread(imageFilePath);

  if (mSourceImage.empty())
  {
    throw extractException;
  }

  //--------------------------------
  //      Compute Image MD5
  //--------------------------------

  // We need this until the very end, so memory deallocation will be handled
  // when the program exits and the heap is deallocated
  // TODO: make this optional
  mpPixelMd5 = Utils::computeMd5((char*)mSourceImage.data, (int)mSourceImage.step[0] * mSourceImage.rows);
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
bool Arion::run()
{

  //----------------------------------
  //        Preprocessing
  //----------------------------------
  if (mInputFile.length())
  {
    try
    {
      // TODO: only read pixels if required by operations...
      // We have an input file, so lets read it
      extractImage(mInputFile);
    }
    catch (boost::exception& e)
    {

      mResult = false;
      mErrorMessage = "Error extracting image";
      constructErrorJson();
      
      return false;
    }

    //----------------------------------
    //     Read metadata from file
    //----------------------------------
    extractMetadata(mInputFile);
  }
  
  //----------------------------------
  //        Write metadata
  //----------------------------------
  if (!mInputTree.empty())
  {
    overrideMeta(mInputTree);
  }
  
  // Make sure we have image data to work with
  if (mSourceImage.empty())
  {
    mResult = false;
    mErrorMessage = "Input image data is empty";
    constructErrorJson();

    return false;
  }
  
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
  
  //----------------------------------
  //       Execute operations
  //----------------------------------
  writer.String("info");
  writer.StartArray();
  
  mTotalOperations = mOperations.size();
  
  BOOST_FOREACH (Operation& operation, mOperations)
  {
    try
    {
      
      operation.setImage(mSourceImage);

      // Give operations meta data if it exists
      if (mpExifData)
      {
        operation.setExifData(mpExifData);
      }

      if (mpXmpData)
      {
        operation.setXmpData(mpXmpData);
      }

      if (mpIptcData)
      {
        operation.setIptcData(mpIptcData);
      }
      
      if (!operation.run())
      {
        mFailedOperations++;
      }

      operation.serialize(writer);
    }
    catch (std::exception& e)
    {
      mFailedOperations++;
      mErrorMessage = e.what();
      constructErrorJson();
      return mResult;
    }
  }
  
  writer.EndArray();

  // Result of command (all operations must succeed to get true)
  if (mFailedOperations == 0)
  {
    mResult = true;
  }
  else
  {
    mResult = false;
  }
  
  writer.String("result");
  writer.Bool(mResult);
  
  // Operation stats
  writer.String("total_operations");
  writer.Uint(mTotalOperations);
  
  writer.String("failed_operations");
  writer.Uint(mFailedOperations);

  writer.EndObject();
  
  mJson = s.GetString();
  
  return mResult;
  
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void Arion::constructErrorJson()
{
  rapidjson::StringBuffer s;

  #ifdef JSON_PRETTY_OUTPUT
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(s);
  #else
    rapidjson::Writer<rapidjson::StringBuffer> writer(s);
  #endif

  writer.StartObject();

  // Result
  writer.String("result");
  writer.Bool(false);

  // Error message
  writer.String("error_message");
  writer.String(mErrorMessage);

  writer.EndObject();

  mJson = s.GetString();
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
std::string Arion::getJson() const
{
  return mJson;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
bool Arion::getJpeg(unsigned operationIndex, std::vector<unsigned char>& data)
{
  
  if (operationIndex >= mOperations.size())
  {
    return false;
  }
  
  Operation& operation = mOperations.at(operationIndex);
  
  bool result = operation.getJpeg(data);
  
  return result;
}
