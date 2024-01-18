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

#include <qglobal.h>
#include <qendian.h>
#include <qsettings.h>

#include <qcoretextfontdatabase_p.h>
#include <qfontengine_coretext_p.h>

#include <sys/param.h>

#import <Cocoa/Cocoa.h>
#import <IOKit/graphics/IOGraphicsLib.h>

#if defined(QT_USE_FREETYPE)
#include <qfontengine_ft_p.h>
#endif

// could become a list of all languages used for each writing
// system, instead of using the single most common language.
static const char *languageForWritingSystem[] = {
   nullptr,   // Any
   "en",      // Latin
   "el",      // Greek
   "ru",      // Cyrillic
   "hy",      // Armenian
   "he",      // Hebrew
   "ar",      // Arabic
   "syr",     // Syriac
   "div",     // Thaana
   "hi",      // Devanagari
   "bn",      // Bengali
   "pa",      // Gurmukhi
   "gu",      // Gujarati
   "or",      // Oriya
   "ta",      // Tamil
   "te",      // Telugu
   "kn",      // Kannada
   "ml",      // Malayalam
   "si",      // Sinhala
   "th",      // Thai
   "lo",      // Lao
   "bo",      // Tibetan
   "my",      // Myanmar
   "ka",      // Georgian
   "km",      // Khmer
   "zh-Hans", // SimplifiedChinese
   "zh-Hant", // TraditionalChinese
   "ja",      // Japanese
   "ko",      // Korean
   "vi",      // Vietnamese
   nullptr,   // Symbol
   "sga",     // Ogham
   "non",     // Runic
   "man"      // N'Ko
};
enum { LanguageCount = sizeof(languageForWritingSystem) / sizeof(const char *) };

#if defined(Q_OS_DARWIN)
static NSInteger languageMapSort(id obj1, id obj2, void *context)
{
   NSArray *map1 = (NSArray *) obj1;
   NSArray *map2 = (NSArray *) obj2;
   NSArray *languages = (NSArray *) context;

   NSString *lang1 = [map1 objectAtIndex: 0];
   NSString *lang2 = [map2 objectAtIndex: 0];

   return [languages indexOfObject: lang1] - [languages indexOfObject: lang2];
}
#endif

QCoreTextFontDatabase::QCoreTextFontDatabase(bool useFreeType)
#if defined(QT_USE_FREETYPE)
   : m_useFreeType(useFreeType)
#endif
{

#if defined(Q_OS_DARWIN)
   QSettings appleSettings("apple.com");
   QVariant appleValue = appleSettings.value("AppleAntiAliasingThreshold");

   if (appleValue.isValid()) {
      QCoreTextFontEngine::antialiasingThreshold = appleValue.toInt();
   }

   /*
       font_smoothing = 0 means no smoothing, while 1-3 means subpixel
       antialiasing with different hinting styles (but we don't care about the
       exact value, only if subpixel rendering is available or not)
   */
   int font_smoothing = 0;
   appleValue = appleSettings.value("AppleFontSmoothing");

   if (appleValue.isValid()) {
      font_smoothing = appleValue.toInt();
   } else {
      // non-Apple displays do not provide enough information about subpixel rendering so
      // draw text with cocoa and compare pixel colors to see if subpixel rendering is enabled
      int w = 10;
      int h = 10;
      NSRect rect = NSMakeRect(0.0, 0.0, w, h);
      NSImage *fontImage = [[NSImage alloc] initWithSize: NSMakeSize(w, h)];

      [fontImage lockFocus];

      [[NSColor whiteColor] setFill];
      NSRectFill(rect);

      NSString *str = @"X\\";
      NSFont *font = [NSFont fontWithName: @"Helvetica" size: 10.0];
      NSMutableDictionary *attrs = [NSMutableDictionary dictionary];
      [attrs setObject: font forKey: NSFontAttributeName];
      [attrs setObject: [NSColor blackColor] forKey: NSForegroundColorAttributeName];

      [str drawInRect: rect withAttributes: attrs];

      NSBitmapImageRep *nsBitmapImage = [[NSBitmapImageRep alloc] initWithFocusedViewRect: rect];

      [fontImage unlockFocus];

      float red, green, blue;
      for (int x = 0; x < w; x++) {
         for (int y = 0; y < h; y++) {
            NSColor *pixelColor = [nsBitmapImage colorAtX: x y: y];
            red = [pixelColor redComponent];
            green = [pixelColor greenComponent];
            blue = [pixelColor blueComponent];

            if (red != green || red != blue) {
               font_smoothing = 1;
            }
         }
      }

      [nsBitmapImage release];
      [fontImage release];
   }

   QCoreTextFontEngine::defaultGlyphFormat = (font_smoothing > 0
         ? QFontEngine::Format_A32
         : QFontEngine::Format_A8);
#else
   QCoreTextFontEngine::defaultGlyphFormat = QFontEngine::Format_A8;
#endif
}

