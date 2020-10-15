# (c) 2020 minim.co
# Plaform independent part of Makefile for unum zip functions library 

CPPFLAGS += -I$(UNUM_PATH)/zip/
# Add code file(s)
# Minizip files
OBJECTS += ./zip/ioapi.o ./zip/zip.o
# Compress and Decompress functions
OBJECTS += ./zip/zip_zlib.o
