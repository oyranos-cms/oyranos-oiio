/** @file oyranos_cmm_oiio.c
 *
 *  OpenImageIO based file i/o module for Oyranos 
 *
 *  @par Copyright:
 *            2014 (C) Kai-Uwe Behrmann
 *
 *  @brief    OIIO filter for Oyranos
 *  @internal
 *  @author   Kai-Uwe Behrmann <oy@oyranos.org>
 *  @par License:
 *            new BSD <http://www.opensource.org/licenses/bsd-license.php>
 *  @since    2014/03/21
 */

#include "oyCMM_s.h"
#include "oyCMMapi4_s.h"
#include "oyCMMapi7_s.h"
#include "oyCMMapiFilter_s.h"
#include "oyCMMui_s.h"
#include "oyConnectorImaging_s.h"
#include "oyProfiles_s.h"

#include "oyranos_config.h"
#include "oyranos_definitions.h"

#include <math.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>   /* isspace() */

#include <OpenImageIO/imageio.h>
#include "iccjpeg.h"
#include <png.h>
#include <tiffconf.h>
#include <tiffio.h>

/* --- internal definitions --- */

/** The CMM_NICK consists of four bytes, which appear as well in the library name. This is important for Oyranos to identify the required filter struct name. */
#define CMM_NICK "oiio"
#define OY_OIIO_FILTER_REGISTRATION OY_TOP_INTERNAL OY_SLASH OY_DOMAIN_INTERNAL OY_SLASH OY_TYPE_STD OY_SLASH "file_loader"

/** The message function pointer to use for messaging. */
oyMessage_f oiio_msg = oyMessageFunc;

/* Helpers */
#if defined(__GNUC__)
# define  OY_DBG_FORMAT_ "%s:%d %s() "
# define  OY_DBG_ARGS_   strrchr(__FILE__,'/') ? strrchr(__FILE__,'/')+1 : __FILE__,__LINE__,__func__
#else
# define  OY_DBG_FORMAT_ "%s:%d "
# define  OY_DBG_ARGS_   strrchr(__FILE__,'/') ? strrchr(__FILE__,'/')+1 : __FILE__,__LINE__
#endif
#define _DBG_FORMAT_ OY_DBG_FORMAT_
#define _DBG_ARGS_ OY_DBG_ARGS_

/* i18n */
#ifdef USE_GETTEXT
# include <libintl.h>
# define _(text) dgettext( "oyranos_"CMM_NICK, text )
#else
# define _(text) text
#endif

extern "C" {
void* oyAllocateFunc_           (size_t        size);
void  oyDeAllocateFunc_         (void *        data);
#define AD oyAllocateFunc_, oyDeAllocateFunc_

int  oiioInit                        ( oyStruct_s        * module_info );
int      oiioFilter_CmmRun           ( oyFilterPlug_s    * requestor_plug,
                                       oyPixelAccess_s   * ticket );
const char * oiioApi4UiGetText2      ( const char        * select,
                                       oyNAME_e            type,
                                       const char        * format );
int                oiioGetOFORMS     ( oyCMMapiFilter_s  * module,
                                       oyOptions_s       * oy_opts,
                                       char             ** ui_text,
                                       oyAlloc_f           allocateFunc );
const char * oiioApi4UiGetText       ( const char        * select,
                                       oyNAME_e            type,
                                       oyStruct_s        * context );
extern const char * oiio_api4_ui_texts[];
char * oiioFilterNode_GetText        ( oyFilterNode_s    * node,
                                       oyNAME_e            type,
                                       oyAlloc_f           allocateFunc );
}

/* --- implementations --- */

/** Function oiioCMMInit
 *  @brief API requirement
 *
 *  @version Oyranos: 0.9.6
 *  @since   2014/03/21 (Oyranos: 0.9.6)
 *  @date    2014/03/21
 */
int                oiioCMMInit       ( oyStruct_s * )
{
  int error = 0;
  return error;
}



/** Function oiioCMMMessageFuncSet
 *  @brief API requirement
 *
 *  A Oyranos user might want its own message function and omit the default
 *  one.
 *
 *  @version Oyranos: 0.9.6
 *  @since   2014/03/21 (Oyranos: 0.9.6)
 *  @date    2014/03/21
 */
int            oiioCMMMessageFuncSet ( oyMessage_f         message_func )
{
  oiio_msg = message_func;
  return 0;
}


/**
 *  This function implements oyCMMinfoGetText_f.
 *
 *  Implement at least "name", "manufacturer" and "copyright". If you like with
 *  internationalisation.
 *
 *  @version Oyranos: 0.9.6
 *  @since   2014/03/21 (Oyranos: 0.9.6)
 *  @date    2014/03/21
 */