QCoreTextFontDatabase::~QCoreTextFontDatabase()
{
   for (CTFontDescriptorRef ref : m_systemFontDescriptors) {
      CFRelease(ref);
   }
}

static CFArrayRef availableFamilyNames()
{
   return CTFontManagerCopyAvailableFontFamilyNames();
}

void QCoreTextFontDatabase::populateFontDatabase()
{
   // caller (QFontDB) expects the db to be populate only with system fonts, so we need
   // to make sure that any previously registered app fonts become invisible.
   removeApplicationFonts();

   QCFType<CFArrayRef> familyNames = availableFamilyNames();
   const int numberOfFamilies = CFArrayGetCount(familyNames);

   for (int i = 0; i < numberOfFamilies; ++i) {
      CFStringRef familyNameRef = (CFStringRef) CFArrayGetValueAtIndex(familyNames, i);
      QString familyName = QCFString::toQString(familyNameRef);

      // do not populate internal fonts
      if (familyName.startsWith('.') || familyName == "LastResort") {
         continue;
      }

      QPlatformFontDatabase::registerFontFamily(familyName);

      QString localizedFamilyName = QString::fromNSString([[NSFontManager sharedFontManager] localizedNameForFamily:
             (NSString *)familyNameRef face: nil]);

      if (familyName != localizedFamilyName) {
         QPlatformFontDatabase::registerAliasToFontFamily(familyName, localizedFamilyName);
      }
   }

   // Force creating the theme fonts to get the descriptors in m_systemFontDescriptors
   if (m_themeFonts.isEmpty()) {
      (void)themeFonts();
   }

   for (CTFontDescriptorRef fontDesc : m_systemFontDescriptors) {
      populateFromDescriptor(fontDesc);
   }
}

void QCoreTextFontDatabase::populateFamily(const QString &familyName)
{
   QCFType<CFMutableDictionaryRef> attributes = CFDictionaryCreateMutable(kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks,
         &kCFTypeDictionaryValueCallBacks);

   QCFString tmp = QCFString(familyName);
   CFDictionaryAddValue(attributes, kCTFontFamilyNameAttribute, tmp.toCFStringRef());

   QCFType<CTFontDescriptorRef> nameOnlyDescriptor = CTFontDescriptorCreateWithAttributes(attributes);

   // a single family might match several different fonts with different styles
   QCFType<CFArrayRef> matchingFonts = (CFArrayRef) CTFontDescriptorCreateMatchingFontDescriptors(nameOnlyDescriptor, nullptr);

   if (! matchingFonts) {
      qWarning() << "QCoreTextFontDatabase::populateFamily(): No matching fonts for family" << familyName;
      return;
   }

   const int numFonts = CFArrayGetCount(matchingFonts);
   for (int i = 0; i < numFonts; ++i) {
      populateFromDescriptor(CTFontDescriptorRef(CFArrayGetValueAtIndex(matchingFonts, i)));
   }
}

struct FontDescription {
   QCFString familyName;
   QCFString styleName;
   QString foundryName;
   QFont::Weight weight;
   QFont::Style style;
   QFont::Stretch stretch;
   int pixelSize;
   bool fixedPitch;
   QSupportedWritingSystems writingSystems;
};

