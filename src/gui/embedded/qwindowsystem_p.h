/***********************************************************************
*
* Copyright (c) 2012-2019 Barbara Geller
* Copyright (c) 2012-2019 Ansel Sermersheim
*
* Copyright (C) 2015 The Qt Company Ltd.
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

#ifndef QWINDOWSYSTEM_P_H
#define QWINDOWSYSTEM_P_H

#include <qwindowsystem_qws.h>
#include <qbrush.h>
#include <qwsproperty_qws.h>
#include <qwscommand_qws_p.h>
#include <QtCore/qbasictimer.h>

QT_BEGIN_NAMESPACE

class QWSServerPrivate {
   friend class QCopChannel;
   friend class QWSMouseHandler;
   friend class QWSWindow;
   friend class QWSDisplay;
   friend class QWSInputMethod;

   Q_DECLARE_PUBLIC(QWSServer)

 public:
   QWSServerPrivate()
      : screensaverintervals(0)
      , screensavereventblocklevel(-1), screensaverblockevents(false)
      , saver(0), cursorClient(0), mouseState(0), nReserved(0)
      , doClientIsActive(false)
   {
   }

   virtual ~QWSServerPrivate()
   {
      closeDisplay();
   
      qDeleteAll(deletedWindows);
      delete [] screensaverintervals;
      delete saver;
   
      qDeleteAll(windows);
      windows.clear();
   
      delete bgBrush;
      bgBrush = 0;
   }

   QTime screensavertime;
   QTimer *screensavertimer;
   int *screensaverintervals;
   int screensavereventblocklevel;
   bool screensaverblockevents;
   bool screensaverblockevent( int index, int *screensaverinterval, bool isDown );
   QWSScreenSaver *saver;
   QWSClient *cursorClient;
   int mouseState;

   //    bool prevWin;
   QList<QWSWindow *> deletedWindows;
   QList<int> crashedClientIds;
   
   void update_regions();
      
   private:
   void initServer(int flags);

   #ifndef QT_NO_COP
   static void sendQCopEvent(QWSClient *c, const QString &ch, const QString &msg, const QByteArray &data, bool response = false);
#endif

   void move_region(const QWSRegionMoveCommand *);

   void set_altitude(const QWSChangeAltitudeCommand *);
   void set_opacity(const QWSSetOpacityCommand *);
   void request_focus(const QWSRequestFocusCommand *);
   QRegion reserve_region(QWSWindow *window, const QRegion &region);

   void request_region(int winId, const QString &surfaceKey, const QByteArray &surfaceData, const QRegion &region);
   void repaint_region(int winId, int windowFlags, bool opaque, const QRegion &);
   void destroy_region(const QWSRegionDestroyCommand *);
   void name_region(const QWSRegionNameCommand *);
   void set_identity(const QWSIdentifyCommand *);

#ifndef QT_NO_QWS_PROPERTIES
   bool get_property(int winId, int property, const char *&data, int &len);
#endif
   
#ifndef QT_NO_QWS_INPUTMETHODS
   void im_response(const QWSIMResponseCommand *);
   
   void im_update(const QWSIMUpdateCommand *);
   
   void send_im_mouse(const QWSIMMouseCommand *);
#endif
   
   // not in ifndef as this results in more readable functions.
   static void sendKeyEventUnfiltered(int unicode, int keycode, Qt::KeyboardModifiers modifiers,
                                      bool isPress, bool autoRepeat);

   static void sendMouseEventUnfiltered(const QPoint &pos, int state, int wheel = 0);
   static void emergency_cleanup();
   
   static QBrush *bgBrush;
   
   void sendMaxWindowRectEvents(const QRect &rect);
   
   void invokeIdentify(const QWSIdentifyCommand *cmd, QWSClient *client);
   void invokeCreate(QWSCreateCommand *cmd, QWSClient *client);
   void invokeRegionName(const QWSRegionNameCommand *cmd, QWSClient *client);
   void invokeRegion(QWSRegionCommand *cmd, QWSClient *client);
   void invokeRegionMove(const QWSRegionMoveCommand *cmd, QWSClient *client);
   void invokeRegionDestroy(const QWSRegionDestroyCommand *cmd, QWSClient *client);
   void invokeSetAltitude(const QWSChangeAltitudeCommand *cmd, QWSClient *client);
   void invokeSetOpacity(const QWSSetOpacityCommand *cmd, QWSClient *client);

#ifndef QT_NO_QWS_PROPERTIES
   void invokeAddProperty(QWSAddPropertyCommand *cmd);
   void invokeSetProperty(QWSSetPropertyCommand *cmd);
   void invokeRemoveProperty(QWSRemovePropertyCommand *cmd);
   void invokeGetProperty(QWSGetPropertyCommand *cmd, QWSClient *client);
#endif

   void invokeSetSelectionOwner(QWSSetSelectionOwnerCommand *cmd);
   void invokeConvertSelection(QWSConvertSelectionCommand *cmd);
   void invokeSetFocus(const QWSRequestFocusCommand *cmd, QWSClient *client);
   
   void initIO();
   void setFocus(QWSWindow *, bool gain);

#ifndef QT_NO_QWS_CURSOR
   void invokeDefineCursor(QWSDefineCursorCommand *cmd, QWSClient *client);
   void invokeSelectCursor(QWSSelectCursorCommand *cmd, QWSClient *client);
   void invokePositionCursor(QWSPositionCursorCommand *cmd, QWSClient *client);
#endif

   void invokeGrabMouse(QWSGrabMouseCommand *cmd, QWSClient *client);
   void invokeGrabKeyboard(QWSGrabKeyboardCommand *cmd, QWSClient *client);

#ifndef QT_NO_SOUND
   void invokePlaySound(QWSPlaySoundCommand *cmd, QWSClient *client);
#endif

#ifndef QT_NO_COP
   void invokeRegisterChannel(QWSQCopRegisterChannelCommand *cmd, QWSClient *client);
   void invokeQCopSend(QWSQCopSendCommand *cmd, QWSClient *client);
#endif

   void invokeRepaintRegion(QWSRepaintRegionCommand *cmd, QWSClient *client);

#ifndef QT_NO_QWSEMBEDWIDGET
   void invokeEmbed(QWSEmbedCommand *cmd, QWSClient *client);
#endif

#ifndef QT_NO_QWS_INPUTMETHODS
   void invokeIMResponse(const QWSIMResponseCommand *cmd, QWSClient *client);
   void invokeIMUpdate(const QWSIMUpdateCommand *cmd, QWSClient *client);
#endif

   void invokeFont(const QWSFontCommand *cmd, QWSClient *client);
   void invokeScreenTransform(const QWSScreenTransformCommand *cmd, QWSClient *client);
   
   QWSMouseHandler *newMouseHandler(const QString &spec);
   void openDisplay();
   void closeDisplay();
   
   void showCursor();
   void hideCursor();
   void initializeCursor();
   
   void resetEngine();
       
#ifndef QT_NO_QWS_MULTIPROCESS
   void _q_clientClosed();
   void _q_doClient();
   void _q_deleteWindowsLater();
#endif
   
   void _q_screenSaverWake();
   void _q_screenSaverSleep();
   void _q_screenSaverTimeout();
#ifndef QT_NO_QWS_MULTIPROCESS
   void _q_newConnection();
#endif
   
   //other private moved from class   
   void disconnectClient(QWSClient *);
   void screenSave(int level);
   void doClient(QWSClient *);
   typedef QMap<int, QWSClient *>::iterator ClientIterator;
   typedef QMap<int, QWSClient *> ClientMap;
   void handleWindowClose(QWSWindow *w);
   void releaseMouse(QWSWindow *w);
   void releaseKeyboard(QWSWindow *w);
   void updateClientCursorPos();
   
   uchar *sharedram;
   int ramlen;
   
   ClientMap clientMap;

#ifndef QT_NO_QWS_PROPERTIES
   QWSPropertyManager propertyManager;
#endif

   struct SelectionOwner {
      int windowid;
      struct Time {
         void set(int h, int m, int s, int s2) {
            hour = h;
            minute = m;
            sec = s;
            ms = s2;
         }
         int hour, minute, sec, ms;
      } time;
   } selectionOwner;
   QTime timer;
   int *screensaverinterval;
   
   QWSWindow *focusw;
   QWSWindow *mouseGrabber;
   bool mouseGrabbing;
   bool inputMethodMouseGrabbed;
   int swidth, sheight, sdepth;

#ifndef QT_NO_QWS_CURSOR
   bool haveviscurs;
   QWSCursor *cursor;      // cursor currently shown
   QWSCursor *nextCursor;  // cursor to show once grabbing is off
#endif
   
   bool disablePainting;
   QList<QWSMouseHandler *> mousehandlers;
#ifndef QT_NO_QWS_KEYBOARD
   QList<QWSKeyboardHandler *> keyboardhandlers;
#endif
   
   QList<QWSCommandStruct *> commandQueue;
   
   // Window management
   QList<QWSWindow *> windows; // first=topmost
   int nReserved;
   QWSWindow *newWindow(int id, QWSClient *client);
   QWSWindow *findWindow(int windowid, QWSClient *client = 0);
   void moveWindowRegion(QWSWindow *, int dx, int dy);
   void setWindowRegion(QWSWindow *, const QRegion &r);
   void raiseWindow(QWSWindow *, int = 0);
   void lowerWindow(QWSWindow *, int = -1);
   void exposeRegion(const QRegion &, int index = 0);
   
   void setCursor(QWSCursor *curs);
   
   // multimedia
#ifndef QT_NO_SOUND
   QWSSoundServer *soundserver;
#endif

#ifndef QT_NO_COP
   QMap<QString, QList<QWSClient *> > channels;
#endif
   
#ifndef QT_NO_QWS_MULTIPROCESS
   QWSServerSocket *ssocket;
#endif
   
   // filename -> refcount
   QMap<QByteArray, int> fontReferenceCount;
   QBasicTimer fontCleanupTimer;
   void referenceFont(QWSClientPrivate *client, const QByteArray &font);
   void dereferenceFont(QWSClientPrivate *client, const QByteArray &font);
   void cleanupFonts(bool force = false);
   void sendFontRemovedEvent(const QByteArray &font);
   
   bool doClientIsActive;
   QList<QWSClient *> pendingDoClients;
};

QT_END_NAMESPACE

#endif
