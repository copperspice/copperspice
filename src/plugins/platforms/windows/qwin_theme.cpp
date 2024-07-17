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

// SHSTOCKICONINFO is only available since Vista
#if _WIN32_WINNT < 0x0600
#  undef _WIN32_WINNT
#  define _WIN32_WINNT 0x0600
#endif

#include <qalgorithms.h>
#include <qwin_theme.h>
#include <qwin_dialoghelpers.h>
#include <qwin_context.h>
#include <qwin_integration.h>
#include <qwin_fontdatabase.h>

#include <qwindowsysteminterface.h>
#include <qt_windows.h>
#include <QVariant>
#include <QCoreApplication>
#include <QDebug>
#include <QTextStream>
#include <QSysInfo>
#include <QCache>
#include <QColor>
#include <QPalette>
#include <QApplication>
#include <QPainter>
#include <QPixmapCache>

#include <qhighdpiscaling_p.h>
#include <qsystemlibrary_p.h>

#ifndef Q_CC_MINGW
#include <commoncontrols.h>
#endif

#include <algorithm>
#include <commctrl.h>
#include <objbase.h>
#include <shellapi.h>

#if defined(__IImageList_INTERFACE_DEFINED__) && defined(__IID_DEFINED__)
#  define USE_IIMAGELIST
#endif

// Shell image list ids
static constexpr const int sHIL_EXTRALARGE = 0x2;      // 48x48 or user-defined
static constexpr const int sHIL_JUMBO      = 0x4;      // 256x256 (Vista or later)

enum FileIconSize {
   SmallFileIcon,         // Standard icons obtainable via shGetFileInfo(), SHGFI_SMALLICON, SHGFI_LARGEICON
   LargeFileIcon,
   ExtraLargeFileIcon,    // Larger icons obtainable via SHGetImageList()
   JumboFileIcon,         // Vista onwards
   FileIconSizeCount
};

QString QWindowsTheme::name              = "windows";
QWindowsTheme *QWindowsTheme::m_instance = nullptr;

static inline QColor COLORREFToQColor(COLORREF cr)
{
   return QColor(GetRValue(cr), GetGValue(cr), GetBValue(cr));
}

static inline QTextStream &operator<<(QTextStream &str, const QColor &c)
{
   str.setIntegerBase(16);
   str.setFieldWidth(2);
   str.setPadChar(QLatin1Char('0'));
   str << " rgb: #" << c.red()  << c.green() << c.blue();
   str.setIntegerBase(10);
   str.setFieldWidth(0);

   return str;
}

static inline bool booleanSystemParametersInfo(UINT what, bool defaultValue)
{
   BOOL result;

   if (SystemParametersInfo(what, 0, &result, 0)) {
      return result ? true : false;
   }

   return defaultValue;
}

static inline DWORD dWordSystemParametersInfo(UINT what, DWORD defaultValue)
{
   DWORD result;

   if (SystemParametersInfo(what, 0, &result, 0)) {
      return result;
   }

   return defaultValue;
}

static inline QColor mixColors(const QColor &c1, const QColor &c2)
{
   return QColor ((c1.red() + c2.red()) / 2,
         (c1.green() + c2.green()) / 2,
         (c1.blue() + c2.blue()) / 2);
}

static inline QColor getSysColor(int index)
{
   return COLORREFToQColor(GetSysColor(index));
}

class ShGetFileInfoFunction
{
 public:
   explicit ShGetFileInfoFunction(const wchar_t *fn, DWORD a, SHFILEINFO *i, UINT f, bool *r) :
      m_fileName(fn), m_attributes(a), m_flags(f), m_info(i), m_result(r) {}

   void operator()() const {

      const UINT oldErrorMode = SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOOPENFILEERRORBOX);

      *m_result = SHGetFileInfo(m_fileName, m_attributes, m_info, sizeof(SHFILEINFO), m_flags);
      SetErrorMode(oldErrorMode);
   }

 private:
   const wchar_t *m_fileName;
   const DWORD m_attributes;
   const UINT m_flags;
   SHFILEINFO *const m_info;
   bool *m_result;
};

