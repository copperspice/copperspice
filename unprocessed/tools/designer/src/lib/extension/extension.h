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

#ifndef EXTENSION_H
#define EXTENSION_H

#include <QtCore/QString>
#include <QtCore/QObject>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

#define Q_TYPEID(IFace) QLatin1String(IFace##_iid)

class QAbstractExtensionFactory
{
public:
    virtual ~QAbstractExtensionFactory() {}

    virtual QObject *extension(QObject *object, const QString &iid) const = 0;
};
Q_DECLARE_INTERFACE(QAbstractExtensionFactory, "com.trolltech.Qt.QAbstractExtensionFactory")

class QAbstractExtensionManager
{
public:
    virtual ~QAbstractExtensionManager() {}

    virtual void registerExtensions(QAbstractExtensionFactory *factory, const QString &iid) = 0;
    virtual void unregisterExtensions(QAbstractExtensionFactory *factory, const QString &iid) = 0;

    virtual QObject *extension(QObject *object, const QString &iid) const = 0;
};
Q_DECLARE_INTERFACE(QAbstractExtensionManager, "com.trolltech.Qt.QAbstractExtensionManager")

#if defined(Q_CC_MSVC) && (_MSC_VER < 1300)

template <class T>
inline T qt_extension_helper(QAbstractExtensionManager *, QObject *, T)
{ return 0; }

template <class T>
inline T qt_extension(QAbstractExtensionManager* manager, QObject *object)
{ return qt_extension_helper(manager, object, T(0)); }

#define Q_DECLARE_EXTENSION_INTERFACE(IFace, IId) \
const char * const IFace##_iid = IId; \
Q_DECLARE_INTERFACE(IFace, IId) \
template <> inline IFace *qt_extension_helper<IFace *>(QAbstractExtensionManager *manager, QObject *object, IFace *) \
{ QObject *extension = manager->extension(object, Q_TYPEID(IFace)); return (IFace *)(extension ? extension->qt_metacast(IFace##_iid) : 0); }

#else

template <class T>
inline T qt_extension(QAbstractExtensionManager* manager, QObject *object)
{ return 0; }

#define Q_DECLARE_EXTENSION_INTERFACE(IFace, IId) \
const char * const IFace##_iid = IId; \
Q_DECLARE_INTERFACE(IFace, IId) \
template <> inline IFace *qt_extension<IFace *>(QAbstractExtensionManager *manager, QObject *object) \
{ QObject *extension = manager->extension(object, Q_TYPEID(IFace)); return extension ? static_cast<IFace *>(extension->qt_metacast(IFace##_iid)) : static_cast<IFace *>(0); }

#endif

QT_END_NAMESPACE

QT_END_HEADER

#endif // EXTENSION_H
