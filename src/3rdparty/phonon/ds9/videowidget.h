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

#ifndef DS9_VIDEOWIDGET_H
#define DS9_VIDEOWIDGET_H

#include <QtGui/QWidget>
#include <phonon/videowidgetinterface.h>
#include <backendnode.h>

QT_BEGIN_NAMESPACE

#ifndef QT_NO_PHONON_VIDEO

namespace Phonon
{
    namespace DS9
    {
        class VideoWindow;
        class AbstractVideoRenderer;

        class VideoWidget : public BackendNode, public Phonon::VideoWidgetInterface
        {
            enum RendererType
            {
                Native = 0,
                NonNative = 1
            };

            DS9_CS_OBJECT(VideoWidget)
                CS_INTERFACES(Phonon::VideoWidgetInterface)
        public:
            VideoWidget(QWidget *parent = nullptr);
            ~VideoWidget();

            Phonon::VideoWidget::AspectRatio aspectRatio() const override;
            void setAspectRatio(Phonon::VideoWidget::AspectRatio aspectRatio) override;
            Phonon::VideoWidget::ScaleMode scaleMode() const override;
            void setScaleMode(Phonon::VideoWidget::ScaleMode) override;
            qreal brightness() const override;
            void setBrightness(qreal) override;
            qreal contrast() const override;
            void setContrast(qreal) override;
            qreal hue() const override;
            void setHue(qreal) override;
            qreal saturation() const override;
            void setSaturation(qreal) override;

            void setCurrentGraph(int index);

            QWidget *widget() override;

            void notifyVideoLoaded();
            AbstractVideoRenderer *switchRendering(AbstractVideoRenderer *current);
            void performSoftRendering(const QImage &currentImage);

            //apply contrast/brightness/hue/saturation
            void applyMixerSettings() const;
            void updateVideoSize() const;

        protected:
            void connected(BackendNode *, const InputPin& inpin) override;

        private:
            AbstractVideoRenderer *getRenderer(int graphIndex, RendererType type, bool autoCreate = false);

            Phonon::VideoWidget::AspectRatio m_aspectRatio;
            Phonon::VideoWidget::ScaleMode m_scaleMode;

            VideoWindow *m_widget;
            qreal m_brightness, m_contrast, m_hue, m_saturation;
            AbstractVideoRenderer* m_renderers[4];
            mutable bool m_noNativeRendererSupported;
        };
    }
}

QT_END_NAMESPACE

#endif // PHONON_VIDEOWIDGET_H

#endif //QT_NO_PHONON_VIDEO
