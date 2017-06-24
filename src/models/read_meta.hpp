#ifndef READ_META_HPP
#define READ_META_HPP

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

#include <boost/property_tree/ptree.hpp>

// OpenCV
#include <opencv2/core/core.hpp>

// Exiv2
#include <exiv2/exiv2.hpp>

// Boost
#include <boost/foreach.hpp>

// Local
#include "./operation.hpp"

enum {
  ReadmetaStatusDidNotTry = 0,
  ReadmetaStatusPending = 1,
  ReadmetaStatusSuccess = 2,
  ReadmetaStatusError = 3,
};

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
class Read_meta : public Operation {
 public:

  Read_meta();
  virtual ~Read_meta();

  virtual void setup(const boost::property_tree::ptree &params);
  virtual bool run();
  virtual bool getJpeg(std::vector<unsigned char> &data);

  bool getStatus() const;

  void outputStatus(std::ostream &s, unsigned indent) const;

#ifdef JSON_PRETTY_OUTPUT
  virtual void serialize(rapidjson::PrettyWriter<rapidjson::StringBuffer>& writer) const;
#else
  virtual void serialize(rapidjson::Writer<rapidjson::StringBuffer> &writer) const;
#endif

 private:

  //-------------------
  //  Private methods
  //-------------------
  void readIptc();
  void readIptcStringByKey(Exiv2::IptcData::const_iterator md,
                           const std::string &key,
                           std::string *value);

  //---------------
  //    Params
  //---------------
  boost::property_tree::ptree mParams;
  bool mReadInfo;

  int mStatus;
  std::string mErrorMessage;

  //---------------
  //     Info
  //---------------
  std::vector <std::string> mKeywords;
  std::string mCaption;
  std::string mCopyright;
  bool mModelReleased;
  bool mPropertyReleased;
  std::string mInstructions;
  std::vector <std::string> mSubject;
  std::string mByline;

  // Location
  std::string mCity;
  std::string mProvinceState;
  std::string mCountryName;
  std::string mCountryCode;

};

#endif // READ_META_HPP
