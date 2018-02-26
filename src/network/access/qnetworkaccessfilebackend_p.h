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

#ifndef QNETWORKACCESSFILEBACKEND_P_H
#define QNETWORKACCESSFILEBACKEND_P_H

#include <qnetworkaccessbackend_p.h>
#include <qnetworkrequest.h>
#include <qnetworkreply.h>
#include <qfile.h>


class QNetworkAccessFileBackend: public QNetworkAccessBackend
{
   NET_CS_OBJECT(QNetworkAccessFileBackend)

 public:
   QNetworkAccessFileBackend();
   virtual ~QNetworkAccessFileBackend();

   void open() override;
   void closeDownstreamChannel() override;

   void downstreamReadyWrite() override;

   NET_CS_SLOT_1(Public, void uploadReadyReadSlot())
   NET_CS_SLOT_2(uploadReadyReadSlot)

 protected:
   QNonContiguousByteDevice *uploadByteDevice;

 private:
   QFile file;
   qint64 totalBytes;
   bool hasUploadFinished;

   bool loadFileInfo();
   bool readMoreFromFile();
};

class QNetworkAccessFileBackendFactory: public QNetworkAccessBackendFactory
{
 public:
    QStringList supportedSchemes() const override;
    QNetworkAccessBackend *create(QNetworkAccessManager::Operation op, const QNetworkRequest &request) const override;
};


#endif
