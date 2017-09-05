if PHONON
lib_LTLIBRARIES += lib/libCsPhonon@BUILD_ABI@.la
endif

lib_libCsPhonon@BUILD_ABI@_la_SOURCES =

lib_libCsPhonon@BUILD_ABI@_la_CXXFLAGS = $(AM_CXXFLAGS) -Iinclude  -Iinclude/phonon -Iprivateinclude/phonon -Iinclude/QtCore \
   -Iinclude/QtGui  -Iinclude/QtNetwork \
   -Iprivateinclude/QtCore -Iprivateinclude -Iprivateinclude/phonon/private  \
   -Isrc/3rdparty/phonon/phonon       \
   -DBUILD_DATE=\"`date +%Y-%m-%d`\"   \
   -DMAKE_PHONON_LIB  \
   -DQT_NO_DBUS  \
	-DQT_BUILD_PHONON_LIB

lib_libCsPhonon@BUILD_ABI@_la_LDFLAGS = $(AM_LDFLAGS) -no-undefined
lib_libCsPhonon@BUILD_ABI@_la_LIBADD = lib/libCsCore@BUILD_ABI@.la lib/libCsGui@BUILD_ABI@.la lib/libCsNetwork@BUILD_ABI@.la

EXTRA_DIST += \
	src/phonon/CMakeLists.txt

if GLIB
if PULSEAUDIO_MAINLOOP_GLIB
lib_libCsPhonon@BUILD_ABI@_la_CXXFLAGS += -DHAVE_PULSEAUDIO $(PULSEAUDIO_MAINLOOP_GLIB_CFLAGS) $(PULSEAUDIO_CFLAGS)
lib_libCsPhonon@BUILD_ABI@_la_LDFLAGS += $(PULSEAUDIO_LIBS) $(PULSEAUDIO_MAINLOOP_GLIB_LIBS)
if PULSEAUDIO_DEVICE_MANAGER
lib_libCsPhonon@BUILD_ABI@_la_CXXFLAGS += -DHAVE_PULSEAUDIO_DEVICE_MANAGER
endif
endif
endif

PHONON_PUBLIC_INCLUDES = \
	AbstractAudioOutput \
	AbstractMediaStream \
	AbstractVideoOutput \
	AddonInterface \
	AudioDataOutput \
	AudioDataOutputInterface \
	AudioOutput \
	AudioOutputInterface \
	BackendCapabilities \
	BackendInterface \
	Effect \
	EffectInterface \
	EffectParameter \
	EffectWidget \
	GlobalConfig \
	MediaController \
	MediaNode \
	MediaObject \
	MediaObjectInterface \
	MediaSource \
	ObjectDescription \
	ObjectDescriptionModel \
	Path \
	PlatformPlugin \
	PulseSupport \
	SeekSlider \
	StreamInterface \
	VideoPlayer \
	VideoWidget \
	VideoWidgetInterface \
	VolumeFaderEffect \
	VolumeFaderInterface \
	VolumeSlider \
	AudioCaptureDevice \
	SubtitleDescription \
	EffectDescription \
	ObjectDescriptionData \
	AudioChannelDescription \
	AudioCaptureDeviceModel \
	AudioChannelDescriptionModel \
	AudioOutputDevice \
	AudioOutputDeviceModel \
	ObjectDescriptionModelData \
	EffectDescriptionModel \
	SubtitleDescriptionModel \
	AudioOutputInterface40 \
	AudioOutputInterface42 \
	VideoWidgetInterface44 \
	VideoWidgetInterfaceLatest \
	Global \
	phonon


PHONON_3RDPARTY_INCLUDES = \
	abstractaudiooutput.h \
	abstractmediastream.h \
	abstractvideooutput.h \
	addoninterface.h \
	audiodataoutput.h \
	audiodataoutputinterface.h \
	audiooutput.h \
	audiooutputinterface.h \
	backendcapabilities.h \
	backendinterface.h \
	effect.h \
	effectinterface.h \
	effectparameter.h \
	effectwidget.h \
	globalconfig.h \
	mediacontroller.h \
	medianode.h \
	mediaobject.h \
	mediaobjectinterface.h \
	mediasource.h \
	objectdescription.h \
	objectdescriptionmodel.h \
	path.h \
	phonondefs.h \
	phonon_export.h \
	phononnamespace.h \
	platformplugin.h \
	pulsesupport.h \
	seekslider.h \
	streaminterface.h \
	videoplayer.h \
	videowidget.h \
	videowidgetinterface.h \
	volumefadereffect.h \
	volumefaderinterface.h \
	volumeslider.h

