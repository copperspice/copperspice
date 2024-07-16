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

#include <qfontdatabase.h>

#include <qalgorithms.h>
#include <qapplication.h>
#include <qcache.h>
#include <qdebug.h>
#include <qdir.h>
#include <qflatmap.h>
#include <qhash.h>
#include <qmath.h>
#include <qmutex.h>
#include <qplatform_integration.h>
#include <qplatform_fontdatabase.h>
#include <qplatform_theme.h>
#include <qthread.h>
#include <qvarlengtharray.h>

#include <qapplication_p.h>
#include <qfontengine_p.h>
#include <qunicodetables_p.h>

#include <memory>
#include <stdlib.h>
#include <limits.h>

#define SMOOTH_SCALABLE 0xffff

static void initializeDb();

static int getFontWeight(const QString &weightString)
{
   QString s1 = weightString.toLower();

   // Test in decreasing order of commonness
   if (s1 == "normal" || s1 == "regular") {
      return QFont::Normal;
   }

   if (s1 == "bold") {
      return QFont::Bold;
   }

   if (s1 == "semibold" || s1 == "semi bold" || s1 == "demibold" || s1 == "demi bold") {
      return QFont::DemiBold;
   }

   if (s1 == "medium") {
      return QFont::Medium;
   }

   if (s1 == "black") {
      return QFont::Black;
   }

   if (s1 == "light") {
      return QFont::Light;
   }

   if (s1 == "thin") {
      return QFont::Thin;
   }

   const QStringView s2 = s1.midView(2);

   if (s1.startsWith("ex") || s1.startsWith("ul")) {
      if (s2 == "tralight" || s1 == "tra light") {
         return QFont::ExtraLight;
      }

      if (s2 == "trabold" || s2 == "tra bold") {
         return QFont::ExtraBold;
      }
   }

   if (s1.contains("bold")) {

      if (s1.contains("demi")) {
         return QFont::DemiBold;
      }

      return QFont::Bold;
   }

   if (s1.contains("thin")) {
      return QFont::Thin;
   }

   if (s1.contains("light")) {
      return QFont::Light;
   }

   if (s1.contains("black")) {
      return QFont::Black;
   }

   // Now, we perform string translations & comparisons with those.
   // These are (very) slow compared to simple string ops, so we do these last.
   // As using translated values for such things is not very common, this should not be too bad.

   if (s1.compare(QCoreApplication::translate("QFontDatabase", "Normal", "Normal or Regular font weight"), Qt::CaseInsensitive) == 0) {
      return QFont::Normal;
   }

   const QString translatedBold = QCoreApplication::translate("QFontDatabase", "Bold").toLower();

   if (s1 == translatedBold) {
      return QFont::Bold;
   }

   if (s1.compare(QCoreApplication::translate("QFontDatabase", "Demi Bold"), Qt::CaseInsensitive) == 0) {
      return QFont::DemiBold;
   }

   if (s1.compare(QCoreApplication::translate("QFontDatabase", "Medium", "Medium font weight"), Qt::CaseInsensitive) == 0) {
      return QFont::Medium;
   }

   if (s1.compare(QCoreApplication::translate("QFontDatabase", "Black"), Qt::CaseInsensitive) == 0) {
      return QFont::Black;
   }

   //
   const QString translatedLight = QCoreApplication::translate("QFontDatabase", "Light").toLower();
   if (s1 == translatedLight) {
      return QFont::Light;
   }

   if (s1.compare(QCoreApplication::translate("QFontDatabase", "Thin"), Qt::CaseInsensitive) == 0) {
      return QFont::Thin;
   }

   if (s1.compare(QCoreApplication::translate("QFontDatabase", "Extra Light"), Qt::CaseInsensitive) == 0) {
      return QFont::ExtraLight;
   }

   if (s1.compare(QCoreApplication::translate("QFontDatabase", "Extra Bold"), Qt::CaseInsensitive) == 0) {
      return QFont::ExtraBold;
   }

   // And now the contains() checks for the translated strings.
   // The word for "Extra" as in "Extra Bold, Extra Thin" used as a pattern for string searches
   const QString translatedExtra = QCoreApplication::translate("QFontDatabase", "Extra").toLower();

   if (s1.contains(translatedBold)) {
      //: The word for "Demi" as in "Demi Bold" used as a pattern for string searches
      QString translatedDemi = QCoreApplication::translate("QFontDatabase", "Demi").toLower();

      if (s1.contains(translatedDemi)) {
         return QFont::DemiBold;
      }

      if (s1.contains(translatedExtra)) {
         return QFont::ExtraBold;
      }

      return QFont::Bold;
   }

   if (s1.contains(translatedLight)) {
      if (s1.contains(translatedExtra)) {
         return QFont::ExtraLight;
      }

      return QFont::Light;
   }

   return QFont::Normal;
}

struct FontSizeHandleDeleter {
   void operator()(void *p) const {
      if (p != nullptr) {
         QPlatformIntegration *integration = QGuiApplicationPrivate::platformIntegration();

         if (integration != nullptr) {
            integration->fontDatabase()->releaseHandle(p);
         }
      }
   }
};

using FontSizeHandle = std::shared_ptr<void>;

FontSizeHandle MakeFontSizeHandle(void *p)
{
   return FontSizeHandle(p, FontSizeHandleDeleter());
}

struct QtFontSize {
   FontSizeHandle handle;
   unsigned short pixelSize : 16;
};

struct QtFontStyle {
   struct Key {

      explicit Key(const QString &styleString);

      Key()
         : style(QFont::StyleNormal), weight(QFont::Normal), stretch(0)
      {
      }

      bool operator==(const Key &other) const {
         return (style == other.style && weight == other.weight &&
                  (stretch == 0 || other.stretch == 0 || stretch == other.stretch));
      }

      bool operator!=(const Key &other) const {
         return ! operator==(other);
      }

      bool operator<(const Key &o) const {
         int x = (style << 12) + (weight << 14) + stretch;
         int y = (o.style << 12) + (o.weight << 14) + o.stretch;

         return (x < y);
      }

      uint style   : 2;
      uint weight  : 8;
      signed int stretch : 12;
   };

   explicit QtFontStyle(const Key &k)
      : key(k), bitmapScalable(false), smoothScalable(false)
   {
   }

   explicit QtFontStyle(const Key &k, const QString &s)
      : key(k), bitmapScalable(false), smoothScalable(false), styleName(s)
   {
   }

   Key key;
   bool bitmapScalable;
   bool smoothScalable;
   bool antialiased;

   QFlatMap<unsigned short, QtFontSize> m_sizes;
   QString styleName;

   QtFontSize *fontSize(unsigned short size, bool create = false);
};

QtFontStyle::Key::Key(const QString &styleString)
   : style(QFont::StyleNormal), weight(QFont::Normal), stretch(0)
{
   weight = getFontWeight(styleString);

   if (! styleString.isEmpty()) {
      // first the straightforward no-translation checks, these are fast

      if (styleString.contains("Italic")) {
         style = QFont::StyleItalic;

      } else if (styleString.contains("Oblique")) {
         style = QFont::StyleOblique;

      } else if (styleString.contains(QCoreApplication::translate("QFontDatabase", "Italic"))) {
         style = QFont::StyleItalic;

      } else if (styleString.contains(QCoreApplication::translate("QFontDatabase", "Oblique"))) {
         style = QFont::StyleOblique;
      }
   }
}

QtFontSize *QtFontStyle::fontSize(unsigned short size, bool create)
{
   auto iter = m_sizes.find(size);

   if (iter != m_sizes.end()) {
      return &iter.value();
   }

   if (! create) {
      return nullptr;
   }

   auto item = m_sizes.insert(size, {nullptr, size});

   return &item.value();
}

struct QtFontFoundry {

   QtFontFoundry() = default;

   explicit QtFontFoundry(const QString &name)
      : m_fontMfg(name)
   {
   }

   ~QtFontFoundry() = default;

   QString m_fontMfg;

   QFlatMap<QtFontStyle::Key, QtFontStyle> m_styles;
   QtFontStyle *style(const QtFontStyle::Key &key, const QString & = QString(), bool create = false);
};

QtFontStyle *QtFontFoundry::style(const QtFontStyle::Key &key, const QString &styleName, bool create)
{
   bool hasStyleName = ! styleName.isEmpty();      // search styleName first if available

   if (hasStyleName) {
      for (auto &style : m_styles) {
         if (style.styleName == styleName) {
            return &style;
         }
      }
   }

   auto iter = m_styles.find(key);

   if (iter != m_styles.end()) {
      return &iter.value();
   }

   if (! create) {
      return nullptr;
   }

   auto item = m_styles.insert(key, QtFontStyle(key, styleName));

   return &item.value();
}

struct QtFontFamily {
   enum WritingSystemStatus {
      Unknown       = 0,
      Supported     = 1,
      UnsupportedFT = 2,
      Unsupported   = UnsupportedFT
   };

   QtFontFamily(const QString &n)
      : m_populated(false), m_fixedPitch(false), m_name(n)
   {
      memset(writingSystems, 0, sizeof(writingSystems));
   }

   ~QtFontFamily() = default;

   bool m_populated;
   bool m_fixedPitch;

   QString m_name;
   QStringList m_aliases;

   QHash<QString, QtFontFoundry> m_foundries;

   unsigned char writingSystems[QFontDatabase::WritingSystemsCount];

   bool matchesFamilyName(const QString &familyName) const;
   QtFontFoundry *foundry(const QString &familyName, bool create = false);
   void ensurePopulated();
};

