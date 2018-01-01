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

#ifndef QAPPLICATION_H
#define QAPPLICATION_H

#if defined(Q_OS_MAC)
#include <cs_carbon_wrapper.h>
#endif

#include <QtCore/qcoreapplication.h>
#include <QtGui/qwindowdefs.h>
#include <QtCore/qpoint.h>
#include <QtCore/qsize.h>
#include <QtGui/qcursor.h>

#ifdef Q_WS_QWS
# include <QtGui/qrgb.h>
# include <QtGui/qtransportauth_qws.h>
#endif

#include <QSessionManager>
#include <QIcon>

QT_BEGIN_NAMESPACE

class QDesktopWidget;
class QStyle;
class QEventLoop;

#ifndef QT_NO_IM
class QInputContext;
#endif

template <typename T> class QList;
class QLocale;

#if defined(Q_WS_QWS)
class QDecoration;
#elif defined(Q_WS_QPA)
class QPlatformNativeInterface;
#endif

class QApplication;
class QApplicationPrivate;

#if defined(qApp)
#undef qApp
#endif

#define qApp (static_cast<QApplication *>(QCoreApplication::instance()))

class Q_GUI_EXPORT QApplication : public QCoreApplication
{
   GUI_CS_OBJECT(QApplication)

   GUI_CS_PROPERTY_READ(layoutDirection, cs_layoutDirection)
   GUI_CS_PROPERTY_WRITE(layoutDirection, cs_setLayoutDirection)

   GUI_CS_PROPERTY_READ(windowIcon, cs_windowIcon)
   GUI_CS_PROPERTY_WRITE(windowIcon, cs_setWindowIcon)

   GUI_CS_PROPERTY_READ(cursorFlashTime, cs_cursorFlashTime)
   GUI_CS_PROPERTY_WRITE(cursorFlashTime, cs_setCursorFlashTime)

   GUI_CS_PROPERTY_READ(doubleClickInterval, cs_doubleClickInterval)
   GUI_CS_PROPERTY_WRITE(doubleClickInterval, cs_setDoubleClickInterval)

   GUI_CS_PROPERTY_READ(keyboardInputInterval, cs_keyboardInputInterval)
   GUI_CS_PROPERTY_WRITE(keyboardInputInterval, cs_setKeyboardInputInterval)

#ifndef QT_NO_WHEELEVENT
   GUI_CS_PROPERTY_READ(wheelScrollLines, cs_wheelScrollLines)
   GUI_CS_PROPERTY_WRITE(wheelScrollLines, cs_setWheelScrollLines)
#endif

   GUI_CS_PROPERTY_READ(globalStrut, cs_globalStrut)
   GUI_CS_PROPERTY_WRITE(globalStrut, cs_setGlobalStrut)

   GUI_CS_PROPERTY_READ(startDragTime, cs_startDragTime)
   GUI_CS_PROPERTY_WRITE(startDragTime, cs_setStartDragTime)

   GUI_CS_PROPERTY_READ(startDragDistance, cs_startDragDistance)
   GUI_CS_PROPERTY_WRITE(startDragDistance, cs_setStartDragDistance)

   GUI_CS_PROPERTY_READ(quitOnLastWindowClosed, cs_quitOnLastWindowClosed)
   GUI_CS_PROPERTY_WRITE(quitOnLastWindowClosed, cs_setQuitOnLastWindowClosed)

#ifndef QT_NO_STYLE_STYLESHEET
   GUI_CS_PROPERTY_READ(styleSheet, styleSheet)
   GUI_CS_PROPERTY_WRITE(styleSheet, setStyleSheet)
#endif

   GUI_CS_PROPERTY_READ(autoSipEnabled, autoSipEnabled)
   GUI_CS_PROPERTY_WRITE(autoSipEnabled, setAutoSipEnabled)

 public:
   enum Type { Tty, GuiClient, GuiServer };

   QApplication(int &argc, char **argv, int = ApplicationFlags);
   QApplication(int &argc, char **argv, bool GUIenabled, int = ApplicationFlags);
   QApplication(int &argc, char **argv, Type, int = ApplicationFlags);

#if defined(Q_WS_X11)
   QApplication(Display *dpy, Qt::HANDLE visual = 0, Qt::HANDLE cmap = 0, int = ApplicationFlags);
   QApplication(Display *dpy, int &argc, char **argv, Qt::HANDLE visual = 0, Qt::HANDLE cmap = 0, int = ApplicationFlags);
#endif

