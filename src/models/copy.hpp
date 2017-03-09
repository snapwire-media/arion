#ifndef COPY_HPP
#define COPY_HPP

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

// OpenCV
#include <opencv2/core/core.hpp>

// Exiv2
#include <exiv2/exiv2.hpp>

// Local
#include "models/operation.hpp"

enum
{
  CopyStatusDidNotTry = 0,
  CopyStatusPending = 1,
  CopyStatusSuccess = 2,
  CopyStatusError = 3,
};

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
class Copy : public Operation
{
  public:

    Copy(std::string inputFile);
    virtual ~Copy();

    virtual void setup(const boost::property_tree::ptree& params);
    virtual bool run();
    virtual bool getJpeg(std::vector<unsigned char>& data);
    virtual bool getPNG(std::vector<unsigned char>& data);

    std::string getOutputFile() const;
    bool getStatus() const;
    void outputStatus(std::ostream& s, unsigned indent) const;
    
  #ifdef JSON_PRETTY_OUTPUT
    virtual void serialize(rapidjson::PrettyWriter<rapidjson::StringBuffer>& writer) const;
  #else
    virtual void serialize(rapidjson::Writer<rapidjson::StringBuffer>& writer) const;
  #endif

  private:

    boost::property_tree::ptree mParams;
    
    int mStatus;
    std::string mErrorMessage;
    
    std::string mInputFile;
    std::string mOutputFile;

};

#endif // COPY_HPP
