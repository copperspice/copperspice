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

#include <qicon.h>
#include <qicon_p.h>

#include <qfileinfo.h>
#include <qiconengine.h>
#include <qiconengineplugin.h>
#include <qpainter.h>
#include <qplatform_theme.h>
#include <qpixmapcache.h>
#include <qvariant.h>
#include <qcache.h>
#include <qdebug.h>
#include <qpalette.h>
#include <qmath.h>

#include <qapplication_p.h>
#include <qhexstring_p.h>
#include <qimagereader.h>
#include <qfactoryloader_p.h>
#include <qiconloader_p.h>

#ifndef QT_NO_ICON

static QAtomicInt serialNumCounter = QAtomicInt { 1 };
static void qt_cleanup_icon_cache();

namespace {

struct IconCache : public QCache<QString, QIcon> {
   IconCache() {
      // will not re-add if QApplication is re-created
      qAddPostRoutine(qt_cleanup_icon_cache);
   }
};

}

static IconCache *qtIconCache()
{
   static IconCache retval;
   return &retval;
}

static void qt_cleanup_icon_cache()
{
   qtIconCache()->clear();
}

static qreal qt_effective_device_pixel_ratio(QWindow *window = nullptr)
{
   if (!qApp->testAttribute(Qt::AA_UseHighDpiPixmaps)) {
      return qreal(1.0);
   }
   if (window) {
      return window->devicePixelRatio();
   }
   return qApp->devicePixelRatio(); // Don't know which window to target.
}

QIconPrivate::QIconPrivate()
   : engine(nullptr), ref(1), serialNum(serialNumCounter.fetchAndAddRelaxed(1)), detach_no(0), is_mask(false)
{
}

qreal QIconPrivate::pixmapDevicePixelRatio(qreal displayDevicePixelRatio, const QSize &requestedSize, const QSize &actualSize)
{
   QSize targetSize = requestedSize * displayDevicePixelRatio;
   qreal scale = 0.5 * (qreal(actualSize.width()) / qreal(targetSize.width()) +
         qreal(actualSize.height() / qreal(targetSize.height())));

   return qMax(qreal(1.0), displayDevicePixelRatio * scale);
}

QPixmapIconEngine::QPixmapIconEngine()
{
}

QPixmapIconEngine::QPixmapIconEngine(const QPixmapIconEngine &other)
   : QIconEngine(other), pixmaps(other.pixmaps)
{
}

QPixmapIconEngine::~QPixmapIconEngine()
{
}

void QPixmapIconEngine::paint(QPainter *painter, const QRect &rect, QIcon::Mode mode, QIcon::State state)
{
   QSize pixmapSize = rect.size() * qt_effective_device_pixel_ratio(nullptr);
   QPixmap px = pixmap(pixmapSize, mode, state);
   painter->drawPixmap(rect, px);
}

static inline int area(const QSize &s)
{
   return s.width() * s.height();
}

// returns the smallest of the two that is still larger than or equal to size.
static QPixmapIconEngineEntry *bestSizeMatch( const QSize &size, QPixmapIconEngineEntry *pa, QPixmapIconEngineEntry *pb)
{
   int s = area(size);
   if (pa->size == QSize() && pa->pixmap.isNull()) {
      pa->pixmap = QPixmap(pa->fileName);
      pa->size = pa->pixmap.size();
   }
   int a = area(pa->size);
   if (pb->size == QSize() && pb->pixmap.isNull()) {
      pb->pixmap = QPixmap(pb->fileName);
      pb->size = pb->pixmap.size();
   }
   int b = area(pb->size);
   int res = a;
   if (qMin(a, b) >= s) {
      res = qMin(a, b);
   } else {
      res = qMax(a, b);
   }
   if (res == a) {
      return pa;
   }
   return pb;
}

QPixmapIconEngineEntry *QPixmapIconEngine::tryMatch(const QSize &size, QIcon::Mode mode, QIcon::State state)
{
   QPixmapIconEngineEntry *pe = nullptr;

   for (int i = 0; i < pixmaps.count(); ++i) {
      if (pixmaps.at(i).mode == mode && pixmaps.at(i).state == state) {
         if (pe) {
            pe = bestSizeMatch(size, &pixmaps[i], pe);
         } else {
            pe = &pixmaps[i];
         }
      }
   }

   return pe;
}

