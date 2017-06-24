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

#include "./resize.hpp"
#include "../utils/utils.hpp"

#include <iostream>
#include <string>
#include <ostream>

// Boost
#include <boost/exception/info.hpp>
#include <boost/exception/error_info.hpp>
#include <boost/exception/all.hpp>
#include <boost/foreach.hpp>

// OpenCV
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>

// Exiv2
#include <exiv2/exiv2.hpp>

using boost::property_tree::ptree;
using namespace cv;
using namespace std;

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
Resize::Resize() :
    Operation(),
    mType(ResizeTypeInvalid),
    mHeight(0),
    mWidth(0),
    mQuality(92),
    mInterpolation(INTER_AREA),
    mGravity(ResizeGravitytCenter),
    mPreFilter(false),
    mSharpenAmount(0),
    mSharpenRadius(0.0),
    mPreserveMeta(false),
    mWatermarkFile(),
    mWatermarkType(ResizeWatermarkTypeStandard),
    mWatermarkAmount(0.05),
    mWatermarkMin(0.05),
    mWatermarkMax(0.5),
    mStatus(ResizeStatusDidNotTry),
    mErrorMessage()
{
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
Resize::~Resize()
{
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void Resize::setup(const ptree& params)
{
  //-------------------------
  //   Required arguments
  //-------------------------
  
  readType(params);
  
  // Height validation handled in run()
  boost::optional<unsigned> height = params.get_optional<unsigned>("height");
  if (height) {// Required, but output error during run()
    mHeight = *height;
  }

  // Width validation handled in run()
  boost::optional<unsigned> width = params.get_optional<unsigned>("width");
  if (width) {// Required, but output error during run()
    mWidth = *width;
  }

  boost::optional<string> outputUrl = params.get_optional<string>("output_url");
  if (outputUrl) {// Required, but output error during run()
    validateOutputUrl(*outputUrl);
  }

  
  //-------------------------
  //   Optional arguments
  //-------------------------
  
  readGravity(params);

  boost::optional<bool> preserve_meta = params.get_optional<bool>("preserve_meta");
  if (preserve_meta) {//Not required
    mPreserveMeta = true;
  } else {
    mPreserveMeta = false;
  }

  boost::optional<unsigned> quality = params.get_optional<unsigned>("quality");
  if (quality) {// Not required
    validateQuality(*quality);
  }

  boost::optional<string> interpolation = params.get_optional<string>("interpolation");
  if (interpolation) {
    setInterpolation(*interpolation);
  }

  boost::optional<bool> pre_filter = params.get_optional<bool>("pre_filter");
  if (pre_filter) {//optional
    mPreFilter = true;
  } else {
    mPreFilter = false;
  }

  boost::optional<unsigned> sharpen_amount = params.get_optional<unsigned>("sharpen_amount");
  if (sharpen_amount) {// Not required
    validateSharpenAmount(*sharpen_amount);
  }

  boost::optional<float> sharpen_radius = params.get_optional<float>("sharpen_radius");
  if (sharpen_radius) {// Not required
    validateSharpenRadius(*sharpen_radius);
  }

  boost::optional<string> watermark_type = params.get_optional<string>("watermark_type");
  if (watermark_type) {// Not required
    validateWatermarkType(*watermark_type);
  }

  boost::optional<string> watermark_url = params.get_optional<string>("watermark_url");
  if (watermark_url) {// Not required
    validateWatermarkUrl(*watermark_url);
  }

  boost::optional<float> watermark_amount = params.get_optional<float>("watermark_amount");
  if (watermark_amount) {// Not required
    validateWatermarkAmount(*watermark_amount);
  }

  boost::optional<float> watermark_min = params.get_optional<float>("watermark_min");
  boost::optional<float> watermark_max = params.get_optional<float>("watermark_max");
  if (watermark_min && watermark_max) {// Not required
    validateWatermarkMinMax(*watermark_min,*watermark_max);;
  }

}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void Resize::setType(const std::string& type)
{
  validateType(type);
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void Resize::setHeight(unsigned height)
{
  mHeight = height;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void Resize::setWidth(unsigned width)
{
  mWidth = width;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void Resize::setQuality(unsigned quality)
{
  validateQuality(quality);
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void Resize::setInterpolation(const std::string& interpolation)
{
    if (interpolation == "nearest"){
        mInterpolation =  INTER_NEAREST;
    } else if (interpolation == "linear"){
        mInterpolation = INTER_LINEAR;
    } else if (interpolation == "cubic"){
        mInterpolation = INTER_CUBIC;
    } else if (interpolation == "area"){
        mInterpolation = INTER_AREA;
    } else if (interpolation == "lanczon4"){
        mInterpolation = INTER_LANCZOS4;
    }
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void Resize::setGravity(std::string gravity)
{
  validateGravity(gravity);
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void Resize::setSharpenAmount(unsigned sharpenAmount)
{
  validateSharpenAmount(sharpenAmount);
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void Resize::setSharpenRadius(float radius)
{
  validateSharpenRadius(radius);
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void Resize::setPreserveMeta(bool preserveMeta)
{
  mPreserveMeta = preserveMeta;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void Resize::setWatermarkUrl(const std::string& watermarkUrl)
{
  validateWatermarkUrl(watermarkUrl);
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void Resize::setWatermarkType(const std::string& watermarkType)
{
  validateWatermarkType(watermarkType);
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void Resize::setWatermarkAmount(float watermarkAmount)
{
  mWatermarkAmount = watermarkAmount;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void Resize::setWatermarkMinMax(float watermarkMin, float watermarkMax)
{
  validateWatermarkMinMax(watermarkMin, watermarkMax);
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void Resize::setOutputUrl(const std::string& outputUrl)
{
  validateOutputUrl(outputUrl);
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
std::string Resize::getOutputFile() const
{
  return mOutputFile;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
bool Resize::getPreserveMeta() const
{
  return mPreserveMeta;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
bool Resize::getStatus() const
{
  return mStatus;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
bool Resize::getJpeg(std::vector<unsigned char>& data)
{
  vector<int> compression_params;
  compression_params.push_back(IMWRITE_JPEG_QUALITY);
  compression_params.push_back(mQuality);

  return imencode(".jpg", mImageResizedFinal, data, compression_params);
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void Resize::readType(const ptree& params)
{
  boost::optional<string> type = params.get_optional<string>("type");
  if (type) {// Required, but output error during run()

    string realType = *type;
    // Make sure it's lowercase
    transform(realType.begin(), realType.end(), realType.begin(), ::tolower);

    validateType(realType);
  }
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void Resize::validateType(const std::string& type)
{
  if (type == "width")
  {
    mType = ResizeTypeFixedWidth;
  }
  else if (type == "height")
  {
    mType = ResizeTypeFixedHeight;
  }
  else if (type == "square")
  {
    mType = ResizeTypeSquare;
  }
  else if (type == "fill")
  {
    mType = ResizeTypeFill;
  }
  else
  {
    // Invalid
    mType = ResizeTypeInvalid;
  }
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void Resize::readGravity(const ptree& params)
{
    boost::optional<string> gravity = params.get_optional<std::string>("gravity");
    if (gravity){
      string realGravity = *gravity;
      // Make sure it's lowercase
      transform(realGravity.begin(), realGravity.end(), realGravity.begin(), ::tolower);
      validateGravity(realGravity);
  }

}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void Resize::validateGravity(const string& gravity)
{
  if (gravity == "center" || gravity == "c")
  {
    mGravity = ResizeGravitytCenter;
  }
  else if (gravity == "north" || gravity == "n")
  {
    mGravity = ResizeGravityNorth;
  }
  else if (gravity == "south" || gravity == "s")
  {
    mGravity = ResizeGravitySouth;
  }
  else if (gravity == "west" || gravity == "w")
  {
    mGravity = ResizeGravityWest;
  }
  else if (gravity == "east" || gravity == "e")
  {
    mGravity = ResizeGravityEast;
  }
  else if (gravity == "northwest" || gravity == "nw")
  {
    mGravity = ResizeGravityNorthWest;
  }
  else if (gravity == "northeast" || gravity == "ne")
  {
    mGravity = ResizeGravityNorthEast;
  }
  else if (gravity == "southwest" || gravity == "sw")
  {
    mGravity = ResizeGravitySouthWest;
  }
  else if (gravity == "southeast" || gravity == "se")
  {
    mGravity = ResizeGravitySouthEast;
  }
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void Resize::validateOutputUrl(const std::string& outputUrl)
{
  int pos = outputUrl.find(Utils::FILE_SOURCE);

  if (pos != string::npos)
  {
    mOutputFile = Utils::getStringTail(outputUrl, pos + Utils::FILE_SOURCE.length());
  }
  else
  {
    // Assume local file
    mOutputFile = outputUrl;
  }
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void Resize::validateWatermarkUrl(const std::string& watermarkUrl)
{
  int pos = watermarkUrl.find(Utils::FILE_SOURCE);

  if (pos != string::npos)
  {
    mWatermarkFile = Utils::getStringTail(watermarkUrl, pos + Utils::FILE_SOURCE.length());
  }
  else
  {
    // Assume local file
    mWatermarkFile = watermarkUrl;
  }
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void Resize::validateWatermarkType(const string& watermarkType)
{
  if (watermarkType == "standard")
  {
    mWatermarkType = ResizeWatermarkTypeStandard;
  }
  else if (watermarkType == "adaptive")
  {
    mWatermarkType = ResizeWatermarkTypeAdaptive;
  }
}

//------------------------------------------------------------------------------
// This value only applies to the standard watermark type
//------------------------------------------------------------------------------
void Resize::validateWatermarkAmount(float watermarkAmount)
{
  if ((watermarkAmount < 0.0) || (watermarkAmount > 1.0))
  {
    // Keep constructor default
    return;
  }

  mWatermarkAmount = watermarkAmount;
}

//------------------------------------------------------------------------------
// These values only apply to the adaptive watermark type
//------------------------------------------------------------------------------
void Resize::validateWatermarkMinMax(float watermarkMin, float watermarkMax)
{
  if ((watermarkMin < 0.0) || (watermarkMin > 1.0))
  {
    // Keep constructor defaults
    return;
  }

  if ((watermarkMax < 0.0) || (watermarkMax > 1.0))
  {
    // Keep constructor defaults
    return;
  }

  if (watermarkMax < watermarkMin)
  {
    // Keep constructor defaults
    return;
  }

  mWatermarkMin = watermarkMin;
  mWatermarkMax = watermarkMax;

}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void Resize::validateQuality(unsigned quality)
{
  if (quality <= 100)
  {
    mQuality = quality;
  }
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void Resize::validateSharpenAmount(unsigned sharpenAmount)
{
  if (sharpenAmount <= 1000)
  {
    mSharpenAmount = sharpenAmount;
  }
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void Resize::validateSharpenRadius(float sharpenRadius)
{
  if ((sharpenRadius > 0.0) && (sharpenRadius < 10.0))
  {
    mSharpenRadius = sharpenRadius;
  }
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void Resize::computeSizeSquare()
{
  // Don't assume the height and width the user specified are the same
  // and just use the width
  mSize = Size(mWidth, mWidth);
  
  const unsigned sourceHeight = (unsigned)mImage.rows;
  const unsigned sourceWidth = (unsigned)mImage.cols;

  if (sourceHeight == sourceWidth)
  {
    // Easy... the image is already square
    mImageToResize = mImage;
  }
  else if (sourceHeight > sourceWidth)
  {
    int y = round(((double)sourceHeight - (double)sourceWidth)/2.0);
    Rect cropRegion(0, y, sourceWidth, sourceWidth);

    mImageToResize = mImage(cropRegion);
  }
  else // sourceWidth < sourceHeight
  {
    int x = round(((double)sourceWidth - (double)sourceHeight)/2.0);
    Rect cropRegion(x, 0, sourceHeight, sourceHeight);

    mImageToResize = mImage(cropRegion);
  }
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void Resize::computeSizeWidth()
{
  const unsigned sourceHeight = (unsigned)mImage.rows;
  const unsigned sourceWidth = (unsigned)mImage.cols;
  
  double aspect = (double)sourceHeight / (double)sourceWidth;
  
  // User specified fixed width. Only use height as an absolute max
  unsigned resizeWidth = mWidth;
  unsigned resizeHeight = getAspectHeight(resizeWidth, aspect);

  if (resizeHeight > mHeight)
  {
    resizeHeight = mHeight;
    resizeWidth = getAspectWidth(resizeHeight, aspect);
  }

  mImageToResize = mImage;
  mSize = Size(resizeWidth, resizeHeight);

}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void Resize::computeSizeHeight()
{
  const unsigned sourceHeight = (unsigned)mImage.rows;
  const unsigned sourceWidth = (unsigned)mImage.cols;
  
  double aspect = (double)sourceHeight / (double)sourceWidth;
  
  // User specified fixed height so we ignore input width and compute our own
  unsigned resizeHeight = mHeight;
  unsigned resizeWidth = getAspectWidth(resizeHeight, aspect);

  if (resizeWidth > mWidth)
  {
    resizeWidth = mWidth;
    resizeHeight = getAspectHeight(resizeWidth, aspect);
  }

  mImageToResize = mImage;
  mSize = Size(resizeWidth, resizeHeight);
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void Resize::computeSizeFill()
{
  const unsigned sourceHeight = mImage.rows;
  const unsigned sourceWidth = mImage.cols;
  
  double destAspect = (double)mHeight / (double)mWidth;
  
  double xf = (double)mWidth / (double)sourceWidth;
  double yf = (double)mHeight / (double)sourceHeight;
  
  //float factor_ratio = xf / yf;
  unsigned cropWidth = 0;
  unsigned cropHeight = 0;
  unsigned cropX = 0;
  unsigned cropY = 0;
  
  if (xf > yf)
  {
    cropWidth = (unsigned)sourceWidth;
    cropHeight = getAspectHeight(cropWidth, destAspect);
  }
  else
  {
    cropHeight = (unsigned)sourceHeight;
    cropWidth = getAspectWidth(cropHeight, destAspect);
  }
  
  switch (mGravity)
  {
    case ResizeGravitytCenter:
      cropX = (sourceWidth - cropWidth) / 2;
			cropY = (sourceHeight - cropHeight) / 2;
      break;
      
    case ResizeGravityNorth:
			cropX = (sourceWidth - cropWidth) / 2;
			cropY = 0;
      break;
      
    case ResizeGravityNorthWest:
			cropX = 0;
			cropY = 0;
      break;
      
    case ResizeGravityNorthEast:
			cropX = (sourceWidth - cropWidth);
			cropY = 0;
      break;
      
    case ResizeGravitySouth:
			cropX = (sourceWidth - cropWidth) / 2;
			cropY = (sourceHeight - cropHeight);
      break;
      
    case ResizeGravitySouthWest:
			cropX = 0;
			cropY = (sourceHeight - cropHeight);
      break;
      
    case ResizeGravitySouthEast:
			cropX = (sourceWidth - cropWidth);
			cropY = (sourceHeight - cropHeight);
      break;
      
    case ResizeGravityWest:
			cropX = 0;
			cropY = (sourceHeight - cropHeight) / 2;
      break;
      
    case ResizeGravityEast:
			cropX = (sourceWidth - cropWidth);
			cropY = (sourceHeight - cropHeight) / 2;
      break;

  }
  
  Rect cropRegion(cropX, cropY, cropWidth, cropHeight);

  mImageToResize = mImage(cropRegion);
  
  mSize = Size(mWidth, mHeight);

}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
bool Resize::run()
{

  mStatus = ResizeStatusPending;
  
  if (mImage.empty())
  {
    mStatus = ResizeStatusError;
    mErrorMessage = "Input image data is empty";
    return false;
  }

  //---------------------------------------------------
  //  Perform the resize operation and write to disk
  //---------------------------------------------------
  try
  {

    switch (mType)
    {
      //--------------------------
      //      Square resize
      //--------------------------
      case ResizeTypeSquare:
      {
        computeSizeSquare();
        break;
      }

      //--------------------------
      //  Height priority resize
      //--------------------------
      case ResizeTypeFixedHeight:
      {
        computeSizeHeight();
        break;
      }
      
      //--------------------------
      //      Fill resize
      //--------------------------
      case ResizeTypeFill:
      {
        computeSizeFill();
        break;
      }

      //--------------------------
      //  Width priority resize
      //--------------------------
      case ResizeTypeFixedWidth:
      {
        computeSizeWidth();
        break;
      }
      
      //--------------------------
      //  Error (unknown type)
      //--------------------------
      default:
        mStatus = ResizeStatusError;
        mErrorMessage = "Invalid resize type";
        return false;
    }
    
    if (mHeight == 0)
    {
      mStatus = ResizeStatusError;
      mErrorMessage = "Height cannot be 0";
      return false;
    }
    
    if (mWidth == 0)
    {
      mStatus = ResizeStatusError;
      mErrorMessage = "Width cannot be 0";
      return false;
    }

    // Don't attempt to resize an image to a size that's greater than our max
    if (mHeight * mWidth > ARION_RESIZE_MAX_PIXELS)
    {
      mStatus = ResizeStatusError;
      mErrorMessage = "Desired resize dimensions exceed maximum";
      return false;
    }

    if (mPreFilter)
    {
      double sigma = (double)mImageToResize.cols/1000.0;

      // Make sure we're not editing the original...
      Mat imageToResizeFiltered;

      GaussianBlur(mImageToResize, imageToResizeFiltered, cv::Size(0, 0), sigma);

      // Resize operation
      resize(imageToResizeFiltered, mImageResized, mSize, 0, 0, mInterpolation);
    }
    else
    {
      // Resize operation
      resize(mImageToResize, mImageResized, mSize, 0, 0, mInterpolation);
    }

    if (mSharpenAmount)
    {
      GaussianBlur(mImageResized, mImageResizedFinal, cv::Size(0, 0), mSharpenRadius);

      addWeighted(mImageResized, 1.0 + (mSharpenAmount/100.0), mImageResizedFinal, -(mSharpenAmount/100.0), 0, mImageResizedFinal);
    }
    else
    {
      // Assign by reference
      mImageResizedFinal = mImageResized;
    }

    if (mWatermarkFile.length())
    {
      applyWatermark();
    }
  }
  catch(boost::exception& e)
  {
    mStatus = ResizeStatusError;
    mErrorMessage = boost::diagnostic_information(e);
    return false;
  }
  
  if (!mOutputFile.empty())
  {
    vector<int> compression_params;
    compression_params.push_back(IMWRITE_JPEG_QUALITY);
    compression_params.push_back(mQuality);

    if (!imwrite(mOutputFile, mImageResizedFinal, compression_params))
    {
      mStatus = ResizeStatusError;
      mErrorMessage = "Failed to write output image";
      return false;
    }
    
    //--------------------------------
    //  Inherit EXIF data if needed
    //--------------------------------
    if (mpExifData || mpXmpData || mpIptcData || mpIccProfile) {
      if (mPreserveMeta) {
        try {
          Exiv2::Image::AutoPtr outputExivImage = Exiv2::ImageFactory::open(mOutputFile.c_str());

          if (outputExivImage.get() != 0) {
            if (mpExifData) {
              // Output image inherits input EXIF data
              outputExivImage->setExifData(*mpExifData);
            }

            if (mpXmpData) {
              // Output image inherits input XMP data
              outputExivImage->setXmpData(*mpXmpData);
            }

            if (mpIptcData) {
              // Output image inherits input IPTC data
              outputExivImage->setIptcData(*mpIptcData);
            }
              //--------------------------------
              //  Keep color profile if defined
              //--------------------------------
              if (mpIccProfile){
                  try { //TODO if we resizing from PNG to JPEG then it was failed. Fix that. See tests
                      outputExivImage->setIccProfile(*new Exiv2::DataBuf(mpIccProfile->pData_,mpIccProfile->size_));
                  } catch (...) {
                      //TODO
                  }
              }
          }

          outputExivImage->writeMetadata();

        }
        catch (Exiv2::AnyError &e) {
          mStatus = ResizeStatusError;
          mErrorMessage = e.what();
          return false;
        }
      }
      else if (mpExifData) {
        //WhiteList for Exif tags
        string exifWhiteList[] = {"Exif.Image.Orientation","Exif.Image.InterColorProfile"};
        Exiv2::ExifData whiteListExifData;
        for (unsigned int i = 0; i < (sizeof(exifWhiteList) / sizeof(exifWhiteList[0])); i++) {//iterate over and try to find key from whitelist
          Exiv2::ExifKey key = Exiv2::ExifKey(exifWhiteList[i]);
          if (mpExifData->findKey(key) != mpExifData->end()) {
            whiteListExifData[exifWhiteList[i]] = mpExifData->findKey(key)->value();
          }
        }
        if (!whiteListExifData.empty() || mpIccProfile) {
          Exiv2::Image::AutoPtr outputExivImage = Exiv2::ImageFactory::open(mOutputFile.c_str());
          if (outputExivImage.get() != 0) {
              if (!whiteListExifData.empty()){
                  outputExivImage->setExifData(whiteListExifData);
              }
              if (mpIccProfile){
                  try { //TODO if we resizing from PNG to JPEG then it was failed. Fix that. See tests
                      outputExivImage->setIccProfile(*new Exiv2::DataBuf(mpIccProfile->pData_,mpIccProfile->size_));
                  } catch (...) {
                      //TODO
                  }
              }

            outputExivImage->writeMetadata();
          }
        }
      }
    }

  }
  
  mStatus = ResizeStatusSuccess;

  return true;
}

//------------------------------------------------------------------------------
// Apply the watermark in place
//------------------------------------------------------------------------------
void Resize::applyWatermark()
{
  Mat watermark = imread(mWatermarkFile, IMREAD_UNCHANGED);

  if (watermark.empty())
  {
    return;
  }

  double blend = mWatermarkAmount / 255.0;
  const double blendMin = mWatermarkMin / 255.0;
  const double blendMax = mWatermarkMax / 255.0;
  const double blendDelta = blendMax - blendMin;
  const double normFactor = 9.0 / 255.0;

  // In case we can't compute brightness for adaptive watermark use the min blend specified
  if (mWatermarkType == ResizeWatermarkTypeAdaptive)
  {
    blend = blendMin;
  }

  int wx = 0;
  int wy = 0;
  
  for (int y = 0; y < mImageResizedFinal.rows; ++y)
  {
    // If the final image is taller than the watermark, repeat it
    if (y >= watermark.rows)
    {
      wy = y % watermark.rows;
    }
    else
    {
      wy = y;
    }

    for (int x = 0; x < mImageResizedFinal.cols; ++x)
    {
      // If the final image is wider than the watermark, repeat it
      if (x >= watermark.cols)
      {
        wx = x % watermark.cols;
      }
      else
      {
        wx = x;
      }

      int watermarkIdx = wy * watermark.step + wx * watermark.channels();
      
      // determine the opacity of the foreground pixel, using its fourth (alpha) channel.
      unsigned char alpha = watermark.data[watermarkIdx + 3];

      // Only apply watermark if alpha is non-zero
      if (alpha)
      {
        int i = y * mImageResizedFinal.step + x * mImageResizedFinal.channels();

        if ((mWatermarkType == ResizeWatermarkTypeAdaptive) && mImageResizedFinal.channels() >= 3)
        {
          unsigned char b = mImageResizedFinal.data[i];
          unsigned char g = mImageResizedFinal.data[i+1];
          unsigned char r = mImageResizedFinal.data[i+2];

          // Use a fast approximation for brightness
          // http://stackoverflow.com/questions/596216/formula-to-determine-brightness-of-rgb-color
          unsigned brightness = (r+r+r+b+g+g+g+g)>>3;

          // Log-based blend
          // blend = (blendMax - blendMin) * log10 ( 9*(brightness / 255) + 1) + blendMin
          blend = blendDelta * log10(1.0 + normFactor * (double)brightness) + blendMin;

          // Linear blend - for reference...
          // blend = (blendMax - blendMin) * ((double)brightness/255.0) + blendMin;

        }

        double opacity = blend * ((double) alpha);

        // Combine the background and watermark pixel, using the opacity, 
        for (int c = 0; c < mImageResizedFinal.channels(); ++c)
        {
          int finalOffset = i + c;
          unsigned char foregroundPx = watermark.data[watermarkIdx + c];
          unsigned char backgroundPx = mImageResizedFinal.data[finalOffset];
          
          // Apply in place
          mImageResizedFinal.data[finalOffset] = backgroundPx * (1.0 - opacity) + foregroundPx * opacity;
        }
      }
    }
  }
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
#ifdef JSON_PRETTY_OUTPUT
void Resize::serialize(rapidjson::PrettyWriter<rapidjson::StringBuffer>& writer) const
#else
void Resize::serialize(rapidjson::Writer<rapidjson::StringBuffer>& writer) const
#endif
{
  writer.StartObject();

  // Result
  writer.String("type");
  writer.String("resize");

  // Output URL
  writer.String("output_url");
  writer.String("file://" + mOutputFile);

  if (mStatus == ResizeStatusSuccess)
  {
    // Result
    writer.String("result");
    writer.Bool(true);

    // Dimensions
    writer.String("output_height");
    writer.Uint(mImageResized.rows);
    writer.String("output_width");
    writer.Uint(mImageResized.cols);

  }
  else
  {
    // Result
    writer.String("result");
    writer.Bool(false);

    // Error message
    if ((mStatus == ResizeStatusError) &&  !mErrorMessage.empty())
    {
      writer.String("error_message");
      writer.String(mErrorMessage);
    }
  }

  writer.EndObject();
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int Resize::getAspectWidth(int resizeHeight, double aspect) const
{
  return (int)round((double)resizeHeight / aspect);
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int Resize::getAspectHeight(int resizeWidth, double aspect) const
{
  return (int)round((double)resizeWidth * aspect);
}