const char * oiioGetText             ( const char        * select,
                                       oyNAME_e            type,
                                       oyStruct_s        * context )
{
         if(strcmp(select, "name")==0)
  {
         if(type == oyNAME_NICK)
      return _(CMM_NICK);
    else if(type == oyNAME_NAME)
      return _("CMM loader");
    else
      return _("CMM loader filter");
  } else if(strcmp(select, "manufacturer")==0)
  {
         if(type == oyNAME_NICK)
      return _("oy");
    else if(type == oyNAME_NAME)
      return _("Kai-Uwe Behrmann");
    else
      return _("Oyranos project; www: http://www.oyranos.com; support/email: ku.b@gmx.de; sources: http://www.oyranos.com/downloads");
  } else if(strcmp(select, "copyright")==0)
  {
         if(type == oyNAME_NICK)
      return _("newBSD");
    else if(type == oyNAME_NAME)
      return _("Copyright (c) 2014 Kai-Uwe Behrmann; newBSD");
    else
      return _("new BSD license: http://www.opensource.org/licenses/bsd-license.php");
  } else if(strcmp(select, "help")==0)
  {
         if(type == oyNAME_NICK)
      return _("help");
    else if(type == oyNAME_NAME)
      return _("The module generates a series of file format reader filters.");
    else
      return _("The module generates a series of file format reader filters. The OpenImageIO library is used to perform the actual reading. This module performs for several formats ICC profile detection, generation and fallback generation.");
  }
  return 0;
}
const char *oiio_texts[5] = {"name","copyright","manufacturer","help",0};
oyIcon_s oiio_icon = {oyOBJECT_ICON_S, 0,0,0, 0,0,0, "oyranos_logo.png"};

/** @instance oiio_cmm_module
 *  @brief    oiio module infos
 *
 *  This structure is dlopened by Oyranos. Its name has to consist of the
 *  following elements:
 *  - the four byte CMM_NICK plus
 *  - "_cmm_module"
 *  This string must be included in the the filters filename.
 *
 *  @version Oyranos: 0.9.6
 *  @since   2014/03/21 (Oyranos: 0.9.6)
 *  @date    2014/03/21
 */
oyCMM_s oiio_cmm_module = {

  oyOBJECT_CMM_INFO_S, /**< ::type; the object type */
  0,0,0,               /**< static objects omit these fields */
  CMM_NICK,            /**< ::cmm; the four char filter id */
  (char*)"0.9.6",      /**< ::backend_version */
  oiioGetText,         /**< ::getText; UI texts */
  (char**)oiio_texts,  /**< ::texts; list of arguments to getText */
  OYRANOS_VERSION,     /**< ::oy_compatibility; last supported Oyranos CMM API*/

  /** ::api; The first filter api structure. */
  NULL,

  /** ::icon; module icon */
  &oiio_icon,
  oiioInit
};


/* OY_OIIO_FILTER_REGISTRATION ----------------------------------------------*/

#define OY_OIIO_FILTER_REGISTRATION_BASE OY_TOP_SHARED OY_SLASH OY_DOMAIN_INTERNAL OY_SLASH OY_TYPE_STD OY_SLASH

const char *icc_file_formats[5] = {"jpeg","tiff","png",0,0};

/** @instance oiio_api7
 *  @brief    oiio oyCMMapi7_s implementation
 *
 *  a filter providing a CMM filter
 *
 *  @version Oyranos: 0.9.6
 *  @date    2014/03/21
 *  @since   2014/03/21 (Oyranos: 0.9.6)
 */