static bool shGetFileInfoBackground(QWindowsThreadPoolRunner &r, const wchar_t *fileName, DWORD attributes,
   SHFILEINFO *info, UINT flags, unsigned long  timeOutMSecs = 5000)
{
   bool result = false;

   if (! r.run(ShGetFileInfoFunction(fileName, attributes, info, flags, &result), timeOutMSecs)) {
      qWarning().noquote() << "ShGetFileInfoBackground() timed out for " << QString::fromStdWString(std::wstring(fileName));

      return false;
   }

   return result;
}

// from QStyle::standardPalette
static inline QPalette standardPalette()
{
   QColor backgroundColor(0xd4, 0xd0, 0xc8); // win 2000 grey
   QColor lightColor(backgroundColor.lighter());
   QColor darkColor(backgroundColor.darker());
   const QBrush darkBrush(darkColor);

   QColor midColor(Qt::gray);
   QPalette palette(Qt::black, backgroundColor, lightColor, darkColor, midColor, Qt::black, Qt::white);
   palette.setBrush(QPalette::Disabled, QPalette::WindowText, darkBrush);
   palette.setBrush(QPalette::Disabled, QPalette::Text, darkBrush);
   palette.setBrush(QPalette::Disabled, QPalette::ButtonText, darkBrush);
   palette.setBrush(QPalette::Disabled, QPalette::Base, QBrush(backgroundColor));

   return palette;
}

static inline QPalette systemPalette()
{
   QPalette result = standardPalette();
   result.setColor(QPalette::WindowText, getSysColor(COLOR_WINDOWTEXT));
   result.setColor(QPalette::Button, getSysColor(COLOR_BTNFACE));
   result.setColor(QPalette::Light, getSysColor(COLOR_BTNHIGHLIGHT));
   result.setColor(QPalette::Dark, getSysColor(COLOR_BTNSHADOW));
   result.setColor(QPalette::Mid, result.button().color().darker(150));
   result.setColor(QPalette::Text, getSysColor(COLOR_WINDOWTEXT));
   result.setColor(QPalette::BrightText, getSysColor(COLOR_BTNHIGHLIGHT));
   result.setColor(QPalette::Base, getSysColor(COLOR_WINDOW));
   result.setColor(QPalette::Window, getSysColor(COLOR_BTNFACE));
   result.setColor(QPalette::ButtonText, getSysColor(COLOR_BTNTEXT));
   result.setColor(QPalette::Midlight, getSysColor(COLOR_3DLIGHT));
   result.setColor(QPalette::Shadow, getSysColor(COLOR_3DDKSHADOW));
   result.setColor(QPalette::Highlight, getSysColor(COLOR_HIGHLIGHT));
   result.setColor(QPalette::HighlightedText, getSysColor(COLOR_HIGHLIGHTTEXT));
   result.setColor(QPalette::Link, Qt::blue);
   result.setColor(QPalette::LinkVisited, Qt::magenta);
   result.setColor(QPalette::Inactive, QPalette::Button, result.button().color());
   result.setColor(QPalette::Inactive, QPalette::Window, result.background().color());
   result.setColor(QPalette::Inactive, QPalette::Light, result.light().color());
   result.setColor(QPalette::Inactive, QPalette::Dark, result.dark().color());

   if (result.midlight() == result.button()) {
      result.setColor(QPalette::Midlight, result.button().color().lighter(110));
   }

   if (result.background() != result.base()) {
      result.setColor(QPalette::Inactive, QPalette::Highlight, result.color(QPalette::Inactive, QPalette::Window));
      result.setColor(QPalette::Inactive, QPalette::HighlightedText, result.color(QPalette::Inactive, QPalette::Text));
   }

   const QColor disabled = mixColors(result.foreground().color(), result.button().color());

   result.setColorGroup(QPalette::Disabled, result.foreground(), result.button(), result.light(), result.dark(),
                  result.mid(), result.text(), result.brightText(), result.base(), result.background());

   result.setColor(QPalette::Disabled, QPalette::WindowText, disabled);
   result.setColor(QPalette::Disabled, QPalette::Text, disabled);
   result.setColor(QPalette::Disabled, QPalette::ButtonText, disabled);
   result.setColor(QPalette::Disabled, QPalette::Highlight, getSysColor(COLOR_HIGHLIGHT));
   result.setColor(QPalette::Disabled, QPalette::HighlightedText, getSysColor(COLOR_HIGHLIGHTTEXT));
   result.setColor(QPalette::Disabled, QPalette::Base, result.background().color());
   return result;
}

