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

#include <camera_container.h>
#include <qregularexpression.h>
#include <qdebug.h>

CameraBinContainer::CameraBinContainer(QObject *parent)
   : QMediaContainerControl(parent)

#ifdef HAVE_GST_ENCODING_PROFILES
   , m_supportedContainers(QGstCodecsInfo::Muxer)
#endif
{
   //extension for containers hard to guess from mimetype
   m_fileExtensions["video/x-matroska"] = "mkv";
   m_fileExtensions["video/quicktime"] = "mov";
   m_fileExtensions["video/x-msvideo"] = "avi";
   m_fileExtensions["video/msvideo"] = "avi";
   m_fileExtensions["audio/mpeg"] = "mp3";
   m_fileExtensions["application/x-shockwave-flash"] = "swf";
   m_fileExtensions["application/x-pn-realmedia"] = "rm";
}

QStringList CameraBinContainer::supportedContainers() const
{
#ifdef HAVE_GST_ENCODING_PROFILES
   return m_supportedContainers.supportedCodecs();
#else
   return QStringList();
#endif
}

QString CameraBinContainer::containerDescription(const QString &formatMimeType) const
{
#ifdef HAVE_GST_ENCODING_PROFILES
   return m_supportedContainers.codecDescription(formatMimeType);
#else
   (void) formatMimeType;
   return QString();
#endif
}

QString CameraBinContainer::containerFormat() const
{
   return m_format;
}

void CameraBinContainer::setContainerFormat(const QString &format)
{
#ifdef HAVE_GST_ENCODING_PROFILES
   if (m_format != format) {
      m_format = format;
      m_actualFormat = format;
      emit settingsChanged();
   }
#else
   (void) format;
#endif
}

QString CameraBinContainer::actualContainerFormat() const
{
   return m_actualFormat;
}

void CameraBinContainer::setActualContainerFormat(const QString &containerFormat)
{
#ifdef HAVE_GST_ENCODING_PROFILES
   m_actualFormat = containerFormat;
#else
   (void) containerFormat;
#endif
}

void CameraBinContainer::resetActualContainerFormat()
{
   m_actualFormat = m_format;
}

#ifdef HAVE_GST_ENCODING_PROFILES

GstEncodingContainerProfile *CameraBinContainer::createProfile()
{
   GstCaps *caps;

   if (m_actualFormat.isEmpty()) {
      return 0;

   } else {
      QString format = m_actualFormat;
      QStringList supportedFormats = m_supportedContainers.supportedCodecs();

      //if format is not in the list of supported gstreamer mime types,
      //try to find the mime type with matching extension
      if (! supportedFormats.contains(format)) {
         QString extension = suggestedFileExtension(m_actualFormat);

         for (const QString &formatCandidate : supportedFormats) {
            if (suggestedFileExtension(formatCandidate) == extension) {
               format = formatCandidate;
               break;
            }
         }
      }

      caps = gst_caps_from_string(format.toLatin1());
   }

   GstEncodingContainerProfile *profile = (GstEncodingContainerProfile *)gst_encoding_container_profile_new(
         "camerabin2_profile", (gchar *)"custom camera profile", caps, NULL); // preset

   gst_caps_unref(caps);

   return profile;
}

#endif

/*!
  Suggest file extension for current container mimetype.
 */
QString CameraBinContainer::suggestedFileExtension(const QString &containerFormat) const
{
   // for container names like avi instead of video/x-msvideo, use it as extension
   if (! containerFormat.contains('/')) {
      return containerFormat;
   }

   QString format    = containerFormat.left(containerFormat.indexOf(','));
   QString extension = m_fileExtensions.value(format);

   if (! extension.isEmpty() || format.isEmpty()) {
      return extension;
   }

   QRegularExpression regexp("[-/]([\\w]+)$");
   QRegularExpressionMatch match = regexp.match(format);

   if (match.hasMatch()) {
      extension = match.captured(1);
   }

   return extension;
}

