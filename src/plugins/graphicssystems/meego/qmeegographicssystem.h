/***********************************************************************
*
* Copyright (c) 2012-2014 Barbara Geller
* Copyright (c) 2012-2014 Ansel Sermersheim
* Copyright (c) 2012-2014 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
*
* This file is part of CopperSpice.
*
* CopperSpice is free software: you can redistribute it and/or 
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with CopperSpice.  If not, see 
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#ifndef MGRAPHICSSYSTEM_H
#define MGRAPHICSSYSTEM_H

#include <private/qgraphicssystem_p.h>
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

extern "C" typedef void (*QMeeGoSwitchCallback)(int type, const char *name);

class QMeeGoGraphicsSystem : public QGraphicsSystem
{
public:
    enum SwitchPolicy { AutomaticSwitch, ManualSwitch, NoSwitch };

    QMeeGoGraphicsSystem();
    ~QMeeGoGraphicsSystem();

    virtual QWindowSurface *createWindowSurface(QWidget *widget) const;
    virtual QPixmapData *createPixmapData(QPixmapData::PixelType) const;
    virtual QPixmapData *createPixmapData(QPixmapData *origin);

    static QPixmapData *wrapPixmapData(QPixmapData *pmd);
    static void setSurfaceFixedSize(int width, int height);
    static void setSurfaceScaling(int x, int y, int width, int height);
    static void setTranslucent(bool translucent);

    static QPixmapData *pixmapDataFromEGLSharedImage(Qt::HANDLE handle, const QImage &softImage);
    static QPixmapData *pixmapDataFromEGLImage(Qt::HANDLE handle);
    static QPixmapData *pixmapDataWithGLTexture(int w, int h);
    static void updateEGLSharedImagePixmap(QPixmap *pixmap);

    static QPixmapData *pixmapDataWithNewLiveTexture(int w, int h, QImage::Format format);
    static QPixmapData *pixmapDataFromLiveTextureHandle(Qt::HANDLE handle);
    static QImage *lockLiveTexture(QPixmap* pixmap, void* fenceSync);
    static bool releaseLiveTexture(QPixmap *pixmap, QImage *image);
    static Qt::HANDLE getLiveTextureHandle(QPixmap *pixmap);

    static void* createFenceSync();
    static void destroyFenceSync(void* fenceSync);

    static void switchToRaster();
    static void switchToMeeGo();
    static QString runningGraphicsSystemName();

    static void registerSwitchCallback(QMeeGoSwitchCallback callback);

    static SwitchPolicy switchPolicy;

private:
    static bool meeGoRunning();
    static EGLSurface getSurfaceForLiveTexturePixmap(QPixmap *pixmap);
    static void destroySurfaceForLiveTexturePixmap(QPixmapData* pmd);
    static void triggerSwitchCallbacks(int type, const char *name);

    static bool surfaceWasCreated;
    static QHash<Qt::HANDLE, QPixmap*> liveTexturePixmaps;
    static QList<QMeeGoSwitchCallback> switchCallbacks;
};

/* C api */

extern "C" {
    Q_DECL_EXPORT int qt_meego_image_to_egl_shared_image(const QImage &image);
    Q_DECL_EXPORT QPixmapData* qt_meego_pixmapdata_from_egl_shared_image(Qt::HANDLE handle, const QImage &softImage);
    Q_DECL_EXPORT QPixmapData* qt_meego_pixmapdata_with_gl_texture(int w, int h);
    Q_DECL_EXPORT void qt_meego_update_egl_shared_image_pixmap(QPixmap *pixmap);
    Q_DECL_EXPORT bool qt_meego_destroy_egl_shared_image(Qt::HANDLE handle);
    Q_DECL_EXPORT void qt_meego_set_surface_fixed_size(int width, int height);
    Q_DECL_EXPORT void qt_meego_set_surface_scaling(int x, int y, int width, int height);
    Q_DECL_EXPORT void qt_meego_set_translucent(bool translucent);
    Q_DECL_EXPORT QPixmapData* qt_meego_pixmapdata_with_new_live_texture(int w, int h, QImage::Format format);
    Q_DECL_EXPORT QPixmapData* qt_meego_pixmapdata_from_live_texture_handle(Qt::HANDLE handle);
    Q_DECL_EXPORT QImage* qt_meego_live_texture_lock(QPixmap *pixmap, void *fenceSync);
    Q_DECL_EXPORT bool qt_meego_live_texture_release(QPixmap *pixmap, QImage *image);
    Q_DECL_EXPORT Qt::HANDLE qt_meego_live_texture_get_handle(QPixmap *pixmap);
    Q_DECL_EXPORT void* qt_meego_create_fence_sync(void);
    Q_DECL_EXPORT void qt_meego_destroy_fence_sync(void* fs);
    Q_DECL_EXPORT void qt_meego_invalidate_live_surfaces(void);
    Q_DECL_EXPORT void qt_meego_switch_to_raster(void);
    Q_DECL_EXPORT void qt_meego_switch_to_meego(void);
    Q_DECL_EXPORT void qt_meego_register_switch_callback(QMeeGoSwitchCallback callback);
    Q_DECL_EXPORT void qt_meego_set_switch_policy(int policy);
}

#endif 
