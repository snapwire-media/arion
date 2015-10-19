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

// OpenSSL
#include <openssl/md5.h>

// OpenCV
#include <opencv2/imgproc.hpp>

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

 //----------------------------------------------------------------------------
 //
 // Copies a transparent 4-channel image over a non-transparent 3 channel 
 // background image.
 //
 // Original source: 
 //  http://jepsonsblog.blogspot.com.br/2012/10/overlay-transparent-image-in-opencv.html
 // 
 // background: must be 3-channel BGR.
 // foreground: must be 4-channel RGBA.
 // output: the destination Mat.
 // location: offset starting point.
 //
 //----------------------------------------------------------------------------
  static void overlayImage(const cv::Mat &background, 
                           const cv::Mat &foreground, 
                           cv::Mat &output, 
                           cv::Point2i location, 
                           double blend)
  {
    background.copyTo(output);
    double blendConstant = blend / 255.0;

    // start at the row indicated by location, or at row 0 if location.y is negative.
    for (int y = std::max(location.y, 0); y < background.rows; ++y)
    {
      int fY = y - location.y; // because of the translation

      // we are done or we have processed all rows of the foreground image.
      if (fY >= foreground.rows)
        break;

      // start at the column indicated by location, or at column 0 if location.x is negative.
      for (int x = std::max(location.x, 0); x < background.cols; ++x)
      {
        int fX = x - location.x; // because of the translation.

        // we are done with this row if the column is outside of the foreground image.
        if (fX >= foreground.cols)
          break;

        // determine the opacity of the foreground pixel, using its fourth (alpha) channel.
        double opacity = blendConstant * ((double) foreground.data[fY * foreground.step + fX * foreground.channels() + 3]);

        // and now combine the background and foreground pixel, using the opacity, but only if opacity > 0.
        for (int c = 0; opacity > 0 && c < output.channels(); ++c)
        {
          unsigned char foregroundPx = foreground.data[fY * foreground.step + fX * foreground.channels() + c];
          unsigned char backgroundPx = background.data[y * background.step + x * background.channels() + c];
          output.data[y * output.step + output.channels() * x + c] = backgroundPx * (1.0 - opacity) + foregroundPx * opacity;
        }
      }
    }
  }
}

#endif // Utils_HPP
