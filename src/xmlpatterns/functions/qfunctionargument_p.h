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

#ifndef QFunctionArgument_P_H
#define QFunctionArgument_P_H

#include <qlist.h>
#include <QSharedData>
#include <qxmlname.h>
#include <qsequencetype_p.h>

namespace QPatternist {

class FunctionArgument : public QSharedData
{
 public:
   typedef QExplicitlySharedDataPointer<FunctionArgument> Ptr;
   typedef QList<FunctionArgument::Ptr> List;

   FunctionArgument(const QXmlName name, const SequenceType::Ptr &type);

   QXmlName name() const;
   SequenceType::Ptr type() const;

 private:
   FunctionArgument(const FunctionArgument &) = delete;
   FunctionArgument &operator=(const FunctionArgument &) = delete;

   const QXmlName m_name;
   const SequenceType::Ptr m_type;
};

}

#endif