PHONON_MAIN_INCLUDES = \
	audiocapturedevice.h \
	audiochanneldescriptionmodel.h \
	audiooutputdevicemodel.h \
	audiochanneldescription.h \
	audiocapturedevicemodel.h \
	audiooutputdevice.h \
	phonon.h \
	objectdescriptiondata.h \
	effectdescription.h \
	audiooutputinterface40.h \
	videowidgetinterfacelatest.h \
	subtitledescriptionmodel.h \
	effectdescriptionmodel.h \
	objectdescriptionmodeldata.h \
	audiooutputinterface42.h \
	videowidgetinterface44.h \
	subtitledescription.h \
	global.h

PHONON_PRIVATE_INCLUDES = \
	abstractaudiooutput_p.h \
	abstractmediastream_p.h \
	abstractvideooutput_p.h \
	audiodataoutput_p.h \
	audiooutputadaptor_p.h \
	audiooutput_p.h \
	backendcapabilities_p.h \
	effect_p.h \
	effectparameter_p.h \
	effectwidget_p.h \
	factory_p.h \
	frontendinterface_p.h \
	globalconfig_p.h \
	globalstatic_p.h \
	iodevicestream_p.h \
	medianode_p.h \
	medianodedestructionhandler_p.h \
	mediaobject_p.h \
	mediasource_p.h \
	objectdescription_p.h \
	objectdescriptionmodel_p.h \
	path_p.h \
	phonondefs_p.h \
	phononnamespace_p.h \
	platform_p.h \
	qsettingsgroup_p.h \
	seekslider_p.h \
	streaminterface_p.h \
	swiftslider_p.h \
	videowidget_p.h \
	volumefadereffect_p.h \
	volumeslider_p.h

PHONON_DIRS = src/phonon src/3rdparty/phonon/phonon

lib_libCsPhonon@BUILD_ABI@_la_SOURCES+= \
	src/3rdparty/phonon/phonon/abstractaudiooutput.cpp \
	src/3rdparty/phonon/phonon/abstractaudiooutput_p.cpp \
	src/3rdparty/phonon/phonon/abstractmediastream.cpp \
	src/3rdparty/phonon/phonon/abstractvideooutput.cpp \
	src/3rdparty/phonon/phonon/abstractvideooutput_p.cpp \
	src/3rdparty/phonon/phonon/audiodataoutput.cpp \
	src/3rdparty/phonon/phonon/audiooutput.cpp \
	src/3rdparty/phonon/phonon/audiooutputinterface.cpp \
	src/3rdparty/phonon/phonon/backendcapabilities.cpp \
	src/3rdparty/phonon/phonon/effect.cpp \
	src/3rdparty/phonon/phonon/effectparameter.cpp \
	src/3rdparty/phonon/phonon/effectwidget.cpp \
	src/3rdparty/phonon/phonon/factory.cpp \
	src/3rdparty/phonon/phonon/globalconfig.cpp \
	src/3rdparty/phonon/phonon/iodevicestream.cpp \
	src/3rdparty/phonon/phonon/mediacontroller.cpp \
	src/3rdparty/phonon/phonon/medianode.cpp \
	src/3rdparty/phonon/phonon/mediaobject.cpp \
	src/3rdparty/phonon/phonon/mediasource.cpp \
	src/3rdparty/phonon/phonon/objectdescription.cpp \
	src/3rdparty/phonon/phonon/objectdescriptionmodel.cpp \
	src/3rdparty/phonon/phonon/path.cpp \
	src/3rdparty/phonon/phonon/phononnamespace.cpp \
	src/3rdparty/phonon/phonon/platform.cpp \
	src/3rdparty/phonon/phonon/pulsesupport.cpp \
	src/3rdparty/phonon/phonon/seekslider.cpp \
	src/3rdparty/phonon/phonon/streaminterface.cpp \
	src/3rdparty/phonon/phonon/swiftslider.cpp \
	src/3rdparty/phonon/phonon/videoplayer.cpp \
	src/3rdparty/phonon/phonon/videowidget.cpp \
	src/3rdparty/phonon/phonon/volumefadereffect.cpp \
	src/3rdparty/phonon/phonon/volumeslider.cpp

PHONON_INCLUDES=$(PHONON_3RDPARTY_INCLUDES) $(PHONON_MAIN_INCLUDES)

