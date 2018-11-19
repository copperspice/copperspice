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

#ifndef QPLUGIN_H
#define QPLUGIN_H

#include <QtCore/qobject.h>
#include <QtCore/qpointer.h>
#include <QtCore/qjsonobject.h>



#ifndef Q_EXTERN_C
#  ifdef __cplusplus
#    define Q_EXTERN_C extern "C"
#  else
#    define Q_EXTERN_C extern
#  endif
#endif

typedef QObject *(*QtPluginInstanceFunction)();
typedef const char *(*QtPluginMetaDataFunction)();

struct Q_CORE_EXPORT QStaticPlugin {
   QtPluginInstanceFunction instance;
   QtPluginMetaDataFunction rawMetaData;



   QJsonObject metaData() const;
};

Q_DECLARE_TYPEINFO(QStaticPlugin, Q_PRIMITIVE_TYPE);
void Q_CORE_EXPORT qRegisterStaticPluginFunction(QStaticPlugin staticPlugin);

#if (defined(Q_OF_ELF) || defined(Q_OS_WIN)) && (defined (Q_CC_GNU) || defined(Q_CC_CLANG))
#  define QT_PLUGIN_METADATA_SECTION \
    __attribute__ ((section (".qtmetadata"))) __attribute__((used))

#elif defined(Q_OS_MAC)

#  define QT_PLUGIN_METADATA_SECTION \
    __attribute__ ((section ("__TEXT,qtmetadata"))) __attribute__((used))

#elif defined(Q_CC_MSVC)

#pragma section(".qtmetadata",read,shared)
#  define QT_PLUGIN_METADATA_SECTION \
    __declspec(allocate(".qtmetadata"))

#else
#  define QT_PLUGIN_VERIFICATION_SECTION
#  define QT_PLUGIN_METADATA_SECTION
#endif
#define Q_IMPORT_PLUGIN(PLUGIN) \
        extern const QT_PREPEND_NAMESPACE(QStaticPlugin) qt_static_plugin_##PLUGIN(); \
        class Static##PLUGIN##PluginInstance{ \
        public: \
                Static##PLUGIN##PluginInstance() { \
                    qRegisterStaticPluginFunction(qt_static_plugin_##PLUGIN()); \
                } \
        }; \
       static Static##PLUGIN##PluginInstance static##PLUGIN##Instance;

#define Q_PLUGIN_INSTANCE(IMPLEMENTATION) \
        { \
            static QT_PREPEND_NAMESPACE(QPointer)<QT_PREPEND_NAMESPACE(QObject)> _instance; \
            if (!_instance)      \
                _instance = new IMPLEMENTATION; \
            return _instance; \
        }


#define Q_EXPORT_PLUGIN(PLUGIN) \
            Q_EXPORT_PLUGIN2(PLUGIN, PLUGIN)
#  define Q_EXPORT_PLUGIN2(PLUGIN, PLUGINCLASS)      \
    Q_STATIC_ASSERT_X(false, "Old plugin system used")

#  define Q_EXPORT_STATIC_PLUGIN2(PLUGIN, PLUGINCLASS) \
    Q_STATIC_ASSERT_X(false, "Old plugin system used")




#endif // Q_PLUGIN_H
