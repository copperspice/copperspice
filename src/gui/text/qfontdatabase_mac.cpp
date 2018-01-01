/***********************************************************************
*
* Copyright (c) 2012-2018 Barbara Geller
* Copyright (c) 2012-2018 Ansel Sermersheim
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
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
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#include <qt_mac_p.h>
#include <qfontengine_p.h>
#include <qfile.h>
#include <qabstractfileengine.h>
#include <stdlib.h>
#include <qendian.h>
#include <qfontengine_coretext_p.h>
#include <qfontengine_mac_p.h>

QT_BEGIN_NAMESPACE

int qt_mac_pixelsize(const QFontDef &def, int dpi); //qfont_mac.cpp
int qt_mac_pointsize(const QFontDef &def, int dpi); //qfont_mac.cpp

// this could become a list of all languages used for each writing
// system, instead of using the single most common language.
static const char *languageForWritingSystem[] = {
   0,     // Any
   "en",  // Latin
   "el",  // Greek
   "ru",  // Cyrillic
   "hy",  // Armenian
   "he",  // Hebrew
   "ar",  // Arabic
   "syr", // Syriac
   "div", // Thaana
   "hi",  // Devanagari
   "bn",  // Bengali
   "pa",  // Gurmukhi
   "gu",  // Gujarati
   "or",  // Oriya
   "ta",  // Tamil
   "te",  // Telugu
   "kn",  // Kannada
   "ml",  // Malayalam
   "si",  // Sinhala
   "th",  // Thai
   "lo",  // Lao
   "bo",  // Tibetan
   "my",  // Myanmar
   "ka",  // Georgian
   "km",  // Khmer
   "zh-Hans", // SimplifiedChinese
   "zh-Hant", // TraditionalChinese
   "ja",  // Japanese
   "ko",  // Korean
   "vi",  // Vietnamese
   0, // Symbol
   0, // Ogham
   0, // Runic
   0 // N'Ko
};
enum { LanguageCount = sizeof(languageForWritingSystem) / sizeof(const char *) };

static void initializeDb()
{
   QFontDatabasePrivate *db = privateDb();
   if (!db || db->count) {
      return;
   }

   if (QSysInfo::MacintoshVersion >= QSysInfo::MV_10_5) {
      QCFType<CTFontCollectionRef> collection = CTFontCollectionCreateFromAvailableFonts(0);
      if (!collection) {
         return;
      }
      QCFType<CFArrayRef> fonts = CTFontCollectionCreateMatchingFontDescriptors(collection);
      if (!fonts) {
         return;
      }
      QString foundry_name = "CoreText";
      const int numFonts = CFArrayGetCount(fonts);
      for (int i = 0; i < numFonts; ++i) {
         CTFontDescriptorRef font = (CTFontDescriptorRef)CFArrayGetValueAtIndex(fonts, i);
         QCFString family_name = (CFStringRef)CTFontDescriptorCopyLocalizedAttribute(font, kCTFontFamilyNameAttribute, NULL);
         QCFString style_name = (CFStringRef)CTFontDescriptorCopyLocalizedAttribute(font, kCTFontStyleNameAttribute, NULL);
         QtFontFamily *family = db->family(family_name, true);

         if (QCFType<CFArrayRef> languages = (CFArrayRef) CTFontDescriptorCopyAttribute(font, kCTFontLanguagesAttribute)) {
            CFIndex length = CFArrayGetCount(languages);
            for (int i = 1; i < LanguageCount; ++i) {
               if (!languageForWritingSystem[i]) {
                  continue;
               }
               QCFString lang = CFStringCreateWithCString(NULL, languageForWritingSystem[i], kCFStringEncodingASCII);
               if (CFArrayContainsValue(languages, CFRangeMake(0, length), lang)) {
                  family->writingSystems[i] = QtFontFamily::Supported;
               }
            }
         }

         QtFontFoundry *foundry = family->foundry(foundry_name, true);

         QtFontStyle::Key styleKey;
         QString styleName = style_name;
         if (QCFType<CFDictionaryRef> styles = (CFDictionaryRef)CTFontDescriptorCopyAttribute(font, kCTFontTraitsAttribute)) {
            if (CFNumberRef weight = (CFNumberRef)CFDictionaryGetValue(styles, kCTFontWeightTrait)) {
               Q_ASSERT(CFNumberIsFloatType(weight));
               double d;
               if (CFNumberGetValue(weight, kCFNumberDoubleType, &d)) {
                  //qDebug() << "BOLD" << (QString)family_name << d;
                  styleKey.weight = (d > 0.0) ? QFont::Bold : QFont::Normal;
               }
            }
            if (CFNumberRef italic = (CFNumberRef)CFDictionaryGetValue(styles, kCTFontSlantTrait)) {
               Q_ASSERT(CFNumberIsFloatType(italic));
               double d;
               if (CFNumberGetValue(italic, kCFNumberDoubleType, &d)) {
                  //qDebug() << "ITALIC" << (QString)family_name << d;
                  if (d > 0.0) {
                     styleKey.style = QFont::StyleItalic;
                  }
               }
            }
         }

         QtFontStyle *style = foundry->style(styleKey, styleName, true);
         style->smoothScalable = true;
         if (QCFType<CFNumberRef> size = (CFNumberRef)CTFontDescriptorCopyAttribute(font, kCTFontSizeAttribute)) {
            //qDebug() << "WHEE";
            int pixel_size = 0;
            if (CFNumberIsFloatType(size)) {
               double d;
               CFNumberGetValue(size, kCFNumberDoubleType, &d);
               pixel_size = d;
            } else {
               CFNumberGetValue(size, kCFNumberIntType, &pixel_size);
            }
            //qDebug() << "SIZE" << (QString)family_name << pixel_size;
            if (pixel_size) {
               style->pixelSize(pixel_size, true);
            }
         } else {
            //qDebug() << "WTF?";
         }
      }
   }

}

static inline void load(const QString & = QString(), int = -1)
{
   initializeDb();
}

static const char *styleHint(const QFontDef &request)
{
   const char *stylehint = 0;
   switch (request.styleHint) {
      case QFont::SansSerif:
         stylehint = "Arial";
         break;
      case QFont::Serif:
         stylehint = "Times New Roman";
         break;
      case QFont::TypeWriter:
         stylehint = "Courier New";
         break;
      default:
         if (request.fixedPitch) {
            stylehint = "Courier New";
         }
         break;
   }
   return stylehint;
}

static inline float weightToFloat(unsigned int weight)
{
   return (weight - 50) / 100.0;
}

static QFontEngine *loadFromDatabase(QFontDef &req, const QFontPrivate *d)
{
   QCFString fontName = NULL;
   QStringList family_list = familyList(req);

   const char *stylehint = styleHint(req);
   if (stylehint) {
      family_list << QLatin1String(stylehint);
   }

   // add QFont::defaultFamily() to the list, for compatibility with previous versions
   family_list << QApplication::font().defaultFamily();

   QMutexLocker locker(fontDatabaseMutex());
   QFontDatabasePrivate *db = privateDb();
   if (!db->count) {
      initializeDb();
   }
   for (int i = 0; i < family_list.size(); ++i) {
      for (int k = 0; k < db->count; ++k) {
         if (db->families[k]->name.compare(family_list.at(i), Qt::CaseInsensitive) == 0) {

            CFStringRef familyName = QCFString::toCFStringRef(db->families[k]->name);
            QCFType<CTFontDescriptorRef> descriptor = CTFontDescriptorCreateWithAttributes(
                     QCFType<CFDictionaryRef>(CFDictionaryCreate(kCFAllocatorDefault,
                                              (const void **)&kCTFontFamilyNameAttribute,
                                              (const void **)&familyName, 1,
                                              &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks)));
            CFRelease(familyName);
            QCFType<CTFontRef> ctFont = CTFontCreateWithFontDescriptor(descriptor, 0, NULL);
            if (ctFont) {
               fontName = CTFontCopyFullName(ctFont);
               goto found;
            }

         }
      }
   }
found:

   if (fontName) {
      return new QCoreTextFontEngineMulti(fontName, req, d->kerning);
   }

   return NULL;
}

void QFontDatabase::load(const QFontPrivate *d, int script)
{
   // sanity checks
   if (!qApp) {
      qWarning("QFont: Must construct a QApplication before a QFont");
   }

   Q_ASSERT(script >= 0 && script < QChar::ScriptCount);
   Q_UNUSED(script);

   QFontDef req = d->request;
   req.pixelSize = qt_mac_pixelsize(req, d->dpi);

   // set the point size to 0 to get better caching
   req.pointSize = 0;
   QFontCache::Key key = QFontCache::Key(req, QChar::Script_Common, d->screen);

   if (!(d->engineData = QFontCache::instance()->findEngineData(key))) {
      d->engineData = new QFontEngineData;
      QFontCache::instance()->insertEngineData(key, d->engineData);
   } else {
      d->engineData->ref.ref();
   }
   if (d->engineData->engine) { // already loaded
      return;
   }

   // set it to the actual pointsize, so QFontInfo will do the right thing
   req.pointSize = qt_mac_pointsize(d->request, d->dpi);

   QFontEngine *e = QFontCache::instance()->findEngine(key);
   if (!e && qt_enable_test_font && req.family == QLatin1String("__Qt__Box__Engine__")) {
      e = new QTestFontEngine(req.pixelSize);
      e->fontDef = req;
   }

   if (e) {
      e->ref.ref();
      d->engineData->engine = e;
      return; // the font info and fontdef should already be filled
   }

   QFontEngine *engine = NULL;

   // Shortcut to get the font directly without going through the font database
   if (!req.family.isEmpty() && !req.styleName.isEmpty()) {
      QCFString expectedFamily = QCFString(req.family);
      QCFString expectedStyle = QCFString(req.styleName);

      QCFType<CFMutableDictionaryRef> attributes = CFDictionaryCreateMutable(NULL, 0,
            &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
      CFDictionaryAddValue(attributes, kCTFontFamilyNameAttribute, expectedFamily);
      CFDictionaryAddValue(attributes, kCTFontStyleNameAttribute, expectedStyle);

      QCFType<CTFontDescriptorRef> descriptor = CTFontDescriptorCreateWithAttributes(attributes);
      CGAffineTransform transform = qt_transform_from_fontdef(req);
      QCFType<CTFontRef> ctFont = CTFontCreateWithFontDescriptor(descriptor, req.pixelSize, &transform);
      if (ctFont) {
         QCFString familyName = CTFontCopyFamilyName(ctFont);
         // Only accept the font if the family name is exactly the same as we specified
         if (CFEqual(expectedFamily, familyName)) {
            engine = new QCoreTextFontEngineMulti(ctFont, req, d->kerning);
         }
      }
   }

   if (!engine) {
      engine = loadFromDatabase(req, d);
   }

   if (engine) {
      d->engineData->engine = engine;
      engine->ref.ref();
      QFontCache::instance()->insertEngine(key, engine);
   }
}

static void registerFont(QFontDatabasePrivate::ApplicationFont *fnt)
{
   ATSFontContainerRef handle;
   OSStatus e  = noErr;

   if (fnt->data.isEmpty()) {


      FSRef ref;
      if (qt_mac_create_fsref(fnt->fileName, &ref) != noErr) {
         return;
      }

      ATSFontActivateFromFileReference(&ref, kATSFontContextLocal, kATSFontFormatUnspecified, 0, kATSOptionFlagsDefault,
                                       &handle);


   } else {
      e = ATSFontActivateFromMemory((void *)fnt->data.constData(), fnt->data.size(), kATSFontContextLocal,
                                    kATSFontFormatUnspecified, 0, kATSOptionFlagsDefault, &handle);

      fnt->data = QByteArray();
   }

   if (e != noErr) {
      return;
   }

   ItemCount fontCount = 0;
   e = ATSFontFindFromContainer(handle, kATSOptionFlagsDefault, 0, 0, &fontCount);
   if (e != noErr) {
      return;
   }

   QVarLengthArray<ATSFontRef> containedFonts(fontCount);
   e = ATSFontFindFromContainer(handle, kATSOptionFlagsDefault, fontCount, containedFonts.data(), &fontCount);
   if (e != noErr) {
      return;
   }

   fnt->families.clear();

   // Make sure that the family name set on the font matches what
   // kCTFontFamilyNameAttribute returns in initializeDb().
   // So far the best solution seems find the installed font
   // using CoreText and get the family name from it.
   // (ATSFontFamilyGetName appears to be the correct API, but also
   // returns the font display name.)
   for (int i = 0; i < containedFonts.size(); ++i) {
      QCFString fontPostScriptName;
      ATSFontGetPostScriptName(containedFonts[i], kATSOptionFlagsDefault, &fontPostScriptName);
      QCFType<CTFontDescriptorRef> font = CTFontDescriptorCreateWithNameAndSize(fontPostScriptName, 14);
      QCFString familyName = (CFStringRef)CTFontDescriptorCopyAttribute(font, kCTFontFamilyNameAttribute);
      fnt->families.append(familyName);
   }

   fnt->handle = handle;
}

bool QFontDatabase::removeApplicationFont(int handle)
{
   QMutexLocker locker(fontDatabaseMutex());

   QFontDatabasePrivate *db = privateDb();
   if (handle < 0 || handle >= db->applicationFonts.count()) {
      return false;
   }

   OSStatus e = ATSFontDeactivate(db->applicationFonts.at(handle).handle,
                                  /*iRefCon=*/0, kATSOptionFlagsDefault);
   if (e != noErr) {
      return false;
   }

   db->applicationFonts[handle] = QFontDatabasePrivate::ApplicationFont();

   db->invalidate();
   return true;
}

