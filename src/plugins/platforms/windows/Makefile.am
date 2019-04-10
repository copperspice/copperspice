PLATFORMS_DIRS += src/plugins/platforms/windows

PLATFORMS_WINDOWS_PRIVATE_INCLUDES = \
	array.h \
	qwindowsbackingstore.h \
	qwindowsgdiintegration.h \
	qwindowsgdinativeinterface.h \
	qwindowswindow.h \
	qwindowsintegration.h \
	qwindowscontext.h \
	qwindowsscreen.h \
	qwindowskeymapper.h \
	qwindowsfontengine.h \
	qwindowsfontdatabase.h \
	qwindowsmousehandler.h \
	qtwindowsglobal.h \
	qtwindows_additional.h \
	qwindowsole.h \
	qwindowsmime.h \
	qwindowsinternalmimedata.h \
	qwindowscursor.h \
	qwindowsinputcontext.h \
	qwindowstheme.h \
	qwindowsdialoghelpers.h \
	qwindowsservices.h \
	qwindowsnativeimage.h \
	qwindowsnativeinterface.h \
	qwindowsglcontext.h \
	qwindowsopenglcontext.h \
	qwindowsopengltester.h \
	qwindowsthreadpoolrunner.h \
	qwindowsclipboard.h \
	qwindowsdrag.h \
	qwindowsfontdatabase_ft.h \
	qwindowssessionmanager.h

PLATFORMS_WINDOWS_ACCESSIBLE_PRIVATE_INCLUDES = \
	accessible/comutils.h \
	accessible/qwindowsaccessibility.h \
	accessible/qwindowsmsaaaccessible.h

PLATFORMS_WINDOWS_EVENTS_PRIVATE_INCLUDES = \
	events/qwindowsguieventdispatcher_p.h

PLATFORMS_WINDOWS_DIRS = accessible events

noinst_HEADERS+=\
	$(patsubst %,src/plugins/platforms/windows/%,$(PLATFORMS_WINDOWS_PRIVATE_INCLUDES)) \
	$(patsubst %,src/plugins/platforms/windows/%,$(PLATFORMS_WINDOWS_ACCESSIBLE_PRIVATE_INCLUDES))  \
	$(patsubst %,src/plugins/platforms/windows/%,$(PLATFORMS_WINDOWS_EVENTS_PRIVATE_INCLUDES))

if BUILD_PLATFORMS_WINDOWS_PLUGIN

PLATFORMS_PRIVATE_INCLUDES += $(PLATFORMS_WINDOWS_PRIVATE_INCLUDES) \
   $(notdir $(PLATFORMS_WINDOWS_ACCESSIBLE_PRIVATE_INCLUDES)) \
   $(notdir $(PLATFORMS_WINDOWS_EVENTS_PRIVATE_INCLUDES))

lib_LTLIBRARIES += lib/CsGuiWin@BUILD_ABI@.la

$(lib_CsGuiWin@BUILD_ABI@_la_SOURCES) : $(CORE_HEADERS_INSTALLED) $(GUI_HEADERS_INSTALLED) $(PLATFORMS_HEADERS_INSTALLED)

lib_CsGuiWin@BUILD_ABI@_la_CXXFLAGS = -Iinclude -Iinclude/QtCore  -Iinclude/QtGui \
   -Iinclude/QtGui/platformheaders \
   -Iprivateinclude  -Iprivateinclude/QtCore/private  -Iprivateinclude/QtGui/private \
   -Iprivateinclude/QtGui/private/platforms \
   -I$(top_srcdir)/src/3rdparty/freetype/include \
   -I$(top_srcdir)/src/3rdparty/freetype/include/freetype \
   -I$(top_srcdir)/src/3rdparty/harfbuzz/src \
   $(AM_CXXFLAGS) \
   -DBUILD_DATE=\"`date +%Y-%m-%d`\" \
   -DQT_PLUGIN \
   -DQT_USE_FREETYPE

lib_CsGuiWin@BUILD_ABI@_la_LDFLAGS = $(AM_LDFLAGS) -avoid-version -no-undefined -module  \
   -lopengl32 -lgdi32 -lwinmm -lole32 -lwinspool -loleaut32 -lcomdlg32 -limm32 -Wl,-luuid -luser32