static inline QPalette toolTipPalette(const QPalette &systemPalette)
{
   QPalette result(systemPalette);
   const QColor tipBgColor(getSysColor(COLOR_INFOBK));
   const QColor tipTextColor(getSysColor(COLOR_INFOTEXT));

   result.setColor(QPalette::All, QPalette::Button, tipBgColor);
   result.setColor(QPalette::All, QPalette::Window, tipBgColor);
   result.setColor(QPalette::All, QPalette::Text, tipTextColor);
   result.setColor(QPalette::All, QPalette::WindowText, tipTextColor);
   result.setColor(QPalette::All, QPalette::ButtonText, tipTextColor);
   result.setColor(QPalette::All, QPalette::Button, tipBgColor);
   result.setColor(QPalette::All, QPalette::Window, tipBgColor);
   result.setColor(QPalette::All, QPalette::Text, tipTextColor);
   result.setColor(QPalette::All, QPalette::WindowText, tipTextColor);
   result.setColor(QPalette::All, QPalette::ButtonText, tipTextColor);
   result.setColor(QPalette::All, QPalette::ToolTipBase, tipBgColor);
   result.setColor(QPalette::All, QPalette::ToolTipText, tipTextColor);

   const QColor disabled = mixColors(result.foreground().color(), result.button().color());
   result.setColor(QPalette::Disabled, QPalette::WindowText, disabled);
   result.setColor(QPalette::Disabled, QPalette::Text, disabled);
   result.setColor(QPalette::Disabled, QPalette::ToolTipText, disabled);
   result.setColor(QPalette::Disabled, QPalette::Base, Qt::white);
   result.setColor(QPalette::Disabled, QPalette::BrightText, Qt::white);
   result.setColor(QPalette::Disabled, QPalette::ToolTipBase, Qt::white);

   return result;
}

static inline QPalette menuPalette(const QPalette &systemPalette)
{
   QPalette result(systemPalette);
   const QColor menuColor(getSysColor(COLOR_MENU));
   const QColor menuTextColor(getSysColor(COLOR_MENUTEXT));
   const QColor disabled(getSysColor(COLOR_GRAYTEXT));

   // might need a special color group for the result.
   result.setColor(QPalette::Active, QPalette::Button, menuColor);
   result.setColor(QPalette::Active, QPalette::Text, menuTextColor);
   result.setColor(QPalette::Active, QPalette::WindowText, menuTextColor);
   result.setColor(QPalette::Active, QPalette::ButtonText, menuTextColor);
   result.setColor(QPalette::Disabled, QPalette::WindowText, disabled);
   result.setColor(QPalette::Disabled, QPalette::Text, disabled);

   const bool isFlat = booleanSystemParametersInfo(SPI_GETFLATMENU, false);

   result.setColor(QPalette::Disabled, QPalette::Highlight, getSysColor(isFlat ? COLOR_MENUHILIGHT : COLOR_HIGHLIGHT));

   result.setColor(QPalette::Disabled, QPalette::HighlightedText, disabled);
   result.setColor(QPalette::Disabled, QPalette::Button, result.color(QPalette::Active, QPalette::Button));
   result.setColor(QPalette::Inactive, QPalette::Button, result.color(QPalette::Active, QPalette::Button));
   result.setColor(QPalette::Inactive, QPalette::Text, result.color(QPalette::Active, QPalette::Text));
   result.setColor(QPalette::Inactive, QPalette::WindowText, result.color(QPalette::Active, QPalette::WindowText));
   result.setColor(QPalette::Inactive, QPalette::ButtonText, result.color(QPalette::Active, QPalette::ButtonText));
   result.setColor(QPalette::Inactive, QPalette::Highlight, result.color(QPalette::Active, QPalette::Highlight));
   result.setColor(QPalette::Inactive, QPalette::HighlightedText, result.color(QPalette::Active, QPalette::HighlightedText));
   result.setColor(QPalette::Inactive, QPalette::ButtonText, systemPalette.color(QPalette::Inactive, QPalette::Dark));

   return result;
}

