/***********************************************************************
*
* Copyright (c) 2012-2019 Barbara Geller
* Copyright (c) 2012-2019 Ansel Sermersheim
*
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

#include <algorithm>

#include "backend.h"
#include "backendnode.h"

#include "audiooutput.h"
#include "effect.h"
#include "mediaobject.h"
#include "videowidget.h"
#include "volumeeffect.h"

//windows specific (DirectX Media Object)
#include <dmo.h>

#include <QtCore/QSettings>
#include <QtCore/QSet>
#include <QtCore/QVariant>
#include <QtCore/QtPlugin>

QT_BEGIN_NAMESPACE

Q_EXPORT_PLUGIN2(phonon_ds9, Phonon::DS9::Backend);

namespace Phonon
{
    namespace DS9
    {
        QMutex *Backend::directShowMutex = 0;

        bool Backend::AudioMoniker::operator==(const AudioMoniker &other) const
        {
            return other->IsEqual(*this) == S_OK;
        }


        Backend::Backend(QObject *parent, const QVariantList &)
            : QObject(parent)
        {
            directShowMutex = &m_directShowMutex;

            ::CoInitialize(0);

            //registering meta types
            qRegisterMetaType<HRESULT>("HRESULT");
            qRegisterMetaType<Graph>("Graph");
        }

        Backend::~Backend()
        {
            m_audioOutputs.clear();
            m_audioEffects.clear();
            ::CoUninitialize();

            directShowMutex = 0;
        }

        QObject *Backend::createObject(BackendInterface::Class c, QObject *parent, const QList<QVariant> &args)
        {
            switch (c)
            {
            case MediaObjectClass:
                return new MediaObject(parent);

            case AudioOutputClass:
                return new AudioOutput(this, parent);

#ifndef QT_NO_PHONON_EFFECT
            case EffectClass:
                return new Effect(m_audioEffects[ args[0].toInt() ], parent);
#endif

#ifndef QT_NO_PHONON_VIDEO
            case VideoWidgetClass:
                return new VideoWidget(qobject_cast<QWidget *>(parent));
#endif

#ifndef QT_NO_PHONON_VOLUMEFADEREFFECT
            case VolumeFaderEffectClass:
                return new VolumeEffect(parent);
#endif
            default:
                return 0;
            }
        }

        bool Backend::supportsVideo() const
        {
#ifndef QT_NO_PHONON_VIDEO
            return true;
#else
            return false;
#endif
        }

        QStringList Backend::availableMimeTypes() const
        {
            QStringList ret;
            {
                QSettings settings(QLatin1String("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Multimedia\\mplayer2\\mime types"),
                  QSettings::NativeFormat);
                ret += settings.childGroups();
            }
            {
                QSettings settings(QLatin1String("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Multimedia\\wmplayer\\mime types"),
                  QSettings::NativeFormat);
                ret += settings.childGroups();
            }

            ret.removeDuplicates();
            ret.replaceInStrings("\\", "/");
            std::sort(ret.begin(), ret.end());

            return ret;
        }

      Filter Backend::getAudioOutputFilter(int index) const
      {
         Filter ret;
         if (index >= 0 && index < m_audioOutputs.count()) {
            m_audioOutputs.at(index)->BindToObject(0, 0, IID_IBaseFilter, reinterpret_cast<void**>(&ret));
         } else {
            //just return the default audio renderer (not directsound)
            ret = Filter(CLSID_AudioRender, IID_IBaseFilter);
         }
         return ret;
      }


      QList<int> Backend::objectDescriptionIndexes(Phonon::ObjectDescriptionType type) const
        {
            QMutexLocker locker(&m_directShowMutex);
            QList<int> ret;

            switch(type)
            {
            case Phonon::AudioOutputDeviceType:
                {
                    ComPointer<ICreateDevEnum> devEnum(CLSID_SystemDeviceEnum, IID_ICreateDevEnum);

                    if (! devEnum) {
                      return ret;       // not impossible to enumerate the devices
                    }

                    ComPointer<IEnumMoniker> enumMon;
                    HRESULT hr = devEnum->CreateClassEnumerator(CLSID_AudioRendererCategory, enumMon.pparam(), 0);
                    if (FAILED(hr)) {
                        break;
                    }
                    AudioMoniker mon;


                    // reorder the devices so direct sound appears first
                    int nbds = 0; //number of direct sound devices

                    while (S_OK == enumMon->Next(1, mon.pparam(), 0)) {
                        LPOLESTR str = 0;
                        mon->GetDisplayName(0, 0, &str);

                        std::wstring tmp(str);
                        const QString name = QString::fromStdWString(tmp);

                        ComPointer<IMalloc> alloc;
                        ::CoGetMalloc(1, alloc.pparam());

                        alloc->Free(str);

                        int insert_pos = 0;

                        if (m_audioOutputs.contains(mon)) {
                           insert_pos = m_audioOutputs.indexOf(mon);

                        } else {
                            insert_pos = m_audioOutputs.count();
                            m_audioOutputs.append(mon);

                        }

                        if (name.contains("DirectSound")) {
                            ret.insert(nbds++, insert_pos);

                        } else {
                            ret.append(insert_pos);
                        }
                    }

                    break;
                }

#ifndef QT_NO_PHONON_EFFECT
            case Phonon::EffectType:
                {
                    m_audioEffects.clear();
                    ComPointer<IEnumDMO> enumDMO;
                    HRESULT hr = ::DMOEnum(DMOCATEGORY_AUDIO_EFFECT, DMO_ENUMF_INCLUDE_KEYED, 0, 0, 0, 0, enumDMO.pparam());

                    if (SUCCEEDED(hr)) {
                        CLSID clsid;
                        while (S_OK == enumDMO->Next(1, &clsid, 0, 0)) {
                            ret += m_audioEffects.count();
                            m_audioEffects.append(clsid);
                        }
                    }
                    break;
                }
                break;
#endif
            default:
                break;
            }

         return ret;
        }

        QHash<QByteArray, QVariant> Backend::objectDescriptionProperties(Phonon::ObjectDescriptionType type, int index) const
        {
            QMutexLocker locker(&m_directShowMutex);
            QHash<QByteArray, QVariant> ret;
            switch (type)
            {
            case Phonon::AudioOutputDeviceType:
                {
                    const AudioMoniker &mon = m_audioOutputs[index];
                    LPOLESTR str = 0;
                    HRESULT hr   = mon->GetDisplayName(0,0, &str);

                    if (SUCCEEDED(hr)) {
                        std::wstring tmp(str);
                        QString name = QString::fromStdWString(tmp);

                        ComPointer<IMalloc> alloc;

                        ::CoGetMalloc(1, alloc.pparam());
                        alloc->Free(str);
                        ret["name"] = name.mid(name.indexOf('\\') + 1);
                }

                }
                break;

#ifndef QT_NO_PHONON_EFFECT
            case Phonon::EffectType:
                {
                    WCHAR name[80]; // 80 is clearly stated in the MSDN doc
                    HRESULT hr = ::DMOGetName(m_audioEffects[index], name);

                    if (SUCCEEDED(hr)) {
                       std::wstring tmp(name);
                       ret["name"] = QString::fromStdWString(tmp);
                    }
                }
                break;
#endif
            default:
                break;
            }

         return ret;
        }

        bool Backend::endConnectionChange(QSet<QObject *> objects)
        {
            //end of a transaction
            for(QSet<QObject *>::const_iterator it = objects.begin(); it != objects.end(); ++it) {
                if (BackendNode *node = qobject_cast<BackendNode*>(*it)) {
                    MediaObject *mo = node->mediaObject();
                    if (mo) {
                        switch(mo->transactionState)
                        {
                        case Phonon::ErrorState:
                        case Phonon::StoppedState:
                        case Phonon::LoadingState:
                            //nothing to do
                            break;
                        case Phonon::PausedState:
                            mo->transactionState = Phonon::StoppedState;
                            mo->pause();
                            break;
                        default:
                            mo->transactionState = Phonon::StoppedState;
                            mo->play();
                            break;
                        }

                        if (mo->state() == Phonon::ErrorState)
                            return false;
                    }
                }
            }

            return true;
        }


        bool Backend::startConnectionChange(QSet<QObject *> objects)
        {
            //let's save the state of the graph (before we stop it)
            for(QSet<QObject *>::const_iterator it = objects.begin(); it != objects.end(); ++it) {
                if (BackendNode *node = qobject_cast<BackendNode*>(*it)) {
                    if (MediaObject *mo = node->mediaObject()) {
                        if (mo->state() != Phonon::StoppedState) {
                            mo->transactionState = mo->state();
                            mo->ensureStopped(); //we have to stop the graph..
                            if (mo->state() == Phonon::ErrorState)
                                return false;
                        }
                    }
                }
            }

            return true;
        }

        bool Backend::connectNodes(QObject *_source, QObject *_sink)
        {
            BackendNode *source = qobject_cast<BackendNode*>(_source);
            if (!source) {
                return false;
            }
            BackendNode *sink = qobject_cast<BackendNode*>(_sink);
            if (!sink) {
                return false;
            }

            //setting the graph if needed
            if (source->mediaObject() == 0 && sink->mediaObject() == 0) {
                    //error: no graph selected
                    return false;
            } else if (source->mediaObject() && source->mediaObject() != sink->mediaObject()) {
                //this' graph becomes the common one
                source->mediaObject()->grabNode(sink);
            } else if (source->mediaObject() == 0) {
                //sink's graph becomes the common one
                sink->mediaObject()->grabNode(source);
            }

            return source->mediaObject()->connectNodes(source, sink);
        }

        bool Backend::disconnectNodes(QObject *_source, QObject *_sink)
        {
            BackendNode *source = qobject_cast<BackendNode*>(_source);
            if (!source) {
                return false;
            }
            BackendNode *sink = qobject_cast<BackendNode*>(_sink);
            if (!sink) {
                return false;
            }

            return source->mediaObject() == 0 ||
                source->mediaObject()->disconnectNodes(source, sink);
        }
    }
}

QT_END_NAMESPACE

