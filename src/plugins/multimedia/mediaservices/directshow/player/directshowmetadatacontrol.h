/***********************************************************************
*
* Copyright (c) 2012-2019 Barbara Geller
* Copyright (c) 2012-2019 Ansel Sermersheim
*
* Copyright (C) 2015 The Qt Company Ltd.
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

#ifndef DIRECTSHOWMETADATACONTROL_H
#define DIRECTSHOWMETADATACONTROL_H

#include <dshow.h>

#include <qmetadatareadercontrol.h>

#include "directshowglobal.h"

#include <QtCore/qcoreevent.h>

class DirectShowPlayerService;

QT_USE_NAMESPACE

class DirectShowMetaDataControl : public QMetaDataReaderControl
{
   CS_OBJECT(DirectShowMetaDataControl)
 public:
   DirectShowMetaDataControl(QObject *parent = 0);
   ~DirectShowMetaDataControl();

   bool isMetaDataAvailable() const;

   QVariant metaData(const QString &key) const;
   QStringList availableMetaData() const;

   void reset();
   void updateMetadata(IFilterGraph2 *graph, IBaseFilter *source,
      const QString &fileSrc = QString());

 protected:
   void customEvent(QEvent *event);

 private:
   void setMetadataAvailable(bool available);

   enum Event {
      MetaDataChanged = QEvent::User
   };

   QVariantMap m_metadata;
   bool m_available;
};

#endif
