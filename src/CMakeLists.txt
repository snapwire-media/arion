CMAKE_MINIMUM_REQUIRED( VERSION 2.6 ) 

PROJECT (ARION)

FIND_PACKAGE( Boost 1.46 COMPONENTS program_options timer filesystem system REQUIRED )
FIND_PACKAGE( OpenCV REQUIRED )
FIND_PACKAGE( Threads )
FIND_PACKAGE( OpenSSL )

INCLUDE_DIRECTORIES( ${Boost_INCLUDE_DIR} )
INCLUDE_DIRECTORIES( ${OPENSSL_INCLUDE_DIR} )
INCLUDE_DIRECTORIES( ${ARION_SOURCE_DIR} )

ADD_DEFINITIONS( -DRAPIDJSON_HAS_STDSTRING=1 )
ADD_DEFINITIONS( -DJSON_PRETTY_OUTPUT )

# Uncomment this for additional console output (warning: may invalidate JSON output)
#ADD_DEFINITIONS( -DDEBUG=1 )

 -------------------------------------------
  This is the stand alone Arion executable
 -------------------------------------------
ADD_EXECUTABLE( arion main.cpp 
                      arion.cpp
                      models/operation.cpp 
                      models/resize.cpp
                      models/read_meta.cpp
                      models/copy.cpp
                      utils/utils.cpp )

TARGET_LINK_LIBRARIES( arion ${Boost_LIBRARIES} ${OpenCV_LIBS} exiv2 ${OPENSSL_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} )

# ---------------------------------------------------
#  This is the shared Arion library with c bindings
# ---------------------------------------------------
add_library(carion SHARED carion.cpp
                          arion.cpp
                          models/operation.cpp 
                          models/resize.cpp
                          models/read_meta.cpp
                          models/copy.cpp
                          utils/utils.cpp)

TARGET_LINK_LIBRARIES( carion ${Boost_LIBRARIES} ${OpenCV_LIBS} exiv2 ${OPENSSL_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} )

install(TARGETS carion DESTINATION lib)
install(FILES carion.h DESTINATION include)