oyCMMapi_s * oiioApi7CmmCreate       ( const char        * format,
                                       const char        * ext )
{
  int32_t cmm_version[3] = {OYRANOS_VERSION_A,OYRANOS_VERSION_B,OYRANOS_VERSION_C},
          module_api[3]  = {0,9,6};
  static oyDATATYPE_e data_types[7] = {oyUINT8, oyUINT16, oyUINT32,
                                       oyHALF, oyFLOAT, oyDOUBLE, (oyDATATYPE_e)0};
  oyConnectorImaging_s * plug = oyConnectorImaging_New(0),
                       * socket = oyConnectorImaging_New(0);
  static oyConnectorImaging_s * plugs[2] = {0,0},
                              * sockets[2] = {0,0};
  
  char * ext_ = NULL;
  const char * properties[] =
  {
    "file=read",    /* file read|write */
    "image=pixel",  /* image type, pixel/vector/font */
    "layers=1",     /* layer count, one for plain images */
    "icc=0",        /* image type ICC profile support */
    "ext=", /* supported extensions */
    0
  };
  oyStringAddPrintf( &ext_, oyAllocateFunc_, oyDeAllocateFunc_, "ext=%s", ext+1 );
  properties[4] = ext_;

  int pos = 0;
  while(icc_file_formats[pos])
    if(strcmp(icc_file_formats[pos++],format))
      properties[3] = "icc=1"; /* image type ICC profile support */

  plugs[0] = plug;
  sockets[0] = socket;
  char * registration = NULL;

  oyStringAddPrintf( &registration, AD,
                     OY_OIIO_FILTER_REGISTRATION_BASE"file_read.input_%s._%s._CPU._ACCEL", format, CMM_NICK );

  if(oy_debug >= 2) oiio_msg(oyMSG_DBG, NULL, _DBG_FORMAT_ "registration:%s oiio v%d %s", _DBG_ARGS_,
                             registration,
                             OpenImageIO::openimageio_version(), ext_ );

#if 0
  oyConnectorImaging_SetDataTypes( plug, data_types, 6 );
  oyConnectorImaging_SetReg( plug, "//" OY_TYPE_STD "/image.data" );
  oyConnectorImaging_SetMatch( plug, oyFilterSocket_MatchImagingPlug );
  oyConnectorImaging_SetTexts( plug, oyCMMgetImageConnectorPlugText,
                               oy_image_connector_texts );
  oyConnectorImaging_SetIsPlug( plug, 1 );
  oyConnectorImaging_SetCapability( plug, oyCONNECTOR_IMAGING_CAP_MAX_COLOR_OFFSET, -1 );
  oyConnectorImaging_SetCapability( plug, oyCONNECTOR_IMAGING_CAP_MIN_CHANNELS_COUNT, 1 );
  oyConnectorImaging_SetCapability( plug, oyCONNECTOR_IMAGING_CAP_MAX_CHANNELS_COUNT, 16 );
  oyConnectorImaging_SetCapability( plug, oyCONNECTOR_IMAGING_CAP_MIN_COLOR_COUNT, 1 );
  oyConnectorImaging_SetCapability( plug, oyCONNECTOR_IMAGING_CAP_MAX_COLOR_COUNT, 16 );
  oyConnectorImaging_SetCapability( plug, oyCONNECTOR_IMAGING_CAP_CAN_INTERWOVEN, 1 );
  oyConnectorImaging_SetCapability( plug, oyCONNECTOR_IMAGING_CAP_CAN_PREMULTIPLIED_ALPHA, 1 );
  oyConnectorImaging_SetCapability( plug, oyCONNECTOR_IMAGING_CAP_CAN_NONPREMULTIPLIED_ALPHA, 1 );
  oyConnectorImaging_SetCapability( plug, oyCONNECTOR_IMAGING_CAP_ID, 1 );
#endif
                               

  oyConnectorImaging_SetDataTypes( socket, data_types, 6 );
  oyConnectorImaging_SetReg( socket, "//" OY_TYPE_STD "/image.data" );
  oyConnectorImaging_SetMatch( socket, oyFilterSocket_MatchImagingPlug );
  oyConnectorImaging_SetTexts( socket, oyCMMgetImageConnectorSocketText,
                               oy_image_connector_texts );
  oyConnectorImaging_SetIsPlug( socket, 0 );
  oyConnectorImaging_SetCapability( socket, oyCONNECTOR_IMAGING_CAP_MAX_COLOR_OFFSET, -1 );
  oyConnectorImaging_SetCapability( socket, oyCONNECTOR_IMAGING_CAP_MIN_CHANNELS_COUNT, 1 );
  oyConnectorImaging_SetCapability( socket, oyCONNECTOR_IMAGING_CAP_MAX_CHANNELS_COUNT, 16 );
  oyConnectorImaging_SetCapability( socket, oyCONNECTOR_IMAGING_CAP_MIN_COLOR_COUNT, 1 );
  oyConnectorImaging_SetCapability( socket, oyCONNECTOR_IMAGING_CAP_MAX_COLOR_COUNT, 16 );
  oyConnectorImaging_SetCapability( socket, oyCONNECTOR_IMAGING_CAP_CAN_INTERWOVEN, 1 );
  oyConnectorImaging_SetCapability( socket, oyCONNECTOR_IMAGING_CAP_CAN_PREMULTIPLIED_ALPHA, 1 );
  oyConnectorImaging_SetCapability( socket, oyCONNECTOR_IMAGING_CAP_CAN_NONPREMULTIPLIED_ALPHA, 1 );
  oyConnectorImaging_SetCapability( socket, oyCONNECTOR_IMAGING_CAP_ID, 1 );

  oyCMMapi7_s * cmm7 = oyCMMapi7_Create( oiioCMMInit, oiioCMMMessageFuncSet,
                                       registration,
                                       cmm_version, module_api,
                                       NULL,
                                       oiioFilter_CmmRun,
                                       (oyConnector_s**)plugs, 0, 0,
                                       (oyConnector_s**)sockets, 1, 0,
                                       properties, 0 );
  //oyFree_m_( registration );
  return (oyCMMapi_s*) cmm7;
}

int deAllocData ( void ** data ) { oyDeAllocateFunc_(*data); *data = NULL; return 0; }
const char oiio_read_extra_options[] = {
 "\n\
  <" OY_TOP_SHARED ">\n\
   <" OY_DOMAIN_INTERNAL ">\n\
    <" OY_TYPE_STD ">\n\
     <" "file_read" ">\n\
      <filename></filename>\n\
     </" "file_read" ">\n\
    </" OY_TYPE_STD ">\n\
   </" OY_DOMAIN_INTERNAL ">\n\
  </" OY_TOP_SHARED ">\n"
};

/** @instance oiio_api4
 *  @brief    oiio oyCMMapi4_s implementation
 *
 *  a filter providing a CMM device link creator
 *
 *  @version Oyranos: 0.9.6
 *  @since   2014/03/21 (Oyranos: 0.9.6)
 *  @date    2014/03/21
 */
oyCMMapi_s * oiioApi4CmmCreate       ( const char        * format )
{
  int32_t cmm_version[3] = {OYRANOS_VERSION_A,OYRANOS_VERSION_B,OYRANOS_VERSION_C},
          module_api[3]  = {0,9,6};
  oyPointer_s * backend_context = oyPointer_New(0);
  char * registration = NULL;
  const char * category = oiioApi4UiGetText2("category", oyNAME_NAME, format);
  oyCMMuiGet_f getOFORMS = oiioGetOFORMS;
  oyCMMui_s * ui = oyCMMui_Create( category, oiioApi4UiGetText,
                                   oiio_api4_ui_texts, 0 );
  oyOptions_s * oy_opts = NULL;
  const char * oforms_options = oiio_read_extra_options;

  oyCMMui_SetUiOptions( ui, oyStringCopy( oforms_options, oyAllocateFunc_ ), getOFORMS ); 

  oyPointer_Set( backend_context, NULL, "oiio_file_format", oyStringCopy(format, oyAllocateFunc_),
                 "char*", deAllocData );

  oyStringAddPrintf( &registration, AD,
                     OY_OIIO_FILTER_REGISTRATION_BASE"file_read.input_%s._oiio._CPU._ACCEL", format );

  oyCMMapi4_s * cmm4 = oyCMMapi4_Create( oiioCMMInit, oiioCMMMessageFuncSet,
                                       registration,
                                       cmm_version, module_api,
                                       "",
                                       NULL,
                                       oiioFilterNode_GetText,
                                       ui,
                                       NULL );

  oyCMMapi4_SetBackendContext( cmm4, backend_context );
  oyOptions_Release( &oy_opts );

  return (oyCMMapi_s*)cmm4;
}