QPixmapIconEngineEntry *QPixmapIconEngine::bestMatch(const QSize &size, QIcon::Mode mode, QIcon::State state,
   bool sizeOnly)
{
   QPixmapIconEngineEntry *pe = tryMatch(size, mode, state);

   while (!pe) {
      QIcon::State oppositeState = (state == QIcon::On) ? QIcon::Off : QIcon::On;
      if (mode == QIcon::Disabled || mode == QIcon::Selected) {
         QIcon::Mode oppositeMode = (mode == QIcon::Disabled) ? QIcon::Selected : QIcon::Disabled;
         if ((pe = tryMatch(size, QIcon::Normal, state))) {
            break;
         }
         if ((pe = tryMatch(size, QIcon::Active, state))) {
            break;
         }
         if ((pe = tryMatch(size, mode, oppositeState))) {
            break;
         }
         if ((pe = tryMatch(size, QIcon::Normal, oppositeState))) {
            break;
         }
         if ((pe = tryMatch(size, QIcon::Active, oppositeState))) {
            break;
         }
         if ((pe = tryMatch(size, oppositeMode, state))) {
            break;
         }
         if ((pe = tryMatch(size, oppositeMode, oppositeState))) {
            break;
         }
      } else {
         QIcon::Mode oppositeMode = (mode == QIcon::Normal) ? QIcon::Active : QIcon::Normal;
         if ((pe = tryMatch(size, oppositeMode, state))) {
            break;
         }
         if ((pe = tryMatch(size, mode, oppositeState))) {
            break;
         }
         if ((pe = tryMatch(size, oppositeMode, oppositeState))) {
            break;
         }
         if ((pe = tryMatch(size, QIcon::Disabled, state))) {
            break;
         }
         if ((pe = tryMatch(size, QIcon::Selected, state))) {
            break;
         }
         if ((pe = tryMatch(size, QIcon::Disabled, oppositeState))) {
            break;
         }
         if ((pe = tryMatch(size, QIcon::Selected, oppositeState))) {
            break;
         }
      }

      if (!pe) {
         return pe;
      }
   }

   if (sizeOnly ? (pe->size.isNull() || !pe->size.isValid()) : pe->pixmap.isNull()) {
      pe->pixmap = QPixmap(pe->fileName);
      if (!pe->pixmap.isNull()) {
         pe->size = pe->pixmap.size();
      }
   }

   return pe;
}