static inline QPalette *menuBarPalette(const QPalette &menuPalette)
{
   QPalette *result = nullptr;

   if (booleanSystemParametersInfo(SPI_GETFLATMENU, false)) {
      result = new QPalette(menuPalette);

      const QColor menubar(getSysColor(COLOR_MENUBAR));

      result->setColor(QPalette::Active, QPalette::Button, menubar);
      result->setColor(QPalette::Disabled, QPalette::Button, menubar);
      result->setColor(QPalette::Inactive, QPalette::Button, menubar);
   }
   return result;
}

QWindowsTheme::QWindowsTheme()
{
   m_instance = this;
   std::fill(m_fonts, m_fonts + NFonts, static_cast<QFont *>(nullptr));
   std::fill(m_palettes, m_palettes + NPalettes, static_cast<QPalette *>(nullptr));
   refresh();
   refreshIconPixmapSizes();
}

QWindowsTheme::~QWindowsTheme()
{
   clearPalettes();
   clearFonts();
   m_instance = nullptr;
}

static inline QStringList iconThemeSearchPaths()
{
   const QFileInfo appDir(QCoreApplication::applicationDirPath() + QString("/icons"));
   return appDir.isDir() ? QStringList(appDir.absoluteFilePath()) : QStringList();
}

static inline QStringList styleNames()
{
   QStringList result;

   if (QSysInfo::WindowsVersion >= QSysInfo::WV_VISTA) {
      result.append(QString("WindowsVista"));
   }

   if (QSysInfo::WindowsVersion >= QSysInfo::WV_XP) {
      result.append(QString("WindowsXP"));
   }

   result.append(QString("Windows"));

   return result;
}

static inline int uiEffects()
{
   int result = 0;
   if (booleanSystemParametersInfo(SPI_GETUIEFFECTS, false)) {
      result |= QPlatformTheme::GeneralUiEffect;
   }

   if (booleanSystemParametersInfo(SPI_GETMENUANIMATION, false)) {
      result |= QPlatformTheme::AnimateMenuUiEffect;
   }

   if (booleanSystemParametersInfo(SPI_GETMENUFADE, false)) {
      result |= QPlatformTheme::FadeMenuUiEffect;
   }

   if (booleanSystemParametersInfo(SPI_GETCOMBOBOXANIMATION, false)) {
      result |= QPlatformTheme::AnimateComboUiEffect;
   }

   if (booleanSystemParametersInfo(SPI_GETTOOLTIPANIMATION, false)) {
      result |= QPlatformTheme::AnimateTooltipUiEffect;
   }

   return result;
}

QVariant QWindowsTheme::themeHint(ThemeHint hint) const
{
   switch (hint) {
      case UseFullScreenForPopupMenu:
         return QVariant(true);

      case DialogButtonBoxLayout:
         return QVariant(QPlatformDialogHelper::WinLayout);

      case IconThemeSearchPaths:
         return QVariant(iconThemeSearchPaths());

      case StyleNames:
         return QVariant(styleNames());

      case TextCursorWidth:
         return QVariant(int(dWordSystemParametersInfo(SPI_GETCARETWIDTH, 1u)));

      case DropShadow:
         return QVariant(booleanSystemParametersInfo(SPI_GETDROPSHADOW, false));

      case MaximumScrollBarDragDistance:
         return QVariant(qRound(qreal(QWindowsContext::instance()->defaultDPI()) * 1.375));

      case KeyboardScheme:
         return QVariant(int(WindowsKeyboardScheme));

      case UiEffects:
         return QVariant(uiEffects());

      case IconPixmapSizes:
         return m_fileIconSizes;

      case DialogSnapToDefaultButton:
         return QVariant(booleanSystemParametersInfo(SPI_GETSNAPTODEFBUTTON, false));

      case ContextMenuOnMouseRelease:
         return QVariant(true);

      case WheelScrollLines: {
         int result = 3;
         const DWORD scrollLines = dWordSystemParametersInfo(SPI_GETWHEELSCROLLLINES, DWORD(result));

         if (scrollLines != DWORD(-1)) {
            // Special value meaning "scroll one screen", unimplemented
            result = int(scrollLines);
         }
         return QVariant(result);
      }

      default:
         break;
   }
   return QPlatformTheme::themeHint(hint);
}