   virtual ~QApplication();

   static Type type();

   static QStyle *style();
   static void setStyle(QStyle *);
   static QStyle *setStyle(const QString &);
   enum ColorSpec { NormalColor = 0, CustomColor = 1, ManyColor = 2 };
   static int colorSpec();
   static void setColorSpec(int);
   static void setGraphicsSystem(const QString &);

#ifndef QT_NO_CURSOR
   static QCursor *overrideCursor();
   static void setOverrideCursor(const QCursor &);
   static void changeOverrideCursor(const QCursor &);
   static void restoreOverrideCursor();
#endif
   static QPalette palette();
   static QPalette palette(const QWidget *);
   static QPalette palette(const char *className);
   static void setPalette(const QPalette &, const char *className = 0);
   static QFont font();
   static QFont font(const QWidget *);
   static QFont font(const char *className);
   static void setFont(const QFont &, const char *className = 0);
   static QFontMetrics fontMetrics();

   static void setWindowIcon(const QIcon &icon);
   static QIcon windowIcon();

   // wrapper for static method
   inline void cs_setWindowIcon(const QIcon &icon);

   // wrapper for static method
   inline QIcon cs_windowIcon() const;

   static QWidgetList allWidgets();
   static QWidgetList topLevelWidgets();

   static QDesktopWidget *desktop();

   static QWidget *activePopupWidget();
   static QWidget *activeModalWidget();

#ifndef QT_NO_CLIPBOARD
   static QClipboard *clipboard();
#endif

   static QWidget *focusWidget();

   static QWidget *activeWindow();
   static void setActiveWindow(QWidget *act);

   static QWidget *widgetAt(const QPoint &p);
   static inline QWidget *widgetAt(int x, int y) {
      return widgetAt(QPoint(x, y));
   }
   static QWidget *topLevelAt(const QPoint &p);
   static inline QWidget *topLevelAt(int x, int y)  {
      return topLevelAt(QPoint(x, y));
   }

   static void syncX();
   static void beep();
   static void alert(QWidget *widget, int duration = 0);

   static Qt::KeyboardModifiers keyboardModifiers();
   static Qt::KeyboardModifiers queryKeyboardModifiers();
   static Qt::MouseButtons mouseButtons();

   static void setDesktopSettingsAware(bool);
   static bool desktopSettingsAware();

   static void setCursorFlashTime(int);
   static int cursorFlashTime();

   // wrapper for static method
   inline void cs_setCursorFlashTime(int un_named_arg1);

   // wrapper for static method
   inline int cs_cursorFlashTime() const;

   static void setDoubleClickInterval(int);
   static int doubleClickInterval();

   // wrapper for static method
   inline void cs_setDoubleClickInterval(int un_named_arg1);

   // wrapper for static method
   inline int cs_doubleClickInterval() const;

   static void setKeyboardInputInterval(int);
   static int keyboardInputInterval();

   // wrapper for static method
   inline void cs_setKeyboardInputInterval(int un_named_arg1);

   // wrapper for static method
   inline int cs_keyboardInputInterval() const;

#ifndef QT_NO_WHEELEVENT
   static void setWheelScrollLines(int);
   static int wheelScrollLines();

   // wrapper for overloaded property
   inline void cs_setWheelScrollLines(int un_named_arg1);

   // wrapper for overloaded property
   inline int cs_wheelScrollLines()  const;
#endif

   static void setGlobalStrut(const QSize &);
   static QSize globalStrut();

   // wrapper for static method
   inline void cs_setGlobalStrut(const QSize &un_named_arg1);

   // wrapper for static method
   inline QSize cs_globalStrut() const;

   static void setStartDragTime(int ms);
   static int startDragTime();

   // wrapper for static method
   inline void cs_setStartDragTime(int ms);

   // wrapper for static method
   inline int cs_startDragTime() const;

   static void setStartDragDistance(int l);
   static int startDragDistance();

   // wrapper for static method
   inline void cs_setStartDragDistance(int l);

   // wrapper for static method
   inline int cs_startDragDistance() const;

   static void setLayoutDirection(Qt::LayoutDirection direction);
   static Qt::LayoutDirection layoutDirection();

