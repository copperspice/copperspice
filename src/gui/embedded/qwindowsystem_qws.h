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

#ifndef QWINDOWSYSTEM_QWS_H
#define QWINDOWSYSTEM_QWS_H

#include <QtCore/qbytearray.h>
#include <QtCore/qmap.h>
#include <QtCore/qdatetime.h>
#include <QtCore/qlist.h>

#include <QtGui/qwsevent_qws.h>
#include <QtGui/qkbd_qws.h>
#include <QtGui/qregion.h>

QT_BEGIN_NAMESPACE

struct QWSWindowPrivate;
class QWSCursor;
class QWSClient;
class QWSRegionManager;
class QBrush;
class QVariant;
class QInputMethodEvent;
class QWSInputMethod;
class QWSBackingStore;
class QWSWindowSurface;

class QWSInternalWindowInfo
{
 public:
   int winid;
   unsigned int clientid;
   QString name;   // Corresponds to QObject name of top-level widget
};


class Q_GUI_EXPORT QWSScreenSaver
{
 public:
   virtual ~QWSScreenSaver();
   virtual void restore() = 0;
   virtual bool save(int level) = 0;
};


class Q_GUI_EXPORT QWSWindow
{
   friend class QWSServer;
   friend class QWSServerPrivate;

 public:
   QWSWindow(int i, QWSClient *client);
   ~QWSWindow();

   int winId() const {
      return id;
   }
   const QString &name() const {
      return rgnName;
   }
   const QString &caption() const {
      return rgnCaption;
   }
   QWSClient *client() const {
      return c;
   }
   const QRegion &requestedRegion() const {
      return requested_region;
   }
   QRegion allocatedRegion() const;
   QRegion paintedRegion() const;
   bool isVisible() const {
      return !requested_region.isEmpty();
   }
   bool isPartiallyObscured() const {
      return requested_region != allocatedRegion();
   }
   bool isFullyObscured() const {
      return allocatedRegion().isEmpty();
   }

   enum State { NoState, Hidden, Showing, Visible, Hiding, Raising, Lowering, Moving, ChangingGeometry, Destroyed };
   State state() const;
   Qt::WindowFlags windowFlags() const;
   QRegion dirtyOnScreen() const;

   void raise();
   void lower();
   void show();
   void hide();
   void setActiveWindow();

   bool isOpaque() const {
      return opaque && _opacity == 255;
   }
   uint opacity() const {
      return _opacity;
   }

   QWSWindowSurface *windowSurface() const {
      return surface;
   }

 private:
   bool hidden() const {
      return requested_region.isEmpty();
   }
   bool forClient(const QWSClient *cl) const {
      return cl == c;
   }

   void setName(const QString &n);
   void setCaption(const QString &c);

   void focus(bool get);
   int focusPriority() const {
      return last_focus_time;
   }
   void operation(QWSWindowOperationEvent::Operation o);
   void shuttingDown() {
      last_focus_time = 0;
   }

#ifdef QT_QWS_CLIENTBLIT
   QRegion directPaintRegion() const;
   inline void setDirectPaintRegion(const QRegion &topmost);
#endif
   inline void setAllocatedRegion(const QRegion &region);

   void createSurface(const QString &key, const QByteArray &data);

#ifndef QT_NO_QWSEMBEDWIDGET
   void startEmbed(QWSWindow *window);
   void stopEmbed(QWSWindow *window);
#endif

 private:
   int id;
   QString rgnName;
   QString rgnCaption;
   bool modified;
   bool onTop;
   QWSClient *c;
   QRegion requested_region;
   QRegion exposed;
   int last_focus_time;
   QWSWindowSurface *surface;
   uint _opacity;
   bool opaque;
   QWSWindowPrivate *d;
};


#ifndef QT_NO_SOUND
class QWSSoundServer;
#ifdef QT_USE_OLD_QWS_SOUND
class QWSSoundServerData;

class Q_GUI_EXPORT QWSSoundServer : public QObject
{
   GUI_CS_OBJECT(QWSSoundServer)
 public:
   QWSSoundServer(QObject *parent);
   ~QWSSoundServer();
   void playFile(const QString &filename);
 private :
   GUI_CS_SLOT_1(Private, void feedDevice(int fd))
   GUI_CS_SLOT_2(feedDevice)
 private:
   QWSSoundServerData *d;
};
#endif
#endif


/*********************************************************************
 *
 * Class: QWSServer
 *
 *********************************************************************/

class QWSMouseHandler;
struct QWSCommandStruct;
class QWSServerPrivate;
class QWSServer;

extern Q_GUI_EXPORT QWSServer *qwsServer;

class Q_GUI_EXPORT QWSServer : public QObject
{
   GUI_CS_OBJECT(QWSServer)
   Q_DECLARE_PRIVATE(QWSServer)

   friend class QCopChannel;
   friend class QWSMouseHandler;
   friend class QWSWindow;
   friend class QWSDisplay;
   friend class QWSInputMethod;

