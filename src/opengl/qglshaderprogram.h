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

#ifndef QGLSHADERPROGRAM_H
#define QGLSHADERPROGRAM_H

#include <QtOpenGL/qgl.h>
#include <QtGui/qvector2d.h>
#include <QtGui/qvector3d.h>
#include <QtGui/qvector4d.h>
#include <QtGui/qmatrix4x4.h>
#include <QScopedPointer>

QT_BEGIN_NAMESPACE

#if !defined(QT_OPENGL_ES_1)

class QGLShaderProgram;
class QGLShaderPrivate;

class Q_OPENGL_EXPORT QGLShader : public QObject
{
   OPENGL_CS_OBJECT(QGLShader)

 public:
   enum ShaderTypeBit {
      Vertex          = 0x0001,
      Fragment        = 0x0002,
      Geometry        = 0x0004
   };
   using ShaderType = QFlags<ShaderTypeBit>;

   explicit QGLShader(QGLShader::ShaderType type, QObject *parent = nullptr);
   QGLShader(QGLShader::ShaderType type, const QGLContext *context, QObject *parent = nullptr);
   virtual ~QGLShader();

   QGLShader::ShaderType shaderType() const;

   bool compileSourceCode(const char *source);
   bool compileSourceCode(const QByteArray &source);
   bool compileSourceCode(const QString &source);
   bool compileSourceFile(const QString &fileName);

   QByteArray sourceCode() const;

   bool isCompiled() const;
   QString log() const;

   GLuint shaderId() const;

   static bool hasOpenGLShaders(ShaderType type, const QGLContext *context = 0);

 private:
   friend class QGLShaderProgram;

   Q_DISABLE_COPY(QGLShader)
   Q_DECLARE_PRIVATE(QGLShader)

 protected:
   QScopedPointer<QGLShaderPrivate> d_ptr;

};

Q_DECLARE_OPERATORS_FOR_FLAGS(QGLShader::ShaderType)


class QGLShaderProgramPrivate;

#ifndef GL_EXT_geometry_shader4
#  define GL_LINES_ADJACENCY_EXT 0xA
#  define GL_LINE_STRIP_ADJACENCY_EXT 0xB
#  define GL_TRIANGLES_ADJACENCY_EXT 0xC
#  define GL_TRIANGLE_STRIP_ADJACENCY_EXT 0xD
#endif


class Q_OPENGL_EXPORT QGLShaderProgram : public QObject
{
   OPENGL_CS_OBJECT(QGLShaderProgram)

 public:
   explicit QGLShaderProgram(QObject *parent = nullptr);
   explicit QGLShaderProgram(const QGLContext *context, QObject *parent = nullptr);
   virtual ~QGLShaderProgram();

   bool addShader(QGLShader *shader);
   void removeShader(QGLShader *shader);
   QList<QGLShader *> shaders() const;

   bool addShaderFromSourceCode(QGLShader::ShaderType type, const char *source);
   bool addShaderFromSourceCode(QGLShader::ShaderType type, const QByteArray &source);
   bool addShaderFromSourceCode(QGLShader::ShaderType type, const QString &source);
   bool addShaderFromSourceFile(QGLShader::ShaderType type, const QString &fileName);

   void removeAllShaders();

   virtual bool link();
   bool isLinked() const;
   QString log() const;

   bool bind();
   void release();

   GLuint programId() const;

   int maxGeometryOutputVertices() const;

   void setGeometryOutputVertexCount(int count);
   int geometryOutputVertexCount() const;

   void setGeometryInputType(GLenum inputType);
   GLenum geometryInputType() const;

   void setGeometryOutputType(GLenum outputType);
   GLenum geometryOutputType() const;

   void bindAttributeLocation(const char *name, int location);
   void bindAttributeLocation(const QByteArray &name, int location);
   void bindAttributeLocation(const QString &name, int location);

   int attributeLocation(const char *name) const;
   int attributeLocation(const QByteArray &name) const;
   int attributeLocation(const QString &name) const;

