/***********************************************************************
*
* Copyright (c) 2012-2020 Barbara Geller
* Copyright (c) 2012-2020 Ansel Sermersheim
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

#ifndef QT_NO_WIZARD
#ifndef QT_NO_STYLE_WINDOWSVISTA

#include <qwizard.h>
#include <qwizard_win_p.h>

#include <qapplication.h>
#include <qdebug.h>
#include <QDesktopWidget>
#include <qplatform_nativeinterface.h>
#include <qmouseevent.h>
#include <qpaintengine.h>
#include <qvariant.h>
#include <qwindow.h>

#include <qapplication_p.h>
#include <qsystemlibrary_p.h>

// test is duplicated in qwizard_win.cpp and qwindowsxpstyle_p.h
#ifdef Q_CC_GNU
#ifdef _WIN32_WINNT
# undef _WIN32_WINNT
#endif

#define _WIN32_WINNT 0x0501       // Windows XP
#include <commctrl.h>
#endif

#include <uxtheme.h>

Q_GUI_EXPORT HICON qt_pixmapToWinHICON(const QPixmap &);

//DWM related
typedef struct  {      // MARGINS
   int cxLeftWidth;    // width of left border that retains its size
   int cxRightWidth;   // width of right border that retains its size
   int cyTopHeight;    // height of top border that retains its size
   int cyBottomHeight; // height of bottom border that retains its size
} WIZ_MARGINS;

typedef struct {       // DTTOPTS
   DWORD dwSize;
   DWORD dwFlags;
   COLORREF crText;
   COLORREF crBorder;
   COLORREF crShadow;
   int eTextShadowType;
   POINT ptShadowOffset;
   int iBorderSize;
   int iFontPropId;
   int iColorPropId;
   int iStateId;
   BOOL fApplyOverlay;
   int iGlowSize;
} WIZ_DTTOPTS;

typedef struct {
   DWORD dwFlags;
   DWORD dwMask;
} WIZ_WTA_OPTIONS;

#define WIZ_WM_THEMECHANGED              0x031A
#define WIZ_WM_DWMCOMPOSITIONCHANGED     0x031E

enum WIZ_WINDOWTHEMEATTRIBUTETYPE {
   WIZ_WTA_NONCLIENT = 1
};

#define WIZ_WTNCA_NODRAWCAPTION          0x00000001
#define WIZ_WTNCA_NODRAWICON             0x00000002

#define WIZ_DT_CENTER                    0x00000001
#define WIZ_DT_VCENTER                   0x00000004
#define WIZ_DT_SINGLELINE                0x00000020
#define WIZ_DT_NOPREFIX                  0x00000800

enum WIZ_NAVIGATIONPARTS {               // NAVIGATIONPARTS
   WIZ_NAV_BACKBUTTON = 1,
   WIZ_NAV_FORWARDBUTTON = 2,
   WIZ_NAV_MENUBUTTON = 3,
};

enum WIZ_NAV_BACKBUTTONSTATES {          //NAV_BACKBUTTONSTATES
   WIZ_NAV_BB_NORMAL = 1,
   WIZ_NAV_BB_HOT = 2,
   WIZ_NAV_BB_PRESSED = 3,
   WIZ_NAV_BB_DISABLED = 4,
};

#define WIZ_TMT_CAPTIONFONT (801)           //TMT_CAPTIONFONT
#define WIZ_DTT_COMPOSITED  (1UL << 13)     //DTT_COMPOSITED
#define WIZ_DTT_GLOWSIZE    (1UL << 11)     //DTT_GLOWSIZE

#define WIZ_WM_NCMOUSELEAVE 674             //WM_NCMOUSELEAVE

#define WIZ_WP_CAPTION             1        //WP_CAPTION
#define WIZ_CS_ACTIVE              1        //CS_ACTIVE
#define WIZ_TMT_FILLCOLORHINT   3821        //TMT_FILLCOLORHINT
#define WIZ_TMT_BORDERCOLORHINT 3822        //TMT_BORDERCOLORHINT

typedef BOOL (WINAPI *PtrDwmDefWindowProc)(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam, LRESULT *plResult);
typedef HRESULT (WINAPI *PtrDwmIsCompositionEnabled)(BOOL *pfEnabled);
typedef HRESULT (WINAPI *PtrDwmExtendFrameIntoClientArea)(HWND hWnd, const WIZ_MARGINS *pMarInset);
typedef HRESULT (WINAPI *PtrSetWindowThemeAttribute)(HWND hwnd, enum WIZ_WINDOWTHEMEATTRIBUTETYPE eAttribute,
   PVOID pvAttribute, DWORD cbAttribute);

static PtrDwmDefWindowProc pDwmDefWindowProc = 0;
static PtrDwmIsCompositionEnabled pDwmIsCompositionEnabled = 0;
static PtrDwmExtendFrameIntoClientArea pDwmExtendFrameIntoClientArea = 0;
static PtrSetWindowThemeAttribute pSetWindowThemeAttribute = 0;

//Theme related
typedef bool (WINAPI *PtrIsAppThemed)();
typedef bool (WINAPI *PtrIsThemeActive)();
typedef HANDLE (WINAPI *PtrOpenThemeData)(HWND hwnd, LPCWSTR pszClassList);
typedef HRESULT (WINAPI *PtrCloseThemeData)(HANDLE hTheme);
typedef HRESULT (WINAPI *PtrGetThemeSysFont)(HANDLE hTheme, int iFontId, LOGFONTW *plf);

typedef HRESULT (WINAPI *PtrDrawThemeTextEx)(HANDLE hTheme, HDC hdc, int iPartId, int iStateId, LPCWSTR pszText,
   int cchText, DWORD dwTextFlags, LPRECT pRect, const WIZ_DTTOPTS *pOptions);

typedef HRESULT (WINAPI *PtrDrawThemeBackground)(HANDLE hTheme, HDC hdc, int iPartId, int iStateId, const RECT *pRect,
   OPTIONAL const RECT *pClipRect);

typedef HRESULT (WINAPI *PtrGetThemePartSize)(HANDLE hTheme, HDC hdc, int iPartId, int iStateId, OPTIONAL RECT *prc,
   enum THEMESIZE eSize, OUT SIZE *psz);

typedef HRESULT (WINAPI *PtrGetThemeColor)(HANDLE hTheme, int iPartId, int iStateId, int iPropId, OUT COLORREF *pColor);

static PtrIsAppThemed pIsAppThemed = 0;
static PtrIsThemeActive pIsThemeActive = 0;
static PtrOpenThemeData pOpenThemeData = 0;
static PtrCloseThemeData pCloseThemeData = 0;
static PtrGetThemeSysFont pGetThemeSysFont = 0;
static PtrDrawThemeTextEx pDrawThemeTextEx = 0;
static PtrDrawThemeBackground pDrawThemeBackground = 0;
static PtrGetThemePartSize pGetThemePartSize = 0;
static PtrGetThemeColor pGetThemeColor = 0;

int QVistaHelper::instanceCount = 0;
int QVistaHelper::m_devicePixelRatio = 1;
bool QVistaHelper::is_vista = false;
QVistaHelper::VistaState QVistaHelper::cachedVistaState = QVistaHelper::Dirty;

/******************************************************************************
** QVistaBackButton
*/

