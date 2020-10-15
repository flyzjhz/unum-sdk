// (c) 2020 minim.co
// unum zlib utils include file

#ifndef _UTIL_ZLIB_H
#define _UTIL_ZLIB_H
#ifdef FEATURE_GZIP // Only if this is feature is enabled
#include <zlib.h>
// Threshold beyond which the message is to be compressed
#define COMPRESS_THRESHOLD 1024
#define COMPRESSED_MSG_MAX_SIZE 1500 // Typicall MTU?
#define COMPRESSION_ENABLED 0

// Default compression algorithm
// Can be overwritten by platform specific code
// For platforms with FUP limits, we can use Z_BEST_SPEED
#define DEFAULT_COMP_ALGO Z_DEFAULT_COMPRESSION
// Compress a message
int util_compress_message(char *msg, int len, char *cmsg, int cmsg_len);
// Decompress a message
int util_decompress_message(char *cmsg, int cmsg_len, char *msg, int len);
#endif // FEATURE_GZIP
#endif // _UTIL_ZLIB_H