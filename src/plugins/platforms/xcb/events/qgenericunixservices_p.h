/***********************************************************************
*
* Copyright (c) 2012-2020 Barbara Geller
* Copyright (c) 2012-2020 Ansel Sermersheim
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

#ifndef QGENERICUNIXDESKTOPSERVICES_H
#define QGENERICUNIXDESKTOPSERVICES_H

#include <qplatform_services.h>
#include <qstring.h>

class QGenericUnixServices : public QPlatformServices
{
 public:
   QGenericUnixServices() {}

   QByteArray desktopEnvironment() const override;

   bool openUrl(const QUrl &url) override;
   bool openDocument(const QUrl &url) override;

 private:
   QString m_webBrowser;
   QString m_documentLauncher;
};

#endif // QGENERICUNIXDESKTOPSERVICES_H
