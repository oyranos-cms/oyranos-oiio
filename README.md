# Oyranos (gr. sky):

The Color Management System (CMS) Oyranos allows the coordination of
device specific Informations (ICC profiles) und system wide settings.
This project wraps OpenImageIO as Oyranos image reader filters.


### Internet:
* [WWW](http://www.oyranos.org)
* [wiki](http://www.oyranos.org/wiki/index.php?title=Oyranos)


### Dependencies:
####    Mandatory:
* [Oyranos](http://www.oyranos.org/)
* [OpenImageIO](http://www.openimageio.org)


### Building:
    $ mkdir build
    $ cd build
    $ cmake ..
    $ make
    $ make install

#### Build Flags
... are typical cmake flags like CMAKE_C_FLAGS to tune compilation.

* CMAKE_INSTALL_PREFIX to install into paths and so on. Use on the command 
  line through -DCMAKE_INSTALL_PREFIX=/my/path .
* LIB_SUFFIX - allows to append a architecture specific suffix like 
  LIB_SUFFIX=64 for 64bit non debian style Linux systems.
