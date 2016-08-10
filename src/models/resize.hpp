#ifndef RESIZE_HPP
#define RESIZE_HPP

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

// Boost
#include <boost/property_tree/ptree.hpp>

// Exiv2
#include <exiv2/exiv2.hpp>

// Local
#include "models/operation.hpp"

// Resize images that are maximum 10,000 x 10,000 pixels
// At the max this will use 3.2GB of memory (a 100MP image)
#define ARION_RESIZE_MAX_PIXELS 100000000

enum
{
  ResizeTypeInvalid     = -1,
  ResizeTypeFixedWidth  = 0,
  ResizeTypeFixedHeight = 1,
  ResizeTypeSquare      = 2,
  ResizeTypeFill        = 3
};

enum
{
  ResizeStatusDidNotTry = 0,
  ResizeStatusPending   = 1,
  ResizeStatusSuccess   = 2,
  ResizeStatusError     = 3
};

enum
{
  ResizeGravitytCenter   = 0,
  ResizeGravityNorth     = 1,
  ResizeGravitySouth     = 2,
  ResizeGravityWest      = 3,
  ResizeGravityEast      = 4,
  ResizeGravityNorthWest = 5,
  ResizeGravityNorthEast = 6,
  ResizeGravitySouthWest = 7,
  ResizeGravitySouthEast = 8
};

enum
{
  ResizeWatermarkTypeStandard = 0,
  ResizeWatermarkTypeAdaptive = 1,
};

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
class Resize : public Operation
{
  public:

    Resize();
    virtual ~Resize();

    virtual void setup(const boost::property_tree::ptree& params);
    virtual bool run();
    virtual bool getJpeg(std::vector<unsigned char>& data);
    
    void setType(const std::string& type);
    void setHeight(unsigned height);
    void setWidth(unsigned width);
    void setQuality(unsigned quality);
    void setGravity(std::string gravity);
    void setSharpenAmount(unsigned sharpenAmount);
    void setSharpenRadius(float radius);
    void setPreserveMeta(bool preserveMeta);
    void setWatermarkUrl(const std::string& watermarkUrl);
    void setWatermarkType(const std::string& watermarkType);
    void setWatermarkAmount(float watermarkAmount);
    void setWatermarkMinMax(float watermarkMin, float watermarkMax);
    void setOutputUrl(const std::string& outputUrl);
    
    std::string getOutputFile() const;
    bool getPreserveMeta() const;
    bool getStatus() const;
    void outputStatus(std::ostream& s, unsigned indent) const;
    
  #ifdef JSON_PRETTY_OUTPUT
    virtual void serialize(rapidjson::PrettyWriter<rapidjson::StringBuffer>& writer) const;
  #else
    virtual void serialize(rapidjson::Writer<rapidjson::StringBuffer>& writer) const;
  #endif

  private:

    int getAspectHeight(int resizeWidth, double aspect) const;
    int getAspectWidth(int resizeHeight, double aspect) const;
    
    void computeSizeSquare();
    void computeSizeWidth();
    void computeSizeHeight();
    void computeSizeFill();
    
    void readType(const boost::property_tree::ptree& params);
    void readGravity(const boost::property_tree::ptree& params);
    
    void validateType(const std::string& type);
    void validateGravity(const std::string& gravity);
    void validateWatermarkUrl(const std::string& watermarkUrl);
    void validateWatermarkType(const std::string& watermarkType);
    void validateOutputUrl(const std::string& outputUrl);
    void validateWatermarkAmount(float watermarkAmount);
    void validateWatermarkMinMax(float watermarkMin, float watermarkMax);
    void validateQuality(unsigned quality);
    void validateSharpenAmount(unsigned sharpenAmount);
    void validateSharpenRadius(float sharpenRadius);
    
    void applyWatermark();

    int mType;
    unsigned mHeight;
    unsigned mWidth;
    unsigned mQuality;
    unsigned mGravity;
    bool mPreFilter;
    unsigned mSharpenAmount;
    float mSharpenRadius;
    bool mPreserveMeta;
    std::string mWatermarkFile;
    unsigned mWatermarkType;
    double mWatermarkAmount;
    double mWatermarkMin;
    double mWatermarkMax;
    std::string mOutputFile;

    cv::Mat mImageResized;
    cv::Mat mImageResizedFinal;
    
    cv::Size mSize;
    cv::Mat mImageToResize;

    int mStatus;
    std::string mErrorMessage;

};

#endif // RESIZE_HPP
