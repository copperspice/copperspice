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

#include "qcoretextfontdatabase.h"

#include <CoreText/CoreText.h>
#include <Foundation/Foundation.h>
#include <qcore_mac_p.h>
#include <qfontengine_coretext_p.h>

QT_BEGIN_NAMESPACE

QStringList QCoreTextFontDatabase::fallbacksForFamily(const QString family,
                                                      const QFont::Style &style,
                                                      const QFont::StyleHint &styleHint,
                                                      const QUnicodeTables::Script &script) const
{
    Q_UNUSED(family);
    Q_UNUSED(style);
    Q_UNUSED(script);
    if (fallbackLists.isEmpty())
        const_cast<QCoreTextFontDatabase *>(this)->populateFontDatabase();

    return fallbackLists[styleHint];
}

static QFont::StyleHint styleHintFromNSString(NSString *style)
{
    if ([style isEqual: @"sans-serif"])
        return QFont::SansSerif;
    else if ([style isEqual: @"monospace"])
        return QFont::Monospace;
    else if ([style isEqual: @"cursive"])
        return QFont::Cursive;
    else if ([style isEqual: @"serif"])
        return QFont::Serif;
    else if ([style isEqual: @"fantasy"])
        return QFont::Fantasy;
    else
        return QFont::AnyStyle;
}

static NSInteger languageMapSort(id obj1, id obj2, void *context)
{
    NSArray *map1 = (NSArray *) obj1;
    NSArray *map2 = (NSArray *) obj2;
    NSArray *languages = (NSArray *) context;

    NSString *lang1 = [map1 objectAtIndex: 0];
    NSString *lang2 = [map2 objectAtIndex: 0];

    return [languages indexOfObject: lang1] - [languages indexOfObject: lang2];
}

static QString familyNameFromPostScriptName(QHash<QString, QString> *psNameToFamily,
                                            NSString *psName)
{
    QString name = QCFString::toQString((CFStringRef) psName);
    if (psNameToFamily->contains(name)) {
        return psNameToFamily->value(name);
    } else {
        QCFType<CTFontRef> font = CTFontCreateWithName((CFStringRef) psName, 12.0, NULL);
        if (font) {
            QCFString family = CTFontCopyFamilyName(font);
            (*psNameToFamily)[name] = family;
            return family;
        }
    }

    return name;
}

