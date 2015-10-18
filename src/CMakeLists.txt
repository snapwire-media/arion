CMAKE_MINIMUM_REQUIRED( VERSION 2.6 ) 

PROJECT (SWRESIZER)

FIND_PACKAGE( Boost 1.46 COMPONENTS program_options timer filesystem system REQUIRED )
FIND_PACKAGE( OpenCV REQUIRED )
FIND_PACKAGE( Threads )
FIND_PACKAGE( OpenSSL )

INCLUDE_DIRECTORIES( ${Boost_INCLUDE_DIR} )
INCLUDE_DIRECTORIES( ${OPENSSL_INCLUDE_DIR} )
INCLUDE_DIRECTORIES( ${SWRESIZER_SOURCE_DIR} ) 

ADD_EXECUTABLE( resizer main.cpp 
                        models/operation.cpp 
                        models/resize.cpp
                        utils/utils.cpp)

TARGET_LINK_LIBRARIES( resizer ${Boost_LIBRARIES} ${OpenCV_LIBS} exiv2 ${OPENSSL_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} )