void QWindowsTheme::clearPalettes()
{
   qDeleteAll(m_palettes, m_palettes + NPalettes);
   std::fill(m_palettes, m_palettes + NPalettes, static_cast<QPalette *>(nullptr));
}

void QWindowsTheme::refreshPalettes()
{

   if (!QApplication::desktopSettingsAware()) {
      return;
   }
   m_palettes[SystemPalette] = new QPalette(systemPalette());
   m_palettes[ToolTipPalette] = new QPalette(toolTipPalette(*m_palettes[SystemPalette]));
   m_palettes[MenuPalette] = new QPalette(menuPalette(*m_palettes[SystemPalette]));
   m_palettes[MenuBarPalette] = menuBarPalette(*m_palettes[MenuPalette]);
}

void QWindowsTheme::clearFonts()
{
   qDeleteAll(m_fonts, m_fonts + NFonts);
   std::fill(m_fonts, m_fonts + NFonts, static_cast<QFont *>(nullptr));
}

void QWindowsTheme::refreshFonts()
{
   clearFonts();

   if (!QApplication::desktopSettingsAware()) {
      return;
   }

   NONCLIENTMETRICS ncm;
   ncm.cbSize = FIELD_OFFSET(NONCLIENTMETRICS, lfMessageFont) + sizeof(LOGFONT);
   SystemParametersInfo(SPI_GETNONCLIENTMETRICS, ncm.cbSize, &ncm, 0);

   const QFont menuFont = QWindowsFontDatabase::LOGFONT_to_QFont(ncm.lfMenuFont);
   const QFont messageBoxFont = QWindowsFontDatabase::LOGFONT_to_QFont(ncm.lfMessageFont);
   const QFont statusFont = QWindowsFontDatabase::LOGFONT_to_QFont(ncm.lfStatusFont);
   const QFont titleFont = QWindowsFontDatabase::LOGFONT_to_QFont(ncm.lfCaptionFont);
   QFont fixedFont(QString("Courier New"), messageBoxFont.pointSize());
   fixedFont.setStyleHint(QFont::TypeWriter);

   LOGFONT lfIconTitleFont;
   SystemParametersInfo(SPI_GETICONTITLELOGFONT, sizeof(lfIconTitleFont), &lfIconTitleFont, 0);
   const QFont iconTitleFont = QWindowsFontDatabase::LOGFONT_to_QFont(lfIconTitleFont);

   m_fonts[SystemFont] = new QFont(QWindowsFontDatabase::systemDefaultFont());
   m_fonts[MenuFont] = new QFont(menuFont);
   m_fonts[MenuBarFont] = new QFont(menuFont);
   m_fonts[MessageBoxFont] = new QFont(messageBoxFont);
   m_fonts[TipLabelFont] = new QFont(statusFont);
   m_fonts[StatusBarFont] = new QFont(statusFont);
   m_fonts[MdiSubWindowTitleFont] = new QFont(titleFont);
   m_fonts[DockWidgetTitleFont] = new QFont(titleFont);
   m_fonts[ItemViewFont] = new QFont(iconTitleFont);
   m_fonts[FixedFont] = new QFont(fixedFont);
}

bool QWindowsTheme::usePlatformNativeDialog(DialogType type) const
{
   return QWindowsDialogs::useHelper(type);
}