bool QFontDatabase::removeAllApplicationFonts()
{
   QMutexLocker locker(fontDatabaseMutex());

   QFontDatabasePrivate *db = privateDb();
   for (int i = 0; i < db->applicationFonts.count(); ++i) {
      if (!removeApplicationFont(i)) {
         return false;
      }
   }
   return true;
}

bool QFontDatabase::supportsThreadedFontRendering()
{
   return true;
}

QString QFontDatabase::resolveFontFamilyAlias(const QString &family)
{
   QCFString expectedFamily = QCFString(family);

   QCFType<CFMutableDictionaryRef> attributes = CFDictionaryCreateMutable(NULL, 0,
         &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
   CFDictionaryAddValue(attributes, kCTFontFamilyNameAttribute, expectedFamily);
   QCFType<CTFontDescriptorRef> descriptor = CTFontDescriptorCreateWithAttributes(attributes);

   QCFType<CFMutableSetRef> mandatoryAttributes = CFSetCreateMutable(NULL, 0, &kCFTypeSetCallBacks);
   CFSetAddValue(mandatoryAttributes, kCTFontFamilyNameAttribute);

   QCFType<CTFontRef> font = CTFontCreateWithFontDescriptor(descriptor, 0.0, NULL);
   QCFType<CTFontDescriptorRef> matched = CTFontDescriptorCreateMatchingFontDescriptor(descriptor, mandatoryAttributes);
   if (!matched) {
      return family;
   }

   QCFString familyName = (CFStringRef) CTFontDescriptorCopyLocalizedAttribute(matched, kCTFontFamilyNameAttribute, NULL);
   return familyName;
}

QT_END_NAMESPACE