static void getFontDescription(CTFontDescriptorRef font, FontDescription *fd)
{
   QCFType<CFDictionaryRef> styles = (CFDictionaryRef) CTFontDescriptorCopyAttribute(font, kCTFontTraitsAttribute);

   fd->foundryName = "CoreText";
   fd->familyName  = (CFStringRef) CTFontDescriptorCopyAttribute(font, kCTFontFamilyNameAttribute);
   fd->styleName   = (CFStringRef) CTFontDescriptorCopyAttribute(font, kCTFontStyleNameAttribute);

   fd->weight     = QFont::Normal;
   fd->style      = QFont::StyleNormal;
   fd->stretch    = QFont::Unstretched;
   fd->fixedPitch = false;

   if (QCFType<CTFontRef> tempFont = CTFontCreateWithFontDescriptor(font, 0.0, nullptr)) {
      uint tag = MAKE_TAG('O', 'S', '/', '2');
      CTFontRef tempFontRef = tempFont;
      void *userData = reinterpret_cast<void *>(&tempFontRef);

      uint length = 128;
      QVarLengthArray<uchar, 128> os2Table(length);

      if (QCoreTextFontEngine::ct_getSfntTable(userData, tag, os2Table.data(), &length) && length >= 86) {

         if (length > uint(os2Table.length())) {
            os2Table.resize(length);

            if (! QCoreTextFontEngine::ct_getSfntTable(userData, tag, os2Table.data(), &length)) {
               // error, may want to throw
            }

            Q_ASSERT(length >= 86);
         }

         quint32 unicodeRange[4] = {
            qFromBigEndian<quint32>(os2Table.data() + 42),
            qFromBigEndian<quint32>(os2Table.data() + 46),
            qFromBigEndian<quint32>(os2Table.data() + 50),
            qFromBigEndian<quint32>(os2Table.data() + 54)
         };

         quint32 codePageRange[2] = {
            qFromBigEndian<quint32>(os2Table.data() + 78),
            qFromBigEndian<quint32>(os2Table.data() + 82)
         };
         fd->writingSystems = QPlatformFontDatabase::writingSystemsFromTrueTypeBits(unicodeRange, codePageRange);
      }
   }

   if (styles) {
      if (CFNumberRef weightValue = (CFNumberRef) CFDictionaryGetValue(styles, kCTFontWeightTrait)) {
         double normalizedWeight;
         if (CFNumberGetValue(weightValue, kCFNumberFloat64Type, &normalizedWeight)) {
            fd->weight = QCoreTextFontEngine::qtWeightFromCFWeight(normalizedWeight);
         }
      }

      if (CFNumberRef italic = (CFNumberRef) CFDictionaryGetValue(styles, kCTFontSlantTrait)) {
         double d;

         if (CFNumberGetValue(italic, kCFNumberDoubleType, &d)) {
            if (d > 0.0) {
               fd->style = QFont::StyleItalic;
            }
         }
      }

      if (CFNumberRef symbolic = (CFNumberRef) CFDictionaryGetValue(styles, kCTFontSymbolicTrait)) {
         int d;
         if (CFNumberGetValue(symbolic, kCFNumberSInt32Type, &d)) {
            if (d & kCTFontMonoSpaceTrait) {
               fd->fixedPitch = true;
            }

            if (d & kCTFontExpandedTrait) {
               fd->stretch = QFont::Expanded;
            } else if (d & kCTFontCondensedTrait) {
               fd->stretch = QFont::Condensed;
            }
         }
      }
   }

   if (QCFType<CFNumberRef> size = (CFNumberRef) CTFontDescriptorCopyAttribute(font, kCTFontSizeAttribute)) {
      if (CFNumberIsFloatType(size)) {
         double d;
         CFNumberGetValue(size, kCFNumberDoubleType, &d);
         fd->pixelSize = d;

      } else {
         CFNumberGetValue(size, kCFNumberIntType, &fd->pixelSize);
      }
   }

   if (QCFType<CFArrayRef> languages = (CFArrayRef) CTFontDescriptorCopyAttribute(font, kCTFontLanguagesAttribute)) {
      CFIndex length = CFArrayGetCount(languages);

      for (int i = 1; i < LanguageCount; ++i) {
         if (!languageForWritingSystem[i]) {
            continue;
         }

         QCFString lang = CFStringCreateWithCString(nullptr, languageForWritingSystem[i], kCFStringEncodingASCII);
         if (CFArrayContainsValue(languages, CFRangeMake(0, length), lang.toCFStringRef())) {
            fd->writingSystems.setSupported(QFontDatabase::WritingSystem(i));
         }
      }
   }
}

void QCoreTextFontDatabase::populateFromDescriptor(CTFontDescriptorRef font)
{
   FontDescription fd;
   getFontDescription(font, &fd);

   CFRetain(font);

   QPlatformFontDatabase::registerFont(fd.familyName.toQString(), fd.styleName.toQString(), fd.foundryName,
            fd.weight, fd.style, fd.stretch, true, true, fd.pixelSize, fd.fixedPitch, fd.writingSystems, (void *) font);
}

void QCoreTextFontDatabase::releaseHandle(void *handle)
{
   CFRelease(CTFontDescriptorRef(handle));
}

#if defined(QT_USE_FREETYPE)
static QByteArray filenameForCFUrl(CFURLRef url)
{
   // The on-stack buffer prevents that a QByteArray allocated for the worst case (MAXPATHLEN)
   // stays around for the lifetime of the font. Additionally, it helps to move the char
   // signedness cast to an acceptable place.
   uchar buffer[MAXPATHLEN];
   QByteArray filename;

   if (! CFURLGetFileSystemRepresentation(url, true, buffer, sizeof(buffer))) {
      qWarning("QCoreTextFontDatabase::filenameForCFUrl: could not resolve file for URL %s",
         url ? csPrintable(QString::fromCFString(CFURLGetString(url))) : "(null)");

   } else {
      QCFType<CFStringRef> scheme = CFURLCopyScheme(url);

      if (QString::fromCFString(scheme) == "qrc") {
         filename = ":";
      }

      filename += reinterpret_cast<char *>(buffer);
   }

   return filename;
}
#endif

