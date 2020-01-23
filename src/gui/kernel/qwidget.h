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

#ifndef QWIDGET_H
#define QWIDGET_H

#include <qwindowdefs.h>
#include <qobject.h>
#include <qmargins.h>
#include <qpaintdevice.h>
#include <qpalette.h>
#include <qfont.h>
#include <qfontmetrics.h>
#include <qfontinfo.h>
#include <qsizepolicy.h>
#include <qregion.h>
#include <qbrush.h>
#include <qcursor.h>
#include <qkeysequence.h>
#include <qicon.h>
#include <qlocale.h>
#include <QScopedPointer>

class QLayout;
class QWSRegionManager;
class QStyle;
class QAction;
class QVariant;
class QWindow;
class QActionEvent;
class QMouseEvent;
class QWheelEvent;
class QHoverEvent;
class QKeyEvent;
class QFocusEvent;
class QPaintEvent;
class QMoveEvent;
class QResizeEvent;
class QCloseEvent;
class QContextMenuEvent;
class QInputMethodEvent;
class QTabletEvent;
class QDragEnterEvent;
class QDragMoveEvent;
class QDragLeaveEvent;
class QDropEvent;
class QShowEvent;
class QHideEvent;

class QBackingStore;
class QPlatformWindow;
class QGraphicsProxyWidget;
class QGraphicsEffect;
class QRasterWindowSurface;
class QUnifiedToolbarSurface;
class QPixmap;
class QDebug;

class QWidgetData
{

 public:
   WId winid;
   uint widget_attributes;
   Qt::WindowFlags window_flags;
   uint window_state : 4;
   uint focus_policy : 4;
   uint sizehint_forced : 1;
   uint is_closing : 1;
   uint in_show : 1;
   uint in_set_window_state : 1;
   mutable uint fstrut_dirty : 1;
   uint context_menu_policy : 3;
   uint window_modality : 2;
   uint in_destructor : 1;
   uint unused : 13;
   QRect crect;
   mutable QPalette pal;
   QFont fnt;

   QRect wrect;
};

class QWidgetPrivate;

class Q_GUI_EXPORT QWidget : public QObject, public QPaintDevice
{
   GUI_CS_OBJECT_MULTIPLE(QWidget, QObject)
   Q_DECLARE_PRIVATE(QWidget)

   GUI_CS_PROPERTY_READ(modal, isModal)

   GUI_CS_PROPERTY_READ(windowModality, windowModality)
   GUI_CS_PROPERTY_WRITE(windowModality, setWindowModality)

   GUI_CS_PROPERTY_READ(enabled, isEnabled)
   GUI_CS_PROPERTY_WRITE(enabled, setEnabled)

   GUI_CS_PROPERTY_READ(geometry, geometry)
   GUI_CS_PROPERTY_WRITE(geometry, cs_setGeometry)

   GUI_CS_PROPERTY_READ(frameGeometry, frameGeometry)
   GUI_CS_PROPERTY_READ(normalGeometry, normalGeometry)

   GUI_CS_PROPERTY_READ(x, x)
   GUI_CS_PROPERTY_READ(y, y)

   GUI_CS_PROPERTY_READ(pos, pos)
   GUI_CS_PROPERTY_WRITE(pos, cs_move)
   GUI_CS_PROPERTY_DESIGNABLE(pos, false)
   GUI_CS_PROPERTY_STORED(pos, false)

   GUI_CS_PROPERTY_READ(size, size)
   GUI_CS_PROPERTY_WRITE(size, cs_resize)
   GUI_CS_PROPERTY_DESIGNABLE(size, false)
   GUI_CS_PROPERTY_STORED(size, false)

   GUI_CS_PROPERTY_READ(frameSize, frameSize)

   GUI_CS_PROPERTY_READ(width, width)
   GUI_CS_PROPERTY_READ(height, height)
   GUI_CS_PROPERTY_READ(rect, rect)
   GUI_CS_PROPERTY_READ(childrenRect, childrenRect)
   GUI_CS_PROPERTY_READ(childrenRegion, childrenRegion)

   GUI_CS_PROPERTY_READ(fullScreen, isFullScreen)
   GUI_CS_PROPERTY_READ(sizeHint, sizeHint)
   GUI_CS_PROPERTY_READ(minimumSizeHint, minimumSizeHint)

   GUI_CS_PROPERTY_READ(sizePolicy, sizePolicy)
   GUI_CS_PROPERTY_WRITE(sizePolicy, cs_setSizePolicy)

   GUI_CS_PROPERTY_READ(minimumSize, minimumSize)
   GUI_CS_PROPERTY_WRITE(minimumSize, cs_setMinimumSize)

   GUI_CS_PROPERTY_READ(maximumSize, maximumSize)
   GUI_CS_PROPERTY_WRITE(maximumSize, cs_setMaximumSize)

