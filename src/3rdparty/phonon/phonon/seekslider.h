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

#ifndef PHONON_SEEKSLIDER_H
#define PHONON_SEEKSLIDER_H

#include <phonon_export.h>
#include <phonondefs.h>
#include <phononnamespace.h>
#include <QWidget>

QT_BEGIN_NAMESPACE

#ifndef QT_NO_PHONON_SEEKSLIDER

namespace Phonon
{

class MediaObject;
class SeekSliderPrivate;

class PHONON_EXPORT SeekSlider : public QWidget
{
    PHN_CS_OBJECT(SeekSlider)
    K_DECLARE_PRIVATE(SeekSlider)

    PHN_CS_PROPERTY_READ(iconVisible, isIconVisible)
    PHN_CS_PROPERTY_WRITE(iconVisible, setIconVisible)

    PHN_CS_PROPERTY_READ(tracking, hasTracking)
    PHN_CS_PROPERTY_WRITE(tracking, setTracking)
   
    PHN_CS_PROPERTY_READ(pageStep, pageStep)
    PHN_CS_PROPERTY_WRITE(pageStep, setPageStep)

    PHN_CS_PROPERTY_READ(singleStep, singleStep)
    PHN_CS_PROPERTY_WRITE(singleStep, setSingleStep)

    PHN_CS_PROPERTY_READ(orientation, orientation)
    PHN_CS_PROPERTY_WRITE(orientation, setOrientation)
  
    PHN_CS_PROPERTY_READ(iconSize, iconSize)
    PHN_CS_PROPERTY_WRITE(iconSize, setIconSize)

    public:       
        explicit SeekSlider(QWidget *parent = nullptr);
        explicit SeekSlider(MediaObject *media, QWidget *parent = nullptr);
       
        ~SeekSlider();

        bool hasTracking() const;
        void setTracking(bool tracking);
        int pageStep() const;
        void setPageStep(int milliseconds);
        int singleStep() const;
        void setSingleStep(int milliseconds);
        Qt::Orientation orientation() const;
        bool isIconVisible() const;
        QSize iconSize() const;
        MediaObject *mediaObject() const;
    
        PHN_CS_SLOT_1(Public, void setOrientation(Qt::Orientation un_named_arg1))
        PHN_CS_SLOT_2(setOrientation) 

        PHN_CS_SLOT_1(Public, void setIconVisible(bool un_named_arg1))
        PHN_CS_SLOT_2(setIconVisible) 

        PHN_CS_SLOT_1(Public, void setIconSize(const QSize & size))
        PHN_CS_SLOT_2(setIconSize) 
        
        PHN_CS_SLOT_1(Public, void setMediaObject(MediaObject * un_named_arg1))
        PHN_CS_SLOT_2(setMediaObject) 

    protected:
        SeekSliderPrivate *const k_ptr;

    private:
        PHN_CS_SLOT_1(Private, void _k_stateChanged(Phonon::State un_named_arg1))
        PHN_CS_SLOT_2(_k_stateChanged)

        PHN_CS_SLOT_1(Private, void _k_seek(int un_named_arg1))
        PHN_CS_SLOT_2(_k_seek)

        PHN_CS_SLOT_1(Private, void _k_tick(qint64 un_named_arg1))
        PHN_CS_SLOT_2(_k_tick)

        PHN_CS_SLOT_1(Private, void _k_length(qint64 un_named_arg1))
        PHN_CS_SLOT_2(_k_length)

        PHN_CS_SLOT_1(Private, void _k_seekableChanged(bool un_named_arg1))
        PHN_CS_SLOT_2(_k_seekableChanged)

        PHN_CS_SLOT_1(Private, void _k_currentSourceChanged())
        PHN_CS_SLOT_2(_k_currentSourceChanged)
};

} // namespace Phonon

#endif //QT_NO_PHONON_SEEKSLIDER

QT_END_NAMESPACE

#endif
