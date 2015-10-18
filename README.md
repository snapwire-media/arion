# Arion

Arion is a tool written in C++ that efficiently creates beautiful looking thumbnail images. 
It batch generates as many thumbnail sizes as desired and has the ability to apply sharpening to each one. 
It also has the ability to produce cropped thumbnails and apply a user-defined watermark. 
Each parameter is completely configurable via an JSON input and Arion can be called through any language that can execute shell commands.

## Installation
Currently this tool needs to be compiled from source to work on your host system. Install instructions are for Ubuntu, but can be easily modified to work on any *nix-based system.

Requirements
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
Download the archive from http://www.exiv2.org/download.html

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
This package doesn't require a particularly new Boost version (1.46) so the package maintainers version should work.

```bash
sudo apt-get install libboost-dev libboost-program-options-dev libboost-timer-dev libboost-filesystem-dev libboost-system-dev
```

**Install OpenCV**
Download the archive from http://opencv.org/downloads.html
```bash
mkdir build
cd build
cmake ../
make
sudo make install
```

**Build Resizer**
This will create the final executable
```bash
mkdir build
cd build
cmake ../
make
```
