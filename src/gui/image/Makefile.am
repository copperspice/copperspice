GUI_DIRS += src/gui/image

GUI_PUBLIC_INCLUDES += \
	QBitmap \
	QIcon \
	QIconEngine \
	QIconEngineFactoryInterface \
	QIconEngineFactoryInterfaceV2 \
	QIconEnginePlugin \
	QIconEnginePluginV2 \
	QIconEngineV2 \
	QIconSet \
	QImage \
	QImageIOHandler \
	QImageIOHandlerFactoryInterface \
	QImageIOPlugin \
	QImageReader \
	QImageTextKeyLang \
	QImageWriter \
	QMovie \
	QPicture \
	QPictureFormatInterface \
	QPictureFormatPlugin \
	QPictureIO \
	QPixmap \
	QPixmapCache

GUI_IMAGE_INCLUDES = \
	qbitmap.h \
	qicon.h \
	qiconengine.h \
	qiconenginefactoryinterface.h \
	qiconenginefactoryinterfacev2.h \
	qiconengineplugin.h \
	qiconenginepluginv2.h \
	qiconenginev2.h \
	qiconset.h \
	qimage.h \
	qimageiohandler.h \
	qimageiohandlerfactoryinterface.h \
	qimageioplugin.h \
	qimagereader.h \
	qimagetextkeylang.h \
	qimagewriter.h \
	qmovie.h \
	qpicture.h \
	qpictureformatinterface.h \
	qpictureformatplugin.h \
	qpictureio.h \
	qpixmap.h \
	qpixmapcache.h

GUI_IMAGE_PRIVATE_INCLUDES = \
	qbmphandler_p.h \
	qicohandler_p.h \
	qicon_p.h \
	qiconloader_p.h \
	qimage_p.h \
	qimagepixmapcleanuphooks_p.h \
	qjpeghandler_p.h \
	qmnghandler_p.h \
	qnativeimage_p.h \
	qnativeimagehandleprovider_p.h \
	qgifhandler_p.h \
	qpaintengine_pic_p.h \
	qpicture_p.h \
	qpixmap_blitter_p.h \
	qpixmap_raster_p.h \
	qpixmap_mac_p.h \
	qpixmap_x11_p.h \
	qpixmapcache_p.h \
	qpixmapdata_p.h \
	qpixmapdatafactory_p.h \
	qpixmapfilter_p.h \
	qpnghandler_p.h \
	qppmhandler_p.h \
	qtiffhandler_p.h \
	qvolatileimage_p.h \
	qvolatileimagedata_p.h \
	qxbmhandler_p.h \
	qxpmhandler_p.h

GUI_INCLUDES += $(GUI_IMAGE_INCLUDES)
GUI_PRIVATE_INCLUDES += $(GUI_IMAGE_PRIVATE_INCLUDES)

noinst_HEADERS+=$(patsubst %,src/gui/image/%,$(GUI_IMAGE_INCLUDES)) \
		$(patsubst %,src/gui/image/%,$(GUI_IMAGE_PRIVATE_INCLUDES))

lib_libCsGui@BUILD_ABI@_la_SOURCES += \
	src/gui/image/qbmphandler.cpp \
	src/gui/image/qbitmap.cpp \
	src/gui/image/qicon.cpp \
	src/gui/image/qiconloader.cpp \
	src/gui/image/qimage.cpp \
	src/gui/image/qimageiohandler.cpp \
	src/gui/image/qimagereader.cpp \
	src/gui/image/qimagewriter.cpp \
	src/gui/image/qpaintengine_pic.cpp \
	src/gui/image/qpicture.cpp \
	src/gui/image/qpictureformatplugin.cpp \
	src/gui/image/qpixmap.cpp \
	src/gui/image/qpixmapcache.cpp \
	src/gui/image/qpixmapdata.cpp \
	src/gui/image/qpixmapdatafactory.cpp \
	src/gui/image/qpixmapfilter.cpp \
	src/gui/image/qiconengine.cpp \
	src/gui/image/qiconengineplugin.cpp \
	src/gui/image/qmovie.cpp \
	src/gui/image/qpixmap_raster.cpp \
	src/gui/image/qpixmap_blitter.cpp \
	src/gui/image/qnativeimage.cpp \
	src/gui/image/qimagepixmapcleanuphooks.cpp \
	src/gui/image/qvolatileimage.cpp \
	src/gui/image/qgifhandler.cpp \
	src/gui/image/qicohandler.cpp \
	src/gui/image/qpnghandler.cpp \
	src/gui/image/qjpeghandler.cpp \
	src/gui/image/qxpmhandler.cpp \
	src/gui/image/qxbmhandler.cpp \
	src/gui/image/qppmhandler.cpp \
	src/gui/image/qtiffhandler.cpp \
	src/gui/image/qmnghandler.cpp \
	src/gui/image/qvolatileimagedata.cpp