PHONON_HEADERS_INSTALLED= \
   $(patsubst %,include/phonon/%,$(PHONON_INCLUDES)) \
	$(patsubst %,include/phonon/%,$(PHONON_PUBLIC_INCLUDES)) \
	$(patsubst %,privateinclude/phonon/private/%,$(PHONON_PRIVATE_INCLUDES)) \
	include/phonon privateinclude/phonon/private

BUILT_SOURCES+=.phonon-header-deps.mk

$(lib_libCsPhonon@BUILD_ABI@_la_SOURCES) : $(CORE_HEADERS_INSTALLED) $(GUI_HEADERS_INSTALLED) $(NETWORK_HEADERS_INSTALLED) $(PHONON_HEADERS_INSTALLED)

-include .phonon-header-deps.mk

.phonon-header-deps.mk: Makefile
	@- rm -rf $@
	echo "include/phonon:" >> $@; \
	echo "	\$$(MKDIR_P) \$$@" >> $@ ; \
	echo "privateinclude/phonon/private:" >> $@; \
	echo "	\$$(MKDIR_P) \$$@" >> $@ ; \
	for i in $(PHONON_PUBLIC_INCLUDES); do \
		echo "include/phonon/$$i: include/phonon/$$(echo $$i | tr A-Z a-z).h | include/phonon" >> $@; \
		echo "	\$$(INSTALL) \$$< \$$@" >> $@ ; \
	done; \
	for i in $(PHONON_DIRS); do \
		echo "include/phonon/%: $$i/% | include/phonon" >> $@ ; \
		echo "	\$$(INSTALL) \$$< \$$@" >> $@ ; \
	done;\
	for i in $(PHONON_DIRS); do \
		echo "privateinclude/phonon/private/%: $$i/% | privateinclude/phonon/private" >> $@ ; \
		echo "	\$$(INSTALL) \$$< \$$@" >> $@ ; \
	done


noinst_HEADERS+= \
	src/3rdparty/phonon/ds9/abstractvideorenderer.h \
	src/3rdparty/phonon/ds9/audiooutput.h \
	src/3rdparty/phonon/ds9/backend.h \
	src/3rdparty/phonon/ds9/backendnode.h \
	src/3rdparty/phonon/ds9/effect.h \
	src/3rdparty/phonon/ds9/fakesource.h \
	src/3rdparty/phonon/ds9/iodevicereader.h \
	src/3rdparty/phonon/ds9/mediagraph.h \
	src/3rdparty/phonon/ds9/mediaobject.h \
	src/3rdparty/phonon/ds9/videowidget.h \
	src/3rdparty/phonon/ds9/videorenderer_soft.h \
	src/3rdparty/phonon/ds9/volumeeffect.h \
	src/3rdparty/phonon/ds9/qbasefilter.h \
	src/3rdparty/phonon/ds9/qpin.h \
	src/3rdparty/phonon/ds9/qasyncreader.h \
	src/3rdparty/phonon/ds9/qaudiocdreader.h \
	src/3rdparty/phonon/ds9/qmeminputpin.h \
	src/3rdparty/phonon/ds9/compointer.h \
	src/3rdparty/phonon/ds9/phononds9_namespace.h \
	src/3rdparty/phonon/ds9/qevr9.h \
	src/3rdparty/phonon/ds9/videorenderer_evr.h \
	src/3rdparty/phonon/ds9/videorenderer_vmr9.h \
	src/3rdparty/phonon/gstreamer/abstractrenderer.h \
	src/3rdparty/phonon/gstreamer/artssink.h \
	src/3rdparty/phonon/gstreamer/audiodataoutput.h \
	src/3rdparty/phonon/gstreamer/audioeffect.h \
	src/3rdparty/phonon/gstreamer/audiooutput.h \
	src/3rdparty/phonon/gstreamer/backend.h \
	src/3rdparty/phonon/gstreamer/common.h \
	src/3rdparty/phonon/gstreamer/devicemanager.h \
	src/3rdparty/phonon/gstreamer/effect.h \
	src/3rdparty/phonon/gstreamer/effectmanager.h \
	src/3rdparty/phonon/gstreamer/glrenderer.h \
	src/3rdparty/phonon/gstreamer/gsthelper.h \
	src/3rdparty/phonon/gstreamer/medianode.h \
	src/3rdparty/phonon/gstreamer/medianodeevent.h \
	src/3rdparty/phonon/gstreamer/mediaobject.h \
	src/3rdparty/phonon/gstreamer/message.h \
	src/3rdparty/phonon/gstreamer/phononsrc.h \
	src/3rdparty/phonon/gstreamer/qwidgetvideosink.h \
	src/3rdparty/phonon/gstreamer/streamreader.h \
	src/3rdparty/phonon/gstreamer/videowidget.h \
	src/3rdparty/phonon/gstreamer/volumefadereffect.h \
	src/3rdparty/phonon/gstreamer/widgetrenderer.h \
	src/3rdparty/phonon/gstreamer/x11renderer.h \
	src/3rdparty/phonon/qt7/medianode.h \
	src/3rdparty/phonon/qt7/backend.h \
	src/3rdparty/phonon/qt7/videowidget.h \
	src/3rdparty/phonon/qt7/mediaobject.h \
	src/3rdparty/phonon/qt7/quicktimevideoplayer.h \
	src/3rdparty/phonon/qt7/backendheader.h \
	src/3rdparty/phonon/qt7/medianodevideopart.h \
	src/3rdparty/phonon/qt7/medianodeevent.h  \
	src/3rdparty/phonon/qt7/quicktimeaudioplayer.h \
	src/3rdparty/phonon/qt7/audionode.h  \
	src/3rdparty/phonon/qt7/audiograph.h \
	src/3rdparty/phonon/qt7/audiooutput.h \
	src/3rdparty/phonon/qt7/quicktimemetadata.h \
	src/3rdparty/phonon/qt7/audiomixer.h \
	src/3rdparty/phonon/qt7/audiodevice.h \
	src/3rdparty/phonon/qt7/backendinfo.h \
	src/3rdparty/phonon/qt7/audioconnection.h \
	src/3rdparty/phonon/qt7/audiopartoutput.h \
	src/3rdparty/phonon/qt7/videoframe.h \
	src/3rdparty/phonon/qt7/audiosplitter.h \
	src/3rdparty/phonon/qt7/audioeffects.h \
	src/3rdparty/phonon/qt7/quicktimestreamreader.h \
	src/3rdparty/phonon/qt7/mediaobjectaudionode.h \
	$(patsubst %,src/phonon/%,$(PHONON_MAIN_INCLUDES)) \
	$(patsubst %,src/3rdparty/phonon/phonon/%,$(PHONON_PRIVATE_INCLUDES)) \
	$(patsubst %,src/3rdparty/phonon/phonon/%,$(PHONON_3RDPARTY_INCLUDES))