QVistaBackButton::QVistaBackButton(QWidget *widget)
   : QAbstractButton(widget)
{
   setFocusPolicy(Qt::NoFocus);

   // Native dialogs use ALT-Left even in RTL mode, even if it might be counter-intuitive
   setShortcut(QKeySequence(Qt::ALT | Qt::Key_Left));
}

QSize QVistaBackButton::sizeHint() const
{
   ensurePolished();
   int size = int(QStyleHelper::dpiScaled(32));
   int width = size, height = size;
   /*
       HANDLE theme = pOpenThemeData(0, L"Navigation");
       SIZE size;
       if (pGetThemePartSize(theme, 0, WIZ_NAV_BACKBUTTON, WIZ_NAV_BB_NORMAL, 0, TS_TRUE, &size) == S_OK) {
           width = size.cx;
           height = size.cy;
       }
   */
   return QSize(width, height);
}

void QVistaBackButton::enterEvent(QEvent *event)
{
   if (isEnabled()) {
      update();
   }
   QAbstractButton::enterEvent(event);
}

void QVistaBackButton::leaveEvent(QEvent *event)
{
   if (isEnabled()) {
      update();
   }
   QAbstractButton::leaveEvent(event);
}

void QVistaBackButton::paintEvent(QPaintEvent *)
{
   QPainter p(this);
   QRect r = rect();
   HANDLE theme = pOpenThemeData(0, L"Navigation");
   //RECT rect;

   QPoint origin;
   const HDC hdc = QVistaHelper::backingStoreDC(parentWidget(), &origin);

   RECT clipRect;
   int xoffset = origin.x() + QWidget::mapToParent(r.topLeft()).x() - 1;
   int yoffset = origin.y() + QWidget::mapToParent(r.topLeft()).y() - 1;
   const int dpr = devicePixelRatio();
   const QRect rDp = QRect(r.topLeft() * dpr, r.size() * dpr);
   const int xoffsetDp = xoffset * dpr;
   const int yoffsetDp = yoffset * dpr;

   clipRect.top = rDp.top() + yoffsetDp;
   clipRect.bottom = rDp.bottom() + yoffsetDp;
   clipRect.left = rDp.left() + xoffsetDp;
   clipRect.right = rDp.right()  + xoffsetDp;

   int state = WIZ_NAV_BB_NORMAL;
   if (!isEnabled()) {
      state = WIZ_NAV_BB_DISABLED;
   } else if (isDown()) {
      state = WIZ_NAV_BB_PRESSED;
   } else if (underMouse()) {
      state = WIZ_NAV_BB_HOT;
   }

   WIZ_NAVIGATIONPARTS buttonType = (layoutDirection() == Qt::LeftToRight
         ? WIZ_NAV_BACKBUTTON
         : WIZ_NAV_FORWARDBUTTON);

   pDrawThemeBackground(theme, hdc, buttonType, state, &clipRect, &clipRect);
}