lib_libCsGui@BUILD_ABI@_la_SOURCES += \
	src/3rdparty/libjpeg/jaricom.c \
	src/3rdparty/libjpeg/jcapimin.c \
	src/3rdparty/libjpeg/jcapistd.c \
	src/3rdparty/libjpeg/jcarith.c \
	src/3rdparty/libjpeg/jccoefct.c \
	src/3rdparty/libjpeg/jccolor.c \
	src/3rdparty/libjpeg/jcdctmgr.c \
	src/3rdparty/libjpeg/jchuff.c \
	src/3rdparty/libjpeg/jcinit.c \
	src/3rdparty/libjpeg/jcmainct.c \
	src/3rdparty/libjpeg/jcmarker.c \
	src/3rdparty/libjpeg/jcmaster.c \
	src/3rdparty/libjpeg/jcomapi.c \
	src/3rdparty/libjpeg/jcparam.c \
	src/3rdparty/libjpeg/jcprepct.c \
	src/3rdparty/libjpeg/jcsample.c \
	src/3rdparty/libjpeg/jctrans.c \
	src/3rdparty/libjpeg/jdapimin.c \
	src/3rdparty/libjpeg/jdapistd.c \
	src/3rdparty/libjpeg/jdarith.c \
	src/3rdparty/libjpeg/jdatadst.c \
	src/3rdparty/libjpeg/jdatasrc.c \
	src/3rdparty/libjpeg/jdcoefct.c \
	src/3rdparty/libjpeg/jdcolor.c \
	src/3rdparty/libjpeg/jddctmgr.c \
	src/3rdparty/libjpeg/jdhuff.c \
	src/3rdparty/libjpeg/jdinput.c \
	src/3rdparty/libjpeg/jdmainct.c \
	src/3rdparty/libjpeg/jdmarker.c \
	src/3rdparty/libjpeg/jdmaster.c \
	src/3rdparty/libjpeg/jdmerge.c \
	src/3rdparty/libjpeg/jdpostct.c \
	src/3rdparty/libjpeg/jdsample.c \
	src/3rdparty/libjpeg/jdtrans.c \
	src/3rdparty/libjpeg/jerror.c \
	src/3rdparty/libjpeg/jfdctflt.c \
	src/3rdparty/libjpeg/jfdctfst.c \
	src/3rdparty/libjpeg/jfdctint.c \
	src/3rdparty/libjpeg/jidctflt.c \
	src/3rdparty/libjpeg/jidctfst.c \
	src/3rdparty/libjpeg/jidctint.c \
	src/3rdparty/libjpeg/jmemmgr.c \
	src/3rdparty/libjpeg/jmemnobs.c \
	src/3rdparty/libjpeg/jquant1.c \
	src/3rdparty/libjpeg/jquant2.c \
	src/3rdparty/libjpeg/jutils.c \
	src/3rdparty/libmng/libmng_callback_xs.c \
	src/3rdparty/libmng/libmng_chunk_descr.c \
	src/3rdparty/libmng/libmng_chunk_io.c \
	src/3rdparty/libmng/libmng_chunk_prc.c \
	src/3rdparty/libmng/libmng_chunk_xs.c \
	src/3rdparty/libmng/libmng_cms.c \
	src/3rdparty/libmng/libmng_display.c \
	src/3rdparty/libmng/libmng_dither.c \
	src/3rdparty/libmng/libmng_error.c \
	src/3rdparty/libmng/libmng_filter.c \
	src/3rdparty/libmng/libmng_hlapi.c \
	src/3rdparty/libmng/libmng_jpeg.c \
	src/3rdparty/libmng/libmng_object_prc.c \
	src/3rdparty/libmng/libmng_pixels.c \
	src/3rdparty/libmng/libmng_prop_xs.c \
	src/3rdparty/libmng/libmng_read.c \
	src/3rdparty/libmng/libmng_trace.c \
	src/3rdparty/libmng/libmng_write.c \
	src/3rdparty/libmng/libmng_zlib.c \
	src/3rdparty/libpng/png.c \
	src/3rdparty/libpng/pngerror.c \
	src/3rdparty/libpng/pngget.c \
	src/3rdparty/libpng/pngmem.c \
	src/3rdparty/libpng/pngpread.c \
	src/3rdparty/libpng/pngread.c \
	src/3rdparty/libpng/pngrio.c \
	src/3rdparty/libpng/pngrtran.c \
	src/3rdparty/libpng/pngrutil.c \
	src/3rdparty/libpng/pngset.c \
	src/3rdparty/libpng/pngtrans.c \
	src/3rdparty/libpng/pngwio.c \
	src/3rdparty/libpng/pngwrite.c \
	src/3rdparty/libpng/pngwtran.c \
	src/3rdparty/libpng/pngwutil.c \
	src/3rdparty/libtiff/libtiff/tif_aux.c \
	src/3rdparty/libtiff/libtiff/tif_close.c \
	src/3rdparty/libtiff/libtiff/tif_codec.c \
	src/3rdparty/libtiff/libtiff/tif_color.c \
	src/3rdparty/libtiff/libtiff/tif_compress.c \
	src/3rdparty/libtiff/libtiff/tif_dir.c \
	src/3rdparty/libtiff/libtiff/tif_dirinfo.c \
	src/3rdparty/libtiff/libtiff/tif_dirread.c \
	src/3rdparty/libtiff/libtiff/tif_dirwrite.c \
	src/3rdparty/libtiff/libtiff/tif_dumpmode.c \
	src/3rdparty/libtiff/libtiff/tif_error.c \
	src/3rdparty/libtiff/libtiff/tif_extension.c \
	src/3rdparty/libtiff/libtiff/tif_fax3.c \
	src/3rdparty/libtiff/libtiff/tif_fax3sm.c \
	src/3rdparty/libtiff/libtiff/tif_flush.c \
	src/3rdparty/libtiff/libtiff/tif_getimage.c \
	src/3rdparty/libtiff/libtiff/tif_luv.c \
	src/3rdparty/libtiff/libtiff/tif_lzw.c \
	src/3rdparty/libtiff/libtiff/tif_next.c \
	src/3rdparty/libtiff/libtiff/tif_open.c \
	src/3rdparty/libtiff/libtiff/tif_packbits.c \
	src/3rdparty/libtiff/libtiff/tif_pixarlog.c \
	src/3rdparty/libtiff/libtiff/tif_predict.c \
	src/3rdparty/libtiff/libtiff/tif_print.c \
	src/3rdparty/libtiff/libtiff/tif_read.c \
	src/3rdparty/libtiff/libtiff/tif_strip.c \
	src/3rdparty/libtiff/libtiff/tif_swab.c \
	src/3rdparty/libtiff/libtiff/tif_thunder.c \
	src/3rdparty/libtiff/libtiff/tif_tile.c \
	src/3rdparty/libtiff/libtiff/tif_version.c \
	src/3rdparty/libtiff/libtiff/tif_warning.c \
	src/3rdparty/libtiff/libtiff/tif_write.c \
	src/3rdparty/libtiff/libtiff/tif_zip.c


