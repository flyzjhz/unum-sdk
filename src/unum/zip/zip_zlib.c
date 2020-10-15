// (c) 2020 minim.co

// Wrapper routines from zlib
#include "unum.h"
#ifdef FEATURE_GZIP // Only if this is feature is enabled
// Zlib is thread safe
// Can safely be called in parallel
// So no locks are needed

// Compress a message using zlib library
// msg - Message to be compressed
// len - Length of the message
// cmsg - Compressed message
// cmsg_len - Size of cmsg
// Returns the length of the compressed message
int util_compress_message(char *msg, int len, char *cmsg, int cmsg_len)
{
    z_stream stream;
    int ret;

    stream.zalloc = Z_NULL;
    stream.zfree = Z_NULL;
    stream.opaque = Z_NULL;

    stream.avail_in = len + 1;
    stream.next_in = (Bytef *) msg;
    stream.avail_out = cmsg_len;
    stream.next_out = (Bytef *)cmsg;
    
    // Init the compression system
    if ((ret = deflateInit(&stream, DEFAULT_COMP_ALGO)) != Z_OK) {
        // Init failed
        // Return
        log("%s: Initing the compression system failed: Error code: %d\n",
                             __func__, ret);
        return -1;
    }

    // Attempt to compress the data
    ret = deflate(&stream, Z_FINISH);
    if (ret != Z_OK && ret != Z_STREAM_END) {
        log("%s: Attempt to compress the data failed: Error code: %d\n",
                             __func__, ret);
        deflateEnd(&stream);
        return -1;
    }
    ret = deflateEnd(&stream);
    if (ret != Z_OK && ret != Z_DATA_ERROR) {
        log("%s: Error while freeing deflate streams: %d\n",
                             __func__, ret);
        return -1;
    }

    return (int) ((char*)stream.next_out - cmsg);
}

// Decompress a message using zlib library
// cmsg - Compressed message (to be decompressed)
// cmsg_len - Length of the compressed message
// msg - Decompressed message
// len - Size of msg array
// Returns the length of the decompressed message
int util_decompress_message(char *cmsg, int cmsg_len, char *msg, int len)
{
    z_stream stream;
    int ret;

    stream.zalloc = Z_NULL;
    stream.zfree = Z_NULL;
    stream.opaque = Z_NULL;

    stream.avail_in = cmsg_len;
    stream.next_in = (Bytef *)cmsg;
    stream.avail_out = len;
    stream.next_out = (Bytef *)msg;
     
    // the actual DE-compression work.
    if ((ret = inflateInit(&stream)) != Z_OK) {
        // Init failed
        // Return
        log("%s: Initing the compression system failed: Error code: %d\n",
                             __func__, ret);
        return -1;
    }
    ret = inflate(&stream, Z_NO_FLUSH);
    if (ret != Z_OK && ret != Z_STREAM_END) {
        log("%s: Attempt to decompress the data failed: Error code: %d\n",
                             __func__, ret);
        // Clear all the resources
        inflateEnd(&stream);
        return -1;
    }
    ret = inflateEnd(&stream);
    if (ret != Z_OK && ret != Z_DATA_ERROR) {
        log("%s: Error while freeing deflate streams: %d\n",
                             __func__, ret);
        return -1;
    }

    return strlen(msg);
}
#endif // FEATURE_GZIP
