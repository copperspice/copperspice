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

#include <cs_carbon_wrapper_p.h>
#include <qpixmap_mac_p.h>
#include <qnativeimage_p.h>
#include <qdebug.h>

QT_BEGIN_NAMESPACE

static CTFontRef CopyCTThemeFont(ThemeFontID themeID)
{
   CS_FontUIFontType ctID = CS_HIThemeGetUIFontType(themeID);
   return CS_CTFontCreateUIFontForLanguage(ctID, 0, 0);
}

QFont qt_mac_fontForThemeFont(ThemeFontID themeID)
{
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

}

static QColor qt_mac_colorFromCGColor(CGColorRef cgcolor)
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
      // Colorspace we ca not deal with
      qWarning("qt_mac_colorFromCGColor() Can not convert from colorspace model: %d", model);
      Q_ASSERT(false);
   }
   return pc;
}

QColor qt_mac_colorForTheme(ThemeBrush brush)
{
   QCFType<CGColorRef> cgClr = 0;
   CS_HIThemeBrushCreateCGColor(brush, &cgClr);

   return qt_mac_colorFromCGColor(cgClr);
}

QColor qt_mac_colorForThemeTextColor(ThemeTextColor themeColor)
{
   // no equivalent to GetThemeTextColor in 64-bit
   // hard coded colors to match standard OS X theme

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
      case kThemeTextColorMenuItemDisabled:
         return QColor(127, 127, 127, 255);

      case kThemeTextColorMenuItemSelected:
         return Qt::white;

      default: {

         // sample the color, similar to the the code below
         return QColor(0, 0, 0, 255);

         /*
                    QNativeImage nativeImage(16,16, QNativeImage::systemFormat());
                    CGRect cgrect = CGRectMake(0, 0, 16, 16);

                    HIThemeSetTextFill(themeColor, 0, nativeImage.cg, kHIThemeOrientationNormal);
                    CGContextFillRect(nativeImage.cg, cgrect);

                    QColor color = nativeImage.image.pixel(0,0);
                    return QColor(nativeImage.image.pixel(0 , 0));
         */
      }
   }

}
QT_END_NAMESPACE
