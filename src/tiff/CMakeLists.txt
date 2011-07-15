include (CheckFunctionExists)
include (CheckIncludeFile)
include (TestBigEndian)
include (CheckTypeSize)

check_type_size (int SIZEOF_INT BUILTIN_TYPES_ONLY)
check_type_size (long SIZEOF_LONG BUILTIN_TYPES_ONLY)
check_type_size (int8 INT8)
check_type_size (int16 INT16)
check_type_size (int32 INT32)

# Check for system headers
check_include_file (assert.h HAVE_ASSERT_H)
check_include_file (fcntl.h HAVE_FCNTL_H)
check_include_file (limits.h HAVE_LIMITS_H)
check_include_file (malloc.h HAVE_MALLOC_H)
check_include_file (search.h HAVE_SEARCH_H)
check_include_file (sys/time.h HAVE_SYS_TIME_H)
check_include_file (unistd.h HAVE_UNISTD_H)
check_include_file (string.h HAVE_STRING_H)
check_include_file (windows.h HAVE_WINDOWS_H)

# Check for standard library functions
check_function_exists (floor HAVE_FLOOR)
check_function_exists (isascii HAVE_ISASCII)
check_function_exists (memmove HAVE_MEMMOVE)
check_function_exists (memset HAVE_MEMSET)
check_function_exists (mmap HAVE_MMAP)
check_function_exists (pow HAVE_POW)
check_function_exists (sqrt HAVE_SQRT)
check_function_exists (strchr HAVE_STRCHR)
check_function_exists (strrchr HAVE_STRRCHR)
check_function_exists (strstr HAVE_STRSTR)
check_function_exists (strtol HAVE_STRTOL)

test_big_endian (HOST_BIGENDIAN)
if (HOST_BIGENDIAN)
	set (HOST_FILLORDER FILLORDER_MSB2LSB)
else ()
	set (HOST_FILLORDER FILLORDER_LSB2MSB)
endif ()

set (HAVE_IEEEFP 1)

set (CCITT_SUPPORT 1)
set (PACKBITS_SUPPORT 1)
set (LZW_SUPPORT 1)
set (THUNDER_SUPPORT 1)
set (NEXT_SUPPORT 1)
set (LOGLUV_SUPPORT 1)
set (STRIP_CHOPPING_SUPPORT 1)
set (EXTRASAMPLE_AS_ALPHA_SUPPORT 1)
set (CHECK_YCBCR_SUBSAMPLING_SUPPORT 1)

configure_file (tif_config.h.in tif_config.h)
configure_file (tiffconf.h.in tiffconf.h)

include_directories (${CMAKE_CURRENT_BINARY_DIR})

set (sources
	tif_aux.c
	tif_close.c
	tif_codec.c
	tif_color.c
	tif_compress.c
	tif_dir.c
	tif_dirinfo.c
	tif_dirread.c
	tif_dirwrite.c
	tif_dumpmode.c
	tif_error.c
	tif_extension.c
	tif_fax3.c
	tif_fax3sm.c
	tif_flush.c
	tif_getimage.c
	tif_jpeg.c
	tif_luv.c
	tif_lzw.c
	tif_next.c
	tif_ojpeg.c
	tif_open.c
	tif_packbits.c
	tif_pixarlog.c
	tif_predict.c
	tif_print.c
	tif_read.c
	tif_strip.c
	tif_swab.c
	tif_thunder.c
	tif_tile.c
	tif_version.c
	tif_warning.c
	tif_write.c
	tif_zip.c
)

if (MSVC)
	set (inline __inline)
else ()
	set (inline inline)
endif ()

if (WIN32)
	set (sources ${sources} tif_win32.c)
elseif (APPLE)
	set (sources ${sources} tif_apple.c)
	include_directories (/Developer/Headers/FlatCarbon)
else ()
	set (sources ${sources} tif_unix.c)
endif ()

add_library(bundled_tiff ${sources})
