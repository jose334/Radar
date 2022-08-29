==============================================================================
Python Examples Code using Radar SDK
==============================================================================

Requirements
------------

The examples in this directory require Python 3.7 or higher and following
modules:
    - numpy
    - scipy
    - matplotlib
    - pyqtgraph

We recommend to use the Anaconda Python distribution which includes all
required packages. You can download Anaconda from
https://www.anaconda.com/products/individual.

On Linux you can also use a package manager to install the dependencies. For
example, on Ubuntu 20.04 you can install the dependencies using apt:
    $ sudo apt update
    $ sudo apt install python3 python3-numpy python3-scipy


The application examples need the module ifxRadarSDK.py to be present in the
same directory. To run the examples, please copy the wrapper ifxRadarSDK.py in
sdk/py/wrapper_avian/src/AvianRDKWrapper to the examples directory.

In addition, copy the radar SDK library (libradar_sdk.so on Linux, radar_sdk.dll
on Windows) from lib/ARCH/ to the directory of the examples. Here, ARCH describes
your current platform which is either win32_x86 for 32bit Windows, win32_x64 for
64bit Windows, raspi for Raspbian Buster, or linux_x64 for Ubuntu 20.04.