   void setAttributeValue(int location, GLfloat value);
   void setAttributeValue(int location, GLfloat x, GLfloat y);
   void setAttributeValue(int location, GLfloat x, GLfloat y, GLfloat z);
   void setAttributeValue(int location, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
   void setAttributeValue(int location, const QVector2D &value);
   void setAttributeValue(int location, const QVector3D &value);
   void setAttributeValue(int location, const QVector4D &value);
   void setAttributeValue(int location, const QColor &value);
   void setAttributeValue(int location, const GLfloat *values, int columns, int rows);

   void setAttributeValue(const char *name, GLfloat value);
   void setAttributeValue(const char *name, GLfloat x, GLfloat y);
   void setAttributeValue(const char *name, GLfloat x, GLfloat y, GLfloat z);
   void setAttributeValue(const char *name, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
   void setAttributeValue(const char *name, const QVector2D &value);
   void setAttributeValue(const char *name, const QVector3D &value);
   void setAttributeValue(const char *name, const QVector4D &value);
   void setAttributeValue(const char *name, const QColor &value);
   void setAttributeValue(const char *name, const GLfloat *values, int columns, int rows);

   void setAttributeArray
   (int location, const GLfloat *values, int tupleSize, int stride = 0);
   void setAttributeArray
   (int location, const QVector2D *values, int stride = 0);
   void setAttributeArray
   (int location, const QVector3D *values, int stride = 0);
   void setAttributeArray
   (int location, const QVector4D *values, int stride = 0);
   void setAttributeArray
   (int location, GLenum type, const void *values, int tupleSize, int stride = 0);
   void setAttributeArray
   (const char *name, const GLfloat *values, int tupleSize, int stride = 0);
   void setAttributeArray
   (const char *name, const QVector2D *values, int stride = 0);
   void setAttributeArray
   (const char *name, const QVector3D *values, int stride = 0);
   void setAttributeArray
   (const char *name, const QVector4D *values, int stride = 0);
   void setAttributeArray
   (const char *name, GLenum type, const void *values, int tupleSize, int stride = 0);

   void setAttributeBuffer
   (int location, GLenum type, int offset, int tupleSize, int stride = 0);
   void setAttributeBuffer
   (const char *name, GLenum type, int offset, int tupleSize, int stride = 0);

#ifdef Q_MAC_COMPAT_GL_FUNCTIONS
   void setAttributeArray
   (int location, QMacCompatGLenum type, const void *values, int tupleSize, int stride = 0);
   void setAttributeArray
   (const char *name, QMacCompatGLenum type, const void *values, int tupleSize, int stride = 0);
   void setAttributeBuffer
   (int location, QMacCompatGLenum type, int offset, int tupleSize, int stride = 0);
   void setAttributeBuffer
   (const char *name, QMacCompatGLenum type, int offset, int tupleSize, int stride = 0);
#endif

   void enableAttributeArray(int location);
   void enableAttributeArray(const char *name);
   void disableAttributeArray(int location);
   void disableAttributeArray(const char *name);

   int uniformLocation(const char *name) const;
   int uniformLocation(const QByteArray &name) const;
   int uniformLocation(const QString &name) const;

#ifdef Q_MAC_COMPAT_GL_FUNCTIONS
   void setUniformValue(int location, QMacCompatGLint value);
   void setUniformValue(int location, QMacCompatGLuint value);
   void setUniformValue(const char *name, QMacCompatGLint value);
   void setUniformValue(const char *name, QMacCompatGLuint value);
   void setUniformValueArray(int location, const QMacCompatGLint *values, int count);
   void setUniformValueArray(int location, const QMacCompatGLuint *values, int count);
   void setUniformValueArray(const char *name, const QMacCompatGLint *values, int count);
   void setUniformValueArray(const char *name, const QMacCompatGLuint *values, int count);
#endif

   void setUniformValue(int location, GLfloat value);
   void setUniformValue(int location, GLint value);
   void setUniformValue(int location, GLuint value);
   void setUniformValue(int location, GLfloat x, GLfloat y);
   void setUniformValue(int location, GLfloat x, GLfloat y, GLfloat z);
   void setUniformValue(int location, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
   void setUniformValue(int location, const QVector2D &value);
   void setUniformValue(int location, const QVector3D &value);
   void setUniformValue(int location, const QVector4D &value);
   void setUniformValue(int location, const QColor &color);
   void setUniformValue(int location, const QPoint &point);
   void setUniformValue(int location, const QPointF &point);
   void setUniformValue(int location, const QSize &size);
   void setUniformValue(int location, const QSizeF &size);
   void setUniformValue(int location, const QMatrix2x2 &value);
   void setUniformValue(int location, const QMatrix2x3 &value);
   void setUniformValue(int location, const QMatrix2x4 &value);
   void setUniformValue(int location, const QMatrix3x2 &value);
   void setUniformValue(int location, const QMatrix3x3 &value);
   void setUniformValue(int location, const QMatrix3x4 &value);
   void setUniformValue(int location, const QMatrix4x2 &value);
   void setUniformValue(int location, const QMatrix4x3 &value);
   void setUniformValue(int location, const QMatrix4x4 &value);
   void setUniformValue(int location, const GLfloat value[2][2]);
   void setUniformValue(int location, const GLfloat value[3][3]);
   void setUniformValue(int location, const GLfloat value[4][4]);
   void setUniformValue(int location, const QTransform &value);

   void setUniformValue(const char *name, GLfloat value);
   void setUniformValue(const char *name, GLint value);
   void setUniformValue(const char *name, GLuint value);
   void setUniformValue(const char *name, GLfloat x, GLfloat y);
   void setUniformValue(const char *name, GLfloat x, GLfloat y, GLfloat z);
   void setUniformValue(const char *name, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
   void setUniformValue(const char *name, const QVector2D &value);
   void setUniformValue(const char *name, const QVector3D &value);
   void setUniformValue(const char *name, const QVector4D &value);
   void setUniformValue(const char *name, const QColor &color);
   void setUniformValue(const char *name, const QPoint &point);
   void setUniformValue(const char *name, const QPointF &point);
   void setUniformValue(const char *name, const QSize &size);
   void setUniformValue(const char *name, const QSizeF &size);
   void setUniformValue(const char *name, const QMatrix2x2 &value);
   void setUniformValue(const char *name, const QMatrix2x3 &value);
   void setUniformValue(const char *name, const QMatrix2x4 &value);
   void setUniformValue(const char *name, const QMatrix3x2 &value);
   void setUniformValue(const char *name, const QMatrix3x3 &value);
   void setUniformValue(const char *name, const QMatrix3x4 &value);
   void setUniformValue(const char *name, const QMatrix4x2 &value);
   void setUniformValue(const char *name, const QMatrix4x3 &value);
   void setUniformValue(const char *name, const QMatrix4x4 &value);
   void setUniformValue(const char *name, const GLfloat value[2][2]);
   void setUniformValue(const char *name, const GLfloat value[3][3]);
   void setUniformValue(const char *name, const GLfloat value[4][4]);
   void setUniformValue(const char *name, const QTransform &value);

   void setUniformValueArray(int location, const GLfloat *values, int count, int tupleSize);
   void setUniformValueArray(int location, const GLint *values, int count);
   void setUniformValueArray(int location, const GLuint *values, int count);
   void setUniformValueArray(int location, const QVector2D *values, int count);
   void setUniformValueArray(int location, const QVector3D *values, int count);
   void setUniformValueArray(int location, const QVector4D *values, int count);
   void setUniformValueArray(int location, const QMatrix2x2 *values, int count);
   void setUniformValueArray(int location, const QMatrix2x3 *values, int count);
   void setUniformValueArray(int location, const QMatrix2x4 *values, int count);
   void setUniformValueArray(int location, const QMatrix3x2 *values, int count);
   void setUniformValueArray(int location, const QMatrix3x3 *values, int count);
   void setUniformValueArray(int location, const QMatrix3x4 *values, int count);
   void setUniformValueArray(int location, const QMatrix4x2 *values, int count);
   void setUniformValueArray(int location, const QMatrix4x3 *values, int count);
   void setUniformValueArray(int location, const QMatrix4x4 *values, int count);

   void setUniformValueArray(const char *name, const GLfloat *values, int count, int tupleSize);
   void setUniformValueArray(const char *name, const GLint *values, int count);
   void setUniformValueArray(const char *name, const GLuint *values, int count);
   void setUniformValueArray(const char *name, const QVector2D *values, int count);
   void setUniformValueArray(const char *name, const QVector3D *values, int count);
   void setUniformValueArray(const char *name, const QVector4D *values, int count);
   void setUniformValueArray(const char *name, const QMatrix2x2 *values, int count);
   void setUniformValueArray(const char *name, const QMatrix2x3 *values, int count);
   void setUniformValueArray(const char *name, const QMatrix2x4 *values, int count);
   void setUniformValueArray(const char *name, const QMatrix3x2 *values, int count);
   void setUniformValueArray(const char *name, const QMatrix3x3 *values, int count);
   void setUniformValueArray(const char *name, const QMatrix3x4 *values, int count);
   void setUniformValueArray(const char *name, const QMatrix4x2 *values, int count);
   void setUniformValueArray(const char *name, const QMatrix4x3 *values, int count);
   void setUniformValueArray(const char *name, const QMatrix4x4 *values, int count);

   static bool hasOpenGLShaderPrograms(const QGLContext *context = 0);

 private :
   OPENGL_CS_SLOT_1(Private, void shaderDestroyed())
   OPENGL_CS_SLOT_2(shaderDestroyed)

   Q_DISABLE_COPY(QGLShaderProgram)
   Q_DECLARE_PRIVATE(QGLShaderProgram)

   bool init();

 protected:
   QScopedPointer<QGLShaderProgramPrivate> d_ptr;

};

#endif

QT_END_NAMESPACE

#endif