 public:
   explicit QWSServer(int flags = 0, QObject *parent = nullptr);

   ~QWSServer();
   enum ServerFlags { DisableKeyboard = 0x01,
                      DisableMouse = 0x02
                    };

   static void sendKeyEvent(int unicode, int keycode, Qt::KeyboardModifiers modifiers,
                            bool isPress, bool autoRepeat);
#ifndef QT_NO_QWS_KEYBOARD
   static void processKeyEvent(int unicode, int keycode, Qt::KeyboardModifiers modifiers,
                               bool isPress, bool autoRepeat);
#endif

   static QWSServer *instance() {
      return qwsServer;
   }

#ifndef QT_NO_QWS_INPUTMETHODS
   enum IMMouse { MousePress, MouseRelease, MouseMove, MouseOutside }; //MouseMove reserved but not used
   void sendIMEvent(const QInputMethodEvent *);
   void sendIMQuery(int property);
#endif

#ifndef QT_NO_QWS_KEYBOARD
   class KeyboardFilter
   {
    public:
      virtual ~KeyboardFilter() {}
      virtual bool filter(int unicode, int keycode, int modifiers,
                          bool isPress, bool autoRepeat) = 0;
   };
   static void addKeyboardFilter(KeyboardFilter *f);
   static void removeKeyboardFilter();
#endif

#ifndef QT_NO_QWS_INPUTMETHODS
   static void setCurrentInputMethod(QWSInputMethod *im);
   static void resetInputMethod();
#endif

   static void setDefaultMouse(const char *);
   static void setDefaultKeyboard(const char *);
   static void setMaxWindowRect(const QRect &);
   static void sendMouseEvent(const QPoint &pos, int state, int wheel = 0);

   static void setBackground(const QBrush &);
   static QWSMouseHandler *mouseHandler();
   static const QList<QWSMouseHandler *> &mouseHandlers();
   static void setMouseHandler(QWSMouseHandler *);

#ifndef QT_NO_QWS_KEYBOARD
   static QWSKeyboardHandler *keyboardHandler();
   static void setKeyboardHandler(QWSKeyboardHandler *kh);
#endif

   QWSWindow *windowAt(const QPoint &pos);

   const QList<QWSWindow *> &clientWindows();

   void openMouse();
   void closeMouse();
   void suspendMouse();
   void resumeMouse();

#ifndef QT_NO_QWS_KEYBOARD
   void openKeyboard();
   void closeKeyboard();
#endif

   static void setScreenSaver(QWSScreenSaver *);
   static void setScreenSaverIntervals(int *ms);
   static void setScreenSaverInterval(int);
   static void setScreenSaverBlockLevel(int);
   static bool screenSaverActive();
   static void screenSaverActivate(bool);

   // the following are internal.
   void refresh();
   void refresh(QRegion &);

   void enablePainting(bool);
   static void processEventQueue();
   static QList<QWSInternalWindowInfo *> *windowList();

   void sendPropertyNotifyEvent(int property, int state);

   static QPoint mousePosition;

   static void startup(int flags);
   static void closedown();

   static void beginDisplayReconfigure();
   static void endDisplayReconfigure();

#ifndef QT_NO_QWS_CURSOR
   static void setCursorVisible(bool);
   static bool isCursorVisible();
#endif

   const QBrush &backgroundBrush() const;

   enum WindowEvent { Create = 0x0001, Destroy = 0x0002, Hide = 0x0004, Show = 0x0008,
                      Raise = 0x0010, Lower = 0x0020, Geometry = 0x0040, Active = 0x0080,
                      Name = 0x0100
                    };

 public:
   GUI_CS_SIGNAL_1(Public, void windowEvent(QWSWindow *w, QWSServer::WindowEvent e))
   GUI_CS_SIGNAL_2(windowEvent, w, e)

#ifndef QT_NO_COP
   GUI_CS_SIGNAL_1(Public, void newChannel(const QString &channel))
   GUI_CS_SIGNAL_2(newChannel, channel)
   GUI_CS_SIGNAL_1(Public, void removedChannel(const QString &channel))
   GUI_CS_SIGNAL_2(removedChannel, channel)

#endif

#ifndef QT_NO_QWS_INPUTMETHODS
   GUI_CS_SIGNAL_1(Public, void markedText(const QString &un_named_arg1))
   GUI_CS_SIGNAL_2(markedText, un_named_arg1)
#endif

 protected:
   void timerEvent(QTimerEvent *e);

 private:
   friend class QApplicationPrivate;
   void updateWindowRegions() const;

#ifndef QT_NO_QWS_MULTIPROCESS
   GUI_CS_SLOT_1(Private, void _q_clientClosed())
   GUI_CS_SLOT_2(_q_clientClosed)

   GUI_CS_SLOT_1(Private, void _q_doClient())
   GUI_CS_SLOT_2(_q_doClient)

   GUI_CS_SLOT_1(Private, void _q_deleteWindowsLater())
   GUI_CS_SLOT_2(_q_deleteWindowsLater)

#endif