   // wrapper for static method
   inline void cs_setLayoutDirection(Qt::LayoutDirection direction);

   // wrapper for static method
   inline Qt::LayoutDirection cs_layoutDirection() const;

   static inline bool isRightToLeft() {
      return layoutDirection() == Qt::RightToLeft;
   }

   static inline bool isLeftToRight() {
      return layoutDirection() == Qt::LeftToRight;
   }

   static bool isEffectEnabled(Qt::UIEffect);
   static void setEffectEnabled(Qt::UIEffect, bool enable = true);

#if defined(Q_OS_MAC)
   virtual bool macEventFilter(EventHandlerCallRef, EventRef);
#endif

#if defined(Q_WS_X11)
   virtual bool x11EventFilter(XEvent *);
   virtual int x11ClientMessage(QWidget *, XEvent *, bool passive_only);
   int x11ProcessEvent(XEvent *);
#endif

#if defined(Q_WS_QWS)
   virtual bool qwsEventFilter(QWSEvent *);
   int qwsProcessEvent(QWSEvent *);
   void qwsSetCustomColors(QRgb *colortable, int start, int numColors);
#ifndef QT_NO_QWS_MANAGER
   static QDecoration &qwsDecoration();
   static void qwsSetDecoration(QDecoration *);
   static QDecoration *qwsSetDecoration(const QString &decoration);
#endif
#endif

#if defined(Q_WS_QPA)
   static QPlatformNativeInterface *platformNativeInterface();
#endif


#if defined(Q_OS_WIN)
   void winFocus(QWidget *, bool);
   static void winMouseButtonUp();
#endif

#ifndef QT_NO_SESSIONMANAGER
   // session management
   bool isSessionRestored() const;
   QString sessionId() const;
   QString sessionKey() const;
   virtual void commitData(QSessionManager &sm);
   virtual void saveState(QSessionManager &sm);
#endif

#ifndef QT_NO_IM
   void setInputContext(QInputContext *);
   QInputContext *inputContext() const;
#endif

   static QLocale keyboardInputLocale();
   static Qt::LayoutDirection keyboardInputDirection();

   static int exec();
   bool notify(QObject *, QEvent *) override;

   static void setQuitOnLastWindowClosed(bool quit);
   static bool quitOnLastWindowClosed();

   // wrapper for static method
   inline void cs_setQuitOnLastWindowClosed(bool quit);

   // wrapper for static method
   inline bool cs_quitOnLastWindowClosed() const;

#ifdef QT_KEYPAD_NAVIGATION
   static void setNavigationMode(Qt::NavigationMode mode);
   static Qt::NavigationMode navigationMode();
#endif

   GUI_CS_SIGNAL_1(Public, void lastWindowClosed())
   GUI_CS_SIGNAL_2(lastWindowClosed)
   GUI_CS_SIGNAL_1(Public, void focusChanged(QWidget *old, QWidget *now))
   GUI_CS_SIGNAL_2(focusChanged, old, now)
   GUI_CS_SIGNAL_1(Public, void fontDatabaseChanged())
   GUI_CS_SIGNAL_2(fontDatabaseChanged)

#ifndef QT_NO_SESSIONMANAGER
   // CopperSpice - api change to pass a pointer instead of a reference
   GUI_CS_SIGNAL_1(Public, void commitDataRequest(QSessionManager *sessionManager))
   GUI_CS_SIGNAL_2(commitDataRequest, sessionManager)

   // CopperSpice - api change to pass a pointer instead of a reference
   GUI_CS_SIGNAL_1(Public, void saveStateRequest(QSessionManager *sessionManager))
   GUI_CS_SIGNAL_2(saveStateRequest, sessionManager)
#endif

   QString styleSheet() const;

#ifndef QT_NO_STYLE_STYLESHEET
   GUI_CS_SLOT_1(Public, void setStyleSheet(const QString &sheet))
   GUI_CS_SLOT_2(setStyleSheet)
#endif

   GUI_CS_SLOT_1(Public, void setAutoSipEnabled(const bool enabled))
   GUI_CS_SLOT_2(setAutoSipEnabled)

   GUI_CS_SLOT_1(Public, bool autoSipEnabled() const)
   GUI_CS_SLOT_2(autoSipEnabled)

