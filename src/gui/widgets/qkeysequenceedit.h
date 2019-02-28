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

#ifndef QKEYSEQUENCEEDIT_H
#define QKEYSEQUENCEEDIT_H

#include <qwidget.h>


#ifndef QT_NO_KEYSEQUENCEEDIT

class QKeySequenceEditPrivate;

class Q_GUI_EXPORT QKeySequenceEdit : public QWidget
{
   GUI_CS_OBJECT(QKeySequenceEdit)

   GUI_CS_PROPERTY_READ(keySequence, keySequence)
   GUI_CS_PROPERTY_WRITE(keySequence, setKeySequence)
   GUI_CS_PROPERTY_NOTIFY(keySequence, keySequenceChanged)
   GUI_CS_PROPERTY_USER(keySequence, true)

 public:
   explicit QKeySequenceEdit(QWidget *parent = nullptr);
   explicit QKeySequenceEdit(const QKeySequence &keySequence, QWidget *parent = nullptr);
   ~QKeySequenceEdit();

   QKeySequence keySequence() const;

   GUI_CS_SLOT_1(Public, void setKeySequence(const QKeySequence &keySequence))
   GUI_CS_SLOT_2(setKeySequence)
   GUI_CS_SLOT_1(Public, void clear())
   GUI_CS_SLOT_2(clear)

   GUI_CS_SIGNAL_1(Public, void editingFinished())
   GUI_CS_SIGNAL_2(editingFinished)
   GUI_CS_SIGNAL_1(Public, void keySequenceChanged(const QKeySequence &keySequence))
   GUI_CS_SIGNAL_2(keySequenceChanged, keySequence)

 protected:
   QKeySequenceEdit(QKeySequenceEditPrivate &d, QWidget *parent, Qt::WindowFlags f);

   bool event(QEvent *) override;
   void keyPressEvent(QKeyEvent *) override;
   void keyReleaseEvent(QKeyEvent *) override;
   void timerEvent(QTimerEvent *) override;

 private:
   Q_DISABLE_COPY(QKeySequenceEdit)
   Q_DECLARE_PRIVATE(QKeySequenceEdit)
};

#endif // QT_NO_KEYSEQUENCEEDIT

#endif
