#cmake file for building Kolor-Manager and related modules
 
PROJECT(oyranos_oiio)
SET( PACKAGE_NAME oyranos-oiio)

SET( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake/modules )

CMAKE_MINIMUM_REQUIRED(VERSION 2.4)

# project version
SET( ${PROJECT_NAME}_MAJOR_VERSION 0)
SET( ${PROJECT_NAME}_MINOR_VERSION 9)
SET( ${PROJECT_NAME}_MICRO_VERSION 6)

SET( ${PROJECT_NAME}_VERSION ${${PROJECT_NAME}_MAJOR_VERSION}.${${PROJECT_NAME}_MINOR_VERSION}.${${PROJECT_NAME}_MICRO_VERSION} )

SET( OY_CMMSUBPATH     "color/modules" )

FIND_PACKAGE(Oyranos REQUIRED)
IF( OYRANOS_FOUND )
  INCLUDE_DIRECTORIES( ${OYRANOS_INCLUDE_DIRS} )
  LINK_DIRECTORIES( ${OYRANOS_LIBRARY_DIRS} )
  SET( HAVE_OYRAOS 1 )
ENDIF( OYRANOS_FOUND )

FIND_PACKAGE( JPEG )
IF( JPEG_FOUND )
  INCLUDE_DIRECTORIES( ${JPEG_INCLUDE_DIR} )
  LINK_DIRECTORIES( ${JPEG_LIBRARY} )
  SET( HAVE_JPEG 1 )
ENDIF( JPEG_FOUND )

FIND_PACKAGE( TIFF )
IF( TIFF_FOUND )
  INCLUDE_DIRECTORIES( ${TIFF_INCLUDE_DIR} )
  LINK_DIRECTORIES( ${TIFF_LIBRARY} )
  SET( HAVE_TIFF 1 )
ENDIF( TIFF_FOUND )

FIND_PACKAGE( OIIO  REQUIRED )
IF( OIIO_FOUND )
  INCLUDE_DIRECTORIES( ${OIIO_INCLUDE_DIR} )
  LINK_DIRECTORIES( ${OIIO_LIBRARY} )
  SET( HAVE_OIIO 1 )
ENDIF( OIIO_FOUND )

FIND_PACKAGE( OpenMP )
IF(OPENMP_FOUND)
  INCLUDE_DIRECTORIES( ${OPENMP_INCLUDE_DIRS} )
  LINK_DIRECTORIES( ${OPENMP_LIBRARY_DIRS} )
  SET( EXTRA_LIBS ${EXTRA_LIBS} ${OPENMP_LIBRARIES} )
  SET( HAVE_OPENMP 1 )
  SET( CMAKE_C_FLAGS_CORE "${CMAKE_C_FLAGS}" )
  SET( CMAKE_CXX_FLAGS_CORE "${CMAKE_CXX_FLAGS}" )
  SET( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}" )
  SET( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}" )
ENDIF(OPENMP_FOUND)


SET( ${CMAKE_BUILD_TYPE} Debug)

#now add all directories
ADD_SUBDIRECTORY( extras )

CONFIGURE_FILE(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/${PACKAGE_NAME}Uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake/${PACKAGE_NAME}Uninstall.cmake"
    IMMEDIATE @ONLY)

ADD_CUSTOM_TARGET( uninstall
	"${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake/${PACKAGE_NAME}Uninstall.cmake")

