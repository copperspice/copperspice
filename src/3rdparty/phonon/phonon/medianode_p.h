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
**  Copyright (C) 2005-2006 Matthias Kretz <kretz@kde.org
********************************************************/

#ifndef PHONON_MEDIANODE_P_H
#define PHONON_MEDIANODE_P_H

#include <QtCore/QtGlobal>
#include <QtCore/QList>
#include <QtCore/QObject>
#include "path.h"
#include "phonon_export.h"

QT_BEGIN_NAMESPACE

class QObject;

namespace Phonon
{
    class MediaNode;
    class MediaNodeDestructionHandler;

    class PHONON_EXPORT MediaNodePrivate
    {
       Q_DECLARE_PUBLIC(MediaNode)

       friend class AudioOutputPrivate;
       friend class FactoryPrivate;

       protected:
           enum CastId {
               MediaNodePrivateType,
               AbstractAudioOutputPrivateType,
               AudioOutputType
           };
   
       public:        
           QObject *backendObject();   
           const CastId castId;

           void addDestructionHandler(MediaNodeDestructionHandler *handler);      
           void removeDestructionHandler(MediaNodeDestructionHandler *handler);
   
           void addOutputPath(const Path &);
           void addInputPath(const Path &);
           void removeOutputPath(const Path &);
           void removeInputPath(const Path &);
   
           const QObject *qObject() const { return const_cast<MediaNodePrivate *>(this)->qObject(); }
           virtual QObject *qObject() { return 0; }

           QObject *m_backendObject;

       protected:
           MediaNodePrivate(CastId _castId = MediaNodePrivateType);   
           virtual ~MediaNodePrivate();   
          
           void deleteBackendObject();   
           virtual bool aboutToDeleteBackendObject() = 0;                    
           virtual void createBackendObject() = 0;
         
           MediaNode *q_ptr;
     
           QList<Path> outputPaths;
           QList<Path> inputPaths;

       private:
           QList<MediaNodeDestructionHandler *> handlers;
           Q_DISABLE_COPY(MediaNodePrivate)
    };

} // namespace Phonon

QT_END_NAMESPACE

#endif 