char * oiioFilterNode_GetText        ( oyFilterNode_s    * node,
                                       oyNAME_e            type,
                                       oyAlloc_f           allocateFunc )
{
  char * t = NULL;
  const char * tmp = NULL;
  oyOptions_s * node_options = oyFilterNode_GetOptions( node, 0 );

  tmp = oyOptions_GetText(node_options, oyNAME_NICK);
  if(tmp)
    t = oyStringCopy( tmp, allocateFunc );

  oyOptions_Release( &node_options );

  return t;
}

#define A(long_text) oyStringAdd_( &tmp, long_text, AD )

int                oiioGetOFORMS     ( oyCMMapiFilter_s  * module,
                                       oyOptions_s       * oy_opts,
                                       char             ** ui_text,
                                       oyAlloc_f           allocateFunc )
{
  int error = 0;
  char * tmp = NULL;

  *ui_text = tmp;
  
  return error;
}


oyOptions_s* oiioFilter_CmmLoaderValidateOptions
                                     ( oyFilterCore_s    * filter,
                                       oyOptions_s       * validate,
                                       int                 statical,
                                       uint32_t          * result )
{
  uint32_t error = !filter;

#if 0
  if(!error)
    error = !oyOptions_FindString( validate, "my_options", 0 );
#endif

  *result = error;

  return 0;
}


void oPNGerror( png_structp png, const char * text )
{
  oiio_msg( oyMSG_ERROR, (oyStruct_s*)NULL/*node*/,
             OY_DBG_FORMAT_ "%s",
             OY_DBG_ARGS_, text );
}
void oPNGwarn( png_structp png, const char * text )
{
  oiio_msg( oyMSG_WARN, (oyStruct_s*)NULL/*node*/,
             OY_DBG_FORMAT_ "%s",
             OY_DBG_ARGS_, text );
}

oyProfile_s * profileFromMatrix( double pandg[9], const char * name  )
{
  oyProfile_s * p = 0;
            oyOption_s * primaries = oyOption_FromRegistration( "//" 
                    OY_TYPE_STD 
                    "/color_matrix."
                    "redx_redy_greenx_greeny_bluex_bluey_whitex_whitey_gamma",
                    0);
              oyOptions_s * opts = oyOptions_New(0),
                          * result = 0;

            int pos = 0;

            /* red */
            oyOption_SetFromDouble( primaries, pandg[pos], pos, 0 ); pos++;
            oyOption_SetFromDouble( primaries, pandg[pos], pos, 0 ); pos++;
            /* green */
            oyOption_SetFromDouble( primaries, pandg[pos], pos, 0 ); pos++;
            oyOption_SetFromDouble( primaries, pandg[pos], pos, 0 ); pos++;
            /* blue */
            oyOption_SetFromDouble( primaries, pandg[pos], pos, 0 ); pos++;
            oyOption_SetFromDouble( primaries, pandg[pos], pos, 0 ); pos++;
            /* white */
            oyOption_SetFromDouble( primaries, pandg[pos], pos, 0 ); pos++;
            oyOption_SetFromDouble( primaries, pandg[pos], pos, 0 ); pos++;
            /* gamma */
            oyOption_SetFromDouble( primaries, pandg[pos], pos, 0 ); pos++;

            oyOptions_MoveIn( opts, &primaries, -1 );
            oyOptions_Handle( "//"OY_TYPE_STD"/create_profile.icc",
                                opts,"create_profile.icc_profile.color_matrix",
                                &result );
            p = (oyProfile_s*)oyOptions_GetType( result, -1, "icc_profile",
                                        oyOBJECT_PROFILE_S );
            oyProfile_AddTagText( p, icSigProfileDescriptionTag, name);
            oyProfile_AddTagText( p, icSigCopyrightTag, "ICC License 2011");
            oyOptions_Release( &result );
            oyOptions_Release( &opts );
  return p;
}

/** Function oiioFilter_CmmRun
 *  @brief   implement oyCMMFilter_GetNext_f()
 *
 *  The primary filter entry for data processing.
 *
 *  @param         requestor_plug      the plug of the requesting node after 
 *                                     my filter in the graph
 *  @param         ticket              the job ticket
 *
 *  @version Oyranos: 0.9.6
 *  @since   2014/03/21 (Oyranos: 0.9.6)
 *  @date    2014/03/21
 */