/******************************************************************************
** QVistaHelper
*/

QVistaHelper::QVistaHelper(QWizard *wizard)
   : QObject(wizard)
   , pressed(false)
   , wizard(wizard)
   , backButton_(0)
{
   QVistaHelper::m_devicePixelRatio = wizard->devicePixelRatio();
   is_vista = QSysInfo::WindowsVersion >= QSysInfo::WV_VISTA && resolveSymbols();

   if (instanceCount++ == 0) {
      cachedVistaState = Dirty;
   }

   if (is_vista) {
      backButton_ = new QVistaBackButton(wizard);
      backButton_->hide();
   }

   // Handle diff between Windows 7 and Vista
   iconSpacing = QStyleHelper::dpiScaled(7);
   textSpacing = QSysInfo::WindowsVersion >= QSysInfo::WV_WINDOWS7 ?
      iconSpacing : QStyleHelper::dpiScaled(20);
}

QVistaHelper::~QVistaHelper()
{
   --instanceCount;
}
void QVistaHelper::updateCustomMargins(bool vistaMargins)
{
   if (QWindow *window = wizard->windowHandle()) {
      // Reduce top frame to zero since we paint it ourselves. Use
      // device pixel to avoid rounding errors.
      const QMargins customMarginsDp = vistaMargins
         ? QMargins(0, -titleBarSizeDp(), 0, 0) : QMargins();

      const QVariant customMarginsV = QVariant::fromValue(customMarginsDp);

      // The dynamic property takes effect when creating the platform window.
      window->setProperty("_q_windowsCustomMargins", customMarginsV);

      // If a platform window exists, change via native interface.
      if (QPlatformWindow *platformWindow = window->handle()) {
         QGuiApplication::platformNativeInterface()->setWindowProperty(platformWindow, QString("WindowsCustomMargins"), customMarginsV);
      }
   }
}

bool QVistaHelper::isCompositionEnabled()
{
   bool value = is_vista;
   if (is_vista) {
      HRESULT hr;
      BOOL bEnabled;

      hr = pDwmIsCompositionEnabled(&bEnabled);
      value = (SUCCEEDED(hr) && bEnabled);
   }
   return value;
}

bool QVistaHelper::isThemeActive()
{
   return is_vista && pIsThemeActive();
}

QVistaHelper::VistaState QVistaHelper::vistaState()
{
   if (instanceCount == 0 || cachedVistaState == Dirty) {
      cachedVistaState =  isCompositionEnabled() ? VistaAero : isThemeActive() ? VistaBasic : Classic;
   }

   return cachedVistaState;
}

void QVistaHelper::disconnectBackButton()
{
   if (backButton_) {
      // Leave QStyleSheetStyle's connections on destroyed() intact.
      backButton_->disconnect(SLOT(clicked()));
   }
}