lib_libCsGui@BUILD_ABI@_la_CFLAGS += -DMNG_BUILD_SO -DMNG_NO_INCLUDE_JNG
lib_libCsGui@BUILD_ABI@_la_CFLAGS += -Iinclude/QtCore
lib_libCsGui@BUILD_ABI@_la_CFLAGS += -I$(top_srcdir)/src/3rdparty/libpng -I$(top_srcdir)/src/3rdparty/zlib
lib_libCsGui@BUILD_ABI@_la_CXXFLAGS += -I$(top_srcdir)/src/3rdparty/libpng


if OSTYPE_WIN
lib_libCsGui@BUILD_ABI@_la_SOURCES += \
	src/3rdparty/libtiff/libtiff/tif_win32.c
else
lib_libCsGui@BUILD_ABI@_la_SOURCES += \
	src/3rdparty/libtiff/libtiff/tif_unix.c
endif


EXTRA_DIST += \
	src/3rdparty/libtiff/libtiff/t4.h \
	src/3rdparty/libtiff/libtiff/tif_config.h \
	src/3rdparty/libtiff/libtiff/tif_config.vc.h \
	src/3rdparty/libtiff/libtiff/tif_dir.h \
	src/3rdparty/libtiff/libtiff/tif_fax3.h \
	src/3rdparty/libtiff/libtiff/tif_predict.h \
	src/3rdparty/libtiff/libtiff/tiff.h \
	src/3rdparty/libtiff/libtiff/tiffconf.h \
	src/3rdparty/libtiff/libtiff/tiffconf.vc.h \
	src/3rdparty/libtiff/libtiff/tiffio.h \
	src/3rdparty/libtiff/libtiff/tiffiop.h \
	src/3rdparty/libtiff/libtiff/tiffvers.h \
	src/3rdparty/libtiff/libtiff/uvcode.h \
	src/3rdparty/libmng/libmng_chunk_descr.h \
	src/3rdparty/libmng/libmng_chunk_io.h \
	src/3rdparty/libmng/libmng_chunk_prc.h \
	src/3rdparty/libmng/libmng_chunks.h \
	src/3rdparty/libmng/libmng_cms.h \
	src/3rdparty/libmng/libmng_conf.h \
	src/3rdparty/libmng/libmng_data.h \
	src/3rdparty/libmng/libmng_display.h \
	src/3rdparty/libmng/libmng_dither.h \
	src/3rdparty/libmng/libmng_error.h \
	src/3rdparty/libmng/libmng_filter.h \
	src/3rdparty/libmng/libmng.h \
	src/3rdparty/libmng/libmng_jpeg.h \
	src/3rdparty/libmng/libmng_memory.h \
	src/3rdparty/libmng/libmng_object_prc.h \
	src/3rdparty/libmng/libmng_objects.h \
	src/3rdparty/libmng/libmng_pixels.h \
	src/3rdparty/libmng/libmng_read.h \
	src/3rdparty/libmng/libmng_trace.h \
	src/3rdparty/libmng/libmng_types.h \
	src/3rdparty/libmng/libmng_write.h \
	src/3rdparty/libmng/libmng_zlib.h \
	src/3rdparty/libjpeg/cderror.h \
	src/3rdparty/libjpeg/cdjpeg.h \
	src/3rdparty/libjpeg/jconfig.h \
	src/3rdparty/libjpeg/jdct.h \
	src/3rdparty/libjpeg/jerror.h \
	src/3rdparty/libjpeg/jinclude.h \
	src/3rdparty/libjpeg/jmemsys.h \
	src/3rdparty/libjpeg/jmorecfg.h \
	src/3rdparty/libjpeg/jpegint.h \
	src/3rdparty/libjpeg/jpeglib.h \
	src/3rdparty/libjpeg/jversion.h \
	src/3rdparty/libjpeg/transupp.h \
	src/3rdparty/libpng/pngconf.h \
	src/3rdparty/libpng/pngdebug.h \
	src/3rdparty/libpng/png.h \
	src/3rdparty/libpng/pnginfo.h \
	src/3rdparty/libpng/pnglibconf.h \
	src/3rdparty/libpng/pngpriv.h \
	src/3rdparty/libpng/pngstruct.h

if OSTYPE_WIN
lib_libCsGui@BUILD_ABI@_la_SOURCES += \
	src/gui/image/qpixmap_win.cpp
endif

if X11
lib_libCsGui@BUILD_ABI@_la_SOURCES += \
	src/gui/image/qpixmap_x11.cpp
endif

if COCOA
lib_libCsGui@BUILD_ABI@_la_SOURCES += \
	src/gui/image/qpixmap_mac.cpp
endif
