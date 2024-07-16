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

#include "qsvgtinydocument_p.h"

#include "qsvghandler_p.h"
#include "qsvgfont_p.h"
#include "qpainter.h"
#include "qfile.h"
#include "qbuffer.h"
#include "qbytearray.h"
#include "qqueue.h"
#include "qstack.h"
#include "qdebug.h"

#ifndef QT_NO_COMPRESS
#include <zlib.h>
#endif

QSvgTinyDocument::QSvgTinyDocument()
   : QSvgStructureNode(nullptr), m_widthPercent(false), m_heightPercent(false)
   , m_animated(false), m_animationDuration(0), m_fps(30)
{
}

QSvgTinyDocument::~QSvgTinyDocument()
{
}

#ifndef QT_NO_COMPRESS
static QByteArray qt_inflateGZipDataFrom(QIODevice *device);

QByteArray qt_inflateGZipDataFrom(QIODevice *device)
{
   if (!device) {
      return QByteArray();
   }

   if (!device->isOpen()) {
      device->open(QIODevice::ReadOnly);
   }

   Q_ASSERT(device->isOpen() && device->isReadable());

   static const int CHUNK_SIZE = 4096;
   int zlibResult = Z_OK;

   QByteArray source;
   QByteArray destination;

   // Initialize zlib stream struct
   z_stream zlibStream;
   zlibStream.next_in   = nullptr;
   zlibStream.avail_in  = 0;
   zlibStream.avail_out = 0;
   zlibStream.zalloc    = nullptr;
   zlibStream.zfree     = nullptr;
   zlibStream.opaque    = nullptr;

   // Adding 16 to the window size gives us gzip decoding
   if (inflateInit2(&zlibStream, MAX_WBITS + 16) != Z_OK) {
      qWarning("Unable to initialize zlib, because: %s",
               (zlibStream.msg != nullptr ? zlibStream.msg : "Unknown error"));

      return QByteArray();
   }

   bool stillMoreWorkToDo = true;
   while (stillMoreWorkToDo) {

      if (!zlibStream.avail_in) {
         source = device->read(CHUNK_SIZE);

         if (source.isEmpty()) {
            break;
         }

         zlibStream.avail_in = source.size();
         zlibStream.next_in = reinterpret_cast<Bytef *>(source.data());
      }

      do {
         // Prepare the destination buffer
         int oldSize = destination.size();
         destination.resize(oldSize + CHUNK_SIZE);
         zlibStream.next_out = reinterpret_cast<Bytef *>(
                                  destination.data() + oldSize - zlibStream.avail_out);
         zlibStream.avail_out += CHUNK_SIZE;

         zlibResult = inflate(&zlibStream, Z_NO_FLUSH);
         switch (zlibResult) {
            case Z_NEED_DICT:
            case Z_DATA_ERROR:
            case Z_STREAM_ERROR:
            case Z_MEM_ERROR: {
               inflateEnd(&zlibStream);
               qWarning("Error while inflating gzip file: %s",
                        (zlibStream.msg != nullptr ? zlibStream.msg : "Unknown error"));
               destination.chop(zlibStream.avail_out);
               return destination;
            }
         }

         // If the output buffer still has more room after calling inflate
         // it means we have to provide more data, so exit the loop here
      } while (!zlibStream.avail_out);

      if (zlibResult == Z_STREAM_END) {
         // Make sure there are no more members to process before exiting
         if (!(zlibStream.avail_in && inflateReset(&zlibStream) == Z_OK)) {
            stillMoreWorkToDo = false;
         }
      }
   }

   // Chop off trailing space in the buffer
   destination.chop(zlibStream.avail_out);

   inflateEnd(&zlibStream);
   return destination;
}
#endif

QSvgTinyDocument *QSvgTinyDocument::load(const QString &fileName)
{
   QFile file(fileName);

   if (! file.open(QFile::ReadOnly)) {
      qWarning("Unable to open file '%s', because: %s",
               csPrintable(fileName), csPrintable(file.errorString()));

      return nullptr;
   }

#ifndef QT_NO_COMPRESS
   if (fileName.endsWith(QLatin1String(".svgz"), Qt::CaseInsensitive)
         || fileName.endsWith(QLatin1String(".svg.gz"), Qt::CaseInsensitive)) {
      return load(qt_inflateGZipDataFrom(&file));
   }
#endif

   QSvgTinyDocument *doc = nullptr;
   QSvgHandler handler(&file);

   if (handler.ok()) {
      doc = handler.document();
      doc->m_animationDuration = handler.animationDuration();
   } else {
      qWarning("Cannot read file '%s', because: %s (line %d)",
               csPrintable(fileName), csPrintable(handler.errorString()), handler.lineNumber());
   }

   return doc;
}

