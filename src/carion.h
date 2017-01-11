#ifndef CARION_H_
#define CARION_H_

#ifdef __cplusplus
extern "C" {
#endif
  
  struct ArionInputOptions {
    // If set to 0 the image orientation will not be corrected
    // (based on the EXIF orientation flag)
    unsigned correctOrientation;
        
    // The location of the input image
    char* inputUrl;
    
    // If an output URL is provided the image will be saved there
    char* outputUrl;

    // The desired output format. 1 = JPEG, 2 = PNG
    int outputFormat;
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
    char* watermarkType;
    float watermarkAmount;
    float watermarkMin;
    float watermarkMax;
    char* outputUrl;
  };
  
  struct ArionResizeResult {
    
    // The JPG encoded image bytes
    unsigned char* outputData;
    
    // The size of the JPEG encoded image bytes
    int outputSize;
    
    // The result of the operation
    // 0 - success
    // -1 - failure
    int returnCode;
    
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