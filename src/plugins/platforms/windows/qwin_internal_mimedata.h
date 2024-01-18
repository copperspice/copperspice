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

#ifndef QWINDOWSINTERNALMIME_H
#define QWINDOWSINTERNALMIME_H

#include <qwin_additional.h>
#include <qvariant.h>

#include <qdnd_p.h>

class QDebug;

// Implementation in qwindowsclipboard.cpp.
class QWindowsInternalMimeData : public QInternalMimeData
{
 public:
   bool hasFormat_sys(const QString &mimetype) const override;
   QStringList formats_sys() const override;
   QVariant retrieveData_sys(const QString &mimetype, QVariant::Type preferredType) const override;

 protected:
   virtual IDataObject *retrieveDataObject() const = 0;
   virtual void releaseDataObject(IDataObject *) const {}
};

#endif
