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

#ifndef QDIALOG_P_H
#define QDIALOG_P_H

#include <qwidget_p.h>
#include <QtCore/qeventloop.h>
#include <QtCore/qpointer.h>
#include <QtGui/qdialog.h>
#include <QtGui/qpushbutton.h>

QT_BEGIN_NAMESPACE

class QSizeGrip;

class QDialogPrivate : public QWidgetPrivate
{
   Q_DECLARE_PUBLIC(QDialog)

 public:

   QDialogPrivate()
      : mainDef(0), orientation(Qt::Horizontal), extension(0), doShowExtension(false),
#ifndef QT_NO_SIZEGRIP
        resizer(0),
        sizeGripEnabled(false),
#endif
        rescode(0), resetModalityTo(-1), wasModalitySet(true), eventLoop(0) {
   }

   QPointer<QPushButton> mainDef;
   Qt::Orientation orientation;
   QWidget *extension;
   bool doShowExtension;
   QSize size, min, max;

#ifndef QT_NO_SIZEGRIP
   QSizeGrip *resizer;
   bool sizeGripEnabled;
#endif

   QPoint lastRMBPress;

   void setDefault(QPushButton *);
   void setMainDefault(QPushButton *);
   void hideDefault();
   void resetModalitySetByOpen();

#ifdef Q_OS_MAC
   virtual void mac_nativeDialogModalHelp() {}
#endif

   int rescode;
   int resetModalityTo;
   bool wasModalitySet;

   QPointer<QEventLoop> eventLoop;
};

QT_END_NAMESPACE

#endif // QDIALOG_P_H
