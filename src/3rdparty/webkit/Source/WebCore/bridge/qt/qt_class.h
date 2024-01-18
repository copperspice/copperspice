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

#ifndef qt_class_h
#define qt_class_h

#include "BridgeJSC.h"
#include "qglobal.h"

class QObject;
class QMetaObject;

namespace JSC {
namespace Bindings {

class QtClass : public Class {


public:
    static QtClass* classForObject(QObject*);
    virtual ~QtClass();

    virtual const char * name() const;
    virtual MethodList methodsNamed(const Identifier&, Instance*) const;
    virtual Field* fieldNamed(const Identifier&, Instance*) const;

    virtual JSValue fallbackObject(ExecState*, Instance*, const Identifier&);

protected:
    QtClass(const QMetaObject *);

private:
    QtClass(const QtClass &);                // prohibit copying
    QtClass& operator=(const QtClass&);      // prohibit assignment

    const QMetaObject* m_metaObject;
};

} // namespace Bindings
} // namespace JSC

#endif
