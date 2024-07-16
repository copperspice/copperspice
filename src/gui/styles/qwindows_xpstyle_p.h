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

#ifndef QWINDOWSXPSTYLE_P_H
#define QWINDOWSXPSTYLE_P_H

#include <qwindows_style_p.h>

#include <qmap.h>
#include <qt_windows.h>

#include <uxtheme.h>

#if WINVER >= 0x0600
#include <vssym32.h>
#else
#include <tmschema.h>
#endif

/*
#define _WIN32_WINNT 0x0501       // Windows XP
#include <commctrl.h>
*/

#include <limits.h>

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


// Older Platform SDKs do not have the extended DrawThemeBackgroundEx
// function. We add the needed parts here, and use the extended
// function dynamically, if available in uxtheme.dll. Else, we revert
// back to using the DrawThemeBackground function.

#ifndef DTBG_OMITBORDER
#  ifndef DTBG_CLIPRECT
#   define DTBG_CLIPRECT        0x00000001
#  endif

#  ifndef DTBG_DRAWSOLID
#   define DTBG_DRAWSOLID       0x00000002
#  endif

#  ifndef DTBG_OMITBORDER
#   define DTBG_OMITBORDER      0x00000004
#  endif

#  ifndef DTBG_OMITCONTENT
#   define DTBG_OMITCONTENT     0x00000008
#  endif

#  ifndef DTBG_COMPUTINGREGION
#   define DTBG_COMPUTINGREGION 0x00000010
#  endif

#  ifndef DTBG_MIRRORDC
#   define DTBG_MIRRORDC        0x00000020
#  endif

typedef struct _DTBGOPTS {
   DWORD dwSize;
   DWORD dwFlags;
   RECT rcClip;
} DTBGOPTS, *PDTBGOPTS;
#endif

// Undefined for some compile environments
#ifndef TMT_TEXTCOLOR
#  define TMT_TEXTCOLOR 3803
#endif

#ifndef TMT_BORDERCOLORHINT
#  define TMT_BORDERCOLORHINT 3822
#endif

#ifndef TMT_BORDERSIZE
#  define TMT_BORDERSIZE 2403
#endif

#ifndef TMT_BORDERONLY
#  define TMT_BORDERONLY 2203
#endif

#ifndef TMT_TRANSPARENTCOLOR
#  define TMT_TRANSPARENTCOLOR 3809
#endif

#ifndef TMT_CAPTIONMARGINS
#  define TMT_CAPTIONMARGINS 3603
#endif

#ifndef TMT_CONTENTMARGINS
#  define TMT_CONTENTMARGINS 3602
#endif

#ifndef TMT_SIZINGMARGINS
#  define TMT_SIZINGMARGINS 3601
#endif

#ifndef TMT_GLYPHTYPE
#  define TMT_GLYPHTYPE 4012
#endif

#ifndef TMT_BGTYPE
#  define TMT_BGTYPE 4001
#endif

#ifndef TMT_TEXTSHADOWTYPE
#    define TMT_TEXTSHADOWTYPE 4010
#endif

#ifndef TMT_BORDERCOLOR
#    define TMT_BORDERCOLOR 3801
#endif

#ifndef BT_IMAGEFILE
#  define BT_IMAGEFILE 0
#endif

#ifndef BT_BORDERFILL
#  define BT_BORDERFILL 1
#endif

#ifndef BT_NONE
#  define BT_NONE 2
#endif

#ifndef TMT_FILLCOLOR
#  define TMT_FILLCOLOR 3802
#endif

#ifndef TMT_PROGRESSCHUNKSIZE
#  define TMT_PROGRESSCHUNKSIZE 2411
#endif

// TMT_TEXTSHADOWCOLOR is wrongly defined in mingw
#if TMT_TEXTSHADOWCOLOR != 3818
#undef TMT_TEXTSHADOWCOLOR
#define TMT_TEXTSHADOWCOLOR 3818
#endif

#ifndef TST_NONE
#  define TST_NONE 0
#endif

#ifndef GT_NONE
#  define GT_NONE 0
#endif

#ifndef GT_IMAGEGLYPH
#  define GT_IMAGEGLYPH 1
#endif