QtFontFoundry *QtFontFamily::foundry(const QString &familyName, bool create)
{
   if (familyName.isEmpty() && m_foundries.size() == 1) {
      auto iter = m_foundries.begin();
      return &iter.value();
   }

   const auto key = familyName.toCaseFolded();
   auto iter      = m_foundries.find(key);

   if (iter == m_foundries.end()) {
      if (create == false) {
         return nullptr;
      }

      iter = m_foundries.insert(key, QtFontFoundry(familyName));
   }

   return &iter.value();
}

bool QtFontFamily::matchesFamilyName(const QString &familyName) const
{
   return m_name.compare(familyName, Qt::CaseInsensitive) == 0 || m_aliases.contains(familyName, Qt::CaseInsensitive);
}

void QtFontFamily::ensurePopulated()
{
   if (m_populated) {
      return;
   }

   QApplicationPrivate::platformIntegration()->fontDatabase()->populateFamily(m_name);
   Q_ASSERT_X(m_populated, "QtFontFamily::ensurePopulated()", csPrintable(m_name));
}

struct FallbacksCacheKey {
   QString family;

   QFont::Style style;
   QFont::StyleHint styleHint;
   QChar::Script script;
};

inline bool operator==(const FallbacksCacheKey &lhs, const FallbacksCacheKey &rhs)
{
   return lhs.script == rhs.script && lhs.styleHint == rhs.styleHint &&
          lhs.style == rhs.style && lhs.family == rhs.family;
}

inline bool operator!=(const FallbacksCacheKey &lhs, const FallbacksCacheKey &rhs)
{
   return ! operator==(lhs, rhs);
}

inline uint qHash(const FallbacksCacheKey &key, uint seed = 0)
{
   seed = qHash(key.family, seed);
   seed = qHash(int(key.style), seed);
   seed = qHash(int(key.styleHint), seed);
   seed = qHash(int(key.script), seed);

   return seed;
}

class QFontDatabasePrivate
{
 public:
   enum FamilyRequestFlags {
      RequestFamily = 0,
      EnsureCreated,
      EnsurePopulated
   };

   QFontDatabasePrivate()
      : fallbacksCache(64), reregisterAppFonts(false)
   {
   }

   ~QFontDatabasePrivate() = default;

   void clear() {
      m_families.clear();
   }

   QtFontFamily *family(const QString &familyName, FamilyRequestFlags flags = EnsurePopulated);

   QHash<QString, QtFontFamily> m_families;
   QCache<FallbacksCacheKey, QStringList> fallbacksCache;

   struct ApplicationFont {
      QString fileName;
      QByteArray data;
      QStringList families;
   };

   QVector<ApplicationFont> applicationFonts;
   int addAppFont(const QByteArray &fontData, const QString &fileName);
   bool reregisterAppFonts;
   bool isApplicationFont(const QString &fileName);

   void invalidate();
};

static QFontDatabasePrivate *privateDb()
{
   static QFontDatabasePrivate retval;
   return &retval;
}

static QRecursiveMutex *fontDatabaseMutex()
{
   static QRecursiveMutex retval;
   return &retval;
}

void QFontDatabasePrivate::invalidate()
{
   QFontCache::instance()->clear();
   fallbacksCache.clear();
   clear();

   QGuiApplicationPrivate::platformIntegration()->fontDatabase()->invalidate();
   emit static_cast<QGuiApplication *>(QCoreApplication::instance())->fontDatabaseChanged();
}

QtFontFamily *QFontDatabasePrivate::family(const QString &familyName, FamilyRequestFlags flags)
{
   QtFontFamily *retval = nullptr;

   QString key = familyName.toCaseFolded();
   auto iter   = m_families.find(key);

   if (iter != m_families.end()) {
      retval = &iter.value();

   } else {
      if (flags & EnsureCreated) {
         auto iter = m_families.insert(key, QtFontFamily(familyName));
         retval    = &iter.value();
      }
   }

   if (retval != nullptr && (flags & EnsurePopulated)) {
      retval->ensurePopulated();
   }

   return retval;
}

static const int scriptForWritingSystem[] = {
   QChar::Script_Common,          // Any
   QChar::Script_Latin,           // Latin
   QChar::Script_Greek,           // Greek
   QChar::Script_Cyrillic,        // Cyrillic
   QChar::Script_Armenian,        // Armenian
   QChar::Script_Hebrew,          // Hebrew
   QChar::Script_Arabic,          // Arabic
   QChar::Script_Syriac,          // Syriac
   QChar::Script_Thaana,          // Thaana
   QChar::Script_Devanagari,      // Devanagari
   QChar::Script_Bengali,         // Bengali
   QChar::Script_Gurmukhi,        // Gurmukhi
   QChar::Script_Gujarati,        // Gujarati
   QChar::Script_Oriya,           // Oriya
   QChar::Script_Tamil,           // Tamil
   QChar::Script_Telugu,          // Telugu
   QChar::Script_Kannada,         // Kannada
   QChar::Script_Malayalam,       // Malayalam
   QChar::Script_Sinhala,         // Sinhala
   QChar::Script_Thai,            // Thai
   QChar::Script_Lao,             // Lao
   QChar::Script_Tibetan,         // Tibetan
   QChar::Script_Myanmar,         // Myanmar
   QChar::Script_Georgian,        // Georgian
   QChar::Script_Khmer,           // Khmer
   QChar::Script_Han,             // SimplifiedChinese
   QChar::Script_Han,             // TraditionalChinese
   QChar::Script_Han,             // Japanese
   QChar::Script_Hangul,          // Korean
   QChar::Script_Latin,           // Vietnamese
   QChar::Script_Common,          // Symbol
   QChar::Script_Ogham,           // Ogham
   QChar::Script_Runic,           // Runic
   QChar::Script_Nko              // Nko
};
static_assert(sizeof(scriptForWritingSystem) / sizeof(scriptForWritingSystem[0]) == QFontDatabase::WritingSystemsCount, "Type mismatch");

Q_GUI_EXPORT int qt_script_for_writing_system(QFontDatabase::WritingSystem writingSystem)
{
   return scriptForWritingSystem[writingSystem];
}

struct QtFontDesc {
   QtFontDesc()
      : family(nullptr), foundry(nullptr), style(nullptr), size(nullptr)
   {
   }

   QtFontFamily *family;
   QtFontFoundry *foundry;
   QtFontStyle *style;
   QtFontSize *size;
};

static void initFontDef(const QtFontDesc &desc, const QFontDef &request, QFontDef *fontDef, bool multi)
{
   fontDef->family = desc.family->m_name;

   if (! desc.foundry->m_fontMfg.isEmpty() && desc.family->m_foundries.size() > 1) {
      fontDef->family += " [" + desc.foundry->m_fontMfg + "]";
   }

   if (desc.style->smoothScalable || QGuiApplicationPrivate::platformIntegration()->fontDatabase()->fontsAlwaysScalable() ||
                  (desc.style->bitmapScalable && (request.styleStrategy & QFont::PreferMatch))) {

      fontDef->pixelSize = request.pixelSize;

   } else {
      fontDef->pixelSize = desc.size->pixelSize;
   }

   fontDef->pointSize     = request.pointSize;
   fontDef->styleHint     = request.styleHint;
   fontDef->styleStrategy = request.styleStrategy;

   if (! multi) {
      fontDef->weight = desc.style->key.weight;
   }

   if (! multi) {
      fontDef->style = desc.style->key.style;
   }

   fontDef->fixedPitch  = desc.family->m_fixedPitch;
   fontDef->stretch     = desc.style->key.stretch;
   fontDef->ignorePitch = false;
}

static QStringList familyList(const QFontDef &req)
{
   // list of families to try
   QStringList familyList;

   if (req.family.isEmpty()) {
      return familyList;
   }

   QStringList list = req.family.split(',');

   const int numFamilies = list.size();

   for (int i = 0; i < numFamilies; ++i) {
      QString str = list.at(i).trimmed();

      if ((str.startsWith('"') && str.endsWith('"')) || (str.startsWith('\'') && str.endsWith('\''))) {
         str = str.mid(1, str.length() - 2);
      }

      familyList.append(str);
   }

   // append an entry to the tmpList for each family in family_list
   QStringList tmpList;

   for (const auto &item : familyList) {
      tmpList += QFont::substitutes(item);
   }

   familyList.append(tmpList);

   return familyList;
}

// used in qapplication.cpp
void qt_cleanupFontDatabase()
{
   QFontDatabasePrivate *db = privateDb();

   if (db != nullptr) {
      db->fallbacksCache.clear();
      db->clear();
   }
}

// used in qfontengine_x11.cpp
QRecursiveMutex *qt_fontdatabase_mutex()
{
   return fontDatabaseMutex();
}

void qt_registerFont(const QString &familyName, const QString &stylename, const QString &foundryname, int weight,
            QFont::Style style, int stretch, bool antialiased, bool scalable, int pixelSize, bool fixedPitch,
            const QSupportedWritingSystems &writingSystems, void *handle)
{
   QtFontStyle::Key styleKey;
   styleKey.style   = style;
   styleKey.weight  = weight;
   styleKey.stretch = stretch;

   QFontDatabasePrivate *db = privateDb();

   QtFontFamily *family = db->family(familyName, QFontDatabasePrivate::EnsureCreated);
   family->m_fixedPitch = fixedPitch;

   for (int i = 0; i < QFontDatabase::WritingSystemsCount; ++i) {
      if (writingSystems.supported(QFontDatabase::WritingSystem(i))) {
         family->writingSystems[i] = QtFontFamily::Supported;
      }
   }

   QtFontFoundry *foundry    = family->foundry(foundryname, true);
   QtFontStyle *fontStyle    = foundry->style(styleKey, stylename, true);
   fontStyle->smoothScalable = scalable;
   fontStyle->antialiased    = antialiased;
   QtFontSize *size          = fontStyle->fontSize(pixelSize ? pixelSize : SMOOTH_SCALABLE, true);

   size->handle        = MakeFontSizeHandle(handle);
   family->m_populated = true;
}

