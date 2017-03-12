#ifndef OPERATION_HPP
#define OPERATION_HPP
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
#include <exception>

// Exiv2
#include <exiv2/exiv2.hpp>

// Boost
#include <boost/property_tree/ptree.hpp>

// OpenCV
#include <opencv2/core/core.hpp>

// Local Third party
#include "../thirdparty/rapidjson/writer.h"
#include "../thirdparty/rapidjson/prettywriter.h"
#include "../thirdparty/rapidjson/stringbuffer.h"

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
class Operation : boost::noncopyable
{
  public:

    Operation();
    virtual ~Operation();

    virtual void setup(const boost::property_tree::ptree& params) {};
    boost::property_tree::ptree getParams() const;
    
    virtual bool run() = 0;
    virtual bool getJpeg(std::vector<unsigned char>& data) = 0;
    
    // There is no obvious way to make use of polymorphism for the writer object
    // so we rely on the preprocessor
  #ifdef JSON_PRETTY_OUTPUT
    virtual void serialize(rapidjson::PrettyWriter<rapidjson::StringBuffer>& writer) const {};
  #else
    virtual void serialize(rapidjson::Writer<rapidjson::StringBuffer>& writer) const {};
  #endif

    void setExifData(const Exiv2::ExifData* exifData);
    void setXmpData(const Exiv2::XmpData* xmpData);
    void setIptcData(const Exiv2::IptcData* iptcData);
    void setImage(cv::Mat& image);

  protected:
    
    void operator=( const Operation& );
    
    boost::property_tree::ptree mParams;

    const Exiv2::ExifData* mpExifData;
    const Exiv2::XmpData* mpXmpData;
    const Exiv2::IptcData* mpIptcData;
    cv::Mat mImage;

};

#endif // OPERATION_HPP
