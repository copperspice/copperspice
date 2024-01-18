/***********************************************************************
*
* Copyright (c) 2012-2024 Barbara Geller
* Copyright (c) 2012-2024 Ansel Sermersheim
*
* Copyright (c) 2013 Klarälvdalens Datakonsult AB, a KDAB Group company
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

#ifndef QOPENGL_P_H
#define QOPENGL_P_H

#include <qopengl.h>
#include <qopenglcontext_p.h>
#include <qset.h>
#include <qstring.h>
#include <qversionnumber.h>

class QJsonDocument;

class Q_GUI_EXPORT QOpenGLExtensionMatcher
{
public:
    QOpenGLExtensionMatcher();

    bool match(const QByteArray &extension) const
    {
        return m_extensions.contains(extension);
    }

    QSet<QByteArray> extensions() const { return m_extensions; }

private:
    QSet<QByteArray> m_extensions;
};

class Q_GUI_EXPORT QOpenGLConfig
{
public:
    struct Q_GUI_EXPORT Gpu {
        Gpu() : vendorId(0), deviceId(0) {}
        bool isValid() const { return deviceId || !glVendor.isEmpty(); }
        bool equals(const Gpu &other) const {
            return vendorId == other.vendorId && deviceId == other.deviceId && driverVersion == other.driverVersion
                && driverDescription == other.driverDescription && glVendor == other.glVendor;
        }

        uint vendorId;
        uint deviceId;
        QVersionNumber driverVersion;
        QByteArray driverDescription;
        QByteArray glVendor;

        static Gpu fromDevice(uint vendorId, uint deviceId, QVersionNumber driverVersion, const QByteArray &driverDescription) {
            Gpu gpu;
            gpu.vendorId = vendorId;
            gpu.deviceId = deviceId;
            gpu.driverVersion = driverVersion;
            gpu.driverDescription = driverDescription;
            return gpu;
        }

        static Gpu fromGLVendor(const QByteArray &glVendor) {
            Gpu gpu;
            gpu.glVendor = glVendor;
            return gpu;
        }

        static Gpu fromContext();
    };

    static QSet<QString> gpuFeatures(const Gpu &gpu, const QString &osName, const QString &osVersion,
                                     const QJsonDocument &doc);

    static QSet<QString> gpuFeatures(const Gpu &gpu, const QString &osName, const QString &osVersion,
                                     const QString &fileName);

    static QSet<QString> gpuFeatures(const Gpu &gpu, const QJsonDocument &doc);
    static QSet<QString> gpuFeatures(const Gpu &gpu, const QString &fileName);
};

inline bool operator==(const QOpenGLConfig::Gpu &a, const QOpenGLConfig::Gpu &b)
{
    return a.equals(b);
}

inline bool operator!=(const QOpenGLConfig::Gpu &a, const QOpenGLConfig::Gpu &b)
{
    return !a.equals(b);
}

inline uint qHash(const QOpenGLConfig::Gpu &gpu)
{
    return qHash(gpu.vendorId) + qHash(gpu.deviceId) + qHash(gpu.driverVersion);
}

#endif
