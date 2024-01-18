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

#ifndef QURILOADER_P_H
#define QURILOADER_P_H

#include <qnetaccess_manager.h>
#include <qxmlname.h>

#include <qnamepool_p.h>
#include <qvariableloader_p.h>

namespace QPatternist {

class URILoader : public QNetworkAccessManager
{
 public:
   URILoader(QObject *const parent, const NamePool::Ptr &np, const VariableLoader::Ptr &variableLoader);

   QNetworkReply *createRequest(Operation op, const QNetworkRequest &req, QIODevice *outgoingData = nullptr) override;

 private:
   const QString             m_variableNS;
   const NamePool::Ptr       m_namePool;
   const VariableLoader::Ptr m_variableLoader;
};

}

#endif
