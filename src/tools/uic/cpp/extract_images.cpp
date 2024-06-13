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

#include <extract_images.h>

#include <driver.h>
#include <ui4.h>
#include <uic.h>
#include <utils.h>
#include <write_icondata.h>

#include <qdatastream.h>
#include <qdir.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qtextcodec.h>
#include <qtextstream.h>

namespace CPP {

ExtractImages::ExtractImages(const Option &opt)
   : m_output(nullptr), m_option(opt)
{
}

void ExtractImages::acceptUI(DomUI *node)
{
   if (! m_option.extractImages) {
      return;
   }

   if (node->elementImages() == nullptr) {
      return;
   }

   QString className = node->elementClass() + m_option.postfix;

   QFile f;
   if (m_option.qrcOutputFile.size()) {
      f.setFileName(m_option.qrcOutputFile);

      if (! f.open(QIODevice::WriteOnly | QFile::Text)) {
         fprintf(stderr, "%s: Error: Could not create resource file\n", csPrintable(m_option.messagePrefix()));
         return;
      }

      QFileInfo fi(m_option.qrcOutputFile);
      QDir dir = fi.absoluteDir();

      if (!dir.exists("images") && ! dir.mkdir("images")) {
         fprintf(stderr, "%s: Error: Could not create image dir\n", csPrintable(m_option.messagePrefix()));
         return;
      }

      dir.cd("images");
      m_imagesDir = dir;

      m_output = new QTextStream(&f);
      m_output->setCodec(QTextCodec::codecForName("UTF-8"));

      QTextStream &out = *m_output;

      out << "<RCC>\n";
      out << "    <qresource prefix=\"/" << className << "\" >\n";
      TreeWalker::acceptUI(node);
      out << "    </qresource>\n";
      out << "</RCC>\n";

      f.close();
      delete m_output;
      m_output = nullptr;
   }
}

void ExtractImages::acceptImages(DomImages *images)
{
   TreeWalker::acceptImages(images);
}

void ExtractImages::acceptImage(DomImage *image)
{
   QString format    = image->elementData()->attributeFormat();
   QString extension = format.left(format.indexOf('.')).toLower();
   QString fname     = m_imagesDir.absoluteFilePath(image->attributeName() + '.' + extension);

   *m_output << "        <file>images/" << image->attributeName() << '.' + extension << "</file>\n";

   QFile f;
   f.setFileName(fname);
   const bool isXPM_GZ = (format == "XPM.GZ");

   QIODevice::OpenMode openMode = QIODevice::WriteOnly;

   if (isXPM_GZ) {
      openMode |= QIODevice::Text;
   }

   if (! f.open(openMode)) {
      fprintf(stderr, "%s: Error: Could not create image file %s: %s",
         csPrintable(m_option.messagePrefix()), csPrintable(fname), csPrintable(f.errorString()));
      return;
   }

   if (isXPM_GZ) {
      QTextStream *imageOut = new QTextStream(&f);
      imageOut->setCodec(QTextCodec::codecForName("UTF-8"));

      CPP::WriteIconData::writeImage(*imageOut, QString(), m_option.limitXPM_LineLength, image);
      delete imageOut;

   } else {
      CPP::WriteIconData::writeImage(f, image);
   }

   f.close();
}

} // namespace CPP