QPixmap QPixmapIconEngine::pixmap(const QSize &size, QIcon::Mode mode, QIcon::State state)
{
   QPixmap pm;
   QPixmapIconEngineEntry *pe = bestMatch(size, mode, state, false);

   if (pe) {
      pm = pe->pixmap;
   }

   if (pm.isNull()) {
      int idx = pixmaps.count();
      while (--idx >= 0) {
         if (pe == &pixmaps[idx]) {
            pixmaps.remove(idx);
            break;
         }
      }
      if (pixmaps.isEmpty()) {
         return pm;
      } else {
         return pixmap(size, mode, state);
      }
   }

   QSize actualSize = pm.size();
   if (!actualSize.isNull() && (actualSize.width() > size.width() || actualSize.height() > size.height())) {
      actualSize.scale(size, Qt::KeepAspectRatio);
   }

   QString key = "cs_"
      + HexString<quint64>(pm.cacheKey())
      + HexString<uint>(pe ? pe->mode : QIcon::Normal)
      + HexString<quint64>(QGuiApplication::palette().cacheKey())
      + HexString<uint>(actualSize.width())
      + HexString<uint>(actualSize.height());

   if (mode == QIcon::Active) {
      if (QPixmapCache::find(key + HexString<uint>(mode), pm)) {
         return pm;   // horray
      }

      if (QPixmapCache::find(key + HexString<uint>(QIcon::Normal), pm)) {
         QPixmap active = pm;

         if (QGuiApplication *guiApp = qobject_cast<QGuiApplication *>(qApp)) {
            active = guiApp->cs_internal_applyQIconStyle(QIcon::Active, pm);
         }

         if (pm.cacheKey() == active.cacheKey()) {
            return pm;
         }
      }
   }

   if (!QPixmapCache::find(key + HexString<uint>(mode), pm)) {
      if (pm.size() != actualSize) {
         pm = pm.scaled(actualSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
      }

      if (pe->mode != mode && mode != QIcon::Normal) {
         QPixmap generated = pm;

         if (QGuiApplication *guiApp = qobject_cast<QGuiApplication *>(qApp)) {
            generated = guiApp->cs_internal_applyQIconStyle(mode, pm);
         }

         if (!generated.isNull()) {
            pm = generated;
         }
      }

      QPixmapCache::insert(key + HexString<uint>(mode), pm);
   }

   return pm;
}

QSize QPixmapIconEngine::actualSize(const QSize &size, QIcon::Mode mode, QIcon::State state)
{
   QSize actualSize;
   if (QPixmapIconEngineEntry *pe = bestMatch(size, mode, state, true)) {
      actualSize = pe->size;
   }

   if (actualSize.isNull()) {
      return actualSize;
   }

   if (!actualSize.isNull() && (actualSize.width() > size.width() || actualSize.height() > size.height())) {
      actualSize.scale(size, Qt::KeepAspectRatio);
   }
   return actualSize;
}

void QPixmapIconEngine::addPixmap(const QPixmap &pixmap, QIcon::Mode mode, QIcon::State state)
{
   if (!pixmap.isNull()) {
      QPixmapIconEngineEntry *pe = tryMatch(pixmap.size(), mode, state);
      if (pe && pe->size == pixmap.size()) {
         pe->pixmap = pixmap;
         pe->fileName.clear();
      } else {
         pixmaps += QPixmapIconEngineEntry(pixmap, mode, state);
      }
   }
}

// Read out original image depth as set by ICOReader
static inline int origIcoDepth(const QImage &image)
{
   const QString s = image.text(QString("_q_icoOrigDepth"));
   return s.isEmpty() ? 32 : s.toInteger<int>();
}

static inline int findBySize(const QVector<QImage> &images, const QSize &size)
{
   for (int i = 0; i < images.size(); ++i) {
      if (images.at(i).size() == size) {
         return i;
      }
   }
   return -1;
}

// Convenience class providing a bool read() function.
namespace {

class ImageReader
{
 public:
   ImageReader(const QString &fileName) : m_reader(fileName), m_atEnd(false) {}

   QString format() const {
      return m_reader.format();
   }

   bool read(QImage *image) {
      if (m_atEnd) {
         return false;
      }
      *image = m_reader.read();
      if (!image->size().isValid()) {
         m_atEnd = true;
         return false;
      }
      m_atEnd = !m_reader.jumpToNextImage();
      return true;
   }

 private:
   QImageReader m_reader;
   bool m_atEnd;
};

} // namespace

void QPixmapIconEngine::addFile(const QString &fileName, const QSize &size, QIcon::Mode mode, QIcon::State state)
{
   if (fileName.isEmpty()) {
      return;
   }

   const QString abs = fileName.startsWith(QLatin1Char(':')) ? fileName : QFileInfo(fileName).absoluteFilePath();
   const bool ignoreSize = !size.isValid();
   ImageReader imageReader(abs);
   const QString format = imageReader.format();

   if (format.isEmpty()) {
      // Device failed to open or unsupported format.
      return;
   }

   QImage image;

   if (format != "ico") {
      if (ignoreSize) { // No size specified: Add all images.
         while (imageReader.read(&image)) {
            pixmaps += QPixmapIconEngineEntry(abs, image, mode, state);
         }
      } else {
         // Try to match size. If that fails, add a placeholder with the filename and empty pixmap for the size.
         while (imageReader.read(&image) && image.size() != size) {}
         pixmaps += image.size() == size ?
            QPixmapIconEngineEntry(abs, image, mode, state) : QPixmapIconEngineEntry(abs, size, mode, state);
      }

      return;
   }

   // Special case for reading Windows ".ico" files. Historically (QTBUG-39287),
   // these files may contain low-resolution images. As this information is lost,
   // ICOReader sets the original format as an image text key value. Read all matching
   // images into a list trying to find the highest quality per size.
   QVector<QImage> icoImages;

   while (imageReader.read(&image)) {
      if (ignoreSize || image.size() == size) {
         const int position = findBySize(icoImages, image.size());

         if (position >= 0) { // Higher quality available? -> replace.
            if (origIcoDepth(image) > origIcoDepth(icoImages.at(position))) {
               icoImages[position] = image;
            }
         } else {
            icoImages.append(image);
         }
      }

   }

   for (const QImage &i : icoImages)  {
      pixmaps += QPixmapIconEngineEntry(abs, i, mode, state);
   }

   if (icoImages.isEmpty() && !ignoreSize) { // Add placeholder with the filename and empty pixmap for the size.
      pixmaps += QPixmapIconEngineEntry(abs, size, mode, state);
   }

}

QString QPixmapIconEngine::key() const
{
   return QLatin1String("QPixmapIconEngine");
}

QIconEngine *QPixmapIconEngine::clone() const
{
   return new QPixmapIconEngine(*this);
}

bool QPixmapIconEngine::read(QDataStream &in)
{
   int num_entries;
   QPixmap pm;
   QString fileName;
   QSize sz;
   uint mode;
   uint state;

   in >> num_entries;
   for (int i = 0; i < num_entries; ++i) {
      if (in.atEnd()) {
         pixmaps.clear();
         return false;
      }
      in >> pm;
      in >> fileName;
      in >> sz;
      in >> mode;
      in >> state;
      if (pm.isNull()) {
         addFile(fileName, sz, QIcon::Mode(mode), QIcon::State(state));
      } else {
         QPixmapIconEngineEntry pe(fileName, sz, QIcon::Mode(mode), QIcon::State(state));
         pe.pixmap = pm;
         pixmaps += pe;
      }
   }
   return true;
}

bool QPixmapIconEngine::write(QDataStream &out) const
{
   int num_entries = pixmaps.size();
   out << num_entries;
   for (int i = 0; i < num_entries; ++i) {
      if (pixmaps.at(i).pixmap.isNull()) {
         out << QPixmap(pixmaps.at(i).fileName);
      } else {
         out << pixmaps.at(i).pixmap;
      }
      out << pixmaps.at(i).fileName;
      out << pixmaps.at(i).size;
      out << (uint) pixmaps.at(i).mode;
      out << (uint) pixmaps.at(i).state;
   }
   return true;
}

void QPixmapIconEngine::virtual_hook(int id, void *data)
{
   switch (id) {
      case QIconEngine::AvailableSizesHook: {
         QIconEngine::AvailableSizesArgument &arg =
            *reinterpret_cast<QIconEngine::AvailableSizesArgument *>(data);
         arg.sizes.clear();
         for (int i = 0; i < pixmaps.size(); ++i) {
            QPixmapIconEngineEntry &pe = pixmaps[i];
            if (pe.size == QSize() && pe.pixmap.isNull()) {
               pe.pixmap = QPixmap(pe.fileName);
               pe.size = pe.pixmap.size();
            }
            if (pe.mode == arg.mode && pe.state == arg.state && !pe.size.isEmpty()) {
               arg.sizes.push_back(pe.size);
            }
         }
         break;
      }

      default:
         QIconEngine::virtual_hook(id, data);
   }
}

static QFactoryLoader *loader()
{
   static QFactoryLoader retval(QIconEngineInterface_ID, "/iconengines", Qt::CaseInsensitive);
   return &retval;
}

QFactoryLoader *cs_internal_iconLoader()
{
   return loader();
}

QIcon::QIcon()
   : d(nullptr)
{
}

QIcon::QIcon(const QPixmap &pixmap)
   : d(nullptr)
{
   addPixmap(pixmap);
}

QIcon::QIcon(const QIcon &other)
   : d(other.d)
{
   if (d) {
      d->ref.ref();
   }
}

QIcon::QIcon(const QString &fileName)
   : d(nullptr)
{
   addFile(fileName);
}

QIcon::QIcon(QIconEngine *engine)
   : d(new QIconPrivate)
{
   d->engine = engine;
}


QIcon::~QIcon()
{
   if (d && !d->ref.deref()) {
      delete d;
   }
}

QIcon &QIcon::operator=(const QIcon &other)
{
   if (other.d) {
      other.d->ref.ref();
   }

   if (d && !d->ref.deref()) {
      delete d;
   }

   d = other.d;
   return *this;
}

QIcon::operator QVariant() const
{
   return QVariant(QVariant::Icon, this);
}


qint64 QIcon::cacheKey() const
{
   if (!d) {
      return 0;
   }

   return (((qint64) d->serialNum) << 32) | ((qint64) (d->detach_no));
}

QPixmap QIcon::pixmap(const QSize &size, Mode mode, State state) const
{
   if (!d) {
      return QPixmap();
   }

   return pixmap(nullptr, size, mode, state);
}

QSize QIcon::actualSize(const QSize &size, Mode mode, State state) const
{
   if (!d) {
      return QSize();
   }

   return actualSize(nullptr, size, mode, state);
}

QPixmap QIcon::pixmap(QWindow *window, const QSize &size, Mode mode, State state) const
{
   if (!d) {
      return QPixmap();
   }

   qreal devicePixelRatio = qt_effective_device_pixel_ratio(window);
   if (!(devicePixelRatio > 1.0)) {
      QPixmap pixmap = d->engine->pixmap(size, mode, state);
      pixmap.setDevicePixelRatio(1.0);
      return pixmap;
   }

   QPixmap pixmap = d->engine->pixmap(size * devicePixelRatio, mode, state);
   pixmap.setDevicePixelRatio(d->pixmapDevicePixelRatio(devicePixelRatio, size, pixmap.size()));

   return pixmap;
}

QSize QIcon::actualSize(QWindow *window, const QSize &size, Mode mode, State state) const
{
   if (!d) {
      return QSize();
   }

   qreal devicePixelRatio = qt_effective_device_pixel_ratio(window);
   if (!(devicePixelRatio > 1.0)) {
      return d->engine->actualSize(size, mode, state);
   }

   QSize actualSize = d->engine->actualSize(size * devicePixelRatio, mode, state);
   return actualSize / d->pixmapDevicePixelRatio(devicePixelRatio, size, actualSize);
}

void QIcon::paint(QPainter *painter, const QRect &rect, Qt::Alignment alignment, Mode mode, State state) const
{
   if (!d || !painter) {
      return;
   }

   // Copy of QStyle::alignedRect
   const QSize size = d->engine->actualSize(rect.size(), mode, state);
   alignment = QGuiApplicationPrivate::visualAlignment(painter->layoutDirection(), alignment);
   int x = rect.x();
   int y = rect.y();
   int w = size.width();
   int h = size.height();
   if ((alignment & Qt::AlignVCenter) == Qt::AlignVCenter) {
      y += rect.size().height() / 2 - h / 2;
   } else if ((alignment & Qt::AlignBottom) == Qt::AlignBottom) {
      y += rect.size().height() - h;
   }
   if ((alignment & Qt::AlignRight) == Qt::AlignRight) {
      x += rect.size().width() - w;
   } else if ((alignment & Qt::AlignHCenter) == Qt::AlignHCenter) {
      x += rect.size().width() / 2 - w / 2;
   }
   QRect alignedRect(x, y, w, h);

   d->engine->paint(painter, alignedRect, mode, state);
}

bool QIcon::isNull() const
{
   return !d;
}

/*!\internal
 */
bool QIcon::isDetached() const
{
   return !d || d->ref.load() == 1;
}

/*! \internal
 */
void QIcon::detach()
{
   if (d) {
      if (d->ref.load() != 1) {
         QIconPrivate *x = new QIconPrivate;
         x->engine = d->engine->clone();
         if (!d->ref.deref()) {
            delete d;
         }
         d = x;
      }
      ++d->detach_no;
   }
}

/*!
    Adds \a pixmap to the icon, as a specialization for \a mode and
    \a state.

    Custom icon engines are free to ignore additionally added
    pixmaps.

    \sa addFile()
*/
void QIcon::addPixmap(const QPixmap &pixmap, Mode mode, State state)
{
   if (pixmap.isNull()) {
      return;
   }
   if (!d) {
      d = new QIconPrivate;
      d->engine = new QPixmapIconEngine;
   } else {
      detach();
   }
   d->engine->addPixmap(pixmap, mode, state);
}

void QIcon::addFile(const QString &fileName, const QSize &size, Mode mode, State state)
{
   if (fileName.isEmpty()) {
      return;
   }

   if (! d) {

      QFileInfo info(fileName);
      QString suffix = info.suffix();

      if (! suffix.isEmpty()) {
         // first try version 2 engines
         QFactoryLoader *factoryObj = loader();

         if (factoryObj != nullptr) {
            // what keys are available
            const QSet<QString> keySet = factoryObj->keySet();

            if (keySet.contains(suffix)) {
               QIconEnginePlugin *factory = dynamic_cast<QIconEnginePlugin *>(factoryObj->instance(suffix));

               if (factory) {
                  if (QIconEngine *engine = factory->create(fileName)) {
                     d = new QIconPrivate;
                     d->engine = engine;
                  }
               }
            }
         }
      }

      // then fall back to the default engine
      if (! d) {
         d = new QIconPrivate;
         d->engine = new QPixmapIconEngine;
      }

   } else {
      detach();
   }

   d->engine->addFile(fileName, size, mode, state);
   QString atNxFileName = qt_findAtNxFile(fileName, qApp->devicePixelRatio());

   if (atNxFileName != fileName) {
      d->engine->addFile(atNxFileName, size, mode, state);
   }
}

QList<QSize> QIcon::availableSizes(Mode mode, State state) const
{
   if (! d || !d->engine) {
      return QList<QSize>();
   }

   return d->engine->availableSizes(mode, state);
}

QString QIcon::name() const
{
   if (! d || !d->engine) {
      return QString();
   }

   return d->engine->iconName();
}


void QIcon::setThemeSearchPaths(const QStringList &paths)
{
   QIconLoader::instance()->setThemeSearchPath(paths);
}

QStringList QIcon::themeSearchPaths()
{
   return QIconLoader::instance()->themeSearchPaths();
}

void QIcon::setThemeName(const QString &name)
{
   QIconLoader::instance()->setThemeName(name);
}


QString QIcon::themeName()
{
   return QIconLoader::instance()->themeName();
}


QIcon QIcon::fromTheme(const QString &name, const QIcon &fallback)
{
   QIcon icon;

   if (qtIconCache()->contains(name)) {
      icon = *qtIconCache()->object(name);
   } else {
      QPlatformTheme *const platformTheme = QGuiApplicationPrivate::platformTheme();
      bool hasUserTheme = QIconLoader::instance()->hasUserTheme();
      QIconEngine *const engine = (platformTheme && !hasUserTheme) ? platformTheme->createIconEngine(name)
         : new QIconLoaderEngine(name);
      QIcon *cachedIcon  = new QIcon(engine);
      icon = *cachedIcon;
      qtIconCache()->insert(name, cachedIcon);
   }

   // Note the qapp check is to allow lazy loading of static icons
   // Supporting fallbacks will not work for this case.
   if (qApp && icon.availableSizes().isEmpty()) {
      return fallback;
   }

   return icon;
}


bool QIcon::hasThemeIcon(const QString &name)
{
   QIcon icon = fromTheme(name);

   return icon.name() == name;
}

void QIcon::setIsMask(bool isMask)
{
   if (!d) {
      d = new QIconPrivate;
      d->engine = new QPixmapIconEngine;
   } else {
      detach();
   }
   d->is_mask = isMask;
}

bool QIcon::isMask() const
{
   if (!d) {
      return false;
   }
   return d->is_mask;
}

QDataStream &operator<<(QDataStream &s, const QIcon &icon)
{
   if (icon.isNull()) {
      s << QString();

   } else {
      s << icon.d->engine->key();
      icon.d->engine->write(s);
      // not really supported
   }

   return s;
}

QDataStream &operator>>(QDataStream &s, QIcon &icon)
{
   icon = QIcon();
   QString key;
   s >> key;

   if (key == "QPixmapIconEngine") {
      icon.d = new QIconPrivate;
      QIconEngine *engine = new QPixmapIconEngine;
      icon.d->engine = engine;
      engine->read(s);

   } else if (key == "QIconLoaderEngine") {
      icon.d = new QIconPrivate;
      QIconEngine *engine = new QIconLoaderEngine();
      icon.d->engine = engine;
      engine->read(s);

   } else if (loader()->keySet().contains(key)) {
      if (QIconEnginePlugin *factory = qobject_cast<QIconEnginePlugin *>(loader()->instance(key))) {

         if (QIconEngine *engine = factory->create()) {
            icon.d = new QIconPrivate;
            icon.d->engine = engine;
            engine->read(s);
         }

      }
   }

   return s;
}

QString qt_findAtNxFile(const QString &baseFileName, qreal targetDevicePixelRatio,
   qreal *sourceDevicePixelRatio)
{
   if (targetDevicePixelRatio <= 1.0) {
      return baseFileName;
   }

   static bool disableNxImageLoading = ! qgetenv("QT_HIGHDPI_DISABLE_2X_IMAGE_LOADING").isEmpty();

   if (disableNxImageLoading) {
      return baseFileName;
   }

   int dotIndex = baseFileName.lastIndexOf('.');
   if (dotIndex == -1) {
      /* no dot */
      dotIndex = baseFileName.size();
   }

   QString atNxfileName = baseFileName;
   atNxfileName.insert(dotIndex, "@2x");

   // Check for @Nx, ..., @3x, @2x file versions,
   for (int n = qMin(qCeil(targetDevicePixelRatio), 9); n > 1; --n) {
      atNxfileName.replace(dotIndex + 1, 1, '0' + n);

      if (QFile::exists(atNxfileName)) {
         if (sourceDevicePixelRatio) {
            *sourceDevicePixelRatio = n;
         }

         return atNxfileName;
      }
   }

   return baseFileName;
}

#endif //QT_NO_ICON
