EXTRA_DIST += \
	src/plugins/platforms/CMakeLists.txt \
	src/plugins/platforms/cocoa/cocoa.cmake \
	src/plugins/platforms/windows/windows.cmake \
	src/plugins/platforms/xcb/xcb.cmake

PLATFORMS_INCLUDES =
PLATFORMS_PUBLIC_INCLUDES =
PLATFORMS_PRIVATE_INCLUDES =
PLATFORMS_DIRS =

PLATFORMS_HEADERS_INSTALLED= \
	$(patsubst %,include/QtGui/%,$(PLATFORMS_INCLUDES)) \
	$(patsubst %,include/QtGui/%,$(PLATFORMS_PUBLIC_INCLUDES)) \
	$(patsubst %,privateinclude/QtGui/private/platforms/%,$(PLATFORMS_PRIVATE_INCLUDES)) \
	include/QtGui  include/QtGui/platformheaders   privateinclude/QtGui/private/platforms

include src/plugins/platforms/cocoa/Makefile.am
include src/plugins/platforms/windows/Makefile.am
include src/plugins/platforms/xcb/Makefile.am

-include .platforms-header-deps.mk

.platforms-header-deps.mk: Makefile
	@- rm -rf $@

	echo "include/QtGui/platformheaders:" >> $@; \
	echo "	\$$(MKDIR_P) \$$@" >> $@ ; \
	echo "privateinclude/QtGui/private/platforms: privateinclude/QtGui/private" >> $@; \
	echo "	\$$(MKDIR_P) \$$@" >> $@ ; \
	for i in $(PLATFORMS_DIRS); do \
		echo "privateinclude/QtGui/private/platforms/%: $$i/% | privateinclude/QtGui/private/platforms" >> $@ ; \
		echo "	\$$(INSTALL) \$$< \$$@" >> $@ ; \
	done; \
	for i in $(PLATFORMS_COCOA_DIRS); do \
		echo "privateinclude/QtGui/private/platforms/%: src/plugins/platforms/cocoa/$$i/% | privateinclude/QtGui/private/platforms" >> $@ ; \
		echo "	\$$(INSTALL) \$$< \$$@" >> $@ ; \
	done; \
	for i in $(PLATFORMS_WINDOWS_DIRS); do \
		echo "privateinclude/QtGui/private/platforms/%: src/plugins/platforms/windows/$$i/% | privateinclude/QtGui/private/platforms" >> $@ ; \
		echo "	\$$(INSTALL) \$$< \$$@" >> $@ ; \
	done; \
	for i in $(PLATFORMS_XCB_DIRS); do \
		echo "privateinclude/QtGui/private/platforms/%: src/plugins/platforms/xcb/$$i/% | privateinclude/QtGui/private/platforms" >> $@ ; \
		echo "	\$$(INSTALL) \$$< \$$@" >> $@ ; \
	done;
