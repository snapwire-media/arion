# Arion [![Build Status](https://travis-ci.org/snapwire-media/arion.svg)](https://travis-ci.org/snapwire-media/arion)
<img align="right" style="margin-left:4px;" src="https://raw.githubusercontent.com/wiki/snapwire-media/arion/images/arion-logo.png?token=ABLvVTS8ATmDqkb6_Al5kWmLIbUPJraeks5WLDi5wA%3D%3D">
Arion extracts metadata and creates beautiful thumbnails from your images.

* Batch generate thumbnails with one call
* Apply output sharpening on each thumbnail
* Resize with height priority, width priority, or square crop
* Apply user-defined watermark
* Get md5 hash of pixel data
* Raw images support

Each parameter is completely configurable via a JSON input and **Arion** can be called through any language that can execute shell commands. See the **[API Documentation](../../wiki/API-Documentation)** for more details.

## Why Arion?
While there are several tools available to generate thumbnails and read image metadata here's what Arion has to offer:
* **Performance** - use a single command to batch generate thumbnails
* **Aesthetics** - independently control output sharpening and JPEG quality on each thumbnail
* **Functionality** - easily apply watermarks and preserve image metadata on thumbnails
* **Compatability** - call from anything that executes shell commands (see examples in **[Python](../../wiki/Examples#python)**, **[PHP](../../wiki/Examples#php)**, **[Ruby](../../wiki/Examples#ruby)**, **[Bash](../../wiki/Examples#shell)**)
* **Extensibility** - use the modular operation framework and OpenCV to process images in new ways

## Installation
Currently this tool needs to be compiled from source to work on your host system. Install instructions are for Ubuntu, but can be easily modified to work on any *nix-based system. For Mac OS X see the following [instructions](../../wiki/Installation#mac-os-x). 

**Requirements**
* CMake 3.1+
* EXIV2 0.26+
* LibRaw 0.19+
* OpenCV 3.4+
* Boost 1.46+
  * core 
  * program options 
  * timer 
  * filesystem 
  * system

**Install dependencies**

***Ubuntu***
```bash
sudo apt-get install cmake wget unzip libexpat1-dev zlib1g-dev libssl-dev build-essential libpng-dev libpng automake
```

***Amazon linux***
```bash
sudo yum install cmake wget unzip expat-devel zlib-devel zlib-static openssl-devel openssl-static make glibc-devel gcc gcc-c++ automake
```

For old version on Amazon linux upgrade cmake to version 3.1+
```bash
sudo yum install cmake3 --enablerepo=epel
```
Then for rest of installation guide replace "cmake" to "cmake3" command

**Install EXIV2**

Download the latest version from http://www.exiv2.org/download.html (or use wget command below)

```bash
cd ~/
wget https://github.com/Exiv2/exiv2/archive/v0.26.zip
unzip v0.26.zip
cd exiv2-0.26
mkdir build
cd build
cmake ../
```

CMake will tell you any dependencies you are missing

Now build EXIV2 and install it into the system
```bash
make
sudo make install
```

**Install LibRaw**

Arion  support raw files via [libRaw](https://www.libraw.org/). Download latest version from https://www.libraw.org/download

```bash
wget https://github.com/LibRaw/LibRaw/archive/0.19.0-Beta2.zip
unzip 0.19.0-Beta2.zip
cd LibRaw-0.19.0-Beta2/
autoreconf --install
./configure
make
sudo make install
```


**Install OpenCV**

Arion requires OpenCV 3.4+ which must be compiled from source.  Download the latest archive from http://opencv.org/downloads.html or use wget latest version

```bash
wget https://github.com/opencv/opencv/archive/3.4.1.zip
unzip 3.4.1.zip
cd opencv-3.4.1/
mkdir build
cd build
cmake ..
make
sudo make install
```

For a more optimized/minimal OpenCV build use the following options. 
NOTE: This requires installation of [libjpeg-turbo](https://github.com/libjpeg-turbo/libjpeg-turbo).  

```
cmake -DBUILD_DOCS=OFF -DBUILD_TESTS=OFF -DBUILD_PERF_TESTS=OFF \
      -DBUILD_WITH_DEBUG_INFO=OFF -DBUILD_opencv_apps=OFF \
      -DBUILD_opencv_calib3d=OFF -DBUILD_opencv_video=OFF \
      -DBUILD_opencv_videoio=OFF -DBUILD_opencv_java=OFF \
      -DWITH_JPEG=ON -DBUILD_JPEG=OFF \
      -DJPEG_INCLUDE_DIR=/opt/libjpeg-turbo/include/ \
      -DJPEG_LIBRARY=/opt/libjpeg-turbo/lib64/libjpeg.a \
      -DENABLE_SSSE3=ON -DENABLE_SSE41=ON -DENABLE_SSE42=ON \
      -DENABLE_AVX=ON ..
```

**Install Boost**

Boost version 1.46+ is required to build Arion.  This is not a particularly new version so the package maintainers version will usually work.

***Ubuntu***
```bash
sudo apt-get install libboost-dev libboost-program-options-dev libboost-timer-dev libboost-filesystem-dev libboost-system-dev
```

***Amazon linux***
```bash
sudo yum install boost-devel boost-program-options boost-timer boost-filesystem boost-system boost-static
```

**Build Arion**

This will create the final executable. You will need to create a new build directory and run CMake to generate the makefile.  CMake will let you know if any dependencies are missing.  

```bash
mkdir build
cd build
cmake ../src/
make
```

**Run Examples**

There are two example images provided and a wide range of example operations via a shell script. 

```bash
cd examples
./examples.sh
```

The output will look like this

```bash
Running example operations on horizontal image

{
  "result" : true,
  "time" : 0.17,
  "height" : 864,
  "width" : 1296
  ...
```

**Fingerprint generation (md5)**

Fingerprint generation is separated operation. For JSON like that
```JSON
{
    "input_url": "../examples/image-2-800-watermark.jpg",
    "operations": [
        {
            "type": "fingerprint",
            "params": {
                "type": "md5"
            }
        }
    ]
}
```

Output will be:
```JSON
{
    "height": 1000,
    "width": 762,
    "info": [
        {
            "type": "fingerprint",
            "result": true,
            "md5": "5e1c56695ee01492ee3976f86a8b7f68"
        }
    ],
    "result": true,
    "total_operations": 1,
    "failed_operations": 0
}

```


# Static build

Sometimes you need to have a 'portable' version of Arion.
For example to use it with AWS Lambda or services like that.
Given services only allow to deploy precompiled binary without any operation system modification.
At that case portable version is very usable.

To build static version of Arion you need to build evix2 and Opencv as a static version.

**If you already build regular version of Arion**

Please clear cmake cache before to do a new build of Arion. You can remove content of build folders from arion,exiv2 and cmake.

**Building**

Please follow main manual except cmake commands.

***Exiv2***

```bash
cmake -DEXIV2_ENABLE_SHARED=OFF ..
```

***OpenCV***
```bash
cmake -DBUILD_SHARED_LIBS=OFF
```
But you also can skip build on not necessary packages

```bash
cmake -DBUILD_DOCS=OFF -DBUILD_TESTS=OFF -DBUILD_PERF_TESTS=OFF \
      -DBUILD_WITH_DEBUG_INFO=OFF -DBUILD_opencv_apps=OFF \
      -DBUILD_opencv_calib3d=OFF -DBUILD_opencv_video=OFF \
      -DBUILD_opencv_videoio=OFF -DBUILD_opencv_java=OFF \
      -DENABLE_SSSE3=ON -DENABLE_SSE41=ON -DENABLE_SSE42=ON \
      -DENABLE_AVX=ON -DENABLE_AVX2=ON -DENABLE_FMA3=ON \
      -DBUILD_SHARED_LIBS=OFF ..
```

***Arion***
```bash
cmake -DARION_ENABLE_SHARED=OFF ../src/
```