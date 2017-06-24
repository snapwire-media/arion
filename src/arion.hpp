#ifndef ARION_HPP
#define ARION_HPP

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

#include <string>
#include <vector>

// Boost
#include <boost/property_tree/ptree.hpp>
#include <boost/ptr_container/ptr_vector.hpp>

// OpenCV
#include <opencv2/core/core.hpp>

// Exiv2
#include <exiv2/exiv2.hpp>

// Local
#include "models/operation.hpp"
#include "carion.h"

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
class Arion {
 public:

  Arion();
  ~Arion();

  bool setup(const std::string &inputJson);

  void setSourceImage(cv::Mat &sourceImage);
  cv::Mat &getSourceImage();
  bool setInputUrl(const std::string &inputUrl);
  bool setOutputUrl(const std::string &outputUrl);
  void setIgnoreMetadata(bool ignoreMetadata);
  void setCorrectOrientation(bool correctOrientation);
  void addResizeOperation(struct ArionResizeOptions options);

  bool run();
  std::string getJson() const;

  bool getJpeg(unsigned operationIndex, std::vector<unsigned char> &data);

 private:

  //--------------------
  //      Helpers
  //--------------------
  bool handleOrientation(Exiv2::ExifData &exifData, cv::Mat &image);
  bool parseOperations(const boost::property_tree::ptree &pt);
  void extractImageData(const std::string &imageFilePath);
  void overrideMeta(const boost::property_tree::ptree &pt);
  void constructErrorJson();
  void parseInputUrl(std::string inputUrl);

  //--------------------
  //      Inputs
  //--------------------
  boost::property_tree::ptree mInputTree;
  std::string mInputFile;
  bool mCorrectOrientation;
  bool mIgnoreMetadata;
  cv::Mat mSourceImage;

  typedef boost::ptr_vector <Operation> Operations;

  Operations mOperations;

  //--------------------
  //     Image info
  //--------------------
  Exiv2::ExifData *mpExifData;
  Exiv2::XmpData *mpXmpData;
  Exiv2::IptcData *mpIptcData;
  Exiv2::DataBuf *mpIccProfile;
  Exiv2::Image::AutoPtr mExivImage;

  // The following describe the result of the operations
  bool mResult;
  std::string mErrorMessage;
  unsigned mTotalOperations;
  unsigned mFailedOperations;

  // This contains the resulting variables in JSON
  std::string mJson;

};

#endif // ARION_HPP
