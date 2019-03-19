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

#include "seekslider.h"
#include "seekslider_p.h"
#include "mediaobject.h"
#include "phonondefs_p.h"

#include <QtGui/QMouseEvent>
#include <QtGui/QApplication>

QT_BEGIN_NAMESPACE

#ifndef QT_NO_PHONON_SEEKSLIDER

namespace Phonon
{

SeekSlider::SeekSlider(QWidget *parent)
    : QWidget(parent), k_ptr(new SeekSliderPrivate(this))
{
    K_D(SeekSlider);
    connect(&d->slider, SIGNAL(valueChanged(int)), this, SLOT(_k_seek(int)));
}

SeekSlider::SeekSlider(MediaObject *mo, QWidget *parent)
    : QWidget(parent), k_ptr(new SeekSliderPrivate(this))
{
    K_D(SeekSlider);
    connect(&d->slider, SIGNAL(valueChanged(int)), this, SLOT(_k_seek(int)));
    setMediaObject(mo);
}

/*SeekSlider::SeekSlider(SeekSliderPrivate &_d, QWidget *parent)
    : QWidget(parent), k_ptr(&_d)
{
} */

SeekSlider::~SeekSlider()
{
    delete k_ptr;
}

void SeekSlider::setMediaObject(MediaObject *media)
{
    K_D(SeekSlider);
    if (d->media) {
        disconnect(d->media, 0, this, 0);
    }
    d->media = media;

    if (media) {
        connect(media, SIGNAL(stateChanged(Phonon::State, Phonon::State)),       this, SLOT(_k_stateChanged(Phonon::State)));
        connect(media, SIGNAL(totalTimeChanged(qint64)),  this, SLOT(_k_length(qint64)));
        connect(media, SIGNAL(tick(qint64)),              this, SLOT(_k_tick(qint64)));
        connect(media, SIGNAL(seekableChanged(bool)),     this, SLOT(_k_seekableChanged(bool)));

        connect(media, SIGNAL(currentSourceChanged(const Phonon::MediaSource&)), this, SLOT(_k_currentSourceChanged()));

        d->_k_stateChanged(media->state());
        d->_k_seekableChanged(media->isSeekable());
        d->_k_length(media->totalTime());

    } else {
        d->_k_stateChanged(Phonon::StoppedState);
        d->_k_seekableChanged(false);
    }
}

MediaObject *SeekSlider::mediaObject() const
{
    K_D(const SeekSlider);
    return d->media;
}

void SeekSliderPrivate::_k_seek(int msec)
{
    if (!ticking && media) {
        media->seek(msec);
    }
}

void SeekSliderPrivate::_k_tick(qint64 msec)
{
    ticking = true;
    slider.setValue(msec);
    ticking = false;
}

void SeekSliderPrivate::_k_length(qint64 msec)
{
    ticking = true;
    slider.setRange(0, msec);
    ticking = false;
}

void SeekSliderPrivate::_k_seekableChanged(bool isSeekable)
{
    if (!isSeekable || !media) {
        setEnabled(false);
    } else {
        switch (media->state()) {
        case Phonon::PlayingState:
            if (media->tickInterval() == 0) {
                // if the tick signal is not enabled the slider is useless
                // set the tickInterval to some common value
                media->setTickInterval(350);
            }
        case Phonon::BufferingState:
        case Phonon::PausedState:
            setEnabled(true);
            break;
        case Phonon::StoppedState:
        case Phonon::LoadingState:
        case Phonon::ErrorState:
            setEnabled(false);
            ticking = true;
            slider.setValue(0);
            ticking = false;
            break;
        }
    }
}

void SeekSliderPrivate::_k_currentSourceChanged()
{
    //this releases the mouse and makes the seek slider stop seeking if the current source has changed
    QMouseEvent event(QEvent::MouseButtonRelease, QPoint(), Qt::LeftButton, 0, 0);
    QApplication::sendEvent(&slider, &event);
}

void SeekSliderPrivate::setEnabled(bool x)
{
    slider.setEnabled(x);
    iconLabel.setPixmap(icon.pixmap(iconSize, x ? QIcon::Normal : QIcon::Disabled));
}

void SeekSliderPrivate::_k_stateChanged(State newstate)
{
    if (!media || !media->isSeekable()) {
        setEnabled(false);
        return;
    }
    switch (newstate) {
    case Phonon::PlayingState:
        if (media->tickInterval() == 0) {
            // if the tick signal is not enabled the slider is useless
            // set the tickInterval to some common value
            media->setTickInterval(350);
        }
    case Phonon::BufferingState:
    case Phonon::PausedState:
        setEnabled(true);
        break;
    case Phonon::StoppedState:
    case Phonon::LoadingState:
    case Phonon::ErrorState:
        setEnabled(false);
        ticking = true;
        slider.setValue(0);
        ticking = false;
        break;
    }
}

bool SeekSlider::hasTracking() const
{
    return k_ptr->slider.hasTracking();
}

void SeekSlider::setTracking(bool tracking)
{
    k_ptr->slider.setTracking(tracking);
}

int SeekSlider::pageStep() const
{
    return k_ptr->slider.pageStep();
}

void SeekSlider::setPageStep(int milliseconds)
{
    k_ptr->slider.setPageStep(milliseconds);
}

int SeekSlider::singleStep() const
{
    return k_ptr->slider.singleStep();
}

void SeekSlider::setSingleStep(int milliseconds)
{
    k_ptr->slider.setSingleStep(milliseconds);
}

bool SeekSlider::isIconVisible() const
{
    K_D(const SeekSlider);
    return d->iconLabel.isVisible();
}

void SeekSlider::setIconVisible(bool vis)
{
    K_D(SeekSlider);
    d->iconLabel.setVisible(vis);
}

Qt::Orientation SeekSlider::orientation() const
{
    return k_ptr->slider.orientation();
}

void SeekSlider::setOrientation(Qt::Orientation o)
{
    K_D(SeekSlider);
    Qt::Alignment align = (o == Qt::Horizontal ? Qt::AlignVCenter : Qt::AlignHCenter);
    d->layout.setAlignment(&d->iconLabel, align);
    d->layout.setAlignment(&d->slider, align);
    d->layout.setDirection(o == Qt::Horizontal ? QBoxLayout::LeftToRight : QBoxLayout::TopToBottom);
    d->slider.setOrientation(o);
}

QSize SeekSlider::iconSize() const
{
    return k_ptr->iconSize;
}

void SeekSlider::setIconSize(const QSize &iconSize)
{
   K_D(SeekSlider);
   d->iconSize = iconSize;
   d->iconLabel.setPixmap(d->icon.pixmap(d->iconSize, d->slider.isEnabled() ? QIcon::Normal : QIcon::Disabled));
}

//
void SeekSlider::_k_stateChanged(Phonon::State un_named_arg1)
{
	K_D(SeekSlider);
	d->_k_stateChanged(un_named_arg1);
}

void SeekSlider::_k_seek(int un_named_arg1)
{
	K_D(SeekSlider);
	d->_k_seek(un_named_arg1);
}

void SeekSlider::_k_tick(qint64 un_named_arg1)
{
	K_D(SeekSlider);
	d->_k_tick(un_named_arg1);
}

void SeekSlider::_k_length(qint64 un_named_arg1)
{
	K_D(SeekSlider);
	d->_k_length(un_named_arg1);
}

void SeekSlider::_k_seekableChanged(bool un_named_arg1)
{
	K_D(SeekSlider);
	d->_k_seekableChanged(un_named_arg1);
}

void SeekSlider::_k_currentSourceChanged()
{
	K_D(SeekSlider);
	d->_k_currentSourceChanged();
}













} // namespace Phonon

#endif //QT_NO_PHONON_SEEKSLIDER

QT_END_NAMESPACE