QSvgTinyDocument *QSvgTinyDocument::load(const QByteArray &contents)
{
#ifndef QT_NO_COMPRESS
   // Check for gzip magic number and inflate if appropriate
   if (contents.startsWith("\x1f\x8b")) {
      QBuffer buffer(const_cast<QByteArray *>(&contents));
      return load(qt_inflateGZipDataFrom(&buffer));
   }
#endif

   QSvgHandler handler(contents);

   QSvgTinyDocument *doc = nullptr;

   if (handler.ok()) {
      doc = handler.document();
      doc->m_animationDuration = handler.animationDuration();
   }

   return doc;
}

QSvgTinyDocument *QSvgTinyDocument::load(QXmlStreamReader *contents)
{
   QSvgHandler handler(contents);

   QSvgTinyDocument *doc = nullptr;

   if (handler.ok()) {
      doc = handler.document();
      doc->m_animationDuration = handler.animationDuration();
   }

   return doc;
}

void QSvgTinyDocument::draw(QPainter *p, const QRectF &bounds)
{
   if (m_time.isNull()) {
      m_time.start();
   }

   if (displayMode() == QSvgNode::NoneMode) {
      return;
   }

   p->save();
   //sets default style on the painter
   //### not the most optimal way
   mapSourceToTarget(p, bounds);

   QPen pen(Qt::NoBrush, 1, Qt::SolidLine, Qt::FlatCap, Qt::SvgMiterJoin);
   pen.setMiterLimit(4);
   p->setPen(pen);
   p->setBrush(Qt::black);
   p->setRenderHint(QPainter::Antialiasing);
   p->setRenderHint(QPainter::SmoothPixmapTransform);

   QList<QSvgNode *>::iterator itr = m_renderers.begin();
   applyStyle(p, m_states);

   while (itr != m_renderers.end()) {
      QSvgNode *node = *itr;

      if ((node->isVisible()) && (node->displayMode() != QSvgNode::NoneMode)) {
         node->draw(p, m_states);
      }

      ++itr;
   }

   revertStyle(p, m_states);
   p->restore();
}


void QSvgTinyDocument::draw(QPainter *p, const QString &id, const QRectF &bounds)
{
   QSvgNode *node = scopeNode(id);

   if (! node) {
#if defined(CS_SHOW_DEBUG_SVG)
      qDebug("Unable to find node %s, skipping rendering.", csPrintable(id));
#endif

      return;
   }

   if (m_time.isNull()) {
      m_time.start();
   }

   if (node->displayMode() == QSvgNode::NoneMode) {
      return;
   }

   p->save();

   const QRectF elementBounds = node->transformedBounds();

   mapSourceToTarget(p, bounds, elementBounds);
   QTransform originalTransform = p->worldTransform();

   //XXX set default style on the painter
   QPen pen(Qt::NoBrush, 1, Qt::SolidLine, Qt::FlatCap, Qt::SvgMiterJoin);
   pen.setMiterLimit(4);
   p->setPen(pen);
   p->setBrush(Qt::black);
   p->setRenderHint(QPainter::Antialiasing);
   p->setRenderHint(QPainter::SmoothPixmapTransform);

   QStack<QSvgNode *> parentApplyStack;
   QSvgNode *parent = node->parent();

   while (parent) {
      parentApplyStack.push(parent);
      parent = parent->parent();
   }

   for (int i = parentApplyStack.size() - 1; i >= 0; --i) {
      parentApplyStack[i]->applyStyle(p, m_states);
   }

   // Reset the world transform so that our parents don't affect
   // the position
   QTransform currentTransform = p->worldTransform();
   p->setWorldTransform(originalTransform);

   node->draw(p, m_states);

   p->setWorldTransform(currentTransform);

   for (int i = 0; i < parentApplyStack.size(); ++i) {
      parentApplyStack[i]->revertStyle(p, m_states);
   }

   //p->fillRect(bounds.adjusted(-5, -5, 5, 5), QColor(0, 0, 255, 100));

   p->restore();
}

QSvgNode::Type QSvgTinyDocument::type() const
{
   return DOC;
}

