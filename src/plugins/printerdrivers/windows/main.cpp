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

#include <qwindowsprintersupport.h>

#include <qplatform_printplugin.h>
#include <qstringlist.h>

class QWindowsPrinterSupportPlugin : public QPlatformPrinterSupportPlugin
{
   CS_OBJECT(QWindowsPrinterSupportPlugin)

   CS_PLUGIN_IID(QPlatformPrinterSupportPlugin_ID)
   CS_PLUGIN_KEY("printerdriver_windows")

 public:
   QPlatformPrinterSupport *create(const QString &);
};

CS_PLUGIN_REGISTER(QWindowsPrinterSupportPlugin)

QPlatformPrinterSupport *QWindowsPrinterSupportPlugin::create(const QString &key)
{
   if (key.compare(key, "printerdriver_windows", Qt::CaseInsensitive) == 0) {
      return new QWindowsPrinterSupport;
   }

   return 0;
}

