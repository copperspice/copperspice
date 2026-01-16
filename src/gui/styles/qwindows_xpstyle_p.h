/***********************************************************************
*
* Copyright (c) 2012-2026 Barbara Geller
* Copyright (c) 2012-2026 Ansel Sermersheim
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

#ifndef QWINDOWSXPSTYLE_P_H
#define QWINDOWSXPSTYLE_P_H

#include <qwindows_style_p.h>

#include <qmap.h>
#include <qt_windows.h>

#include <limits.h>
#include <uxtheme.h>
#include <vssym32.h>

#if ! defined(QT_NO_STYLE_WINDOWSXP)

class QWindowsXPStylePrivate;

class QWindowsXPStyle : public QWindowsStyle
{
   GUI_CS_OBJECT(QWindowsXPStyle)

 public:
   QWindowsXPStyle();
   QWindowsXPStyle(QWindowsXPStylePrivate &dd);

   QWindowsXPStyle(const QWindowsXPStyle &) = delete;
   QWindowsXPStyle &operator=(const QWindowsXPStyle &) = delete;

   ~QWindowsXPStyle();

   void drawPrimitive(PrimitiveElement pe, const QStyleOption *option, QPainter *painter,
      const QWidget *widget = nullptr) const override;

   void drawControl(ControlElement element, const QStyleOption *option,
      QPainter *painter, const QWidget *widget = nullptr) const override;

   QRect subElementRect(SubElement subElement, const QStyleOption *option, const QWidget *widget = nullptr) const override;

   QRect subControlRect(ComplexControl cc, const QStyleOptionComplex *option, SubControl subControl,
      const QWidget *widget = nullptr) const override;

   void drawComplexControl(ComplexControl cc, const QStyleOptionComplex *option, QPainter *painter,
      const QWidget *widget = nullptr) const override;

   QSize sizeFromContents(ContentsType ct, const QStyleOption *option, const QSize &contentsSize,
      const QWidget *widget = nullptr) const override;

   int pixelMetric(PixelMetric pm, const QStyleOption *option = nullptr, const QWidget *widget = nullptr) const override;

   int styleHint(StyleHint hint, const QStyleOption *option = nullptr, const QWidget *widget = nullptr,
      QStyleHintReturn *styleHintReturn = nullptr) const override;

   QPalette standardPalette() const override;

   QPixmap standardPixmap(StandardPixmap standardPixmap, const QStyleOption *option,
      const QWidget *widget = nullptr) const override;

   QIcon standardIcon(StandardPixmap standardPixmap, const QStyleOption *option = nullptr,
      const QWidget *widget = nullptr) const override;

   void polish(QApplication *app) override;
   void polish(QWidget *widget) override;
   void polish(QPalette &palette) override;
   void unpolish(QApplication *app) override;
   void unpolish(QWidget *widget) override;

 private:
   Q_DECLARE_PRIVATE(QWindowsXPStyle)

   friend class QStyleFactory;
};

#endif // QT_NO_STYLE_WINDOWSXP


// TMT_TEXTSHADOWCOLOR is wrongly defined in mingw
#if TMT_TEXTSHADOWCOLOR != 3818
#undef TMT_TEXTSHADOWCOLOR
#define TMT_TEXTSHADOWCOLOR 3818
#endif

#ifndef TST_NONE
#  define TST_NONE 0
#endif

// These defines are missing from the tmschema, but still exist as states for their parts
#ifndef MINBS_INACTIVE
#define MINBS_INACTIVE 5
#endif

#ifndef MAXBS_INACTIVE
#define MAXBS_INACTIVE 5
#endif

#ifndef RBS_INACTIVE
#define RBS_INACTIVE 5
#endif

#ifndef HBS_INACTIVE
#define HBS_INACTIVE 5
#endif

#ifndef CBS_INACTIVE
#define CBS_INACTIVE 5
#endif

#if ! defined(QT_NO_STYLE_WINDOWSXP)

class XPThemeData
{
 public:
   explicit XPThemeData(const QWidget *w = nullptr, QPainter *p = nullptr, int themeIn = -1,
      int part = 0, int state = 0, const QRect &r = QRect())
      : m_xpWidget(w), painter(p), theme(themeIn), partId(part), stateId(state),
        mirrorHorizontally(false), mirrorVertically(false), noBorder(false),
        noContent(false), rotate(0), htheme(nullptr), m_xpRect(r)
   { }

   HRGN mask(QWidget *widget);
   HTHEME handle();

   static RECT toRECT(const QRect &qr);
   bool isValid();

   QSizeF size();
   QMarginsF margins(const QRect &rect, int propId = TMT_CONTENTMARGINS);
   QMarginsF margins(int propId = TMT_CONTENTMARGINS);

   static QSizeF themeSize(const QWidget *widget = nullptr, QPainter *painter = nullptr, int themeIn = -1,
         int part = 0, int state = 0);

   static QMarginsF themeMargins(const QRect &rect, const QWidget *widget = nullptr, QPainter *painter = nullptr,
         int themeIn = -1, int part = 0, int state = 0, int propId = TMT_CONTENTMARGINS);

   static QMarginsF themeMargins(const QWidget *widget = nullptr, QPainter *painter = nullptr, int themeIn = -1,
         int part = 0, int state = 0, int propId = TMT_CONTENTMARGINS);

   const QWidget *m_xpWidget;
   QPainter *painter;

   int theme;
   int partId;
   int stateId;

   uint mirrorHorizontally : 1;
   uint mirrorVertically : 1;
   uint noBorder : 1;
   uint noContent : 1;
   uint rotate;

   HTHEME htheme;

   QRect m_xpRect;
};

struct ThemeMapKey {
   int theme;
   int partId;
   int stateId;
   bool noBorder;
   bool noContent;

   ThemeMapKey()
      : partId(-1), stateId(-1)
   { }

   ThemeMapKey(const XPThemeData &data)
      : theme(data.theme), partId(data.partId), stateId(data.stateId),
        noBorder(data.noBorder), noContent(data.noContent)
   { }
};

inline uint qHash(const ThemeMapKey &key)
{
   return key.theme ^ key.partId ^ key.stateId;
}

inline bool operator==(const ThemeMapKey &k1, const ThemeMapKey &k2)
{
   return k1.theme == k2.theme && k1.partId == k2.partId && k1.stateId == k2.stateId;
}

enum AlphaChannelType {
   UnknownAlpha = -1,          // Alpha of part & state not yet known
   NoAlpha,                    // Totally opaque, no need to touch alpha (RGB)
   MaskAlpha,                  // Alpha channel must be fixed            (ARGB)
   RealAlpha                   // Proper alpha values from Windows       (ARGB_Premultiplied)
};

struct ThemeMapData {
   AlphaChannelType alphaType; // Which type of alpha on part & state

   bool dataValid         : 1; // Only used to detect if hash value is ok
   bool partIsTransparent : 1;
   bool hasAlphaChannel   : 1; // True =  part & state has real Alpha
   bool wasAlphaSwapped   : 1; // True =  alpha channel needs to be swapped
   bool hadInvalidAlpha   : 1; // True =  alpha channel contained invalid alpha values

   ThemeMapData() : dataValid(false), partIsTransparent(false),
      hasAlphaChannel(false), wasAlphaSwapped(false), hadInvalidAlpha(false) {}
};

class QWindowsXPStylePrivate : public QWindowsStylePrivate
{
   Q_DECLARE_PUBLIC(QWindowsXPStyle)

 public:
   enum Theme {
      ButtonTheme,
      ComboboxTheme,
      EditTheme,
      HeaderTheme,
      ListViewTheme,
      MenuTheme,
      ProgressTheme,
      RebarTheme,
      ScrollBarTheme,
      SpinTheme,
      TabTheme,
      TaskDialogTheme,
      ToolBarTheme,
      ToolTipTheme,
      TrackBarTheme,
      XpTreeViewTheme,      // '+'/'-' shape treeview indicators (XP)
      WindowTheme,
      StatusTheme,
      VistaTreeViewTheme,   // arrow shape treeview indicators (Vista) obtained from "explorer" theme.
      NThemes
   };

   QWindowsXPStylePrivate()
      : QWindowsStylePrivate(), hasInitColors(false), bufferDC(nullptr), bufferBitmap(nullptr),
        nullBitmap(nullptr), bufferPixels(nullptr), bufferW(0), bufferH(0)
   {
      init();
   }

   ~QWindowsXPStylePrivate() {
      cleanup();
   }

   static int pixelMetricFromSystemDp(QStyle::PixelMetric pm, const QStyleOption *option = nullptr,
               const QWidget *widget = nullptr);
   static int fixedPixelMetric(QStyle::PixelMetric pm, const QStyleOption *option = nullptr,
               const QWidget *widget = nullptr);

   static HWND winId(const QWidget *widget);

   void init(bool force = false);
   void cleanup(bool force = false);
   void cleanupHandleMap();
   const QPixmap *tabBody(QWidget *widget);

   HBITMAP buffer(int w = 0, int h = 0);
   HDC bufferHDC() {
      return bufferDC;
   }

   static bool useXP(bool update = false);
   static QRect scrollBarGripperBounds(QStyle::State flags, const QWidget *widget, XPThemeData *theme);

   bool isTransparent(XPThemeData &themeData);
   QRegion region(XPThemeData &themeData);

   void setTransparency(QWidget *widget, XPThemeData &themeData);
   bool drawBackground(XPThemeData &themeData);
   bool drawBackgroundThruNativeBuffer(XPThemeData &themeData, qreal aditionalDevicePixelRatio);
   bool drawBackgroundDirectly(HDC dc, XPThemeData &themeData, qreal aditionalDevicePixelRatio);

   bool hasAlphaChannel(const QRect &rect);
   bool fixAlphaChannel(const QRect &rect);
   bool swapAlphaChannel(const QRect &rect, bool allPixels = false);

   QRgb groupBoxTextColor;
   QRgb groupBoxTextColorDisabled;
   QRgb sliderTickColor;
   bool hasInitColors;

   static HTHEME createTheme(int theme, HWND hwnd);
   static QString themeName(int theme);

   static bool hasTheme(int theme) {
      return theme >= 0 && theme < NThemes && m_themes[theme];
   }

   static bool isItemViewDelegateLineEdit(const QWidget *widget);
   static bool isLineEditBaseColorSet(const QStyleOption *option, const QWidget *widget);

   QIcon dockFloat;
   QIcon dockClose;

 private:
   static bool initVistaTreeViewTheming();
   static void cleanupVistaTreeViewTheming();
   static QAtomicInt ref;
   static bool use_xp;
   static QPixmap *tabbody;

   QHash<ThemeMapKey, ThemeMapData> alphaCache;
   HDC bufferDC;
   HBITMAP bufferBitmap;
   HBITMAP nullBitmap;
   uchar *bufferPixels;
   int bufferW, bufferH;
   static HWND m_vistaTreeViewHelper;
   static HTHEME m_themes[NThemes];
};

inline QSizeF XPThemeData::size()
{
   QSizeF result(0, 0);

   if (isValid()) {
      SIZE size;

      if (SUCCEEDED(GetThemePartSize(handle(), nullptr, partId, stateId, nullptr, TS_TRUE, &size))) {
         result = QSize(size.cx, size.cy);
      }
   }

   return result;
}

inline QMarginsF XPThemeData::margins(const QRect &qRect, int propId)
{
   QMarginsF result(0, 0, 0, 0);

   if (isValid()) {
      MARGINS margins;
      RECT rect = XPThemeData::toRECT(qRect);

      if (SUCCEEDED(GetThemeMargins(handle(), nullptr, partId, stateId, propId, &rect, &margins))) {
         result = QMargins(margins.cxLeftWidth, margins.cyTopHeight, margins.cxRightWidth, margins.cyBottomHeight);
      }
   }

   return result;
}

inline QMarginsF XPThemeData::margins(int propId)
{
   QMarginsF result(0, 0, 0, 0);

   if (isValid()) {
      MARGINS margins;

      if (SUCCEEDED(GetThemeMargins(handle(), nullptr, partId, stateId, propId, nullptr, &margins))) {
         result = QMargins(margins.cxLeftWidth, margins.cyTopHeight, margins.cxRightWidth, margins.cyBottomHeight);
      }
   }

   return result;
}

inline QSizeF XPThemeData::themeSize(const QWidget *w, QPainter *p, int themeIn, int part, int state)
{
   XPThemeData theme(w, p, themeIn, part, state);
   return theme.size();
}

inline QMarginsF XPThemeData::themeMargins(const QRect &rect, const QWidget *w, QPainter *p, int themeIn,
   int part, int state, int propId)
{
   XPThemeData theme(w, p, themeIn, part, state);
   return theme.margins(rect, propId);
}

inline QMarginsF XPThemeData::themeMargins(const QWidget *w, QPainter *p, int themeIn,
   int part, int state, int propId)
{
   XPThemeData theme(w, p, themeIn, part, state);
   return theme.margins(propId);
}
#endif // QT_NO_STYLE_WINDOWS

#endif // QWINDOWSXPSTYLE_P_H
