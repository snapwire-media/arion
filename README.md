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
* **Compatability** - call from any framework that executes shell commands (see **[examples](../../wiki/Examples)**)
* **Extensibility** - use the modular operation framework and OpenCV to process images in new ways

## Installation
Currently this tool needs to be compiled from source to work on your host system. Install instructions are for Ubuntu, but can be easily modified to work on any *nix-based system.

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

**Install EXIV2**

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

**Install Boost**

Boost version 1.46+ is required to build Arion.  This is not a particularly new version so the package maintainers version will usually work.

```bash
sudo apt-get install libboost-dev libboost-program-options-dev libboost-timer-dev libboost-filesystem-dev libboost-system-dev
```

**Install OpenCV**

Arion requires OpenCV 3.0+ which must be compiled from source.  Download the archive from http://opencv.org/downloads.html

```bash
mkdir build
cd build
cmake ../
make
sudo make install
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
  "width" : 1296,
  "md5" : "636ee0572d42df5e3764372cb08d6ade",
  
  ...
```
