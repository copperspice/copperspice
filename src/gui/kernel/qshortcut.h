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

#ifndef QSHORTCUT_H
#define QSHORTCUT_H

#include <QtGui/qwidget.h>
#include <QtGui/qkeysequence.h>
#include <QScopedPointer>

QT_BEGIN_NAMESPACE

#ifndef QT_NO_SHORTCUT

class QShortcutPrivate;

class Q_GUI_EXPORT QShortcut : public QObject
{
   GUI_CS_OBJECT(QShortcut)
   Q_DECLARE_PRIVATE(QShortcut)

   GUI_CS_PROPERTY_READ(key, key)
   GUI_CS_PROPERTY_WRITE(key, setKey)

   GUI_CS_PROPERTY_READ(whatsThis, whatsThis)
   GUI_CS_PROPERTY_WRITE(whatsThis, setWhatsThis)

   GUI_CS_PROPERTY_READ(enabled, isEnabled)
   GUI_CS_PROPERTY_WRITE(enabled, setEnabled)

   GUI_CS_PROPERTY_READ(autoRepeat, autoRepeat)
   GUI_CS_PROPERTY_WRITE(autoRepeat, setAutoRepeat)

   GUI_CS_PROPERTY_READ(context, context)
   GUI_CS_PROPERTY_WRITE(context, setContext)

 public:
   explicit QShortcut(QWidget *parent);
   QShortcut(const QKeySequence &key, QWidget *parent,
             const char *member = 0, const char *ambiguousMember = 0,
             Qt::ShortcutContext context = Qt::WindowShortcut);
   ~QShortcut();

   void setKey(const QKeySequence &key);
   QKeySequence key() const;

   void setEnabled(bool enable);
   bool isEnabled() const;

   void setContext(Qt::ShortcutContext context);
   Qt::ShortcutContext context() const;

   void setWhatsThis(const QString &text);
   QString whatsThis() const;

   void setAutoRepeat(bool on);
   bool autoRepeat() const;

   int id() const;

   inline QWidget *parentWidget() const {
      return static_cast<QWidget *>(QObject::parent());
   }

   GUI_CS_SIGNAL_1(Public, void activated())
   GUI_CS_SIGNAL_2(activated)

   GUI_CS_SIGNAL_1(Public, void activatedAmbiguously())
   GUI_CS_SIGNAL_2(activatedAmbiguously)

 protected:
   bool event(QEvent *e) override;
   QScopedPointer<QShortcutPrivate> d_ptr;

};

#endif // QT_NO_SHORTCUT

QT_END_NAMESPACE

#endif // QSHORTCUT_H
