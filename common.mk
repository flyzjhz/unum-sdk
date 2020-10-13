# (c) 2020 minim.co
TARGET_LIST := zlib  $(TARGET_LIST)
TARGET_INSTALL_LIST := $(TARGET_LIST) files
### zlib
ZLIB_VERSION := zlib-1.2.11
TARGET_VARS_zlib := VERSION=$(ZLIB_VERSION)
TARGET_VARS_unum += ZLIB=$(ZLIB_VERSION)
TARGET_CPPFLAGS_unum += -I$(TARGET_OBJ)/zlib/$(ZLIB_VERSION)/
TARGET_LDFLAGS_unum += -L$(TARGET_OBJ)/zlib/$(ZLIB_VERSION)/ -lz
zlib.install:
	mkdir -p "$(TARGET_RFS)/lib" "$(TARGET_RFS)/bin"
	$(STRIP) -o "$(TARGET_RFS)/lib/libz.so" "$(TARGET_OBJ)/zlib/$(ZLIB_VERSION)/libz.so"
