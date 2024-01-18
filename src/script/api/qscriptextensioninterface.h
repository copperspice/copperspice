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

#ifndef QSCRIPTEXTENSIONINTERFACE_H
#define QSCRIPTEXTENSIONINTERFACE_H

#include <qfactoryinterface.h>
#include <qobject.h>

class QScriptEngine;

struct Q_SCRIPT_EXPORT QScriptExtensionInterface : public QFactoryInterface {
   virtual void initialize(const QString &key, QScriptEngine *engine) = 0;
};

#define QScriptExtensionInterface_iid "com.copperspice.CS.QScriptExtensionInterface/1.0"
CS_DECLARE_INTERFACE(QScriptExtensionInterface, QScriptExtensionInterface_iid)


#endif