void qt_registerFontFamily(const QString &familyName)
{
   // Create uninitialized/unpopulated family
   privateDb()->family(familyName, QFontDatabasePrivate::EnsureCreated);
}

void qt_registerAliasToFontFamily(const QString &familyName, const QString &alias)
{
   if (alias.isEmpty()) {
      return;
   }

   QFontDatabasePrivate *db = privateDb();

   QtFontFamily *family = db->family(familyName, QFontDatabasePrivate::RequestFamily);

   if (family == nullptr) {
      return;
   }

   if (family->m_aliases.contains(alias, Qt::CaseInsensitive)) {
      return;
   }

   family->m_aliases.push_back(alias);
}

QString qt_resolveFontFamilyAlias(const QString &alias)
{
   if (alias.isEmpty()) {
      return alias;
   }

   const auto &families = privateDb()->m_families;

   auto iter = families.find(alias.toCaseFolded());

   if (iter != families.end()) {
      return iter->m_name;
   }

   for (const auto &family : families) {
      if (family.matchesFamilyName(alias)) {
         return family.m_name;
      }
   }

   return alias;
}

QStringList QPlatformFontDatabase::fallbacksForFamily(const QString &familyName, QFont::Style style,
      QFont::StyleHint styleHint, QChar::Script script) const
{
   (void) familyName;
   (void) styleHint;

   QStringList preferredFallbacks;
   QStringList otherFallbacks;

   size_t writingSystem = std::find(scriptForWritingSystem, scriptForWritingSystem +
         QFontDatabase::WritingSystemsCount, script) - scriptForWritingSystem;

   if (writingSystem >= QFontDatabase::WritingSystemsCount) {
      writingSystem = QFontDatabase::Any;
   }

   for (QtFontFamily &family : privateDb()->m_families) {
      family.ensurePopulated();

      if (writingSystem > QFontDatabase::Any && family.writingSystems[writingSystem] != QtFontFamily::Supported) {
         continue;
      }

      for (const QtFontFoundry &item : family.m_foundries) {
         QString name = family.m_name;

         if (! item.m_fontMfg.isEmpty()) {
            name = " [" + item.m_fontMfg + ']';
         }

         for (auto &fontStyle : item.m_styles) {
            if (style == fontStyle.key.style) {
               preferredFallbacks.append(name);
            } else {
               otherFallbacks.append(name);
            }
         }
      }
   }

   return preferredFallbacks + otherFallbacks;
}

static QStringList fallbacksForFamily(const QString &familyName, QFont::Style style,
      QFont::StyleHint styleHint, QChar::Script script)
{
   QFontDatabasePrivate *db = privateDb();

   if (db->m_families.empty()) {
      initializeDb();
   }

   const FallbacksCacheKey cacheKey = {familyName, style, styleHint, script};

   if (const QStringList *fallbacks = db->fallbacksCache.object(cacheKey)) {
      return *fallbacks;
   }

   // make sure the db has all fallback families
   QStringList retval = QGuiApplicationPrivate::platformIntegration()->fontDatabase()->
         fallbacksForFamily(familyName, style, styleHint, script);

   QStringList::iterator iter = retval.begin();

   while (iter != retval.end()) {
      bool found = db->m_families.contains(iter->toCaseFolded());

      if (! found) {
         for (const auto &item : db->m_families) {
            if (item.matchesFamilyName(*iter)) {
               found = true;
               break;
            }
         }
      }

      if (found) {
         ++iter;
      } else {
         iter = retval.erase(iter);
      }
   }

   db->fallbacksCache.insert(cacheKey, new QStringList(retval));

   return retval;
}

QStringList qt_fallbacksForFamily(const QString &family, QFont::Style style, QFont::StyleHint styleHint, QChar::Script script)
{
   QRecursiveMutexLocker locker(fontDatabaseMutex());
   return fallbacksForFamily(family, style, styleHint, script);
}

static void registerFont(QFontDatabasePrivate::ApplicationFont *fnt);

static void initializeDb()
{
   QFontDatabasePrivate *db = privateDb();

   // init by asking for the platformfontdb for the first time or after invalidation
   if (db->m_families.empty()) {
      QGuiApplicationPrivate::platformIntegration()->fontDatabase()->populateFontDatabase();
   }

   if (db->reregisterAppFonts) {

      for (int i = 0; i < db->applicationFonts.count(); ++i) {
         if (! db->applicationFonts.at(i).families.isEmpty()) {
            registerFont(&db->applicationFonts[i]);
         }
      }

      db->reregisterAppFonts = false;
   }
}

static inline void loadDb(const QString & = QString(), int = -1)
{
   // Only initialize the database if it has been cleared or not initialized yet
   if (privateDb()->m_families.empty()) {
      initializeDb();
   }
}

static QFontEngine *loadSingleEngine(int script, const QFontDef &request,
            QtFontFamily *family, QtFontFoundry *foundry, QtFontStyle *style, QtFontSize *size)
{
   (void) foundry;

   Q_ASSERT(size);

   QPlatformFontDatabase *pfdb = QGuiApplicationPrivate::platformIntegration()->fontDatabase();

   int pixelSize = size->pixelSize;

   if (! pixelSize || (style->smoothScalable && pixelSize == SMOOTH_SCALABLE) || pfdb->fontsAlwaysScalable()) {
      pixelSize = request.pixelSize;
   }

   QFontDef def  = request;
   def.pixelSize = pixelSize;

   QFontCache *fontCache = QFontCache::instance();

   QFontCache::Key key(def, script);
   QFontEngine *engine = fontCache->findEngine(key);

   if (! engine) {
      const bool cacheForCommonScript = script != QChar::Script_Common &&
                  (family->writingSystems[QFontDatabase::Latin] & QtFontFamily::Supported) != 0;

      if (cacheForCommonScript) {
         // fast path: check if engine was loaded for another script
         key.script = QChar::Script_Common;
         engine     = fontCache->findEngine(key);
         key.script = script;

         if (engine) {
            // Also check for OpenType tables when using complex scripts
            if (! engine->supportsScript(QChar::Script(script))) {
               qWarning("loadSingleEngine() OpenType support missing for script %d", script);
               return nullptr;
            }

            engine->isSmoothlyScalable = style->smoothScalable;
            fontCache->insertEngine(key, engine);
            return engine;
         }
      }

      // If the font data's native stretch matches the requested stretch we need to set stretch to 100
      // to avoid the fontengine synthesizing stretch. If they didn't match exactly we need to calculate
      // the new stretch factor. This only done if not matched by styleName.
      if (style->key.stretch != 0 && request.stretch != 0 && (request.styleName.isEmpty() || request.styleName != style->styleName)) {
         def.stretch = (request.stretch * 100 + 50) / style->key.stretch;
      }

      engine = pfdb->fontEngine(def, size->handle.get());

      if (engine) {
         // Also check for OpenType tables when using complex scripts
         if (! engine->supportsScript(QChar::Script(script))) {
            qWarning("loadSingleEngine() OpenType support missing for script %d", script);

            if (engine->m_refCount.load() == 0) {
               delete engine;
            }

            return nullptr;
         }

         engine->isSmoothlyScalable = style->smoothScalable;
         fontCache->insertEngine(key, engine);

         if (cacheForCommonScript && ! engine->symbol) {
            // cache engine for Common script as well
            key.script = QChar::Script_Common;

            if (! fontCache->findEngine(key)) {
               fontCache->insertEngine(key, engine);
            }
         }
      }
   }

   return engine;
}

static QFontEngine *loadEngine(int script, const QFontDef &request, QtFontFamily *family,
            QtFontFoundry *foundry, QtFontStyle *style, QtFontSize *size)
{
   QFontEngine *engine = loadSingleEngine(script, request, family, foundry, style, size);

   if (engine && ! (request.styleStrategy & QFont::NoFontMerging) && ! engine->symbol) {
      QPlatformFontDatabase *pfdb     = QGuiApplicationPrivate::platformIntegration()->fontDatabase();
      QFontEngineMulti *pfMultiEngine = pfdb->fontEngineMulti(engine, QChar::Script(script));

      if (! request.fallBackFamilies.isEmpty()) {
         QStringList fallbacks = request.fallBackFamilies;

         QFont::StyleHint styleHint = QFont::StyleHint(request.styleHint);
         if (styleHint == QFont::AnyStyle && request.fixedPitch) {
            styleHint = QFont::TypeWriter;
         }

         fallbacks += fallbacksForFamily(family->m_name, QFont::Style(style->key.style), styleHint, QChar::Script(script));
         pfMultiEngine->setFallbackFamiliesList(fallbacks);
      }
      engine = pfMultiEngine;

      // Cache Multi font engine as well in case we got the single
      // font engine when we are actually looking for a Multi one
      QFontCache::Key key(request, script, 1);
      QFontCache::instance()->insertEngine(key, engine);
   }

   return engine;
}