int      oiioFilter_CmmRun           ( oyFilterPlug_s    * requestor_plug,
                                       oyPixelAccess_s   * ticket )
{
  oyFilterSocket_s * socket = 0;
  oyStruct_s * socket_data = 0;
  oyFilterNode_s * node = 0;
  oyOptions_s * tags = 0;
  int error = 0;
  const char * filename = 0;
  FILE * fp = 0;
  oyDATATYPE_e data_type = oyUINT8;
  oyPROFILE_e profile_type = oyASSUMED_RGB;
  oyProfile_s * prof = 0;
  oyImage_s * image_in = 0,
            * output_image = 0;
  oyPixel_t pixel_type = 0;
  size_t  fsize = 0;
  uint8_t * buf = 0;
  size_t  mem_n = 0;   /* needed memory in bytes */
    
  int info_good = 1;
  int32_t icc_profile_flags = 0;

  if(requestor_plug->type_ == oyOBJECT_FILTER_PLUG_S)
  {
    socket = oyFilterPlug_GetSocket( requestor_plug );
    socket_data = oyFilterSocket_GetData( socket );
  }

  /* passing through the data reading */
  if(requestor_plug->type_ == oyOBJECT_FILTER_PLUG_S &&
     socket_data)
  {
    error = oyFilterPlug_ImageRootRun( requestor_plug, ticket );

    return error;

  } else if(requestor_plug->type_ == oyOBJECT_FILTER_SOCKET_S)
  {
    /* To open the a image here seems not so straight forward.
     * Still the plug-in should be prepared to initialise the image data before
     * normal processing occurs.
     */
    socket = oyFilterSocket_Copy( (oyFilterSocket_s*)requestor_plug, 0 );
    requestor_plug = 0;
  }

  node = oyFilterSocket_GetNode( socket );

  /* parse options */
  if(error <= 0)
  {
    oyOptions_s * opts = oyFilterNode_GetOptions( node ,0 );
    filename = oyOptions_FindString( opts, "filename", 0 );
    oyOptions_FindInt( opts, "icc_profile_flags", 0, &icc_profile_flags );
    oyOptions_Release( &opts );
  }

  /* file tests */
  if(filename)
    fp = fopen( filename, "rm" );

  if(!fp)
  {
    oiio_msg( oyMSG_WARN, (oyStruct_s*)node,
             OY_DBG_FORMAT_ " could not open: %s",
             OY_DBG_ARGS_, oyNoEmptyString_m( filename ) );
    return 1;
  }

  /* file size fun */
  fseek(fp,0L,SEEK_END);
  fsize = ftell(fp);
  rewind(fp);
  if(oy_debug)
    oiio_msg( oyMSG_DBG, (oyStruct_s*)node,
             OY_DBG_FORMAT_ "file size %u",
             OY_DBG_ARGS_, fsize );

  /* open the image */
  OpenImageIO::ImageInput * image = OpenImageIO::ImageInput::create( filename );
  info_good = image ? 1:0;

  if( !info_good )
  {
    oiio_msg( oyMSG_WARN, (oyStruct_s*)node,
             OY_DBG_FORMAT_ "failed to get info of %s\n%s",
             OY_DBG_ARGS_, oyNoEmptyString_m( filename ),
             OpenImageIO::geterror().c_str() );
    return FALSE;
  }

  /* get image informations */
  OpenImageIO::ImageSpec spec;
  image->open( std::string(filename), spec );
  // set a ICC profile
  // spec.attribute ("icc-profile", TypeDesc(TypeDesc::UINT8, blobsize), &blob);

  OpenImageIO::TypeDesc::BASETYPE type = (OpenImageIO::TypeDesc::BASETYPE) spec.format.basetype;
  switch(spec.format.basetype)
  {
    case OpenImageIO::TypeDesc::UINT8: data_type = oyUINT8; break;
    case OpenImageIO::TypeDesc::UINT16: data_type = oyUINT16; break;
    case OpenImageIO::TypeDesc::UINT32: data_type = oyUINT32; break;
    case OpenImageIO::TypeDesc::HALF: data_type = oyHALF; break;
    case OpenImageIO::TypeDesc::FLOAT: data_type = oyFLOAT; break;
    case OpenImageIO::TypeDesc::DOUBLE: data_type = oyDOUBLE; break;
    default: data_type = oyFLOAT; type = OpenImageIO::TypeDesc::FLOAT; break;
  }

  if( !info_good )
  {
    oiio_msg( oyMSG_WARN, (oyStruct_s*)node,
             OY_DBG_FORMAT_ "failed to handle %s %s",
             OY_DBG_ARGS_, oyNoEmptyString_m( filename ),
             spec.format.c_str() );
    image->close();
    return FALSE;
  }

  pixel_type = oyChannels_m(spec.nchannels) | oyDataType_m(data_type); 

  /* allocate a buffer to hold the whole image */
  mem_n = spec.width*spec.height*oyDataTypeGetSize(data_type)*spec.nchannels;
  if(mem_n)
  {
    buf = (uint8_t*) oyAllocateFunc_(mem_n * sizeof(uint8_t));
    if(!buf)
    {
      oiio_msg(oyMSG_WARN, (oyStruct_s *) 0, _DBG_FORMAT_ "Could not allocate enough memory.", _DBG_ARGS_);
      return 1;
    }
  }
  if(oy_debug)
  oiio_msg( oyMSG_DBG, (oyStruct_s *) 0, "allocate image data: 0x%x size: %d ", (int)(intptr_t)
            buf, mem_n );

  /* decode the image into our buffer */
  image->read_image( type, buf );


  /* get ICC Profile */
  if(strcmp(image->format_name(),"jpeg") == 0)
  {
    struct jpeg_decompress_struct cinfo; 
    struct jpeg_error_mgr jerr;

    // Setup decompression structure
    cinfo.err = jpeg_std_error(&jerr); 
    jpeg_create_decompress (&cinfo);

    jpeg_stdio_src (&cinfo, fp);

    setup_read_icc_profile(&cinfo);  
    for (int m = 0; m < 16; m++)
      jpeg_save_markers(&cinfo, JPEG_APP0 + m, 0xFFFF);

    (void) jpeg_read_header (&cinfo, TRUE);

    int lIsITUFax = prepareColour( &cinfo );

    jpeg_start_decompress (&cinfo);


    unsigned char * icc = NULL;
    unsigned int len = 0;

    if (read_icc_profile(&cinfo, &icc, &len))
    { if(oy_debug)
      oiio_msg( oyMSG_DBG, (oyStruct_s*)node, OY_DBG_FORMAT_ "jpeg embedded profile found: %d", OY_DBG_ARGS_, len);
    } else if (read_icc_profile2(&cinfo, filename, &icc, &len))
    { if(oy_debug)
      oiio_msg( oyMSG_DBG, (oyStruct_s*)node, OY_DBG_FORMAT_ "jpeg default profile selected: %d", OY_DBG_ARGS_, len);
    } else
      if(oy_debug)
      oiio_msg( oyMSG_DBG, (oyStruct_s*)node, OY_DBG_FORMAT_ "jpeg no profile found", OY_DBG_ARGS_);

    if(icc && len)
      prof = oyProfile_FromMem( len, icc, 0, 0 );

    icColorSpaceSignature csp = (icColorSpaceSignature) oyProfile_GetSignature(prof,oySIGNATURE_COLOR_SPACE);
    if(csp == icSigCmykData)
    {
      int n = spec.width * spec.height * 4;
      if(data_type == oyUINT8)
      {
        uint8_t * d = (uint8_t*)buf;
        int i;
#pragma omp parallel for private(i)
        for(i = 0; i < n; ++i)
          d[i] = 255 - d[i];
      }
    } else
    if(lIsITUFax)
    {
      int n = spec.width * spec.height;
      if(data_type == oyUINT8)
      {
        uint8_t * d = (uint8_t*)buf;
        int i;
#pragma omp parallel for private(i)
        for(i = 0; i < n; ++i)
          ycbcr2rgb( &d[i*spec.nchannels], &d[i*spec.nchannels]);
      }
    }
   

    //jpeg_finish_decompress (&cinfo);
    jpeg_destroy_decompress (&cinfo);

  } else if(strcmp(image->format_name(),"png") == 0)
  {
    png_structp png_ptr = 0;
    png_infop info_ptr = 0;
    png_ptr = png_create_read_struct( PNG_LIBPNG_VER_STRING,
                                    (png_voidp)filename,
                                    oPNGerror, oPNGwarn );
    info_ptr = png_create_info_struct(png_ptr);
    png_init_io( png_ptr, fp );
    png_read_info( png_ptr, info_ptr );
    png_read_update_info( png_ptr, info_ptr );
#if defined(PNG_iCCP_SUPPORTED)
    png_charp name = 0;
    png_bytep profile = 0;
    png_uint_32 proflen = 0;
    int compression = 0;

    if( png_get_iCCP( png_ptr, info_ptr, &name, &compression,
                      &profile, &proflen ) )
    {
      prof = oyProfile_FromMem( proflen, profile, 0,0 );
      if(oy_debug)
      oiio_msg( oyMSG_DBG, node,
             OY_DBG_FORMAT_ "ICC profile (size: %d): \"%s\"",
             OY_DBG_ARGS_, proflen, oyNoEmptyString_m( name ) );
    } else
      if(oy_debug)
        oiio_msg( oyMSG_DBG, node,
             OY_DBG_FORMAT_ "no ICC profile",
             OY_DBG_ARGS_);
#endif
    png_destroy_read_struct( &png_ptr, &info_ptr, (png_infopp)NULL );

  } else if(strcmp(image->format_name(),"tiff") == 0)
  {
    size_t   proflen = 0;
    char   * tiff_ptr = NULL;
    uint16_t photomet = PHOTOMETRIC_MINISBLACK;

    TIFF * tif = TIFFOpen(filename, "rm");
   
    if(!TIFFGetField( tif, TIFFTAG_PHOTOMETRIC, &photomet ))
      oiio_msg( oyMSG_DBG, node,
                OY_DBG_FORMAT_ "no TIFFTAG_PHOTOMETRIC",
                OY_DBG_ARGS_);

    if(TIFFGetField( tif, TIFFTAG_ICCPROFILE, &proflen, &tiff_ptr ))
    {
      prof = oyProfile_FromMem( proflen, tiff_ptr, 0,0 );
      if(oy_debug)
        oiio_msg( oyMSG_DBG, node,
             OY_DBG_FORMAT_ "ICC profile (size: %d)",
             OY_DBG_ARGS_, proflen);
    } else
    {
      oyProfile_s * p = NULL;
      if(oy_debug)
        oiio_msg( oyMSG_DBG, node,
             OY_DBG_FORMAT_ "no embedded ICC profile found",
             OY_DBG_ARGS_);

      /* build color spaces from tiff tags */
      switch(photomet)
      { case PHOTOMETRIC_LOGLUV:
          { /* creating an special profile with equal energy white point */
            double primaries_and_gamma[9] = {
            1.0,0.0, 0.0,1.0, 0.0,0.0, 0.5,0.5, 1.0};

            p = profileFromMatrix( primaries_and_gamma, "XYZ D*E" );

            oiio_msg( oyMSG_DBG, node,
                      OY_DBG_FORMAT_ "set XYZ Profil with D*E",
                      OY_DBG_ARGS_);
          }
          break;
        case PHOTOMETRIC_RGB:
        case PHOTOMETRIC_PALETTE:
        case PHOTOMETRIC_YCBCR:
          p = oyProfile_FromStd( oyASSUMED_RGB, icc_profile_flags, 0 );
          break;
        case PHOTOMETRIC_CIELAB:
        {
          int n = spec.width * spec.height;
          if(data_type == oyUINT8)
          {
            uint8_t * d = (uint8_t*)buf;
            for(int i = 0; i < n; ++i)
            {
              d[i*spec.nchannels+1] -= 128;
              d[i*spec.nchannels+2] -= 128;
            }
          } else
          if(data_type == oyUINT16)
          {
            uint16_t * d = (uint16_t*)buf;
            for(int i = 0; i < n; ++i)
            {
              d[i*spec.nchannels+1] -= 32768;
              d[i*spec.nchannels+2] -= 32768;
            }
          } else
          if(data_type == oyFLOAT)
          {
            float * f = (float*)buf;
            for(int i = 0; i < n; ++i)
            {
              f[i*spec.nchannels+0] /= 100.;
              f[i*spec.nchannels+1] /= 256.+.5;
              f[i*spec.nchannels+2] /= 256.+.5;
            }
          } else
          if(data_type == oyDOUBLE)
          {
            double * d = (double*)buf;
            for(int i = 0; i < n; ++i)
            {
              d[i*spec.nchannels+0] /= 100.;
              d[i*spec.nchannels+1] /= 256.+.5;
              d[i*spec.nchannels+2] /= 256.+.5;
            }
          }
        }
        case PHOTOMETRIC_ICCLAB: /**@todo TODO signiert / unsigniert */
          p = oyProfile_FromStd( oyASSUMED_LAB, icc_profile_flags, 0 );
          break;
        case PHOTOMETRIC_ITULAB:
          p = oyProfile_FromFile( "ITULab.icc", 0, 0 );
          break;
        case PHOTOMETRIC_SEPARATED:
          p = oyProfile_FromStd( oyASSUMED_CMYK, icc_profile_flags, 0 );
          break;
        case PHOTOMETRIC_MINISWHITE:
        case PHOTOMETRIC_MINISBLACK:
        case PHOTOMETRIC_LOGL:
          p = oyProfile_FromStd( oyASSUMED_GRAY, icc_profile_flags, 0 );
          break;
      }
      prof = p;
    }

    TIFFClose(tif);
  }

  /* fallback profile */
  if(!prof)
    prof = oyProfile_FromStd( profile_type, icc_profile_flags, 0 );

  if(oy_debug)
    oiio_msg( oyMSG_DBG, (oyStruct_s*)node,
             OY_DBG_FORMAT_ "%dx%d %s|%s[%d] %s\n%s",
             OY_DBG_ARGS_,  spec.width, spec.height,
             spec.format.c_str(), oyDataTypeToText(data_type), spec.nchannels,
             image->format_name(),
             spec.to_xml().c_str() );

  /* create a Oyranos image */
  image_in = oyImage_Create( spec.width, spec.height, buf, pixel_type, prof, 0 );

  if (!image_in)
  {
    oiio_msg( oyMSG_WARN, (oyStruct_s*)node,
             OY_DBG_FORMAT_ "can't create a new image\n%dx%d %s[%d]",
             OY_DBG_ARGS_,  spec.width, spec.height, spec.format.c_str(), spec.nchannels );
    return FALSE;
  }

  /* remember the meta data like file name */
  tags = oyImage_GetTags( image_in );
  error = oyOptions_SetFromText( &tags,
                                 "//" OY_TYPE_STD "/file_read.input_oiio"
                                                                    "/filename",
                                 filename, OY_CREATE_NEW );

  for (size_t i = 0; i < spec.extra_attribs.size(); ++i)
  {
    const OpenImageIO::ParamValue &p (spec.extra_attribs[i]);
    printf (" \%s: ", p.name().c_str());
    if (p.type() == OpenImageIO::TypeDesc::STRING)
      printf ("\"\%s\"", *(const char **)p.data());
    else if (p.type() == OpenImageIO::TypeDesc::FLOAT)
      printf ("\%g", *(const float *)p.data());
    else if (p.type() == OpenImageIO::TypeDesc::INT)
      printf ("\%d", *(const int *)p.data());
    else if (p.type() == OpenImageIO::TypeDesc::UINT)
      printf ("\%u", *(const unsigned int *)p.data());
    else
      printf ("<unknown data type>");
    printf("\n");
  }

  oyOptions_Release( &tags );

  /* close the image and FILE pointer */
  image->close();
  fclose(fp); fp = NULL;

  /* add image to filter socket */
  if(error <= 0)
  {
    oyFilterSocket_SetData( socket, (oyStruct_s*)image_in );
  }

  /* update the job ticket */
  if(ticket)
    output_image = oyPixelAccess_GetOutputImage( ticket );

  if(ticket &&
     output_image &&
     oyImage_GetWidth( output_image ) == 0 &&
     oyImage_GetHeight( output_image ) == 0)
  {
    oyImage_SetCritical( output_image, oyImage_GetPixelLayout( image_in,
                                                               oyLAYOUT ),
                         0,0,
                         oyImage_GetWidth( image_in ),
                         oyImage_GetHeight( image_in ) );
  }

  /* release Oyranos stuff */
  oyImage_Release( &image_in );
  oyImage_Release( &output_image );
  oyFilterNode_Release( &node );
  oyFilterSocket_Release( &socket );

  /* return an error to cause the graph to retry */
  return 1;
}


