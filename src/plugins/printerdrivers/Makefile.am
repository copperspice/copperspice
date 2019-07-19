EXTRA_DIST += \
	src/plugins/printerdrivers/CMakeLists.txt \
	src/plugins/printerdrivers/cocoa/cocoaprinter.cmake \
	src/plugins/printerdrivers/windows/windowsprinter.cmake \
	src/plugins/printerdrivers/cups/cupsprinter.cmake

PRINTERDRIVERS_PRIVATE_INCLUDES =
PRINTERDRIVERS_DIRS =

PRINTERDRIVERS_HEADERS_INSTALLED= \
	$(patsubst %,privateinclude/QtGui/private/printerdrivers/%,$(PRINTERDRIVERS_PRIVATE_INCLUDES)) \
	$(patsubst %,include/QtGui/%,$(PLATFORMS_PUBLIC_INCLUDES)) \
	privateinclude/QtGui/private/printerdrivers

include src/plugins/printerdrivers/cocoa/Makefile.am
include src/plugins/printerdrivers/windows/Makefile.am
include src/plugins/printerdrivers/cups/Makefile.am

-include .printerdrivers-header-deps.mk

.printerdrivers-header-deps.mk: Makefile
	@- rm -rf $@
	echo "privateinclude/QtGui/private/printerdrivers:" >> $@; \
	echo "	\$$(MKDIR_P) \$$@" >> $@ ; \
	for i in $(PRINTERDRIVERS_DIRS); do \
		echo "privateinclude/QtGui/private/printerdrivers/%: $$i/% | privateinclude/QtGui/private/printerdrivers" >> $@ ; \
		echo "	\$$(INSTALL) \$$< \$$@" >> $@ ; \
	done;
