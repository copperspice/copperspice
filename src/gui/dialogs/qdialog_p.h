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

#ifndef QDIALOG_P_H
#define QDIALOG_P_H

#include <qwidget_p.h>
#include <qeventloop.h>
#include <qpointer.h>
#include <qdialog.h>
#include <qpushbutton.h>
#include <qplatform_dialoghelper.h>

class QSizeGrip;

class QDialogPrivate : public QWidgetPrivate
{
   Q_DECLARE_PUBLIC(QDialog)

 public:

   QDialogPrivate()
      : mainDef(nullptr), orientation(Qt::Horizontal), extension(nullptr), doShowExtension(false),
#ifndef QT_NO_SIZEGRIP
        resizer(nullptr), sizeGripEnabled(false),
#endif
        rescode(0), resetModalityTo(-1), wasModalitySet(true), eventLoop(nullptr),
        nativeDialogInUse(false), m_platformHelper(nullptr), m_platformHelperCreated(false)
   {}

   ~QDialogPrivate() {
      delete m_platformHelper;
   }

   QWindow *parentWindow() const;
   bool setNativeDialogVisible(bool visible);
   QVariant styleHint(QPlatformDialogHelper::StyleHint hint) const;
   void deletePlatformHelper();

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

   int rescode;
   int resetModalityTo;
   bool wasModalitySet;

   QPointer<QEventLoop> eventLoop;
   bool nativeDialogInUse;
   QPlatformDialogHelper *platformHelper() const;
   virtual bool canBeNativeDialog() const;

 private:
   virtual void initHelper(QPlatformDialogHelper *) {}
   virtual void helperPrepareShow(QPlatformDialogHelper *) {}
   virtual void helperDone(QDialog::DialogCode, QPlatformDialogHelper *) {}

   mutable QPlatformDialogHelper *m_platformHelper;
   mutable bool m_platformHelperCreated;
};

#endif