lib_CsGuiWin@BUILD_ABI@_la_LIBADD=lib/libCsCore@BUILD_ABI@.la lib/libCsGui@BUILD_ABI@.la

lib_CsGuiWin@BUILD_ABI@_la_SOURCES = \
	src/plugins/platforms/windows/main.cpp \
	src/plugins/platforms/windows/qwindowsbackingstore.cpp \
	src/plugins/platforms/windows/qwindowsgdiintegration.cpp \
	src/plugins/platforms/windows/qwindowsgdinativeinterface.cpp \
	src/plugins/platforms/windows/qwindowswindow.cpp \
	src/plugins/platforms/windows/qwindowsintegration.cpp \
	src/plugins/platforms/windows/qwindowscontext.cpp \
	src/plugins/platforms/windows/qwindowsscreen.cpp \
	src/plugins/platforms/windows/qwindowskeymapper.cpp \
	src/plugins/platforms/windows/qwindowsfontengine.cpp \
	src/plugins/platforms/windows/qwindowsfontdatabase.cpp \
	src/plugins/platforms/windows/qwindowsmousehandler.cpp \
	src/plugins/platforms/windows/qwindowsole.cpp \
	src/plugins/platforms/windows/qwindowsmime.cpp \
	src/plugins/platforms/windows/qwindowsinternalmimedata.cpp \
	src/plugins/platforms/windows/qwindowscursor.cpp \
	src/plugins/platforms/windows/qwindowsinputcontext.cpp \
	src/plugins/platforms/windows/qwindowstheme.cpp \
	src/plugins/platforms/windows/qwindowsdialoghelpers.cpp \
	src/plugins/platforms/windows/qwindowsservices.cpp \
	src/plugins/platforms/windows/qwindowsnativeimage.cpp \
	src/plugins/platforms/windows/qwindowsnativeinterface.cpp \
	src/plugins/platforms/windows/qwindowsglcontext.cpp \
	src/plugins/platforms/windows/qwindowsopengltester.cpp \
	src/plugins/platforms/windows/qwindowsclipboard.cpp \
	src/plugins/platforms/windows/qwindowsdrag.cpp \
	src/plugins/platforms/windows/qwindowsfontdatabase_ft.cpp \
	src/plugins/platforms/windows/qwindowssessionmanager.cpp \
	\
	src/plugins/platforms/windows/accessible/comutils.cpp \
	src/plugins/platforms/windows/accessible/qwindowsaccessibility.cpp \
	src/plugins/platforms/windows/accessible/qwindowsmsaaaccessible.cpp \
	src/plugins/platforms/windows/events/qwindowsguieventdispatcher.cpp

nodist_lib_CsGuiWin@BUILD_ABI@_la_SOURCES = \
	src/plugins/platforms/windows/qrc_cursors.cpp

endif

EXTRA_DIST += src/plugins/platforms/windows/cursors.qrc \
	src/plugins/platforms/windows/images/closedhandcursor_32.png \
	src/plugins/platforms/windows/images/closedhandcursor_48.png \
	src/plugins/platforms/windows/images/closedhandcursor_64.png \
	src/plugins/platforms/windows/images/dragcopycursor_32.png \
	src/plugins/platforms/windows/images/dragcopycursor_48.png \
	src/plugins/platforms/windows/images/dragcopycursor_64.png \
	src/plugins/platforms/windows/images/draglinkcursor_32.png \
	src/plugins/platforms/windows/images/draglinkcursor_48.png \
	src/plugins/platforms/windows/images/draglinkcursor_64.png \
	src/plugins/platforms/windows/images/dragmovecursor_32.png \
	src/plugins/platforms/windows/images/dragmovecursor_48.png \
	src/plugins/platforms/windows/images/dragmovecursor_64.png \
	src/plugins/platforms/windows/images/openhandcursor_32.png \
	src/plugins/platforms/windows/images/openhandcursor_48.png \
	src/plugins/platforms/windows/images/openhandcursor_64.png \
	src/plugins/platforms/windows/images/splithcursor_32.png \
	src/plugins/platforms/windows/images/splithcursor_48.png \
	src/plugins/platforms/windows/images/splithcursor_64.png \
	src/plugins/platforms/windows/images/splitvcursor_32.png \
	src/plugins/platforms/windows/images/splitvcursor_48.png \
	src/plugins/platforms/windows/images/splitvcursor_64.png