QColor QVistaHelper::basicWindowFrameColor()
{
   DWORD rgb;
   HWND handle = QApplicationPrivate::getHWNDForWidget(QApplication::desktop());
   HANDLE hTheme = pOpenThemeData(handle, L"WINDOW");

   pGetThemeColor(
      hTheme, WIZ_WP_CAPTION, WIZ_CS_ACTIVE,
      wizard->isActiveWindow() ? WIZ_TMT_FILLCOLORHINT : WIZ_TMT_BORDERCOLORHINT,
      &rgb);

   BYTE r = GetRValue(rgb);
   BYTE g = GetGValue(rgb);
   BYTE b = GetBValue(rgb);
   return QColor(r, g, b);
}

bool QVistaHelper::setDWMTitleBar(TitleBarChangeType type)
{
   bool value = false;

   if (vistaState() == VistaAero) {
      WIZ_MARGINS mar = {0, 0, 0, 0};

      if (type == NormalTitleBar) {
         mar.cyTopHeight = 0;
      } else {
         mar.cyTopHeight = (titleBarSize() + topOffset()) * QVistaHelper::m_devicePixelRatio;
      }

      if (const HWND wizardHandle = wizardHWND())
         if (SUCCEEDED(pDwmExtendFrameIntoClientArea(wizardHandle, &mar))) {
            value = true;
         }
   }

   return value;
}

Q_GUI_EXPORT HICON qt_pixmapToWinHICON(const QPixmap &);

static LOGFONT getCaptionLogFont(HANDLE hTheme)
{
   LOGFONT result = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, { 0 } };
   if (!hTheme || FAILED(pGetThemeSysFont(hTheme, WIZ_TMT_CAPTIONFONT, &result))) {
      NONCLIENTMETRICS ncm;
      ncm.cbSize = sizeof(NONCLIENTMETRICS);
      SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICS), &ncm, false);
      result = ncm.lfMessageFont;
   }

   return result;
}

static bool getCaptionQFont(int dpi, QFont *result)
{
   if (!pOpenThemeData) {
      return false;
   }

   const HANDLE hTheme = pOpenThemeData(QApplicationPrivate::getHWNDForWidget(QApplication::desktop()), L"WINDOW");

   if (! hTheme) {
      return false;
   }

   // Call into QWindowsNativeInterface to convert the LOGFONT into a QFont
   const LOGFONT logFont = getCaptionLogFont(hTheme);
   QPlatformNativeInterface *ni = QGuiApplication::platformNativeInterface();

   return ni && QMetaObject::invokeMethod(ni, "logFontToQFont", Qt::DirectConnection,
         Q_RETURN_ARG(QFont, *result), Q_ARG(const void *, &logFont), Q_ARG(int, dpi));
}

void QVistaHelper::drawTitleBar(QPainter *painter)
{
   Q_ASSERT(backButton_);
   QPoint origin;

   const bool isWindow = wizard->isWindow();
   const HDC hdc = QVistaHelper::backingStoreDC(wizard, &origin);

   if (vistaState() == VistaAero && isWindow) {
      drawBlackRect(QRect(0, 0, wizard->width(),
            titleBarSize() + topOffset()), hdc);
   }

   const int btnTop = backButton_->mapToParent(QPoint()).y();
   const int btnHeight = backButton_->size().height();
   const int verticalCenter = (btnTop + btnHeight / 2) - 1;

   const QString text = wizard->window()->windowTitle();

   QFont font;
   if (!isWindow || !getCaptionQFont(wizard->logicalDpiY() * wizard->devicePixelRatio(), &font)) {
      font = QApplication::font("QMdiSubWindowTitleBar");
   }

   const QFontMetrics fontMetrics(font);
   const QRect brect = fontMetrics.boundingRect(text);

   int textHeight = brect.height();
   int textWidth = brect.width();
   int glowOffset = 0;

   if (vistaState() == VistaAero) {
      textHeight += 2 * glowSize();
      textWidth += 2 * glowSize();
      glowOffset = glowSize();
   }

   const int titleLeft = (wizard->layoutDirection() == Qt::LeftToRight
         ? titleOffset() - glowOffset
         : wizard->width() - titleOffset() - textWidth + glowOffset);
   const QRect textRectangle(titleLeft, verticalCenter - textHeight / 2, textWidth, textHeight);
   if (isWindow) {
      drawTitleText(painter, text, textRectangle, hdc);
   } else {
      painter->save();
      painter->setFont(font);
      painter->drawText(textRectangle, Qt::AlignVCenter | Qt::AlignHCenter, text);
      painter->restore();
   }

   const QIcon windowIcon = wizard->windowIcon();
   if (! windowIcon.isNull()) {
      const int size = QVistaHelper::iconSize();

      const int iconLeft = (wizard->layoutDirection() == Qt::LeftToRight
            ? leftMargin()
            : wizard->width() - leftMargin() - size);

      const QPoint pos(origin.x() + iconLeft, origin.y() + verticalCenter - size / 2);
      const QPoint posDp = pos * QVistaHelper::m_devicePixelRatio;
      const HICON hIcon = qt_pixmapToWinHICON(windowIcon.pixmap(size * QVistaHelper::m_devicePixelRatio));

      DrawIconEx(hdc, posDp.x(), posDp.y(), hIcon, 0, 0, 0, NULL, DI_NORMAL | DI_COMPAT);

      DestroyIcon(hIcon);
   }
}

