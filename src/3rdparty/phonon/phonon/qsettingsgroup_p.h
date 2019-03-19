/***********************************************************************
*
* Copyright (c) 2012-2019 Barbara Geller
* Copyright (c) 2012-2019 Ansel Sermersheim
*
* Copyright (C) 2005-2006 Matthias Kretz <kretz@kde.org>
* Copyright (C) 2015 The Qt Company Ltd.
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

#ifndef PHONON_QSETTINGSGROUP_P_H
#define PHONON_QSETTINGSGROUP_P_H

#include <QtCore/QSettings>
#include <QtCore/QString>
#include <QtCore/QVariant>

#ifndef QT_NO_PHONON_SETTINGSGROUP

QT_BEGIN_NAMESPACE

namespace Phonon
{
class QSettingsGroup
{
    public:
        inline QSettingsGroup(QSettings *settings, const QString &name)
            : m_mutableSettings(settings), m_settings(settings), m_group(name + '/')
        {
        }

        inline QSettingsGroup(const QSettings *settings, const QString &name)
            : m_mutableSettings(0), m_settings(settings), m_group(name + '/')
        {
        }

        template<typename T>
        inline T value(const QString &key, const T &def) const
        {
            return qvariant_cast<T>(value(key, QVariant::fromValue(def)));
        }

        inline QVariant value(const QString &key, const QVariant &def) const
        {
            return m_settings->value(m_group + key, def);
        }

        template<typename T>
        inline void setValue(const QString &key, const T &value)
        {
            Q_ASSERT(m_mutableSettings);
            m_mutableSettings->setValue(m_group + key, QVariant::fromValue(value));
        }

        inline void removeEntry(const QString &key)
        {
            Q_ASSERT(m_mutableSettings);
            m_mutableSettings->remove(m_group + key);
        }

        inline bool hasKey(const QString &key) const
        {
            return m_settings->contains(m_group + key);
        }

    private:
        QSettings *const m_mutableSettings;
        const QSettings *const m_settings;
        QString m_group;
};
} // namespace Phonon

QT_END_NAMESPACE

#endif //QT_NO_PHONON_SETTINGSGROUP

#endif // PHONON_QSETTINGSGROUP_P_H
