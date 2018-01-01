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

#ifndef PHONON_VIDEOWIDGET_P_H
#define PHONON_VIDEOWIDGET_P_H

#include "videowidget.h"
#include "abstractvideooutput_p.h"
#include <QtGui/QBoxLayout>
#include <qcoreevent.h>
#include <QtCore/QCoreApplication>
#include <QtGui/QPalette>
#include <QtGui/QKeyEvent>
#include <QtCore/QTimer>

QT_BEGIN_NAMESPACE

#ifndef QT_NO_PHONON_VIDEO

namespace Phonon
{

class VideoWidgetPrivate : public Phonon::AbstractVideoOutputPrivate
{
    Q_DECLARE_PUBLIC(VideoWidget)

    public:
        QObject *qObject() override { return q_func(); }

    protected:
        bool aboutToDeleteBackendObject() override;
        void createBackendObject() override;

        void setupBackendObject();

        VideoWidgetPrivate(VideoWidget *parent)
            : layout(parent), aspectRatio(VideoWidget::AspectRatioAuto),
              scaleMode(VideoWidget::FitInView), brightness(0),
              contrast(0), hue(0), saturation(0)
        {
            layout.setMargin(0);
        }

        QHBoxLayout layout;
        VideoWidget::AspectRatio aspectRatio;
        VideoWidget::ScaleMode scaleMode;
        Qt::WindowFlags changeFlags;

        qreal brightness;
        qreal contrast;
        qreal hue;
        qreal saturation;

    private:
        void init();
};

} // namespace Phonon

#endif //QT_NO_PHONON_VIDEO

QT_END_NAMESPACE
#endif // VIDEOWIDGET_P_H