static void registerFont(QFontDatabasePrivate::ApplicationFont *fnt)
{
   QFontDatabasePrivate *db = privateDb();

   fnt->families = QGuiApplicationPrivate::platformIntegration()->fontDatabase()->addApplicationFont(fnt->data, fnt->fileName);
   db->reregisterAppFonts = true;
}

static QtFontStyle *bestStyle(QtFontFoundry *foundry, const QtFontStyle::Key &styleKey, const QString &styleName = QString())
{
   QtFontStyle *retval = nullptr;
   int dist = 0xffff;

   for (auto &style : foundry->m_styles) {
      if (! styleName.isEmpty() && styleName == style.styleName) {
         dist   = 0;
         retval = &style;
         break;
      }

      int tmpDist = qAbs(styleKey.weight - style.key.weight);

      if (styleKey.stretch != 0 && style.key.stretch != 0) {
         tmpDist += qAbs(styleKey.stretch - style.key.stretch);
      }

      if (styleKey.style != style.key.style) {

         if (styleKey.style != QFont::StyleNormal && style.key.style != QFont::StyleNormal) {
            // one is italic, the other oblique
            tmpDist += 0x0001;

         } else {
            tmpDist += 0x1000;
         }
      }

      if (tmpDist < dist) {
         retval = &style;
         dist   = tmpDist;
      }
   }

#if defined(CS_SHOW_DEBUG_GUI_TEXT)
   qDebug("QtFontStyle::bestStyle() Style has a distance 0x%x", dist);
#endif

   return retval;
}

static unsigned int bestFoundry(int script, unsigned int score, int styleStrategy,
      QtFontFamily *family, const QString &foundryName, QtFontStyle::Key styleKey, int pixelSize,
      char pitch, QtFontDesc *desc, const QString &styleName = QString())
{
   (void) script;

   desc->foundry = nullptr;
   desc->style   = nullptr;
   desc->size    = nullptr;

   for (QtFontFoundry &fontFoundry : family->m_foundries) {
      if (! foundryName.isEmpty() && fontFoundry.m_fontMfg.compare(foundryName, Qt::CaseInsensitive) != 0) {
         continue;
      }

      QtFontStyle *style = bestStyle(&fontFoundry, styleKey, styleName);

      if (! style->smoothScalable && (styleStrategy & QFont::ForceOutline)) {
         continue;
      }

      int px = -1;

      QtFontSize *size = nullptr;

      // 1. see if we have an exact matching size
      if (! (styleStrategy & QFont::ForceOutline)) {
         size = style->fontSize(pixelSize);

         if (size) {
            px = size->pixelSize;
         }
      }

      // 2. see if we have a smoothly scalable font
      if (! size && style->smoothScalable && ! (styleStrategy & QFont::PreferBitmap)) {
         size = style->fontSize(SMOOTH_SCALABLE);

         if (size) {
#if defined(CS_SHOW_DEBUG_GUI_TEXT)
            qDebug("bestFoundry() Scalable font is %d pixels", pixelSize);
#endif

            px = pixelSize;
         }
      }

      // 3. see if we have a bitmap scalable font
      if (! size && style->bitmapScalable && (styleStrategy & QFont::PreferMatch)) {
         size = style->fontSize(0);

         if (size) {
#if defined(CS_SHOW_DEBUG_GUI_TEXT)
            qDebug("bestFoundry() Found bitmap scalable font of %d pixels", pixelSize);
#endif

            px = pixelSize;
         }
      }

      // 4. find closest size match
      if (! size) {
         uint distance = ~0u;

         for (auto &fontSize : style->m_sizes) {
            uint tmpDist;

            if (fontSize.pixelSize < pixelSize) {
               // penalize sizes that are smaller than the requested size,
               // due to truncation from floating point to integer conversions
               tmpDist = pixelSize - fontSize.pixelSize + 1;

            } else {
               tmpDist = fontSize.pixelSize - pixelSize;
            }

            if (tmpDist < distance) {
               distance = tmpDist;
               size     = &fontSize;

#if defined(CS_SHOW_DEBUG_GUI_TEXT)
               qDebug("bestFoundry() Best size so far is %3d (%d)", size->pixelSize, pixelSize);
#endif
            }
         }

         if (! size) {
#if defined(CS_SHOW_DEBUG_GUI_TEXT)
            qDebug("bestFoundry() No size supports requiested script");
#endif
            continue;
         }

         if (style->bitmapScalable && ! (styleStrategy & QFont::PreferQuality) && (distance * 10 / pixelSize) >= 2) {
            // closest size is not close enough, go ahead and use a bitmap scaled font
            size = style->fontSize(0);
            px   = pixelSize;

         } else {
            px = size->pixelSize;
         }
      }

      uint currentScore = 0x0000;

      enum {
         PitchMismatch       = 0x4000,
         StyleMismatch       = 0x2000,
         BitmapScaledPenalty = 0x1000,
      };

      if (pitch != '*') {
         if ((pitch == 'm' && ! family->m_fixedPitch) || (pitch == 'p' && family->m_fixedPitch)) {
            currentScore += PitchMismatch;
         }
      }

      if (styleKey != style->key) {
         currentScore += StyleMismatch;
      }

      if (! style->smoothScalable && px != size->pixelSize) {
         // bitmap scaled
         currentScore += BitmapScaledPenalty;
      }

      if (px != pixelSize) {
         // close, but not exact, size match
         currentScore += qAbs(px - pixelSize);
      }

      if (currentScore < score) {
#if defined(CS_SHOW_DEBUG_GUI_TEXT)
         qDebug("bestFoundry() Found a match, score %x best score so far %x", currentScore, score);
#endif

         score         = currentScore;
         desc->foundry = &fontFoundry;
         desc->style   = style;
         desc->size    = size;

      } else {
#if defined(CS_SHOW_DEBUG_GUI_TEXT)
         qDebug("bestFoundry() Score %x no better than best %x", currentScore, score);
#endif

      }
   }

   return score;
}

static bool matchFamilyName(const QString &familyName, QtFontFamily *family)
{
   if (familyName.isEmpty()) {
      return true;
   }

   return family->matchesFamilyName(familyName);
}

QtFontFamily *match(int script, const QFontDef &request, const QString &familyName, const QString &foundryName,
      QtFontDesc *desc, const QList<QtFontFamily *> &blacklistedFamilies)
{
   QtFontFamily *retval = nullptr;

   QtFontStyle::Key styleKey;
   styleKey.style   = request.style;
   styleKey.weight  = request.weight;
   styleKey.stretch = request.stretch;

   char pitch = 'p';

   if (request.ignorePitch) {
      pitch = '*';
   } else if (request.fixedPitch) {
      pitch = 'm';
   }

   desc->family  = nullptr;
   desc->foundry = nullptr;
   desc->style   = nullptr;
   desc->size    = nullptr;

   unsigned int score = ~0u;

   loadDb(familyName, script);

   size_t writingSystem = std::find(scriptForWritingSystem, scriptForWritingSystem +
         QFontDatabase::WritingSystemsCount, script) - scriptForWritingSystem;

   if (writingSystem >= QFontDatabase::WritingSystemsCount) {
      writingSystem = QFontDatabase::Any;
   }

   for (auto &family : privateDb()->m_families) {
      if (blacklistedFamilies.contains(&family)) {
         continue;
      }

      QtFontDesc test;
      test.family = &family;

      if (! matchFamilyName(familyName, test.family)) {
         continue;
      }

      test.family->ensurePopulated();

      if (writingSystem != QFontDatabase::Any && ! (test.family->writingSystems[writingSystem] & QtFontFamily::Supported)) {
         continue;
      }

      // as we know the script is supported, we can be sure to find a matching font here.
      uint newscore = bestFoundry(script, score, request.styleStrategy,
                  test.family, foundryName, styleKey, request.pixelSize, pitch, &test, request.styleName);

      if (test.foundry == nullptr && ! foundryName.isEmpty()) {
         // the specific foundry was not found, so look for
         // any foundry matching our requirements

         newscore = bestFoundry(script, score, request.styleStrategy, test.family,
                  QString(), styleKey, request.pixelSize, pitch, &test, request.styleName);
      }

      if (newscore < score) {
         retval = &family;
         score  = newscore;
         *desc  = test;
      }

      if (newscore < 10) {
         // xlfd instead of FT, just accept it
         break;
      }
   }

   return retval;
}

static QString styleStringHelper(int weight, QFont::Style style)
{
   QString retval;

   if (weight > QFont::Normal) {
      if (weight >= QFont::Black) {
         retval = QCoreApplication::translate("QFontDatabase", "Black");

      } else if (weight >= QFont::ExtraBold) {
         retval = QCoreApplication::translate("QFontDatabase", "Extra Bold");

      } else if (weight >= QFont::Bold) {
         retval = QCoreApplication::translate("QFontDatabase", "Bold");

      } else if (weight >= QFont::DemiBold) {
         retval = QCoreApplication::translate("QFontDatabase", "Demi Bold");

      } else if (weight >= QFont::Medium) {
         retval = QCoreApplication::translate("QFontDatabase", "Medium", "Medium font weight");
      }

   } else {

      if (weight <= QFont::Thin) {
         retval = QCoreApplication::translate("QFontDatabase", "Thin");

      } else if (weight <= QFont::ExtraLight) {
         retval = QCoreApplication::translate("QFontDatabase", "Extra Light");

      } else if (weight <= QFont::Light) {
         retval = QCoreApplication::translate("QFontDatabase", "Light");
      }
   }

   if (style == QFont::StyleItalic) {
      retval += ' ' + QCoreApplication::translate("QFontDatabase", "Italic");

   } else if (style == QFont::StyleOblique) {
      retval += ' ' + QCoreApplication::translate("QFontDatabase", "Oblique");
   }

   if (retval.isEmpty()) {
      retval = QCoreApplication::translate("QFontDatabase", "Normal", "Normal or Regular font weight");
   }

   return retval.simplified();
}

