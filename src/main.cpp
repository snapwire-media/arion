//------------------------------------------------------------------------------
//
// Resizer
//
// A utility for importing, resizing, extracting meta-data, and exporting images.
//
// ------------
//   main.cpp
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
#include "utils/utils.hpp"

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
using namespace std;

#define DEBUG 0

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void showHelp(options_description& desc)
{
  cerr << desc << endl;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void exitWithError(string errorMessage, stringstream* pAdditionalInfo = NULL)
{
  cout << "{" << endl;

  cout << "  \"result\" : false," << endl;
  cout << "  \"error_message\" : \"" << errorMessage << "\"";

  if (pAdditionalInfo)
  {
    cout << "," << endl;
    cout << "  \"info\" : " << endl;
    cout << pAdditionalInfo->str() << endl;
  }
  else
  {
    cout << endl;
  }

  cout << "}" << endl;

  exit(-1);
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void exitWithSuccess(double time, 
                     char* md5, 
                     int height, 
                     int width, 
                     stringstream* pAdditionalInfo = NULL)
{
  cout << "{" << endl;

  cout << "  \"result\" : true," << endl;
  cout << "  \"time\" : "   << time << "," << endl;
  cout << "  \"height\" : " << height << "," << endl;
  cout << "  \"width\" : "  << width << "," << endl;
  cout << "  \"md5\" : "    << "\"" << md5 << "\"";

  if (pAdditionalInfo)
  {
    cout << "," << endl;
    cout << "  \"info\" : " << endl;
    cout << pAdditionalInfo->str() << endl;
  }
  else
  {
    cout << endl;
  }

  cout << "}" << endl;

  exit(0);
}

//------------------------------------------------------------------------------
// Return true if image was rotated, false otherwise
//------------------------------------------------------------------------------
bool handleOrientation(Exiv2::ExifData& exifData, cv::Mat& image)
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
void exifDebug(Exiv2::ExifData& exifData)
{
  cout << "Has exif!" << endl;

  // DEBUG
  Exiv2::ExifData::const_iterator end = exifData.end();

  for (Exiv2::ExifData::const_iterator i = exifData.begin(); i != end; ++i)
  {
    const char* tn = i->typeName();

    cout << std::setw(44)
         << std::setfill(' ') << std::left
         << i->key() << " "
         << "0x" << std::setw(4)
         << std::setfill('0') << std::right
         << std::hex << i->tag() << " "
         << std::setw(9) << std::setfill(' ')
         << std::left << (tn ? tn : "Unknown")
         << " " << std::dec
         << std::setw(3) << std::setfill(' ')
         << std::right << i->count() << "  "
         << std::dec << i->value()
         << "\n";
  }
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void xmpDebug(Exiv2::XmpData& xmpData)
{
  cout << "Has XMP!" << endl;

  // DEBUG
  Exiv2::XmpData::const_iterator end = xmpData.end();

  // Output XMP properties
  for (Exiv2::XmpData::const_iterator md = xmpData.begin(); md != xmpData.end(); ++md)
  {
      std::cout << std::setfill(' ') << std::left
                << std::setw(44)
                << md->key() << " "
                << std::setw(9) << std::setfill(' ') << std::left
                << md->typeName() << " "
                << std::dec << std::setw(3)
                << std::setfill(' ') << std::right
                << md->count() << "  "
                << std::dec << md->value()
                << std::endl;
  }
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void run(const string& inputJson)
{
  boost::timer::cpu_timer timer;

  vector<Operation> operations;
  bool hasValidExif = false;
  bool hasValidXmp = false;
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

  Exiv2::Image::AutoPtr imageExiv;

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
      exitWithError("Unsupported input source. Use 'file://' prefix");
    }

    sourceImage = cv::imread(imageFilePath);

    if (sourceImage.empty())
    {
      exitWithError("Could not read source image");
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
      imageExiv = Exiv2::ImageFactory::open(imageFilePath.c_str());

      if (imageExiv.get() != 0)
      {
        imageExiv->readMetadata();

        Exiv2::ExifData& exifData = imageExiv->exifData();

        if (!exifData.empty())
        {
          // Got this far so we have at least some valid EXIF data...
          hasValidExif = true;

#if DEBUG
          exifDebug(exifData);
#endif

          if (correctOrientation)
          {
            handleOrientation(exifData, sourceImage);
          }
        }

        Exiv2::XmpData& xmpData = imageExiv->xmpData();

        if (!xmpData.empty())
        {

          hasValidXmp = true;

#if DEBUG
          xmpDebug(xmpData);
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

    exitWithError(ss.str());
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

      exitWithError(ss.str());

    }
  }

  //----------------------------------
  //       Execute operations
  //----------------------------------

  stringstream oss;
  oss << "  [" << endl;

  int operationCount = 0;

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
            r.setExifData(&imageExiv->exifData());
          }

          if (hasValidXmp)
          {
            r.setXmpData(&imageExiv->xmpData());
          }

          bool result = r.run(sourceImage);

          if (operationCount > 0)
          {
            // Make sure it's valid JSON
            oss << "," << endl;
          }

          if (result)
          {
            // Operation success!
            r.outputStatus(oss, 2);
          }
          else
          {
            // Operation failure
            r.outputStatus(oss, 2);
            oss << endl << "  ]" << endl;
            exitWithError("Operation failed", &oss);
          }

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
      oss << endl << "  ]" << endl;
      exitWithError(e.what(), &oss);
    }

    // Got through this operation, increment...
    operationCount++;
  }

  oss << endl << "  ]" << endl;

  typedef boost::chrono::duration<double> sec; // seconds, stored with a double
  sec seconds = boost::chrono::nanoseconds(timer.elapsed().user + timer.elapsed().system);

  exitWithSuccess(seconds.count(), md5, sourceImage.rows, sourceImage.cols, &oss);

}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int main(int argc, char* argv[])
{
  try
  {
    positional_options_description p;
    p.add("input", 1);

    options_description desc("Arguments");

    desc.add_options()
        ("help", "Produce this help message")
        ("input", value< string >(), "The input operations to execute in JSON");

    variables_map vm;

    store(command_line_parser(argc, argv).options(desc).positional(p).run(), vm);

    notify(vm);

    string inputJson;

    if (vm.count("help"))
    {
      showHelp(desc);
      return 1;
    }

    if (vm.count("input"))
    {
      inputJson = vm["input"].as<string>();
    }
    else
    {
      cout << "You must provide the input operations to execute" << endl << endl;
      showHelp(desc);
      return 1;
    }
    
    run(inputJson);
  }
  catch (std::exception& e)
  {
    exitWithError(e.what());
  }

  return 0;
}



