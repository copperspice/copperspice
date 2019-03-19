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

#include "qxlibeglintegration.h"

static int countBits(unsigned long mask)
{
    int count = 0;
    while (mask != 0) {
        if (mask & 1)
            ++count;
        mask >>= 1;
    }
    return count;
}

VisualID QXlibEglIntegration::getCompatibleVisualId(Display *display, EGLDisplay eglDisplay, EGLConfig config)
{
    VisualID    visualId = 0;
    EGLint      eglValue = 0;

    EGLint configRedSize = 0;
    eglGetConfigAttrib(eglDisplay, config, EGL_RED_SIZE, &configRedSize);

    EGLint configGreenSize = 0;
    eglGetConfigAttrib(eglDisplay, config, EGL_GREEN_SIZE, &configGreenSize);

    EGLint configBlueSize = 0;
    eglGetConfigAttrib(eglDisplay, config, EGL_BLUE_SIZE, &configBlueSize);

    EGLint configAlphaSize = 0;
    eglGetConfigAttrib(eglDisplay, config, EGL_ALPHA_SIZE, &configAlphaSize);

    eglGetConfigAttrib(eglDisplay, config, EGL_CONFIG_ID, &eglValue);
    int configId = eglValue;

    // See if EGL provided a valid VisualID:
    eglGetConfigAttrib(eglDisplay, config, EGL_NATIVE_VISUAL_ID, &eglValue);
    visualId = (VisualID)eglValue;
    if (visualId) {
        // EGL has suggested a visual id, so get the rest of the visual info for that id:
        XVisualInfo visualInfoTemplate;
        memset(&visualInfoTemplate, 0, sizeof(XVisualInfo));
        visualInfoTemplate.visualid = visualId;

        XVisualInfo *chosenVisualInfo;
        int matchingCount = 0;
        chosenVisualInfo = XGetVisualInfo(display, VisualIDMask, &visualInfoTemplate, &matchingCount);
        if (chosenVisualInfo) {
            // Skip size checks if implementation supports non-matching visual
            // and config (http://bugreports.qt-project.org/browse/QTBUG-9444).
            if (q_hasEglExtension(eglDisplay,"EGL_NV_post_convert_rounding")) {
                XFree(chosenVisualInfo);
                return visualId;
            }

            int visualRedSize = countBits(chosenVisualInfo->red_mask);
            int visualGreenSize = countBits(chosenVisualInfo->green_mask);
            int visualBlueSize = countBits(chosenVisualInfo->blue_mask);
            int visualAlphaSize = -1; // Need XRender to tell us the alpha channel size

            bool visualMatchesConfig = false;
            if ( visualRedSize == configRedSize &&
                 visualGreenSize == configGreenSize &&
                 visualBlueSize == configBlueSize )
            {
                // We need XRender to check the alpha channel size of the visual. If we don't have
                // the alpha size, we don't check it against the EGL config's alpha size.
                if (visualAlphaSize >= 0)
                    visualMatchesConfig = visualAlphaSize == configAlphaSize;
                else
                    visualMatchesConfig = true;
            }

            if (!visualMatchesConfig) {
                if (visualAlphaSize >= 0) {
                    qWarning("Warning: EGL suggested using X Visual ID %d (ARGB%d%d%d%d) for EGL config %d (ARGB%d%d%d%d), but this is incompatable",
                             (int)visualId, visualAlphaSize, visualRedSize, visualGreenSize, visualBlueSize,
                             configId, configAlphaSize, configRedSize, configGreenSize, configBlueSize);
                } else {
                    qWarning("Warning: EGL suggested using X Visual ID %d (RGB%d%d%d) for EGL config %d (RGB%d%d%d), but this is incompatable",
                             (int)visualId, visualRedSize, visualGreenSize, visualBlueSize,
                             configId, configRedSize, configGreenSize, configBlueSize);
                }
                visualId = 0;
            }
        } else {
            qWarning("Warning: EGL suggested using X Visual ID %d for EGL config %d, but that isn't a valid ID",
                     (int)visualId, configId);
            visualId = 0;
        }
        XFree(chosenVisualInfo);
    }
#ifdef QT_DEBUG_X11_VISUAL_SELECTION
    else
        qDebug("EGL did not suggest a VisualID (EGL_NATIVE_VISUAL_ID was zero) for EGLConfig %d", configId);
#endif

    if (visualId) {
#ifdef QT_DEBUG_X11_VISUAL_SELECTION
        if (configAlphaSize > 0)
            qDebug("Using ARGB Visual ID %d provided by EGL for config %d", (int)visualId, configId);
        else
            qDebug("Using Opaque Visual ID %d provided by EGL for config %d", (int)visualId, configId);
#endif
        return visualId;
    }

    // Finally, try to
    // use XGetVisualInfo and only use the bit depths to match on:
    if (!visualId) {
        XVisualInfo visualInfoTemplate;
        memset(&visualInfoTemplate, 0, sizeof(XVisualInfo));
        XVisualInfo *matchingVisuals;
        int matchingCount = 0;

        visualInfoTemplate.depth = configRedSize + configGreenSize + configBlueSize + configAlphaSize;
        matchingVisuals = XGetVisualInfo(display,
                                         VisualDepthMask,
                                         &visualInfoTemplate,
                                         &matchingCount);
        if (!matchingVisuals) {
            // Try again without taking the alpha channel into account:
            visualInfoTemplate.depth = configRedSize + configGreenSize + configBlueSize;
            matchingVisuals = XGetVisualInfo(display,
                                             VisualDepthMask,
                                             &visualInfoTemplate,
                                             &matchingCount);
        }

        if (matchingVisuals) {
            visualId = matchingVisuals[0].visualid;
            XFree(matchingVisuals);
        }
    }

    if (visualId) {
#ifdef QT_DEBUG_X11_VISUAL_SELECTION
        qDebug("Using Visual ID %d provided by XGetVisualInfo for EGL config %d", (int)visualId, configId);
#endif
        return visualId;
    }

    qWarning("Unable to find an X11 visual which matches EGL config %d", configId);
    return (VisualID)0;
}
