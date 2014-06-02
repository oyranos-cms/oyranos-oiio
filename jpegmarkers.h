/*
 * iccprofile.h
 *
 * This file provides code to read and write International Color Consortium
 * (ICC) device profiles embedded in JFIF JPEG image files.  The ICC has
 * defined a standard format for including such data in JPEG "APP2" markers.
 * The code given here does not know anything about the internal structure
 * of the ICC profile data; it just knows how to put the profile data into
 * a JPEG file being written, or get it back out when reading.
 *
 * This code depends on new features added to the IJG JPEG library as of
 * IJG release 6b; it will not compile or work with older IJG versions.
 *
 * NOTE: this code would need surgery to work on 16-bit-int machines
 * with ICC profiles exceeding 64K bytes in size.  See iccprofile.c
 * for details.
 */

#ifdef __cplusplus
#ifndef DONT_USE_EXTERN_C
extern "C" {
#endif
#endif


#include <stdio.h>		/* needed to define "FILE", "NULL" */
#include "jpeglib.h"



/*
 * Reading a JPEG file that may contain marker data requires two steps:
 *
 * 1. After jpeg_create_decompress() but before jpeg_read_header(),
 *    save the APP2 markers in memory.

      for (int m = 0; m < 16; m++)
        jpeg_save_markers(&cinfo, JPEG_APP0 + m, 0xFFFF);

 *
 * 2. After jpeg_read_header(), call jpeg_get_marker_size() to find out
 *    whether there was a profile and obtain it if so.
 */


/* recheck after read_icc_profile */
int read_icc_profile2(j_decompress_ptr cinfo,
                      const char * filename,
                      JOCTET **icc_data_ptr,
                      unsigned int *icc_data_len);


/*
 * This routine writes the given data into a JPEG file.
 * It *must* be called AFTER calling jpeg_start_compress() and BEFORE
 * the first call to jpeg_write_scanlines().
 * (This ordering ensures that the APP0 marker(s) will appear after the
 * SOI and JFIF or Adobe markers, but before all else.)
 */
EXTERN(void) jpeg_write_marker_APP JPP((j_compress_ptr cinfo,
                   unsigned int marker_code,
                   const JOCTET *marker_name,
                   unsigned int marker_name_length,
		   const JOCTET *data_ptr,
		   unsigned int data_len));
/*
 * This routine writes the given data into a JPEG file.
 * It *must* be called AFTER calling jpeg_start_compress() and BEFORE
 * the first call to jpeg_write_scanlines().
 * (This ordering ensures that the APP2 marker(s) will appear after the
 * SOI and JFIF or Adobe markers, but before all else.)
 * Data size can exceed the JPEG 65533-marker_name_length limit like 
 * with ICC profiles.
 */
EXTERN(void) jpeg_write_marker_APP2 JPP((j_compress_ptr cinfo,
                   const JOCTET *marker_name,
                   unsigned int marker_name_length,
		   const JOCTET *data_ptr,
		   unsigned int data_len));
/*
 * Tell how many APP0 markers are present.
 * Return error.
 */
EXTERN(int) jpeg_count_markers JPP((j_decompress_ptr cinfo,
                              int *markers_count));
/*
 * Obtain single marker.
 * Return error.
 */
EXTERN(int) jpeg_get_marker JPP((j_decompress_ptr cinfo,
                              int pos,
                              jpeg_saved_marker_ptr *marker_return));
/*
 * Obtain single marker name, name size estimation and APP0 code.
 * Return error.
 */
EXTERN(int) jpeg_get_marker_name JPP((j_decompress_ptr cinfo,
                          int pos,
                          unsigned int *marker_code,
                          JOCTET **marker_name,
                          int *marker_name_length));
/*
 * Obtain single marker size in order to allocate memory.
 * This function can be used to check for the existence of a marker.
 * Return error.
 */
EXTERN(int) jpeg_get_marker_size JPP((j_decompress_ptr cinfo,
                          unsigned int marker_code,
                          JOCTET *marker_name,
                          int marker_name_length,
                          unsigned int *data_len));
/*
 * Obtain single marker data in user allocated memory.
 * Return error.
 */
EXTERN(int) jpeg_get_marker_data JPP((j_decompress_ptr cinfo,
                          unsigned int marker_code,
                          JOCTET *marker_name,
                          int marker_name_length,
                          unsigned int data_len,
                          JOCTET *data_ptr));

void ycbcr2rgb (uint8_t * rgb, uint8_t * ycbcr);

#ifdef __cplusplus
#ifndef DONT_USE_EXTERN_C
}
#endif
#endif

