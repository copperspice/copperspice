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

#import <Cocoa/Cocoa.h>

#include <qcocoatheme.h>
#include <messages.h>
#include <qvariant.h>

#include "qcocoacolordialoghelper.h"
#include "qcocoafiledialoghelper.h"
#include "qcocoafontdialoghelper.h"
#include "qcocoasystemsettings.h"
#include "qcocoasystemtrayicon.h"
#include "qcocoamenuitem.h"
#include "qcocoamenu.h"
#include "qcocoamenubar.h"
#include "qcocoahelpers.h"

#include <qfileinfo.h>
#include <qpainter.h>
#include <qplatform_integration.h>
#include <qplatform_nativeinterface.h>
#include <qtextformat.h>

#include <qapplication_p.h>
#include <qcoretextfontdatabase_p.h>

QString QCocoaTheme::name = "cocoa";

QCocoaTheme::QCocoaTheme()
   : m_systemPalette(nullptr)
{
}

QCocoaTheme::~QCocoaTheme()
{
   delete m_systemPalette;
   qDeleteAll(m_palettes);
   qDeleteAll(m_fonts);
}

bool QCocoaTheme::usePlatformNativeDialog(DialogType dialogType) const
{
   if (dialogType == QPlatformTheme::FileDialog) {
      return true;
   }

#ifndef QT_NO_COLORDIALOG
   if (dialogType == QPlatformTheme::ColorDialog) {
      return true;
   }
#endif

#ifndef QT_NO_FONTDIALOG
   if (dialogType == QPlatformTheme::FontDialog) {
      return true;
   }
#endif

   return false;
}

QPlatformDialogHelper *QCocoaTheme::createPlatformDialogHelper(DialogType dialogType) const
{
   switch (dialogType) {
      case QPlatformTheme::FileDialog:
         return new QCocoaFileDialogHelper();

#ifndef QT_NO_COLORDIALOG
      case QPlatformTheme::ColorDialog:
         return new QCocoaColorDialogHelper();
#endif

#ifndef QT_NO_FONTDIALOG
      case QPlatformTheme::FontDialog:
         return new QCocoaFontDialogHelper();
#endif

      default:
         return nullptr;
   }
}

#ifndef QT_NO_SYSTEMTRAYICON
QPlatformSystemTrayIcon *QCocoaTheme::createPlatformSystemTrayIcon() const
{
   return new QCocoaSystemTrayIcon;
}
#endif

const QPalette *QCocoaTheme::palette(Palette type) const
{
   if (type == SystemPalette) {
      if (!m_systemPalette) {
         m_systemPalette = qt_mac_createSystemPalette();
      }
      return m_systemPalette;

   } else {
      if (m_palettes.isEmpty()) {
         m_palettes = qt_mac_createRolePalettes();
      }
      return m_palettes.value(type, nullptr);
   }

   return nullptr;
}

QHash<QPlatformTheme::Font, QFont *> qt_mac_createRoleFonts()
{
   QCoreTextFontDatabase *ctfd = static_cast<QCoreTextFontDatabase *>(QApplicationPrivate::platformIntegration()->fontDatabase());
   return ctfd->themeFonts();
}

const QFont *QCocoaTheme::font(Font type) const
{
   if (m_fonts.isEmpty()) {
      m_fonts = qt_mac_createRoleFonts();
   }

   return m_fonts.value(type, nullptr);
}

//! \internal
QPixmap qt_mac_convert_iconref(const IconRef icon, int width, int height)
{
   QPixmap ret(width, height);
   ret.fill(QColor(0, 0, 0, 0));

   CGRect rect = CGRectMake(0, 0, width, height);

   CGContextRef ctx = qt_mac_cg_context(&ret);
   CGAffineTransform old_xform = CGContextGetCTM(ctx);
   CGContextConcatCTM(ctx, CGAffineTransformInvert(old_xform));
   CGContextConcatCTM(ctx, CGAffineTransformIdentity);

   ::RGBColor b;
   b.blue = b.green = b.red = 255 * 255;
   PlotIconRefInContext(ctx, &rect, kAlignNone, kTransformNone, &b, kPlotIconRefNormalFlags, icon);
   CGContextRelease(ctx);
   return ret;
}

