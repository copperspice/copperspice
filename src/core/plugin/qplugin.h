/***********************************************************************
*
* Copyright (c) 2012-2017 Barbara Geller
* Copyright (c) 2012-2017 Ansel Sermersheim
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

QT_BEGIN_NAMESPACE

#ifndef Q_EXTERN_C
#  ifdef __cplusplus
#    define Q_EXTERN_C extern "C"
#  else
#    define Q_EXTERN_C extern
#  endif
#endif

typedef QObject *(*QtPluginInstanceFunction)();

void Q_CORE_EXPORT qRegisterStaticPluginInstanceFunction(QtPluginInstanceFunction function);

#define Q_IMPORT_PLUGIN(PLUGIN) \
        extern QT_PREPEND_NAMESPACE(QObject) *qt_plugin_instance_##PLUGIN(); \
        class Static##PLUGIN##PluginInstance{ \
        public: \
                Static##PLUGIN##PluginInstance()  \
                { \
                   qRegisterStaticPluginInstanceFunction(qt_plugin_instance_##PLUGIN); \
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

#define Q_EXPORT_PLUGIN(PLUGIN)         Q_EXPORT_PLUGIN2(PLUGIN, PLUGIN)
#define Q_EXPORT_STATIC_PLUGIN(PLUGIN)  Q_EXPORT_STATIC_PLUGIN2(PLUGIN, PLUGIN)

#if defined(QT_STATICPLUGIN)

#  define Q_EXPORT_PLUGIN2(PLUGIN, PLUGINCLASS) \
            QT_PREPEND_NAMESPACE(QObject) *qt_plugin_instance_##PLUGIN()  Q_PLUGIN_INSTANCE(PLUGINCLASS)

#  define Q_EXPORT_STATIC_PLUGIN2(PLUGIN, PLUGINCLASS)   Q_EXPORT_PLUGIN2(PLUGIN, PLUGINCLASS)

#else

// NOTE: if you change the variable "pattern", this MUST also be modified in qlibrary.cpp and qplugin.cpp
// QT5: should probably remove the entire pattern concept and do the section trick

#  ifdef QPLUGIN_DEBUG_STR
#    undef QPLUGIN_DEBUG_STR
#  endif

#  ifdef QT_NO_DEBUG
#    define QPLUGIN_DEBUG_STR "false"
#    define QPLUGIN_SECTION_DEBUG_STR ""
#  else
#    define QPLUGIN_DEBUG_STR "true"
#    define QPLUGIN_SECTION_DEBUG_STR ".debug"
#  endif

#  define Q_PLUGIN_VERIFICATION_DATA \
    static const char qt_plugin_verification_data[] = \
      "pattern=CS_PLUGIN_VERIFICATION_DATA\n" \
      "version=" CS_VERSION_STR "\n" \
      "debug=" QPLUGIN_DEBUG_STR "\n" \
      "buildkey=" QT_BUILD_KEY;

#  if defined (Q_OF_ELF) && defined (Q_CC_GNU)
#  define Q_PLUGIN_VERIFICATION_SECTION \
    __attribute__ ((section (".qtplugin"))) __attribute__((used))
#  else
#  define Q_PLUGIN_VERIFICATION_SECTION
#  endif

#define Q_STANDARD_CALL

#  define Q_EXPORT_PLUGIN2(PLUGIN, PLUGINCLASS)      \
            Q_PLUGIN_VERIFICATION_SECTION Q_PLUGIN_VERIFICATION_DATA \
            Q_EXTERN_C Q_DECL_EXPORT \
            const char * Q_STANDARD_CALL cs_plugin_query_verification_data() \
                { return qt_plugin_verification_data; } \
            Q_EXTERN_C Q_DECL_EXPORT QT_PREPEND_NAMESPACE(QObject) * Q_STANDARD_CALL qt_plugin_instance() \
            Q_PLUGIN_INSTANCE(PLUGINCLASS)

#  define Q_EXPORT_STATIC_PLUGIN2(PLUGIN, PLUGINCLASS)

#endif

QT_END_NAMESPACE

#endif // Q_PLUGIN_H
