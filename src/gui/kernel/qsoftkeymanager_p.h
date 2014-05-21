/***********************************************************************
*
* Copyright (c) 2012-2014 Barbara Geller
* Copyright (c) 2012-2014 Ansel Sermersheim
* Copyright (c) 2012-2014 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
*
* This file is part of CopperSpice.
*
* CopperSpice is free software: you can redistribute it and/or 
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with CopperSpice.  If not, see 
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#ifndef QSOFTKEYMANAGER_P_H
#define QSOFTKEYMANAGER_P_H

#include <QtCore/qobject.h>
#include "QtGui/qaction.h"
#include <QScopedPointer>

#ifndef QT_NO_SOFTKEYMANAGER

QT_BEGIN_NAMESPACE

class QSoftKeyManagerPrivate;

class QSoftKeyManager : public QObject
{
    CS_OBJECT(QSoftKeyManager)
    Q_DECLARE_PRIVATE(QSoftKeyManager)

public:

    enum StandardSoftKey {
        OkSoftKey,
        SelectSoftKey,
        DoneSoftKey,
        MenuSoftKey,
        CancelSoftKey
    };

    static void updateSoftKeys();
    static QAction *createAction(StandardSoftKey standardKey, QWidget *actionWidget);
    static QAction *createKeyedAction(StandardSoftKey standardKey, Qt::Key key, QWidget *actionWidget);
    static QString standardSoftKeyText(StandardSoftKey standardKey);
    static void setForceEnabledInSoftkeys(QAction *action);
    static bool isForceEnabledInSofkeys(QAction *action);

protected:
    bool event(QEvent *e);
    QScopedPointer<QSoftKeyManagerPrivate> d_ptr;

private:
    QSoftKeyManager();
    static QSoftKeyManager *instance();
    bool appendSoftkeys(const QWidget &source, int level);
    QWidget *softkeySource(QWidget *previousSource, bool& recursiveMerging);
    bool handleUpdateSoftKeys();

    GUI_CS_SLOT_1(Private, void cleanupHash(QObject * obj))
    GUI_CS_SLOT_2(cleanupHash) 

    GUI_CS_SLOT_1(Private, void sendKeyEvent())
    GUI_CS_SLOT_2(sendKeyEvent) 

    Q_DISABLE_COPY(QSoftKeyManager)
 
};

QT_END_NAMESPACE

#endif //QT_NO_SOFTKEYMANAGER

#endif //QSOFTKEYMANAGER_P_H
