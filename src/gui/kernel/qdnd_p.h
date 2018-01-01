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

#ifndef QDND_P_H
#define QDND_P_H

#include <QtCore/qobject.h>
#include <QtCore/qmap.h>
#include <QtGui/qmime.h>
#include <QtGui/qdrag.h>
#include <QtGui/qpixmap.h>
#include <QtGui/qcursor.h>
#include <QtCore/qpoint.h>

#ifdef Q_OS_MAC
# include <qt_mac_p.h>
#endif

#if defined(Q_OS_WIN)
# include <qt_windows.h>
# include <objidl.h>
#endif

QT_BEGIN_NAMESPACE

class QEventLoop;

#if ! (defined(QT_NO_DRAGANDDROP) && defined(QT_NO_CLIPBOARD))

class Q_GUI_EXPORT QInternalMimeData : public QMimeData
{
   GUI_CS_OBJECT(QInternalMimeData)

 public:
   QInternalMimeData();
   ~QInternalMimeData();

   bool hasFormat(const QString &mimeType) const override;
   QStringList formats() const override;
   static bool canReadData(const QString &mimeType);


   static QStringList formatsHelper(const QMimeData *data);
   static bool hasFormatHelper(const QString &mimeType, const QMimeData *data);
   static QByteArray renderDataHelper(const QString &mimeType, const QMimeData *data);

 protected:
   QVariant retrieveData(const QString &mimeType, QVariant::Type type) const override;

   virtual bool hasFormat_sys(const QString &mimeType) const = 0;
   virtual QStringList formats_sys() const = 0;
   virtual QVariant retrieveData_sys(const QString &mimeType, QVariant::Type type) const = 0;
};

#ifdef Q_OS_WIN
class QOleDataObject : public IDataObject
{
 public:
   explicit QOleDataObject(QMimeData *mimeData);
   virtual ~QOleDataObject();

   void releaseQt();
   const QMimeData *mimeData() const;
   DWORD reportedPerformedEffect() const;

   // IUnknown methods
   STDMETHOD(QueryInterface)(REFIID riid, void FAR *FAR *ppvObj) override;
   STDMETHOD_(ULONG, AddRef)(void) override;
   STDMETHOD_(ULONG, Release)(void) override;

   // IDataObject methods
   STDMETHOD(GetData)(LPFORMATETC pformatetcIn, LPSTGMEDIUM pmedium) override;
   STDMETHOD(GetDataHere)(LPFORMATETC pformatetc, LPSTGMEDIUM pmedium) override;
   STDMETHOD(QueryGetData)(LPFORMATETC pformatetc) override;
   STDMETHOD(GetCanonicalFormatEtc)(LPFORMATETC pformatetc, LPFORMATETC pformatetcOut) override;
   STDMETHOD(SetData)(LPFORMATETC pformatetc, STGMEDIUM FAR *pmedium, BOOL fRelease) override;
   STDMETHOD(EnumFormatEtc)(DWORD dwDirection, LPENUMFORMATETC FAR *ppenumFormatEtc) override;
   STDMETHOD(DAdvise)(FORMATETC FAR *pFormatetc, DWORD advf, LPADVISESINK pAdvSink, DWORD FAR *pdwConnection) override;
   STDMETHOD(DUnadvise)(DWORD dwConnection) override;
   STDMETHOD(EnumDAdvise)(LPENUMSTATDATA FAR *ppenumAdvise) override;

 private:
   ULONG m_refs;
   QPointer<QMimeData> data;
   int CF_PERFORMEDDROPEFFECT;
   DWORD performedEffect;
};

class QOleEnumFmtEtc : public IEnumFORMATETC
{
 public:
   explicit QOleEnumFmtEtc(const QVector<FORMATETC> &fmtetcs);
   explicit QOleEnumFmtEtc(const QVector<LPFORMATETC> &lpfmtetcs);
   virtual ~QOleEnumFmtEtc();

   bool isNull() const;

   // IUnknown methods
   STDMETHOD(QueryInterface)(REFIID riid, void FAR *FAR *ppvObj) override;
   STDMETHOD_(ULONG, AddRef)(void) override;
   STDMETHOD_(ULONG, Release)(void) override;

   // IEnumFORMATETC methods
   STDMETHOD(Next)(ULONG celt, LPFORMATETC rgelt, ULONG FAR *pceltFetched) override;
   STDMETHOD(Skip)(ULONG celt) override;
   STDMETHOD(Reset)(void) override;
   STDMETHOD(Clone)(LPENUMFORMATETC FAR *newEnum) override;

 private:
   bool copyFormatEtc(LPFORMATETC dest, LPFORMATETC src) const;

   ULONG m_dwRefs;
   ULONG m_nIndex;
   QVector<LPFORMATETC> m_lpfmtetcs;
   bool m_isNull;
};

#endif

