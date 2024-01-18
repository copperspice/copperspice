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

#ifndef QPLUGIN_H
#define QPLUGIN_H

#include <qobject.h>
#include <qpointer.h>

#ifndef Q_EXTERN_C
#  ifdef __cplusplus
#    define Q_EXTERN_C extern "C"
#  else
#    define Q_EXTERN_C extern
#  endif
#endif

/* emerald - hold for now

void Q_CORE_EXPORT qRegisterStaticPluginFunction(QStaticPlugin staticPlugin);

#define Q_IMPORT_PLUGIN(PLUGIN) \
   extern const QT_PREPEND_NAMESPACE(QStaticPlugin) qt_static_plugin_##PLUGIN(); \
   class Static##PLUGIN##PluginInstance{ \
   public: \
          Static##PLUGIN##PluginInstance() { \
              qRegisterStaticPluginFunction(qt_static_plugin_##PLUGIN()); \
          } \
   }; \
   static Static##PLUGIN##PluginInstance static##PLUGIN##Instance;

*/

#define Q_PLUGIN_INSTANCE(IMPLEMENTATION)              static_assert(false, "Obsolete plugin system")
#define Q_EXPORT_PLUGIN(PLUGIN)                        static_assert(false, "Obsolete plugin system")
#define Q_EXPORT_PLUGIN2(PLUGIN, PLUGINCLASS)          static_assert(false, "Obsolete plugin system")
#define Q_EXPORT_STATIC_PLUGIN2(PLUGIN, PLUGINCLASS)   static_assert(false, "Obsolete plugin system")

#endif