QPlatformDialogHelper *QWindowsTheme::createPlatformDialogHelper(DialogType type) const
{
   return QWindowsDialogs::createHelper(type);
}

void QWindowsTheme::windowsThemeChanged(QWindow *window)
{
   refresh();
   QWindowSystemInterface::handleThemeChange(window);
}

static int fileIconSizes[FileIconSizeCount];

void QWindowsTheme::refreshIconPixmapSizes()
{
   // Standard sizes: 16, 32, 48, 256
   fileIconSizes[SmallFileIcon] = GetSystemMetrics(SM_CXSMICON); // corresponds to SHGFI_SMALLICON);
   fileIconSizes[LargeFileIcon] = GetSystemMetrics(SM_CXICON); // corresponds to SHGFI_LARGEICON

   fileIconSizes[ExtraLargeFileIcon] = fileIconSizes[LargeFileIcon] + fileIconSizes[LargeFileIcon] / 2;
   fileIconSizes[JumboFileIcon]      = 8 * fileIconSizes[LargeFileIcon]; // empirical, has not been observed to work

   QList<int> sizes;
   sizes << fileIconSizes[SmallFileIcon] << fileIconSizes[LargeFileIcon];

#ifdef USE_IIMAGELIST
   sizes << fileIconSizes[ExtraLargeFileIcon]; // sHIL_EXTRALARGE

   if (QSysInfo::WindowsVersion >= QSysInfo::WV_VISTA) {
      // SHIL_JUMBO
      sizes << fileIconSizes[JumboFileIcon];
   }
#endif

#if defined(CS_SHOW_DEBUG_PLATFORM)
   qDebug() << "QWindowsTheme::refreshIconPixmapSizes() Size = " << sizes;
#endif

   m_fileIconSizes = QVariant::fromValue(sizes);
}

// Defined in qpixmap_win.cpp
Q_GUI_EXPORT QPixmap qt_pixmapFromWinHICON(HICON icon);

static QPixmap loadIconFromShell32(int resourceId, QSizeF size)
{
   HMODULE hmod = QSystemLibrary::load(QString("shell32"));

   if (hmod) {
      HICON iconHandle = static_cast<HICON>(LoadImage(hmod, MAKEINTRESOURCE(resourceId), IMAGE_ICON,
            int(size.width()), int(size.height()), 0));

      if (iconHandle) {
         QPixmap iconpixmap = qt_pixmapFromWinHICON(iconHandle);
         DestroyIcon(iconHandle);
         return iconpixmap;
      }
   }

   return QPixmap();
}