void QCoreTextFontDatabase::populateFontDatabase()
{
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

    QCFType<CTFontCollectionRef> collection = CTFontCollectionCreateFromAvailableFonts(0);
    if (!collection)
        return;
    QCFType<CFArrayRef> fonts = CTFontCollectionCreateMatchingFontDescriptors(collection);
    if (!fonts)
        return;
    QSupportedWritingSystems supportedWritingSystems;
    for (int i = 0; i < QFontDatabase::WritingSystemsCount; ++i)
        supportedWritingSystems.setSupported((QFontDatabase::WritingSystem)i, true);
    QString foundry_name = "CoreText";
    const int numFonts = CFArrayGetCount(fonts);
    QHash<QString, QString> psNameToFamily;
    for (int i = 0; i < numFonts; ++i) {
        CTFontDescriptorRef font = (CTFontDescriptorRef)CFArrayGetValueAtIndex(fonts, i);

        QCFString family_name = (CFStringRef)CTFontDescriptorCopyAttribute(font, kCTFontFamilyNameAttribute);
//        QCFString style_name = (CFStringRef)CTFontDescriptorCopyAttribute(font, kCTFontStyleNameAttribute);

        QFont::Weight fontWeight = QFont::Normal;
        QFont::Style fontStyle = QFont::StyleNormal;
        if (QCFType<CFDictionaryRef> styles = (CFDictionaryRef)CTFontDescriptorCopyAttribute(font, kCTFontTraitsAttribute)) {
            if (CFNumberRef weight = (CFNumberRef)CFDictionaryGetValue(styles, kCTFontWeightTrait)) {
                Q_ASSERT(CFNumberIsFloatType(weight));
                double d;
                if (CFNumberGetValue(weight, kCFNumberDoubleType, &d)) {
                    if (d > 0.0)
                        fontWeight = QFont::Bold;
                }
            }
            if (CFNumberRef italic = (CFNumberRef)CFDictionaryGetValue(styles, kCTFontSlantTrait)) {
                Q_ASSERT(CFNumberIsFloatType(italic));
                double d;
                if (CFNumberGetValue(italic, kCFNumberDoubleType, &d)) {
                    if (d > 0.0)
                        fontStyle = QFont::StyleItalic;
                }
            }
        }

        int pixelSize = 0;
        if (QCFType<CFNumberRef> size = (CFNumberRef)CTFontDescriptorCopyAttribute(font, kCTFontSizeAttribute)) {
            if (CFNumberIsFloatType(size)) {
                double d;
                CFNumberGetValue(size, kCFNumberDoubleType, &d);
                pixelSize = d;
            } else {
                CFNumberGetValue(size, kCFNumberIntType, &pixelSize);
            }
        }
        QString familyName = QCFString::toQString(family_name);
        registerFont(familyName,
                     foundry_name,
                     fontWeight,
                     fontStyle,
                     QFont::Unstretched,
                     true,
                     true,
                     pixelSize,
                     supportedWritingSystems,
                     0);

        CFStringRef psName = (CFStringRef) CTFontDescriptorCopyAttribute(font,
                                                                         kCTFontNameAttribute);
        psNameToFamily[QCFString::toQString(psName)] = familyName;
        CFRelease(psName);
    }

    NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
    NSArray *languages = [defaults stringArrayForKey: @"AppleLanguages"];

    NSDictionary *fallbackDict = [NSDictionary dictionaryWithContentsOfFile: @"/System/Library/Frameworks/CoreText.framework/DefaultFontFallbacks.plist"];
    for (NSString *style in [fallbackDict allKeys]) {
        NSArray *list = [fallbackDict valueForKey: style];
        QFont::StyleHint styleHint = styleHintFromNSString(style);
        QStringList fallbackList;
        for (id item in list) {
            if ([item isKindOfClass: [NSArray class]]) {
                NSArray *langs = [(NSArray *) item sortedArrayUsingFunction: languageMapSort
                                                                             context: languages];
                for (NSArray *map in langs)
                    fallbackList.append(familyNameFromPostScriptName(&psNameToFamily, [map objectAtIndex: 1]));
            } else if ([item isKindOfClass: [NSString class]]) {
                fallbackList.append(familyNameFromPostScriptName(&psNameToFamily, item));
            }
        }

        fallbackLists[styleHint] = fallbackList;
    }

    [pool release];
}

QFontEngine *QCoreTextFontDatabase::fontEngine(const QFontDef &fontDef, QUnicodeTables::Script script, void *handle)
{
    Q_UNUSED(script)
    Q_UNUSED(handle)
    CTFontSymbolicTraits symbolicTraits = 0;
    if (fontDef.weight >= QFont::Bold)
        symbolicTraits |= kCTFontBoldTrait;
    switch (fontDef.style) {
    case QFont::StyleNormal:
        break;
    case QFont::StyleItalic:
    case QFont::StyleOblique:
        symbolicTraits |= kCTFontItalicTrait;
        break;
    }

    CGAffineTransform transform = CGAffineTransformIdentity;
    if (fontDef.stretch != 100) {
        transform = CGAffineTransformMakeScale(float(fontDef.stretch) / float(100), 1);
    }

    QCFType<CTFontRef> baseFont = CTFontCreateWithName(QCFString(fontDef.family), fontDef.pixelSize, &transform);
    QCFType<CTFontRef> ctFont = NULL;
    // There is a side effect in Core Text: if we apply 0 as symbolic traits to a font in normal weight,
    // we will get the light version of that font (while the way supposed to work doesn't:
    // setting kCTFontWeightTrait to some value between -1.0 to 0.0 has no effect on font selection)
    if (fontDef.weight != QFont::Normal || symbolicTraits)
        ctFont = CTFontCreateCopyWithSymbolicTraits(baseFont, fontDef.pixelSize, &transform, symbolicTraits, symbolicTraits);

    // CTFontCreateCopyWithSymbolicTraits returns NULL if we ask for a trait that does
    // not exist for the given font. (for example italic)
    if (ctFont == 0) {
        ctFont = baseFont;
    }

    if (ctFont)
        return new QCoreTextFontEngine(ctFont, fontDef);
    return 0;
}

QT_END_NAMESPACE
