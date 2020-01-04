/***********************************************************************
*
* Copyright (c) 2012-2020 Barbara Geller
* Copyright (c) 2012-2020 Ansel Sermersheim
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

#ifndef QWINDOWSDRAG_H
#define QWINDOWSDRAG_H

#include "qwindowsinternalmimedata.h"

#include <qplatform_drag.h>
#include <QPixmap>

struct IDropTargetHelper;

class QPlatformScreen;

class QWindowsDropMimeData : public QWindowsInternalMimeData
{
 public:
   QWindowsDropMimeData() {}
   IDataObject *retrieveDataObject() const override;
};

class QWindowsOleDropTarget : public IDropTarget
{
 public:
   explicit QWindowsOleDropTarget(QWindow *w);
   virtual ~QWindowsOleDropTarget();

   // IUnknown methods
   STDMETHOD(QueryInterface)(REFIID riid, void FAR *FAR *ppvObj);
   STDMETHOD_(ULONG, AddRef)(void);
   STDMETHOD_(ULONG, Release)(void);

   // IDropTarget methods
   STDMETHOD(DragEnter)(LPDATAOBJECT pDataObj, DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect);
   STDMETHOD(DragOver)(DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect);
   STDMETHOD(DragLeave)();
   STDMETHOD(Drop)(LPDATAOBJECT pDataObj, DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect);

 private:
   void handleDrag(QWindow *window, DWORD grfKeyState, const QPoint &, LPDWORD pdwEffect);

   ULONG m_refs;
   QWindow *const m_window;
   QRect m_answerRect;
   QPoint m_lastPoint;
   DWORD m_chosenEffect;
   DWORD m_lastKeyState;
};

class QWindowsDrag : public QPlatformDrag
{
 public:
   QWindowsDrag();
   virtual ~QWindowsDrag();

   QMimeData *platformDropData() override {
      return &m_dropData;
   }

   Qt::DropAction drag(QDrag *drag) override;

   static QWindowsDrag *instance();

   IDataObject *dropDataObject() const             {
      return m_dropDataObject;
   }
   void setDropDataObject(IDataObject *dataObject) {
      m_dropDataObject = dataObject;
   }
   void releaseDropDataObject();
   QMimeData *dropData();

   IDropTargetHelper *dropHelper();

 private:
   QWindowsDropMimeData m_dropData;
   IDataObject *m_dropDataObject;

   IDropTargetHelper *m_cachedDropTargetHelper;
};

#endif
