PRINTERDRIVERS_DIRS += src/plugins/printerdrivers/windows

PRINTERDRIVERS_WINDOWS_PRIVATE_INCLUDES = \
	qwindowsprintersupport.h \
	qwindowsprintdevice.h

noinst_HEADERS+=\
	$(patsubst %,src/plugins/printerdrivers/windows/%,$(PRINTERDRIVERS_WINDOWS_PRIVATE_INCLUDES))

if OSTYPE_WIN

PRINTERDRIVERS_PRIVATE_INCLUDES += $(PRINTERDRIVERS_WINDOWS_PRIVATE_INCLUDES)

lib_LTLIBRARIES += lib/CsPrinterDriverWin@BUILD_ABI@.la

$(lib_CsPrinterDriverWin@BUILD_ABI@_la_SOURCES) : $(CORE_HEADERS_INSTALLED) $(GUI_HEADERS_INSTALLED) \
   $(PRINTERDRIVERS_HEADERS_INSTALLED)

lib_CsPrinterDriverWin@BUILD_ABI@_la_CXXFLAGS = -Iinclude -Iprivateinclude \
   -Iinclude/QtCore -Iprivateinclude/QtCore/private \
   -Iinclude/QtGui  -Iprivateinclude/QtGui/private -Iprivateinclude/QtGui/private/printerdrivers \
   $(AM_CXXFLAGS) \
   -DBUILD_DATE=\"`date +%Y-%m-%d`\" \
   -DQT_PLUGIN

lib_CsPrinterDriverWin@BUILD_ABI@_la_LDFLAGS = $(AM_LDFLAGS) -avoid-version -no-undefined -module  \
   -lwinspool -lcomdlg32 -lgdi32 -luser32

lib_CsPrinterDriverWin@BUILD_ABI@_la_LIBADD=lib/libCsCore@BUILD_ABI@.la lib/libCsGui@BUILD_ABI@.la

lib_CsPrinterDriverWin@BUILD_ABI@_la_SOURCES = \
	src/plugins/printerdrivers/windows/main.cpp \
	src/plugins/printerdrivers/windows/qwindowsprintersupport.cpp \
	src/plugins/printerdrivers/windows/qwindowsprintdevice.cpp

endif