void QVistaHelper::setTitleBarIconAndCaptionVisible(bool visible)
{
   if (is_vista) {
      WIZ_WTA_OPTIONS opt;
      opt.dwFlags = WIZ_WTNCA_NODRAWICON | WIZ_WTNCA_NODRAWCAPTION;

      if (visible) {
         opt.dwMask = 0;

      } else {
         opt.dwMask = WIZ_WTNCA_NODRAWICON | WIZ_WTNCA_NODRAWCAPTION;
      }

      if (const HWND handle = wizardHWND()) {
         pSetWindowThemeAttribute(handle, WIZ_WTA_NONCLIENT, &opt, sizeof(WIZ_WTA_OPTIONS));
      }
   }
}

bool QVistaHelper::winEvent(MSG *msg, long *result)
{
   switch (msg->message) {
      case WM_NCHITTEST: {
         LRESULT lResult;
         // Perform hit testing using DWM
         if (pDwmDefWindowProc(msg->hwnd, msg->message, msg->wParam, msg->lParam, &lResult)) {
            // DWM returned a hit, no further processing necessary
            *result = lResult;
         } else {
            // DWM didn't return a hit, process using DefWindowProc
            lResult = DefWindowProc(msg->hwnd, msg->message, msg->wParam, msg->lParam);
            // If DefWindowProc returns a window caption button, just return HTCLIENT (client area).
            // This avoid unnecessary hits to Windows NT style caption buttons which aren't visible but are
            // located just under the Aero style window close button.
            if (lResult == HTCLOSE || lResult == HTMAXBUTTON || lResult == HTMINBUTTON || lResult == HTHELP) {
               *result = HTCLIENT;
            } else {
               *result = lResult;
            }
         }
         break;
      }

      default:
         LRESULT lResult;
         // Pass to DWM to handle
         if (pDwmDefWindowProc(msg->hwnd, msg->message, msg->wParam, msg->lParam, &lResult)) {
            *result = lResult;
         }
         // If the message wasn't handled by DWM, continue processing it as normal
         else {
            return false;
         }
   }

   return true;
}

void QVistaHelper::setMouseCursor(QPoint pos)
{
#ifndef QT_NO_CURSOR
   if (rtTop.contains(pos)) {
      wizard->setCursor(Qt::SizeVerCursor);
   } else {
      wizard->setCursor(Qt::ArrowCursor);
   }
#endif
}

void QVistaHelper::mouseEvent(QEvent *event)
{
   switch (event->type()) {
      case QEvent::MouseMove:
         mouseMoveEvent(static_cast<QMouseEvent *>(event));
         break;
      case QEvent::MouseButtonPress:
         mousePressEvent(static_cast<QMouseEvent *>(event));
         break;
      case QEvent::MouseButtonRelease:
         mouseReleaseEvent(static_cast<QMouseEvent *>(event));
         break;
      default:
         break;
   }
}

bool QVistaHelper::handleWinEvent(MSG *message, long *result)
{
   if (message->message == WIZ_WM_THEMECHANGED || message->message == WIZ_WM_DWMCOMPOSITIONCHANGED) {
      cachedVistaState = Dirty;
   }

   bool status = false;
   if (wizard->wizardStyle() == QWizard::AeroStyle && vistaState() == VistaAero) {
      status = winEvent(message, result);

      if (message->message == WM_NCPAINT) {
         wizard->update();
      }
   }

   return status;
}

void QVistaHelper::resizeEvent(QResizeEvent *event)
{
   (void) event;

   rtTop = QRect (0, 0, wizard->width(), frameSize());
   int height = captionSize() + topOffset();

   if (vistaState() == VistaBasic) {
      height -= titleBarSize();
   }

   rtTitle = QRect (0, frameSize(), wizard->width(), height);
}

