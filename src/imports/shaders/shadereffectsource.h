/***********************************************************************
*
* Copyright (c) 2012-2024 Barbara Geller
* Copyright (c) 2012-2024 Ansel Sermersheim
*
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

#ifndef SHADEREFFECTSOURCE_H
#define SHADEREFFECTSOURCE_H

#include <QDeclarativeItem>
#include <QtOpenGL>

QT_BEGIN_NAMESPACE

class ShaderEffectBuffer;

class ShaderEffectSource : public QDeclarativeItem
{
    Q_OBJECT
    Q_PROPERTY(QDeclarativeItem *sourceItem READ sourceItem WRITE setSourceItem NOTIFY sourceItemChanged)
    Q_PROPERTY(QRectF sourceRect READ sourceRect WRITE setSourceRect NOTIFY sourceRectChanged)
    Q_PROPERTY(QSize textureSize READ textureSize WRITE setTextureSize NOTIFY textureSizeChanged)
    Q_PROPERTY(bool live READ isLive WRITE setLive NOTIFY liveChanged)
    Q_PROPERTY(bool hideSource READ hideSource WRITE setHideSource NOTIFY hideSourceChanged)
    Q_PROPERTY(WrapMode wrapMode READ wrapMode WRITE setWrapMode NOTIFY wrapModeChanged)
    Q_ENUMS(WrapMode)
    Q_ENUMS(Format)

public:
    enum WrapMode {
            ClampToEdge,
            RepeatHorizontally,
            RepeatVertically,
            Repeat
        };

    enum Format {
        Alpha = GL_ALPHA,
        RGB = GL_RGB,
        RGBA = GL_RGBA
    };

    ShaderEffectSource(QDeclarativeItem *parent = 0);
    virtual ~ShaderEffectSource();

    QDeclarativeItem *sourceItem() const { return m_sourceItem.data(); }
    void setSourceItem(QDeclarativeItem *item);

    QRectF sourceRect() const { return m_sourceRect; };
    void setSourceRect(const QRectF &rect);

    QSize textureSize() const { return m_textureSize; }
    void setTextureSize(const QSize &size);

    bool isLive() const { return m_live; }
    void setLive(bool s);

    bool hideSource() const { return m_hideSource; }
    void setHideSource(bool hide);

    WrapMode wrapMode() const { return m_wrapMode; };
    void setWrapMode(WrapMode mode);

    bool isActive() const { return m_refs; }
    void bind();
    void refFromEffectItem();
    void derefFromEffectItem();
    void updateBackbuffer();

    ShaderEffectBuffer* fbo() { return m_fbo; }
    bool isDirtyTexture() { return m_dirtyTexture; }
    bool isMirrored() { return m_mirrored; }

    Q_INVOKABLE void grab();

Q_SIGNALS:
    void sourceItemChanged();
    void sourceRectChanged();
    void textureSizeChanged();
    void formatChanged();
    void liveChanged();
    void hideSourceChanged();
    void activeChanged();
    void repaintRequired();
    void wrapModeChanged();

public Q_SLOTS:
    void markSceneGraphDirty();
    void markSourceSizeDirty();
    void markSourceItemDirty();

private:
    void updateSizeAndTexture();
    void attachSourceItem();
    void detachSourceItem();

    QPointer<QDeclarativeItem> m_sourceItem;
    WrapMode m_wrapMode;
    QRectF m_sourceRect;
    QSize m_textureSize;
    Format m_format;
    QSize m_size;

    ShaderEffectBuffer *m_fbo;
    ShaderEffectBuffer *m_multisampledFbo;
    int m_refs;
    bool m_dirtyTexture : 1;
    bool m_dirtySceneGraph : 1;
    bool m_multisamplingSupported : 1;
    bool m_checkedForMultisamplingSupport : 1;
    bool m_live : 1;
    bool m_hideSource : 1;
    bool m_mirrored : 1;
};


QT_END_NAMESPACE


#endif // SHADEREFFECTSOURCE_H