QPixmap QWindowsTheme::standardPixmap(StandardPixmap sp, const QSizeF &pixmapSize) const
{
   int resourceId = -1;
   LPCTSTR iconName = nullptr;

   switch (sp) {
      case DriveCDIcon:
      case DriveDVDIcon:
         resourceId = 12;
         break;
      case DriveNetIcon:
         resourceId = 10;
         break;
      case DriveHDIcon:
         resourceId = 9;
         break;

      case DriveFDIcon:
         resourceId = 7;
         break;

      case FileIcon:
      case FileLinkIcon:
         resourceId = 1;
         break;

      case DirIcon:
      case DirLinkIcon:
      case DirClosedIcon:
         resourceId = 4;
         break;

      case DesktopIcon:
         resourceId = 35;
         break;

      case ComputerIcon:
         resourceId = 16;
         break;

      case DirOpenIcon:
      case DirLinkOpenIcon:
         resourceId = 5;
         break;

      case FileDialogNewFolder:
         resourceId = 319;
         break;

      case DirHomeIcon:
         resourceId = 235;
         break;

      case TrashIcon:
         resourceId = 191;
         break;

      case MessageBoxInformation:
         iconName = IDI_INFORMATION;
         break;

      case MessageBoxWarning:
         iconName = IDI_WARNING;
         break;

      case MessageBoxCritical:
         iconName = IDI_ERROR;
         break;

      case MessageBoxQuestion:
         iconName = IDI_QUESTION;
         break;

      case VistaShield:
         if (QSysInfo::WindowsVersion >= QSysInfo::WV_VISTA && (QSysInfo::WindowsVersion & QSysInfo::WV_NT_based)) {
            if (!QWindowsContext::shell32dll.sHGetStockIconInfo) {
               return QPixmap();
            }

            QPixmap pixmap;
            SHSTOCKICONINFO iconInfo;
            memset(&iconInfo, 0, sizeof(iconInfo));
            iconInfo.cbSize = sizeof(iconInfo);
            const int iconSize = pixmapSize.width() > 16 ? SHGFI_LARGEICON : SHGFI_SMALLICON;

            if (QWindowsContext::shell32dll.sHGetStockIconInfo(SIID_SHIELD, SHGFI_ICON | iconSize, &iconInfo) == S_OK) {
               pixmap = qt_pixmapFromWinHICON(iconInfo.hIcon);
               DestroyIcon(iconInfo.hIcon);
               return pixmap;
            }
         }
         break;

      default:
         break;
   }

   if (resourceId != -1) {
      QPixmap pixmap = loadIconFromShell32(resourceId, pixmapSize);
      if (!pixmap.isNull()) {
         if (sp == FileLinkIcon || sp == DirLinkIcon || sp == DirLinkOpenIcon) {
            QPainter painter(&pixmap);
            QPixmap link = loadIconFromShell32(30, pixmapSize);
            painter.drawPixmap(0, 0, int(pixmapSize.width()), int(pixmapSize.height()), link);
         }
         return pixmap;
      }
   }

   if (iconName) {
      HICON iconHandle = LoadIcon(nullptr, iconName);
      QPixmap pixmap = qt_pixmapFromWinHICON(iconHandle);
      DestroyIcon(iconHandle);
      if (!pixmap.isNull()) {
         return pixmap;
      }
   }

   return QPlatformTheme::standardPixmap(sp, pixmapSize);
}

static QString dirIconPixmapCacheKey(int iIcon, int iconSize, int imageListSize)
{
   QString key = "qt_dir_" + QString::number(iIcon);

   if (iconSize == SHGFI_LARGEICON) {
      key.append('l');
   }

   switch (imageListSize) {
      case sHIL_EXTRALARGE:
         key.append('e');
         break;

      case sHIL_JUMBO:
         key.append('j');
         break;
   }

   return key;
}

template <typename T>
class FakePointer
{
 public:
   static_assert(sizeof(T) <= sizeof(void *), "Problem with pointer size");

   static FakePointer *create(T thing) {
      return reinterpret_cast<FakePointer *>(qintptr(thing));
   }

   T operator * () const {
      return T(qintptr(this));
   }

   void operator delete (void *) {}
};

// Shell image list helper functions
static QPixmap pixmapFromShellImageList(int iImageList, const SHFILEINFO &info)
{
   QPixmap result;

#ifdef USE_IIMAGELIST
   // For MinGW
   static const IID iID_IImageList = {0x46eb5926, 0x582e, 0x4017, {0x9f, 0xdf, 0xe8, 0x99, 0x8d, 0xaa, 0x9, 0x50}};

   if (!QWindowsContext::shell32dll.sHGetImageList) {
      return result;
   }
   if (iImageList == sHIL_JUMBO && QSysInfo::WindowsVersion < QSysInfo::WV_VISTA) {
      return result;
   }

   IImageList *imageList = 0;
   HRESULT hr = QWindowsContext::shell32dll.sHGetImageList(iImageList, iID_IImageList,
         reinterpret_cast<void **>(&imageList));

   if (hr != S_OK) {
      return result;
   }

   HICON hIcon;
   hr = imageList->GetIcon(info.iIcon, ILD_TRANSPARENT, &hIcon);

   if (hr == S_OK) {
      result = qt_pixmapFromWinHICON(hIcon);
      DestroyIcon(hIcon);
   }

   imageList->Release();

#else
   (void) iImageList;
   (void) info;
#endif

   return result;
}

