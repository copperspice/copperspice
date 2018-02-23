lib_LTLIBRARIES += lib/libCsXmlPatterns@BUILD_ABI@.la
lib_libCsXmlPatterns@BUILD_ABI@_la_SOURCES =

lib_libCsXmlPatterns@BUILD_ABI@_la_CXXFLAGS = $(AM_CXXFLAGS) -Iinclude -Iinclude/QtCore -Iinclude/QtXmlPatterns   \
   -Iprivateinclude/QtCore/private -Iprivateinclude/QtXmlPatterns/private \
   -Iinclude/QtNetwork   -I$(top_srcdir)/src/3rdparty/harfbuzz/src   \
   -DBUILD_DATE=\"`date +%Y-%m-%d`\"   \
   -DQT_BUILD_XMLPATTERNS_LIB

lib_libCsXmlPatterns@BUILD_ABI@_la_CFLAGS = $(AM_CFLAGS)
lib_libCsXmlPatterns@BUILD_ABI@_la_LDFLAGS = $(AM_LDFLAGS) -no-undefined
lib_libCsXmlPatterns@BUILD_ABI@_la_LIBADD = lib/libCsCore@BUILD_ABI@.la lib/libCsNetwork@BUILD_ABI@.la

XMLPATTERNS_INCLUDES =
XMLPATTERNS_PUBLIC_INCLUDES =
XMLPATTERNS_PRIVATE_INCLUDES =
XMLPATTERNS_DIRS =

EXTRA_DIST += \
	src/xmlpatterns/CMakeLists.txt \
	src/xmlpatterns/acceltree/acceltree.cmake \
	src/xmlpatterns/api/api.cmake \
	src/xmlpatterns/data/data.cmake \
	src/xmlpatterns/environment/environment.cmake \
	src/xmlpatterns/expr/expr.cmake \
	src/xmlpatterns/functions/functions.cmake \
	src/xmlpatterns/iterators/iterators.cmake \
	src/xmlpatterns/janitors/janitors.cmake \
	src/xmlpatterns/parser/parser.cmake \
	src/xmlpatterns/projection/projection.cmake \
	src/xmlpatterns/schema/schema.cmake \
	src/xmlpatterns/type/type.cmake \
	src/xmlpatterns/utils/utils.cmake

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
	$(patsubst %,privateinclude/QtXmlPatterns/private/%,$(XMLPATTERNS_PRIVATE_INCLUDES)) \
	include/QtXmlPatterns privateinclude/QtXmlPatterns/private

BUILT_SOURCES+=.xmlpatterns-header-deps.mk

$(lib_libCsXmlPatterns@BUILD_ABI@_la_SOURCES) : $(CORE_HEADERS_INSTALLED) $(NETWORK_HEADERS_INSTALLED) \
                 $(XML_HEADERS_INSTALLED) $(XMLPATTERNS_HEADERS_INSTALLED)

-include .xmlpatterns-header-deps.mk

.xmlpatterns-header-deps.mk: Makefile
	@- rm -rf $@
	echo "include/QtXmlPatterns:" >> $@; \
	echo "	\$$(MKDIR_P) \$$@" >> $@ ; \
	echo "privateinclude/QtXmlPatterns/private:" >> $@; \
	echo "	\$$(MKDIR_P) \$$@" >> $@ ; \
	for i in $(XMLPATTERNS_PUBLIC_INCLUDES); do \
		echo "include/QtXmlPatterns/$$i: include/QtXmlPatterns/$$(echo $$i | tr A-Z a-z).h | include/QtXmlPatterns" >> $@; \
		echo "	\$$(INSTALL) \$$< \$$@" >> $@ ; \
	done; \
	for i in $(XMLPATTERNS_DIRS); do \
		echo "include/QtXmlPatterns/%: $$i/% | include/QtXmlPatterns" >> $@ ; \
		echo "	\$$(INSTALL) \$$< \$$@" >> $@ ; \
	done;\
	for i in $(XMLPATTERNS_DIRS); do \
		echo "privateinclude/QtXmlPatterns/private/%: $$i/% | privateinclude/QtXmlPatterns/private" >> $@ ; \
		echo "	\$$(INSTALL) \$$< \$$@" >> $@ ; \
	done