extern CGAffineTransform qt_transform_from_fontdef(const QFontDef &fontDef);

QFontEngine *QCoreTextFontDatabase::fontEngine(const QFontDef &f, void *usrPtr)
{
   CTFontDescriptorRef descriptor = static_cast<CTFontDescriptorRef>(usrPtr);

#if defined(QT_USE_FREETYPE)
   if (m_useFreeType) {
      QCFType<CFURLRef> url(static_cast<CFURLRef>(CTFontDescriptorCopyAttribute(descriptor, kCTFontURLAttribute)));

      QByteArray filename;

      if (url) {
         filename = filenameForCFUrl(url);
      }

      return freeTypeFontEngine(f, filename);
   }
#endif

   // Since we do not pass in the destination DPI to CoreText when making
   // the font, we need to pass in a point size which is scaled to include
   // the DPI. The default DPI for the screen is 72, thus the scale factor
   // is destinationDpi / 72, but since pixelSize = pointSize / 72 * dpi,
   // the pixelSize is actually the scaled point size for the destination
   // DPI, and we can use that directly.
   qreal scaledPointSize = f.pixelSize;

   CGAffineTransform matrix = qt_transform_from_fontdef(f);
   CTFontRef font = CTFontCreateWithFontDescriptor(descriptor, scaledPointSize, &matrix);

   if (font) {
      QFontEngine *engine = new QCoreTextFontEngine(font, f);
      engine->fontDef = f;
      CFRelease(font);
      return engine;
   }

   return nullptr;
}

static void releaseFontData(void *info, const void *data, size_t size)
{
   (void) data;
   (void) size;
   delete (QByteArray *)info;
}

QFontEngine *QCoreTextFontDatabase::fontEngine(const QByteArray &fontData, qreal pixelSize, QFont::HintingPreference hintingPreference)
{
#if defined(QT_USE_FREETYPE)
   if (m_useFreeType) {
      QByteArray *fontDataCopy = new QByteArray(fontData);

      QCFType<CGDataProviderRef> dataProvider = CGDataProviderCreateWithData(fontDataCopy,
            fontDataCopy->constData(), fontDataCopy->size(), releaseFontData);
      QCFType<CGFontRef> cgFont(CGFontCreateWithDataProvider(dataProvider));

      if (! cgFont) {
         qWarning("QCoreTextFontDatabase::fontEngine: CGFontCreateWithDataProvider failed");
         return nullptr;
      }

      QFontDef fontDef;
      fontDef.pixelSize = pixelSize;
      fontDef.pointSize = pixelSize * 72.0 / qt_defaultDpi();
      fontDef.hintingPreference = hintingPreference;
      CGAffineTransform transform = qt_transform_from_fontdef(fontDef);
      QCFType<CTFontRef> ctFont(CTFontCreateWithGraphicsFont(cgFont, fontDef.pixelSize, &transform, nullptr));
      QCFType<CFURLRef> url(static_cast<CFURLRef>(CTFontCopyAttribute(ctFont, kCTFontURLAttribute)));
      return freeTypeFontEngine(fontDef, filenameForCFUrl(url), fontData);
   }
#endif

   (void) hintingPreference;

   QByteArray *fontDataCopy = new QByteArray(fontData);
   QCFType<CGDataProviderRef> dataProvider = CGDataProviderCreateWithData(fontDataCopy,
         fontDataCopy->constData(), fontDataCopy->size(), releaseFontData);

   CGFontRef cgFont = CGFontCreateWithDataProvider(dataProvider);

   QFontEngine *fontEngine = nullptr;

   if (cgFont == nullptr) {
      qWarning("QCoreTextFontDatabase::fontEngine: CGFontCreateWithDataProvider failed");
   } else {
      QFontDef def;
      def.pixelSize = pixelSize;
      def.pointSize = pixelSize * 72.0 / qt_defaultDpi();
      fontEngine = new QCoreTextFontEngine(cgFont, def);
      CFRelease(cgFont);
   }

   return fontEngine;
}

