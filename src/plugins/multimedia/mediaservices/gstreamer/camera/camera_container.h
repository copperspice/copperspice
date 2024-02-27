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

#ifndef CAMERABINMEDIACONTAINERCONTROL_H
#define CAMERABINMEDIACONTAINERCONTROL_H

#include <qmediacontainercontrol.h>
#include <qstringlist.h>
#include <qset.h>

#include <gst/gst.h>
#include <gst/pbutils/pbutils.h>

#ifdef HAVE_GST_ENCODING_PROFILES
#include <gst/pbutils/encoding-profile.h>
#include <qgstcodecsinfo_p.h>
#endif

class CameraBinContainer : public QMediaContainerControl
{
   CS_OBJECT(CameraBinContainer)

 public:
   CameraBinContainer(QObject *parent);
   virtual ~CameraBinContainer() { }

   QStringList supportedContainers() const override;
   QString containerDescription(const QString &formatMimeType) const override;

   QString containerFormat() const override;
   void setContainerFormat(const QString &format) override;

   QString actualContainerFormat() const;
   void setActualContainerFormat(const QString &containerFormat);
   void resetActualContainerFormat();

   QString suggestedFileExtension(const QString &containerFormat) const;

#ifdef HAVE_GST_ENCODING_PROFILES
   GstEncodingContainerProfile *createProfile();
#endif

 public:
   CS_SIGNAL_1(Public, void settingsChanged())
   CS_SIGNAL_2(settingsChanged)

 private:
   QString m_format;
   QString m_actualFormat;
   QMap<QString, QString> m_fileExtensions;

#ifdef HAVE_GST_ENCODING_PROFILES
   QGstCodecsInfo m_supportedContainers;
#endif
};

#endif