void QSvgTinyDocument::setWidth(int len, bool percent)
{
   m_size.setWidth(len);
   m_widthPercent = percent;
}

void QSvgTinyDocument::setHeight(int len, bool percent)
{
   m_size.setHeight(len);
   m_heightPercent = percent;
}

void QSvgTinyDocument::setViewBox(const QRectF &rect)
{
   m_viewBox = rect;
}

void QSvgTinyDocument::addSvgFont(QSvgFont *font)
{
   m_fonts.insert(font->familyName(), font);
}

QSvgFont *QSvgTinyDocument::svgFont(const QString &family) const
{
   return m_fonts[family];
}

void QSvgTinyDocument::addNamedNode(const QString &id, QSvgNode *node)
{
   m_namedNodes.insert(id, node);
}

QSvgNode *QSvgTinyDocument::namedNode(const QString &id) const
{
   return m_namedNodes.value(id);
}

void QSvgTinyDocument::addNamedStyle(const QString &id, QSvgFillStyleProperty *style)
{
   m_namedStyles.insert(id, style);
}

QSvgFillStyleProperty *QSvgTinyDocument::namedStyle(const QString &id) const
{
   return m_namedStyles.value(id);
}

void QSvgTinyDocument::restartAnimation()
{
   m_time.restart();
}

bool QSvgTinyDocument::animated() const
{
   return m_animated;
}

void QSvgTinyDocument::setAnimated(bool a)
{
   m_animated = a;
}

void QSvgTinyDocument::draw(QPainter *p)
{
   draw(p, QRectF());
}

void QSvgTinyDocument::draw(QPainter *p, QSvgExtraStates &)
{
   draw(p);
}

void QSvgTinyDocument::mapSourceToTarget(QPainter *p, const QRectF &targetRect, const QRectF &sourceRect)
{
   QRectF target = targetRect;
   if (target.isNull()) {
      QPaintDevice *dev = p->device();
      QRectF deviceRect(0, 0, dev->width(), dev->height());
      if (deviceRect.isNull()) {
         if (sourceRect.isNull()) {
            target = QRectF(QPointF(0, 0), size());
         } else {
            target = QRectF(QPointF(0, 0), sourceRect.size());
         }
      } else {
         target = deviceRect;
      }
   }

   QRectF source = sourceRect;
   if (source.isNull()) {
      source = viewBox();
   }

   if (source != target && !source.isNull()) {
      QTransform transform;
      transform.scale(target.width() / source.width(), target.height() / source.height());

      QRectF c2 = transform.mapRect(source);
      p->translate(target.x() - c2.x(), target.y() - c2.y());
      p->scale(target.width() / source.width(), target.height() / source.height());
   }
}

QRectF QSvgTinyDocument::boundsOnElement(const QString &id) const
{
   const QSvgNode *node = scopeNode(id);
   if (! node) {
      node = this;
   }

   return node->transformedBounds();
}

bool QSvgTinyDocument::elementExists(const QString &id) const
{
   QSvgNode *node = scopeNode(id);

   return (node != nullptr);
}

QMatrix QSvgTinyDocument::matrixForElement(const QString &id) const
{
   QSvgNode *node = scopeNode(id);

   if (! node) {
#if defined(CS_SHOW_DEBUG_SVG)
      qDebug("Unable to find node %s, skipping rendering.", csPrintable(id));
#endif

      return QMatrix();
   }

   QTransform t;

   node = node->parent();
   while (node) {
      if (node->m_style.transform) {
         t *= node->m_style.transform->qtransform();
      }
      node = node->parent();
   }

   return t.toAffine();
}

int QSvgTinyDocument::currentFrame() const
{
   double runningPercentage = qMin(m_time.elapsed() / double(m_animationDuration), 1.);

   int totalFrames = m_fps * m_animationDuration;

   return int(runningPercentage * totalFrames);
}

void QSvgTinyDocument::setCurrentFrame(int frame)
{
   int totalFrames = m_fps * m_animationDuration;
   double framePercentage = frame / double(totalFrames);
   double timeForFrame = m_animationDuration * framePercentage; //in S
   timeForFrame *= 1000; //in ms
   int timeToAdd = int(timeForFrame - m_time.elapsed());
   m_time = m_time.addMSecs(timeToAdd);
}

void QSvgTinyDocument::setFramesPerSecond(int num)
{
   m_fps = num;
}