   GUI_CS_PROPERTY_READ(minimumWidth, minimumWidth)
   GUI_CS_PROPERTY_WRITE(minimumWidth, setMinimumWidth)
   GUI_CS_PROPERTY_STORED(minimumWidth, false)
   GUI_CS_PROPERTY_DESIGNABLE(minimumWidth, false)

   GUI_CS_PROPERTY_READ(minimumHeight, minimumHeight)
   GUI_CS_PROPERTY_WRITE(minimumHeight, setMinimumHeight)
   GUI_CS_PROPERTY_STORED(minimumHeight, false)
   GUI_CS_PROPERTY_DESIGNABLE(minimumHeight, false)

   GUI_CS_PROPERTY_READ(maximumWidth, maximumWidth)
   GUI_CS_PROPERTY_WRITE(maximumWidth, setMaximumWidth)
   GUI_CS_PROPERTY_STORED(maximumWidth, false)
   GUI_CS_PROPERTY_DESIGNABLE(maximumWidth, false)

   GUI_CS_PROPERTY_READ(maximumHeight, maximumHeight)
   GUI_CS_PROPERTY_WRITE(maximumHeight, setMaximumHeight)
   GUI_CS_PROPERTY_STORED(maximumHeight, false)
   GUI_CS_PROPERTY_DESIGNABLE(maximumHeight, false)

   GUI_CS_PROPERTY_READ(sizeIncrement, sizeIncrement)
   GUI_CS_PROPERTY_WRITE(sizeIncrement, cs_setSizeIncrement)

   GUI_CS_PROPERTY_READ(baseSize, baseSize)
   GUI_CS_PROPERTY_WRITE(baseSize, cs_setBaseSize)

   GUI_CS_PROPERTY_READ(palette, palette)
   GUI_CS_PROPERTY_WRITE(palette, setPalette)

   GUI_CS_PROPERTY_READ(font, font)
   GUI_CS_PROPERTY_WRITE(font, setFont)

#ifndef QT_NO_CURSOR
   GUI_CS_PROPERTY_READ(cursor, cursor)
   GUI_CS_PROPERTY_WRITE(cursor, setCursor)
   GUI_CS_PROPERTY_RESET(cursor, unsetCursor)
#endif

   GUI_CS_PROPERTY_READ(mouseTracking, hasMouseTracking)
   GUI_CS_PROPERTY_WRITE(mouseTracking, setMouseTracking)

   GUI_CS_PROPERTY_READ(isActiveWindow, isActiveWindow)

   GUI_CS_PROPERTY_READ(focusPolicy, focusPolicy)
   GUI_CS_PROPERTY_WRITE(focusPolicy, setFocusPolicy)

   GUI_CS_PROPERTY_READ(focus, hasFocus)

   GUI_CS_PROPERTY_READ(contextMenuPolicy, contextMenuPolicy)
   GUI_CS_PROPERTY_WRITE(contextMenuPolicy, setContextMenuPolicy)

   GUI_CS_PROPERTY_READ(updatesEnabled, updatesEnabled)
   GUI_CS_PROPERTY_WRITE(updatesEnabled, setUpdatesEnabled)
   GUI_CS_PROPERTY_DESIGNABLE(updatesEnabled, false)

   GUI_CS_PROPERTY_READ(visible, isVisible)
   GUI_CS_PROPERTY_WRITE(visible, setVisible)
   GUI_CS_PROPERTY_DESIGNABLE(visible, false)

   GUI_CS_PROPERTY_READ(minimized, isMinimized)
   GUI_CS_PROPERTY_READ(maximized, isMaximized)
   GUI_CS_PROPERTY_READ(acceptDrops, acceptDrops)
   GUI_CS_PROPERTY_WRITE(acceptDrops, setAcceptDrops)

   GUI_CS_PROPERTY_READ(windowTitle, windowTitle)
   GUI_CS_PROPERTY_WRITE(windowTitle, setWindowTitle)
   GUI_CS_PROPERTY_NOTIFY(windowTitle, windowTitleChanged)
   GUI_CS_PROPERTY_DESIGNABLE_NONSTATIC(windowTitle, isWindow())

   GUI_CS_PROPERTY_READ(windowIcon, windowIcon)
   GUI_CS_PROPERTY_WRITE(windowIcon, setWindowIcon)
   GUI_CS_PROPERTY_NOTIFY(windowIcon, windowIconChanged)
   GUI_CS_PROPERTY_DESIGNABLE_NONSTATIC(windowIcon, isWindow())

   GUI_CS_PROPERTY_READ(windowIconText, windowIconText)
   GUI_CS_PROPERTY_WRITE(windowIconText, setWindowIconText)
   GUI_CS_PROPERTY_NOTIFY(windowIconText, windowIconTextChanged)
   GUI_CS_PROPERTY_DESIGNABLE_NONSTATIC(windowIconText, isWindow())