// These defines are missing from the tmschema, but still exist as
// states for their parts
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
      : widget(w), painter(p), theme(themeIn), htheme(nullptr), partId(part), stateId(state),
        mirrorHorizontally(false), mirrorVertically(false), noBorder(false),
        noContent(false), rotate(0), rect(r)
   {
   }

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

   const QWidget *widget;
   QPainter *painter;

   int theme;
   HTHEME htheme;
   int partId;
   int stateId;

   uint mirrorHorizontally : 1;
   uint mirrorVertically : 1;
   uint noBorder : 1;
   uint noContent : 1;
   uint rotate;
   QRect rect;
};

struct ThemeMapKey {
   int theme;
   int partId;
   int stateId;
   bool noBorder;
   bool noContent;

   ThemeMapKey() : partId(-1), stateId(-1) {}
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
   return k1.theme == k2.theme
      && k1.partId == k2.partId
      && k1.stateId == k2.stateId;
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

struct QWindowsUxThemeLib {
   typedef bool (WINAPI *PtrIsAppThemed)();
   typedef bool (WINAPI *PtrIsThemeActive)();
   typedef HTHEME (WINAPI *PtrOpenThemeData)(HWND hwnd, LPCWSTR pszClassList);
   typedef HRESULT (WINAPI *PtrCloseThemeData)(HTHEME hTheme);

   typedef HRESULT (WINAPI *PtrDrawThemeBackground)(HTHEME hTheme, HDC hdc, int iPartId, int iStateId,
      const RECT *pRect, OPTIONAL const RECT *pClipRect);

   typedef HRESULT (WINAPI *PtrDrawThemeBackgroundEx)(HTHEME hTheme, HDC hdc, int iPartId, int iStateId,
      const RECT *pRect, OPTIONAL const DTBGOPTS *pOptions);

   typedef HRESULT (WINAPI *PtrGetCurrentThemeName)(OUT LPWSTR pszThemeFileName, int cchMaxNameChars,
      OUT OPTIONAL LPWSTR pszColorBuff, int cchMaxColorChars, OUT OPTIONAL LPWSTR pszSizeBuff, int cchMaxSizeChars);

   typedef HRESULT (WINAPI *PtrGetThemeDocumentationProperty)(LPCWSTR pszThemeName, LPCWSTR pszPropertyName,
      OUT LPWSTR pszValueBuff, int cchMaxValChars);

   typedef HRESULT (WINAPI *PtrGetThemeBool)(HTHEME hTheme, int iPartId, int iStateId, int iPropId, OUT BOOL *pfVal);
   typedef HRESULT (WINAPI *PtrGetThemeColor)(HTHEME hTheme, int iPartId, int iStateId, int iPropId, OUT COLORREF *pColor);
   typedef HRESULT (WINAPI *PtrGetThemeEnumValue)(HTHEME hTheme, int iPartId, int iStateId, int iPropId, OUT int *piVal);
   typedef HRESULT (WINAPI *PtrGetThemeFilename)(HTHEME hTheme, int iPartId, int iStateId, int iPropId,
      OUT LPWSTR pszThemeFileName, int cchMaxBuffChars);

   typedef HRESULT (WINAPI *PtrGetThemeFont)(HTHEME hTheme, OPTIONAL HDC hdc, int iPartId, int iStateId,
      int iPropId, OUT LOGFONT *pFont);

   typedef HRESULT (WINAPI *PtrGetThemeInt)(HTHEME hTheme, int iPartId, int iStateId, int iPropId, OUT int *piVal);
   typedef HRESULT (WINAPI *PtrGetThemeIntList)(HTHEME hTheme, int iPartId, int iStateId, int iPropId, OUT INTLIST *pIntList);
   typedef HRESULT (WINAPI *PtrGetThemeMargins)(HTHEME hTheme, OPTIONAL HDC hdc, int iPartId, int iStateId, int iPropId,
      OPTIONAL RECT *prc, OUT MARGINS *pMargins);

   typedef HRESULT (WINAPI *PtrGetThemeMetric)(HTHEME hTheme, OPTIONAL HDC hdc, int iPartId, int iStateId,
      int iPropId, OUT int *piVal);