QPixmap QWindowsTheme::fileIconPixmap(const QFileInfo &fileInfo, const QSizeF &size,
   QPlatformTheme::IconOptions iconOptions) const
{
   // We do not use the variable, but by storing it statically, we
   // ensure CoInitialize is only called once.

   static HRESULT comInit = CoInitialize(nullptr);
   (void) comInit;

   static QCache<QString, FakePointer<int>> dirIconEntryCache(1000);
   static QMutex mx;
   static int defaultFolderIIcon = -1;
   const bool useDefaultFolderIcon = iconOptions & QPlatformTheme::DontUseCustomDirectoryIcons;

   QPixmap pixmap;
   const QString filePath = QDir::toNativeSeparators(fileInfo.filePath());
   const int width = int(size.width());
   const int iconSize = width > fileIconSizes[SmallFileIcon] ? SHGFI_LARGEICON : SHGFI_SMALLICON;

   const int requestedImageListSize =
#ifdef USE_IIMAGELIST
      width > fileIconSizes[ExtraLargeFileIcon] ? sHIL_JUMBO : (width > fileIconSizes[LargeFileIcon] ? sHIL_EXTRALARGE : 0);
#else
      0;
#endif

   bool cacheableDirIcon = fileInfo.isDir() && !fileInfo.isRoot();

   if (cacheableDirIcon) {
      QMutexLocker locker(&mx);
      int iIcon = (useDefaultFolderIcon && defaultFolderIIcon >= 0)
            ? defaultFolderIIcon : **dirIconEntryCache.object(filePath);

      if (iIcon) {
         QPixmapCache::find(dirIconPixmapCacheKey(iIcon, iconSize, requestedImageListSize), pixmap);
         if (pixmap.isNull()) {
            // keep both caches in sync
            dirIconEntryCache.remove(filePath);
         } else {
            return pixmap;
         }
      }
   }

   SHFILEINFO info;
   const unsigned int flags = SHGFI_ICON | iconSize | SHGFI_SYSICONINDEX | SHGFI_ADDOVERLAYS | SHGFI_OVERLAYINDEX;

   const bool val = cacheableDirIcon && useDefaultFolderIcon
      ? shGetFileInfoBackground(m_threadPoolRunner, L"dummy", FILE_ATTRIBUTE_DIRECTORY, &info, flags | SHGFI_USEFILEATTRIBUTES)
      : shGetFileInfoBackground(m_threadPoolRunner, filePath.toStdWString().data(), 0, &info, flags);

   // Even if GetFileInfo returns a valid result, hIcon can be empty in some cases
   if (val && info.hIcon) {
      QString key;

      if (cacheableDirIcon) {
         if (useDefaultFolderIcon && defaultFolderIIcon < 0) {
            defaultFolderIIcon = info.iIcon;
         }

         //using the unique icon index provided by windows save us from duplicate keys
         key = dirIconPixmapCacheKey(info.iIcon, iconSize, requestedImageListSize);
         QPixmapCache::find(key, pixmap);
         if (!pixmap.isNull()) {
            QMutexLocker locker(&mx);
            dirIconEntryCache.insert(filePath, FakePointer<int>::create(info.iIcon));
         }
      }

      if (pixmap.isNull()) {
         if (requestedImageListSize) {
            pixmap = pixmapFromShellImageList(requestedImageListSize, info);
            if (pixmap.isNull() && requestedImageListSize == sHIL_JUMBO) {
               pixmap = pixmapFromShellImageList(sHIL_EXTRALARGE, info);
            }
         }
         if (pixmap.isNull()) {
            pixmap = qt_pixmapFromWinHICON(info.hIcon);
         }
         if (!pixmap.isNull()) {
            if (cacheableDirIcon) {
               QMutexLocker locker(&mx);
               QPixmapCache::insert(key, pixmap);
               dirIconEntryCache.insert(filePath, FakePointer<int>::create(info.iIcon));
            }
         } else {
            qWarning("QWindowsTheme::fileIconPixmap() no icon found");
         }
      }
      DestroyIcon(info.hIcon);
   }

   if (!pixmap.isNull()) {
      return pixmap;
   }
   return QPlatformTheme::fileIconPixmap(fileInfo, size);
}
