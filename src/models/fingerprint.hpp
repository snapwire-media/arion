#ifndef FINGERPRINT_HPP
#define FINGERPRINT_HPP

//------------------------------------------------------------------------------
//
// Copyright (c) 2015-2016 Paul Filitchkin
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

// OpenCV
#include <opencv2/core/core.hpp>

// Exiv2
#include <exiv2/exiv2.hpp>

// Local
#include "./operation.hpp"

enum
{
  FingerprintStatusDidNotTry = 0,
  FingerprintStatusPending   = 1,
  FingerprintStatusSuccess   = 2,
  FingerprintStatusError     = 3,
};

enum
{
  FingerprintTypeInvalid = 0,
  FingerprintTypeMD5     = 1,
};

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
class Fingerprint : public Operation
{
  public:

    Fingerprint();
    virtual ~Fingerprint();

    virtual void setup(const boost::property_tree::ptree& params);
    virtual bool run();
    virtual bool getJpeg(std::vector<unsigned char>& data);

    void setType(const std::string& type);
    bool getStatus() const;
    void outputStatus(std::ostream& s, unsigned indent) const;
    
  #ifdef JSON_PRETTY_OUTPUT
    virtual void serialize(rapidjson::PrettyWriter<rapidjson::StringBuffer>& writer) const;
  #else
    virtual void serialize(rapidjson::Writer<rapidjson::StringBuffer>& writer) const;
  #endif

  private:

    void readType(const boost::property_tree::ptree& params);    
    void decodeType(const std::string& type);
    
    boost::property_tree::ptree mParams;
    
    unsigned mStatus;
    unsigned mType;
    std::string mErrorMessage;
    char* mpPixelMd5;

};

#endif // FINGERPRINT_HPP
