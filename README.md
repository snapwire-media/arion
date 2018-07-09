# Arion [![Build Status](https://travis-ci.org/snapwire-media/arion.svg)](https://travis-ci.org/snapwire-media/arion)
<img align="right" style="margin-left:4px;" src="https://raw.githubusercontent.com/wiki/snapwire-media/arion/images/arion-logo.png?token=ABLvVTS8ATmDqkb6_Al5kWmLIbUPJraeks5WLDi5wA%3D%3D">
Arion extracts metadata and creates beautiful thumbnails from your images. 
* Batch generate thumbnails with one call
* Apply output sharpening on each thumbnail
* Resize with height priority, width priority, or square crop
* Apply user-defined watermark
* Get md5 hash of pixel data

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
* CMake
* EXIV2 0.25+
* OpenCV 3.0+
* Boost 1.46+
  * core 
  * program options 
  * timer 
  * filesystem 
  * system

**Install dependencies**

```bash
sudo apt-get install cmake wget unzip libexpat1-dev zlib1g-dev libssl-dev
```

**Install EXIV2 (before Ubuntu 16.04)**

Download the latest version from http://www.exiv2.org/download.html (or use wget command below)

```bash
cd ~/
wget http://www.exiv2.org/exiv2-0.25.tar.gz
tar -xvf ~/exiv2-0.25.tar.gz
cd exiv2-0.25/build
cmake ../
```

CMake will tell you any dependencies you are missing

Now build EXIV2 and install it into the system
```bash
make
sudo make install
```

**Install EXIV2 (Ubuntu 16.04+)**

Ubuntu 16.04 come with EXIV2 0.25 from default. So you can skip manually build and install EXIV2 from repository
```bash
sudo apt-get isntall libexiv2-dev
```

**Install Boost**

Boost version 1.46+ is required to build Arion.  This is not a particularly new version so the package maintainers version will usually work.

```bash
sudo apt-get install libboost-dev libboost-program-options-dev libboost-timer-dev libboost-filesystem-dev libboost-system-dev
```

**Install OpenCV**

Arion requires OpenCV 3.0+ which must be compiled from source.  Download the latest archive from http://opencv.org/downloads.html or use wget get version 3.0.0

```bash
wget https://github.com/Itseez/opencv/archive/3.0.0.zip
unzip 3.0.0.zip 
cd opencv-3.0.0
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

**Build Arion**

This will create the final executable. You will need to create a new build directory and run CMake to generate the makefile.  CMake will let you know if any dependencies are missing.  

```bash
mkdir build
cd build
cmake ../src/
make
```

**Packaging**

A simple `.deb` package can be created using `fpm`.

```bash
sudo gem install fpm

cd build

mkdir deb
cd deb
mkdir -p usr/local/lib/
mkdir -p usr/local/include
cp ../libcarion.so usr/local/
cp ../../src/carion.h usr/local/include/
cd ..

# Create a .deb package for version 0.3.3
fpm -s dir -t deb --name arion --version 0.3.3 -C deb .
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