if OSTYPE_WIN

if PHONON
lib_LTLIBRARIES += lib/phonon_ds9.la
endif

lib_phonon_ds9_la_SOURCES= \
	src/3rdparty/phonon/ds9/abstractvideorenderer.cpp \
	src/3rdparty/phonon/ds9/audiooutput.cpp \
	src/3rdparty/phonon/ds9/backend.cpp \
	src/3rdparty/phonon/ds9/backendnode.cpp \
	src/3rdparty/phonon/ds9/effect.cpp \
	src/3rdparty/phonon/ds9/fakesource.cpp \
	src/3rdparty/phonon/ds9/iodevicereader.cpp \
	src/3rdparty/phonon/ds9/mediagraph.cpp \
	src/3rdparty/phonon/ds9/mediaobject.cpp \
	src/3rdparty/phonon/ds9/videowidget.cpp \
	src/3rdparty/phonon/ds9/videorenderer_soft.cpp \
	src/3rdparty/phonon/ds9/volumeeffect.cpp \
	src/3rdparty/phonon/ds9/qbasefilter.cpp \
	src/3rdparty/phonon/ds9/qpin.cpp \
	src/3rdparty/phonon/ds9/qasyncreader.cpp \
	src/3rdparty/phonon/ds9/qaudiocdreader.cpp \
	src/3rdparty/phonon/ds9/qmeminputpin.cpp \
	src/3rdparty/phonon/ds9/videorenderer_evr.cpp \
	src/3rdparty/phonon/ds9/videorenderer_vmr9.cpp

lib_phonon_ds9_la_CXXFLAGS=$(AM_CXXFLAGS) -I$(top_srcdir)/src/3rdparty/phonon/ds9/ -Iinclude -Iinclude/QtCore -Iinclude/QtGui \
      -Isrc/3rdparty/phonon/ds9  -DQT_BUILD_DS9_LIB

