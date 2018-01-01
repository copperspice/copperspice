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

/********************************************************
**  This file is part of the KDE project.
********************************************************/

#ifndef PHONON_BACKEND_H
#define PHONON_BACKEND_H

#include <phonon/backendinterface.h>
#include <phonon/phononnamespace.h>

#include <QtCore/QList>



QT_BEGIN_NAMESPACE

namespace Phonon
{
    namespace WaveOut
    {
        class AudioOutput;
        class MediaObject;

        class Backend : public QObject, public Phonon::BackendInterface
        {
            PHN_CS_OBJECT(Backend)
            CS_INTERFACES(Phonon::BackendInterface)

        public:
            Backend(QObject *parent = nullptr, const QVariantList & = QVariantList());
            virtual ~Backend();

            QObject *createObject(Phonon::BackendInterface::Class, QObject *parent, const QList<QVariant> &args);

            bool supportsVideo() const;
            QStringList availableMimeTypes() const;

            QList<int> objectDescriptionIndexes(Phonon::ObjectDescriptionType type) const;
            QHash<QByteArray, QVariant> objectDescriptionProperties(Phonon::ObjectDescriptionType type, int index) const;

            bool connectNodes(QObject *, QObject *);
            bool disconnectNodes(QObject *, QObject *);

            //transaction management
            bool startConnectionChange(QSet<QObject *>);
            bool endConnectionChange(QSet<QObject *>);
       
            PHN_CS_SIGNAL_1(Public, void objectDescriptionChanged(ObjectDescriptionType un_named_arg1))
            PHN_CS_SIGNAL_2(objectDescriptionChanged,un_named_arg1) 

        };
    }
}

QT_END_NAMESPACE

#endif // PHONON_BACKEND_H