   typedef HRESULT (WINAPI *PtrGetThemePartSize)(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, OPTIONAL RECT *prc,
      enum THEMESIZE eSize, OUT SIZE *psz);

   typedef HRESULT (WINAPI *PtrGetThemePosition)(HTHEME hTheme, int iPartId, int iStateId, int iPropId, OUT POINT *pPoint);
   typedef HRESULT (WINAPI *PtrGetThemePropertyOrigin)(HTHEME hTheme, int iPartId, int iStateId, int iPropId,
      OUT enum PROPERTYORIGIN *pOrigin);

   typedef HRESULT (WINAPI *PtrGetThemeRect)(HTHEME hTheme, int iPartId, int iStateId, int iPropId, OUT RECT *pRect);
   typedef HRESULT (WINAPI *PtrGetThemeString)(HTHEME hTheme, int iPartId, int iStateId, int iPropId, OUT LPWSTR pszBuff,
      int cchMaxBuffChars);

   typedef HRESULT (WINAPI *PtrGetThemeBackgroundRegion)(HTHEME hTheme, OPTIONAL HDC hdc, int iPartId,
      int iStateId, const RECT *pRect, OUT HRGN *pRegion);

   typedef BOOL (WINAPI *PtrIsThemeBackgroundPartiallyTransparent)(HTHEME hTheme, int iPartId, int iStateId);
   typedef HRESULT (WINAPI *PtrSetWindowTheme)(HWND hwnd, LPCWSTR pszSubAppName, LPCWSTR pszSubIdList);

   typedef HRESULT (WINAPI *PtrGetThemeTransitionDuration)(HTHEME hTheme, int iPartId, int iStateFromId,
      int iStateToId, int iPropId, int *pDuration);

   static bool resolveSymbols();

   static PtrIsAppThemed pIsAppThemed;
   static PtrIsThemeActive pIsThemeActive;
   static PtrOpenThemeData pOpenThemeData;
   static PtrCloseThemeData pCloseThemeData;
   static PtrDrawThemeBackground pDrawThemeBackground;
   static PtrDrawThemeBackgroundEx pDrawThemeBackgroundEx;
   static PtrGetCurrentThemeName pGetCurrentThemeName;
   static PtrGetThemeBool pGetThemeBool;
   static PtrGetThemeColor pGetThemeColor;
   static PtrGetThemeEnumValue pGetThemeEnumValue;
   static PtrGetThemeFilename pGetThemeFilename;
   static PtrGetThemeFont pGetThemeFont;
   static PtrGetThemeInt pGetThemeInt;
   static PtrGetThemeIntList pGetThemeIntList;
   static PtrGetThemeMargins pGetThemeMargins;
   static PtrGetThemeMetric pGetThemeMetric;
   static PtrGetThemePartSize pGetThemePartSize;
   static PtrGetThemePosition pGetThemePosition;
   static PtrGetThemePropertyOrigin pGetThemePropertyOrigin;
   static PtrGetThemeRect pGetThemeRect;
   static PtrGetThemeString pGetThemeString;
   static PtrGetThemeBackgroundRegion pGetThemeBackgroundRegion;
   static PtrGetThemeDocumentationProperty pGetThemeDocumentationProperty;
   static PtrIsThemeBackgroundPartiallyTransparent pIsThemeBackgroundPartiallyTransparent;
   static PtrSetWindowTheme pSetWindowTheme;
   static PtrGetThemeTransitionDuration pGetThemeTransitionDuration; // Windows Vista onwards.
};

class QWindowsXPStylePrivate : public QWindowsStylePrivate, public QWindowsUxThemeLib
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

   static bool resolveSymbols();
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
   static inline bool hasTheme(int theme) {
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
      if (SUCCEEDED(QWindowsXPStylePrivate::pGetThemePartSize(handle(), nullptr, partId, stateId, nullptr, TS_TRUE, &size))) {
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
      if (SUCCEEDED(QWindowsXPStylePrivate::pGetThemeMargins(handle(), nullptr, partId, stateId, propId, &rect, &margins))) {
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
      if (SUCCEEDED(QWindowsXPStylePrivate::pGetThemeMargins(handle(), nullptr, partId, stateId, propId, nullptr, &margins))) {
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

#endif //QWINDOWSXPSTYLE_P_H