QString QFontDatabase::styleString(const QFont &font) const
{
   return font.styleName().isEmpty()
                  ? styleStringHelper(font.weight(), font.style()) : font.styleName();
}

QString QFontDatabase::styleString(const QFontInfo &fontInfo) const
{
   return fontInfo.styleName().isEmpty()
                  ? styleStringHelper(fontInfo.weight(), fontInfo.style()) : fontInfo.styleName();
}

QFontDatabase::QFontDatabase()
{
   if (! qApp || ! QGuiApplicationPrivate::platformIntegration()) {
      qFatal("QFontDatabase: Must construct a QApplication before accessing QFontDatabase");
   }

   QRecursiveMutexLocker locker(fontDatabaseMutex());
   createDatabase();
   m_fontdatabase = privateDb();
}

QList<QFontDatabase::WritingSystem> QFontDatabase::writingSystems() const
{
   QRecursiveMutexLocker locker(fontDatabaseMutex());
   loadDb();

   quint64 writingSystemsFound = 0;
   static_assert(WritingSystemsCount < 64, "Count must be less than 64");

   for (auto &family : m_fontdatabase->m_families) {
      family.ensurePopulated();

      if (family.m_foundries.isEmpty()) {
         continue;
      }

      for (uint index = QFontDatabase::Latin; index < uint(WritingSystemsCount); ++index) {
         if (family.writingSystems[index] & QtFontFamily::Supported) {
            writingSystemsFound |= quint64(1) << index;
         }
      }
   }

   // done with the lock, working on local data now
   locker.unlock();

   QList<WritingSystem> list;

   for (uint index = QFontDatabase::Latin; index < uint(WritingSystemsCount); ++index) {
      if (writingSystemsFound & (quint64(1) << index)) {
         list.push_back(WritingSystem(index));
      }
   }

   return list;
}

QList<QFontDatabase::WritingSystem> QFontDatabase::writingSystems(const QString &family) const
{
   QString familyName;
   QString foundryName;

   parseFontName(family, foundryName, familyName);

   QRecursiveMutexLocker locker(fontDatabaseMutex());
   loadDb();

   QList<WritingSystem> list;
   const QtFontFamily *fontFamily = m_fontdatabase->family(familyName);

   if (fontFamily == nullptr || fontFamily->m_foundries.size() == 0) {
      return list;
   }

   for (int x = QFontDatabase::Latin; x < WritingSystemsCount; ++x) {
      const WritingSystem writingSystem = WritingSystem(x);

      if (fontFamily->writingSystems[writingSystem] & QtFontFamily::Supported) {
         list.append(writingSystem);
      }
   }

   return list;
}

QStringList QFontDatabase::families(WritingSystem writingSystem) const
{
   QRecursiveMutexLocker locker(fontDatabaseMutex());
   loadDb();

   QStringList familyList;

   for (auto &fontFamily : m_fontdatabase->m_families) {

      if (fontFamily.m_populated && fontFamily.m_foundries.size() == 0) {
         continue;
      }

      if (writingSystem != Any) {
         fontFamily.ensurePopulated();

         if (fontFamily.writingSystems[writingSystem] != QtFontFamily::Supported) {
            continue;
         }
      }

      if (! fontFamily.m_populated || fontFamily.m_foundries.size() == 1) {
         familyList.append(fontFamily.m_name);

      } else {
         for (const auto &fontFoundry : fontFamily.m_foundries) {
            QString str     = fontFamily.m_name;
            QString foundry = fontFoundry.m_fontMfg;

            if (! foundry.isEmpty()) {
               str += " [" + foundry + "]";
            }

            familyList.append(str);
         }
      }
   }

   std::sort(familyList.begin(), familyList.end());

   return familyList;
}

QStringList QFontDatabase::styles(const QString &family) const
{
   QString familyName;
   QString foundryName;

   parseFontName(family, foundryName, familyName);

   QRecursiveMutexLocker locker(fontDatabaseMutex());
   loadDb(familyName);

   const QtFontFamily *fontFamily = m_fontdatabase->family(familyName);

   if (fontFamily == nullptr) {
      return QStringList();
   }

   QtFontFoundry allStyles(foundryName);

   for (const auto &fontFoundry : fontFamily->m_foundries) {
      if (foundryName.isEmpty() || fontFoundry.m_fontMfg.compare(foundryName, Qt::CaseInsensitive) == 0) {

         for (const auto &style : fontFoundry.m_styles) {
            QtFontStyle::Key key(style.key);
            key.stretch = 0;
            allStyles.style(key, style.styleName, true);
         }
      }
   }

   QStringList retval;
   for (const auto &style : allStyles.m_styles) {
      retval.append(style.styleName.isEmpty() ? styleStringHelper(style.key.weight,
                  (QFont::Style) style.key.style) : style.styleName);
   }

   std::sort(retval.begin(), retval.end());

   return retval;
}

bool QFontDatabase::isFixedPitch(const QString &family, const QString &style) const
{
   (void) style;

   QString familyName;
   QString foundryName;

   parseFontName(family, foundryName, familyName);

   QRecursiveMutexLocker locker(fontDatabaseMutex());
   loadDb(familyName);

   const QtFontFamily *fontFamily = m_fontdatabase->family(familyName);
   return (fontFamily && fontFamily->m_fixedPitch);
}

bool QFontDatabase::isBitmapScalable(const QString &family, const QString &styleName) const
{
   bool bitmapScalable = false;

   QString familyName;
   QString foundryName;
   parseFontName(family, foundryName, familyName);

   QRecursiveMutexLocker locker(fontDatabaseMutex());
   loadDb(familyName);

   const QtFontFamily *fontFamily = m_fontdatabase->family(familyName);
   if (! fontFamily) {
      return bitmapScalable;
   }

   QtFontStyle::Key styleKey(styleName);

   for (const auto &fontFoundry : fontFamily->m_foundries) {
      if (foundryName.isEmpty() || fontFoundry.m_fontMfg.compare(foundryName, Qt::CaseInsensitive) == 0) {

         for (const auto &style : fontFoundry.m_styles) {
            if ((styleName.isEmpty() || style.styleName == styleName || style.key == styleKey) &&
                  style.bitmapScalable && ! style.smoothScalable) {

               bitmapScalable = true;
               goto end;
            }
         }
      }
   }

end:
   return bitmapScalable;
}

bool QFontDatabase::isSmoothlyScalable(const QString &family, const QString &styleName) const
{
   bool smoothScalable = false;

   QString familyName;
   QString foundryName;

   parseFontName(family, foundryName, familyName);

   QRecursiveMutexLocker locker(fontDatabaseMutex());
   loadDb(familyName);

   QtFontFamily *fontFamily = m_fontdatabase->family(familyName);

   if (! fontFamily) {
      for (auto &family : m_fontdatabase->m_families) {
         if (family.matchesFamilyName(familyName)) {
            fontFamily = &family;
            fontFamily->ensurePopulated();
            break;
         }
      }
   }

   if (! fontFamily) {
      return smoothScalable;
   }

   QtFontStyle::Key styleKey(styleName);

   for (const auto &fontFoundry : fontFamily->m_foundries) {
      if (foundryName.isEmpty() || fontFoundry.m_fontMfg.compare(foundryName, Qt::CaseInsensitive) == 0) {

         for (const auto &style : fontFoundry.m_styles) {
            if ((styleName.isEmpty() || style.styleName == styleName || style.key == styleKey) && style.smoothScalable) {
               smoothScalable = true;
               goto end;
            }
         }
      }
   }

end:
   return smoothScalable;
}

bool QFontDatabase::isScalable(const QString &family, const QString &style) const
{
   QRecursiveMutexLocker locker(fontDatabaseMutex());

   if (isSmoothlyScalable(family, style)) {
      return true;
   }

   return isBitmapScalable(family, style);
}

QList<int> QFontDatabase::pointSizes(const QString &family, const QString &styleName)
{
   if (QGuiApplicationPrivate::platformIntegration()->fontDatabase()->fontsAlwaysScalable()) {
      // smoothly scalable
      return standardSizes();
   }

   bool smoothScalable = false;
   QString familyName;
   QString foundryName;
   parseFontName(family, foundryName, familyName);

   QRecursiveMutexLocker locker(fontDatabaseMutex());
   loadDb(familyName);

   QList<int> sizes;

   QtFontFamily *fontFamily = m_fontdatabase->family(familyName);
   if (! fontFamily) {
      return sizes;
   }

   const int dpi = qt_defaultDpiY();      // embedded
   QtFontStyle::Key styleKey(styleName);

   for (auto &fontFoundry : fontFamily->m_foundries) {
      if (foundryName.isEmpty() || fontFoundry.m_fontMfg.compare(foundryName, Qt::CaseInsensitive) == 0) {
         QtFontStyle *style = fontFoundry.style(styleKey, styleName);

         if (! style) {
            continue;
         }

         if (style->smoothScalable) {
            smoothScalable = true;
            break;
         }

         for (const auto &fontSize : style->m_sizes) {
            if (fontSize.pixelSize != 0 && fontSize.pixelSize != SMOOTH_SCALABLE) {
               const uint pointSize = qRound(fontSize.pixelSize * 72.0 / dpi);

               if (! sizes.contains(pointSize)) {
                  sizes.append(pointSize);
               }
            }
         }
      }
   }

   if (smoothScalable) {
      return standardSizes();
   }

   std::sort(sizes.begin(), sizes.end());

   return sizes;
}