void QVistaHelper::paintEvent(QPaintEvent *event)
{
   (void) event;

   QPainter painter(wizard);
   drawTitleBar(&painter);
}

void QVistaHelper::mouseMoveEvent(QMouseEvent *event)
{
   if (wizard->windowState() & Qt::WindowMaximized) {
      event->ignore();
      return;
   }

   QRect rect = wizard->geometry();
   if (pressed) {
      switch (change) {
         case resizeTop: {
            const int dy = event->pos().y() - pressedPos.y();
            if ((dy > 0 && rect.height() > wizard->minimumHeight())
               || (dy < 0 && rect.height() < wizard->maximumHeight())) {
               rect.setTop(rect.top() + dy);
            }
         }
         break;
         case movePosition: {
            QPoint newPos = event->pos() - pressedPos;
            rect.moveLeft(rect.left() + newPos.x());
            rect.moveTop(rect.top() + newPos.y());
            break;
         }
         default:
            break;
      }
      wizard->setGeometry(rect);

   } else if (vistaState() == VistaAero) {
      setMouseCursor(event->pos());
   }
   event->ignore();
}

void QVistaHelper::mousePressEvent(QMouseEvent *event)
{
   change = noChange;

   if (event->button() != Qt::LeftButton || wizard->windowState() & Qt::WindowMaximized) {
      event->ignore();
      return;
   }

   if (rtTitle.contains(event->pos())) {
      change = movePosition;
   } else if (rtTop.contains(event->pos())) {
      change = (vistaState() == VistaAero) ? resizeTop : movePosition;
   }

   if (change != noChange) {
      if (vistaState() == VistaAero) {
         setMouseCursor(event->pos());
      }
      pressed = true;
      pressedPos = event->pos();
   } else {
      event->ignore();
   }
}

void QVistaHelper::mouseReleaseEvent(QMouseEvent *event)
{
   change = noChange;
   if (pressed) {
      pressed = false;
      wizard->releaseMouse();
      if (vistaState() == VistaAero) {
         setMouseCursor(event->pos());
      }
   }
   event->ignore();
}

bool QVistaHelper::eventFilter(QObject *obj, QEvent *event)
{
   if (obj != wizard) {
      return QObject::eventFilter(obj, event);
   }

   if (event->type() == QEvent::MouseMove) {
      QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
      long result;
      MSG msg;
      msg.message = WM_NCHITTEST;
      msg.wParam  = 0;
      msg.lParam = MAKELPARAM(mouseEvent->globalX(), mouseEvent->globalY());
      msg.hwnd = wizardHWND();
      winEvent(&msg, &result);
      msg.wParam = result;
      msg.message = WM_NCMOUSEMOVE;
      winEvent(&msg, &result);

   } else if (event->type() == QEvent::MouseButtonPress) {
      QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);

      if (mouseEvent->button() == Qt::LeftButton) {
         long result;
         MSG msg;
         msg.message = WM_NCHITTEST;
         msg.wParam  = 0;
         msg.lParam = MAKELPARAM(mouseEvent->globalX(), mouseEvent->globalY());
         msg.hwnd = wizardHWND();
         winEvent(&msg, &result);
         msg.wParam = result;
         msg.message = WM_NCLBUTTONDOWN;
         winEvent(&msg, &result);
      }
   } else if (event->type() == QEvent::MouseButtonRelease) {
      QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);

      if (mouseEvent->button() == Qt::LeftButton) {
         long result;
         MSG msg;
         msg.message = WM_NCHITTEST;
         msg.wParam  = 0;
         msg.lParam = MAKELPARAM(mouseEvent->globalX(), mouseEvent->globalY());
         msg.hwnd = wizardHWND();
         winEvent(&msg, &result);
         msg.wParam = result;
         msg.message = WM_NCLBUTTONUP;
         winEvent(&msg, &result);
      }
   }

   return false;
}

// Return a HDC for the wizard along with the transformation if the
// wizard is a child window.
HDC QVistaHelper::backingStoreDC(const QWidget *wizard, QPoint *offset)
{
   HDC hdc = static_cast<HDC>(QGuiApplication::platformNativeInterface()->nativeResourceForBackingStore(QByteArray("getDC"),
            wizard->backingStore()));

   *offset = QPoint(0, 0);

   if (! wizard->windowHandle()) {
      if (QWidget *nativeParent = wizard->nativeParentWidget()) {
         *offset = wizard->mapTo(nativeParent, *offset);
      }
   }

   return hdc;
}

