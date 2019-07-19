PRINTERDRIVERS_DIRS += src/plugins/printerdrivers/cocoa

PRINTERDRIVERS_COCOA_PRIVATE_INCLUDES =

noinst_HEADERS+=\
	$(patsubst %,src/plugins/printerdrivers/cocoa/%,$(PRINTERDRIVERS_COCOA_PRIVATE_INCLUDES))


if OSTYPE_DARWIN

PRINTERDRIVERS_PRIVATE_INCLUDES += $(PRINTERDRIVERS_COCOA_PRIVATE_INCLUDES)

lib_LTLIBRARIES += lib/CsPrinterDriverCocoa@BUILD_ABI@.la

$(lib_CsPrinterDriverCocoa@BUILD_ABI@_la_SOURCES) : $(CORE_HEADERS_INSTALLED) $(GUI_HEADERS_INSTALLED) \
   $(PRINTERDRIVERS_HEADERS_INSTALLED)

lib_CsPrinterDriverCocoa@BUILD_ABI@_la_CXXFLAGS = -Iinclude \
   -Iinclude/QtCore -Iprivateinclude/QtCore/private \
   -Iinclude/QtGui  -Iprivateinclude/QtGui/private -Iprivateinclude/QtGui/private/printerdrivers \
   $(AM_CXXFLAGS) \
   -DBUILD_DATE=\"`date +%Y-%m-%d`\" \
   -DQT_PLUGIN

lib_CsPrinterDriverCocoa@BUILD_ABI@_la_OBJCXXFLAGS = $(lib_CsGuiCocoa@BUILD_ABI@_la_CXXFLAGS)

lib_CsPrinterDriverCocoa@BUILD_ABI@_la_LDFLAGS = $(AM_LDFLAGS) \
   -avoid-version -no-undefined -module \
   -framework Cocoa

lib_CsPrinterDriverCocoa@BUILD_ABI@_la_LIBADD=lib/libCsCore@BUILD_ABI@.la lib/libCsGui@BUILD_ABI@.la

lib_CsPrinterDriverCocoa@BUILD_ABI@_la_SOURCES = \
	src/plugins/printerdrivers/cocoa/main.cop

endif