QFont::StyleHint styleHintFromNSString(NSString *style)
{
   if ([style isEqual: @"sans-serif"]) {
      return QFont::SansSerif;

   } else if ([style isEqual: @"monospace"]) {
      return QFont::Monospace;

   } else if ([style isEqual: @"cursive"]) {
      return QFont::Cursive;

   } else if ([style isEqual: @"serif"]) {
      return QFont::Serif;

   } else if ([style isEqual: @"fantasy"]) {
      return QFont::Fantasy;

   } else { // if ([style isEqual: @"default"])
      return QFont::AnyStyle;
   }
}

#if defined(Q_OS_DARWIN)
static QString familyNameFromPostScriptName(NSString *psName)
{
   QCFType<CTFontDescriptorRef> fontDescriptor = (CTFontDescriptorRef) CTFontDescriptorCreateWithNameAndSize((CFStringRef)psName, 12.0);
   QCFString familyName = (CFStringRef) CTFontDescriptorCopyAttribute(fontDescriptor, kCTFontFamilyNameAttribute);
   QString name = familyName.toQString();

   if (name.isEmpty()) {
      qWarning() << "QCoreTextFontDatabase: Failed to resolve family name for PostScript name " << QCFString::toQString((
               CFStringRef)psName);
   }

   return name;
}
#endif

QStringList QCoreTextFontDatabase::fallbacksForFamily(const QString &family, QFont::Style style, QFont::StyleHint styleHint,
   QChar::Script script) const
{
   (void) style;
   (void) script;
   QMacAutoReleasePool pool;

   static QHash<QString, QStringList> fallbackLists;

   if (! family.isEmpty()) {
      // CTFontCopyDefaultCascadeListForLanguages is available in the SDK

      QCFType<CFMutableDictionaryRef> attributes = CFDictionaryCreateMutable(kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks,
            &kCFTypeDictionaryValueCallBacks);

      QCFString tmp = QCFString(family);
      CFDictionaryAddValue(attributes, kCTFontFamilyNameAttribute, tmp.toCFStringRef());

      if (QCFType<CTFontDescriptorRef> fontDescriptor = CTFontDescriptorCreateWithAttributes(attributes)) {
         if (QCFType<CTFontRef> font = CTFontCreateWithFontDescriptor(fontDescriptor, 12.0, nullptr)) {
            NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
            NSArray *languages = [defaults stringArrayForKey: @"AppleLanguages"];

            QCFType<CFArrayRef> cascadeList = (CFArrayRef) CTFontCopyDefaultCascadeListForLanguages(font, (CFArrayRef) languages);

            if (cascadeList) {
               QStringList fallbackList;
               const int numCascades = CFArrayGetCount(cascadeList);

               for (int i = 0; i < numCascades; ++i) {
                  CTFontDescriptorRef fontFallback = (CTFontDescriptorRef) CFArrayGetValueAtIndex(cascadeList, i);

                  QCFString fallbackFamilyName = (CFStringRef) CTFontDescriptorCopyAttribute(fontFallback, kCTFontFamilyNameAttribute);
                  fallbackList.append(fallbackFamilyName.toQString());
               }

#if defined(Q_OS_DARWIN)
               // Since we are only returning a list of default fonts for the current language, we do not
               // cover all unicode completely. This was especially an issue for some of the common script
               // symbols such as mathematical symbols, currency or geometric shapes. To minimize the risk
               // of missing glyphs, we add Arial Unicode MS as a final fail safe, since this covers most of Unicode 2.1.

               if (! fallbackList.contains("Arial Unicode MS")) {
                  fallbackList.append("Arial Unicode MS");
               }
#endif

               return fallbackList;
            }
         }
      }

   }

   // We were not able to find a fallback for the specific family,
   // so we fall back to the stylehint.

   static const QString styleLookupKey = QString::fromLatin1(".QFontStyleHint_%1");

   static bool didPopulateStyleFallbacks = false;
   if (!didPopulateStyleFallbacks) {

#if defined(Q_OS_DARWIN)
      NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
      NSArray *languages = [defaults stringArrayForKey: @"AppleLanguages"];

      NSDictionary *fallbackDict = [NSDictionary dictionaryWithContentsOfFile:
                         @"/System/Library/Frameworks/ApplicationServices.framework/Frameworks/CoreText.framework/Resources/DefaultFontFallbacks.plist"];

      for (NSString * style in [fallbackDict allKeys]) {
         NSArray *list = [fallbackDict valueForKey: style];
         QFont::StyleHint fallbackStyleHint = styleHintFromNSString(style);
         QStringList fallbackList;

         for (id item in list) {
            // sort the array based on system language preferences
            if ([item isKindOfClass: [NSArray class]]) {
               NSArray *langs = [(NSArray *) item sortedArrayUsingFunction: languageMapSort
                                                                   context: languages];
               for (NSArray * map in langs) {
                  fallbackList.append(familyNameFromPostScriptName([map objectAtIndex: 1]));
               }

            } else if ([item isKindOfClass: [NSString class]]) {
               fallbackList.append(familyNameFromPostScriptName(item));
            }
         }

         fallbackList.append("Apple Color Emoji");

         // Since we are only returning a list of default fonts for the current language, we do not
         // cover all unicode completely. This was especially an issue for some of the common script
         // symbols such as mathematical symbols, currency or geometric shapes. To minimize the risk
         // of missing glyphs, we add Arial Unicode MS as a final fail safe, since this covers most of Unicode 2.1.

         if (! fallbackList.contains("Arial Unicode MS")) {
            fallbackList.append("Arial Unicode MS");
         }

         fallbackLists[styleLookupKey.formatArg(fallbackStyleHint)] = fallbackList;
      }
#else
      QStringList staticFallbackList;
      staticFallbackList << QString::fromLatin1("Helvetica,Apple Color Emoji,Geeza Pro,Arial Hebrew,Thonburi,Kailasa"
            "Hiragino Kaku Gothic ProN,.Heiti J,Apple SD Gothic Neo,.Heiti K,Heiti SC,Heiti TC"
            "Bangla Sangam MN,Devanagari Sangam MN,Gujarati Sangam MN,Gurmukhi MN,Kannada Sangam MN"
            "Malayalam Sangam MN,Oriya Sangam MN,Sinhala Sangam MN,Tamil Sangam MN,Telugu Sangam MN"
            "Euphemia UCAS,.PhoneFallback").split(",");

      for (int i = QFont::Helvetica; i <= QFont::Fantasy; ++i) {
         fallbackLists[styleLookupKey.formatArg(i)] = staticFallbackList;
      }
#endif

      didPopulateStyleFallbacks = true;
   }

   Q_ASSERT(!fallbackLists.isEmpty());
   return fallbackLists[styleLookupKey.formatArg(styleHint)];
}

