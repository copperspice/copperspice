lib_LTLIBRARIES += lib/libCsDeclarative1.la
lib_libCsDeclarative1_la_SOURCES =

lib_libCsDeclarative1_la_CXXFLAGS = $(AM_CXXFLAGS)  \
	-Iinclude -Iinclude/QtCore -Iinclude/QtGui  -Iinclude/QtNetwork \
	-Iinclude/QtScript  -Iinclude/QtSql -Iinclude/QtXmlPatterns 	-Iinclude/QtDeclarative \
	-Isrc/declarative/graphicsitems -Isrc/declarative/debugger -Isrc/declarative/qml -Isrc/declarative/util \
	-Iprivateinclude/QtDeclarative/private  \
	-Iprivateinclude  -Iprivateinclude/QtCore -Iprivateinclude/QtDeclarative -Iprivateinclude/QtGui  \
   -Iprivateinclude/QtScript -I$(top_srcdir)/src/3rdparty/harfbuzz/src  \
	-DQT_BUILD_DECLARATIVE_LIB

lib_libCsDeclarative1_la_LDFLAGS = $(AM_LDFLAGS) -no-undefined
lib_libCsDeclarative1_la_LIBADD = lib/libCsCore@BUILD_ABI@.la  lib/libCsGui@BUILD_ABI@.la  lib/libCsNetwork@BUILD_ABI@.la  lib/libCsScript@BUILD_ABI@.la \
					lib/libCsSql@BUILD_ABI@.la  lib/libCsXmlPatterns@BUILD_ABI@.la

DECLARATIVE_INCLUDES =
DECLARATIVE_PUBLIC_INCLUDES =
DECLARATIVE_PRIVATE_INCLUDES =
DECLARATIVE_DIRS =

EXTRA_DIST += \
	src/declarative/CMakeLists.txt \
	src/declarative/debugger/debugger.cmake \
	src/declarative/graphicsitems/graphicsitems.cmake \
	src/declarative/qml/parser/parser.cmake \
	src/declarative/qml/qml.cmake \
	src/declarative/qml/rewriter/rewriter.cmake \
	src/declarative/util/util.cmake

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
	echo "include/QtDeclarative:" >> $@; \
	echo "	\$$(MKDIR_P) \$$@" >> $@ ; \
	echo "privateinclude/QtDeclarative/private:" >> $@; \
	echo "	\$$(MKDIR_P) \$$@" >> $@ ; \
	for i in $(DECLARATIVE_PUBLIC_INCLUDES); do \
		echo "include/QtDeclarative/$$i: include/QtDeclarative/$$(echo $$i | tr A-Z a-z).h include/QtDeclarative" >> $@; \
		echo "	\$$(INSTALL) \$$< \$$@" >> $@ ; \
	done; \
	for i in $(DECLARATIVE_DIRS); do \
		echo "include/QtDeclarative/%: $$i/% include/QtDeclarative" >> $@ ; \
		echo "	\$$(INSTALL) \$$< \$$@" >> $@ ; \
	done;\
	for i in $(DECLARATIVE_DIRS); do \
		echo "privateinclude/QtDeclarative/private/%: $$i/% privateinclude/QtDeclarative/private" >> $@ ; \
		echo "	\$$(INSTALL) \$$< \$$@" >> $@ ; \
	done