   GUI_CS_PROPERTY_READ(windowOpacity, windowOpacity)
   GUI_CS_PROPERTY_WRITE(windowOpacity, setWindowOpacity)
   GUI_CS_PROPERTY_DESIGNABLE_NONSTATIC(windowOpacity, isWindow())

   GUI_CS_PROPERTY_READ(windowModified, isWindowModified)
   GUI_CS_PROPERTY_WRITE(windowModified, setWindowModified)
   GUI_CS_PROPERTY_DESIGNABLE_NONSTATIC(windowModified, isWindow())

#ifndef QT_NO_ACCESSIBILITY
   GUI_CS_PROPERTY_READ(accessibleName, accessibleName)
   GUI_CS_PROPERTY_WRITE(accessibleName, setAccessibleName)

   GUI_CS_PROPERTY_READ(accessibleDescription, accessibleDescription)
   GUI_CS_PROPERTY_WRITE(accessibleDescription, setAccessibleDescription)
#endif

#ifndef QT_NO_STATUSTIP
   GUI_CS_PROPERTY_READ(statusTip, statusTip)
   GUI_CS_PROPERTY_WRITE(statusTip, setStatusTip)
#endif

#ifndef QT_NO_STYLE_STYLESHEET
   GUI_CS_PROPERTY_READ(styleSheet, styleSheet)
   GUI_CS_PROPERTY_WRITE(styleSheet, setStyleSheet)
#endif

#ifndef QT_NO_TOOLTIP
   GUI_CS_PROPERTY_READ(toolTip, toolTip)
   GUI_CS_PROPERTY_WRITE(toolTip, setToolTip)

   GUI_CS_PROPERTY_READ(toolTipDuration, toolTipDuration)
   GUI_CS_PROPERTY_WRITE(toolTipDuration, setToolTipDuration)

#endif

#ifndef QT_NO_WHATSTHIS
   GUI_CS_PROPERTY_READ(whatsThis, whatsThis)
   GUI_CS_PROPERTY_WRITE(whatsThis, setWhatsThis)
#endif

   GUI_CS_PROPERTY_READ(layoutDirection, layoutDirection)
   GUI_CS_PROPERTY_WRITE(layoutDirection, setLayoutDirection)
   GUI_CS_PROPERTY_RESET(layoutDirection, unsetLayoutDirection)

   // following 2 were qdoc_property 1/5/2014
   GUI_CS_PROPERTY_READ(windowFlags, windowFlags)
   GUI_CS_PROPERTY_WRITE(windowFlags, setWindowFlags)

   GUI_CS_PROPERTY_READ(autoFillBackground, autoFillBackground)
   GUI_CS_PROPERTY_WRITE(autoFillBackground, setAutoFillBackground)

   GUI_CS_PROPERTY_READ(locale, locale)
   GUI_CS_PROPERTY_WRITE(locale, setLocale)
   GUI_CS_PROPERTY_RESET(locale, unsetLocale)

   GUI_CS_PROPERTY_READ(windowFilePath, windowFilePath)
   GUI_CS_PROPERTY_WRITE(windowFilePath, setWindowFilePath)
   GUI_CS_PROPERTY_DESIGNABLE_NONSTATIC(windowFilePath, isWindow())

   GUI_CS_PROPERTY_READ(inputMethodHints, inputMethodHints)
   GUI_CS_PROPERTY_WRITE(inputMethodHints, setInputMethodHints)

 public:
   enum RenderFlag {
      DrawWindowBackground = 0x1,
      DrawChildren = 0x2,
      IgnoreMask = 0x4
   };
   using RenderFlags = QFlags<RenderFlag>;

