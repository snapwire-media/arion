#ifndef CARION_H_
#define CARION_H_

#ifdef __cplusplus
extern "C" {
#endif
  
  struct ArionInputOptions {
    unsigned correctOrientation;
    //void* inputBytes;
    char* inputUrl;
    char* outputUrl;
  };
  
  struct ArionResizeOptions {
    char* algo;
    unsigned height;
    unsigned width;
    char* gravity;
    unsigned quality;
    unsigned sharpenAmount;
    float sharpenRadius;
    unsigned preserveMeta;
    char* watermarkUrl;
    unsigned watermarkAmount;
    char* outputUrl;
  };
  
  struct ArionResult {
    unsigned char* outputData;
    int outputSize;
    char* resultJson;
  };

  const char* ArionRunJson(const char* inputJson);
  struct ArionResult ArionResize(struct ArionInputOptions inputOptions,
                                 struct ArionResizeOptions resizeOptions);

#ifdef __cplusplus
}
#endif

#endif