QFont QFontDatabase::font(const QString &family, const QString &styleName, int pointSize) const
{
   QString familyName;
   QString foundryName;

   parseFontName(family, foundryName, familyName);

   QRecursiveMutexLocker locker(fontDatabaseMutex());
   loadDb(familyName);

   QtFontFoundry allStyles(foundryName);
   const QtFontFamily *fontFamily = m_fontdatabase->family(familyName);

   if (fontFamily == nullptr) {
      return QGuiApplication::font();
   }

   for (const auto &item : fontFamily->m_foundries) {
      if (foundryName.isEmpty() || item.m_fontMfg.compare(foundryName, Qt::CaseInsensitive) == 0) {

         for (const auto &style : item.m_styles) {
            allStyles.style(style.key, style.styleName, true);
         }
      }
   }

   QtFontStyle::Key styleKey(styleName);
   const QtFontStyle *style = bestStyle(&allStyles, styleKey, styleName);

   if (style == nullptr) {
      // no styles found
      return QGuiApplication::font();
   }

   QFont font(family, pointSize, style->key.weight);
   font.setStyle((QFont::Style) style->key.style);

   if (! style->styleName.isEmpty()) {
      font.setStyleName(style->styleName);
   }

   return font;
}

QList<int> QFontDatabase::smoothSizes(const QString &family, const QString &styleName)
{
   if (QApplicationPrivate::platformIntegration()->fontDatabase()->fontsAlwaysScalable()) {
      return standardSizes();
   }

   bool smoothScalable = false;
   QString familyName;
   QString foundryName;

   parseFontName(family, foundryName, familyName);

   QRecursiveMutexLocker locker(fontDatabaseMutex());

   loadDb(familyName);

   QList<int> sizes;

   QtFontFamily *fontFamily = m_fontdatabase->family(familyName);
   if (fontFamily == nullptr) {
      return sizes;
   }

   const int dpi = qt_defaultDpiY();      // embedded

   QtFontStyle::Key styleKey(styleName);

   for (auto &fontFoundry : fontFamily->m_foundries) {
      if (foundryName.isEmpty() || fontFoundry.m_fontMfg.compare(foundryName, Qt::CaseInsensitive) == 0) {
         QtFontStyle *style = fontFoundry.style(styleKey, styleName);

         if (style  == nullptr) {
            continue;
         }

         if (style->smoothScalable) {
            smoothScalable = true;
            goto end;
         }

         for (const auto &fontSize : style->m_sizes) {
            if (fontSize.pixelSize != 0 && fontSize.pixelSize != SMOOTH_SCALABLE) {
               const uint pointSize = qRound(fontSize.pixelSize * 72.0 / dpi);

               if (! sizes.contains(pointSize)) {
                  sizes.append(pointSize);
               }
            }
         }
      }
   }

end:
   if (smoothScalable) {
      return QFontDatabase::standardSizes();
   }

   std::sort(sizes.begin(), sizes.end());

   return sizes;
}

QList<int> QFontDatabase::standardSizes()
{
   return QGuiApplicationPrivate::platformIntegration()->fontDatabase()->standardSizes();
}

bool QFontDatabase::italic(const QString &family, const QString &style) const
{
   QString familyName, foundryName;
   parseFontName(family, foundryName, familyName);

   QRecursiveMutexLocker locker(fontDatabaseMutex());
   loadDb(familyName);

   QtFontFoundry allStyles(foundryName);
   const QtFontFamily *fontFamily = m_fontdatabase->family(familyName);

   if (fontFamily == nullptr) {
      return false;
   }

   for (auto &fontFoundry : fontFamily->m_foundries) {
      if (foundryName.isEmpty() || fontFoundry.m_fontMfg.compare(foundryName, Qt::CaseInsensitive) == 0) {
         for (const auto &style : fontFoundry.m_styles) {
            allStyles.style(style.key, style.styleName, true);
         }
      }
   }

   QtFontStyle::Key styleKey(style);
   const QtFontStyle *fontStyle = allStyles.style(styleKey, style);

   return fontStyle != nullptr && (fontStyle->key.style == QFont::StyleItalic);
}

bool QFontDatabase::bold(const QString &family, const QString &style) const
{
   QString familyName, foundryName;
   parseFontName(family, foundryName, familyName);

   QRecursiveMutexLocker locker(fontDatabaseMutex());
   loadDb(familyName);

   QtFontFoundry allStyles(foundryName);
   const QtFontFamily *fontFamily = m_fontdatabase->family(familyName);

   if (fontFamily == nullptr) {
      return false;
   }

   for (auto &fontFoundry : fontFamily->m_foundries) {
      if (foundryName.isEmpty() || fontFoundry.m_fontMfg.compare(foundryName, Qt::CaseInsensitive) == 0) {
         for (const auto &style : fontFoundry.m_styles) {
            allStyles.style(style.key, style.styleName, true);
         }
      }
   }

   QtFontStyle::Key styleKey(style);
   const QtFontStyle *fontStyle  = allStyles.style(styleKey, style);

   return fontStyle != nullptr && (fontStyle ->key.weight >= QFont::Bold);
}

int QFontDatabase::weight(const QString &family, const QString &style) const
{
   QString familyName, foundryName;
   parseFontName(family, foundryName, familyName);

   QRecursiveMutexLocker locker(fontDatabaseMutex());
   loadDb(familyName);

   QtFontFoundry allStyles(foundryName);
   const QtFontFamily *fontFamily = m_fontdatabase->family(familyName);

   if (fontFamily == nullptr) {
      return -1;
   }

   for (auto &fontFoundry : fontFamily->m_foundries) {
      if (foundryName.isEmpty() || fontFoundry.m_fontMfg.compare(foundryName, Qt::CaseInsensitive) == 0) {
         for (const auto &style : fontFoundry.m_styles) {
            allStyles.style(style.key, style.styleName, true);
         }
      }
   }

   QtFontStyle::Key styleKey(style);
   const QtFontStyle *fontStyle = allStyles.style(styleKey, style);

   return fontStyle == nullptr ? -1 : fontStyle->key.weight;
}

// internal
bool QFontDatabase::hasFamily(const QString &family) const
{
   QString parsedFamily, foundry;
   parseFontName(family, foundry, parsedFamily);
   const QString familyAlias = resolveFontFamilyAlias(parsedFamily);

   return families().contains(familyAlias, Qt::CaseInsensitive);
}

bool QFontDatabase::isPrivateFamily(const QString &family) const
{
   return QGuiApplicationPrivate::platformIntegration()->fontDatabase()->isPrivateFontFamily(family);
}

QString QFontDatabase::writingSystemName(WritingSystem writingSystem)
{
   QString name;

   switch (writingSystem) {
      case Any:
         name = cs_mark_tr("QFontDatabase", "Any");
         break;

      case Latin:
         name = cs_mark_tr("QFontDatabase", "Latin");
         break;

      case Greek:
         name = cs_mark_tr("QFontDatabase", "Greek");
         break;

      case Cyrillic:
         name = cs_mark_tr("QFontDatabase", "Cyrillic");
         break;

      case Armenian:
         name = cs_mark_tr("QFontDatabase", "Armenian");
         break;

      case Hebrew:
         name = cs_mark_tr("QFontDatabase", "Hebrew");
         break;

      case Arabic:
         name = cs_mark_tr("QFontDatabase", "Arabic");
         break;

      case Syriac:
         name = cs_mark_tr("QFontDatabase", "Syriac");
         break;

      case Thaana:
         name = cs_mark_tr("QFontDatabase", "Thaana");
         break;

      case Devanagari:
         name = cs_mark_tr("QFontDatabase", "Devanagari");
         break;

      case Bengali:
         name = cs_mark_tr("QFontDatabase", "Bengali");
         break;

      case Gurmukhi:
         name = cs_mark_tr("QFontDatabase", "Gurmukhi");
         break;

      case Gujarati:
         name = cs_mark_tr("QFontDatabase", "Gujarati");
         break;

      case Oriya:
         name = cs_mark_tr("QFontDatabase", "Oriya");
         break;

      case Tamil:
         name = cs_mark_tr("QFontDatabase", "Tamil");
         break;

      case Telugu:
         name = cs_mark_tr("QFontDatabase", "Telugu");
         break;

      case Kannada:
         name = cs_mark_tr("QFontDatabase", "Kannada");
         break;

      case Malayalam:
         name = cs_mark_tr("QFontDatabase", "Malayalam");
         break;

      case Sinhala:
         name = cs_mark_tr("QFontDatabase", "Sinhala");
         break;

      case Thai:
         name = cs_mark_tr("QFontDatabase", "Thai");
         break;
      case Lao:
         name = cs_mark_tr("QFontDatabase", "Lao");
         break;

      case Tibetan:
         name = cs_mark_tr("QFontDatabase", "Tibetan");
         break;

      case Myanmar:
         name = cs_mark_tr("QFontDatabase", "Myanmar");
         break;

      case Georgian:
         name = cs_mark_tr("QFontDatabase", "Georgian");
         break;

      case Khmer:
         name = cs_mark_tr("QFontDatabase", "Khmer");
         break;

      case SimplifiedChinese:
         name = cs_mark_tr("QFontDatabase", "Simplified Chinese");
         break;

      case TraditionalChinese:
         name = cs_mark_tr("QFontDatabase", "Traditional Chinese");
         break;

      case Japanese:
         name = cs_mark_tr("QFontDatabase", "Japanese");
         break;

      case Korean:
         name = cs_mark_tr("QFontDatabase", "Korean");
         break;

      case Vietnamese:
         name = cs_mark_tr("QFontDatabase", "Vietnamese");
         break;

      case Symbol:
         name = cs_mark_tr("QFontDatabase", "Symbol");
         break;

      case Ogham:
         name = cs_mark_tr("QFontDatabase", "Ogham");
         break;

      case Runic:
         name = cs_mark_tr("QFontDatabase", "Runic");
         break;

      case Nko:
         name = cs_mark_tr("QFontDatabase", "N'Ko");
         break;

      default:
         Q_ASSERT_X(false, "QFontDatabase::writingSystemName()", "Invalid writingSystem parameter");
         break;
   }

   return QCoreApplication::translate("QFontDatabase", name.constData());
}

