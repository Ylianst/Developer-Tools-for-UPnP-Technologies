This folder contains sample code for various uses under BSD 2.0 license. 


DeviceProtectionSample
----------------------

This is experimental code that is not garantied to be compatible with the specification.

In order to use "DeviceProtectionSample", create a new folder called "openssl" at this location and extract OpenSSL inside of it.
You can get it at: http://openssl.org. For Windows, create an additional folder within the "openssl" folder called "libstatic"
and add the following files inside of it:

libeay32MD.lib
libeay32MDd.lib
libeay32MT.lib
libeay32MTd.lib
ssleay32MD.lib
ssleay32MDd.lib
ssleay32MT.lib
ssleay32MTd.lib

These Windows libraries can be compiled yourself or found at many places online. This project and solution file for this sample
are in Microsoft Windows 2010 format. The code will compile under VS 2008, but a new project and solution will have to be created.

Note that the DeviceProtection sample uses modified versions of the Microstack files.

For further information, open the "DeviceProtectionSample.doc" file.


EmbeddedSamples
---------------

This folder contains many historical samples for Windows and PocketPC build around 2006.

