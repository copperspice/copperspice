lib_LTLIBRARIES += lib/libCsNetwork@BUILD_ABI@.la
lib_libCsNetwork@BUILD_ABI@_la_SOURCES =

lib_libCsNetwork@BUILD_ABI@_la_CXXFLAGS = $(AM_CXXFLAGS) -Iinclude -Iinclude/QtCore -Iinclude/QtNetwork  \
   -Iprivateinclude/QtCore/private  -Iprivateinclude/QtNetwork/private    \
	-Isrc/network/access -Isrc/network/bearer   \
	-Isrc/network/socket     \
	-DBUILD_DATE=\"`date +%Y-%m-%d`\"  \
	-DQT_BUILD_NETWORK_LIB

lib_libCsNetwork@BUILD_ABI@_la_LDFLAGS = $(AM_LDFLAGS) -no-undefined
lib_libCsNetwork@BUILD_ABI@_la_LIBADD = lib/libCsCore@BUILD_ABI@.la

if SECURETRANSPORT_FOUND
   lib_libCsNetwork@BUILD_ABI@_la_CXXFLAGS += -DQT_SSL
   lib_libCsNetwork@BUILD_ABI@_la_CXXFLAGS += -DQT_SECURETRANSPORT

else

if OPENSSL_FOUND
   lib_libCsNetwork@BUILD_ABI@_la_CXXFLAGS += -DQT_SSL
   lib_libCsNetwork@BUILD_ABI@_la_CXXFLAGS += -DQT_OPENSSL

   lib_libCsNetwork@BUILD_ABI@_la_CXXFLAGS += $(SSL_CFLAGS)
   lib_libCsNetwork@BUILD_ABI@_la_LDFLAGS  += $(SSL_LIBS)
endif

endif

if OSTYPE_WIN
   lib_libCsNetwork@BUILD_ABI@_la_LDFLAGS += -lws2_32 -lcrypt32 -ldnsapi -liphlpapi
endif

if OSTYPE_DARWIN
   lib_libCsNetwork@BUILD_ABI@_la_CXXFLAGS += -D__APPLE_USE_RFC_3542
   lib_libCsNetwork@BUILD_ABI@_la_LDFLAGS  += -framework CoreServices -framework Security
endif

NETWORK_INCLUDES =
NETWORK_PUBLIC_INCLUDES =
NETWORK_PRIVATE_INCLUDES =
NETWORK_DIRS =

EXTRA_DIST += \
	src/network/CMakeLists.txt \
	src/network/access/access.cmake \
	src/network/bearer/bearer.cmake \
	src/network/kernel/kernel.cmake \
	src/network/socket/socket.cmake \
	src/network/ssl/ssl.cmake

include src/network/access/Makefile.am
include src/network/bearer/Makefile.am
include src/network/kernel/Makefile.am
include src/network/socket/Makefile.am
include src/network/ssl/Makefile.am

NETWORK_HEADERS_INSTALLED= \
	$(patsubst %,include/QtNetwork/%,$(NETWORK_INCLUDES)) \
	$(patsubst %,include/QtNetwork/%,$(NETWORK_PUBLIC_INCLUDES)) \
	$(patsubst %,privateinclude/QtNetwork/private/%,$(NETWORK_PRIVATE_INCLUDES)) \
	include/QtNetwork privateinclude/QtNetwork/private

BUILT_SOURCES+=.network-header-deps.mk

$(lib_libCsNetwork@BUILD_ABI@_la_SOURCES) : $(CORE_HEADERS_INSTALLED) $(NETWORK_HEADERS_INSTALLED)

-include .network-header-deps.mk

.network-header-deps.mk: Makefile
	@- rm -rf $@
	echo "include/QtNetwork:" >> $@; \
	echo "	\$$(MKDIR_P) \$$@" >> $@ ; \
	echo "privateinclude/QtNetwork/private:" >> $@; \
	echo "	\$$(MKDIR_P) \$$@" >> $@ ; \
	for i in $(NETWORK_PUBLIC_INCLUDES); do \
		echo "include/QtNetwork/$$i: include/QtNetwork/$$(echo $$i | tr A-Z a-z).h | include/QtNetwork" >> $@; \
		echo "	\$$(INSTALL) \$$< \$$@" >> $@ ; \
	done; \
	for i in $(NETWORK_DIRS); do \
		echo "include/QtNetwork/%: $$i/% | include/QtNetwork" >> $@ ; \
		echo "	\$$(INSTALL) \$$< \$$@" >> $@ ; \
	done;\
	for i in $(NETWORK_DIRS); do \
		echo "privateinclude/QtNetwork/private/%: $$i/% | privateinclude/QtNetwork/private" >> $@ ; \
		echo "	\$$(INSTALL) \$$< \$$@" >> $@ ; \
	done