QString QFontDatabase::writingSystemSample(WritingSystem writingSystem)
{
   QString sample;

   switch (writingSystem) {
      case Any:
      case Symbol:
         // show only ascii characters
         sample += "AaBbzZ";
         break;

      case Latin:
         sample = "Aa";
         sample += QChar(0x00C3);
         sample += QChar(0x00E1);
         sample += "Zz";
         break;

      case Greek:
         sample += QChar(0x0393);
         sample += QChar(0x03B1);
         sample += QChar(0x03A9);
         sample += QChar(0x03C9);
         break;

      case Cyrillic:
         sample += QChar(0x0414);
         sample += QChar(0x0434);
         sample += QChar(0x0436);
         sample += QChar(0x044f);
         break;

      case Armenian:
         sample += QChar(0x053f);
         sample += QChar(0x054f);
         sample += QChar(0x056f);
         sample += QChar(0x057f);
         break;

      case Hebrew:
         sample += QChar(0x05D0);
         sample += QChar(0x05D1);
         sample += QChar(0x05D2);
         sample += QChar(0x05D3);
         break;

      case Arabic:
         sample += QChar(0x0628);
         sample += QChar(0x0629);
         sample += QChar(0x062A);
         sample += QChar(0x063A);
         break;
      case Syriac:
         sample += QChar(0x0715);
         sample += QChar(0x0725);
         sample += QChar(0x0716);
         sample += QChar(0x0726);
         break;

      case Thaana:
         sample += QChar(0x0784);
         sample += QChar(0x0794);
         sample += QChar(0x078c);
         sample += QChar(0x078d);
         break;

      case Devanagari:
         sample += QChar(0x0905);
         sample += QChar(0x0915);
         sample += QChar(0x0925);
         sample += QChar(0x0935);
         break;

      case Bengali:
         sample += QChar(0x0986);
         sample += QChar(0x0996);
         sample += QChar(0x09a6);
         sample += QChar(0x09b6);
         break;

      case Gurmukhi:
         sample += QChar(0x0a05);
         sample += QChar(0x0a15);
         sample += QChar(0x0a25);
         sample += QChar(0x0a35);
         break;

      case Gujarati:
         sample += QChar(0x0a85);
         sample += QChar(0x0a95);
         sample += QChar(0x0aa5);
         sample += QChar(0x0ab5);
         break;

      case Oriya:
         sample += QChar(0x0b06);
         sample += QChar(0x0b16);
         sample += QChar(0x0b2b);
         sample += QChar(0x0b36);
         break;

      case Tamil:
         sample += QChar(0x0b89);
         sample += QChar(0x0b99);
         sample += QChar(0x0ba9);
         sample += QChar(0x0bb9);
         break;

      case Telugu:
         sample += QChar(0x0c05);
         sample += QChar(0x0c15);
         sample += QChar(0x0c25);
         sample += QChar(0x0c35);
         break;

      case Kannada:
         sample += QChar(0x0c85);
         sample += QChar(0x0c95);
         sample += QChar(0x0ca5);
         sample += QChar(0x0cb5);
         break;

      case Malayalam:
         sample += QChar(0x0d05);
         sample += QChar(0x0d15);
         sample += QChar(0x0d25);
         sample += QChar(0x0d35);
         break;

      case Sinhala:
         sample += QChar(0x0d90);
         sample += QChar(0x0da0);
         sample += QChar(0x0db0);
         sample += QChar(0x0dc0);
         break;

      case Thai:
         sample += QChar(0x0e02);
         sample += QChar(0x0e12);
         sample += QChar(0x0e22);
         sample += QChar(0x0e32);
         break;

      case Lao:
         sample += QChar(0x0e8d);
         sample += QChar(0x0e9d);
         sample += QChar(0x0ead);
         sample += QChar(0x0ebd);
         break;

      case Tibetan:
         sample += QChar(0x0f00);
         sample += QChar(0x0f01);
         sample += QChar(0x0f02);
         sample += QChar(0x0f03);
         break;

      case Myanmar:
         sample += QChar(0x1000);
         sample += QChar(0x1001);
         sample += QChar(0x1002);
         sample += QChar(0x1003);
         break;

      case Georgian:
         sample += QChar(0x10a0);
         sample += QChar(0x10b0);
         sample += QChar(0x10c0);
         sample += QChar(0x10d0);
         break;

      case Khmer:
         sample += QChar(0x1780);
         sample += QChar(0x1790);
         sample += QChar(0x17b0);
         sample += QChar(0x17c0);
         break;

      case SimplifiedChinese:
         sample += QChar(0x4e2d);
         sample += QChar(0x6587);
         sample += QChar(0x8303);
         sample += QChar(0x4f8b);
         break;

      case TraditionalChinese:
         sample += QChar(0x4e2d);
         sample += QChar(0x6587);
         sample += QChar(0x7bc4);
         sample += QChar(0x4f8b);
         break;

      case Japanese:
         sample += QChar(0x30b5);
         sample += QChar(0x30f3);
         sample += QChar(0x30d7);
         sample += QChar(0x30eb);
         sample += QChar(0x3067);
         sample += QChar(0x3059);
         break;

      case Korean:
         sample += QChar(0xac00);
         sample += QChar(0xac11);
         sample += QChar(0xac1a);
         sample += QChar(0xac2f);
         break;

      case Vietnamese: {
         sample += QChar(0xFEFF);
         sample += QChar(0x1ED7);
         sample += QChar(0x1ED9);
         sample += QChar(0x1ED1);
         sample += QChar(0x1ED3);
         break;
      }

      case Ogham:
         sample += QChar(0x1681);
         sample += QChar(0x1682);
         sample += QChar(0x1683);
         sample += QChar(0x1684);
         break;

      case Runic:
         sample += QChar(0x16a0);
         sample += QChar(0x16a1);
         sample += QChar(0x16a2);
         sample += QChar(0x16a3);
         break;

      case Nko:
         sample += QChar(0x7ca);
         sample += QChar(0x7cb);
         sample += QChar(0x7cc);
         sample += QChar(0x7cd);
         break;

      default:
         break;
   }

   return sample;
}

void QFontDatabase::parseFontName(const QString &name, QString &foundry, QString &family)
{
   int indexFirst = name.indexOf('[');
   int indexLast  = name.lastIndexOf(']');

   if (indexFirst >= 0 && indexLast >= 0 && indexFirst < indexLast) {
      foundry = name.mid(indexFirst + 1, indexLast - indexFirst - 1);

      if (indexFirst > 0 && name[indexFirst - 1] == ' ') {
         --indexFirst;
      }
      family = name.left(indexFirst);

   } else {
      foundry.clear();
      family = name;
   }

   // capitalize the family names
   bool isSpace = true;
   QString tmpStr;

   for (const auto &ch : family) {
      if (isSpace) {
         tmpStr.append(ch.toUpper());

      } else {
         tmpStr.append(ch);
      }

      isSpace = ch.isSpace();
   }

   family = tmpStr;

   // capitalize the foundry names
   isSpace = true;
   tmpStr  = "";

   for (const auto &ch : foundry) {
      if (isSpace) {
         tmpStr.append(ch.toUpper());

      } else {
         tmpStr.append(ch);
      }

      isSpace = ch.isSpace();
   }

   foundry = tmpStr;
}

void QFontDatabase::createDatabase()
{
   initializeDb();
}

// used from qfontengine_ft.cpp
Q_GUI_EXPORT QByteArray qt_fontdata_from_index(int index)
{
   QRecursiveMutexLocker locker(fontDatabaseMutex());
   return privateDb()->applicationFonts.value(index).data;
}

int QFontDatabasePrivate::addAppFont(const QByteArray &fontData, const QString &fileName)
{
   QFontDatabasePrivate::ApplicationFont font;
   font.data     = fontData;
   font.fileName = fileName;

   int index;

   for (index = 0; index < applicationFonts.count(); ++index)
      if (applicationFonts.at(index).families.isEmpty()) {
         break;
      }

   if (index >= applicationFonts.count()) {
      applicationFonts.append(ApplicationFont());
      index = applicationFonts.count() - 1;
   }

   if (font.fileName.isEmpty() && ! fontData.isEmpty()) {
      font.fileName = ":qmemoryfonts/" + QString::number(index);
   }

   registerFont(&font);
   if (font.families.isEmpty()) {
      return -1;
   }

   applicationFonts[index] = font;
   invalidate();

   return index;
}

