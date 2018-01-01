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

#ifndef PHONON_AUDIOOUTPUTADAPTOR_P_H
#define PHONON_AUDIOOUTPUTADAPTOR_P_H

#include <QtCore/QObject>

#ifndef QT_NO_DBUS
#include <QtDBus/QtDBus>

QT_BEGIN_NAMESPACE

class QByteArray;
template<class T> class QList;
template<class Key, class Value> class QMap;
class QString;
class QStringList;
class QVariant;

namespace Phonon
{
    class AudioOutputPrivate;
    class AudioOutput;

/*
 * Adaptor class for interface org.kde.Phonon.AudioOutput
 */
class AudioOutputAdaptor: public QDBusAbstractAdaptor
{
    friend class Phonon::AudioOutputPrivate;
    friend class Phonon::AudioOutput;

    PHN_CS_OBJECT(AudioOutputAdaptor)

    PHN_CS_CLASSINFO("D-Bus Interface", "org.kde.Phonon.AudioOutput")
    PHN_CS_CLASSINFO("D-Bus Introspection", ""
"    <interface name=\"org.kde.Phonon.AudioOutput\" >\n"
"    <property access=\"readwrite\" type=\"d\" name=\"volume\" />\n"
"    <property access=\"readwrite\" type=\"b\" name=\"muted\" />\n"
"    <property access=\"readwrite\" type=\"i\" name=\"outputDeviceIndex\" />\n"
"    <signal name=\"volumeChanged\" >\n"
"      <arg direction=\"out\" type=\"d\" />\n"
"    </signal>\n"
"    <signal name=\"mutedChanged\" >\n"
"      <arg direction=\"out\" type=\"b\" />\n"
"    </signal>\n"
"    <signal name=\"outputDeviceIndexChanged\" >\n"
"      <arg direction=\"out\" type=\"i\" />\n"
"    </signal>\n"
"    <signal name=\"nameChanged\" >\n"
"      <arg direction=\"out\" type=\"s\" name=\"newName\" />\n"
"    </signal>\n"
"    <signal name=\"newOutputAvailable\" >\n"
"      <arg direction=\"out\" type=\"s\" name=\"service\" />\n"
"      <arg direction=\"out\" type=\"s\" name=\"path\" />\n"
"    </signal>\n"
"    <signal name=\"outputDestroyed\" >\n"
"    </signal>\n"
"    <method name=\"category\" >\n"
"      <arg direction=\"out\" type=\"s\" />\n"
"    </method>\n"
"    <method name=\"name\" >\n"
"      <arg direction=\"out\" type=\"s\" />\n"
"    </method>\n"
"    </interface>\n"
        "")
public:
    AudioOutputAdaptor(QObject *parent);
    virtual ~AudioOutputAdaptor();

    PHN_CS_PROPERTY_READ(muted, muted)
    PHN_CS_PROPERTY_WRITE(muted, setMuted)
    bool muted() const;
    void setMuted(bool value);

    PHN_CS_PROPERTY_READ(outputDeviceIndex, outputDeviceIndex)
    PHN_CS_PROPERTY_WRITE(outputDeviceIndex, setOutputDeviceIndex)
    int outputDeviceIndex() const;
    void setOutputDeviceIndex(int value);

    PHN_CS_PROPERTY_READ(volume, volume)
    PHN_CS_PROPERTY_WRITE(volume, setVolume)
    double volume() const;
    void setVolume(double value);

    PHN_CS_SLOT_1(Public, QString category())
    PHN_CS_SLOT_2(category) 
    PHN_CS_SLOT_1(Public, QString name())
    PHN_CS_SLOT_2(name) 

    PHN_CS_SIGNAL_1(Public, void mutedChanged(bool in0))
    PHN_CS_SIGNAL_2(mutedChanged,in0) 
    PHN_CS_SIGNAL_1(Public, void nameChanged(const QString & newName))
    PHN_CS_SIGNAL_2(nameChanged,newName) 
    PHN_CS_SIGNAL_1(Public, void newOutputAvailable(const QString & service,const QString & path))
    PHN_CS_SIGNAL_2(newOutputAvailable,service,path) 
    PHN_CS_SIGNAL_1(Public, void outputDestroyed())
    PHN_CS_SIGNAL_2(outputDestroyed) 
    PHN_CS_SIGNAL_1(Public, void outputDeviceIndexChanged(int in0))
    PHN_CS_SIGNAL_2(outputDeviceIndexChanged,in0) 
    PHN_CS_SIGNAL_1(Public, void volumeChanged(qreal in0))
    PHN_CS_SIGNAL_2(volumeChanged,in0) 
};

} // namespace Phonon

QT_END_NAMESPACE

#endif // QT_NO_DBUS

#endif // AUDIOOUTPUTADAPTOR_P_H
