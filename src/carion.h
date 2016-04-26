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
  
  struct ArionResizeResult {
    
    // The JPG encoded image bytes
    unsigned char* outputData;
    
    // The size of the JPEG encoded image bytes
    int outputSize;
    
    // An error message if an error is encountered
    char* errorMessage;
    
    // The JSON formated summary of the operation
    char* resultJson;
  };

  // This is strictly a JSON endpoint and does not accept or pass back image data
  // All operations such as reading the image and resizing is handled
  // inside the wrapper function and the JSON result is returned directly
  const char* ArionRunJson(const char* inputJson);
  
  struct ArionResizeResult ArionResize(struct ArionInputOptions inputOptions,
                                       struct ArionResizeOptions resizeOptions);

#ifdef __cplusplus
}
#endif

#endif