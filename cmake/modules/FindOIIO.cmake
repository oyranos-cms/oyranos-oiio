# - Try to find OpenImageIO
# Once done this will define
#
#  OIIO_FOUND - system has OpenImageIO
#  OIIO_INCLUDE_DIR - the OpenImageIO include directory
#  OIIO_LIBRARY - Link these to use OpenImageIO

# Copyright (c) 2006, Alexander Neundorf, <neundorf@kde.org>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

# This file is based on cmake-2.6/Modules/FindBZip2.cmake
# Copyright (c) 2010, Yiannis Belias, <jonnyb@hol.gr>
# modify for oiio
# Copyright (c) 2014, Kai-Uwe Behrmann, <ku.b@gmx.de>

IF (OIIO_INCLUDE_DIR AND OIIO_LIBRARY)
    SET(OIIO_FIND_QUIETLY TRUE)
ENDIF (OIIO_INCLUDE_DIR AND OIIO_LIBRARY)

FIND_PATH(OIIO_INCLUDE_DIR OpenImageIO/imageio.h )

FIND_LIBRARY(OIIO_LIBRARY NAMES OpenImageIO )

# handle the QUIETLY and REQUIRED arguments and set OIIO_FOUND to TRUE if 
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(OIIO DEFAULT_MSG OIIO_LIBRARY OIIO_INCLUDE_DIR)

IF(OIIO_FOUND)
  SET( HAVE_OIIO TRUE )
ENDIF(OIIO_FOUND)

MARK_AS_ADVANCED(OIIO_INCLUDE_DIR OIIO_LIBRARY)

