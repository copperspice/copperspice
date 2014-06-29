/***********************************************************************
*
* Copyright (c) 2012-2013 Barbara Geller
* Copyright (c) 2012-2013 Ansel Sermersheim
* Copyright (c) 2011 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
*
* This file is part of CopperSpice.
*
* CopperSpice is free software: you can redistribute it and/or 
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with CopperSpice.  If not, see 
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#include "qdesigner_qsettings_p.h"

#include <QtCore/QSettings>
#include <QtCore/QCoreApplication>
#include <QtCore/QTextStream>
#include <QtCore/QString>
#include <QtCore/QDebug>

/*!
  \class QDesignerSettingsSimple

  \brief Implements QDesignerSettingsInterface by calls to QSettings
  */

QDesignerQSettings::QDesignerQSettings() :
    m_settings(qApp->organizationName(), settingsApplicationName())
{
}

QString QDesignerQSettings::settingsApplicationName()
{
    return qApp->applicationName();
}

void QDesignerQSettings::beginGroup(const QString &prefix)
{
    m_settings.beginGroup(prefix);
}

void QDesignerQSettings::endGroup()
{
    m_settings.endGroup();
}

bool QDesignerQSettings::contains(const QString &key) const
{
    return m_settings.contains(key);
}

void QDesignerQSettings::setValue(const QString &key, const QVariant &value)
{
    m_settings.setValue(key, value);
}

QVariant QDesignerQSettings::value(const QString &key, const QVariant &defaultValue) const
{
    return m_settings.value(key, defaultValue);
}

void QDesignerQSettings::remove(const QString &key)
{
    m_settings.remove(key);
}