const char * oiioApi4UiGetText2      ( const char        * select,
                                       oyNAME_e            type,
                                       const char        * format )
{
  char * category = 0;

  if(strcmp(select,"name") == 0)
  {
    if(type == oyNAME_NICK)
      return "read";
    else if(type == oyNAME_NAME)
      return _("read");
    else if(type == oyNAME_DESCRIPTION)
      return _("Load Image File Object");
  } else if(strcmp(select,"help") == 0)
  {
    if(type == oyNAME_NICK)
      return "help";
    else if(type == oyNAME_NAME)
      return _("Option \"filename\", a valid filename of a existing image");
    else if(type == oyNAME_DESCRIPTION)
      return _("The Option \"filename\" should contain a valid filename to read the png data from. If the file does not exist, a error will occure.");
  }
  else if(strcmp(select,"category") == 0)
  {
    if(!category)
    {
      /* The following strings must match the categories for a menu entry. */
      const char * i18n[] = {_("Files"),_("Read"),0};
      int len =  strlen(i18n[0]) + strlen(i18n[1]) + strlen(format);

      category = (char*)malloc( len + 64 );
      if(category)
      {
        char * t;
        /* Create a translation for oiio_api4_ui_cmm_loader::category. */
        sprintf( category,"%s/%s %s", i18n[0], i18n[1], format );
        t = strstr(category, format);
        if(t) t[0] = toupper(t[0]);
      } else
        oiio_msg(oyMSG_WARN, (oyStruct_s *) 0, _DBG_FORMAT_ "\n " "Could not allocate enough memory.", _DBG_ARGS_);
    }

         if(type == oyNAME_NICK)
      return "category";
    else if(type == oyNAME_NAME)
      return category;
    else
      return category;
  }
  return 0;
}
/**
 *  This function implements oyCMMGetText_f.
 *
 */
