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

#ifndef QNETWORKACCESSFILEBACKEND_P_H
#define QNETWORKACCESSFILEBACKEND_P_H

#include "qnetworkaccessbackend_p.h"
#include "qnetworkrequest.h"
#include "qnetworkreply.h"
#include "QtCore/qfile.h"

QT_BEGIN_NAMESPACE

class QNetworkAccessFileBackend: public QNetworkAccessBackend
{
    CS_OBJECT(QNetworkAccessFileBackend)

public:
    QNetworkAccessFileBackend();
    virtual ~QNetworkAccessFileBackend();

    virtual void open();
    virtual void closeDownstreamChannel();

    virtual void downstreamReadyWrite();

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
    virtual QNetworkAccessBackend *create(QNetworkAccessManager::Operation op,
                                          const QNetworkRequest &request) const;
};

QT_END_NAMESPACE

#endif
