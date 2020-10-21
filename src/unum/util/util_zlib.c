// (c) 2020 minim.co

#ifdef FEATURE_GZIP_REQUESTS // Only if this is feature is enabled
// Wrapper routines from zlib
// Zlib is thread safe
// Can safely be called in parallel
// So no locks are needed

#include "unum.h"

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
    int GZIP_ENCODING = 16;
    int WINDOW_BITS = 15;

    stream.zalloc = Z_NULL;
    stream.zfree = Z_NULL;
    stream.opaque = Z_NULL;

    stream.avail_in = len + 1;
    stream.next_in = (Bytef *) msg;
    stream.avail_out = cmsg_len;
    stream.next_out = (Bytef *)cmsg;
    
    // Init the compression system
    if ((ret = deflateInit2(&stream, DEFAULT_COMP_ALGO,  Z_DEFLATED,
                             WINDOW_BITS | GZIP_ENCODING, 8,
                             Z_DEFAULT_STRATEGY)) != Z_OK) {
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

    return cmsg_len - stream.avail_out;
}

#endif // FEATURE_GZIP_REQUESTS
