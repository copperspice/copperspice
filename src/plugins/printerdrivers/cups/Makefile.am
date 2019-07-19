PRINTERDRIVERS_DIRS += src/plugins/printerdrivers/cups

PRINTERDRIVERS_CUPS_PRIVATE_INCLUDES = \
	qppdprintdevice.h \
	qcupsprintersupport_p.h \
	qcupsprintengine_p.h

noinst_HEADERS+=\
	$(patsubst %,src/plugins/printerdrivers/cups/%,$(PRINTERDRIVERS_CUPS_PRIVATE_INCLUDES))

if Cups_FOUND

PRINTERDRIVERS_PRIVATE_INCLUDES += $(PRINTERDRIVERS_CUPS_PRIVATE_INCLUDES)

lib_LTLIBRARIES += lib/CsPrinterDriverCups@BUILD_ABI@.la

$(lib_CsPrinterDriverCups@BUILD_ABI@_la_SOURCES) : $(CORE_HEADERS_INSTALLED) $(GUI_HEADERS_INSTALLED) \
   $(PRINTERDRIVERS_HEADERS_INSTALLED)

lib_CsPrinterDriverCups@BUILD_ABI@_la_CXXFLAGS = -Iinclude -Iprivateinclude \
   -Iinclude/QtCore -Iprivateinclude/QtCore/private \
   -Iinclude/QtGui  -Iprivateinclude/QtGui/private -Iprivateinclude/QtGui/private/printerdrivers \
   $(AM_CXXFLAGS) \
   -DBUILD_DATE=\"`date +%Y-%m-%d`\" \
   -DQT_PLUGIN \
   -DCS_BUILDING_CUPS

lib_CsPrinterDriverCups@BUILD_ABI@_la_OBJCXXFLAGS = $(lib_CsPrinterDriverCups@BUILD_ABI@_la_CXXFLAGS)

lib_CsPrinterDriverCups@BUILD_ABI@_la_LDFLAGS = $(AM_LDFLAGS) \
   -avoid-version -no-undefined -module  \
   -lcups

lib_CsPrinterDriverCups@BUILD_ABI@_la_LIBADD=lib/libCsCore@BUILD_ABI@.la lib/libCsGui@BUILD_ABI@.la

lib_CsPrinterDriverCups@BUILD_ABI@_la_SOURCES = \
	src/plugins/printerdrivers/cups/main.cpp \
	src/plugins/printerdrivers/cups/qppdprintdevice.cpp \
	src/plugins/printerdrivers/cups/qcupsprintengine.cpp \
	src/plugins/printerdrivers/cups/qcupsprintersupport.cpp

endif


