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

#ifndef QWSINPUTCONTEXT_P_H
#define QWSINPUTCONTEXT_P_H

#include <QtGui/qinputcontext.h>

#ifndef QT_NO_QWS_INPUTMETHODS

QT_BEGIN_NAMESPACE

class QWSIMEvent;
class QWSIMQueryEvent;
class QWSIMInitEvent;

class QWSInputContext : public QInputContext
{
   GUI_CS_OBJECT(QWSInputContext)

 public:
   explicit QWSInputContext(QObject *parent = nullptr);
   ~QWSInputContext() {}

   QString identifierName() {
      return QString();
   }

   QString language() {
      return QString();
   }

   void reset();
   void update();
   void mouseHandler( int x, QMouseEvent *event);

   void setFocusWidget( QWidget *w );
   void widgetDestroyed(QWidget *w);

   bool isComposing() const;

   static QWidget *activeWidget();
   static bool translateIMEvent(QWidget *w, const QWSIMEvent *e);
   static bool translateIMQueryEvent(QWidget *w, const QWSIMQueryEvent *e);
   static bool translateIMInitEvent(const QWSIMInitEvent *e);
   static void updateImeStatus(QWidget *w, bool hasFocus);
};

QT_END_NAMESPACE

#endif // QT_NO_QWS_INPUTMETHODS

#endif // QWSINPUTCONTEXT_P_H
