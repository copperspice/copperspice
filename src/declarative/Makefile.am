lib_LTLIBRARIES += lib/libCsDeclarative1.la
lib_libCsDeclarative1_la_SOURCES =

lib_libCsDeclarative1_la_CXXFLAGS = $(AM_CXXFLAGS)  \
	-Iinclude -Iinclude/QtCore -Iinclude/QtGui  -Iinclude/QtNetwork \
	-Iinclude/QtScript  -Iinclude/QtSql -Iinclude/QtXmlPatterns 	-Iinclude/QtDeclarative \
	-Isrc/declarative/graphicsitems -Isrc/declarative/debugger -Isrc/declarative/qml -Isrc/declarative/util \
	-Iprivateinclude/QtDeclarative/private  \
	-Iprivateinclude  -Iprivateinclude/QtCore -Iprivateinclude/QtDeclarative -Iprivateinclude/QtGui  \
   -Iprivateinclude/QtScript -I$(top_srcdir)/src/3rdparty/harfbuzz/src  \
   -DQT_NO_OPENSSL   \
	-DQT_BUILD_DECLARATIVE_LIB \
	-DQT_SHARED   \
	-DQT_MAKEDLL

lib_libCsDeclarative1_la_LDFLAGS = $(AM_LDFLAGS) -no-undefined
lib_libCsDeclarative1_la_LIBADD = lib/libCsCore1.la  lib/libCsGui1.la  lib/libCsNetwork1.la  lib/libCsScript1.la \
					lib/libCsSql1.la  lib/libCsXmlPatterns1.la

if OSTYPE_LINUX
lib_libCsDeclarative1_la_CXXFLAGS += -I$(top_srcdir)/mkspecs/linux-g++
endif

if OSTYPE_WIN
lib_libCsDeclarative1_la_CXXFLAGS += -I$(top_srcdir)/mkspecs/win32-g++
endif

if OSTYPE_DARWIN
lib_libCsDeclarative1_la_CXXFLAGS += -I$(top_srcdir)/mkspecs/macx-g++
endif

DECLARATIVE_INCLUDES =
DECLARATIVE_PUBLIC_INCLUDES =
DECLARATIVE_PRIVATE_INCLUDES =
DECLARATIVE_DIRS =

include src/declarative/debugger/Makefile.am
include src/declarative/graphicsitems/Makefile.am
include src/declarative/qml/Makefile.am
include src/declarative/qml/parser/Makefile.am
include src/declarative/qml/rewriter/Makefile.am
include src/declarative/util/Makefile.am

DECLARATIVE_HEADERS_INSTALLED= \
	$(patsubst %,include/QtDeclarative/%,$(DECLARATIVE_INCLUDES)) \
	$(patsubst %,include/QtDeclarative/%,$(DECLARATIVE_PUBLIC_INCLUDES)) \
	$(patsubst %,privateinclude/QtDeclarative/private/%,$(DECLARATIVE_PRIVATE_INCLUDES))

BUILT_SOURCES+=declarative-header-deps.mk

$(lib_libCsDeclarative1_la_SOURCES) : $(CORE_HEADERS_INSTALLED) $(GUI_HEADERS_INSTALLED) $(NETWORK_HEADERS_INSTALLED) \
      $(SCRIPT_HEADERS_INSTALLED) $(SQL_HEADERS_INSTALLED)  \
      $(XML_HEADERS_INSTALLED) $(XMLPATTERNS_HEADERS_INSTALLED) $(DECLARATIVE_HEADERS_INSTALLED)

-include .declarative-header-deps.mk

.declarative-header-deps.mk: Makefile
	@- rm -rf $@
	for i in $(DECLARATIVE_PUBLIC_INCLUDES); do \
		echo "include/QtDeclarative/$$i: include/QtDeclarative/$$(echo $$i | tr A-Z a-z).h" >> $@; \
		echo "	\$$(INSTALL) -D \$$^ \$$@" >> $@ ; \
	done; \
	for i in $(DECLARATIVE_DIRS); do \
		echo "include/QtDeclarative/%: $$i/%" >> $@ ; \
		echo "	\$$(INSTALL) -D \$$^ \$$@" >> $@ ; \
	done;\
	for i in $(DECLARATIVE_DIRS); do \
		echo "privateinclude/QtDeclarative/private/%: $$i/%" >> $@ ; \
		echo "	\$$(INSTALL) -D \$$^ \$$@" >> $@ ; \
	done