HWND QVistaHelper::wizardHWND() const
{
   // Obtain the HWND if the wizard is a top-level window.
   // Do not use winId() as this enforces native children of the parent
   // widget when called before show() as happens when calling setWizardStyle().

   if (QWindow *window = wizard->windowHandle()) {
      if (window->handle()) {
         if (void *vHwnd = QGuiApplication::platformNativeInterface()->nativeResourceForWindow(QByteArray("handle"), window)) {
            return static_cast<HWND>(vHwnd);
         }
      }
   }

   qWarning().nospace() << "Failed to obtain HWND for wizard.";

   return 0;
}

bool QVistaHelper::drawTitleText(QPainter *painter, const QString &text, const QRect &rect, HDC hdc)
{
   bool value = false;

   if (vistaState() == VistaAero) {
      const QRect rectDp = QRect(rect.topLeft() * QVistaHelper::m_devicePixelRatio,
            rect.size() * QVistaHelper::m_devicePixelRatio);

      HWND handle   = QApplicationPrivate::getHWNDForWidget(QApplication::desktop());
      HANDLE hTheme = pOpenThemeData(handle, L"WINDOW");

      if (! hTheme) {
         return false;
      }

      // Set up a memory DC and bitmap that we'll draw into
      HDC dcMem;
      HBITMAP bmp;
      BITMAPINFO dib = { };
      dcMem = CreateCompatibleDC(hdc);

      dib.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
      dib.bmiHeader.biWidth       = rectDp.width();
      dib.bmiHeader.biHeight      = -rectDp.height();
      dib.bmiHeader.biPlanes      = 1;
      dib.bmiHeader.biBitCount    = 32;
      dib.bmiHeader.biCompression = BI_RGB;

      bmp = CreateDIBSection(hdc, &dib, DIB_RGB_COLORS, NULL, NULL, 0);

      // Set up the DC
      const LOGFONT captionLogFont = getCaptionLogFont(hTheme);
      const HFONT hCaptionFont     = CreateFontIndirect(&captionLogFont);

      HBITMAP hOldBmp = (HBITMAP)SelectObject(dcMem, (HGDIOBJ) bmp);
      HFONT hOldFont  = (HFONT)SelectObject(dcMem, (HGDIOBJ) hCaptionFont);

      // Draw the text
      WIZ_DTTOPTS dto;
      dto.dwSize = sizeof(WIZ_DTTOPTS);

      const UINT uFormat = WIZ_DT_SINGLELINE | WIZ_DT_CENTER | WIZ_DT_VCENTER | WIZ_DT_NOPREFIX;
      RECT rctext = {0, 0, rectDp.width(), rectDp.height()};

      dto.dwFlags = WIZ_DTT_COMPOSITED | WIZ_DTT_GLOWSIZE;
      dto.iGlowSize = glowSize();

      pDrawThemeTextEx(hTheme, dcMem, 0, 0, &text.toStdWString()[0], -1, uFormat, &rctext, &dto );
      BitBlt(hdc, rectDp.left(), rectDp.top(), rectDp.width(), rectDp.height(), dcMem, 0, 0, SRCCOPY);

      SelectObject(dcMem, (HGDIOBJ) hOldBmp);
      SelectObject(dcMem, (HGDIOBJ) hOldFont);
      DeleteObject(bmp);
      DeleteObject(hCaptionFont);
      DeleteDC(dcMem);
      //ReleaseDC(hwnd, hdc);

   } else if (vistaState() == VistaBasic) {
      painter->drawText(rect, text);
   }
   return value;
}

bool QVistaHelper::drawBlackRect(const QRect &rect, HDC hdc)
{
   bool value = false;
   if (vistaState() == VistaAero) {
      // Set up a memory DC and bitmap that we'll draw into

      const QRect rectDp = QRect(rect.topLeft() * QVistaHelper::m_devicePixelRatio,
            rect.size() * QVistaHelper::m_devicePixelRatio);
      HDC dcMem;
      HBITMAP bmp;
      BITMAPINFO dib = { };
      dcMem = CreateCompatibleDC(hdc);

      dib.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
      dib.bmiHeader.biWidth = rectDp.width();
      dib.bmiHeader.biHeight = -rectDp.height();
      dib.bmiHeader.biPlanes = 1;
      dib.bmiHeader.biBitCount = 32;
      dib.bmiHeader.biCompression = BI_RGB;

      bmp = CreateDIBSection(hdc, &dib, DIB_RGB_COLORS, NULL, NULL, 0);
      HBITMAP hOldBmp = (HBITMAP)SelectObject(dcMem, (HGDIOBJ) bmp);

      BitBlt(hdc, rectDp.left(), rectDp.top(), rectDp.width(), rectDp.height(), dcMem, 0, 0, SRCCOPY);
      SelectObject(dcMem, (HGDIOBJ) hOldBmp);

      DeleteObject(bmp);
      DeleteDC(dcMem);
   }

   return value;
}

