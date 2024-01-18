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

#ifndef QDECLARATIVEGLOBAL_P_H
#define QDECLARATIVEGLOBAL_P_H

#include <QtCore/qglobal.h>
#include <QtCore/QObject>

QT_BEGIN_NAMESPACE

#define DEFINE_BOOL_CONFIG_OPTION(name, var) \
    static bool name() \
    { \
        static enum { Yes, No, Unknown } status = Unknown; \
        if (status == Unknown) { \
            QByteArray v = qgetenv(#var); \
            bool value = !v.isEmpty() && v != "0" && v != "false"; \
            if (value) status = Yes; \
            else status = No; \
        } \
        return status == Yes; \
    }


#define Q_DECLARATIVE_PRIVATE_EXPORT Q_DECLARATIVE_EXPORT

struct QDeclarativeGraphics_DerivedObject : public QObject {
   void setParent_noEvent(QObject *parent) {
      bool sendChildEvents = CSInternalEvents::get_m_sendChildEvents(this);

      CSInternalEvents::set_m_sendChildEvents(this, false);
      setParent(parent);

      CSInternalEvents::set_m_sendChildEvents(this, sendChildEvents);
   }

};

/*!
    Returns true if the case of \a fileName is equivalent to the file case of
    \a fileName on disk, and false otherwise.

    This is used to ensure that the behavior of QML on a case-insensitive file
    system is the same as on a case-sensitive file system.  This function
    performs a "best effort" attempt to determine the real case of the file.
    It may have false positives (say the case is correct when it isn't), but it
    should never have a false negative (say the case is incorrect when it is
    correct).
*/
bool QDeclarative_isFileCaseCorrect(const QString &fileName);

/*!
    Makes the \a object a child of \a parent.  Note that when using this method,
    neither \a parent nor the object's previous parent (if it had one) will
    receive ChildRemoved or ChildAdded events.
*/
inline void QDeclarative_setParent_noEvent(QObject *object, QObject *parent)
{
   static_cast<QDeclarativeGraphics_DerivedObject *>(object)->setParent_noEvent(parent);
}

QT_END_NAMESPACE

#endif // QDECLARATIVEGLOBAL_H













