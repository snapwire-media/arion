#include "models/resize.hpp"
#include "utils/utils.hpp"

#include <iostream>
#include <string>
#include <ostream>

#include <sys/time.h>

#include <boost/exception/info.hpp>
#include <boost/exception/error_info.hpp>
#include <boost/exception/all.hpp>
#include <boost/foreach.hpp>
#include <boost/timer/timer.hpp>

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
Resize::Resize(const ptree& params) :
    mType(ResizeTypeFixedWidth),
    mPreserveMeta(false),
    mQuality(95),
    mPreFilter(false),
    mSharpenAmount(0),
    mSharpenRadius(0.0),
    mStatus(ResizeStatusDidNotTry),
    mErrorMessage(),
    mpExifData(0),
    mpXmpData(0),
    mWatermarkFile(),
    mOperationTime(0)
{

  try
  {
    string type = params.get<std::string>("type");

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
    else
    {
      // Invalid
      mType = -1;
    }
  }
  catch (boost::exception& e)
  {
    //cerr << "ERROR: Could not read the resize operation type" << endl;
    //return;
  }

  // TODO: error on invalid type

  try
  {
    mHeight = params.get<int>("height");
  }
  catch (boost::exception& e)
  {
    cerr << "ERROR: Could not read the resize operation height" << endl;
    return;
  }

  try
  {
    mWidth = params.get<int>("width");
  }
  catch (boost::exception& e)
  {
    cerr << "ERROR: Could not read the resize operation width" << endl;
    return;
  }

  try
  {
    mPreserveMeta = params.get<bool>("preserve_meta");
  }
  catch (boost::exception& e)
  {
    cerr << "ERROR: Could not read the resize operation preserve_meta flag" << endl;
    return;
  }

  try
  {
    mQuality = params.get<int>("quality");
  }
  catch (boost::exception& e)
  {
    // Not required
  }

  try
  {
    mPreFilter= params.get<bool>("pre_filter");
  }
  catch (boost::exception& e)
  {
    // Not required
  }

  try
  {
    mSharpenAmount = params.get<int>("sharpen_amount");
  }
  catch (boost::exception& e)
  {
    // Not required
  }

  try
  {
    mSharpenRadius = params.get<float>("sharpen_radius");
  }
  catch (boost::exception& e)
  {
    // Not required
  }

  try
  {
    string outputUrl = params.get<string>("output_url");

    int pos = outputUrl.find(Utils::FILE_SOURCE);

    if (pos != string::npos)
    {
      mOutputFile = Utils::getStringTail(outputUrl, pos + Utils::FILE_SOURCE.length());
    }
  }
  catch (boost::exception& e)
  {
    cerr << "ERROR: Could not read the resize operation output url" << endl;
    return;
  }

  try
  {
    string outputUrl = params.get<string>("watermark_url");

    int pos = outputUrl.find(Utils::FILE_SOURCE);

    if (pos != string::npos)
    {
      mWatermarkFile = Utils::getStringTail(outputUrl, pos + Utils::FILE_SOURCE.length());
    }
  }
  catch (boost::exception& e)
  {
    // Not required
  }
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

bool Resize::getStatus() const
{
  return mStatus;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void Resize::setExifData(const Exiv2::ExifData* exifData)
{
  mpExifData = exifData;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void Resize::setXmpData(const Exiv2::XmpData* xmpData)
{
  mpXmpData = xmpData;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
string Resize::type2str(int type)
{
  string r;

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

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
bool Resize::run(Mat& image)
{

  boost::timer::cpu_timer timer;

  mStatus = ResizeStatusPending;

  //---------------------------------------------------
  //  Perform the resize operation and write to disk
  //---------------------------------------------------
  try
  {
    static const int interpolation = INTER_AREA;

    int resizeHeight;
    int resizeWidth;

    int sourceHeight = image.rows;
    int sourceWidth = image.cols;

    double aspect = (double)sourceHeight / (double)sourceWidth;

    Mat imageToResize;

    Size size;

    switch (mType)
    {
      //--------------------------
      //      Square resize
      //--------------------------
      case ResizeTypeSquare:
      {
        // Don't assume the height and width the user specified are the same
        // and just use the width
        size = Size(mWidth, mWidth);

        if (sourceHeight == sourceWidth)
        {
          // Easy... the image is already square
          imageToResize = image;
        }
        else if (sourceHeight > sourceWidth)
        {
          int y = round(((double)sourceHeight - (double)sourceWidth)/2.0);
          Rect cropRegion(0, y, sourceWidth, sourceWidth);

          imageToResize = image(cropRegion);
        }
        else // sourceWidth < sourceHeight
        {
          int x = round(((double)sourceWidth - (double)sourceHeight)/2.0);
          Rect cropRegion(x, 0, sourceHeight, sourceHeight);

          imageToResize = image(cropRegion);
        }

        break;
      }

      //--------------------------
      //  Height priority resize
      //--------------------------
      case ResizeTypeFixedHeight:
      {
        // User specified fixed height so we ignore input width and compute our own
        resizeHeight = mHeight;
        resizeWidth = getAspectWidth(resizeHeight, aspect);

        if ((mWidth >= 0) && (resizeWidth > mWidth))
        {
          resizeWidth = mWidth;
          resizeHeight = getAspectHeight(resizeWidth, aspect);
        }

        imageToResize = image;
        size = Size(resizeWidth, resizeHeight);

        break;
      }

      //--------------------------
      //  Width priority resize
      //--------------------------
      default:
      case ResizeTypeFixedWidth:
      {
        // User specified fixed width. Only use height as an absolute max
        resizeWidth = mWidth;
        resizeHeight = getAspectHeight(resizeWidth, aspect);

        if ((mHeight >= 0) && (resizeHeight > mHeight))
        {
          resizeHeight = mHeight;
          resizeWidth = getAspectWidth(resizeHeight, aspect);
        }

        imageToResize = image;
        size = Size(resizeWidth, resizeHeight);

        break;
      }
    }

    if (mPreFilter)
    {
      double sigma = (double)imageToResize.cols/1000.0;

      // Make sure we're not editing the original...
      Mat imageToResizeFiltered;

      GaussianBlur(imageToResize, imageToResizeFiltered, cv::Size(0, 0), sigma);

      // Resize operation
      resize(imageToResizeFiltered, mImageResized, size, 0, 0, interpolation);
    }
    else
    {
      // Resize operation
      resize(imageToResize, mImageResized, size, 0, 0, interpolation);
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

      Mat watermark = imread(mWatermarkFile, IMREAD_UNCHANGED);

      int x = 0;
      int y = 0;
      int width;
      int height;

      if (mImageResizedFinal.cols > watermark.cols)
      {
        width = watermark.cols;
      }
      else
      {
        width = mImageResizedFinal.cols;
      }

      if (mImageResizedFinal.rows > watermark.rows)
      {
        height = watermark.rows;
      }
      else
      {
        height = mImageResizedFinal.rows;
      }

      Rect roi(x, y, width, height);

      Mat watermarkRoi = watermark(roi);

      overlayImage(mImageResizedFinal, watermarkRoi, mImageResizedFinal, cv::Point(0, 0), 0.05);

    }

    vector<int> compression_params;
    compression_params.push_back(IMWRITE_JPEG_QUALITY);
    compression_params.push_back(mQuality);

    if (!imwrite(mOutputFile, mImageResizedFinal, compression_params))
    {
      mStatus = ResizeStatusError;
      mErrorMessage = "Failed to write output image";
      return false;
    }
  }
  catch(boost::exception& e)
  {
    mStatus = ResizeStatusError;
    mErrorMessage = boost::diagnostic_information(e);
    return false;
  }

  //--------------------------------
  //  Inherit EXIF data if needed
  //--------------------------------
  if (mPreserveMeta && (mpExifData || mpXmpData))
  {
    try
    {
      Exiv2::Image::AutoPtr outputImageExiv = Exiv2::ImageFactory::open(mOutputFile.c_str());

      if (mpExifData)
      {
        if (outputImageExiv.get() != 0)
        {
          // Output image inherits input EXIF data
          outputImageExiv->setExifData(*mpExifData);
        }
      }

        outputImageExiv->writeMetadata();

    }
    catch(Exiv2::AnyError& e)
    {
      mStatus = ResizeStatusError;
      mErrorMessage = e.what();
      return false;
    }
  }

  mStatus = ResizeStatusSuccess;

  typedef boost::chrono::duration<double> sec; // seconds, stored with a double
  sec seconds = boost::chrono::nanoseconds(timer.elapsed().user + timer.elapsed().system);

  mOperationTime = seconds.count();


  return true;

}

/*
 * overlayImage: copies a transparent 4-channel image over a solid background image.
 * Original source: http://jepsonsblog.blogspot.com.br/2012/10/overlay-transparent-image-in-opencv.html
 *
 * background: must be 3-channel BGR.
 * foreground: must be 4-channel RGBA.
 * output: the destination Mat.
 * location: offset starting point.
 */
void Resize::overlayImage(const cv::Mat &background, const cv::Mat &foreground, cv::Mat &output, cv::Point2i location, double blend) const
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

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void Resize::outputStatus(ostream& s, unsigned indent) const
{
  string p = string(indent, ' ');

  s << p << "{" << endl;
  s << p << "  \"type\" : \"resize\"," << endl;

  if (mStatus == ResizeStatusSuccess)
  {
    s << p << "  \"result\" : true," << endl;
    s << p << "  \"output_url\" : \"file://" << mOutputFile << "\"," << endl;
    s << p << "  \"output_height\" : \"" << mImageResized.rows << "\"," << endl;
    s << p << "  \"output_width\" : \"" << mImageResized.cols << "\"," << endl;
    s << p << "  \"time\" : \"" << mOperationTime << "\"" << endl;
  }
  else
  {
    s << p << "  \"result\" : false," << endl;

    if ((mStatus == ResizeStatusError) &&  !mErrorMessage.empty())
    {
      s << p << "  \"error_message\" : \"" << mErrorMessage << "\"," << endl;
    }

    s << p << "  \"output_url\" : \"file://" << mOutputFile << "\"" << endl;
  }

  s << p << "}";

  // NOTE: Tried doing property tree -> JSON, but this doesn't quite work...
  //       In addition BOOST outputs everything as a string

}

