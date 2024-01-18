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

#ifndef QDECLARATIVEJSNODEPOOL_P_H
#define QDECLARATIVEJSNODEPOOL_P_H

#include <qdeclarativejsglobal_p.h>
#include <qdeclarativejsmemorypool_p.h>
#include <QtCore/QHash>
#include <QtCore/QString>

QT_QML_BEGIN_NAMESPACE

namespace QDeclarativeJS {

namespace AST {
class Node;
} // namespace AST

class Code;
class CompilationUnit;
class Engine;

template <typename NodeType>
inline NodeType *makeAstNode(MemoryPool *storage)
{
   NodeType *node = new (storage->allocate(sizeof(NodeType))) NodeType();
   return node;
}

template <typename NodeType, typename Arg1>
inline NodeType *makeAstNode(MemoryPool *storage, Arg1 arg1)
{
   NodeType *node = new (storage->allocate(sizeof(NodeType))) NodeType(arg1);
   return node;
}

template <typename NodeType, typename Arg1, typename Arg2>
inline NodeType *makeAstNode(MemoryPool *storage, Arg1 arg1, Arg2 arg2)
{
   NodeType *node = new (storage->allocate(sizeof(NodeType))) NodeType(arg1, arg2);
   return node;
}

template <typename NodeType, typename Arg1, typename Arg2, typename Arg3>
inline NodeType *makeAstNode(MemoryPool *storage, Arg1 arg1, Arg2 arg2, Arg3 arg3)
{
   NodeType *node = new (storage->allocate(sizeof(NodeType))) NodeType(arg1, arg2, arg3);
   return node;
}

template <typename NodeType, typename Arg1, typename Arg2, typename Arg3, typename Arg4>
inline NodeType *makeAstNode(MemoryPool *storage, Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4)
{
   NodeType *node = new (storage->allocate(sizeof(NodeType))) NodeType(arg1, arg2, arg3, arg4);
   return node;
}

class QML_PARSER_EXPORT NodePool : public MemoryPool
{
 public:
   NodePool(const QString &fileName, Engine *engine);
   virtual ~NodePool();

   Code *createCompiledCode(AST::Node *node, CompilationUnit &compilation);

   inline QString fileName() const {
      return m_fileName;
   }
   inline Engine *engine() const {
      return m_engine;
   }
#ifndef J_SCRIPT_NO_EVENT_NOTIFY
   inline qint64 id() const {
      return m_id;
   }
#endif

 private:
   QHash<AST::Node *, Code *> m_codeCache;
   QString m_fileName;
   Engine *m_engine;
#ifndef J_SCRIPT_NO_EVENT_NOTIFY
   qint64 m_id;
#endif

 private:
   Q_DISABLE_COPY(NodePool)
};

} // namespace QDeclarativeJS

QT_QML_END_NAMESPACE

#endif