lib_phonon_ds9_la_LDFLAGS=$(AM_LDFLAGS) -no-undefined -module -lopengl32 -Wl,-lstrmiids -Wl,-ldmoguids -Wl,-luuid -lmsdmo \
      -lole32 -loleaut32 -lgdi32

lib_phonon_ds9_la_LIBADD=lib/libCsCore@BUILD_ABI@.la lib/libCsGui@BUILD_ABI@.la lib/libCsPhonon@BUILD_ABI@.la

endif


if OSTYPE_LINUX

if PHONON
lib_LTLIBRARIES += lib/phonon_gstreamer.la
endif

lib_phonon_gstreamer_la_SOURCES= \
	src/3rdparty/phonon/gstreamer/abstractrenderer.cpp \
	src/3rdparty/phonon/gstreamer/artssink.cpp \
	src/3rdparty/phonon/gstreamer/audiodataoutput.cpp \
	src/3rdparty/phonon/gstreamer/audioeffect.cpp \
	src/3rdparty/phonon/gstreamer/audiooutput.cpp \
	src/3rdparty/phonon/gstreamer/backend.cpp \
	src/3rdparty/phonon/gstreamer/devicemanager.cpp \
	src/3rdparty/phonon/gstreamer/effect.cpp \
	src/3rdparty/phonon/gstreamer/effectmanager.cpp \
	src/3rdparty/phonon/gstreamer/glrenderer.cpp \
	src/3rdparty/phonon/gstreamer/gsthelper.cpp \
	src/3rdparty/phonon/gstreamer/medianode.cpp \
	src/3rdparty/phonon/gstreamer/medianodeevent.cpp \
	src/3rdparty/phonon/gstreamer/mediaobject.cpp \
	src/3rdparty/phonon/gstreamer/message.cpp \
	src/3rdparty/phonon/gstreamer/phononsrc.cpp \
	src/3rdparty/phonon/gstreamer/qwidgetvideosink.cpp \
	src/3rdparty/phonon/gstreamer/streamreader.cpp \
	src/3rdparty/phonon/gstreamer/videowidget.cpp \
	src/3rdparty/phonon/gstreamer/volumefadereffect.cpp \
	src/3rdparty/phonon/gstreamer/widgetrenderer.cpp \
	src/3rdparty/phonon/gstreamer/x11renderer.cpp

lib_phonon_gstreamer_la_CXXFLAGS=$(AM_CXXFLAGS) -DQT_NO_OPENGL $(GSTREAMER_CFLAGS) $(GSTREAMER_AUDIO_CFLAGS) \
      -I$(top_srcdir)/src/3rdparty/phonon/gstreamer -Iinclude -Iinclude/QtCore -Iinclude/QtGui -Iinclude/phonon \
      -Isrc/3rdparty/phonon/gstreamer -DQT_BUILD_GSTREAMER_LIB

lib_phonon_gstreamer_la_LDFLAGS=$(AM_LDFLAGS) -avoid-version -no-undefined -module $(GSTREAMER_LIBS) $(GSTREAMER_AUDIO_LIBS)
lib_phonon_gstreamer_la_LIBADD=lib/libCsCore@BUILD_ABI@.la lib/libCsGui@BUILD_ABI@.la lib/libCsPhonon@BUILD_ABI@.la

endif

if OSTYPE_BSD

if PHONON
lib_LTLIBRARIES += lib/phonon_gstreamer.la
endif

lib_phonon_gstreamer_la_SOURCES= \
        src/3rdparty/phonon/gstreamer/abstractrenderer.cpp \
        src/3rdparty/phonon/gstreamer/artssink.cpp \
        src/3rdparty/phonon/gstreamer/audiodataoutput.cpp \
        src/3rdparty/phonon/gstreamer/audioeffect.cpp \
        src/3rdparty/phonon/gstreamer/audiooutput.cpp \
        src/3rdparty/phonon/gstreamer/backend.cpp \
        src/3rdparty/phonon/gstreamer/devicemanager.cpp \
        src/3rdparty/phonon/gstreamer/effect.cpp \
        src/3rdparty/phonon/gstreamer/effectmanager.cpp \
        src/3rdparty/phonon/gstreamer/glrenderer.cpp \
        src/3rdparty/phonon/gstreamer/gsthelper.cpp \
        src/3rdparty/phonon/gstreamer/medianode.cpp \
        src/3rdparty/phonon/gstreamer/medianodeevent.cpp \
        src/3rdparty/phonon/gstreamer/mediaobject.cpp \
        src/3rdparty/phonon/gstreamer/message.cpp \
        src/3rdparty/phonon/gstreamer/phononsrc.cpp \
        src/3rdparty/phonon/gstreamer/qwidgetvideosink.cpp \
        src/3rdparty/phonon/gstreamer/streamreader.cpp \
        src/3rdparty/phonon/gstreamer/videowidget.cpp \
        src/3rdparty/phonon/gstreamer/volumefadereffect.cpp \
        src/3rdparty/phonon/gstreamer/widgetrenderer.cpp \
        src/3rdparty/phonon/gstreamer/x11renderer.cpp