const char * oiioApi4UiGetText       ( const char        * select,
                                       oyNAME_e            type,
                                       oyStruct_s        * context )
{
  oyCMMapiFilter_s * api = oyCMMui_GetParent( (oyCMMui_s *) context );
  oyPointer_s * backend_context = oyCMMapiFilter_GetBackendContext( api );
  const char * format = (const char*) oyPointer_GetPointer( backend_context );
  oyPointer_Release( &backend_context );
  api->release( (oyStruct_s**) &api );

  return oiioApi4UiGetText2(select, type, format);
}
const char * oiio_api4_ui_texts[] = {"name", "category", "help", NULL};

/* OY_OIIO_FILTER_REGISTRATION ----------------------------------------------*/

extern oyCMM_s oiio_cmm_module;
extern "C" {
int  oiioInit                        ( oyStruct_s        * module_info )
{
  oyCMMapi_s * a = 0,
             * a_tmp = 0,
             * m = 0;
  int i,n = 1, formats_n, attr_n;
  char * t = NULL, ** formats = NULL, ** attr = NULL;

  if((oyStruct_s*)&oiio_cmm_module != module_info)
    oiio_msg( oyMSG_WARN, module_info, _DBG_FORMAT_ "wrong module info passed in", _DBG_ARGS_ );

  /* search the last filter */
  a = oiio_cmm_module.api;
  while(a && ((a_tmp = oyCMMapi_GetNext( a )) != 0))
    a = a_tmp;

  OpenImageIO::getattribute( std::string("format_list"), &t );
  formats = oyStringSplit( t, ',', &formats_n, oyAllocateFunc_ );
  n = formats_n;
  if(oy_debug)
  oiio_msg( oyMSG_DBG, module_info, _DBG_FORMAT_ "format_list: %s", _DBG_ARGS_, t );
  OpenImageIO::getattribute( std::string("extension_list"), &t );
  attr = oyStringSplit( t, ';', &attr_n, oyAllocateFunc_ );
  if(oy_debug)
  oiio_msg( oyMSG_DBG, module_info, _DBG_FORMAT_ "extension_list: %s", _DBG_ARGS_, t );

  /* append new items */
  for( i = 0; i < n; ++i)
  {
    if(strcmp(formats[i],"pnm") == 0)
    {
      if(oy_debug)
        oiio_msg( oyMSG_DBG, module_info, _DBG_FORMAT_ "skipping: %s", _DBG_ARGS_, formats[i] );  
      continue;
    }

    m = oiioApi4CmmCreate( formats[i] );
    if(!a)
    {
      oiio_cmm_module.api = m;
      a = m;
    }
    else
      if(a && m)
      {
        oyCMMapi_SetNext( a, m ); a = m;
      }
  }
  for( i = 0; i < n; ++i)
  {
    if(strcmp(formats[i],"pnm") == 0) continue;

    m = oiioApi7CmmCreate( formats[i], strchr(attr[i],':') );
    if(!oiio_cmm_module.api)
    {
      oiio_cmm_module.api = m;
      a = m;
    }
    else
      if(a && m)
      {
        oyCMMapi_SetNext( a, m ); a = m;
      }
  }

  return 0;
}
}
