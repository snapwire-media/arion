#ifndef CARION_H_
#define CARION_H_

#ifdef __cplusplus
extern "C" {
#endif
  
  struct ArionInputOptions {
    bool  correctOrientation;
    void* inputBytes;
    char* inputUrl;
  };
  
  struct ArionResizeOptions {
    char* type;
    unsigned height;
    unsigned width;
    unsigned quality;
    char* gravity;
    unsigned sharpenAmount;
    float sharpenRadius;
    bool preserveMeta;
  };
  
//    mImage(image),
//    mType(ResizeTypeInvalid),
//    mHeight(0),
//    mWidth(0),
//    mQuality(92),
//    mGravity(ResizeGravitytCenter),
//    mSharpenAmount(0),
//    mSharpenRadius(0.0),
//    mPreserveMeta(false),
//    mWatermarkFile(),

  const char* ArionRunJson(const char* inputJson);
  const char* ArionResize(struct ArionInputOptions inputOptions,
                          struct ArionResizeOptions resizeOptions);

#ifdef __cplusplus
}
#endif

#endif