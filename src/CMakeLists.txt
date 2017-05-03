CMAKE_MINIMUM_REQUIRED( VERSION 2.6 ) 

PROJECT (ARION)

SET(CMAKE_MACOSX_RPATH 1)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/../cmake/Modules/")

FIND_PACKAGE( Boost 1.46 COMPONENTS program_options timer filesystem system REQUIRED )
FIND_PACKAGE( OpenCV REQUIRED )
FIND_PACKAGE( Threads REQUIRED )
FIND_PACKAGE( OpenSSL REQUIRED )
FIND_PACKAGE( Exiv2 0.26 REQUIRED )

INCLUDE_DIRECTORIES( ${Boost_INCLUDE_DIR} )
INCLUDE_DIRECTORIES( ${OPENSSL_INCLUDE_DIR} )
INCLUDE_DIRECTORIES( ${ARION_SOURCE_DIR} )

MESSAGE( STATUS "ARION_SOURCE_DIR:     ${ARION_SOURCE_DIR}"  )
MESSAGE( STATUS "OPENSSL_INCLUDE_DIR:  ${OPENSSL_INCLUDE_DIR}"  )
MESSAGE( STATUS "OPENSSL_LIBRARIES:    ${OPENSSL_LIBRARIES}"  )
MESSAGE( STATUS "OpenCV_DIR:           ${OpenCV_DIR}"  )
MESSAGE( STATUS "OpenCV_VERSION:       ${OpenCV_VERSION}"  )
MESSAGE( STATUS "EXIV2_VERSION:        ${EXIV2_VERSION}"  )
MESSAGE( STATUS "EXIV2_LIBRARIES:      ${EXIV2_LIBRARIES}"  )
MESSAGE( STATUS "OpenCV_LIBS:          ${OpenCV_LIBS}"  )
MESSAGE( STATUS "Boost_LIBRARIES:      ${Boost_LIBRARIES}"  )


ADD_DEFINITIONS( -DRAPIDJSON_HAS_STDSTRING=1 )
ADD_DEFINITIONS( -DJSON_PRETTY_OUTPUT )

# Uncomment this for additional console output (warning: may invalidate JSON output)
#ADD_DEFINITIONS( -DDEBUG=1 )
# Uncomment this to set a pixel limit for the resize command (the value below ensures
# the desired output must be below 50MP)
#ADD_DEFINITIONS( -DARION_RESIZE_MAX_PIXELS=50000000 )

# -------------------------------------------
#  This is the stand alone Arion executable
# -------------------------------------------
ADD_EXECUTABLE( arion main.cpp
                      arion.cpp
                      models/operation.cpp
                      models/resize.cpp
                      models/read_meta.cpp
                      models/copy.cpp
                      models/fingerprint.cpp
                      utils/utils.cpp)

TARGET_LINK_LIBRARIES( arion ${Boost_LIBRARIES} ${OpenCV_LIBS} ${EXIV2_LIBRARIES} ${OPENSSL_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} )

# ---------------------------------------------------
#  This is the shared Arion library with c bindings
# ---------------------------------------------------
add_library(carion SHARED carion.cpp
                          arion.cpp
                          models/operation.cpp
                          models/resize.cpp
                          models/read_meta.cpp
                          models/copy.cpp
                          models/fingerprint.cpp
                          utils/utils.cpp)

TARGET_LINK_LIBRARIES( carion ${Boost_LIBRARIES} ${OpenCV_LIBS} ${EXIV2_LIBRARIES} ${OPENSSL_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} )

install(TARGETS carion DESTINATION lib)
install(FILES carion.h DESTINATION include)

#uncomment this to see all cmake variables
#get_cmake_property(_variableNames VARIABLES)
#foreach (_variableName ${_variableNames})
#    message(STATUS "${_variableName}=${${_variableName}}")
#endforeach()

#uncomment this to see environment variables
#execute_process(COMMAND "${CMAKE_COMMAND}" "-E" "environment")

