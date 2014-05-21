/***********************************************************************
*
* Copyright (c) 2012-2014 Barbara Geller
* Copyright (c) 2012-2014 Ansel Sermersheim
* Copyright (c) 2012-2014 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
*
* This file is part of CopperSpice.
*
* CopperSpice is free software: you can redistribute it and/or 
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with CopperSpice.  If not, see 
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#include <private/qt_mac_p.h>
#include <private/qpixmap_mac_p.h>
#include <private/qnativeimage_p.h>
#include <qdebug.h>

QT_BEGIN_NAMESPACE
#ifdef QT_MAC_USE_COCOA
static CTFontRef CopyCTThemeFont(ThemeFontID themeID)
{
    CTFontUIFontType ctID = HIThemeGetUIFontType(themeID);
    return CTFontCreateUIFontForLanguage(ctID, 0, 0);
}
#endif

QFont qfontForThemeFont(ThemeFontID themeID)
{
#ifndef QT_MAC_USE_COCOA
    static const ScriptCode Script = smRoman;
    Str255 f_name;
    SInt16 f_size;
    Style f_style;
    GetThemeFont(themeID, Script, f_name, &f_size, &f_style);
    return QFont(qt_mac_from_pascal_string(f_name), f_size,
                 (f_style & ::bold) ? QFont::Bold : QFont::Normal,
                 (bool)(f_style & ::italic));
#else
    QCFType<CTFontRef> ctfont = CopyCTThemeFont(themeID);
    QString familyName = QCFString(CTFontCopyFamilyName(ctfont));
    QCFType<CFDictionaryRef> dict = CTFontCopyTraits(ctfont);
    CFNumberRef num = static_cast<CFNumberRef>(CFDictionaryGetValue(dict, kCTFontWeightTrait));
    float fW;
    CFNumberGetValue(num, kCFNumberFloat32Type, &fW);
    QFont::Weight wght = fW > 0. ? QFont::Bold : QFont::Normal;
    num = static_cast<CFNumberRef>(CFDictionaryGetValue(dict, kCTFontSlantTrait));
    CFNumberGetValue(num, kCFNumberFloatType, &fW);
    bool italic = (fW != 0.0);
    return QFont(familyName, CTFontGetSize(ctfont), wght, italic);
#endif
}

#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_5)
static QColor qcolorFromCGColor(CGColorRef cgcolor)
{
    QColor pc;
    CGColorSpaceModel model = CGColorSpaceGetModel(CGColorGetColorSpace(cgcolor));
    const CGFloat *components = CGColorGetComponents(cgcolor);
    if (model == kCGColorSpaceModelRGB) {
        pc.setRgbF(components[0], components[1], components[2], components[3]);
    } else if (model == kCGColorSpaceModelCMYK) {
        pc.setCmykF(components[0], components[1], components[2], components[3]);
    } else if (model == kCGColorSpaceModelMonochrome) {
        pc.setRgbF(components[0], components[0], components[0], components[1]);
    } else {
        // Colorspace we can't deal with.
        qWarning("Qt: qcolorFromCGColor: cannot convert from colorspace model: %d", model);
        Q_ASSERT(false);
    }
    return pc;
}

static inline QColor leopardBrush(ThemeBrush brush)
{
    QCFType<CGColorRef> cgClr = 0;
    HIThemeBrushCreateCGColor(brush, &cgClr);
    return qcolorFromCGColor(cgClr);
}
#endif

QColor qcolorForTheme(ThemeBrush brush)
{
#ifndef QT_MAC_USE_COCOA
#  if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_5)
    if (QSysInfo::MacintoshVersion >= QSysInfo::MV_10_5) {
        return leopardBrush(brush);
    } else
#  endif
    {
        RGBColor rgbcolor;
        GetThemeBrushAsColor(brush, 32, true, &rgbcolor);
        return QColor(rgbcolor.red / 256, rgbcolor.green / 256, rgbcolor.blue / 256);
    }
#else
    return leopardBrush(brush);
#endif
}

QColor qcolorForThemeTextColor(ThemeTextColor themeColor)
{
#ifdef Q_OS_MAC32
    RGBColor c;
    GetThemeTextColor(themeColor, 32, true, &c);
    QColor color = QColor(c.red / 256, c.green / 256, c.blue / 256);
    return color;
#else
    // There is no equivalent to GetThemeTextColor in 64-bit and it was rather bad that
    // I didn't file a request to implement this for Snow Leopard. So, in the meantime
    // I've encoded the values from the GetThemeTextColor. This is not exactly ideal
    // as if someone really wants to mess with themeing, these colors will be wrong.
    // It also means that we need to make sure the values for differences between
    // OS releases (and it will be likely that we are a step behind.)
    switch (themeColor) {
    case kThemeTextColorAlertActive:
    case kThemeTextColorTabFrontActive:
    case kThemeTextColorBevelButtonActive:
    case kThemeTextColorListView:
    case kThemeTextColorPlacardActive:
    case kThemeTextColorPopupButtonActive:
    case kThemeTextColorPopupLabelActive:
    case kThemeTextColorPushButtonActive:
        return Qt::black;
    case kThemeTextColorAlertInactive:
    case kThemeTextColorDialogInactive:
    case kThemeTextColorPlacardInactive:
        return QColor(69, 69, 69, 255);
    case kThemeTextColorPopupButtonInactive:
    case kThemeTextColorPopupLabelInactive:
    case kThemeTextColorPushButtonInactive:
    case kThemeTextColorTabFrontInactive:
    case kThemeTextColorBevelButtonInactive:
        return QColor(127, 127, 127, 255);
    default: {
        QNativeImage nativeImage(16,16, QNativeImage::systemFormat());
        CGRect cgrect = CGRectMake(0, 0, 16, 16);
        HIThemeSetTextFill(themeColor, 0, nativeImage.cg, kHIThemeOrientationNormal);
        CGContextFillRect(nativeImage.cg, cgrect);
        QColor color = nativeImage.image.pixel(0,0);
        return QColor(nativeImage.image.pixel(0 , 0));
    }
    }
#endif
}
QT_END_NAMESPACE