   GUI_CS_SLOT_1(Private, void _q_screenSaverWake())
   GUI_CS_SLOT_2(_q_screenSaverWake)

   GUI_CS_SLOT_1(Private, void _q_screenSaverSleep())
   GUI_CS_SLOT_2(_q_screenSaverSleep)

   GUI_CS_SLOT_1(Private, void _q_screenSaverTimeout())
   GUI_CS_SLOT_2(_q_screenSaverTimeout)

#ifndef QT_NO_QWS_MULTIPROCESS
   GUI_CS_SLOT_1(Private, void _q_newConnection())
   GUI_CS_SLOT_2(_q_newConnection)

#endif

};

#ifndef QT_NO_QWS_INPUTMETHODS
class Q_GUI_EXPORT QWSInputMethod : public QObject
{
   GUI_CS_OBJECT(QWSInputMethod)

 public:
   QWSInputMethod();
   virtual ~QWSInputMethod();

   enum UpdateType {Update, FocusIn, FocusOut, Reset, Destroyed};

   virtual bool filter(int unicode, int keycode, int modifiers,
                       bool isPress, bool autoRepeat);

   virtual bool filter(const QPoint &, int state, int wheel);

   virtual void reset();
   virtual void updateHandler(int type);
   virtual void mouseHandler(int pos, int state);
   virtual void queryResponse(int property, const QVariant &);

 protected:
   uint setInputResolution(bool isHigh);
   uint inputResolutionShift() const;
   // needed for required transform
   void sendMouseEvent(const QPoint &pos, int state, int wheel);

   void sendEvent(const QInputMethodEvent *);
   void sendPreeditString(const QString &preeditString, int cursorPosition, int selectionLength = 0);
   void sendCommitString(const QString &commitString, int replaceFrom = 0, int replaceLength = 0);
   void sendQuery(int property);

 private:
   bool mIResolution;
};

inline void QWSInputMethod::sendEvent(const QInputMethodEvent *ime)
{
   qwsServer->sendIMEvent(ime);
}

inline void QWSInputMethod::sendQuery(int property)
{
   qwsServer->sendIMQuery(property);
}

// mouse events not inline as involve transformations.
#endif // QT_NO_QWS_INPUTMETHODS



/*********************************************************************
 *
 * Class: QWSClient
 *
 *********************************************************************/

struct QWSMouseEvent;

typedef QMap<int, QWSCursor *> QWSCursorMap;

class QWSClientPrivate;
class QWSCommand;
class QWSConvertSelectionCommand;

class Q_GUI_EXPORT QWSClient : public QObject
{
   GUI_CS_OBJECT(QWSClient)
   Q_DECLARE_PRIVATE(QWSClient)

 public:
   QWSClient(QObject *parent, QWS_SOCK_BASE *, int id);
   ~QWSClient();

   int socket() const;

   void setIdentity(const QString &);
   QString identity() const {
      return id;
   }

   void sendEvent(QWSEvent *event);
   void sendConnectedEvent(const char *display_spec);
   void sendMaxWindowRectEvent(const QRect &rect);
   void sendPropertyNotifyEvent(int property, int state);
   void sendPropertyReplyEvent(int property, int len, const char *data);
   void sendSelectionClearEvent(int windowid);
   void sendSelectionRequestEvent(QWSConvertSelectionCommand *cmd, int windowid);

#ifndef QT_QWS_CLIENTBLIT
   void sendRegionEvent(int winid, QRegion rgn, int type);
#else
   void sendRegionEvent(int winid, QRegion rgn, int type, int id = 0);
#endif

#ifndef QT_NO_QWSEMBEDWIDGET
   void sendEmbedEvent(int winid, QWSEmbedEvent::Type type, const QRegion &region = QRegion());
#endif

   QWSCommand *readMoreCommand();

   int clientId() const {
      return cid;
   }

   QWSCursorMap cursors; // cursors defined by this client

 public:
   GUI_CS_SIGNAL_1(Public, void connectionClosed())
   GUI_CS_SIGNAL_2(connectionClosed)
   GUI_CS_SIGNAL_1(Public, void readyRead())
   GUI_CS_SIGNAL_2(readyRead)

 private :
   GUI_CS_SLOT_1(Private, void closeHandler())
   GUI_CS_SLOT_2(closeHandler)
   GUI_CS_SLOT_1(Private, void errorHandler())
   GUI_CS_SLOT_2(errorHandler)

#ifndef QT_NO_QWS_MULTIPROCESS
   friend class QWSWindow;
   void removeUnbufferedSurface();
   void addUnbufferedSurface();
#endif

 private:
   int socketDescriptor;

#ifndef QT_NO_QWS_MULTIPROCESS
   QWSSocket *csocket;
#endif

   QWSCommand *command;
   uint isClosed : 1;
   QString id;
   int cid;

   friend class QWSServerPrivate;
};

QT_END_NAMESPACE

#endif // QWINDOWSYSTEM_QWS_H
