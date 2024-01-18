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

#include <qapplication.h>
#include <qstring.h>

#include <qplatform_nativeinterface.h>
#include <qplatform_printplugin.h>

class QCocoaPrinterSupportPlugin : public QPlatformPrinterSupportPlugin
{
   CS_OBJECT(QCocoaPrinterSupportPlugin)

   CS_PLUGIN_IID(QPlatformPrinterSupportPlugin_ID)
   CS_PLUGIN_KEY("printerdriver_cocoa")

 public:
   QPlatformPrinterSupport *create(const QString &) override;
};

CS_PLUGIN_REGISTER(QCocoaPrinterSupportPlugin)

QPlatformPrinterSupport *QCocoaPrinterSupportPlugin::create(const QString &key)
{
   if (key.compare(key, "printerdriver_cocoa", Qt::CaseInsensitive) != 0) {
      return nullptr;
   }

   QApplication *app = dynamic_cast<QApplication *>(QCoreApplication::instance());
   if (! app) {
      return nullptr;
   }

   QPlatformNativeInterface *platformNativeInterface = app->platformNativeInterface();
   int at = platformNativeInterface->metaObject()->indexOfMethod("createPlatformPrinterSupport()");

   if (at == -1) {
      return nullptr;
   }

   QMetaMethod createPlatformPrinterSupport = platformNativeInterface->metaObject()->method(at);
   QPlatformPrinterSupport *platformPrinterSupport = nullptr;

   if (! createPlatformPrinterSupport.invoke(platformNativeInterface, Q_RETURN_ARG(QPlatformPrinterSupport *, platformPrinterSupport))) {
      return nullptr;
   }

   return platformPrinterSupport;
}