   GUI_CS_SLOT_1(Public, static void closeAllWindows())
   GUI_CS_SLOT_2(closeAllWindows)

   GUI_CS_SLOT_1(Public, static void aboutCs())
   GUI_CS_SLOT_2(aboutCs)

   GUI_CS_SLOT_1(Public, static void aboutQt())
   GUI_CS_SLOT_2(aboutQt)

 protected:

#if defined(Q_WS_QWS)
   void setArgs(int, char **);
#endif

   bool event(QEvent *) override;
   bool compressEvent(QEvent *, QObject *receiver, QPostEventList *) override;

 private:
   Q_DISABLE_COPY(QApplication)
   Q_DECLARE_PRIVATE(QApplication)

   friend class QGraphicsWidget;
   friend class QGraphicsItem;
   friend class QGraphicsScene;
   friend class QGraphicsScenePrivate;
   friend class QWidget;
   friend class QWidgetPrivate;
   friend class QETWidget;
   friend class QTranslator;
   friend class QWidgetAnimator;

#ifndef QT_NO_SHORTCUT
   friend class QShortcut;
   friend class QLineEdit;
   friend class QTextControl;
#endif

   friend class QAction;
   friend class QFontDatabasePrivate;

#if defined(Q_WS_QWS)
   friend class QInputContext;
   friend class QWSDirectPainterSurface;
   friend class QDirectPainter;
   friend class QDirectPainterPrivate;
#endif

#ifndef QT_NO_GESTURES
   friend class QGestureManager;
#endif

#if defined(Q_OS_MAC) || defined(Q_WS_X11)
   GUI_CS_SLOT_1(Private, void _q_alertTimeOut())
   GUI_CS_SLOT_2(_q_alertTimeOut)
#endif

#if defined(QT_RX71_MULTITOUCH)
   GUI_CS_SLOT_1(Private, void _q_readRX71MultiTouchEvents())
   GUI_CS_SLOT_2(_q_readRX71MultiTouchEvents)
#endif

};

void QApplication::cs_setWindowIcon(const QIcon &icon)
{
   setWindowIcon(icon);
}

QIcon QApplication::cs_windowIcon() const
{
   return windowIcon();
}

void QApplication::cs_setCursorFlashTime(int un_named_arg1)
{
   setCursorFlashTime(un_named_arg1);
}

int QApplication::cs_cursorFlashTime() const
{
   return cursorFlashTime();
}

void QApplication::cs_setDoubleClickInterval(int un_named_arg1)
{
   setDoubleClickInterval(un_named_arg1);
}

int QApplication::cs_doubleClickInterval() const
{
   return doubleClickInterval();
}

void QApplication::cs_setKeyboardInputInterval(int un_named_arg1)
{
   setKeyboardInputInterval(un_named_arg1);
}

int QApplication::cs_keyboardInputInterval() const
{
   return keyboardInputInterval();
}

#ifndef QT_NO_WHEELEVENT
void QApplication::cs_setWheelScrollLines(int un_named_arg1)
{
   setWheelScrollLines(un_named_arg1);
}

int QApplication::cs_wheelScrollLines()  const
{
   return wheelScrollLines();
}
#endif

void QApplication::cs_setGlobalStrut(const QSize &un_named_arg1)
{
   setGlobalStrut(un_named_arg1);
}

QSize QApplication::cs_globalStrut() const
{
   return globalStrut();
}

void QApplication::cs_setStartDragTime(int ms)
{
   setStartDragTime(ms);
}

int QApplication::cs_startDragTime() const
{
   return startDragTime();
}

void QApplication::cs_setStartDragDistance(int l)
{
   setStartDragDistance(l);
}

int QApplication::cs_startDragDistance() const
{
   return startDragDistance();
}

void QApplication::cs_setLayoutDirection(Qt::LayoutDirection direction)
{
   setLayoutDirection(direction);
}

void QApplication::cs_setQuitOnLastWindowClosed(bool quit)
{
   setQuitOnLastWindowClosed(quit);
}

bool QApplication::cs_quitOnLastWindowClosed() const
{
   return quitOnLastWindowClosed();
}

Qt::LayoutDirection QApplication::cs_layoutDirection() const
{
   return layoutDirection();
}


QT_END_NAMESPACE

#endif // QAPPLICATION_H
