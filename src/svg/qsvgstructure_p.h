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

#ifndef QSVGSTRUCTURE_P_H
#define QSVGSTRUCTURE_P_H

#include <qhash.h>
#include <qlist.h>

#include <qsvgnode_p.h>

class QPainter;
class QSvgDefs;
class QSvgNode;
class QSvgTinyDocument;

class QSvgStructureNode : public QSvgNode
{
 public:
   QSvgStructureNode(QSvgNode *parent);
   ~QSvgStructureNode();

   QSvgNode *scopeNode(const QString &id) const;
   void addChild(QSvgNode *child, const QString &id);
   QRectF bounds(QPainter *p, QSvgExtraStates &states) const override;
   QSvgNode *previousSiblingNode(QSvgNode *n) const;

   QList<QSvgNode *> renderers() const {
      return m_renderers;
   }

 protected:
   QList<QSvgNode *>          m_renderers;
   QHash<QString, QSvgNode *> m_scope;
   QList<QSvgStructureNode *> m_linkedScopes;
};

class QSvgG : public QSvgStructureNode
{
 public:
   QSvgG(QSvgNode *parent);

   void draw(QPainter *p, QSvgExtraStates &states) override;
   Type type() const override;
};

class QSvgDefs : public QSvgStructureNode
{
 public:
   QSvgDefs(QSvgNode *parent);

   void draw(QPainter *p, QSvgExtraStates &states) override;
   Type type() const override;
};

class QSvgSwitch : public QSvgStructureNode
{
 public:
   QSvgSwitch(QSvgNode *parent);

   void draw(QPainter *p, QSvgExtraStates &states) override;
   Type type() const override;

 private:
   void init();


   QString m_systemLanguage;
   QString m_systemLanguagePrefix;
};


#endif