#endif //QT_NO_DRAGANDDROP && QT_NO_CLIPBOARD

#ifndef QT_NO_DRAGANDDROP

class QDragPrivate
{

 public:
   virtual ~QDragPrivate() {};

   QWidget *source;
   QWidget *target;
   QMimeData *data;
   QPixmap pixmap;
   QPoint hotspot;
   Qt::DropActions possible_actions;
   Qt::DropAction executed_action;
   QMap<Qt::DropAction, QPixmap> customCursors;
   Qt::DropAction defaultDropAction;
};

class QDropData : public QInternalMimeData
{
   GUI_CS_OBJECT(QDropData)

 public:
   QDropData();
   ~QDropData();

 protected:
   bool hasFormat_sys(const QString &mimeType) const override;
   QStringList formats_sys() const override;
   QVariant retrieveData_sys(const QString &mimeType, QVariant::Type type) const override;

#if defined(Q_OS_WIN)
 public:
   LPDATAOBJECT currentDataObject;
#endif
};

class QDragManager: public QObject
{
   GUI_CS_OBJECT(QDragManager)

   QDragManager();
   ~QDragManager();
   // only friend classes can use QDragManager.
   friend class QDrag;
   friend class QDragMoveEvent;
   friend class QDropEvent;
   friend class QApplication;

#ifdef Q_OS_MAC
   friend class QWidgetPrivate; //dnd is implemented here
#endif

   bool eventFilter(QObject *, QEvent *) override;
   void timerEvent(QTimerEvent *) override;

 public:
   Qt::DropAction drag(QDrag *);

   void cancel(bool deleteSource = true);
   void move(const QPoint &);
   void drop();
   void updatePixmap();
   QWidget *source() const {
      return object ? object->d_func()->source : 0;
   }
   QDragPrivate *dragPrivate() const {
      return object ? object->d_func() : 0;
   }
   static QDragPrivate *dragPrivate(QDrag *drag) {
      return drag ? drag->d_func() : 0;
   }

   static QDragManager *self();
   Qt::DropAction defaultAction(Qt::DropActions possibleActions, Qt::KeyboardModifiers modifiers) const;

   QDrag *object;

   void updateCursor();

   bool beingCancelled;
   bool restoreCursor;
   bool willDrop;
   QEventLoop *eventLoop;

   QPixmap dragCursor(Qt::DropAction action) const;

   bool hasCustomDragCursors() const;

   QDropData *dropData;

   void emitActionChanged(Qt::DropAction newAction) {
      if (object) {
         emit object->actionChanged(newAction);
      }
   }

   void setCurrentTarget(QWidget *target, bool dropped = false);
   QWidget *currentTarget();

#ifdef Q_WS_X11
   QPixmap xdndMimeTransferedPixmap[2];
   int xdndMimeTransferedPixmapIndex;
#endif

 private:

#if defined(Q_WS_QWS) || defined(Q_WS_QPA)
   Qt::DropAction currentActionForOverrideCursor;
#endif

   QWidget *currentDropTarget;

   static QDragManager *instance;
   Q_DISABLE_COPY(QDragManager)
};


#if defined(Q_OS_WIN)

class QOleDropTarget : public IDropTarget
{
 public:
   QOleDropTarget(QWidget *w);
   virtual ~QOleDropTarget() {}

   void releaseQt();

   // IUnknown methods
   STDMETHOD(QueryInterface)(REFIID riid, void FAR *FAR *ppvObj) override;
   STDMETHOD_(ULONG, AddRef)(void) override;
   STDMETHOD_(ULONG, Release)(void) override;

   // IDropTarget methods
   STDMETHOD(DragEnter)(LPDATAOBJECT pDataObj, DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect) override;
   STDMETHOD(DragOver)(DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect) override;
   STDMETHOD(DragLeave)() override;
   STDMETHOD(Drop)(LPDATAOBJECT pDataObj, DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect) override;

 private:
   ULONG m_refs;
   QWidget *widget;
   QPointer<QWidget> currentWidget;
   QRect answerRect;
   QPoint lastPoint;
   DWORD chosenEffect;
   DWORD lastKeyState;

   void sendDragEnterEvent(QWidget *to, DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect);
};

#endif

#if defined (Q_OS_MAC)
class QCocoaDropData : public QInternalMimeData
{
   GUI_CS_OBJECT(QCocoaDropData)

 public:
   QCocoaDropData(CFStringRef pasteboard);
   ~QCocoaDropData();

   CFStringRef dropPasteboard;

 protected:
   bool hasFormat_sys(const QString &mimeType) const override;
   QStringList formats_sys() const override;
   QVariant retrieveData_sys(const QString &mimeType, QVariant::Type type) const override;
};
#endif

#endif // !QT_NO_DRAGANDDROP


QT_END_NAMESPACE

#endif // QDND_P_H
