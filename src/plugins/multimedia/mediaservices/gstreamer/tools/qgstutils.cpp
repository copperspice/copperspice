/***********************************************************************
*
* Copyright (c) 2012-2024 Barbara Geller
* Copyright (c) 2012-2024 Ansel Sermersheim
*
* Copyright (c) 2015 The Qt Company Ltd.
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
*
* This file is part of CopperSpice.
*
* CopperSpice is free software. You can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*
* https://www.gnu.org/licenses/
*
***********************************************************************/

#include <qgstutils_p.h>

#include <qdatetime.h>
#include <qdir.h>
#include <qbytearray.h>
#include <qvariant.h>
#include <qsize.h>
#include <qset.h>
#include <qstringlist.h>
#include <qimage.h>
#include <qaudioformat.h>
#include <qelapsedtimer.h>
#include <qvideosurfaceformat.h>
#include <qmultimediautils_p.h>

#include <gst/audio/audio.h>
#include <gst/video/video.h>

template<typename T, int N> static int lengthOf(const T (&)[N])
{
   return N;
}

#ifdef USE_V4L
#  include <qcore_unix_p.h>
#  include <linux/videodev2.h>
#endif

#include <qgstreamervideoinputdevicecontrol_p.h>

//internal
static void addTagToMap(const GstTagList *list, const gchar *tag, gpointer user_data)
{
   QMap<QByteArray, QVariant> *map = reinterpret_cast<QMap<QByteArray, QVariant>* >(user_data);

   GValue val;
   val.g_type = 0;
   gst_tag_list_copy_value(&val, list, tag);

   switch ( G_VALUE_TYPE(&val) ) {
      case G_TYPE_STRING: {
         const gchar *str_value = g_value_get_string(&val);
         map->insert(QByteArray(tag), QString::fromUtf8(str_value));
         break;
      }
      case G_TYPE_INT:
         map->insert(QByteArray(tag), g_value_get_int(&val));
         break;
      case G_TYPE_UINT:
         map->insert(QByteArray(tag), g_value_get_uint(&val));
         break;
      case G_TYPE_LONG:
         map->insert(QByteArray(tag), qint64(g_value_get_long(&val)));
         break;
      case G_TYPE_BOOLEAN:
         map->insert(QByteArray(tag), g_value_get_boolean(&val));
         break;
      case G_TYPE_CHAR:
#if GLIB_CHECK_VERSION(2,32,0)
         map->insert(QByteArray(tag), g_value_get_schar(&val));
#else
         map->insert(QByteArray(tag), g_value_get_char(&val));
#endif
         break;
      case G_TYPE_DOUBLE:
         map->insert(QByteArray(tag), g_value_get_double(&val));
         break;
      default:
         // GST_TYPE_DATE is a function, not a constant, so pull it out of the switch
#if GST_CHECK_VERSION(1,0,0)
         if (G_VALUE_TYPE(&val) == G_TYPE_DATE) {
            const GDate *date = (const GDate *)g_value_get_boxed(&val);
#else
         if (G_VALUE_TYPE(&val) == GST_TYPE_DATE) {
            const GDate *date = gst_value_get_date(&val);
#endif
            if (g_date_valid(date)) {
               int year = g_date_get_year(date);
               int month = g_date_get_month(date);
               int day = g_date_get_day(date);
               map->insert(QByteArray(tag), QDate(year, month, day));
               if (!map->contains("year")) {
                  map->insert("year", year);
               }
            }
         } else if (G_VALUE_TYPE(&val) == GST_TYPE_FRACTION) {
            int nom = gst_value_get_fraction_numerator(&val);
            int denom = gst_value_get_fraction_denominator(&val);

            if (denom > 0) {
               map->insert(QByteArray(tag), double(nom) / denom);
            }
         }
         break;
   }

   g_value_unset(&val);
}

/*!
  Convert GstTagList structure to QMap<QByteArray, QVariant>.

  Mapping to int, bool, char, string, fractions and date are supported.
  Fraction values are converted to doubles.
*/
QMap<QByteArray, QVariant> QGstUtils::gstTagListToMap(const GstTagList *tags)
{
   QMap<QByteArray, QVariant> res;
   gst_tag_list_foreach(tags, addTagToMap, &res);

   return res;
}

/*!
  Returns resolution of \a caps.
  If caps doesn't have a valid size, and ampty QSize is returned.
*/
QSize QGstUtils::capsResolution(const GstCaps *caps)
{
   if (gst_caps_get_size(caps) == 0) {
      return QSize();
   }

   return structureResolution(gst_caps_get_structure(caps, 0));
}

/*!
  Returns aspect ratio corrected resolution of \a caps.
  If caps doesn't have a valid size, an empty QSize is returned.
*/
QSize QGstUtils::capsCorrectedResolution(const GstCaps *caps)
{
   QSize size;

   if (caps) {
      size = capsResolution(caps);

      gint aspectNum = 0;
      gint aspectDenum = 0;
      if (!size.isEmpty() && gst_structure_get_fraction(
            gst_caps_get_structure(caps, 0), "pixel-aspect-ratio", &aspectNum, &aspectDenum)) {
         if (aspectDenum > 0) {
            size.setWidth(size.width()*aspectNum / aspectDenum);
         }
      }
   }

   return size;
}


#if GST_CHECK_VERSION(1,0,0)
namespace {

struct AudioFormat {
   GstAudioFormat format;
   QAudioFormat::SampleType sampleType;
   QAudioFormat::Endian byteOrder;
   int sampleSize;
};
static const AudioFormat qt_audioLookup[] = {
   { GST_AUDIO_FORMAT_S8, QAudioFormat::SignedInt, QAudioFormat::LittleEndian, 8  },
   { GST_AUDIO_FORMAT_U8, QAudioFormat::UnSignedInt, QAudioFormat::LittleEndian, 8  },
   { GST_AUDIO_FORMAT_S16LE, QAudioFormat::SignedInt, QAudioFormat::LittleEndian, 16 },
   { GST_AUDIO_FORMAT_S16BE, QAudioFormat::SignedInt, QAudioFormat::BigEndian, 16 },
   { GST_AUDIO_FORMAT_U16LE, QAudioFormat::UnSignedInt, QAudioFormat::LittleEndian, 16 },
   { GST_AUDIO_FORMAT_U16BE, QAudioFormat::UnSignedInt, QAudioFormat::BigEndian, 16 },
   { GST_AUDIO_FORMAT_S32LE, QAudioFormat::SignedInt, QAudioFormat::LittleEndian, 32 },
   { GST_AUDIO_FORMAT_S32BE, QAudioFormat::SignedInt, QAudioFormat::BigEndian, 32 },
   { GST_AUDIO_FORMAT_U32LE, QAudioFormat::UnSignedInt, QAudioFormat::LittleEndian, 32 },
   { GST_AUDIO_FORMAT_U32BE, QAudioFormat::UnSignedInt, QAudioFormat::BigEndian, 32 },
   { GST_AUDIO_FORMAT_S24LE, QAudioFormat::SignedInt, QAudioFormat::LittleEndian, 24 },
   { GST_AUDIO_FORMAT_S24BE, QAudioFormat::SignedInt, QAudioFormat::BigEndian, 24 },
   { GST_AUDIO_FORMAT_U24LE, QAudioFormat::UnSignedInt, QAudioFormat::LittleEndian, 24 },
   { GST_AUDIO_FORMAT_U24BE, QAudioFormat::UnSignedInt, QAudioFormat::BigEndian, 24 },
   { GST_AUDIO_FORMAT_F32LE, QAudioFormat::Float, QAudioFormat::LittleEndian, 32 },
   { GST_AUDIO_FORMAT_F32BE, QAudioFormat::Float, QAudioFormat::BigEndian, 32 },
   { GST_AUDIO_FORMAT_F64LE, QAudioFormat::Float, QAudioFormat::LittleEndian, 64 },
   { GST_AUDIO_FORMAT_F64BE, QAudioFormat::Float, QAudioFormat::BigEndian, 64 }
};

}
#endif

/*!
  Returns audio format for caps.
  If caps doesn't have a valid audio format, an empty QAudioFormat is returned.
*/

QAudioFormat QGstUtils::audioFormatForCaps(const GstCaps *caps)
{
   QAudioFormat format;
#if GST_CHECK_VERSION(1,0,0)
   GstAudioInfo info;
   if (gst_audio_info_from_caps(&info, caps)) {
      for (int i = 0; i < lengthOf(qt_audioLookup); ++i) {
         if (qt_audioLookup[i].format != info.finfo->format) {
            continue;
         }

         format.setSampleType(qt_audioLookup[i].sampleType);
         format.setByteOrder(qt_audioLookup[i].byteOrder);
         format.setSampleSize(qt_audioLookup[i].sampleSize);
         format.setSampleRate(info.rate);
         format.setChannelCount(info.channels);
         format.setCodec("audio/pcm");

         return format;
      }
   }
#else
   const GstStructure *structure = gst_caps_get_structure(caps, 0);

   if (qstrcmp(gst_structure_get_name(structure), "audio/x-raw-int") == 0) {

      format.setCodec("audio/pcm");

      int endianness = 0;
      gst_structure_get_int(structure, "endianness", &endianness);
      if (endianness == 1234) {
         format.setByteOrder(QAudioFormat::LittleEndian);
      } else if (endianness == 4321) {
         format.setByteOrder(QAudioFormat::BigEndian);
      }

      gboolean isSigned = FALSE;
      gst_structure_get_boolean(structure, "signed", &isSigned);
      if (isSigned) {
         format.setSampleType(QAudioFormat::SignedInt);
      } else {
         format.setSampleType(QAudioFormat::UnSignedInt);
      }

      // Number of bits allocated per sample.
      int width = 0;
      gst_structure_get_int(structure, "width", &width);

      // The number of bits used per sample. This must be less than or equal to the width.
      int depth = 0;
      gst_structure_get_int(structure, "depth", &depth);

      if (width != depth) {
         // Unsupported sample layout.
         return QAudioFormat();
      }
      format.setSampleSize(width);

      int rate = 0;
      gst_structure_get_int(structure, "rate", &rate);
      format.setSampleRate(rate);

      int channels = 0;
      gst_structure_get_int(structure, "channels", &channels);
      format.setChannelCount(channels);

   } else if (qstrcmp(gst_structure_get_name(structure), "audio/x-raw-float") == 0) {

      format.setCodec("audio/pcm");

      int endianness = 0;
      gst_structure_get_int(structure, "endianness", &endianness);
      if (endianness == 1234) {
         format.setByteOrder(QAudioFormat::LittleEndian);
      } else if (endianness == 4321) {
         format.setByteOrder(QAudioFormat::BigEndian);
      }

      format.setSampleType(QAudioFormat::Float);

      int width = 0;
      gst_structure_get_int(structure, "width", &width);

      format.setSampleSize(width);

      int rate = 0;
      gst_structure_get_int(structure, "rate", &rate);
      format.setSampleRate(rate);

      int channels = 0;
      gst_structure_get_int(structure, "channels", &channels);
      format.setChannelCount(channels);

   } else {
      return QAudioFormat();
   }
#endif
   return format;
}

#if GST_CHECK_VERSION(1,0,0)
/*!
  Returns audio format for a sample.
  If the buffer doesn't have a valid audio format, an empty QAudioFormat is returned.
*/
QAudioFormat QGstUtils::audioFormatForSample(GstSample *sample)
{
   GstCaps *caps = gst_sample_get_caps(sample);
   if (!caps) {
      return QAudioFormat();
   }

   return QGstUtils::audioFormatForCaps(caps);
}
#else
/*!
  Returns audio format for a buffer.
  If the buffer doesn't have a valid audio format, an empty QAudioFormat is returned.
*/
QAudioFormat QGstUtils::audioFormatForBuffer(GstBuffer *buffer)
{
   GstCaps *caps = gst_buffer_get_caps(buffer);
   if (!caps) {
      return QAudioFormat();
   }

   QAudioFormat format = QGstUtils::audioFormatForCaps(caps);
   gst_caps_unref(caps);
   return format;
}
#endif

/*!
  Builds GstCaps for an audio format.
  Returns 0 if the audio format is not valid.
  Caller must unref GstCaps.
*/

GstCaps *QGstUtils::capsForAudioFormat(const QAudioFormat &format)
{
   if (!format.isValid()) {
      return nullptr;
   }

#if GST_CHECK_VERSION(1,0,0)
   const QAudioFormat::SampleType sampleType = format.sampleType();
   const QAudioFormat::Endian byteOrder = format.byteOrder();
   const int sampleSize = format.sampleSize();

   for (int i = 0; i < lengthOf(qt_audioLookup); ++i) {
      if (qt_audioLookup[i].sampleType != sampleType
         || qt_audioLookup[i].byteOrder != byteOrder
         || qt_audioLookup[i].sampleSize != sampleSize) {
         continue;
      }

      return gst_caps_new_simple(
            "audio/x-raw",
            "format", G_TYPE_STRING, gst_audio_format_to_string(qt_audioLookup[i].format),
            "rate", G_TYPE_INT, format.sampleRate(),
            "channels", G_TYPE_INT, format.channelCount(),
            nullptr);
   }
   return nullptr;
#else
   GstStructure *structure = nullptr;

   if (format.isValid()) {
      if (format.sampleType() == QAudioFormat::SignedInt || format.sampleType() == QAudioFormat::UnSignedInt) {
         structure = gst_structure_new("audio/x-raw-int", nullptr);
      } else if (format.sampleType() == QAudioFormat::Float) {
         structure = gst_structure_new("audio/x-raw-float", nullptr);
      }
   }

   GstCaps *caps = nullptr;

   if (structure) {
      gst_structure_set(structure, "rate", G_TYPE_INT, format.sampleRate(), nullptr);
      gst_structure_set(structure, "channels", G_TYPE_INT, format.channelCount(), nullptr);
      gst_structure_set(structure, "width", G_TYPE_INT, format.sampleSize(), nullptr);
      gst_structure_set(structure, "depth", G_TYPE_INT, format.sampleSize(), nullptr);

      if (format.byteOrder() == QAudioFormat::LittleEndian) {
         gst_structure_set(structure, "endianness", G_TYPE_INT, 1234, nullptr);
      } else if (format.byteOrder() == QAudioFormat::BigEndian) {
         gst_structure_set(structure, "endianness", G_TYPE_INT, 4321, nullptr);
      }

      if (format.sampleType() == QAudioFormat::SignedInt) {
         gst_structure_set(structure, "signed", G_TYPE_BOOLEAN, TRUE, nullptr);
      } else if (format.sampleType() == QAudioFormat::UnSignedInt) {
         gst_structure_set(structure, "signed", G_TYPE_BOOLEAN, FALSE, nullptr);
      }

      caps = gst_caps_new_empty();
      Q_ASSERT(caps);
      gst_caps_append_structure(caps, structure);
   }

   return caps;
#endif
}

void QGstUtils::initializeGst()
{
   static bool initialized = false;
   if (!initialized) {
      initialized = true;
      gst_init(nullptr, nullptr);
   }
}

namespace {
const char *getCodecAlias(const QString &codec)
{
   if (codec.startsWith("avc1.")) {
      return "video/x-h264";
   }

   if (codec.startsWith("mp4a.")) {
      return "audio/mpeg4";
   }

   if (codec.startsWith("mp4v.20.")) {
      return "video/mpeg4";
   }

   if (codec == "samr") {
      return "audio/amr";
   }

   return nullptr;
}

const char *getMimeTypeAlias(const QString &mimeType)
{
   if (mimeType == "video/mp4") {
      return "video/mpeg4";
   }

   if (mimeType == "audio/mp4") {
      return "audio/mpeg4";
   }

   if (mimeType == "video/ogg"
      || mimeType == "audio/ogg") {
      return "application/ogg";
   }

   return nullptr;
}
}

QMultimedia::SupportEstimate QGstUtils::hasSupport(const QString &mimeType, const QStringList &codecs,
   const QSet<QString> &supportedMimeTypeSet)
{
   if (supportedMimeTypeSet.isEmpty()) {
      return QMultimedia::NotSupported;
   }

   QString mimeTypeLowcase = mimeType.toLower();
   bool containsMimeType = supportedMimeTypeSet.contains(mimeTypeLowcase);

   if (! containsMimeType) {
      const char *mimeTypeAlias = getMimeTypeAlias(mimeTypeLowcase);
      containsMimeType = supportedMimeTypeSet.contains(QString::fromLatin1(mimeTypeAlias));

      if (! containsMimeType) {
         containsMimeType = supportedMimeTypeSet.contains("video/" + mimeTypeLowcase)
            || supportedMimeTypeSet.contains("video/x-" + mimeTypeLowcase)
            || supportedMimeTypeSet.contains("audio/" + mimeTypeLowcase)
            || supportedMimeTypeSet.contains("audio/x-" + mimeTypeLowcase);
      }
   }

   int supportedCodecCount = 0;

   for (const QString &codec : codecs) {
      QString codecLowcase = codec.toLower();
      const char *codecAlias = getCodecAlias(codecLowcase);

      if (codecAlias) {
         if (supportedMimeTypeSet.contains(QString::fromLatin1(codecAlias))) {
            supportedCodecCount++;
         }

      } else if (supportedMimeTypeSet.contains("video/" + codecLowcase)
         || supportedMimeTypeSet.contains("video/x-" + codecLowcase)
         || supportedMimeTypeSet.contains("audio/" + codecLowcase)
         || supportedMimeTypeSet.contains("audio/x-" + codecLowcase)) {
         supportedCodecCount++;
      }
   }

   if (supportedCodecCount > 0 && supportedCodecCount == codecs.size()) {
      return QMultimedia::ProbablySupported;
   }

   if (supportedCodecCount == 0 && !containsMimeType) {
      return QMultimedia::NotSupported;
   }

   return QMultimedia::MaybeSupported;
}

namespace {

using FactoryCameraInfoMap = QHash<GstElementFactory *, QVector<QGstUtils::CameraInfo>>;

static FactoryCameraInfoMap *qt_camera_device_info()
{
   static FactoryCameraInfoMap retval;
   return &retval;
}

}

QVector<QGstUtils::CameraInfo> QGstUtils::enumerateCameras(GstElementFactory *factory)
{
   static QElapsedTimer camerasCacheAgeTimer;
   if (camerasCacheAgeTimer.isValid() && camerasCacheAgeTimer.elapsed() > 500) {
      // ms
      qt_camera_device_info()->clear();
   }

   FactoryCameraInfoMap::const_iterator it = qt_camera_device_info()->constFind(factory);
   if (it != qt_camera_device_info()->constEnd()) {
      return *it;
   }

   QVector<CameraInfo> &devices = (*qt_camera_device_info())[factory];

   if (factory) {
      bool hasVideoSource = false;

      const GType type = gst_element_factory_get_element_type(factory);

      GObjectClass *const objectClass = type ? static_cast<GObjectClass *>(g_type_class_ref(type)) : nullptr;

      if (objectClass) {
         if (g_object_class_find_property(objectClass, "camera-device")) {
            const CameraInfo primary = {
               "primary",
               QGstreamerVideoInputDeviceControl::primaryCamera(),
               0,
               QCamera::BackFace,
               QByteArray()
            };

            const CameraInfo secondary = {
               "secondary",
               QGstreamerVideoInputDeviceControl::secondaryCamera(),
               0,
               QCamera::FrontFace,
               QByteArray()
            };

            devices.append(primary);
            devices.append(secondary);

            GstElement *camera = g_object_class_find_property(objectClass, "sensor-mount-angle")
               ? gst_element_factory_create(factory, nullptr) : nullptr;

            if (camera) {
               if (gst_element_set_state(camera, GST_STATE_READY) != GST_STATE_CHANGE_SUCCESS) {
                  // no-op
               } else
                  for (int i = 0; i < 2; ++i) {
                     gint orientation = 0;
                     g_object_set(G_OBJECT(camera), "camera-device", i, nullptr);
                     g_object_get(G_OBJECT(camera), "sensor-mount-angle", &orientation, nullptr);

                     devices[i].orientation = (720 - orientation) % 360;
                  }

               gst_element_set_state(camera, GST_STATE_NULL);
               gst_object_unref(GST_OBJECT(camera));
            }

         } else if (g_object_class_find_property(objectClass, "video-source")) {
            hasVideoSource = true;
         }

         g_type_class_unref(objectClass);
      }

      if (!devices.isEmpty() || ! hasVideoSource) {
         camerasCacheAgeTimer.restart();
         return devices;
      }
   }

#ifdef USE_V4L
   QDir devDir("/dev");
   devDir.setFilter(QDir::System);

   QFileInfoList entries = devDir.entryInfoList(QStringList() << "video*");

   for (const QFileInfo &entryInfo : entries) {
      // qDebug() << "Try" << entryInfo.filePath();

      int fd = qt_safe_open(entryInfo.filePath().toLatin1().constData(), O_RDWR );
      if (fd == -1) {
         continue;
      }

      bool isCamera = false;

      v4l2_input input;
      memset(&input, 0, sizeof(input));
      for (; ::ioctl(fd, VIDIOC_ENUMINPUT, &input) >= 0; ++input.index) {
         if (input.type == V4L2_INPUT_TYPE_CAMERA || input.type == 0) {
            isCamera = ::ioctl(fd, VIDIOC_S_INPUT, input.index) != 0;
            break;
         }
      }

      if (isCamera) {
         // find out its driver "name"
         QByteArray driver;
         QString name;
         struct v4l2_capability vcap;
         memset(&vcap, 0, sizeof(struct v4l2_capability));

         if (ioctl(fd, VIDIOC_QUERYCAP, &vcap) != 0) {
            name = entryInfo.fileName();

         } else {
            driver = QByteArray((const char *)vcap.driver);
            name   = QString::fromUtf8((const char *)vcap.card);

            if (name.isEmpty()) {
               name = entryInfo.fileName();
            }
         }
         //qDebug() << "found camera: " << name;


         CameraInfo device = {
            entryInfo.absoluteFilePath(),
            name,
            0,
            QCamera::UnspecifiedPosition,
            driver
         };
         devices.append(device);
      }
      qt_safe_close(fd);
   }
   camerasCacheAgeTimer.restart();
#endif // USE_V4L

   return devices;
}

QList<QString> QGstUtils::cameraDevices(GstElementFactory *factory)
{
   QList<QString> devices;

   for (const CameraInfo &camera : enumerateCameras(factory)) {
      devices.append(camera.name);
   }

   return devices;
}

QString QGstUtils::cameraDescription(const QString &device, GstElementFactory *factory)
{
   for (const CameraInfo &camera : enumerateCameras(factory)) {
      if (camera.name == device) {
         return camera.description;
      }
   }

   return QString();
}

QCamera::Position QGstUtils::cameraPosition(const QString &device, GstElementFactory *factory)
{
   for (const CameraInfo &camera : enumerateCameras(factory)) {
      if (camera.name == device) {
         return camera.position;
      }
   }
   return QCamera::UnspecifiedPosition;
}

int QGstUtils::cameraOrientation(const QString &device, GstElementFactory *factory)
{
   for (const CameraInfo &camera : enumerateCameras(factory)) {
      if (camera.name == device) {
         return camera.orientation;
      }
   }

   return 0;
}

QByteArray QGstUtils::cameraDriver(const QString &device, GstElementFactory *factory)
{
   for (const CameraInfo &camera : enumerateCameras(factory)) {
      if (camera.name == device) {
         return camera.driver;
      }
   }

   return QByteArray();
}

QSet<QString> QGstUtils::supportedMimeTypes(bool (*isValidFactory)(GstElementFactory *factory))
{
   QSet<QString> supportedMimeTypes;

   //enumerate supported mime types
   gst_init(nullptr, nullptr);

#if GST_CHECK_VERSION(1,0,0)
   GstRegistry *registry = gst_registry_get();
   GList *orig_plugins = gst_registry_get_plugin_list(registry);
#else
   GstRegistry *registry = gst_registry_get_default();
   GList *orig_plugins = gst_default_registry_get_plugin_list ();
#endif

   for (GList *plugins = orig_plugins; plugins; plugins = g_list_next(plugins)) {
      GstPlugin *plugin = (GstPlugin *) (plugins->data);

#if GST_CHECK_VERSION(1,0,0)
      if (GST_OBJECT_FLAG_IS_SET(GST_OBJECT(plugin), GST_PLUGIN_FLAG_BLACKLISTED)) {
         continue;
      }
#else
      if (plugin->flags & (1 << 1)) { //GST_PLUGIN_FLAG_BLACKLISTED
         continue;
      }
#endif

      GList *orig_features = gst_registry_get_feature_list_by_plugin(
            registry, gst_plugin_get_name(plugin));

      for (GList *features = orig_features; features; features = g_list_next(features)) {
         if (features->data == nullptr) {
            continue;
         }

         GstPluginFeature *feature = GST_PLUGIN_FEATURE(features->data);
         GstElementFactory *factory;

         if (GST_IS_TYPE_FIND_FACTORY(feature)) {
            QString name(QString::fromLatin1(gst_plugin_feature_get_name(feature)));

            if (name.contains('/')) {
               //filter out any string without '/' which is obviously not a mime type
               supportedMimeTypes.insert(name.toLower());
            }
            continue;

         } else if (!GST_IS_ELEMENT_FACTORY (feature)
            || ! (factory = GST_ELEMENT_FACTORY(gst_plugin_feature_load(feature)))) {
            continue;

         } else if (!isValidFactory(factory)) {
            // Do nothing

         } else

            for (const GList *pads = gst_element_factory_get_static_pad_templates(factory); pads; pads = g_list_next(pads)) {

               GstStaticPadTemplate *padtemplate = static_cast<GstStaticPadTemplate *>(pads->data);

               if (padtemplate->direction == GST_PAD_SINK && padtemplate->static_caps.string) {
                  GstCaps *caps = gst_static_caps_get(&padtemplate->static_caps);

                  if (gst_caps_is_any(caps) || gst_caps_is_empty(caps)) {
                     //

                  } else
                     for (guint i = 0; i < gst_caps_get_size(caps); i++) {
                        GstStructure *structure = gst_caps_get_structure(caps, i);

                        QString nameLowcase = QString::fromLatin1(gst_structure_get_name(structure)).toLower();

                        supportedMimeTypes.insert(nameLowcase);

                        if (nameLowcase.contains("mpeg")) {
                           // Because mpeg version number is only included in the detail description,
                           // it is necessary to manually extract this information in order to match
                           // the mime type of mpeg4.

                           const GValue *value = gst_structure_get_value(structure, "mpegversion");

                           if (value) {
                              gchar *str = gst_value_serialize(value);

                              QString versions( QString::fromLatin1(str));
                              QStringList elements = versions.split(QRegularExpression("\\D+"), QStringParser::SkipEmptyParts);

                              for (const QString &e : elements) {
                                 supportedMimeTypes.insert(nameLowcase + e);
                              }

                              g_free(str);
                           }
                        }
                     }
               }
            }
         gst_object_unref(factory);
      }
      gst_plugin_feature_list_free(orig_features);
   }

   gst_plugin_list_free (orig_plugins);

   return supportedMimeTypes;
}

#if GST_CHECK_VERSION(1, 0, 0)
namespace {

struct ColorFormat {
   QImage::Format imageFormat;
   GstVideoFormat gstFormat;
};

static const ColorFormat qt_colorLookup[] = {
   { QImage::Format_RGBX8888, GST_VIDEO_FORMAT_RGBx  },
   { QImage::Format_RGBA8888, GST_VIDEO_FORMAT_RGBA  },
   { QImage::Format_RGB888,   GST_VIDEO_FORMAT_RGB   },
   { QImage::Format_RGB16,    GST_VIDEO_FORMAT_RGB16 }
};

}
#endif

#if GST_CHECK_VERSION(1,0,0)
QImage QGstUtils::bufferToImage(GstBuffer *buffer, const GstVideoInfo &videoInfo)
#else
QImage QGstUtils::bufferToImage(GstBuffer *buffer)
#endif
{
   QImage img;

#if GST_CHECK_VERSION(1,0,0)
   GstVideoInfo info = videoInfo;
   GstVideoFrame frame;
   if (!gst_video_frame_map(&frame, &info, buffer, GST_MAP_READ)) {
      return img;
   }
#else
   GstCaps *caps = gst_buffer_get_caps(buffer);
   if (!caps) {
      return img;
   }

   GstStructure *structure = gst_caps_get_structure (caps, 0);
   gint width = 0;
   gint height = 0;

   if (!structure
      || !gst_structure_get_int(structure, "width", &width)
      || !gst_structure_get_int(structure, "height", &height)
      || width <= 0
      || height <= 0) {
      gst_caps_unref(caps);
      return img;
   }
   gst_caps_unref(caps);
#endif

#if GST_CHECK_VERSION(1,0,0)
   if (videoInfo.finfo->format == GST_VIDEO_FORMAT_I420) {
      const int width = videoInfo.width;
      const int height = videoInfo.height;

      const int stride[] = { frame.info.stride[0], frame.info.stride[1], frame.info.stride[2] };
      const uchar *data[] = {
         static_cast<const uchar *>(frame.data[0]),
         static_cast<const uchar *>(frame.data[1]),
         static_cast<const uchar *>(frame.data[2])
      };
#else
   if (qstrcmp(gst_structure_get_name(structure), "video/x-raw-yuv") == 0) {
      const int stride[] = { width, width / 2, width / 2 };
      const uchar *data[] = {
         (const uchar *)buffer->data,
         (const uchar *)buffer->data + width * height,
         (const uchar *)buffer->data + width *height * 5 / 4
      };
#endif
      img = QImage(width / 2, height / 2, QImage::Format_RGB32);

      for (int y = 0; y < height; y += 2) {
         const uchar *yLine = data[0] + (y * stride[0]);
         const uchar *uLine = data[1] + (y * stride[1] / 2);
         const uchar *vLine = data[2] + (y * stride[2] / 2);

         for (int x = 0; x < width; x += 2) {
            const qreal Y = 1.164 * (yLine[x] - 16);
            const int U = uLine[x / 2] - 128;
            const int V = vLine[x / 2] - 128;

            int b = qBound(0, int(Y + 2.018 * U), 255);
            int g = qBound(0, int(Y - 0.813 * V - 0.391 * U), 255);
            int r = qBound(0, int(Y + 1.596 * V), 255);

            img.setPixel(x / 2, y / 2, qRgb(r, g, b));
         }
      }

#if GST_CHECK_VERSION(1,0,0)
   } else
      for (int i = 0; i < lengthOf(qt_colorLookup); ++i) {
         if (qt_colorLookup[i].gstFormat != videoInfo.finfo->format) {
            continue;
         }

         const QImage image(
            static_cast<const uchar *>(frame.data[0]),
            videoInfo.width,
            videoInfo.height,
            frame.info.stride[0],
            qt_colorLookup[i].imageFormat);
         img = image;
         img.detach();

         break;
      }

   gst_video_frame_unmap(&frame);

#else
   } else if (qstrcmp(gst_structure_get_name(structure), "video/x-raw-rgb") == 0)
   {
      QImage::Format format = QImage::Format_Invalid;
      int bpp = 0;
      gst_structure_get_int(structure, "bpp", &bpp);

      if (bpp == 24) {
         format = QImage::Format_RGB888;
      } else if (bpp == 32) {
         format = QImage::Format_RGB32;
      }

      if (format != QImage::Format_Invalid) {
         img = QImage((const uchar *)buffer->data,
               width,
               height,
               format);
         img.bits(); //detach
      }
   }
#endif
   return img;
}


namespace {

#if GST_CHECK_VERSION(1,0,0)

struct VideoFormat {
   QVideoFrame::PixelFormat pixelFormat;
   GstVideoFormat gstFormat;
};

static const VideoFormat qt_videoFormatLookup[] = {
   { QVideoFrame::Format_YUV420P, GST_VIDEO_FORMAT_I420 },
   { QVideoFrame::Format_YV12, GST_VIDEO_FORMAT_YV12 },
   { QVideoFrame::Format_UYVY, GST_VIDEO_FORMAT_UYVY },
   { QVideoFrame::Format_YUYV, GST_VIDEO_FORMAT_YUY2 },
   { QVideoFrame::Format_NV12, GST_VIDEO_FORMAT_NV12 },
   { QVideoFrame::Format_NV21, GST_VIDEO_FORMAT_NV21 },
   { QVideoFrame::Format_AYUV444, GST_VIDEO_FORMAT_AYUV },
#if Q_BYTE_ORDER == Q_LITTLE_ENDIAN
   { QVideoFrame::Format_RGB32,  GST_VIDEO_FORMAT_BGRx },
   { QVideoFrame::Format_BGR32,  GST_VIDEO_FORMAT_RGBx },
   { QVideoFrame::Format_ARGB32,  GST_VIDEO_FORMAT_BGRA },
   { QVideoFrame::Format_BGRA32,  GST_VIDEO_FORMAT_ARGB },
#else
   { QVideoFrame::Format_RGB32,  GST_VIDEO_FORMAT_xRGB },
   { QVideoFrame::Format_BGR32,  GST_VIDEO_FORMAT_xBGR },
   { QVideoFrame::Format_ARGB32,  GST_VIDEO_FORMAT_ARGB },
   { QVideoFrame::Format_BGRA32,  GST_VIDEO_FORMAT_BGRA },
#endif
   { QVideoFrame::Format_RGB24,  GST_VIDEO_FORMAT_RGB },
   { QVideoFrame::Format_BGR24,  GST_VIDEO_FORMAT_BGR },
   { QVideoFrame::Format_RGB565,  GST_VIDEO_FORMAT_RGB16 }
};

static int indexOfVideoFormat(QVideoFrame::PixelFormat format)
{
   for (int i = 0; i < lengthOf(qt_videoFormatLookup); ++i)
      if (qt_videoFormatLookup[i].pixelFormat == format) {
         return i;
      }

   return -1;
}

static int indexOfVideoFormat(GstVideoFormat format)
{
   for (int i = 0; i < lengthOf(qt_videoFormatLookup); ++i)
      if (qt_videoFormatLookup[i].gstFormat == format) {
         return i;
      }

   return -1;
}

#else

struct YuvFormat {
   QVideoFrame::PixelFormat pixelFormat;
   guint32 fourcc;
   int bitsPerPixel;
};

static const YuvFormat qt_yuvColorLookup[] = {
   { QVideoFrame::Format_YUV420P, GST_MAKE_FOURCC('I', '4', '2', '0'), 8 },
   { QVideoFrame::Format_YV12,    GST_MAKE_FOURCC('Y', 'V', '1', '2'), 8 },
   { QVideoFrame::Format_UYVY,    GST_MAKE_FOURCC('U', 'Y', 'V', 'Y'), 16 },
   { QVideoFrame::Format_YUYV,    GST_MAKE_FOURCC('Y', 'U', 'Y', '2'), 16 },
   { QVideoFrame::Format_NV12,    GST_MAKE_FOURCC('N', 'V', '1', '2'), 8 },
   { QVideoFrame::Format_NV21,    GST_MAKE_FOURCC('N', 'V', '2', '1'), 8 },
   { QVideoFrame::Format_AYUV444, GST_MAKE_FOURCC('A', 'Y', 'U', 'V'), 32 }
};

static int indexOfYuvColor(QVideoFrame::PixelFormat format)
{
   const int count = sizeof(qt_yuvColorLookup) / sizeof(YuvFormat);

   for (int i = 0; i < count; ++i)
      if (qt_yuvColorLookup[i].pixelFormat == format) {
         return i;
      }

   return -1;
}

static int indexOfYuvColor(guint32 fourcc)
{
   const int count = sizeof(qt_yuvColorLookup) / sizeof(YuvFormat);

   for (int i = 0; i < count; ++i)
      if (qt_yuvColorLookup[i].fourcc == fourcc) {
         return i;
      }

   return -1;
}

struct RgbFormat {
   QVideoFrame::PixelFormat pixelFormat;
   int bitsPerPixel;
   int depth;
   int endianness;
   int red;
   int green;
   int blue;
   int alpha;
};

static const RgbFormat qt_rgbColorLookup[] = {
   { QVideoFrame::Format_RGB32, 32, 24, 4321, 0x0000FF00, 0x00FF0000, int(0xFF000000), 0x00000000 },
   { QVideoFrame::Format_RGB32, 32, 24, 1234, 0x00FF0000, 0x0000FF00, 0x000000FF, 0x00000000 },
   { QVideoFrame::Format_BGR32, 32, 24, 4321, int(0xFF000000), 0x00FF0000, 0x0000FF00, 0x00000000 },
   { QVideoFrame::Format_BGR32, 32, 24, 1234, 0x000000FF, 0x0000FF00, 0x00FF0000, 0x00000000 },
   { QVideoFrame::Format_ARGB32, 32, 24, 4321, 0x0000FF00, 0x00FF0000, int(0xFF000000), 0x000000FF },
   { QVideoFrame::Format_ARGB32, 32, 24, 1234, 0x00FF0000, 0x0000FF00, 0x000000FF, int(0xFF000000) },
   { QVideoFrame::Format_RGB24, 24, 24, 4321, 0x00FF0000, 0x0000FF00, 0x000000FF, 0x00000000 },
   { QVideoFrame::Format_BGR24, 24, 24, 4321, 0x000000FF, 0x0000FF00, 0x00FF0000, 0x00000000 },
   { QVideoFrame::Format_RGB565, 16, 16, 1234, 0x0000F800, 0x000007E0, 0x0000001F, 0x00000000 }
};

static int indexOfRgbColor(
   int bits, int depth, int endianness, int red, int green, int blue, int alpha)
{
   const int count = sizeof(qt_rgbColorLookup) / sizeof(RgbFormat);

   for (int i = 0; i < count; ++i) {
      if (qt_rgbColorLookup[i].bitsPerPixel == bits
         && qt_rgbColorLookup[i].depth == depth
         && qt_rgbColorLookup[i].endianness == endianness
         && qt_rgbColorLookup[i].red == red
         && qt_rgbColorLookup[i].green == green
         && qt_rgbColorLookup[i].blue == blue
         && qt_rgbColorLookup[i].alpha == alpha) {
         return i;
      }
   }
   return -1;
}
#endif

}

#if GST_CHECK_VERSION(1,0,0)

QVideoSurfaceFormat QGstUtils::formatForCaps(
   GstCaps *caps, GstVideoInfo *info, QAbstractVideoBuffer::HandleType handleType)
{
   GstVideoInfo vidInfo;
   GstVideoInfo *infoPtr = info ? info : &vidInfo;

   if (gst_video_info_from_caps(infoPtr, caps)) {
      int index = indexOfVideoFormat(infoPtr->finfo->format);

      if (index != -1) {
         QVideoSurfaceFormat format(
            QSize(infoPtr->width, infoPtr->height),
            qt_videoFormatLookup[index].pixelFormat,
            handleType);

         if (infoPtr->fps_d > 0) {
            format.setFrameRate(qreal(infoPtr->fps_n) / infoPtr->fps_d);
         }

         if (infoPtr->par_d > 0) {
            format.setPixelAspectRatio(infoPtr->par_n, infoPtr->par_d);
         }

         return format;
      }
   }
   return QVideoSurfaceFormat();
}

#else

QVideoSurfaceFormat QGstUtils::formatForCaps(
   GstCaps *caps, int *bytesPerLine, QAbstractVideoBuffer::HandleType handleType)
{
   const GstStructure *structure = gst_caps_get_structure(caps, 0);

   int bitsPerPixel = 0;
   QSize size = structureResolution(structure);
   QVideoFrame::PixelFormat pixelFormat = structurePixelFormat(structure, &bitsPerPixel);

   if (pixelFormat != QVideoFrame::Format_Invalid) {
      QVideoSurfaceFormat format(size, pixelFormat, handleType);

      QPair<qreal, qreal> rate = structureFrameRateRange(structure);
      if (rate.second) {
         format.setFrameRate(rate.second);
      }

      format.setPixelAspectRatio(structurePixelAspectRatio(structure));

      if (bytesPerLine) {
         *bytesPerLine = ((size.width() * bitsPerPixel / 8) + 3) & ~3;
      }

      return format;
   }
   return QVideoSurfaceFormat();
}

#endif

GstCaps *QGstUtils::capsForFormats(const QList<QVideoFrame::PixelFormat> &formats)
{
   GstCaps *caps = gst_caps_new_empty();

#if GST_CHECK_VERSION(1,0,0)
   for (QVideoFrame::PixelFormat format : formats) {
      int index = indexOfVideoFormat(format);

      if (index != -1) {
         gst_caps_append_structure(caps, gst_structure_new(
               "video/x-raw",
               "format", G_TYPE_STRING, gst_video_format_to_string(qt_videoFormatLookup[index].gstFormat),
               nullptr));
      }
   }
#else
   for (QVideoFrame::PixelFormat format : formats) {
      int index = indexOfYuvColor(format);

      if (index != -1) {
         gst_caps_append_structure(caps, gst_structure_new("video/x-raw-yuv", "format",
               GST_TYPE_FOURCC, qt_yuvColorLookup[index].fourcc, nullptr));
         continue;
      }

      const int count = sizeof(qt_rgbColorLookup) / sizeof(RgbFormat);

      for (int i = 0; i < count; ++i) {
         if (qt_rgbColorLookup[i].pixelFormat == format) {
            GstStructure *structure = gst_structure_new(
                  "video/x-raw-rgb",
                  "bpp", G_TYPE_INT, qt_rgbColorLookup[i].bitsPerPixel,
                  "depth", G_TYPE_INT, qt_rgbColorLookup[i].depth,
                  "endianness", G_TYPE_INT, qt_rgbColorLookup[i].endianness,
                  "red_mask", G_TYPE_INT, qt_rgbColorLookup[i].red,
                  "green_mask", G_TYPE_INT, qt_rgbColorLookup[i].green,
                  "blue_mask", G_TYPE_INT, qt_rgbColorLookup[i].blue,
                  nullptr);

            if (qt_rgbColorLookup[i].alpha != 0) {
               gst_structure_set(
                  structure, "alpha_mask", G_TYPE_INT, qt_rgbColorLookup[i].alpha, nullptr);
            }
            gst_caps_append_structure(caps, structure);
         }
      }
   }
#endif

   gst_caps_set_simple(caps, "framerate", GST_TYPE_FRACTION_RANGE, 0, 1, INT_MAX, 1,
      "width", GST_TYPE_INT_RANGE, 1, INT_MAX, "height", GST_TYPE_INT_RANGE, 1, INT_MAX, nullptr);

   return caps;
}

void QGstUtils::setFrameTimeStamps(QVideoFrame *frame, GstBuffer *buffer)
{
   // GStreamer uses nanoseconds, Qt uses microseconds
   qint64 startTime = GST_BUFFER_TIMESTAMP(buffer);

   if (startTime >= 0) {
      frame->setStartTime(startTime / G_GINT64_CONSTANT (1000));

      qint64 duration = GST_BUFFER_DURATION(buffer);
      if (duration >= 0) {
         frame->setEndTime((startTime + duration) / G_GINT64_CONSTANT (1000));
      }
   }
}

void QGstUtils::setMetaData(GstElement *element, const QMap<QByteArray, QVariant> &data)
{
   if (!GST_IS_TAG_SETTER(element)) {
      return;
   }

   gst_tag_setter_reset_tags(GST_TAG_SETTER(element));

   QMapIterator<QByteArray, QVariant> it(data);
   while (it.hasNext()) {
      it.next();
      const QString tagName = it.key();
      const QVariant tagValue = it.value();

      switch (tagValue.type()) {
         case QVariant::String:
            gst_tag_setter_add_tags(GST_TAG_SETTER(element),
               GST_TAG_MERGE_REPLACE,
               tagName.toUtf8().constData(),
               tagValue.toString().toUtf8().constData(),
               nullptr);
            break;

         case QVariant::Int:
         case QVariant::LongLong:
            gst_tag_setter_add_tags(GST_TAG_SETTER(element),
               GST_TAG_MERGE_REPLACE,
               tagName.toUtf8().constData(),
               tagValue.toInt(),
               nullptr);
            break;

         case QVariant::Double:
            gst_tag_setter_add_tags(GST_TAG_SETTER(element),
               GST_TAG_MERGE_REPLACE,
               tagName.toUtf8().constData(),
               tagValue.toDouble(),
               nullptr);
            break;

#if GST_CHECK_VERSION(0, 10, 31)
         case QVariant::DateTime: {
            QDateTime date = tagValue.toDateTime().toLocalTime();
            gst_tag_setter_add_tags(GST_TAG_SETTER(element),
               GST_TAG_MERGE_REPLACE,
               tagName.toUtf8().constData(),
               gst_date_time_new_local_time(
                  date.date().year(), date.date().month(), date.date().day(),
                  date.time().hour(), date.time().minute(), date.time().second()),
               nullptr);
            break;
         }
#endif
         default:
            break;
      }
   }
}

void QGstUtils::setMetaData(GstBin *bin, const QMap<QByteArray, QVariant> &data)
{
   GstIterator *elements = gst_bin_iterate_all_by_interface(bin, GST_TYPE_TAG_SETTER);

#if GST_CHECK_VERSION(1,0,0)
   GValue item = G_VALUE_INIT;
   while (gst_iterator_next(elements, &item) == GST_ITERATOR_OK) {
      GstElement *const element = GST_ELEMENT(g_value_get_object(&item));
#else
   GstElement *element = nullptr;
   while (gst_iterator_next(elements, (void **)&element) == GST_ITERATOR_OK) {
#endif

      setMetaData(element, data);
   }
   gst_iterator_free(elements);
}


GstCaps *QGstUtils::videoFilterCaps()
{
   static GstStaticCaps staticCaps = GST_STATIC_CAPS(
#if GST_CHECK_VERSION(1,2,0)
         "video/x-raw(ANY);"
#elif GST_CHECK_VERSION(1,0,0)
         "video/x-raw;"
#else
         "video/x-raw-yuv;"
         "video/x-raw-rgb;"
         "video/x-raw-data;"
         "video/x-android-buffer;"
#endif
         "image/jpeg;"
         "video/x-h264");

   return gst_caps_make_writable(gst_static_caps_get(&staticCaps));
}

QSize QGstUtils::structureResolution(const GstStructure *s)
{
   QSize size;

   int w, h;
   if (s && gst_structure_get_int(s, "width", &w) && gst_structure_get_int(s, "height", &h)) {
      size.rwidth() = w;
      size.rheight() = h;
   }

   return size;
}

QVideoFrame::PixelFormat QGstUtils::structurePixelFormat(const GstStructure *structure, int *bpp)
{
   QVideoFrame::PixelFormat pixelFormat = QVideoFrame::Format_Invalid;

   if (!structure) {
      return pixelFormat;
   }

#if GST_CHECK_VERSION(1,0,0)
   (void) bpp;

   if (gst_structure_has_name(structure, "video/x-raw")) {
      const gchar *s = gst_structure_get_string(structure, "format");
      if (s) {
         GstVideoFormat format = gst_video_format_from_string(s);
         int index = indexOfVideoFormat(format);

         if (index != -1) {
            pixelFormat = qt_videoFormatLookup[index].pixelFormat;
         }
      }
   }
#else
   if (qstrcmp(gst_structure_get_name(structure), "video/x-raw-yuv") == 0) {
      guint32 fourcc = 0;
      gst_structure_get_fourcc(structure, "format", &fourcc);

      int index = indexOfYuvColor(fourcc);
      if (index != -1) {
         pixelFormat = qt_yuvColorLookup[index].pixelFormat;
         if (bpp) {
            *bpp = qt_yuvColorLookup[index].bitsPerPixel;
         }
      }
   } else if (qstrcmp(gst_structure_get_name(structure), "video/x-raw-rgb") == 0) {
      int bitsPerPixel = 0;
      int depth = 0;
      int endianness = 0;
      int red = 0;
      int green = 0;
      int blue = 0;
      int alpha = 0;

      gst_structure_get_int(structure, "bpp", &bitsPerPixel);
      gst_structure_get_int(structure, "depth", &depth);
      gst_structure_get_int(structure, "endianness", &endianness);
      gst_structure_get_int(structure, "red_mask", &red);
      gst_structure_get_int(structure, "green_mask", &green);
      gst_structure_get_int(structure, "blue_mask", &blue);
      gst_structure_get_int(structure, "alpha_mask", &alpha);

      int index = indexOfRgbColor(bitsPerPixel, depth, endianness, red, green, blue, alpha);

      if (index != -1) {
         pixelFormat = qt_rgbColorLookup[index].pixelFormat;
         if (bpp) {
            *bpp = qt_rgbColorLookup[index].bitsPerPixel;
         }
      }
   }
#endif

   return pixelFormat;
}

QSize QGstUtils::structurePixelAspectRatio(const GstStructure *s)
{
   QSize ratio(1, 1);

   gint aspectNum = 0;
   gint aspectDenum = 0;
   if (s && gst_structure_get_fraction(s, "pixel-aspect-ratio", &aspectNum, &aspectDenum)) {
      if (aspectDenum > 0) {
         ratio.rwidth() = aspectNum;
         ratio.rheight() = aspectDenum;
      }
   }

   return ratio;
}

QPair<qreal, qreal> QGstUtils::structureFrameRateRange(const GstStructure *s)
{
   QPair<qreal, qreal> rate;

   if (!s) {
      return rate;
   }

   int n, d;
   if (gst_structure_get_fraction(s, "framerate", &n, &d)) {
      rate.second = qreal(n) / d;
      rate.first = rate.second;
   } else if (gst_structure_get_fraction(s, "max-framerate", &n, &d)) {
      rate.second = qreal(n) / d;
      if (gst_structure_get_fraction(s, "min-framerate", &n, &d)) {
         rate.first = qreal(n) / d;
      } else {
         rate.first = qreal(1);
      }
   }

   return rate;
}

void qt_gst_object_ref_sink(gpointer object)
{
#if GST_CHECK_VERSION(0,10,24)
   gst_object_ref_sink(object);
#else
   g_return_if_fail (GST_IS_OBJECT(object));

   GST_OBJECT_LOCK(object);
   if (G_LIKELY(GST_OBJECT_IS_FLOATING(object))) {
      GST_OBJECT_FLAG_UNSET(object, GST_OBJECT_FLOATING);
      GST_OBJECT_UNLOCK(object);
   } else {
      GST_OBJECT_UNLOCK(object);
      gst_object_ref(object);
   }
#endif
}

GstCaps *qt_gst_pad_get_current_caps(GstPad *pad)
{
#if GST_CHECK_VERSION(1,0,0)
   return gst_pad_get_current_caps(pad);
#else
   return gst_pad_get_negotiated_caps(pad);
#endif
}

GstCaps *qt_gst_pad_get_caps(GstPad *pad)
{
#if GST_CHECK_VERSION(1,0,0)
   return gst_pad_query_caps(pad, nullptr);
#elif GST_CHECK_VERSION(0, 10, 26)
   return gst_pad_get_caps_reffed(pad);
#else
   return gst_pad_get_caps(pad);
#endif
}

GstStructure *qt_gst_structure_new_empty(const char *name)
{
#if GST_CHECK_VERSION(1,0,0)
   return gst_structure_new_empty(name);
#else
   return gst_structure_new(name, nullptr);
#endif
}

gboolean qt_gst_element_query_position(GstElement *element, GstFormat format, gint64 *cur)
{
#if GST_CHECK_VERSION(1,0,0)
   return gst_element_query_position(element, format, cur);
#else
   return gst_element_query_position(element, &format, cur);
#endif
}

gboolean qt_gst_element_query_duration(GstElement *element, GstFormat format, gint64 *cur)
{
#if GST_CHECK_VERSION(1,0,0)
   return gst_element_query_duration(element, format, cur);
#else
   return gst_element_query_duration(element, &format, cur);
#endif
}

GstCaps *qt_gst_caps_normalize(GstCaps *caps)
{
#if GST_CHECK_VERSION(1,0,0)
   // gst_caps_normalize() takes ownership of the argument in 1.0
   return gst_caps_normalize(caps);
#else
   // in 0.10, it doesn't. Unref the argument to mimic the 1.0 behavior
   GstCaps *res = gst_caps_normalize(caps);
   gst_caps_unref(caps);
   return res;
#endif
}

const gchar *qt_gst_element_get_factory_name(GstElement *element)
{
   const gchar *name = nullptr;
   const GstElementFactory *factory = nullptr;

   if (element && (factory = gst_element_get_factory(element))) {
      name = gst_plugin_feature_get_name(GST_PLUGIN_FEATURE(factory));
   }

   return name;
}

gboolean qt_gst_caps_can_intersect(const GstCaps *caps1, const GstCaps *caps2)
{
#if GST_CHECK_VERSION(0, 10, 25)
   return gst_caps_can_intersect(caps1, caps2);
#else
   GstCaps *intersection = gst_caps_intersect(caps1, caps2);
   gboolean res = !gst_caps_is_empty(intersection);
   gst_caps_unref(intersection);
   return res;
#endif
}

#if !GST_CHECK_VERSION(0, 10, 31)
static gboolean qt_gst_videosink_factory_filter(GstPluginFeature *feature, gpointer)
{
   guint rank;
   const gchar *klass;

   if (!GST_IS_ELEMENT_FACTORY(feature)) {
      return FALSE;
   }

   klass = gst_element_factory_get_klass(GST_ELEMENT_FACTORY(feature));
   if (!(strstr(klass, "Sink") && strstr(klass, "Video"))) {
      return FALSE;
   }

   rank = gst_plugin_feature_get_rank(feature);
   if (rank < GST_RANK_MARGINAL) {
      return FALSE;
   }

   return TRUE;
}

static gint qt_gst_compare_ranks(GstPluginFeature *f1, GstPluginFeature *f2)
{
   gint diff;

   diff = gst_plugin_feature_get_rank(f2) - gst_plugin_feature_get_rank(f1);
   if (diff != 0) {
      return diff;
   }

   return strcmp(gst_plugin_feature_get_name(f2), gst_plugin_feature_get_name (f1));
}
#endif

GList *qt_gst_video_sinks()
{
   GList *list = nullptr;

#if GST_CHECK_VERSION(0, 10, 31)
   list = gst_element_factory_list_get_elements(GST_ELEMENT_FACTORY_TYPE_SINK | GST_ELEMENT_FACTORY_TYPE_MEDIA_VIDEO,
         GST_RANK_MARGINAL);
#else
   list = gst_registry_feature_filter(gst_registry_get_default(),
         (GstPluginFeatureFilter)qt_gst_videosink_factory_filter,
         FALSE, nullptr);
   list = g_list_sort(list, (GCompareFunc)qt_gst_compare_ranks);
#endif

   return list;
}

void qt_gst_util_double_to_fraction(gdouble src, gint *dest_n, gint *dest_d)
{
#if GST_CHECK_VERSION(0, 10, 26)
   gst_util_double_to_fraction(src, dest_n, dest_d);
#else
   qt_real_to_fraction(src, dest_n, dest_d);
#endif
}

QDebug operator <<(QDebug debug, GstCaps *caps)
{
   if (caps) {
      gchar *string = gst_caps_to_string(caps);
      debug = debug << string;
      g_free(string);
   }
   return debug;
}