QPixmap QCocoaTheme::standardPixmap(StandardPixmap sp, const QSizeF &size) const
{
   OSType iconType = 0;
   switch (sp) {
      case MessageBoxQuestion:
         iconType = kQuestionMarkIcon;
         break;
      case MessageBoxInformation:
         iconType = kAlertNoteIcon;
         break;
      case MessageBoxWarning:
         iconType = kAlertCautionIcon;
         break;
      case MessageBoxCritical:
         iconType = kAlertStopIcon;
         break;
      case DesktopIcon:
         iconType = kDesktopIcon;
         break;
      case TrashIcon:
         iconType = kTrashIcon;
         break;
      case ComputerIcon:
         iconType = kComputerIcon;
         break;
      case DriveFDIcon:
         iconType = kGenericFloppyIcon;
         break;
      case DriveHDIcon:
         iconType = kGenericHardDiskIcon;
         break;
      case DriveCDIcon:
      case DriveDVDIcon:
         iconType = kGenericCDROMIcon;
         break;
      case DriveNetIcon:
         iconType = kGenericNetworkIcon;
         break;
      case DirOpenIcon:
         iconType = kOpenFolderIcon;
         break;
      case DirClosedIcon:
      case DirLinkIcon:
         iconType = kGenericFolderIcon;
         break;
      case FileLinkIcon:
      case FileIcon:
         iconType = kGenericDocumentIcon;
         break;
      default:
         break;
   }
   if (iconType != 0) {
      QPixmap pixmap;
      IconRef icon = nullptr;
      GetIconRef(kOnSystemDisk, kSystemIconsCreator, iconType, &icon);

      if (icon) {
         pixmap = qt_mac_convert_iconref(icon, size.width(), size.height());
         ReleaseIconRef(icon);
      }

      return pixmap;
   }

   return QPlatformTheme::standardPixmap(sp, size);
}

QPixmap QCocoaTheme::fileIconPixmap(const QFileInfo &fileInfo, const QSizeF &size,
   QPlatformTheme::IconOptions iconOptions) const
{
   (void) iconOptions;
   QMacAutoReleasePool pool;

   NSImage *iconImage = [[NSWorkspace sharedWorkspace] iconForFile: QCFString::toNSString(fileInfo.canonicalFilePath())];
   if (!iconImage) {
      return QPixmap();
   }
   NSSize pixmapSize = NSMakeSize(size.width(), size.height());
   QPixmap pixmap(pixmapSize.width, pixmapSize.height);
   pixmap.fill(Qt::transparent);
   [iconImage setSize: pixmapSize];
   NSRect iconRect = NSMakeRect(0, 0, pixmapSize.width, pixmapSize.height);
   CGContextRef ctx = qt_mac_cg_context(&pixmap);
   NSGraphicsContext *gc = [NSGraphicsContext graphicsContextWithGraphicsPort: ctx flipped: YES];
   [NSGraphicsContext saveGraphicsState];
   [NSGraphicsContext setCurrentContext: gc];
   [iconImage drawInRect: iconRect fromRect: iconRect operation: NSCompositingOperationSourceOver fraction: 1.0 respectFlipped: YES hints:
                nil];
   [NSGraphicsContext restoreGraphicsState];
   return pixmap;
}

QVariant QCocoaTheme::themeHint(ThemeHint hint) const
{
   switch (hint) {
      case QPlatformTheme::StyleNames:
         return QStringList("macintosh");

      case QPlatformTheme::DialogButtonBoxLayout:
         return QVariant(QPlatformDialogHelper::MacLayout);

      case KeyboardScheme:
         return QVariant(int(MacKeyboardScheme));

      case TabFocusBehavior:
         return QVariant([[NSApplication sharedApplication] isFullKeyboardAccessEnabled] ?
               int(Qt::TabFocusAllControls) : int(Qt::TabFocusTextControls | Qt::TabFocusListControls));

      case IconPixmapSizes: {
         qreal devicePixelRatio = qGuiApp->devicePixelRatio();

         QList<int> sizes;
         sizes << 16  * devicePixelRatio
            << 32  * devicePixelRatio
            << 64  * devicePixelRatio
            << 128 * devicePixelRatio;

         return QVariant::fromValue(sizes);
      }

      case QPlatformTheme::PasswordMaskCharacter:
         return QVariant(QChar(kBulletUnicode));

      case QPlatformTheme::SpellCheckUnderlineStyle:
         return QVariant(int(QTextCharFormat::DotLine));

      default:
         break;
   }

   return QPlatformTheme::themeHint(hint);
}

QString QCocoaTheme::standardButtonText(int button) const
{
   return button == QPlatformDialogHelper::Discard ? msgDialogButtonDiscard() : QPlatformTheme::standardButtonText(button);
}

QPlatformMenuItem *QCocoaTheme::createPlatformMenuItem() const
{
   return new QCocoaMenuItem();
}

QPlatformMenu *QCocoaTheme::createPlatformMenu() const
{
   return new QCocoaMenu();
}

QPlatformMenuBar *QCocoaTheme::createPlatformMenuBar() const
{
   static bool haveMenubar = false;
   if (! haveMenubar) {
      haveMenubar = true;

      QObject::connect(qGuiApp, SIGNAL(focusWindowChanged(QWindow *)),
         QApplicationPrivate::platformIntegration()->nativeInterface(),
         SLOT(onAppFocusWindowChanged(QWindow *)));
   }

   return new QCocoaMenuBar();
}
