lib_LTLIBRARIES += lib/libCsNetwork1.la
lib_libCsNetwork1_la_SOURCES =

lib_libCsNetwork1_la_CXXFLAGS = $(AM_CXXFLAGS) -Iinclude -Iinclude/QtCore -Iinclude/QtNetwork  \
   -Iprivateinclude/QtCore/private  -Iprivateinclude/QtNetwork/private    \
	-Isrc/network/access -Isrc/network/bearer   \
	-Isrc/network/socket     \
	-DBUILD_DATE=\"`date +%Y-%m-%d`\"  \
	-DQT_BUILD_NETWORK_LIB   \
	-DQT_SHARED  \
	-DQT_MAKEDLL

lib_libCsNetwork1_la_LDFLAGS = $(AM_LDFLAGS) -no-undefined
lib_libCsNetwork1_la_LIBADD = lib/libCsCore1.la

if !SSL
lib_libCsNetwork1_la_CXXFLAGS += -DQT_NO_OPENSSL
endif

if OSTYPE_WIN
lib_libCsNetwork1_la_LDFLAGS += -lws2_32
endif

if OSTYPE_DARWIN
lib_libCsNetwork1_la_LDFLAGS += -framework CoreServices
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

$(lib_libCsNetwork1_la_SOURCES) : $(CORE_HEADERS_INSTALLED) $(NETWORK_HEADERS_INSTALLED) 

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
