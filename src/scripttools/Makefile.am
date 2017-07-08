lib_LTLIBRARIES += lib/libCsScriptTools1.la
lib_libCsScriptTools1_la_SOURCES =

lib_libCsScriptTools1_la_CXXFLAGS = $(AM_CXXFLAGS) -Iinclude -Iinclude/QtCore -Iinclude/QtGui \
   -Iinclude/QtScriptTools -Iprivateinclude/QtCore -Iprivateinclude/QtGui   \
   -Iprivateinclude/QtScriptTools -Isrc/scripttools/debugging   \
	-DQT_BUILD_SCRIPTTOOLS_LIB

lib_libCsScriptTools1_la_LDFLAGS = $(AM_LDFLAGS) -no-undefined
lib_libCsScriptTools1_la_LIBADD = lib/libCsCore@BUILD_ABI@.la lib/libCsScript@BUILD_ABI@.la lib/libCsGui@BUILD_ABI@.la

SCRIPTTOOLS_INCLUDES =
SCRIPTTOOLS_PUBLIC_INCLUDES =
SCRIPTTOOLS_PRIVATE_INCLUDES =
SCRIPTTOOLS_DIRS =

EXTRA_DIST += \
	src/scripttools/CMakeLists.txt \
	src/scripttools/debugging/debugging.cmake

include src/scripttools/debugging/Makefile.am

SCRIPTTOOLS_HEADERS_INSTALLED= \
	$(patsubst %,include/QtScriptTools/%,$(SCRIPTTOOLS_INCLUDES)) \
	$(patsubst %,include/QtScriptTools/%,$(SCRIPTTOOLS_PUBLIC_INCLUDES)) \
	$(patsubst %,privateinclude/QtScriptTools/private/%,$(SCRIPTTOOLS_PRIVATE_INCLUDES))
	include/QtScriptTools privateinclude/QtScriptTools/private

BUILT_SOURCES+=.scripttools-header-deps.mk

$(lib_libCsScriptTools1_la_SOURCES) : $(CORE_HEADERS_INSTALLED) $(GUI_HEADERS_INSTALLED) \
      $(SCRIPT_HEADERS_INSTALLED)  $(SCRIPTTOOLS_HEADERS_INSTALLED)

-include .scripttools-header-deps.mk

.scripttools-header-deps.mk: Makefile
	@- rm -rf $@
	echo "include/QtScriptTools:" >> $@; \
	echo "	\$$(MKDIR_P) \$$@" >> $@ ; \
	echo "privateinclude/QtScriptTools/private:" >> $@; \
	echo "	\$$(MKDIR_P) \$$@" >> $@ ; \
	for i in $(SCRIPTTOOLS_PUBLIC_INCLUDES); do \
		echo "include/QtScriptTools/$$i: include/QtScriptTools/$$(echo $$i | tr A-Z a-z).h | include/QtScriptTools" >> $@; \
		echo "	\$$(INSTALL) \$$< \$$@" >> $@ ; \
	done; \
	for i in $(SCRIPTTOOLS_DIRS); do \
		echo "include/QtScriptTools/%: $$i/% | include/QtScriptTools" >> $@ ; \
		echo "	\$$(INSTALL) \$$< \$$@" >> $@ ; \
	done;\
	for i in $(SCRIPTTOOLS_DIRS); do \
		echo "privateinclude/QtScriptTools/private/%: $$i/% | privateinclude/QtScriptTools/private" >> $@ ; \
		echo "	\$$(INSTALL) \$$< \$$@" >> $@ ; \
	done
