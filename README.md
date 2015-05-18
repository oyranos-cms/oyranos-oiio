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
... are typical cmake flags like CMAKE\_C\_FLAGS to tune compilation.

* CMAKE\_INSTALL\_PREFIX to install into paths and so on. Use on the command 
  line through -DCMAKE\_INSTALL\_PREFIX=/my/path .
* LIB\_SUFFIX - allows to append a architecture specific suffix like 
  LIB\_SUFFIX=64 for 64bit non debian style Linux systems.
