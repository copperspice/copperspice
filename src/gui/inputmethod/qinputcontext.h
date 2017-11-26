/***********************************************************************
*
* Copyright (c) 2012-2017 Barbara Geller
* Copyright (c) 2012-2017 Ansel Sermersheim
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

#ifndef QINPUTCONTEXT_H
#define QINPUTCONTEXT_H

#include <qobject.h>
#include <qglobal.h>
#include <qevent.h>
#include <qstring.h>
#include <qlist.h>
#include <qaction.h>
#include <QScopedPointer>

#ifndef QT_NO_IM

QT_BEGIN_NAMESPACE

class QWidget;
class QFont;
class QPopupMenu;
class QInputContextPrivate;

class Q_GUI_EXPORT QInputContext : public QObject
{
   GUI_CS_OBJECT(QInputContext)
   Q_DECLARE_PRIVATE(QInputContext)

 public:
   explicit QInputContext(QObject *parent = nullptr);
   virtual ~QInputContext();

   virtual QString identifierName() = 0;
   virtual QString language() = 0;

   virtual void reset() = 0;
   virtual void update();

   virtual void mouseHandler( int x, QMouseEvent *event);
   virtual QFont font() const;
   virtual bool isComposing() const = 0;

   QWidget *focusWidget() const;
   virtual void setFocusWidget( QWidget *w );

   virtual void widgetDestroyed(QWidget *w);

   virtual QList<QAction *> actions();

#if defined(Q_WS_X11)
   virtual bool x11FilterEvent( QWidget *keywidget, XEvent *event );
#endif

   virtual bool filterEvent( const QEvent *event );

   void sendEvent(const QInputMethodEvent &event);

   enum StandardFormat {
      PreeditFormat,
      SelectionFormat
   };
   QTextFormat standardFormat(StandardFormat s) const;

 protected:
   QScopedPointer<QInputContextPrivate> d_ptr;

 private:
   friend class QWidget;
   friend class QWidgetPrivate;
   friend class QInputContextFactory;
   friend class QApplication;

   // Disabled copy constructor and operator=
   QInputContext( const QInputContext &);
   QInputContext &operator=( const QInputContext &);

};

QT_END_NAMESPACE


#endif //Q_NO_IM

#endif // QINPUTCONTEXT_H
