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

#include "qwebview_plugin.h"

#include <QtDesigner/QExtensionFactory>
#include <QtDesigner/QExtensionManager>

#include <QtCore/qplugin.h>
#include <QtWebKit/QWebView>

static const char *toolTipC = "QtWebKit Web widget";

QT_BEGIN_NAMESPACE

QWebViewPlugin::QWebViewPlugin(QObject *parent) :
    QObject(parent),
    m_initialized(false)
{
}

QString QWebViewPlugin::name() const
{
    return QLatin1String("QWebView");
}

QString QWebViewPlugin::group() const
{
    return QLatin1String("Display Widgets");
}

QString QWebViewPlugin::toolTip() const
{
    return tr(toolTipC);
}

QString QWebViewPlugin::whatsThis() const
{
    return tr(toolTipC);
}

QString QWebViewPlugin::includeFile() const
{
    return QLatin1String("QtWebKit/QWebView");
}

QIcon QWebViewPlugin::icon() const
{
    return QIcon(QLatin1String(":/trolltech/qwebview/images/qwebview.png"));
}

bool QWebViewPlugin::isContainer() const
{
    return false;
}

QWidget *QWebViewPlugin::createWidget(QWidget *parent)
{
    return new QWebView(parent);
}

bool QWebViewPlugin::isInitialized() const
{
    return m_initialized;
}

void QWebViewPlugin::initialize(QDesignerFormEditorInterface * /*core*/)
{
    if (m_initialized)
        return;

    m_initialized = true;
}

QString QWebViewPlugin::domXml() const
{
    return QLatin1String("\
    <ui language=\"c++\">\
        <widget class=\"QWebView\" name=\"webView\">\
            <property name=\"url\">\
                <url>\
                    <string>about:blank</string>\
                </url>\
            </property>\
            <property name=\"geometry\">\
                <rect>\
                    <x>0</x>\
                    <y>0</y>\
                    <width>300</width>\
                    <height>200</height>\
                </rect>\
            </property>\
        </widget>\
    </ui>");
}

Q_EXPORT_PLUGIN2(customwidgetplugin, QWebViewPlugin)

QT_END_NAMESPACE
