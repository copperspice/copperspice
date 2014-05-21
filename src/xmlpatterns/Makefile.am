lib_LTLIBRARIES += lib/libCsXmlPatterns1.la
lib_libCsXmlPatterns1_la_SOURCES =

lib_libCsXmlPatterns1_la_CXXFLAGS = $(AM_CXXFLAGS) -Iinclude -Iinclude/QtCore -Iinclude/QtXmlPatterns   \
   -Iprivateinclude/QtCore/private -Iprivateinclude/QtXmlPatterns/private \
   -Iinclude/QtNetwork   -I$(top_srcdir)/src/3rdparty/harfbuzz/src   \
   -DBUILD_DATE=\"`date +%Y-%m-%d`\"   \
   -DQT_BUILD_XMLPATTERNS_LIB          \
	-DQT_SHARED   \
	-DQT_MAKEDLL

lib_libCsXmlPatterns1_la_CFLAGS = $(AM_CFLAGS)
lib_libCsXmlPatterns1_la_LDFLAGS = $(AM_LDFLAGS) -no-undefined
lib_libCsXmlPatterns1_la_LIBADD = lib/libCsCore1.la lib/libCsNetwork1.la

# BROOM (enable OpenSSL)
lib_libCsXmlPatterns1_la_CXXFLAGS += -DQT_NO_OPENSSL

if OSTYPE_LINUX
lib_libCsXmlPatterns1_la_CXXFLAGS += -I$(top_srcdir)/mkspecs/linux-g++
endif

if OSTYPE_WIN
lib_libCsXmlPatterns1_la_CXXFLAGS += -I$(top_srcdir)/mkspecs/win32-g++
endif

if OSTYPE_DARWIN
lib_libCsXmlPatterns1_la_CXXFLAGS += -I$(top_srcdir)/mkspecs/macx-g++
endif

XMLPATTERNS_INCLUDES =
XMLPATTERNS_PUBLIC_INCLUDES =
XMLPATTERNS_PRIVATE_INCLUDES =
XMLPATTERNS_DIRS =

include src/xmlpatterns/acceltree/Makefile.am
include src/xmlpatterns/api/Makefile.am
include src/xmlpatterns/data/Makefile.am
include src/xmlpatterns/environment/Makefile.am
include src/xmlpatterns/expr/Makefile.am
include src/xmlpatterns/functions/Makefile.am
include src/xmlpatterns/iterators/Makefile.am
include src/xmlpatterns/janitors/Makefile.am
include src/xmlpatterns/parser/Makefile.am
include src/xmlpatterns/projection/Makefile.am
include src/xmlpatterns/schema/Makefile.am
include src/xmlpatterns/type/Makefile.am
include src/xmlpatterns/utils/Makefile.am

XMLPATTERNS_HEADERS_INSTALLED= \
	$(patsubst %,include/QtXmlPatterns/%,$(XMLPATTERNS_INCLUDES)) \
	$(patsubst %,include/QtXmlPatterns/%,$(XMLPATTERNS_PUBLIC_INCLUDES)) \
	$(patsubst %,privateinclude/QtXmlPatterns/private/%,$(XMLPATTERNS_PRIVATE_INCLUDES))

BUILT_SOURCES+=.xmlpatterns-header-deps.mk

$(lib_libCsXmlPatterns1_la_SOURCES) : $(CORE_HEADERS_INSTALLED) $(NETWORK_HEADERS_INSTALLED) \
                 $(XML_HEADERS_INSTALLED) $(XMLPATTERNS_HEADERS_INSTALLED)

-include .xmlpatterns-header-deps.mk

.xmlpatterns-header-deps.mk: Makefile
	@- rm -rf $@
	for i in $(XMLPATTERNS_PUBLIC_INCLUDES); do \
		echo "include/QtXmlPatterns/$$i: include/QtXmlPatterns/$$(echo $$i | tr A-Z a-z).h" >> $@; \
		echo "	\$$(INSTALL) -D \$$^ \$$@" >> $@ ; \
	done; \
	for i in $(XMLPATTERNS_DIRS); do \
		echo "include/QtXmlPatterns/%: $$i/%" >> $@ ; \
		echo "	\$$(INSTALL) -D \$$^ \$$@" >> $@ ; \
	done;\
	for i in $(XMLPATTERNS_DIRS); do \
		echo "privateinclude/QtXmlPatterns/private/%: $$i/%" >> $@ ; \
		echo "	\$$(INSTALL) -D \$$^ \$$@" >> $@ ; \
	done