   explicit QWidget(QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
   ~QWidget();

   int devType() const override;

   WId winId() const;
   void createWinId();       // internal, may go away

   WId internalWinId() const {
      return data->winid;
   }

   WId effectiveWinId() const;

   // GUI style setting
   QStyle *style() const;
   void setStyle(QStyle *);

   // Widget types and states
   inline bool isTopLevel() const;
   inline bool isWindow() const;

   inline bool isModal() const;
   Qt::WindowModality windowModality() const;
   void setWindowModality(Qt::WindowModality windowModality);

   inline bool isEnabled() const;
   bool isEnabledTo(const QWidget *) const;
   inline bool isEnabledToTLW() const;

   GUI_CS_SLOT_1(Public, void setEnabled(bool enabled))
   GUI_CS_SLOT_2(setEnabled)

   GUI_CS_SLOT_1(Public, void setDisabled(bool disable))
   GUI_CS_SLOT_2(setDisabled)

   GUI_CS_SLOT_1(Public, void setWindowModified(bool modified))
   GUI_CS_SLOT_2(setWindowModified)

   // Widget coordinates
   QRect frameGeometry() const;
   inline const QRect &geometry() const;
   QRect normalGeometry() const;

   int x() const;
   int y() const;
   QPoint pos() const;
   QSize frameSize() const;
   inline QSize size() const;
   inline int width() const;
   inline int height() const;
   inline QRect rect() const;
   QRect childrenRect() const;
   QRegion childrenRegion() const;

   QSize minimumSize() const;
   QSize maximumSize() const;
   inline int minimumWidth() const;
   inline int minimumHeight() const;
   inline int maximumWidth() const;
   inline int maximumHeight() const;

   inline void setMinimumSize(const QSize &);
   void setMinimumSize(int minw, int minh);

   // wrapper for overloaded property
   inline void cs_setMinimumSize(const QSize &size);

   inline void setMaximumSize(const QSize &);
   void setMaximumSize(int maxw, int maxh);

   // wrapper for overloaded property
   inline void cs_setMaximumSize(const QSize &size);

   void setMinimumWidth(int minw);
   void setMinimumHeight(int minh);
   void setMaximumWidth(int maxw);
   void setMaximumHeight(int maxh);

   QSize sizeIncrement() const;
   inline void setSizeIncrement(const QSize &);
   void setSizeIncrement(int w, int h);

   // wrapper for overloaded property
   inline void cs_setSizeIncrement(const QSize &un_named_arg1);

   QSize baseSize() const;
   inline void setBaseSize(const QSize &size);
   void setBaseSize(int basew, int baseh);

   // wrapper for overloaded property
   inline void cs_setBaseSize(const QSize &un_named_arg1);

   void setFixedSize(const QSize &);
   void setFixedSize(int w, int h);
   void setFixedWidth(int w);
   void setFixedHeight(int h);

   // Widget coordinate mapping
   QPoint mapToGlobal(const QPoint &) const;
   QPoint mapFromGlobal(const QPoint &) const;
   QPoint mapToParent(const QPoint &) const;
   QPoint mapFromParent(const QPoint &) const;
   QPoint mapTo(const QWidget *, const QPoint &) const;
   QPoint mapFrom(const QWidget *, const QPoint &) const;

   QWidget *window() const;
   QWidget *nativeParentWidget() const;

   inline QWidget *topLevelWidget() const {
      return window();
   }

   // Widget appearance functions
   const QPalette &palette() const;
   void setPalette(const QPalette &);

   void setBackgroundRole(QPalette::ColorRole);
   QPalette::ColorRole backgroundRole() const;

   void setForegroundRole(QPalette::ColorRole);
   QPalette::ColorRole foregroundRole() const;

   const QFont &font() const;
   void setFont(const QFont &);
   QFontMetrics fontMetrics() const;
   QFontInfo fontInfo() const;

#ifndef QT_NO_CURSOR
   QCursor cursor() const;
   void setCursor(const QCursor &);
   void unsetCursor();
#endif

   void setMouseTracking(bool enable);
   bool hasMouseTracking() const;
   bool underMouse() const;

   void setMask(const QBitmap &);
   void setMask(const QRegion &);
   QRegion mask() const;
   void clearMask();

   void render(QPaintDevice *target, const QPoint &targetOffset = QPoint(),
      const QRegion &sourceRegion = QRegion(),
      RenderFlags renderFlags = RenderFlags(DrawWindowBackground | DrawChildren));

   void render(QPainter *painter, const QPoint &targetOffset = QPoint(),
      const QRegion &sourceRegion = QRegion(),
      RenderFlags renderFlags = RenderFlags(DrawWindowBackground | DrawChildren));

   QPixmap grab(const QRect &rectangle = QRect(QPoint(0, 0), QSize(-1, -1)));
#ifndef QT_NO_GRAPHICSEFFECT
   QGraphicsEffect *graphicsEffect() const;
   void setGraphicsEffect(QGraphicsEffect *effect);
#endif

#ifndef QT_NO_GESTURES
   void grabGesture(Qt::GestureType type, Qt::GestureFlags flags = Qt::GestureFlags());
   void ungrabGesture(Qt::GestureType type);
#endif

   GUI_CS_SLOT_1(Public, void setWindowTitle(const QString &title))
   GUI_CS_SLOT_2(setWindowTitle)

#ifndef QT_NO_STYLE_STYLESHEET
   GUI_CS_SLOT_1(Public, void setStyleSheet(const QString &styleSheet))
   GUI_CS_SLOT_2(setStyleSheet)
#endif

#ifndef QT_NO_STYLE_STYLESHEET
   QString styleSheet() const;
#endif

   QString windowTitle() const;

   void setWindowIcon(const QIcon &icon);
   QIcon windowIcon() const;

   void setWindowIconText(const QString &);
   QString windowIconText() const;

   void setWindowRole(const QString &);
   QString windowRole() const;

   void setWindowFilePath(const QString &filePath);
   QString windowFilePath() const;

   void setWindowOpacity(qreal level);
   qreal windowOpacity() const;

   bool isWindowModified() const;

#ifndef QT_NO_TOOLTIP
   void setToolTip(const QString &data);
   QString toolTip() const;

   void setToolTipDuration(int msec);
   int toolTipDuration() const;
#endif

#ifndef QT_NO_STATUSTIP
   void setStatusTip(const QString &data);
   QString statusTip() const;
#endif

#ifndef QT_NO_WHATSTHIS
   void setWhatsThis(const QString &str);
   QString whatsThis() const;
#endif

#ifndef QT_NO_ACCESSIBILITY
   QString accessibleName() const;
   void setAccessibleName(const QString &name);
   QString accessibleDescription() const;
   void setAccessibleDescription(const QString &description);
#endif

   void setLayoutDirection(Qt::LayoutDirection direction);
   Qt::LayoutDirection layoutDirection() const;
   void unsetLayoutDirection();

   void setLocale(const QLocale &locale);
   QLocale locale() const;
   void unsetLocale();

   bool isRightToLeft() const {
      return layoutDirection() == Qt::RightToLeft;
   }

   bool isLeftToRight() const {
      return layoutDirection() == Qt::LeftToRight;
   }

   GUI_CS_SLOT_1(Public, void setFocus())
   GUI_CS_SLOT_OVERLOAD(setFocus, ())

   bool isActiveWindow() const;
   void activateWindow();

   void clearFocus();
   bool hasFocus() const;
   void setFocus(Qt::FocusReason reason);

   Qt::FocusPolicy focusPolicy() const;
   void setFocusPolicy(Qt::FocusPolicy policy);

   void setFocusProxy(QWidget *);
   QWidget *focusProxy() const;

   static void setTabOrder(QWidget *, QWidget *);

   Qt::ContextMenuPolicy contextMenuPolicy() const;
   void setContextMenuPolicy(Qt::ContextMenuPolicy policy);

   // Grab functions
   void grabMouse();

#ifndef QT_NO_CURSOR
   void grabMouse(const QCursor &cursor);
#endif

   void releaseMouse();
   void grabKeyboard();
   void releaseKeyboard();

#ifndef QT_NO_SHORTCUT
   int grabShortcut(const QKeySequence &key, Qt::ShortcutContext context = Qt::WindowShortcut);
   void releaseShortcut(int id);
   void setShortcutEnabled(int id, bool enable = true);
   void setShortcutAutoRepeat(int id, bool enable = true);
#endif

   static QWidget *mouseGrabber();
   static QWidget *keyboardGrabber();

   // Update/refresh functions
   inline bool updatesEnabled() const;
   void setUpdatesEnabled(bool enable);

#ifndef QT_NO_GRAPHICSVIEW
   QGraphicsProxyWidget *graphicsProxyWidget() const;
#endif

   GUI_CS_SLOT_1(Public, void update())
   GUI_CS_SLOT_OVERLOAD(update, ())

   GUI_CS_SLOT_1(Public, void repaint())
   GUI_CS_SLOT_OVERLOAD(repaint, ())

   inline void update(int x, int y, int w, int h);
   void update(const QRect &);
   void update(const QRegion &);

   void repaint(int x, int y, int w, int h);
   void repaint(const QRect &);
   void repaint(const QRegion &);

   // Widget management functions
   GUI_CS_SLOT_1(Public, virtual void setVisible(bool visible))
   GUI_CS_SLOT_2(setVisible)

   GUI_CS_SLOT_1(Public, void setHidden(bool hidden))
   GUI_CS_SLOT_2(setHidden)

   GUI_CS_SLOT_1(Public, void show())
   GUI_CS_SLOT_2(show)

   GUI_CS_SLOT_1(Public, void hide())
   GUI_CS_SLOT_2(hide)

   GUI_CS_SLOT_1(Public, void showMinimized())
   GUI_CS_SLOT_2(showMinimized)

   GUI_CS_SLOT_1(Public, void showMaximized())
   GUI_CS_SLOT_2(showMaximized)

   GUI_CS_SLOT_1(Public, void showFullScreen())
   GUI_CS_SLOT_2(showFullScreen)

   GUI_CS_SLOT_1(Public, void showNormal())
   GUI_CS_SLOT_2(showNormal)

   GUI_CS_SLOT_1(Public, bool close())
   GUI_CS_SLOT_2(close)

   GUI_CS_SLOT_1(Public, void raise())
   GUI_CS_SLOT_2(raise)

   GUI_CS_SLOT_1(Public, void lower())
   GUI_CS_SLOT_2(lower)

   void stackUnder(QWidget *);

   inline void move(int x, int y);
   void move(const QPoint &);

   // wrapper for overloaded property
   inline void cs_move(const QPoint &point);

   inline void resize(int w, int h);
   void resize(const QSize &);

   // wrapper for overloaded property
   inline void cs_resize(const QSize &size);

   inline void setGeometry(int x, int y, int w, int h);
   void setGeometry(const QRect &);

   // wrapper for overloaded property
   inline void cs_setGeometry(const QRect &rect);

   QByteArray saveGeometry() const;
   bool restoreGeometry(const QByteArray &geometry);
   void adjustSize();
   inline bool isVisible() const;
   bool isVisibleTo(const QWidget *) const;

   // ### Qt5/bool isVisibleTo(_const_ QWidget *) const
   inline bool isHidden() const;

   bool isMinimized() const;
   bool isMaximized() const;
   bool isFullScreen() const;

   Qt::WindowStates windowState() const;
   void setWindowState(Qt::WindowStates state);
   void overrideWindowState(Qt::WindowStates state);

   virtual QSize sizeHint() const;
   virtual QSize minimumSizeHint() const;

   QSizePolicy sizePolicy() const;
   void setSizePolicy(QSizePolicy);
   inline void setSizePolicy(QSizePolicy::Policy horizontal, QSizePolicy::Policy vertical);

   // wrapper for overloaded property
   inline void cs_setSizePolicy(const QSizePolicy un_named_arg1);

   virtual int heightForWidth(int) const;
   virtual bool hasHeightForWidth() const;

   QRegion visibleRegion() const;

   void setContentsMargins(int left, int top, int right, int bottom);
   void setContentsMargins(const QMargins &margins);
   void getContentsMargins(int *left, int *top, int *right, int *bottom) const;
   QMargins contentsMargins() const;

   QRect contentsRect() const;

   QLayout *layout() const;
   void setLayout(QLayout *layout);
   void updateGeometry();

   void setParent(QWidget *parent);
   void setParent(QWidget *parent, Qt::WindowFlags f);

   void scroll(int dx, int dy);
   void scroll(int dx, int dy, const QRect &rect);

   // misc functions
   QWidget *focusWidget() const;
   QWidget *nextInFocusChain() const;
   QWidget *previousInFocusChain() const;

   // drag and drop
   bool acceptDrops() const;
   void setAcceptDrops(bool on);

#ifndef QT_NO_ACTION
   void addAction(QAction *action);
   void addActions(const QList<QAction *> &actions);
   void insertAction(QAction *before, QAction *action);
   void insertActions(QAction *before, QList<QAction *> actions);
   void removeAction(QAction *action);

   QList<QAction *> actions() const;
#endif

   inline QWidget *parentWidget() const;

   void setWindowFlags(Qt::WindowFlags type);
   inline Qt::WindowFlags windowFlags() const;
   void overrideWindowFlags(Qt::WindowFlags type);

   inline Qt::WindowType windowType() const;

   static QWidget *find(WId);
   inline QWidget *childAt(int x, int y) const;
   QWidget *childAt(const QPoint &p) const;

   void setAttribute(Qt::WidgetAttribute, bool on = true);
   inline bool testAttribute(Qt::WidgetAttribute) const;

   QPaintEngine *paintEngine() const override;

   void ensurePolished() const;

   bool isAncestorOf(const QWidget *child) const;

#ifdef QT_KEYPAD_NAVIGATION
   bool hasEditFocus() const;
   void setEditFocus(bool on);
#endif

   bool autoFillBackground() const;
   void setAutoFillBackground(bool enabled);

   QBackingStore *backingStore() const;
   QWindow *windowHandle() const;

   static QWidget *createWindowContainer(QWindow *window, QWidget *parent = nullptr, Qt::WindowFlags flags = Qt::WindowFlags());
   GUI_CS_SIGNAL_1(Public, void windowTitleChanged(const QString &title))
   GUI_CS_SIGNAL_2(windowTitleChanged, title)

   GUI_CS_SIGNAL_1(Public, void windowIconChanged(const QIcon &icon))
   GUI_CS_SIGNAL_2(windowIconChanged, icon)

   GUI_CS_SIGNAL_1(Public, void windowIconTextChanged(const QString &iconText))
   GUI_CS_SIGNAL_2(windowIconTextChanged, iconText)

   GUI_CS_SIGNAL_1(Public, void customContextMenuRequested(const QPoint &pos))
   GUI_CS_SIGNAL_2(customContextMenuRequested, pos)

   virtual QVariant inputMethodQuery(Qt::InputMethodQuery) const;
   Qt::InputMethodHints inputMethodHints() const;
   void setInputMethodHints(Qt::InputMethodHints hints);

   friend class QDesktopScreenWidget;

 protected:
   bool event(QEvent *event) override;
   virtual void mousePressEvent(QMouseEvent *event);
   virtual void mouseReleaseEvent(QMouseEvent *event);
   virtual void mouseDoubleClickEvent(QMouseEvent *event);
   virtual void mouseMoveEvent(QMouseEvent *event);

#ifndef QT_NO_WHEELEVENT
   virtual void wheelEvent(QWheelEvent *event);
#endif

   virtual void keyPressEvent(QKeyEvent *event);
   virtual void keyReleaseEvent(QKeyEvent *event);
   virtual void focusInEvent(QFocusEvent *event);
   virtual void focusOutEvent(QFocusEvent *event);
   virtual void enterEvent(QEvent *event);
   virtual void leaveEvent(QEvent *event);
   virtual void paintEvent(QPaintEvent *event);
   virtual void moveEvent(QMoveEvent *event);
   virtual void resizeEvent(QResizeEvent *event);
   virtual void closeEvent(QCloseEvent *event);

#ifndef QT_NO_CONTEXTMENU
   virtual void contextMenuEvent(QContextMenuEvent *event);
#endif

#ifndef QT_NO_TABLETEVENT
   virtual void tabletEvent(QTabletEvent *event);
#endif

#ifndef QT_NO_ACTION
   virtual void actionEvent(QActionEvent *event);
#endif

#ifndef QT_NO_DRAGANDDROP
   virtual void dragEnterEvent(QDragEnterEvent *event);
   virtual void dragMoveEvent(QDragMoveEvent *event);
   virtual void dragLeaveEvent(QDragLeaveEvent *event);
   virtual void dropEvent(QDropEvent *event);
#endif

   virtual void showEvent(QShowEvent *event);
   virtual void hideEvent(QHideEvent *event);
   virtual bool nativeEvent(const QByteArray &eventType, void *message, long *result);
   virtual void changeEvent(QEvent *);
   int metric(PaintDeviceMetric) const override;
   void initPainter(QPainter *painter) const override;
   QPaintDevice *redirected(QPoint *offset) const override;
   QPainter *sharedPainter() const override;
   virtual void inputMethodEvent(QInputMethodEvent *);

 protected:
   bool cs_isWidgetType() const override;

   void create(WId = 0, bool initializeWindow = true, bool destroyOldWindow = true);
   void destroy(bool destroyWindow = true, bool destroySubWindows = true);

   virtual bool focusNextPrevChild(bool next);

   inline bool focusNextChild() {
      return focusNextPrevChild(true);
   }

   inline bool focusPreviousChild() {
      return focusNextPrevChild(false);
   }

   QWidget(QWidgetPrivate &d, QWidget *parent, Qt::WindowFlags f);

   GUI_CS_SLOT_1(Protected, void updateMicroFocus())
   GUI_CS_SLOT_2(updateMicroFocus)

   QScopedPointer<QWidgetPrivate> d_ptr;

   friend class QDataWidgetMapperPrivate; // for access to focusNextPrevChild

 private:
   Q_DISABLE_COPY(QWidget)

   GUI_CS_SLOT_1(Private, void _q_showIfNotHidden())
   GUI_CS_SLOT_2(_q_showIfNotHidden)

   QWidgetData *data;

   void setBackingStore(QBackingStore *store);
   bool testAttribute_helper(Qt::WidgetAttribute) const;

   QLayout *takeLayout();

   friend class QAccessibleWidget;
   friend class QAccessibleTable;
   friend class QApplication;
   friend class QApplicationPrivate;

   friend class QBaseApplication;
   friend class QBackingStoreDevice;

   friend class QFontMetrics;
   friend class QFontInfo;
   friend class QGraphicsProxyWidget;
   friend class QGraphicsProxyWidgetPrivate;
   friend class QGLContext;
   friend class QGLWidget;
   friend class QGLWindowSurface;

   friend class QLayout;

   friend class QPainter;
   friend class QPainterPrivate;
   friend class QPixmap;
   friend class QStyleSheetStyle;
   friend class QShortcutPrivate;

   friend class QX11PaintEngine;
   friend class QWin32PaintEngine;

   friend class QWindowSurface;;
   friend class QWidgetItem;
   friend class QWidgetItemV2;
   friend class QWidgetWindow;
   friend class QWidgetBackingStore;
   friend class QWidgetEffectSourcePrivate;

   friend struct QWidgetExceptionCleaner;

#ifndef QT_NO_GESTURES
   friend class QGestureManager;
   friend class QWinNativePanGestureRecognizer;
#endif

#ifdef Q_OS_DARWIN
   friend bool qt_mac_is_metal(const QWidget *w);
#endif

   friend Q_GUI_EXPORT QWidgetData *qt_qwidget_data(QWidget *widget);
   friend Q_GUI_EXPORT QWidgetPrivate *qt_widget_private(QWidget *widget);
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QWidget::RenderFlags)

template <>
inline QWidget *qobject_cast<QWidget *>(QObject *o)
{
   if (!o || !o->isWidgetType()) {
      return nullptr;
   }
   return static_cast<QWidget *>(o);
}

template <>
inline const QWidget *qobject_cast<const QWidget *>(const QObject *o)
{
   if (!o || !o->isWidgetType()) {
      return nullptr;
   }
   return static_cast<const QWidget *>(o);
}

inline QWidget *QWidget::childAt(int ax, int ay) const
{
   return childAt(QPoint(ax, ay));
}

inline Qt::WindowType QWidget::windowType() const
{
   return static_cast<Qt::WindowType>(int(data->window_flags & Qt::WindowType_Mask));
}

inline Qt::WindowFlags QWidget::windowFlags() const
{
   return data->window_flags;
}

inline bool QWidget::isTopLevel() const
{
   return (windowType() & Qt::Window);
}

inline bool QWidget::isWindow() const
{
   return (windowType() & Qt::Window);
}

inline bool QWidget::isEnabled() const
{
   return !testAttribute(Qt::WA_Disabled);
}

inline bool QWidget::isModal() const
{
   return data->window_modality != Qt::NonModal;
}

inline bool QWidget::isEnabledToTLW() const
{
   return isEnabled();
}

inline int QWidget::minimumWidth() const
{
   return minimumSize().width();
}

inline int QWidget::minimumHeight() const
{
   return minimumSize().height();
}

inline int QWidget::maximumWidth() const
{
   return maximumSize().width();
}

inline int QWidget::maximumHeight() const
{
   return maximumSize().height();
}

inline void QWidget::setMinimumSize(const QSize &s)
{
   setMinimumSize(s.width(), s.height());
}

inline void QWidget::setMaximumSize(const QSize &s)
{
   setMaximumSize(s.width(), s.height());
}

inline void QWidget::setSizeIncrement(const QSize &s)
{
   setSizeIncrement(s.width(), s.height());
}

inline void QWidget::setBaseSize(const QSize &s)
{
   setBaseSize(s.width(), s.height());
}

inline const QFont &QWidget::font() const
{
   return data->fnt;
}

inline QFontMetrics QWidget::fontMetrics() const
{
   return QFontMetrics(data->fnt);
}

inline QFontInfo QWidget::fontInfo() const
{
   return QFontInfo(data->fnt);
}

inline void QWidget::setMouseTracking(bool enable)
{
   setAttribute(Qt::WA_MouseTracking, enable);
}

inline bool QWidget::hasMouseTracking() const
{
   return testAttribute(Qt::WA_MouseTracking);
}

inline bool QWidget::underMouse() const
{
   return testAttribute(Qt::WA_UnderMouse);
}

inline bool QWidget::updatesEnabled() const
{
   return !testAttribute(Qt::WA_UpdatesDisabled);
}

inline void QWidget::update(int ax, int ay, int aw, int ah)
{
   update(QRect(ax, ay, aw, ah));
}

inline bool QWidget::isVisible() const
{
   return testAttribute(Qt::WA_WState_Visible);
}

inline bool QWidget::isHidden() const
{
   return testAttribute(Qt::WA_WState_Hidden);
}

inline void QWidget::move(int ax, int ay)
{
   move(QPoint(ax, ay));
}

inline void QWidget::resize(int w, int h)
{
   resize(QSize(w, h));
}

inline void QWidget::setGeometry(int ax, int ay, int aw, int ah)
{
   setGeometry(QRect(ax, ay, aw, ah));
}

inline QRect QWidget::rect() const
{
   return QRect(0, 0, data->crect.width(), data->crect.height());
}

inline const QRect &QWidget::geometry() const
{
   return data->crect;
}

inline QSize QWidget::size() const
{
   return data->crect.size();
}

inline int QWidget::width() const
{
   return data->crect.width();
}

inline int QWidget::height() const
{
   return data->crect.height();
}

inline QWidget *QWidget::parentWidget() const
{
   return static_cast<QWidget *>(QObject::parent());
}

inline void QWidget::setSizePolicy(QSizePolicy::Policy hor, QSizePolicy::Policy ver)
{
   setSizePolicy(QSizePolicy(hor, ver));
}

inline bool QWidget::testAttribute(Qt::WidgetAttribute attribute) const
{
   if (attribute < int(8 * sizeof(uint))) {
      return data->widget_attributes & (1 << attribute);
   }
   return testAttribute_helper(attribute);
}

void QWidget::cs_setMinimumSize(const QSize &size)
{
   setMinimumSize(size);
}

void QWidget::cs_setMaximumSize(const QSize &size)
{
   setMaximumSize(size);
}

void QWidget::cs_setSizePolicy(const QSizePolicy sizePolicy)
{
   return setSizePolicy(sizePolicy);
}

void QWidget::cs_setSizeIncrement(const QSize &size)
{
   setSizeIncrement(size);
}

void QWidget::cs_setBaseSize(const QSize &size)
{
   setBaseSize(size);
}

void QWidget::cs_move(const QPoint &point)
{
   return move(point);
}

void QWidget::cs_resize(const QSize &size)
{
   return resize(size);
}

void QWidget::cs_setGeometry(const QRect &rect)
{
   return setGeometry(rect);
}

#define QWIDGETSIZE_MAX ((1<<24)-1)



#endif