ADD_CUSTOM_TARGET( dist
                COMMAND cd ${CMAKE_CURRENT_SOURCE_DIR}/ && git archive --prefix=${PACKAGE_NAME}-${${PROJECT_NAME}_VERSION}/ --format=tar HEAD > ${CMAKE_CURRENT_BINARY_DIR}/${PACKAGE_NAME}-${${PROJECT_NAME}_VERSION}.tar
                COMMAND echo "git sha1:  " | tr -d "\\n" >> ${CMAKE_CURRENT_BINARY_DIR}/${PACKAGE_NAME}-${${PROJECT_NAME}_VERSION}.tar.bz2.hashsum
                COMMAND cd ${CMAKE_CURRENT_SOURCE_DIR}/ && git rev-parse HEAD >> ${CMAKE_CURRENT_BINARY_DIR}/${PACKAGE_NAME}-${${PROJECT_NAME}_VERSION}.tar.bz2.hashsum
                COMMAND rm -rf ${PACKAGE_NAME}-${${PROJECT_NAME}_VERSION} && tar xf ${PACKAGE_NAME}-${${PROJECT_NAME}_VERSION}.tar
                COMMAND tar cf - ${PACKAGE_NAME}-${${PROJECT_NAME}_VERSION} | bzip2 > ${PACKAGE_NAME}-${${PROJECT_NAME}_VERSION}.tar.bz2
                COMMAND echo  "size:      " | tr -d "\\n" >> ${PACKAGE_NAME}-${${PROJECT_NAME}_VERSION}.tar.bz2.hashsum
                COMMAND du -b ${PACKAGE_NAME}-${${PROJECT_NAME}_VERSION}.tar.bz2 | sed s/[[:space:]][[:print:]]*// >> ${PACKAGE_NAME}-${${PROJECT_NAME}_VERSION}.tar.bz2.hashsum
                COMMAND echo  "md5sum:    " | tr -d "\\n" >> ${PACKAGE_NAME}-${${PROJECT_NAME}_VERSION}.tar.bz2.hashsum
                COMMAND cd ${CMAKE_CURRENT_BINARY_DIR}/ && md5sum ${PACKAGE_NAME}-${${PROJECT_NAME}_VERSION}.tar.bz2 >> ${PACKAGE_NAME}-${${PROJECT_NAME}_VERSION}.tar.bz2.hashsum
                COMMAND echo  "sha1sum:   " | tr -d "\\n" >> ${PACKAGE_NAME}-${${PROJECT_NAME}_VERSION}.tar.bz2.hashsum
                COMMAND sha1sum ${PACKAGE_NAME}-${${PROJECT_NAME}_VERSION}.tar.bz2 >> ${PACKAGE_NAME}-${${PROJECT_NAME}_VERSION}.tar.bz2.hashsum
                COMMAND echo  "sha256sum: " | tr -d "\\n" >> ${PACKAGE_NAME}-${${PROJECT_NAME}_VERSION}.tar.bz2.hashsum
                COMMAND sha256sum ${PACKAGE_NAME}-${${PROJECT_NAME}_VERSION}.tar.bz2 >> ${PACKAGE_NAME}-${${PROJECT_NAME}_VERSION}.tar.bz2.hashsum
                COMMENT Make a ${PACKAGE_NAME} source package by using git
                VERBATIM
                WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
                )
EXECUTE_PROCESS(COMMAND rpmbuild --showrc
                COMMAND awk '/^build arch/ {print $$4}'
                OUTPUT_VARIABLE RPMARCH
                ERROR_VARIABLE RPMBUILD_ERROR
               )
ADD_CUSTOM_TARGET( rpm
		COMMAND test -d ${PACKAGE_NAME}-${${PROJECT_NAME}_VERSION}/ && rm -rf ${PACKAGE_NAME}-${${PROJECT_NAME}_VERSION}/
		COMMAND tar xjf ${PACKAGE_NAME}-${${PROJECT_NAME}_VERSION}.tar.bz2
                COMMAND mkdir -p rpmdir/BUILD rpmdir/SPECS rpmdir/SOURCES rpmdir/SRPMS rpmdir/RPMS/${RPMARCH}
                COMMAND cp -f ${PACKAGE_NAME}-${${PROJECT_NAME}_VERSION}.tar.bz2 rpmdir/SOURCES
                COMMAND rpmbuild --clean -ba extras/${PACKAGE_NAME}.spec --define "_topdir ${CMAKE_CURRENT_BINARY_DIR}/rpmdir"
                DEPENDS dist
		COMMENT Make a ${PACKAGE_NAME} RPM package
		VERBATIM
		)

ADD_CUSTOM_TARGET( debsrc
		COMMAND test -d ${PACKAGE_NAME}-${${PROJECT_NAME}_VERSION}/ && rm -rf ${PACKAGE_NAME}-${${PROJECT_NAME}_VERSION}/
		COMMAND tar xjf ${PACKAGE_NAME}-${${PROJECT_NAME}_VERSION}.tar.bz2
                COMMAND mv ${PACKAGE_NAME}-${${PROJECT_NAME}_VERSION}.tar.bz2 ${PACKAGE_NAME}_${${PROJECT_NAME}_VERSION}.orig.tar.bz2
                COMMAND cp -R ${CMAKE_SOURCE_DIR}/extras/debian ${PACKAGE_NAME}-${${PROJECT_NAME}_VERSION}/
                COMMAND dpkg-source -b ${PACKAGE_NAME}-${${PROJECT_NAME}_VERSION}/
                DEPENDS dist
		COMMENT Make a ${PROJECT_NAME} DEB source package
		VERBATIM
		)
ADD_CUSTOM_TARGET( debian
		COMMAND cd ${PACKAGE_NAME}-${${PROJECT_NAME}_VERSION}/ && debuild
                DEPENDS debsrc
		COMMENT Make a ${PROJECT_NAME} DEB package
		VERBATIM
		)

 
SET( OY_MODULE_NAME "_cmm_module" )

# --- extra modules ---

SET( OY_MODULE oiio )
SET( OY_MODULE_LIB ${PROJECT_NAME}${OY_MODULE_NAME} )
ADD_LIBRARY( ${OY_MODULE_LIB}
             SHARED oyranos_cmm_${OY_MODULE}.cpp iccjpeg.c )
TARGET_LINK_LIBRARIES ( ${OY_MODULE_LIB}  ${OYRANOS_LIBRARIES} ${OIIO_LIBRARY} ${EXTRA_LIBS} )
INSTALL (TARGETS ${OY_MODULE_LIB} DESTINATION lib${LIB_SUFFIX}/${OYRANOS_CMMSUBPATH})
