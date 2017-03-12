#ifndef Utils_HPP
#define Utils_HPP
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

#include <string>
#include <vector>
#include <cstdlib>
#include <cstdio>
#include <iostream>

// OpenSSL
#include <openssl/md5.h>

// OpenCV
#include <opencv2/imgproc.hpp>

// Exiv2
#include <exiv2/exiv2.hpp>

// Local Third party
#include "../thirdparty/rapidjson/writer.h"
#include "../thirdparty/rapidjson/prettywriter.h"
#include "../thirdparty/rapidjson/stringbuffer.h"

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
namespace Utils
{
  static const std::string FILE_SOURCE = "file://";
    
  static std::string getStringTail(const std::string& string, int start)
  {
    const int end = string.length() - start;

    return string.substr(start, end);
  }

  //----------------------------------------------------------------------------
  // Compute the MD5 hash of a given array
  //----------------------------------------------------------------------------
  static char* computeMd5(const char* str, int length)
  {
    int n;
    MD5_CTX c;
    unsigned char digest[16];
    char* out = (char*)calloc(1, 33);

    MD5_Init(&c);

    while (length > 0)
    {
      if (length > 512)
      {
        MD5_Update(&c, str, 512);
      }
      else
      {
        MD5_Update(&c, str, length);
      }

      length -= 512;
      str += 512;
    }

    MD5_Final(digest, &c);

    for (n = 0; n < 16; ++n)
    {
      snprintf(&(out[n*2]), 16*2, "%02x", (unsigned int)digest[n]);
    }

    return out;
  }

  //------------------------------------------------------------------------------
  //------------------------------------------------------------------------------
  static void exifDebug(Exiv2::ExifData& exifData)
  {
    std::cout << "Has exif!" << std::endl;

    // DEBUG
    Exiv2::ExifData::const_iterator end = exifData.end();

    for (Exiv2::ExifData::const_iterator i = exifData.begin(); i != end; ++i)
    {
      const char* tn = i->typeName();

      std::cout << std::setw(44)
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
  static void xmpDebug(Exiv2::XmpData& xmpData)
  {
    std::cout << "Has XMP!" << std::endl;

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
  static void iptcDebug(Exiv2::IptcData &iptcData)
  {
    std::cout << "Has IPTC!" << std::endl;

    Exiv2::IptcData::iterator end = iptcData.end();

    for (Exiv2::IptcData::iterator md = iptcData.begin(); md != end; ++md) 
    {
      std::cout << std::setw(44) << std::setfill(' ') << std::left
                << md->key() << " "
                << "0x" << std::setw(4) << std::setfill('0') << std::right
                << std::hex << md->tag() << " "
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
  static void exitWithError(std::string errorMessage)
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
    writer.String(errorMessage);
    
    // Assume we weren't able to read any operations
    writer.String("total_operations");
    writer.Uint(0);

    writer.String("failed_operations");
    writer.Uint(0);

    writer.EndObject();
    
    std::cout << s.GetString() << std::endl;

    exit(-1);
  }

  //------------------------------------------------------------------------------
  // Returns a character representation of an OpenCV image type
  //------------------------------------------------------------------------------
  static std::string type2str(int type)
  {
    std::string r;

    uchar depth = type & CV_MAT_DEPTH_MASK;
    uchar chans = 1 + (type >> CV_CN_SHIFT);

    switch ( depth ) {
      case CV_8U:  r = "8U"; break;
      case CV_8S:  r = "8S"; break;
      case CV_16U: r = "16U"; break;
      case CV_16S: r = "16S"; break;
      case CV_32S: r = "32S"; break;
      case CV_32F: r = "32F"; break;
      case CV_64F: r = "64F"; break;
      default:     r = "User"; break;
    }

    r += "C";
    r += (chans+'0');

    return r;
  }
}

#endif // Utils_HPP
