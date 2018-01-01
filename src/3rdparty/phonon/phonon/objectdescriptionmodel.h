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

#ifndef PHONON_OBJECTDESCRIPTIONMODEL_H
#define PHONON_OBJECTDESCRIPTIONMODEL_H

#include <phonon_export.h>
#include <phonondefs.h>
#include <objectdescription.h>
#include <QList>
#include <QModelIndex>
#include <QStringList>

QT_BEGIN_NAMESPACE

#ifndef QT_NO_PHONON_OBJECTDESCRIPTIONMODEL

namespace Phonon
{
    class ObjectDescriptionModelDataPrivate;

    // internal
    class PHONON_EXPORT ObjectDescriptionModelData
    {
        public:
           
            int rowCount(const QModelIndex &parent = QModelIndex()) const;
            QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;            

            Qt::ItemFlags flags(const QModelIndex &index) const;           
            QList<int> tupleIndexOrder() const;          
           
            int tupleIndexAtPositionIndex(int positionIndex) const;
            QMimeData *mimeData(ObjectDescriptionType type, const QModelIndexList &indexes) const;

            void moveUp(const QModelIndex &index);           
            void moveDown(const QModelIndex &index);

            void setModelData(const QList<QExplicitlySharedDataPointer<ObjectDescriptionData> > &data);
            QList<QExplicitlySharedDataPointer<ObjectDescriptionData> > modelData() const;
            QExplicitlySharedDataPointer<ObjectDescriptionData> modelData(const QModelIndex &index) const;
            Qt::DropActions supportedDropActions() const;

            bool dropMimeData(ObjectDescriptionType type, const QMimeData *data, Qt::DropAction action, 
                  int row, int column, const QModelIndex &parent);

            bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex());
            QStringList mimeTypes(ObjectDescriptionType type) const;

            ObjectDescriptionModelData(QAbstractListModel *);

        protected:
            ~ObjectDescriptionModelData();

            //ObjectDescriptionModelData(ObjectDescriptionModelDataPrivate *dd);
            ObjectDescriptionModelDataPrivate *const d;
    };


/* Required to ensure template class vtables are exported on both symbian and existing builds. */
#if defined(Q_CC_CLANG)

// clang also requires the export declaration to be on the class to export vtables
#define PHONON_TEMPLATE_CLASS_EXPORT           PHONON_EXPORT
#define PHONON_TEMPLATE_CLASS_MEMBER_EXPORT
#else

// Windows builds (at least) do not support export declaration on templated class
// But if you export a member function, the vtable is implicitly exported
#define PHONON_TEMPLATE_CLASS_EXPORT
#define PHONON_TEMPLATE_CLASS_MEMBER_EXPORT    PHONON_EXPORT
#endif


template<ObjectDescriptionType type>
class PHONON_TEMPLATE_CLASS_EXPORT ObjectDescriptionModel : public QAbstractListModel
{
   PHN_CS_OBJECT(ObjectDescriptionModel)

public:    
         
   inline int rowCount(const QModelIndex &parent = QModelIndex()) const { return d->rowCount(parent); } 
   inline QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const { return d->data(index, role); }

   inline Qt::ItemFlags flags(const QModelIndex &index) const { return d->flags(index); }
   inline QList<int> tupleIndexOrder() const { return d->tupleIndexOrder(); }
           
   inline int tupleIndexAtPositionIndex(int positionIndex) const { return d->tupleIndexAtPositionIndex(positionIndex); }          
   inline QMimeData *mimeData(const QModelIndexList &indexes) const { return d->mimeData(type, indexes); }
           
   inline void moveUp(const QModelIndex &index) { d->moveUp(index); }           
   inline void moveDown(const QModelIndex &index) { d->moveDown(index); }

   explicit inline ObjectDescriptionModel(QObject *parent = nullptr) : QAbstractListModel(parent), 
         d(new ObjectDescriptionModelData(this)) {} 
      
   explicit inline ObjectDescriptionModel(const QList<ObjectDescription<type> > &data, QObject *parent = nullptr) :
         QAbstractListModel(parent), d(new ObjectDescriptionModelData(this)) { setModelData(data); }
           
   inline void setModelData(const QList<ObjectDescription<type> > &data) 
      { 
          QList<QExplicitlySharedDataPointer<ObjectDescriptionData> > list;
          for (int i = 0; i < data.count(); ++i) {
              list += data.at(i).d;
          }
          d->setModelData(list);
      }
           
   inline QList<ObjectDescription<type> > modelData() const
      { 
                QList<ObjectDescription<type> > ret;
                QList<QExplicitlySharedDataPointer<ObjectDescriptionData> > list = d->modelData();
                for (int i = 0; i < list.count(); ++i) {
                    ret << ObjectDescription<type>(list.at(i));
                }
                return ret;
      }
           
   inline ObjectDescription<type> modelData(const QModelIndex &index) const
      { return ObjectDescription<type>(d->modelData(index)); } 

   inline Qt::DropActions supportedDropActions() const 
      { return d->supportedDropActions(); }
           
   inline bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) 
      { 
         return d->dropMimeData(type, data, action, row, column, parent);
      }

   inline bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) 
      { 
         return d->removeRows(row, count, parent);
      }
            
   inline QStringList mimeTypes() const 
      { return d->mimeTypes(type); } 

protected:
   ObjectDescriptionModelData *const d;

};

typedef ObjectDescriptionModel<AudioOutputDeviceType> AudioOutputDeviceModel;
typedef ObjectDescriptionModel<AudioCaptureDeviceType> AudioCaptureDeviceModel;
typedef ObjectDescriptionModel<EffectType> EffectDescriptionModel;
typedef ObjectDescriptionModel<AudioChannelType> AudioChannelDescriptionModel;
typedef ObjectDescriptionModel<SubtitleType> SubtitleDescriptionModel;


/* ** unused Qt 4 code 
    typedef ObjectDescriptionModel<VideoOutputDeviceType> VideoOutputDeviceModel;
    typedef ObjectDescriptionModel<VideoCaptureDeviceType> VideoCaptureDeviceModel;
    typedef ObjectDescriptionModel<AudioCodecType> AudioCodecDescriptionModel;
    typedef ObjectDescriptionModel<VideoCodecType> VideoCodecDescriptionModel;
    typedef ObjectDescriptionModel<ContainerFormatType> ContainerFormatDescriptionModel;
    typedef ObjectDescriptionModel<VisualizationType> VisualizationDescriptionModel;
*/

}

#endif //QT_NO_PHONON_OBJECTDESCRIPTIONMODEL

QT_END_NAMESPACE

#endif 