#if ! defined(_MSC_VER)
static inline int getWindowBottomMargin()
{
   return GetSystemMetrics(SM_CYSIZEFRAME);
}

#else
static inline int getWindowBottomMargin()
{
   RECT rect = {0, 0, 0, 0};
   AdjustWindowRectEx(&rect, WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_THICKFRAME | WS_DLGFRAME, FALSE, 0);
   return qAbs(rect.bottom);
}
#endif

int QVistaHelper::frameSizeDp()
{
   return getWindowBottomMargin();
}

int QVistaHelper::captionSizeDp()
{
   return GetSystemMetrics(SM_CYCAPTION);
}
bool QVistaHelper::resolveSymbols()
{
   static bool tried = false;
   if (! tried) {
      tried = true;
      QSystemLibrary dwmLib("dwmapi");

      pDwmIsCompositionEnabled =
         (PtrDwmIsCompositionEnabled)dwmLib.resolve("DwmIsCompositionEnabled");

      if (pDwmIsCompositionEnabled) {
         pDwmDefWindowProc = (PtrDwmDefWindowProc)dwmLib.resolve("DwmDefWindowProc");
         pDwmExtendFrameIntoClientArea = (PtrDwmExtendFrameIntoClientArea)dwmLib.resolve("DwmExtendFrameIntoClientArea");
      }

      QSystemLibrary themeLib("uxtheme");
      pIsAppThemed = (PtrIsAppThemed)themeLib.resolve("IsAppThemed");

      if (pIsAppThemed) {
         pDrawThemeBackground = (PtrDrawThemeBackground)themeLib.resolve("DrawThemeBackground");
         pGetThemePartSize = (PtrGetThemePartSize)themeLib.resolve("GetThemePartSize");
         pGetThemeColor = (PtrGetThemeColor)themeLib.resolve("GetThemeColor");
         pIsThemeActive = (PtrIsThemeActive)themeLib.resolve("IsThemeActive");
         pOpenThemeData = (PtrOpenThemeData)themeLib.resolve("OpenThemeData");
         pCloseThemeData = (PtrCloseThemeData)themeLib.resolve("CloseThemeData");
         pGetThemeSysFont = (PtrGetThemeSysFont)themeLib.resolve("GetThemeSysFont");
         pDrawThemeTextEx = (PtrDrawThemeTextEx)themeLib.resolve("DrawThemeTextEx");
         pSetWindowThemeAttribute = (PtrSetWindowThemeAttribute)themeLib.resolve("SetWindowThemeAttribute");
      }
   }

   return (
         pDwmIsCompositionEnabled != 0
         && pDwmDefWindowProc != 0
         && pDwmExtendFrameIntoClientArea != 0
         && pIsAppThemed != 0
         && pDrawThemeBackground != 0
         && pGetThemePartSize != 0
         && pGetThemeColor != 0
         && pIsThemeActive != 0
         && pOpenThemeData != 0
         && pCloseThemeData != 0
         && pGetThemeSysFont != 0
         && pDrawThemeTextEx != 0
         && pSetWindowThemeAttribute != 0
      );
}

int QVistaHelper::titleOffset()
{
   int iconOffset = wizard ->windowIcon().isNull() ? 0 : iconSize() + textSpacing;
   return leftMargin() + iconOffset;
}

int QVistaHelper::iconSize()
{
   return QStyleHelper::dpiScaled(16); // Standard Aero
}

int QVistaHelper::glowSize()
{
   return QStyleHelper::dpiScaled(10);
}
int QVistaHelper::topOffset()
{
   if (vistaState() != VistaAero) {
      return titleBarSize() + 3;
   }

   static const int aeroOffset = QSysInfo::WindowsVersion == QSysInfo::WV_WINDOWS7 ?
      QStyleHelper::dpiScaled(4) : QStyleHelper::dpiScaled(13);

   return aeroOffset + titleBarSize();
}



#endif // QT_NO_STYLE_WINDOWSVISTA

#endif // QT_NO_WIZARD