static CFArrayRef createDescriptorArrayForFont(CTFontRef font, const QString &fileName = QString())
{
   CFMutableArrayRef array = CFArrayCreateMutable(kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks);
   QCFType<CTFontDescriptorRef> descriptor = CTFontCopyFontDescriptor(font);

#if defined(QT_USE_FREETYPE)
   // The physical font source URL (usually a local file or resource) is only required for
   // FreeType, when using non-system fonts, and needs some hackery to attach in a format agreeable to OSX.

   if (! fileName.isEmpty()) {
      QCFType<CFURLRef> fontURL;

      if (fileName.startsWith(":/")) {
         // QUrl::fromLocalFile() does not accept qrc pseudo-paths like ":/fonts/myfont.ttf".
         // Therefore construct from QString with the qrc:// scheme -> "qrc:///fonts/myfont.ttf".

         fontURL = QUrl("qrc://" + fileName.mid(1)).toCFURL();

      } else if (!fileName.isEmpty()) {
         // At this point we hope that filename is in a format that QUrl can handle.
         fontURL = QUrl::fromLocalFile(fileName).toCFURL();
      }

      QCFType<CFMutableDictionaryRef> attributes = CFDictionaryCreateMutable(kCFAllocatorDefault, 1,
            &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
      CFDictionaryAddValue(attributes, kCTFontURLAttribute, fontURL);
      descriptor = CTFontDescriptorCreateCopyWithAttributes(descriptor, attributes);
   }
#endif

   CFArrayAppendValue(array, descriptor);
   return array;
}

QStringList QCoreTextFontDatabase::addApplicationFont(const QByteArray &fontData, const QString &fileName)
{
   QCFType<CFArrayRef> fonts;
   QStringList families;

   if (&CTFontManagerRegisterGraphicsFont) {
      CFErrorRef error = nullptr;

      if (! fontData.isEmpty()) {
         QByteArray *fontDataCopy = new QByteArray(fontData);

         QCFType<CGDataProviderRef> dataProvider = CGDataProviderCreateWithData(fontDataCopy,
               fontDataCopy->constData(), fontDataCopy->size(), releaseFontData);

         QCFType<CGFontRef> cgFont = CGFontCreateWithDataProvider(dataProvider);

         if (cgFont) {
            if (CTFontManagerRegisterGraphicsFont(cgFont, &error)) {
               QCFType<CTFontRef> font = CTFontCreateWithGraphicsFont(cgFont, 0.0, nullptr, nullptr);
               fonts = createDescriptorArrayForFont(font

#if defined(QT_USE_FREETYPE)
                     , m_useFreeType ? fileName : QString()
#endif
                  );
               m_applicationFonts.append(QVariant::fromValue(QCFType<CGFontRef>::constructFromGet(cgFont)));
            }
         }

      } else {
         QCFType<CFURLRef> fontURL = CFURLCreateWithFileSystemPath(nullptr, QCFString(fileName).toCFStringRef(), kCFURLPOSIXPathStyle, false);

         if (CTFontManagerRegisterFontsForURL(fontURL, kCTFontManagerScopeProcess, &error)) {

            if (&CTFontManagerCreateFontDescriptorsFromURL) {
               fonts = CTFontManagerCreateFontDescriptorsFromURL(fontURL);

            } else {
               // limited to a single font per file, unless we dive into the font tables
               QCFType<CFMutableDictionaryRef> attributes = CFDictionaryCreateMutable(kCFAllocatorDefault, 1,
                     &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);

               CFDictionaryAddValue(attributes, kCTFontURLAttribute, fontURL);
               QCFType<CTFontDescriptorRef> descriptor = CTFontDescriptorCreateWithAttributes(attributes);
               QCFType<CTFontRef> font = CTFontCreateWithFontDescriptor(descriptor, 0.0, nullptr);
               fonts = createDescriptorArrayForFont(font);
            }

            m_applicationFonts.append(QVariant::fromValue(QCFType<CFURLRef>::constructFromGet(fontURL)));
         }
      }

      if (error) {
         NSLog(@"Unable to register font: %@", error);
         CFRelease(error);
      }
   }

   if (fonts) {
      const int numFonts = CFArrayGetCount(fonts);

      for (int i = 0; i < numFonts; ++i) {
         CTFontDescriptorRef fontDescriptor = CTFontDescriptorRef(CFArrayGetValueAtIndex(fonts, i));
         populateFromDescriptor(fontDescriptor);

         QCFType<CFStringRef> familyName = CFStringRef(CTFontDescriptorCopyAttribute(fontDescriptor, kCTFontFamilyNameAttribute));
         families.append(QString::fromCFString(familyName));
      }
   }

   return families;
}

bool QCoreTextFontDatabase::isPrivateFontFamily(const QString &family) const
{
   if (family.startsWith('.')) {
      return true;
   }

   return QPlatformFontDatabase::isPrivateFontFamily(family);
}

static CTFontUIFontType fontTypeFromTheme(QPlatformTheme::Font f)
{
   switch (f) {
      case QPlatformTheme::SystemFont:
         return kCTFontUIFontSystem;

      case QPlatformTheme::MenuFont:
      case QPlatformTheme::MenuBarFont:
      case QPlatformTheme::MenuItemFont:
         return kCTFontUIFontMenuItem;

      case QPlatformTheme::MessageBoxFont:
         return kCTFontUIFontEmphasizedSystem;

      case QPlatformTheme::LabelFont:
         return kCTFontUIFontSystem;

      case QPlatformTheme::TipLabelFont:
         return kCTFontUIFontToolTip;

      case QPlatformTheme::StatusBarFont:
         return kCTFontUIFontSystem;

      case QPlatformTheme::TitleBarFont:
         return kCTFontUIFontWindowTitle;

      case QPlatformTheme::MdiSubWindowTitleFont:
         return kCTFontUIFontSystem;

      case QPlatformTheme::DockWidgetTitleFont:
         return kCTFontUIFontSystem;

      case QPlatformTheme::PushButtonFont:
         return kCTFontUIFontPushButton;

      case QPlatformTheme::CheckBoxFont:
      case QPlatformTheme::RadioButtonFont:
         return kCTFontUIFontSystem;

      case QPlatformTheme::ToolButtonFont:
         return kCTFontUIFontSmallToolbar;

      case QPlatformTheme::ItemViewFont:
         return kCTFontUIFontSystem;

      case QPlatformTheme::ListViewFont:
         return kCTFontUIFontViews;

      case QPlatformTheme::HeaderViewFont:
         return kCTFontUIFontSmallSystem;

      case QPlatformTheme::ListBoxFont:
         return kCTFontUIFontViews;

      case QPlatformTheme::ComboMenuItemFont:
         return kCTFontUIFontSystem;

      case QPlatformTheme::ComboLineEditFont:
         return kCTFontUIFontViews;

      case QPlatformTheme::SmallFont:
         return kCTFontUIFontSmallSystem;

      case QPlatformTheme::MiniFont:
         return kCTFontUIFontMiniSystem;

      case QPlatformTheme::FixedFont:
         return kCTFontUIFontUserFixedPitch;

      default:
         return kCTFontUIFontSystem;
   }
}

static CTFontDescriptorRef fontDescriptorFromTheme(QPlatformTheme::Font f)
{
   // OSX default case and iOS fallback
   CTFontUIFontType fontType = fontTypeFromTheme(f);
   QCFType<CTFontRef> ctFont = CTFontCreateUIFontForLanguage(fontType, 0.0, nullptr);

   return CTFontCopyFontDescriptor(ctFont);
}

const QHash<QPlatformTheme::Font, QFont *> &QCoreTextFontDatabase::themeFonts() const
{
   if (m_themeFonts.isEmpty()) {
      for (long f = QPlatformTheme::SystemFont; f < QPlatformTheme::NFonts; f++) {
         QPlatformTheme::Font ft = static_cast<QPlatformTheme::Font>(f);
         m_themeFonts.insert(ft, themeFont(ft));
      }
   }

   return m_themeFonts;
}

QFont *QCoreTextFontDatabase::themeFont(QPlatformTheme::Font f) const
{
   CTFontDescriptorRef fontDesc = fontDescriptorFromTheme(f);
   FontDescription fd;
   getFontDescription(fontDesc, &fd);

   if (! m_systemFontDescriptors.contains(fontDesc)) {
      m_systemFontDescriptors.insert(fontDesc);
   } else {
      CFRelease(fontDesc);
   }

   QFont *font = new QFont(fd.familyName.toQString(), fd.pixelSize, fd.weight, fd.style == QFont::StyleItalic);

   return font;
}

QFont QCoreTextFontDatabase::defaultFont() const
{
   if (defaultFontName.isEmpty()) {
      QCFType<CTFontRef> font = CTFontCreateUIFontForLanguage(kCTFontUIFontSystem, 12.0, nullptr);
      defaultFontName = QCFString(CTFontCopyFullName(font)).toQString();
   }

   return QFont(defaultFontName);
}

bool QCoreTextFontDatabase::fontsAlwaysScalable() const
{
   return true;
}

QList<int> QCoreTextFontDatabase::standardSizes() const
{
   QList<int> retval;

   static const unsigned short standard[] = { 9, 10, 11, 12, 13, 14, 18, 24, 36, 48, 64, 72, 96, 144, 288, 0 };
   const unsigned short *sizes = standard;

   while (*sizes) {
      retval << *sizes++;
   }

   return retval;
}

void QCoreTextFontDatabase::removeApplicationFonts()
{
   if (m_applicationFonts.isEmpty()) {
      return;
   }

   for (const QVariant &font : m_applicationFonts) {

      if (&CTFontManagerUnregisterGraphicsFont && &CTFontManagerUnregisterFontsForURL) {
         CFErrorRef error;

         if (font.canConvert<QCFType<CGFontRef>>()) {
            CTFontManagerUnregisterGraphicsFont(font.value<QCFType<CGFontRef>>(), &error);

         } else if (font.canConvert<QCFType<CFURLRef>>()) {
            CTFontManagerUnregisterFontsForURL(font.value<QCFType<CFURLRef>>(), kCTFontManagerScopeProcess, &error);

         }
      }
   }

   m_applicationFonts.clear();
}

#if defined(QT_USE_FREETYPE)
QFontEngine *QCoreTextFontDatabase::freeTypeFontEngine(const QFontDef &fontDef, const QString &filename, const QByteArray &fontData)
{
   QFontEngine::FaceId faceId;
   faceId.filename = filename;

   const bool antialias = !(fontDef.styleStrategy & QFont::NoAntialias);

   QScopedPointer<QFontEngineFT> engine(new QFontEngineFT(fontDef));
   QFontEngineFT::GlyphFormat format = QFontEngineFT::Format_Mono;

   if (antialias) {
      QFontEngine::SubpixelAntialiasingType subpixelType = subpixelAntialiasingTypeHint();

      if (subpixelType == QFontEngine::Subpixel_None || (fontDef.styleStrategy & QFont::NoSubpixelAntialias)) {
         format = QFontEngineFT::Format_A8;
         engine->subpixelType = QFontEngine::Subpixel_None;
      } else {
         format = QFontEngineFT::Format_A32;
         engine->subpixelType = subpixelType;
      }
   }

   if (!engine->init(faceId, antialias, format, fontData) || engine->invalid()) {
      qWarning() << "QCoreTextFontDatabase::freeTypefontEngine Failed to create engine";
      return nullptr;
   }

   engine->setQtDefaultHintStyle(static_cast<QFont::HintingPreference>(fontDef.hintingPreference));

   return engine.take();
}
#endif

CS_DECLARE_METATYPE(QCFType<CGFontRef>)
CS_DECLARE_METATYPE(QCFType<CFURLRef>)
