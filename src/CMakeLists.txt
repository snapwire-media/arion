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

ADD_EXECUTABLE( arion main.cpp 
                      arion.cpp
                      models/operation.cpp 
                      models/resize.cpp
                      models/readmeta.cpp
                      utils/utils.cpp )

TARGET_LINK_LIBRARIES( arion ${Boost_LIBRARIES} ${OpenCV_LIBS} exiv2 ${OPENSSL_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} )