bool QFontDatabasePrivate::isApplicationFont(const QString &fileName)
{
   for (int i = 0; i < applicationFonts.count(); ++i) {
      if (applicationFonts.at(i).fileName == fileName) {
         return true;
      }
   }

   return false;
}

int QFontDatabase::addApplicationFont(const QString &fileName)
{
   QByteArray data;

   if (! QFileInfo(fileName).isNativePath()) {
      QFile file(fileName);

      if (! file.open(QIODevice::ReadOnly)) {
         return -1;
      }

      data = file.readAll();
   }

   QRecursiveMutexLocker locker(fontDatabaseMutex());

   return privateDb()->addAppFont(data, fileName);
}

int QFontDatabase::addApplicationFontFromData(const QByteArray &fontData)
{
   QRecursiveMutexLocker locker(fontDatabaseMutex());
   return privateDb()->addAppFont(fontData, QString());
}

QStringList QFontDatabase::applicationFontFamilies(int id)
{
   QRecursiveMutexLocker locker(fontDatabaseMutex());
   return privateDb()->applicationFonts.value(id).families;
}

QFont QFontDatabase::systemFont(QFontDatabase::SystemFont type)
{
   const QFont *font = nullptr;

   const QPlatformTheme *theme = QGuiApplicationPrivate::platformTheme();

   if (theme != nullptr) {

      switch (type) {
         case GeneralFont:
            font = theme->font(QPlatformTheme::SystemFont);
            break;

         case FixedFont:
            font = theme->font(QPlatformTheme::FixedFont);
            break;

         case TitleFont:
            font = theme->font(QPlatformTheme::TitleBarFont);
            break;

         case SmallestReadableFont:
            font = theme->font(QPlatformTheme::MiniFont);
            break;
      }
   }

   if (font != nullptr) {
      return *font;

   } else if (QPlatformIntegration *integration = QGuiApplicationPrivate::platformIntegration()) {
      return integration->fontDatabase()->defaultFont();

   } else {
      return QFont();
   }
}

bool QFontDatabase::removeApplicationFont(int handle)
{
   QRecursiveMutexLocker locker(fontDatabaseMutex());

   QFontDatabasePrivate *db = privateDb();
   if (handle < 0 || handle >= db->applicationFonts.count()) {
      return false;
   }

   db->applicationFonts[handle] = QFontDatabasePrivate::ApplicationFont();

   db->reregisterAppFonts = true;
   db->invalidate();

   return true;
}

bool QFontDatabase::removeAllApplicationFonts()
{
   QRecursiveMutexLocker locker(fontDatabaseMutex());

   QFontDatabasePrivate *db = privateDb();
   if (db->applicationFonts.isEmpty()) {
      return false;
   }

   db->applicationFonts.clear();
   db->invalidate();

   return true;
}

QFontEngine *QFontDatabase::findFont(const QFontDef &request, int script)
{
   QRecursiveMutexLocker locker(fontDatabaseMutex());

   if (privateDb()->m_families.empty()) {
      initializeDb();
   }

   QFontEngine *engine;

   QFontCache *fontCache = QFontCache::instance();

   // Until we specifically asked not to, try looking for Multi font engine first,
   // the last '1' indicates that we want Multi font engine instead of single ones
   bool multi = ! (request.styleStrategy & QFont::NoFontMerging);
   QFontCache::Key key(request, script, multi ? 1 : 0);
   engine = fontCache->findEngine(key);

   if (engine != nullptr) {
#if defined(CS_SHOW_DEBUG_GUI_TEXT)
      qDebug("QFontDatabase::findFont() Cache hit level 1");
#endif

      return engine;
   }

   QString family_name;
   QString foundry_name;

   parseFontName(request.family, foundry_name, family_name);

   QtFontDesc desc;
   QList<QtFontFamily *> blackListed;
   auto fontFamily = match(script, request, family_name, foundry_name, &desc, blackListed);

   if (fontFamily != nullptr) {
      engine = loadEngine(script, request, desc.family, desc.foundry, desc.style, desc.size);

      if (engine != nullptr) {
         initFontDef(desc, request, &engine->fontDef, multi);
      } else {
         blackListed.append(fontFamily);
      }

   } else {
#if defined(CS_SHOW_DEBUG_GUI_TEXT)
      qDebug("QFontDatabase::findFont() No match found");
#endif

   }

   if (engine == nullptr) {
      if (! request.family.isEmpty()) {
         QFont::StyleHint styleHint = QFont::StyleHint(request.styleHint);

         if (styleHint == QFont::AnyStyle && request.fixedPitch) {
            styleHint = QFont::TypeWriter;
         }

         QStringList fallbacks = request.fallBackFamilies + fallbacksForFamily(request.family,
               QFont::Style(request.style), styleHint, QChar::Script(script));

         if (script > QChar::Script_Common) {
            // Find the first font matching the specified script
            fallbacks += QString();
         }

         for (int i = 0; ! engine && i < fallbacks.size(); ++i) {
            QFontDef def = request;
            def.family   = fallbacks.at(i);

            QFontCache::Key key(def, script, multi ? 1 : 0);
            engine = fontCache->findEngine(key);

            if (engine == nullptr) {
               QtFontDesc desc;

               do {
                  fontFamily = match(script, def, def.family, QString(""), &desc, blackListed);
                  if (fontFamily != nullptr) {
                     QFontDef loadDef = def;

                     if (loadDef.family.isEmpty()) {
                        loadDef.family = desc.family->m_name;
                     }

                     engine = loadEngine(script, loadDef, desc.family, desc.foundry, desc.style, desc.size);

                     if (engine != nullptr) {
                        initFontDef(desc, loadDef, &engine->fontDef, multi);
                     } else {
                        blackListed.append(fontFamily);
                     }
                  }

               } while ((fontFamily != nullptr) && ! engine);
            }
         }
      }

      if (! engine) {
         engine = new QFontEngineBox(request.pixelSize);
      }

#if defined(CS_SHOW_DEBUG_GUI_TEXT)
      qDebug("QFontDatabase::findFont() Returning box engine");
#endif

   }

   return engine;
}

void QFontDatabase::load(const QFontPrivate *font, int script)
{
   QFontDef req = font->request;

   if (req.pixelSize == -1) {
      req.pixelSize = std::floor(((req.pointSize * font->dpi) / 72) * 100 + 0.5) / 100;
      req.pixelSize = qRound(req.pixelSize);
   }

   if (req.pointSize < 0) {
      req.pointSize = req.pixelSize * 72.0 / font->dpi;
   }

   if (req.stretch == 0) {
      req.stretch = 100;
   }

   // respect the fallback families that might be passed through the request
   const QStringList fallBackFamilies = familyList(req);

   if (! font->engineData) {
      QFontCache *fontCache = QFontCache::instance();
      // look for the requested font in the engine data cache
      // note: fallBackFamilies are not respected in the EngineData cache key;
      //       join them with the primary selection family to avoid cache misses

      req.family = fallBackFamilies.join(QChar(','));

      font->engineData = fontCache->findEngineData(req);

      if (! font->engineData) {
         // create a new one
         font->engineData = new QFontEngineData;
         fontCache->insertEngineData(req, font->engineData);
      }
      font->engineData->m_refCount.ref();
   }

   // cached engineData could have already loaded the engine we want
   if (font->engineData->engines[script]) {
      return;
   }

   QFontEngine *fontEngine = nullptr;

   req.fallBackFamilies = fallBackFamilies;
   if (! req.fallBackFamilies.isEmpty()) {
      req.family = req.fallBackFamilies.takeFirst();
   }

   // list of families to try
   QStringList familyList;

   if (! req.family.isEmpty()) {
      // Add primary selection
      familyList.append(req.family);

      // add the default family
      QString defaultFamily = QGuiApplication::font().family();

      if (! familyList.contains(defaultFamily)) {
         familyList.append(defaultFamily);
      }
   }

   // null family means find the first font matching the specified script
   familyList.append(QString());

   QStringList::const_iterator iter     = familyList.constBegin();
   QStringList::const_iterator iter_end = familyList.constEnd();

   for (; ! fontEngine && iter != iter_end; ++iter) {
      req.family = *iter;
      fontEngine = QFontDatabase::findFont(req, script);

      if (fontEngine) {
         if (fontEngine->type() == QFontEngine::Box && ! req.family.isEmpty()) {
            if (fontEngine->m_refCount.load() == 0) {
               delete fontEngine;
            }

            fontEngine = nullptr;

         } else {
            if (font->dpi > 0) {
               fontEngine->fontDef.pointSize = qreal(double((fontEngine->fontDef.pixelSize * 72) / font->dpi));
            }
         }
      }

      // no need to check requested fallback families again
      req.fallBackFamilies.clear();
   }

   Q_ASSERT(fontEngine);

   if (fontEngine->symbol || (font->request.styleStrategy & QFont::NoFontMerging)) {
      for (int i = 0; i < QChar::ScriptCount; ++i) {
         if (! font->engineData->engines[i]) {
            font->engineData->engines[i] = fontEngine;
            fontEngine->m_refCount.ref();
         }
      }

   } else {
      font->engineData->engines[script] = fontEngine;
      fontEngine->m_refCount.ref();
   }
}

QString QFontDatabase::resolveFontFamilyAlias(const QString &family)
{
   return QGuiApplicationPrivate::platformIntegration()->fontDatabase()->resolveFontFamilyAlias(family);
}