lib_phonon_gstreamer_la_CXXFLAGS=$(AM_CXXFLAGS) -DQT_NO_OPENGL $(GSTREAMER_CFLAGS) $(GSTREAMER_AUDIO_CFLAGS) \
      -I$(top_srcdir)/src/3rdparty/phonon/gstreamer -Iinclude -Iinclude/QtCore -Iinclude/QtGui -Iinclude/phonon \
      -Isrc/3rdparty/phonon/gstreamer -DQT_BUILD_GSTREAMER_LIB

lib_phonon_gstreamer_la_LDFLAGS=$(AM_LDFLAGS) -avoid-version -no-undefined -module $(GSTREAMER_LIBS) $(GSTREAMER_AUDIO_LIBS)
lib_phonon_gstreamer_la_LIBADD=lib/libCsCore@BUILD_ABI@.la lib/libCsGui@BUILD_ABI@.la lib/libCsPhonon@BUILD_ABI@.la

endif


EXTRA_DIST+= \
	src/3rdparty/phonon/qt7/videoeffect.h

if OSTYPE_DARWIN

if PHONON
lib_LTLIBRARIES += lib/phonon_qt7.la
endif

lib_phonon_qt7_la_SOURCES= \
	src/3rdparty/phonon/qt7/quicktimevideoplayer.mm \
	src/3rdparty/phonon/qt7/backendheader.mm \
	src/3rdparty/phonon/qt7/medianodevideopart.mm \
	src/3rdparty/phonon/qt7/medianodeevent.mm \
	src/3rdparty/phonon/qt7/audiooutput.mm \
	src/3rdparty/phonon/qt7/backendinfo.mm \
	src/3rdparty/phonon/qt7/audiosplitter.mm \
	src/3rdparty/phonon/qt7/audioeffects.mm \
	src/3rdparty/phonon/qt7/quicktimestreamreader.mm \
	src/3rdparty/phonon/qt7/medianode.mm \
	src/3rdparty/phonon/qt7/backend.mm \
	src/3rdparty/phonon/qt7/mediaobject.mm \
	src/3rdparty/phonon/qt7/mediaobjectaudionode.mm \
	src/3rdparty/phonon/qt7/audiomixer.mm  \
	src/3rdparty/phonon/qt7/quicktimeaudioplayer.mm \
	src/3rdparty/phonon/qt7/videoframe.mm \
	src/3rdparty/phonon/qt7/quicktimemetadata.mm \
	src/3rdparty/phonon/qt7/audiodevice.mm \
	src/3rdparty/phonon/qt7/audioconnection.mm \
	src/3rdparty/phonon/qt7/audiograph.mm \
	src/3rdparty/phonon/qt7/audionode.mm \
	src/3rdparty/phonon/qt7/videowidget.mm

lib_phonon_qt7_la_OBJCXXFLAGS=$(AM_CXXFLAGS) -DQT_NO_OPENGL -I$(top_srcdir)/src/3rdparty/phonon/qt7 -Iinclude -Iinclude/QtCore \
      -Iinclude/QtGui -Iinclude/phonon -Isrc/3rdparty/phonon/qt7/ -Iprivateinclude -IprivateInclude/OpenGL \
      -DQT_BUILD_QT7_LIB

lib_phonon_qt7_la_LDFLAGS=$(AM_LDFLAGS) -no-undefined -module \
	-Xarch_i386 -framework QuickTime -Xarch_x86_64 -framework QTKit -framework AppKit  -framework AudioUnit \
   -framework AudioToolbox 	-framework CoreAudio  -framework QuartzCore

lib_phonon_qt7_la_LIBADD=lib/libCsCore@BUILD_ABI@.la lib/libCsGui@BUILD_ABI@.la lib/libCsPhonon@BUILD_ABI@.la lib/libCsOpenGL@BUILD_ABI@.la

endif
