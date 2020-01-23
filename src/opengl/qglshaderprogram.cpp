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

#include <qglshaderprogram.h>
#include <qopenglextensions_p.h>
#include <qgl_p.h>
#include <QtCore/qdebug.h>
#include <QtCore/qfile.h>
#include <QtCore/qvarlengtharray.h>
#include <QtCore/qvector.h>

#ifndef GL_FRAGMENT_SHADER
#define GL_FRAGMENT_SHADER 0x8B30
#endif

#ifndef GL_VERTEX_SHADER
#define GL_VERTEX_SHADER 0x8B31
#endif

#ifndef GL_COMPILE_STATUS
#define GL_COMPILE_STATUS 0x8B81
#endif

#ifndef GL_LINK_STATUS
#define GL_LINK_STATUS 0x8B82
#endif

#ifndef GL_INFO_LOG_LENGTH
#define GL_INFO_LOG_LENGTH 0x8B84
#endif

#ifndef GL_ACTIVE_UNIFORMS
#define GL_ACTIVE_UNIFORMS 0x8B86
#endif

#ifndef GL_ACTIVE_UNIFORM_MAX_LENGTH
#define GL_ACTIVE_UNIFORM_MAX_LENGTH 0x8B87
#endif

#ifndef GL_ACTIVE_ATTRIBUTES
#define GL_ACTIVE_ATTRIBUTES 0x8B89
#endif

#ifndef GL_ACTIVE_ATTRIBUTE_MAX_LENGTH
#define GL_ACTIVE_ATTRIBUTE_MAX_LENGTH 0x8B8A
#endif

#ifndef GL_CURRENT_VERTEX_ATTRIB
#define GL_CURRENT_VERTEX_ATTRIB 0x8626
#endif

#ifndef GL_SHADER_SOURCE_LENGTH
#define GL_SHADER_SOURCE_LENGTH 0x8B88
#endif

#ifndef GL_SHADER_BINARY_FORMATS
#define GL_SHADER_BINARY_FORMATS          0x8DF8
#endif

#ifndef GL_NUM_SHADER_BINARY_FORMATS
#define GL_NUM_SHADER_BINARY_FORMATS      0x8DF9
#endif

class QGLShaderPrivate
{
   Q_DECLARE_PUBLIC(QGLShader)

 public:
   QGLShaderPrivate(const QGLContext *ctx, QGLShader::ShaderType type)
      : shaderGuard(0), shaderType(type), compiled(false),
        glfuncs(new QOpenGLFunctions(ctx->contextHandle()))
   { }

   virtual ~QGLShaderPrivate();

   QGLSharedResourceGuardBase *shaderGuard;
   QGLShader::ShaderType shaderType;
   bool compiled;
   QString log;

   QOpenGLFunctions *glfuncs;
   bool create();
   bool compile(QGLShader *q);
   void deleteShader();

 protected:
   QGLShader *q_ptr;

};

namespace {
void freeShaderFunc(QGLContext *ctx, GLuint id)
{
   Q_ASSERT(ctx);
   ctx->contextHandle()->functions()->glDeleteShader(id);
}
}

#define ctx QGLContext::currentContext()

QGLShaderPrivate::~QGLShaderPrivate()
{
   delete glfuncs;

   if (shaderGuard) {
      shaderGuard->free();
   }
}

bool QGLShaderPrivate::create()
{
   QGLContext *context = const_cast<QGLContext *>(QGLContext::currentContext());
   if (!context) {
      return false;
   }

   if (glfuncs->hasOpenGLFeature(QOpenGLFunctions::Shaders)) {
      GLuint shader;
      if (shaderType == QGLShader::Vertex) {
         shader = glfuncs->glCreateShader(GL_VERTEX_SHADER);
      }

#if !defined(QT_OPENGL_ES_2)
      else if (shaderType == QGLShader::Geometry
         && !context->contextHandle()->isOpenGLES()) {
         shader = glfuncs->glCreateShader(GL_GEOMETRY_SHADER_EXT);
      }
#endif

      else {
         shader = glfuncs->glCreateShader(GL_FRAGMENT_SHADER);
      }
      if (!shader) {
         qWarning("Could not create shader of type %d.", int(shaderType));
         return false;
      }
      shaderGuard = createSharedResourceGuard(context, shader, freeShaderFunc);
      return true;
   } else {
      return false;
   }
}

bool QGLShaderPrivate::compile(QGLShader *q)
{
   GLuint shader = shaderGuard ? shaderGuard->id() : 0;
   if (!shader) {
      return false;
   }
   glfuncs->glCompileShader(shader);
   GLint value = 0;
   glfuncs->glGetShaderiv(shader, GL_COMPILE_STATUS, &value);
   compiled = (value != 0);
   value = 0;
   glfuncs->glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &value);
   if (!compiled && value > 1) {
      char *logbuf = new char [value];
      GLint len;
      glfuncs->glGetShaderInfoLog(shader, value, &len, logbuf);
      log = QString::fromLatin1(logbuf);
      QString name = q->objectName();

      const char *types[] = {
         "Fragment",
         "Vertex",
         "Geometry",
         ""
      };

      const char *type = types[3];
      if (shaderType == QGLShader::Fragment) {
         type = types[0];
      } else if (shaderType == QGLShader::Vertex) {
         type = types[1];
      } else if (shaderType == QGLShader::Geometry) {
         type = types[2];
      }

      if (name.isEmpty()) {
         qWarning("QGLShader::compile(%s): %s", type, qPrintable(log));
      } else {
         qWarning("QGLShader::compile(%s)[%s]: %s", type, qPrintable(name), qPrintable(log));
      }

      delete [] logbuf;
   }
   return compiled;
}

void QGLShaderPrivate::deleteShader()
{
   if (shaderGuard) {
      shaderGuard->free();
      shaderGuard = 0;
   }
}

QGLShader::QGLShader(QGLShader::ShaderType type, QObject *parent)
   : QObject(parent), d_ptr(new QGLShaderPrivate(QGLContext::currentContext(), type))
{
   Q_D(QGLShader);

   d->create();
}

QGLShader::QGLShader(QGLShader::ShaderType type, const QGLContext *context, QObject *parent)
   : QObject(parent), d_ptr(new QGLShaderPrivate(context ? context : QGLContext::currentContext(), type))
{
   Q_D(QGLShader);

#ifndef QT_NO_DEBUG
   if (context && !QGLContext::areSharing(context, QGLContext::currentContext())) {
      qWarning("QGLShader::QGLShader: \'context\' must be the current context or sharing with it.");
      return;
   }
#endif

   d->create();
}

QGLShader::~QGLShader()
{
}

QGLShader::ShaderType QGLShader::shaderType() const
{
   Q_D(const QGLShader);
   return d->shaderType;
}

// The precision qualifiers are useful on OpenGL/ES systems,
// but usually not present on desktop systems.  Define the
// keywords to empty strings on desktop systems.

#if !defined(QT_OPENGL_ES) || defined(QT_OPENGL_FORCE_SHADER_DEFINES)

#define QGL_DEFINE_QUALIFIERS 1
static const char qualifierDefines[] =
   "#define lowp\n"
   "#define mediump\n"
   "#define highp\n";

#else

// The "highp" qualifier doesn't exist in fragment shaders
// on all ES platforms.  When it doesn't exist, use "mediump".

#define QGL_REDEFINE_HIGHP 1
static const char redefineHighp[] =
   "#ifndef GL_FRAGMENT_PRECISION_HIGH\n"
   "#define highp mediump\n"
   "#endif\n";
#endif


bool QGLShader::compileSourceCode(const char *source)
{
   Q_D(QGLShader);

   if (d->shaderGuard && d->shaderGuard->id()) {
      QVarLengthArray<const char *, 4> src;
      QVarLengthArray<GLint, 4> srclen;
      int headerLen = 0;
      while (source && source[headerLen] == '#') {
         // Skip #version and #extension directives at the start of
         // the shader code.  We need to insert the qualifierDefines
         // and redefineHighp just after them.
         if (qstrncmp(source + headerLen, "#version", 8) != 0 &&
            qstrncmp(source + headerLen, "#extension", 10) != 0) {
            break;
         }
         while (source[headerLen] != '\0' && source[headerLen] != '\n') {
            ++headerLen;
         }
         if (source[headerLen] == '\n') {
            ++headerLen;
         }
      }

      if (headerLen > 0) {
         src.append(source);
         srclen.append(GLint(headerLen));
      }

#ifdef QGL_DEFINE_QUALIFIERS
      if (!QOpenGLContext::currentContext()->isOpenGLES()) {
         src.append(qualifierDefines);
         srclen.append(GLint(sizeof(qualifierDefines) - 1));
      }
#endif

#ifdef QGL_REDEFINE_HIGHP
      if (d->shaderType == Fragment
         && QOpenGLContext::currentContext()->isOpenGLES()) {
         src.append(redefineHighp);
         srclen.append(GLint(sizeof(redefineHighp) - 1));
      }
#endif
      src.append(source + headerLen);
      srclen.append(GLint(qstrlen(source + headerLen)));
      d->glfuncs->glShaderSource(d->shaderGuard->id(), src.size(), src.data(), srclen.data());
      return d->compile(this);
   } else {
      return false;
   }
}

/*!
    \overload

    Sets the \a source code for this shader and compiles it.
    Returns true if the source was successfully compiled, false otherwise.

    \sa compileSourceFile()
*/
bool QGLShader::compileSourceCode(const QByteArray &source)
{
   return compileSourceCode(source.constData());
}

/*!
    \overload

    Sets the \a source code for this shader and compiles it.
    Returns true if the source was successfully compiled, false otherwise.

    \sa compileSourceFile()
*/
bool QGLShader::compileSourceCode(const QString &source)
{
   return compileSourceCode(source.toLatin1().constData());
}

/*!
    Sets the source code for this shader to the contents of \a fileName
    and compiles it.  Returns true if the file could be opened and the
    source compiled, false otherwise.

    \sa compileSourceCode()
*/
bool QGLShader::compileSourceFile(const QString &fileName)
{
   QFile file(fileName);
   if (!file.open(QFile::ReadOnly)) {
      qWarning() << "QGLShader: Unable to open file" << fileName;
      return false;
   }

   QByteArray contents = file.readAll();
   return compileSourceCode(contents.constData());
}

/*!
    Returns the source code for this shader.

    \sa compileSourceCode()
*/
QByteArray QGLShader::sourceCode() const
{
   Q_D(const QGLShader);
   GLuint shader = d->shaderGuard ? d->shaderGuard->id() : 0;
   if (!shader) {
      return QByteArray();
   }
   GLint size = 0;
   d->glfuncs->glGetShaderiv(shader, GL_SHADER_SOURCE_LENGTH, &size);
   if (size <= 0) {
      return QByteArray();
   }
   GLint len = 0;
   char *source = new char [size];
   d->glfuncs->glGetShaderSource(shader, size, &len, source);
   QByteArray src(source);
   delete [] source;
   return src;
}

/*!
    Returns true if this shader has been compiled; false otherwise.

    \sa compileSourceCode(), compileSourceFile()
*/
bool QGLShader::isCompiled() const
{
   Q_D(const QGLShader);
   return d->compiled;
}

/*!
    Returns the errors and warnings that occurred during the last compile.

    \sa compileSourceCode(), compileSourceFile()
*/
QString QGLShader::log() const
{
   Q_D(const QGLShader);
   return d->log;
}

GLuint QGLShader::shaderId() const
{
   Q_D(const QGLShader);
   return d->shaderGuard ? d->shaderGuard->id() : 0;
}


#undef ctx
class ShaderProgramOpenGLFunctions : public QOpenGLFunctions
{
 public:
   ShaderProgramOpenGLFunctions()
      : QOpenGLFunctions()
      , glProgramParameteri(0) {
   }

   typedef void (QOPENGLF_APIENTRYP type_glProgramParameteri)(GLuint program, GLenum pname, GLint value);

   void initializeGeometryShaderFunctions() {
      QOpenGLContext *context = QOpenGLContext::currentContext();
      if (!context->isOpenGLES()) {
         glProgramParameteri = (type_glProgramParameteri)
            context->getProcAddress("glProgramParameteri");

         if (!glProgramParameteri) {
            glProgramParameteri = (type_glProgramParameteri)
               context->getProcAddress("glProgramParameteriEXT");
         }
      }
   }

   type_glProgramParameteri glProgramParameteri;
};
class QGLShaderProgramPrivate
{
   Q_DECLARE_PUBLIC(QGLShaderProgram)

 public:
   QGLShaderProgramPrivate(const QGLContext *context)
      : programGuard(0), linked(false), inited(false), removingShaders(false), geometryVertexCount(64)
      , geometryInputType(0), geometryOutputType(0), glfuncs(new ShaderProgramOpenGLFunctions)
   {
      (void) context;
   }

   ~QGLShaderProgramPrivate();

   QGLSharedResourceGuardBase *programGuard;
   bool linked;
   bool inited;
   bool removingShaders;

   int geometryVertexCount;
   GLenum geometryInputType;
   GLenum geometryOutputType;

   QString log;
   QList<QGLShader *> shaders;
   QList<QGLShader *> anonShaders;

   ShaderProgramOpenGLFunctions *glfuncs;
   bool hasShader(QGLShader::ShaderType type) const;

 protected:
   QGLShaderProgram *q_ptr;
};

namespace {
    void freeProgramFunc(QGLContext *ctx, GLuint id)
    {
        Q_ASSERT(ctx);
        ctx->contextHandle()->functions()->glDeleteProgram(id);
    }
}
QGLShaderProgramPrivate::~QGLShaderProgramPrivate()
{
   delete glfuncs;
   if (programGuard) {
      programGuard->free();
   }
}

bool QGLShaderProgramPrivate::hasShader(QGLShader::ShaderType type) const
{
   for (QGLShader *shader : shaders) {
      if (shader->shaderType() == type) {
         return true;
      }
   }

   return false;
}

#define ctx QGLContext::currentContext()

/*!
    Constructs a new shader program and attaches it to \a parent.
    The program will be invalid until addShader() is called.

    The shader program will be associated with the current QGLContext.

    \sa addShader()
*/
QGLShaderProgram::QGLShaderProgram(QObject *parent)
   : QObject(parent), d_ptr(new QGLShaderProgramPrivate(QGLContext::currentContext()))
{
   d_ptr->q_ptr = this;
}

QGLShaderProgram::QGLShaderProgram(const QGLContext *context, QObject *parent)
   : QObject(parent), d_ptr(new QGLShaderProgramPrivate(context))
{
   d_ptr->q_ptr = this;
}

/*!
    Deletes this shader program.
*/
QGLShaderProgram::~QGLShaderProgram()
{
}

bool QGLShaderProgram::init()
{
   Q_D(QGLShaderProgram);
   if ((d->programGuard && d->programGuard->id()) || d->inited) {
      return true;
   }

   d->inited = true;
   QGLContext *context = const_cast<QGLContext *>(QGLContext::currentContext());

   if (! context) {
      return false;
   }

   d->glfuncs->initializeOpenGLFunctions();
   d->glfuncs->initializeGeometryShaderFunctions();

   if (d->glfuncs->hasOpenGLFeature(QOpenGLFunctions::Shaders)) {
      GLuint program = d->glfuncs->glCreateProgram();
      if (!program) {
         qWarning() << "QGLShaderProgram: could not create shader program";
         return false;
      }
      if (d->programGuard) {
         delete d->programGuard;
      }
      d->programGuard = createSharedResourceGuard(context, program, freeProgramFunc);
      return true;

   } else {
      qWarning() << "QGLShaderProgram: shader programs are not supported";
      return false;
   }
}

bool QGLShaderProgram::addShader(QGLShader *shader)
{
   Q_D(QGLShaderProgram);
   if (!init()) {
      return false;
   }
   if (d->shaders.contains(shader)) {
      return true;   // Already added to this shader program.
   }
   if (d->programGuard && d->programGuard->id() && shader) {
      if (!shader->d_func()->shaderGuard || !shader->d_func()->shaderGuard->id()) {
         return false;
      }
      if (d->programGuard->group() != shader->d_func()->shaderGuard->group()) {
         qWarning("QGLShaderProgram::addShader: Program and shader are not associated with same context.");
         return false;
      }
      d->glfuncs->glAttachShader(d->programGuard->id(), shader->d_func()->shaderGuard->id());
      d->linked = false;  // Program needs to be relinked.
      d->shaders.append(shader);
      connect(shader, SIGNAL(destroyed()), this, SLOT(shaderDestroyed()));
      return true;
   } else {
      return false;
   }
}

/*!
    Compiles \a source as a shader of the specified \a type and
    adds it to this shader program.  Returns true if compilation
    was successful, false otherwise.  The compilation errors
    and warnings will be made available via log().

    This function is intended to be a short-cut for quickly
    adding vertex and fragment shaders to a shader program without
    creating an instance of QGLShader first.

    \sa addShader(), addShaderFromSourceFile()
    \sa removeShader(), link(), log(), removeAllShaders()
*/
bool QGLShaderProgram::addShaderFromSourceCode(QGLShader::ShaderType type, const char *source)
{
   Q_D(QGLShaderProgram);
   if (!init()) {
      return false;
   }
   QGLShader *shader = new QGLShader(type, this);
   if (!shader->compileSourceCode(source)) {
      d->log = shader->log();
      delete shader;
      return false;
   }
   d->anonShaders.append(shader);
   return addShader(shader);
}

/*!
    \overload

    Compiles \a source as a shader of the specified \a type and
    adds it to this shader program.  Returns true if compilation
    was successful, false otherwise.  The compilation errors
    and warnings will be made available via log().

    This function is intended to be a short-cut for quickly
    adding vertex and fragment shaders to a shader program without
    creating an instance of QGLShader first.

    \sa addShader(), addShaderFromSourceFile()
    \sa removeShader(), link(), log(), removeAllShaders()
*/
bool QGLShaderProgram::addShaderFromSourceCode(QGLShader::ShaderType type, const QByteArray &source)
{
   return addShaderFromSourceCode(type, source.constData());
}

/*!
    \overload

    Compiles \a source as a shader of the specified \a type and
    adds it to this shader program.  Returns true if compilation
    was successful, false otherwise.  The compilation errors
    and warnings will be made available via log().

    This function is intended to be a short-cut for quickly
    adding vertex and fragment shaders to a shader program without
    creating an instance of QGLShader first.

    \sa addShader(), addShaderFromSourceFile()
    \sa removeShader(), link(), log(), removeAllShaders()
*/
bool QGLShaderProgram::addShaderFromSourceCode(QGLShader::ShaderType type, const QString &source)
{
   return addShaderFromSourceCode(type, source.toLatin1().constData());
}

/*!
    Compiles the contents of \a fileName as a shader of the specified
    \a type and adds it to this shader program.  Returns true if
    compilation was successful, false otherwise.  The compilation errors
    and warnings will be made available via log().

    This function is intended to be a short-cut for quickly
    adding vertex and fragment shaders to a shader program without
    creating an instance of QGLShader first.

    \sa addShader(), addShaderFromSourceCode()
*/
bool QGLShaderProgram::addShaderFromSourceFile
(QGLShader::ShaderType type, const QString &fileName)
{
   Q_D(QGLShaderProgram);
   if (!init()) {
      return false;
   }

   QGLShader *shader = new QGLShader(type, this);
   if (!shader->compileSourceFile(fileName)) {
      d->log = shader->log();
      delete shader;
      return false;
   }
   d->anonShaders.append(shader);
   return addShader(shader);
}

/*!
    Removes \a shader from this shader program.  The object is not deleted.

    \sa addShader(), link(), removeAllShaders()
*/
void QGLShaderProgram::removeShader(QGLShader *shader)
{
   Q_D(QGLShaderProgram);
   if (d->programGuard && d->programGuard->id()
      && shader && shader->d_func()->shaderGuard) {
      d->glfuncs->glDetachShader(d->programGuard->id(), shader->d_func()->shaderGuard->id());
   }
   d->linked = false;  // Program needs to be relinked.
   if (shader) {
      d->shaders.removeAll(shader);
      d->anonShaders.removeAll(shader);
      disconnect(shader, SIGNAL(destroyed()), this, SLOT(shaderDestroyed()));
   }
}

/*!
    Returns a list of all shaders that have been added to this shader
    program using addShader().

    \sa addShader(), removeShader()
*/
QList<QGLShader *> QGLShaderProgram::shaders() const
{
   Q_D(const QGLShaderProgram);
   return d->shaders;
}

/*!
    Removes all of the shaders that were added to this program previously.
    The QGLShader objects for the shaders will not be deleted if they
    were constructed externally.  QGLShader objects that are constructed
    internally by QGLShaderProgram will be deleted.

    \sa addShader(), removeShader()
*/
void QGLShaderProgram::removeAllShaders()
{
   Q_D(QGLShaderProgram);
   d->removingShaders = true;

   for (QGLShader *shader : d->shaders) {
      if (d->programGuard && d->programGuard->id()
         && shader && shader->d_func()->shaderGuard) {
         d->glfuncs->glDetachShader(d->programGuard->id(), shader->d_func()->shaderGuard->id());
      }
   }

   for (QGLShader *shader : d->anonShaders) {
      // Delete shader objects that were created anonymously.
      delete shader;
   }

   d->shaders.clear();
   d->anonShaders.clear();
   d->linked = false;  // Program needs to be relinked.
   d->removingShaders = false;
}

/*!
    Links together the shaders that were added to this program with
    addShader().  Returns true if the link was successful or
    false otherwise.  If the link failed, the error messages can
    be retrieved with log().

    Subclasses can override this function to initialize attributes
    and uniform variables for use in specific shader programs.

    If the shader program was already linked, calling this
    function again will force it to be re-linked.

    \sa addShader(), log()
*/
bool QGLShaderProgram::link()
{
   Q_D(QGLShaderProgram);
   GLuint program = d->programGuard ? d->programGuard->id() : 0;

   if (!program) {
      return false;
   }

   GLint value;
   if (d->shaders.isEmpty()) {
      // If there are no explicit shaders, then it is possible that the
      // application added a program binary with glProgramBinaryOES(),
      // or otherwise populated the shaders itself.  Check to see if the
      // program is already linked and bail out if so.
      value = 0;
      d->glfuncs->glGetProgramiv(program, GL_LINK_STATUS, &value);
      d->linked = (value != 0);
      if (d->linked) {
         return true;
      }
   }

#if !defined(QT_OPENGL_ES_2)
   // Set up the geometry shader parameters
   if (!QOpenGLContext::currentContext()->isOpenGLES() && d->glfuncs->glProgramParameteri) {
      for (QGLShader *shader : d->shaders) {
         if (shader->shaderType() & QGLShader::Geometry) {
            d->glfuncs->glProgramParameteri(program, GL_GEOMETRY_INPUT_TYPE_EXT,
               d->geometryInputType);
            d->glfuncs->glProgramParameteri(program, GL_GEOMETRY_OUTPUT_TYPE_EXT,
               d->geometryOutputType);
            d->glfuncs->glProgramParameteri(program, GL_GEOMETRY_VERTICES_OUT_EXT,
               d->geometryVertexCount);
            break;
         }
      }
   }
#endif

   d->glfuncs->glLinkProgram(program);
   value = 0;
   d->glfuncs->glGetProgramiv(program, GL_LINK_STATUS, &value);
   d->linked = (value != 0);
   value = 0;
   d->glfuncs->glGetProgramiv(program, GL_INFO_LOG_LENGTH, &value);
   d->log = QString();
   if (value > 1) {
      char *logbuf = new char [value];
      GLint len;
      d->glfuncs->glGetProgramInfoLog(program, value, &len, logbuf);
      d->log = QString::fromLatin1(logbuf);
      QString name = objectName();
      if (!d->linked) {
         if (name.isEmpty()) {
            qWarning() << "QGLShader::link:" << d->log;
         } else {
            qWarning() << "QGLShader::link[" << name << "]:" << d->log;
         }
      }
      delete [] logbuf;
   }

   return d->linked;
}

/*!
    Returns true if this shader program has been linked; false otherwise.

    \sa link()
*/
bool QGLShaderProgram::isLinked() const
{
   Q_D(const QGLShaderProgram);
   return d->linked;
}

/*!
    Returns the errors and warnings that occurred during the last link()
    or addShader() with explicitly specified source code.

    \sa link()
*/
QString QGLShaderProgram::log() const
{
   Q_D(const QGLShaderProgram);
   return d->log;
}

/*!
    Binds this shader program to the active QGLContext and makes
    it the current shader program.  Any previously bound shader program
    is released.  This is equivalent to calling \c{glUseProgram()} on
    programId().  Returns true if the program was successfully bound;
    false otherwise.  If the shader program has not yet been linked,
    or it needs to be re-linked, this function will call link().

    \sa link(), release()
*/
bool QGLShaderProgram::bind()
{
   Q_D(QGLShaderProgram);
   GLuint program = d->programGuard ? d->programGuard->id() : 0;
   if (!program) {
      return false;
   }
   if (!d->linked && !link()) {
      return false;
   }
#ifndef QT_NO_DEBUG
   if (d->programGuard->group() != QOpenGLContextGroup::currentContextGroup()) {
      qWarning("QGLShaderProgram::bind: program is not valid in the current context.");
      return false;
   }
#endif
   d->glfuncs->glUseProgram(program);
   return true;
}

#undef ctx
#define ctx QGLContext::currentContext()

/*!
    Releases the active shader program from the current QGLContext.
    This is equivalent to calling \c{glUseProgram(0)}.

    \sa bind()
*/
void QGLShaderProgram::release()
{
   Q_D(QGLShaderProgram);
#ifndef QT_NO_DEBUG
   if (d->programGuard && d->programGuard->group() != QOpenGLContextGroup::currentContextGroup()) {
      qWarning("QGLShaderProgram::release: program is not valid in the current context.");
   }
#endif
   d->glfuncs->glUseProgram(0);
}

/*!
    Returns the OpenGL identifier associated with this shader program.

    \sa QGLShader::shaderId()
*/
GLuint QGLShaderProgram::programId() const
{
   Q_D(const QGLShaderProgram);
   GLuint id = d->programGuard ? d->programGuard->id() : 0;
   if (id) {
      return id;
   }

   // Create the identifier if we don't have one yet.  This is for
   // applications that want to create the attached shader configuration
   // themselves, particularly those using program binaries.
   if (!const_cast<QGLShaderProgram *>(this)->init()) {
      return 0;
   }

   return d->programGuard ? d->programGuard->id() : 0;
}

/*!
    Binds the attribute \a name to the specified \a location.  This
    function can be called before or after the program has been linked.
    Any attributes that have not been explicitly bound when the program
    is linked will be assigned locations automatically.

    When this function is called after the program has been linked,
    the program will need to be relinked for the change to take effect.

    \sa attributeLocation()
*/
void QGLShaderProgram::bindAttributeLocation(const char *name, int location)
{
   Q_D(QGLShaderProgram);
   if (!init() || !d->programGuard || !d->programGuard->id()) {
      return;
   }
   d->glfuncs->glBindAttribLocation(d->programGuard->id(), location, name);
   d->linked = false;  // Program needs to be relinked.
}

/*!
    \overload

    Binds the attribute \a name to the specified \a location.  This
    function can be called before or after the program has been linked.
    Any attributes that have not been explicitly bound when the program
    is linked will be assigned locations automatically.

    When this function is called after the program has been linked,
    the program will need to be relinked for the change to take effect.

    \sa attributeLocation()
*/
void QGLShaderProgram::bindAttributeLocation(const QByteArray &name, int location)
{
   bindAttributeLocation(name.constData(), location);
}

/*!
    \overload

    Binds the attribute \a name to the specified \a location.  This
    function can be called before or after the program has been linked.
    Any attributes that have not been explicitly bound when the program
    is linked will be assigned locations automatically.

    When this function is called after the program has been linked,
    the program will need to be relinked for the change to take effect.

    \sa attributeLocation()
*/
void QGLShaderProgram::bindAttributeLocation(const QString &name, int location)
{
   bindAttributeLocation(name.toLatin1().constData(), location);
}

/*!
    Returns the location of the attribute \a name within this shader
    program's parameter list.  Returns -1 if \a name is not a valid
    attribute for this shader program.

    \sa uniformLocation(), bindAttributeLocation()
*/
int QGLShaderProgram::attributeLocation(const char *name) const
{
   Q_D(const QGLShaderProgram);
   if (d->linked && d->programGuard && d->programGuard->id()) {
      return d->glfuncs->glGetAttribLocation(d->programGuard->id(), name);
   } else {
      qWarning() << "QGLShaderProgram::attributeLocation(" << name
         << "): shader program is not linked";
      return -1;
   }
}

/*!
    \overload

    Returns the location of the attribute \a name within this shader
    program's parameter list.  Returns -1 if \a name is not a valid
    attribute for this shader program.

    \sa uniformLocation(), bindAttributeLocation()
*/
int QGLShaderProgram::attributeLocation(const QByteArray &name) const
{
   return attributeLocation(name.constData());
}

/*!
    \overload

    Returns the location of the attribute \a name within this shader
    program's parameter list.  Returns -1 if \a name is not a valid
    attribute for this shader program.

    \sa uniformLocation(), bindAttributeLocation()
*/
int QGLShaderProgram::attributeLocation(const QString &name) const
{
   return attributeLocation(name.toLatin1().constData());
}

/*!
    Sets the attribute at \a location in the current context to \a value.

    \sa setUniformValue()
*/
void QGLShaderProgram::setAttributeValue(int location, GLfloat value)
{
   Q_D(QGLShaderProgram);

   if (location != -1) {
      d->glfuncs->glVertexAttrib1fv(location, &value);
   }
}

/*!
    \overload

    Sets the attribute called \a name in the current context to \a value.

    \sa setUniformValue()
*/
void QGLShaderProgram::setAttributeValue(const char *name, GLfloat value)
{
   setAttributeValue(attributeLocation(name), value);
}

/*!
    Sets the attribute at \a location in the current context to
    the 2D vector (\a x, \a y).

    \sa setUniformValue()
*/
void QGLShaderProgram::setAttributeValue(int location, GLfloat x, GLfloat y)
{
   Q_D(QGLShaderProgram);

   if (location != -1) {
      GLfloat values[2] = {x, y};
      d->glfuncs->glVertexAttrib2fv(location, values);
   }
}

/*!
    \overload

    Sets the attribute called \a name in the current context to
    the 2D vector (\a x, \a y).

    \sa setUniformValue()
*/
void QGLShaderProgram::setAttributeValue(const char *name, GLfloat x, GLfloat y)
{
   setAttributeValue(attributeLocation(name), x, y);
}

/*!
    Sets the attribute at \a location in the current context to
    the 3D vector (\a x, \a y, \a z).

    \sa setUniformValue()
*/
void QGLShaderProgram::setAttributeValue
(int location, GLfloat x, GLfloat y, GLfloat z)
{
   Q_D(QGLShaderProgram);
   Q_UNUSED(d);
   if (location != -1) {
      GLfloat values[3] = {x, y, z};
      d->glfuncs->glVertexAttrib3fv(location, values);
   }
}

void QGLShaderProgram::setAttributeValue(const char *name, GLfloat x, GLfloat y, GLfloat z)
{
   setAttributeValue(attributeLocation(name), x, y, z);
}

void QGLShaderProgram::setAttributeValue(int location, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
   Q_D(QGLShaderProgram);

   if (location != -1) {
      GLfloat values[4] = {x, y, z, w};
      d->glfuncs->glVertexAttrib4fv(location, values);
   }
}

void QGLShaderProgram::setAttributeValue(const char *name, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
   setAttributeValue(attributeLocation(name), x, y, z, w);
}


void QGLShaderProgram::setAttributeValue(int location, const QVector2D &value)
{
   Q_D(QGLShaderProgram);

   if (location != -1) {
      d->glfuncs->glVertexAttrib2fv(location, reinterpret_cast<const GLfloat *>(&value));
   }
}

/*!
    \overload

    Sets the attribute called \a name in the current context to \a value.

    \sa setUniformValue()
*/
void QGLShaderProgram::setAttributeValue(const char *name, const QVector2D &value)
{
   setAttributeValue(attributeLocation(name), value);
}

/*!
    Sets the attribute at \a location in the current context to \a value.

    \sa setUniformValue()
*/
void QGLShaderProgram::setAttributeValue(int location, const QVector3D &value)
{
   Q_D(QGLShaderProgram);
   Q_UNUSED(d);
   if (location != -1) {
      d->glfuncs->glVertexAttrib3fv(location, reinterpret_cast<const GLfloat *>(&value));
   }
}

/*!
    \overload

    Sets the attribute called \a name in the current context to \a value.

    \sa setUniformValue()
*/
void QGLShaderProgram::setAttributeValue(const char *name, const QVector3D &value)
{
   setAttributeValue(attributeLocation(name), value);
}

/*!
    Sets the attribute at \a location in the current context to \a value.

    \sa setUniformValue()
*/
void QGLShaderProgram::setAttributeValue(int location, const QVector4D &value)
{
   Q_D(QGLShaderProgram);
   Q_UNUSED(d);
   if (location != -1) {
      d->glfuncs->glVertexAttrib4fv(location, reinterpret_cast<const GLfloat *>(&value));
   }
}

/*!
    \overload

    Sets the attribute called \a name in the current context to \a value.

    \sa setUniformValue()
*/
void QGLShaderProgram::setAttributeValue(const char *name, const QVector4D &value)
{
   setAttributeValue(attributeLocation(name), value);
}

/*!
    Sets the attribute at \a location in the current context to \a value.

    \sa setUniformValue()
*/
void QGLShaderProgram::setAttributeValue(int location, const QColor &value)
{
   Q_D(QGLShaderProgram);
   Q_UNUSED(d);

   if (location != -1) {
      GLfloat values[4] = {GLfloat(value.redF()), GLfloat(value.greenF()),
            GLfloat(value.blueF()), GLfloat(value.alphaF())
         };
      d->glfuncs->glVertexAttrib4fv(location, values);
   }
}

/*!
    \overload

    Sets the attribute called \a name in the current context to \a value.

    \sa setUniformValue()
*/
void QGLShaderProgram::setAttributeValue(const char *name, const QColor &value)
{
   setAttributeValue(attributeLocation(name), value);
}

/*!
    Sets the attribute at \a location in the current context to the
    contents of \a values, which contains \a columns elements, each
    consisting of \a rows elements.  The \a rows value should be
    1, 2, 3, or 4.  This function is typically used to set matrix
    values and column vectors.

    \sa setUniformValue()
*/
void QGLShaderProgram::setAttributeValue
(int location, const GLfloat *values, int columns, int rows)
{
   Q_D(QGLShaderProgram);
   Q_UNUSED(d);
   if (rows < 1 || rows > 4) {
      qWarning() << "QGLShaderProgram::setAttributeValue: rows" << rows << "not supported";
      return;
   }
   if (location != -1) {
      while (columns-- > 0) {
         if (rows == 1) {
            d->glfuncs->glVertexAttrib1fv(location, values);
         } else if (rows == 2) {
            d->glfuncs->glVertexAttrib2fv(location, values);
         } else if (rows == 3) {
            d->glfuncs->glVertexAttrib3fv(location, values);
         } else {
            d->glfuncs->glVertexAttrib4fv(location, values);
         }

         values += rows;
         ++location;
      }
   }
}

/*!
    \overload

    Sets the attribute called \a name in the current context to the
    contents of \a values, which contains \a columns elements, each
    consisting of \a rows elements.  The \a rows value should be
    1, 2, 3, or 4.  This function is typically used to set matrix
    values and column vectors.

    \sa setUniformValue()
*/
void QGLShaderProgram::setAttributeValue
(const char *name, const GLfloat *values, int columns, int rows)
{
   setAttributeValue(attributeLocation(name), values, columns, rows);
}

/*!
    Sets an array of vertex \a values on the attribute at \a location
    in this shader program.  The \a tupleSize indicates the number of
    components per vertex (1, 2, 3, or 4), and the \a stride indicates
    the number of bytes between vertices.  A default \a stride value
    of zero indicates that the vertices are densely packed in \a values.

    The array will become active when enableAttributeArray() is called
    on the \a location.  Otherwise the value specified with
    setAttributeValue() for \a location will be used.

    \sa setAttributeValue(), setUniformValue(), enableAttributeArray()
    \sa disableAttributeArray()
*/
void QGLShaderProgram::setAttributeArray
(int location, const GLfloat *values, int tupleSize, int stride)
{
   Q_D(QGLShaderProgram);
   Q_UNUSED(d);
   if (location != -1) {
      d->glfuncs->glVertexAttribPointer(location, tupleSize, GL_FLOAT, GL_FALSE,
         stride, values);
   }
}

/*!
    Sets an array of 2D vertex \a values on the attribute at \a location
    in this shader program.  The \a stride indicates the number of bytes
    between vertices.  A default \a stride value of zero indicates that
    the vertices are densely packed in \a values.

    The array will become active when enableAttributeArray() is called
    on the \a location.  Otherwise the value specified with
    setAttributeValue() for \a location will be used.

    \sa setAttributeValue(), setUniformValue(), enableAttributeArray()
    \sa disableAttributeArray()
*/
void QGLShaderProgram::setAttributeArray
(int location, const QVector2D *values, int stride)
{
   Q_D(QGLShaderProgram);
   Q_UNUSED(d);
   if (location != -1) {
      d->glfuncs->glVertexAttribPointer(location, 2, GL_FLOAT, GL_FALSE,
         stride, values);
   }
}

/*!
    Sets an array of 3D vertex \a values on the attribute at \a location
    in this shader program.  The \a stride indicates the number of bytes
    between vertices.  A default \a stride value of zero indicates that
    the vertices are densely packed in \a values.

    The array will become active when enableAttributeArray() is called
    on the \a location.  Otherwise the value specified with
    setAttributeValue() for \a location will be used.

    \sa setAttributeValue(), setUniformValue(), enableAttributeArray()
    \sa disableAttributeArray()
*/
void QGLShaderProgram::setAttributeArray
(int location, const QVector3D *values, int stride)
{
   Q_D(QGLShaderProgram);
   Q_UNUSED(d);
   if (location != -1) {
      d->glfuncs->glVertexAttribPointer(location, 3, GL_FLOAT, GL_FALSE,
         stride, values);
   }
}

/*!
    Sets an array of 4D vertex \a values on the attribute at \a location
    in this shader program.  The \a stride indicates the number of bytes
    between vertices.  A default \a stride value of zero indicates that
    the vertices are densely packed in \a values.

    The array will become active when enableAttributeArray() is called
    on the \a location.  Otherwise the value specified with
    setAttributeValue() for \a location will be used.

    \sa setAttributeValue(), setUniformValue(), enableAttributeArray()
    \sa disableAttributeArray()
*/
void QGLShaderProgram::setAttributeArray
(int location, const QVector4D *values, int stride)
{
   Q_D(QGLShaderProgram);
   Q_UNUSED(d);
   if (location != -1) {
      d->glfuncs->glVertexAttribPointer(location, 4, GL_FLOAT, GL_FALSE,
         stride, values);
   }
}

/*!
    Sets an array of vertex \a values on the attribute at \a location
    in this shader program.  The \a stride indicates the number of bytes
    between vertices.  A default \a stride value of zero indicates that
    the vertices are densely packed in \a values.

    The \a type indicates the type of elements in the \a values array,
    usually \c{GL_FLOAT}, \c{GL_UNSIGNED_BYTE}, etc.  The \a tupleSize
    indicates the number of components per vertex: 1, 2, 3, or 4.

    The array will become active when enableAttributeArray() is called
    on the \a location.  Otherwise the value specified with
    setAttributeValue() for \a location will be used.

    The setAttributeBuffer() function can be used to set the attribute
    array to an offset within a vertex buffer.

    \sa setAttributeValue(), setUniformValue(), enableAttributeArray()
    \sa disableAttributeArray(), setAttributeBuffer()
    \since 4.7
*/
void QGLShaderProgram::setAttributeArray
(int location, GLenum type, const void *values, int tupleSize, int stride)
{
   Q_D(QGLShaderProgram);
   Q_UNUSED(d);
   if (location != -1) {
      d->glfuncs->glVertexAttribPointer(location, tupleSize, type, GL_TRUE,
         stride, values);
   }
}

/*!
    \overload

    Sets an array of vertex \a values on the attribute called \a name
    in this shader program.  The \a tupleSize indicates the number of
    components per vertex (1, 2, 3, or 4), and the \a stride indicates
    the number of bytes between vertices.  A default \a stride value
    of zero indicates that the vertices are densely packed in \a values.

    The array will become active when enableAttributeArray() is called
    on \a name.  Otherwise the value specified with setAttributeValue()
    for \a name will be used.

    \sa setAttributeValue(), setUniformValue(), enableAttributeArray()
    \sa disableAttributeArray()
*/
void QGLShaderProgram::setAttributeArray
(const char *name, const GLfloat *values, int tupleSize, int stride)
{
   setAttributeArray(attributeLocation(name), values, tupleSize, stride);
}

/*!
    \overload

    Sets an array of 2D vertex \a values on the attribute called \a name
    in this shader program.  The \a stride indicates the number of bytes
    between vertices.  A default \a stride value of zero indicates that
    the vertices are densely packed in \a values.

    The array will become active when enableAttributeArray() is called
    on \a name.  Otherwise the value specified with setAttributeValue()
    for \a name will be used.

    \sa setAttributeValue(), setUniformValue(), enableAttributeArray()
    \sa disableAttributeArray()
*/
void QGLShaderProgram::setAttributeArray
(const char *name, const QVector2D *values, int stride)
{
   setAttributeArray(attributeLocation(name), values, stride);
}

/*!
    \overload

    Sets an array of 3D vertex \a values on the attribute called \a name
    in this shader program.  The \a stride indicates the number of bytes
    between vertices.  A default \a stride value of zero indicates that
    the vertices are densely packed in \a values.

    The array will become active when enableAttributeArray() is called
    on \a name.  Otherwise the value specified with setAttributeValue()
    for \a name will be used.

    \sa setAttributeValue(), setUniformValue(), enableAttributeArray()
    \sa disableAttributeArray()
*/
void QGLShaderProgram::setAttributeArray
(const char *name, const QVector3D *values, int stride)
{
   setAttributeArray(attributeLocation(name), values, stride);
}

/*!
    \overload

    Sets an array of 4D vertex \a values on the attribute called \a name
    in this shader program.  The \a stride indicates the number of bytes
    between vertices.  A default \a stride value of zero indicates that
    the vertices are densely packed in \a values.

    The array will become active when enableAttributeArray() is called
    on \a name.  Otherwise the value specified with setAttributeValue()
    for \a name will be used.

    \sa setAttributeValue(), setUniformValue(), enableAttributeArray()
    \sa disableAttributeArray()
*/
void QGLShaderProgram::setAttributeArray
(const char *name, const QVector4D *values, int stride)
{
   setAttributeArray(attributeLocation(name), values, stride);
}

/*!
    \overload

    Sets an array of vertex \a values on the attribute called \a name
    in this shader program.  The \a stride indicates the number of bytes
    between vertices.  A default \a stride value of zero indicates that
    the vertices are densely packed in \a values.

    The \a type indicates the type of elements in the \a values array,
    usually \c{GL_FLOAT}, \c{GL_UNSIGNED_BYTE}, etc.  The \a tupleSize
    indicates the number of components per vertex: 1, 2, 3, or 4.

    The array will become active when enableAttributeArray() is called
    on the \a name.  Otherwise the value specified with
    setAttributeValue() for \a name will be used.

    The setAttributeBuffer() function can be used to set the attribute
    array to an offset within a vertex buffer.

    \sa setAttributeValue(), setUniformValue(), enableAttributeArray()
    \sa disableAttributeArray(), setAttributeBuffer()
    \since 4.7
*/
void QGLShaderProgram::setAttributeArray
(const char *name, GLenum type, const void *values, int tupleSize, int stride)
{
   setAttributeArray(attributeLocation(name), type, values, tupleSize, stride);
}

/*!
    Sets an array of vertex values on the attribute at \a location in
    this shader program, starting at a specific \a offset in the
    currently bound vertex buffer.  The \a stride indicates the number
    of bytes between vertices.  A default \a stride value of zero
    indicates that the vertices are densely packed in the value array.

    The \a type indicates the type of elements in the vertex value
    array, usually \c{GL_FLOAT}, \c{GL_UNSIGNED_BYTE}, etc.  The \a
    tupleSize indicates the number of components per vertex: 1, 2, 3,
    or 4.

    The array will become active when enableAttributeArray() is called
    on the \a location.  Otherwise the value specified with
    setAttributeValue() for \a location will be used.

    \sa setAttributeArray()
    \since 4.7
*/
void QGLShaderProgram::setAttributeBuffer
(int location, GLenum type, int offset, int tupleSize, int stride)
{
   Q_D(QGLShaderProgram);
   Q_UNUSED(d);
   if (location != -1) {
      d->glfuncs->glVertexAttribPointer(location, tupleSize, type, GL_TRUE, stride,
         reinterpret_cast<const void *>(qintptr(offset)));
   }
}

/*!
    \overload

    Sets an array of vertex values on the attribute called \a name
    in this shader program, starting at a specific \a offset in the
    currently bound vertex buffer.  The \a stride indicates the number
    of bytes between vertices.  A default \a stride value of zero
    indicates that the vertices are densely packed in the value array.

    The \a type indicates the type of elements in the vertex value
    array, usually \c{GL_FLOAT}, \c{GL_UNSIGNED_BYTE}, etc.  The \a
    tupleSize indicates the number of components per vertex: 1, 2, 3,
    or 4.

    The array will become active when enableAttributeArray() is called
    on the \a name.  Otherwise the value specified with
    setAttributeValue() for \a name will be used.

    \sa setAttributeArray()
    \since 4.7
*/
void QGLShaderProgram::setAttributeBuffer
(const char *name, GLenum type, int offset, int tupleSize, int stride)
{
   setAttributeBuffer(attributeLocation(name), type, offset, tupleSize, stride);
}

/*!
    Enables the vertex array at \a location in this shader program
    so that the value set by setAttributeArray() on \a location
    will be used by the shader program.

    \sa disableAttributeArray(), setAttributeArray(), setAttributeValue()
    \sa setUniformValue()
*/
void QGLShaderProgram::enableAttributeArray(int location)
{
   Q_D(QGLShaderProgram);
   Q_UNUSED(d);
   if (location != -1) {
      d->glfuncs->glEnableVertexAttribArray(location);
   }
}

/*!
    \overload

    Enables the vertex array called \a name in this shader program
    so that the value set by setAttributeArray() on \a name
    will be used by the shader program.

    \sa disableAttributeArray(), setAttributeArray(), setAttributeValue()
    \sa setUniformValue()
*/
void QGLShaderProgram::enableAttributeArray(const char *name)
{
   enableAttributeArray(attributeLocation(name));
}

/*!
    Disables the vertex array at \a location in this shader program
    that was enabled by a previous call to enableAttributeArray().

    \sa enableAttributeArray(), setAttributeArray(), setAttributeValue()
    \sa setUniformValue()
*/
void QGLShaderProgram::disableAttributeArray(int location)
{
   Q_D(QGLShaderProgram);
   Q_UNUSED(d);
   if (location != -1) {
      d->glfuncs->glDisableVertexAttribArray(location);
   }
}

/*!
    \overload

    Disables the vertex array called \a name in this shader program
    that was enabled by a previous call to enableAttributeArray().

    \sa enableAttributeArray(), setAttributeArray(), setAttributeValue()
    \sa setUniformValue()
*/
void QGLShaderProgram::disableAttributeArray(const char *name)
{
   disableAttributeArray(attributeLocation(name));
}

/*!
    Returns the location of the uniform variable \a name within this shader
    program's parameter list.  Returns -1 if \a name is not a valid
    uniform variable for this shader program.

    \sa attributeLocation()
*/
int QGLShaderProgram::uniformLocation(const char *name) const
{
   Q_D(const QGLShaderProgram);
   Q_UNUSED(d);
   if (d->linked && d->programGuard && d->programGuard->id()) {
      return d->glfuncs->glGetUniformLocation(d->programGuard->id(), name);
   } else {
      qWarning() << "QGLShaderProgram::uniformLocation(" << name
         << "): shader program is not linked";
      return -1;
   }
}

/*!
    \overload

    Returns the location of the uniform variable \a name within this shader
    program's parameter list.  Returns -1 if \a name is not a valid
    uniform variable for this shader program.

    \sa attributeLocation()
*/
int QGLShaderProgram::uniformLocation(const QByteArray &name) const
{
   return uniformLocation(name.constData());
}

/*!
    \overload

    Returns the location of the uniform variable \a name within this shader
    program's parameter list.  Returns -1 if \a name is not a valid
    uniform variable for this shader program.

    \sa attributeLocation()
*/
int QGLShaderProgram::uniformLocation(const QString &name) const
{
   return uniformLocation(name.toLatin1().constData());
}

/*!
    Sets the uniform variable at \a location in the current context to \a value.

    \sa setAttributeValue()
*/
void QGLShaderProgram::setUniformValue(int location, GLfloat value)
{
   Q_D(QGLShaderProgram);
   Q_UNUSED(d);
   if (location != -1) {
      d->glfuncs->glUniform1fv(location, 1, &value);
   }
}

/*!
    \overload

    Sets the uniform variable called \a name in the current context
    to \a value.

    \sa setAttributeValue()
*/
void QGLShaderProgram::setUniformValue(const char *name, GLfloat value)
{
   setUniformValue(uniformLocation(name), value);
}

/*!
    Sets the uniform variable at \a location in the current context to \a value.

    \sa setAttributeValue()
*/
void QGLShaderProgram::setUniformValue(int location, GLint value)
{
   Q_D(QGLShaderProgram);
   Q_UNUSED(d);
   if (location != -1) {
      d->glfuncs->glUniform1i(location, value);
   }
}

/*!
    \overload

    Sets the uniform variable called \a name in the current context
    to \a value.

    \sa setAttributeValue()
*/
void QGLShaderProgram::setUniformValue(const char *name, GLint value)
{
   setUniformValue(uniformLocation(name), value);
}

/*!
    Sets the uniform variable at \a location in the current context to \a value.
    This function should be used when setting sampler values.

    \sa setAttributeValue()
*/
void QGLShaderProgram::setUniformValue(int location, GLuint value)
{
   Q_D(QGLShaderProgram);
   Q_UNUSED(d);
   if (location != -1) {
      d->glfuncs->glUniform1i(location, value);
   }
}

/*!
    \overload

    Sets the uniform variable called \a name in the current context
    to \a value.  This function should be used when setting sampler values.

    \sa setAttributeValue()
*/
void QGLShaderProgram::setUniformValue(const char *name, GLuint value)
{
   setUniformValue(uniformLocation(name), value);
}

/*!
    Sets the uniform variable at \a location in the current context to
    the 2D vector (\a x, \a y).

    \sa setAttributeValue()
*/
void QGLShaderProgram::setUniformValue(int location, GLfloat x, GLfloat y)
{
   Q_D(QGLShaderProgram);
   Q_UNUSED(d);
   if (location != -1) {
      GLfloat values[2] = {x, y};
      d->glfuncs->glUniform2fv(location, 1, values);
   }
}

/*!
    \overload

    Sets the uniform variable called \a name in the current context to
    the 2D vector (\a x, \a y).

    \sa setAttributeValue()
*/
void QGLShaderProgram::setUniformValue(const char *name, GLfloat x, GLfloat y)
{
   setUniformValue(uniformLocation(name), x, y);
}

/*!
    Sets the uniform variable at \a location in the current context to
    the 3D vector (\a x, \a y, \a z).

    \sa setAttributeValue()
*/
void QGLShaderProgram::setUniformValue
(int location, GLfloat x, GLfloat y, GLfloat z)
{
   Q_D(QGLShaderProgram);
   Q_UNUSED(d);
   if (location != -1) {
      GLfloat values[3] = {x, y, z};
      d->glfuncs->glUniform3fv(location, 1, values);
   }
}

/*!
    \overload

    Sets the uniform variable called \a name in the current context to
    the 3D vector (\a x, \a y, \a z).

    \sa setAttributeValue()
*/
void QGLShaderProgram::setUniformValue
(const char *name, GLfloat x, GLfloat y, GLfloat z)
{
   setUniformValue(uniformLocation(name), x, y, z);
}

/*!
    Sets the uniform variable at \a location in the current context to
    the 4D vector (\a x, \a y, \a z, \a w).

    \sa setAttributeValue()
*/
void QGLShaderProgram::setUniformValue
(int location, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
   Q_D(QGLShaderProgram);
   Q_UNUSED(d);
   if (location != -1) {
      GLfloat values[4] = {x, y, z, w};
      d->glfuncs->glUniform4fv(location, 1, values);
   }
}

/*!
    \overload

    Sets the uniform variable called \a name in the current context to
    the 4D vector (\a x, \a y, \a z, \a w).

    \sa setAttributeValue()
*/
void QGLShaderProgram::setUniformValue
(const char *name, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
   setUniformValue(uniformLocation(name), x, y, z, w);
}

/*!
    Sets the uniform variable at \a location in the current context to \a value.

    \sa setAttributeValue()
*/
void QGLShaderProgram::setUniformValue(int location, const QVector2D &value)
{
   Q_D(QGLShaderProgram);
   Q_UNUSED(d);
   if (location != -1) {
      d->glfuncs->glUniform2fv(location, 1, reinterpret_cast<const GLfloat *>(&value));
   }
}

/*!
    \overload

    Sets the uniform variable called \a name in the current context
    to \a value.

    \sa setAttributeValue()
*/
void QGLShaderProgram::setUniformValue(const char *name, const QVector2D &value)
{
   setUniformValue(uniformLocation(name), value);
}

/*!
    Sets the uniform variable at \a location in the current context to \a value.

    \sa setAttributeValue()
*/
void QGLShaderProgram::setUniformValue(int location, const QVector3D &value)
{
   Q_D(QGLShaderProgram);
   Q_UNUSED(d);
   if (location != -1) {
      d->glfuncs->glUniform3fv(location, 1, reinterpret_cast<const GLfloat *>(&value));
   }
}

/*!
    \overload

    Sets the uniform variable called \a name in the current context
    to \a value.

    \sa setAttributeValue()
*/
void QGLShaderProgram::setUniformValue(const char *name, const QVector3D &value)
{
   setUniformValue(uniformLocation(name), value);
}

/*!
    Sets the uniform variable at \a location in the current context to \a value.

    \sa setAttributeValue()
*/
void QGLShaderProgram::setUniformValue(int location, const QVector4D &value)
{
   Q_D(QGLShaderProgram);
   Q_UNUSED(d);
   if (location != -1) {
      d->glfuncs->glUniform4fv(location, 1, reinterpret_cast<const GLfloat *>(&value));
   }
}

/*!
    \overload

    Sets the uniform variable called \a name in the current context
    to \a value.

    \sa setAttributeValue()
*/
void QGLShaderProgram::setUniformValue(const char *name, const QVector4D &value)
{
   setUniformValue(uniformLocation(name), value);
}

/*!
    Sets the uniform variable at \a location in the current context to
    the red, green, blue, and alpha components of \a color.

    \sa setAttributeValue()
*/
void QGLShaderProgram::setUniformValue(int location, const QColor &color)
{
   Q_D(QGLShaderProgram);
   Q_UNUSED(d);
   if (location != -1) {
      GLfloat values[4] = {GLfloat(color.redF()), GLfloat(color.greenF()),
            GLfloat(color.blueF()), GLfloat(color.alphaF())
         };
      d->glfuncs->glUniform4fv(location, 1, values);
   }
}

/*!
    \overload

    Sets the uniform variable called \a name in the current context to
    the red, green, blue, and alpha components of \a color.

    \sa setAttributeValue()
*/
void QGLShaderProgram::setUniformValue(const char *name, const QColor &color)
{
   setUniformValue(uniformLocation(name), color);
}

/*!
    Sets the uniform variable at \a location in the current context to
    the x and y coordinates of \a point.

    \sa setAttributeValue()
*/
void QGLShaderProgram::setUniformValue(int location, const QPoint &point)
{
   Q_D(QGLShaderProgram);
   Q_UNUSED(d);
   if (location != -1) {
      GLfloat values[4] = {GLfloat(point.x()), GLfloat(point.y())};
      d->glfuncs->glUniform2fv(location, 1, values);
   }
}

/*!
    \overload

    Sets the uniform variable associated with \a name in the current
    context to the x and y coordinates of \a point.

    \sa setAttributeValue()
*/
void QGLShaderProgram::setUniformValue(const char *name, const QPoint &point)
{
   setUniformValue(uniformLocation(name), point);
}

/*!
    Sets the uniform variable at \a location in the current context to
    the x and y coordinates of \a point.

    \sa setAttributeValue()
*/
void QGLShaderProgram::setUniformValue(int location, const QPointF &point)
{
   Q_D(QGLShaderProgram);
   Q_UNUSED(d);
   if (location != -1) {
      GLfloat values[4] = {GLfloat(point.x()), GLfloat(point.y())};
      d->glfuncs->glUniform2fv(location, 1, values);
   }
}

/*!
    \overload

    Sets the uniform variable associated with \a name in the current
    context to the x and y coordinates of \a point.

    \sa setAttributeValue()
*/
void QGLShaderProgram::setUniformValue(const char *name, const QPointF &point)
{
   setUniformValue(uniformLocation(name), point);
}

/*!
    Sets the uniform variable at \a location in the current context to
    the width and height of the given \a size.

    \sa setAttributeValue()
*/
void QGLShaderProgram::setUniformValue(int location, const QSize &size)
{
   Q_D(QGLShaderProgram);
   Q_UNUSED(d);
   if (location != -1) {
      GLfloat values[4] = {GLfloat(size.width()), GLfloat(size.height())};
      d->glfuncs->glUniform2fv(location, 1, values);
   }
}

/*!
    \overload

    Sets the uniform variable associated with \a name in the current
    context to the width and height of the given \a size.

    \sa setAttributeValue()
*/
void QGLShaderProgram::setUniformValue(const char *name, const QSize &size)
{
   setUniformValue(uniformLocation(name), size);
}

/*!
    Sets the uniform variable at \a location in the current context to
    the width and height of the given \a size.

    \sa setAttributeValue()
*/
void QGLShaderProgram::setUniformValue(int location, const QSizeF &size)
{
   Q_D(QGLShaderProgram);

   if (location != -1) {
      GLfloat values[4] = {GLfloat(size.width()), GLfloat(size.height())};
      d->glfuncs->glUniform2fv(location, 1, values);
   }
}


void QGLShaderProgram::setUniformValue(const char *name, const QSizeF &size)
{
   setUniformValue(uniformLocation(name), size);
}


/*!
    Sets the uniform variable at \a location in the current context
    to a 2x2 matrix \a value.

    \sa setAttributeValue()
*/
void QGLShaderProgram::setUniformValue(int location, const QMatrix2x2 &value)
{
   Q_D(QGLShaderProgram);

   constexpr int max = 4;
   GLfloat tmp[max];
   for (int i = 0; i < max; ++i) {
      tmp[i] = value.constData()[i];
   }

   d->glfuncs->glUniformMatrix2fv(location, 1, GL_FALSE, tmp);
}

/*!
    \overload

    Sets the uniform variable called \a name in the current context
    to a 2x2 matrix \a value.

    \sa setAttributeValue()
*/
void QGLShaderProgram::setUniformValue(const char *name, const QMatrix2x2 &value)
{
   setUniformValue(uniformLocation(name), value);
}

/*!
    Sets the uniform variable at \a location in the current context
    to a 2x3 matrix \a value.

    \sa setAttributeValue()
*/
void QGLShaderProgram::setUniformValue(int location, const QMatrix2x3 &value)
{
   Q_D(QGLShaderProgram);

   constexpr int max = 6;
   GLfloat tmp[max];
   for (int i = 0; i < max; ++i) {
      tmp[i] = value.constData()[i];
   }

   d->glfuncs->glUniform3fv(location, 2, tmp);
}

/*!
    \overload

    Sets the uniform variable called \a name in the current context
    to a 2x3 matrix \a value.

    \sa setAttributeValue()
*/
void QGLShaderProgram::setUniformValue(const char *name, const QMatrix2x3 &value)
{
   setUniformValue(uniformLocation(name), value);
}

/*!
    Sets the uniform variable at \a location in the current context
    to a 2x4 matrix \a value.

    \sa setAttributeValue()
*/
void QGLShaderProgram::setUniformValue(int location, const QMatrix2x4 &value)
{
   Q_D(QGLShaderProgram);

   constexpr int max = 8;
   GLfloat tmp[max];
   for (int i = 0; i < max; ++i) {
      tmp[i] = value.constData()[i];
   }

   d->glfuncs->glUniform4fv(location, 2, tmp);
}

/*!
    \overload

    Sets the uniform variable called \a name in the current context
    to a 2x4 matrix \a value.

    \sa setAttributeValue()
*/
void QGLShaderProgram::setUniformValue(const char *name, const QMatrix2x4 &value)
{
   setUniformValue(uniformLocation(name), value);
}

/*!
    Sets the uniform variable at \a location in the current context
    to a 3x2 matrix \a value.

    \sa setAttributeValue()
*/
void QGLShaderProgram::setUniformValue(int location, const QMatrix3x2 &value)
{
   Q_D(QGLShaderProgram);

   constexpr int max = 6;
   GLfloat tmp[max];
   for (int i = 0; i < max; ++i) {
      tmp[i] = value.constData()[i];
   }

   d->glfuncs->glUniform2fv(location, 3, tmp);
}

/*!
    \overload

    Sets the uniform variable called \a name in the current context
    to a 3x2 matrix \a value.

    \sa setAttributeValue()
*/
void QGLShaderProgram::setUniformValue(const char *name, const QMatrix3x2 &value)
{
   setUniformValue(uniformLocation(name), value);
}

/*!
    Sets the uniform variable at \a location in the current context
    to a 3x3 matrix \a value.

    \sa setAttributeValue()
*/
void QGLShaderProgram::setUniformValue(int location, const QMatrix3x3 &value)
{
   Q_D(QGLShaderProgram);

   constexpr int max = 9;
   GLfloat tmp[max];
   for (int i = 0; i < max; ++i) {
      tmp[i] = value.constData()[i];
   }

   d->glfuncs->glUniformMatrix3fv(location, 1, GL_FALSE, tmp);
}

/*!
    \overload

    Sets the uniform variable called \a name in the current context
    to a 3x3 matrix \a value.

    \sa setAttributeValue()
*/
void QGLShaderProgram::setUniformValue(const char *name, const QMatrix3x3 &value)
{
   setUniformValue(uniformLocation(name), value);
}

/*!
    Sets the uniform variable at \a location in the current context
    to a 3x4 matrix \a value.

    \sa setAttributeValue()
*/
void QGLShaderProgram::setUniformValue(int location, const QMatrix3x4 &value)
{
   Q_D(QGLShaderProgram);

   constexpr int max = 12;
   GLfloat tmp[max];
   for (int i = 0; i < max; ++i) {
      tmp[i] = value.constData()[i];
   }

   d->glfuncs->glUniform4fv(location, 3, tmp);
}

/*!
    \overload

    Sets the uniform variable called \a name in the current context
    to a 3x4 matrix \a value.

    \sa setAttributeValue()
*/
void QGLShaderProgram::setUniformValue(const char *name, const QMatrix3x4 &value)
{
   setUniformValue(uniformLocation(name), value);
}

/*!
    Sets the uniform variable at \a location in the current context
    to a 4x2 matrix \a value.

    \sa setAttributeValue()
*/
void QGLShaderProgram::setUniformValue(int location, const QMatrix4x2 &value)
{
   Q_D(QGLShaderProgram);

   constexpr int max = 8;
   GLfloat tmp[max];
   for (int i = 0; i < max; ++i) {
      tmp[i] = value.constData()[i];
   }

   d->glfuncs->glUniform2fv(location, 4, tmp);
}

/*!
    \overload

    Sets the uniform variable called \a name in the current context
    to a 4x2 matrix \a value.

    \sa setAttributeValue()
*/
void QGLShaderProgram::setUniformValue(const char *name, const QMatrix4x2 &value)
{
   setUniformValue(uniformLocation(name), value);
}

/*!
    Sets the uniform variable at \a location in the current context
    to a 4x3 matrix \a value.

    \sa setAttributeValue()
*/
void QGLShaderProgram::setUniformValue(int location, const QMatrix4x3 &value)
{
   Q_D(QGLShaderProgram);

   constexpr int max = 12;
   GLfloat tmp[max];
   for (int i = 0; i < max; ++i) {
      tmp[i] = value.constData()[i];
   }

   d->glfuncs->glUniform3fv(location, 4, tmp);
}

/*!
    \overload

    Sets the uniform variable called \a name in the current context
    to a 4x3 matrix \a value.

    \sa setAttributeValue()
*/
void QGLShaderProgram::setUniformValue(const char *name, const QMatrix4x3 &value)
{
   setUniformValue(uniformLocation(name), value);
}

/*!
    Sets the uniform variable at \a location in the current context
    to a 4x4 matrix \a value.

    \sa setAttributeValue()
*/
void QGLShaderProgram::setUniformValue(int location, const QMatrix4x4 &value)
{
   Q_D(QGLShaderProgram);

   constexpr int max = 16;
   GLfloat tmp[max];
   for (int i = 0; i < max; ++i) {
      tmp[i] = value.constData()[i];
   }

   d->glfuncs->glUniformMatrix4fv(location, 1, GL_FALSE, tmp);
}

/*!
    \overload

    Sets the uniform variable called \a name in the current context
    to a 4x4 matrix \a value.

    \sa setAttributeValue()
*/
void QGLShaderProgram::setUniformValue(const char *name, const QMatrix4x4 &value)
{
   setUniformValue(uniformLocation(name), value);
}

/*!
    \overload

    Sets the uniform variable at \a location in the current context
    to a 2x2 matrix \a value.  The matrix elements must be specified
    in column-major order.

    \sa setAttributeValue()
    \since 4.7
*/
void QGLShaderProgram::setUniformValue(int location, const GLfloat value[2][2])
{
   Q_D(QGLShaderProgram);
   if (location != -1) {
      d->glfuncs->glUniformMatrix2fv(location, 1, GL_FALSE, value[0]);
   }
}

/*!
    \overload

    Sets the uniform variable at \a location in the current context
    to a 3x3 matrix \a value.  The matrix elements must be specified
    in column-major order.

    \sa setAttributeValue()
    \since 4.7
*/
void QGLShaderProgram::setUniformValue(int location, const GLfloat value[3][3])
{
   Q_D(QGLShaderProgram);
   if (location != -1) {
      d->glfuncs->glUniformMatrix3fv(location, 1, GL_FALSE, value[0]);
   }
}

/*!
    \overload

    Sets the uniform variable at \a location in the current context
    to a 4x4 matrix \a value.  The matrix elements must be specified
    in column-major order.

    \sa setAttributeValue()
*/
void QGLShaderProgram::setUniformValue(int location, const GLfloat value[4][4])
{
   Q_D(QGLShaderProgram);
   if (location != -1) {
      d->glfuncs->glUniformMatrix4fv(location, 1, GL_FALSE, value[0]);
   }
}


/*!
    \overload

    Sets the uniform variable called \a name in the current context
    to a 2x2 matrix \a value.  The matrix elements must be specified
    in column-major order.

    \sa setAttributeValue()
    \since 4.7
*/
void QGLShaderProgram::setUniformValue(const char *name, const GLfloat value[2][2])
{
   setUniformValue(uniformLocation(name), value);
}

/*!
    \overload

    Sets the uniform variable called \a name in the current context
    to a 3x3 matrix \a value.  The matrix elements must be specified
    in column-major order.

    \sa setAttributeValue()
    \since 4.7
*/
void QGLShaderProgram::setUniformValue(const char *name, const GLfloat value[3][3])
{
   setUniformValue(uniformLocation(name), value);
}

/*!
    \overload

    Sets the uniform variable called \a name in the current context
    to a 4x4 matrix \a value.  The matrix elements must be specified
    in column-major order.

    \sa setAttributeValue()
*/
void QGLShaderProgram::setUniformValue(const char *name, const GLfloat value[4][4])
{
   setUniformValue(uniformLocation(name), value);
}

/*!
    Sets the uniform variable at \a location in the current context to a
    3x3 transformation matrix \a value that is specified as a QTransform value.

    To set a QTransform value as a 4x4 matrix in a shader, use
    \c{setUniformValue(location, QMatrix4x4(value))}.
*/
void QGLShaderProgram::setUniformValue(int location, const QTransform &value)
{
   Q_D(QGLShaderProgram);

   if (location != -1) {
      GLfloat mat[3][3] = {
         {GLfloat(value.m11()), GLfloat(value.m12()), GLfloat(value.m13())},
         {GLfloat(value.m21()), GLfloat(value.m22()), GLfloat(value.m23())},
         {GLfloat(value.m31()), GLfloat(value.m32()), GLfloat(value.m33())}
      };
      d->glfuncs->glUniformMatrix3fv(location, 1, GL_FALSE, mat[0]);
   }
}

/*!
    \overload

    Sets the uniform variable called \a name in the current context to a
    3x3 transformation matrix \a value that is specified as a QTransform value.

    To set a QTransform value as a 4x4 matrix in a shader, use
    \c{setUniformValue(name, QMatrix4x4(value))}.
*/
void QGLShaderProgram::setUniformValue
(const char *name, const QTransform &value)
{
   setUniformValue(uniformLocation(name), value);
}

/*!
    Sets the uniform variable array at \a location in the current
    context to the \a count elements of \a values.

    \sa setAttributeValue()
*/
void QGLShaderProgram::setUniformValueArray(int location, const GLint *values, int count)
{
   Q_D(QGLShaderProgram);

   if (location != -1) {
      d->glfuncs->glUniform1iv(location, count, values);
   }
}

/*!
    \overload

    Sets the uniform variable array called \a name in the current
    context to the \a count elements of \a values.

    \sa setAttributeValue()
*/
void QGLShaderProgram::setUniformValueArray
(const char *name, const GLint *values, int count)
{
   setUniformValueArray(uniformLocation(name), values, count);
}

/*!
    Sets the uniform variable array at \a location in the current
    context to the \a count elements of \a values.  This overload
    should be used when setting an array of sampler values.

    \sa setAttributeValue()
*/
void QGLShaderProgram::setUniformValueArray(int location, const GLuint *values, int count)
{
   Q_D(QGLShaderProgram);

   if (location != -1) {
      d->glfuncs->glUniform1iv(location, count, reinterpret_cast<const GLint *>(values));
   }
}

/*!
    \overload

    Sets the uniform variable array called \a name in the current
    context to the \a count elements of \a values.  This overload
    should be used when setting an array of sampler values.

    \sa setAttributeValue()
*/
void QGLShaderProgram::setUniformValueArray
(const char *name, const GLuint *values, int count)
{
   setUniformValueArray(uniformLocation(name), values, count);
}

/*!
    Sets the uniform variable array at \a location in the current
    context to the \a count elements of \a values.  Each element
    has \a tupleSize components.  The \a tupleSize must be 1, 2, 3, or 4.

    \sa setAttributeValue()
*/
void QGLShaderProgram::setUniformValueArray(int location, const GLfloat *values, int count, int tupleSize)
{
   Q_D(QGLShaderProgram);

   if (location != -1) {
      if (tupleSize == 1) {
         d->glfuncs->glUniform1fv(location, count, values);
      } else if (tupleSize == 2) {
         d->glfuncs->glUniform2fv(location, count, values);
      } else if (tupleSize == 3) {
         d->glfuncs->glUniform3fv(location, count, values);
      } else if (tupleSize == 4) {
         d->glfuncs->glUniform4fv(location, count, values);
      } else {
         qWarning() << "QGLShaderProgram::setUniformValue: size" << tupleSize << "not supported";
      }
   }
}

/*!
    \overload

    Sets the uniform variable array called \a name in the current
    context to the \a count elements of \a values.  Each element
    has \a tupleSize components.  The \a tupleSize must be 1, 2, 3, or 4.

    \sa setAttributeValue()
*/
void QGLShaderProgram::setUniformValueArray
(const char *name, const GLfloat *values, int count, int tupleSize)
{
   setUniformValueArray(uniformLocation(name), values, count, tupleSize);
}

/*!
    Sets the uniform variable array at \a location in the current
    context to the \a count 2D vector elements of \a values.

    \sa setAttributeValue()
*/
void QGLShaderProgram::setUniformValueArray(int location, const QVector2D *values, int count)
{
   Q_D(QGLShaderProgram);

   if (location != -1) {
      d->glfuncs->glUniform2fv(location, count, reinterpret_cast<const GLfloat *>(values));
   }
}

/*!
    \overload

    Sets the uniform variable array called \a name in the current
    context to the \a count 2D vector elements of \a values.

    \sa setAttributeValue()
*/
void QGLShaderProgram::setUniformValueArray(const char *name, const QVector2D *values, int count)
{
   setUniformValueArray(uniformLocation(name), values, count);
}

/*!
    Sets the uniform variable array at \a location in the current
    context to the \a count 3D vector elements of \a values.

    \sa setAttributeValue()
*/
void QGLShaderProgram::setUniformValueArray(int location, const QVector3D *values, int count)
{
   Q_D(QGLShaderProgram);

   if (location != -1) {
      d->glfuncs->glUniform3fv(location, count, reinterpret_cast<const GLfloat *>(values));
   }
}

/*!
    \overload

    Sets the uniform variable array called \a name in the current
    context to the \a count 3D vector elements of \a values.

    \sa setAttributeValue()
*/
void QGLShaderProgram::setUniformValueArray(const char *name, const QVector3D *values, int count)
{
   setUniformValueArray(uniformLocation(name), values, count);
}

/*!
    Sets the uniform variable array at \a location in the current
    context to the \a count 4D vector elements of \a values.

    \sa setAttributeValue()
*/
void QGLShaderProgram::setUniformValueArray(int location, const QVector4D *values, int count)
{
   Q_D(QGLShaderProgram);

   if (location != -1) {
      d->glfuncs->glUniform4fv(location, count, reinterpret_cast<const GLfloat *>(values));
   }
}

/*!
    \overload

    Sets the uniform variable array called \a name in the current
    context to the \a count 4D vector elements of \a values.

    \sa setAttributeValue()
*/
void QGLShaderProgram::setUniformValueArray(const char *name, const QVector4D *values, int count)
{
   setUniformValueArray(uniformLocation(name), values, count);
}

// We have to repack matrix arrays from qreal to GLfloat.
#define setUniformMatrixArray(func,location,values,count,type,cols,rows) \
    if (location == -1 || count <= 0) \
        return; \
    if (sizeof(type) == sizeof(GLfloat) * cols * rows) { \
        func(location, count, GL_FALSE, \
             reinterpret_cast<const GLfloat *>(values[0].constData())); \
    } else { \
        QVarLengthArray<GLfloat> temp(cols * rows * count); \
        for (int index = 0; index < count; ++index) { \
            for (int index2 = 0; index2 < (cols * rows); ++index2) { \
                temp.data()[cols * rows * index + index2] = \
                    values[index].constData()[index2]; \
            } \
        } \
        func(location, count, GL_FALSE, temp.constData()); \
    }

#define setUniformGenericMatrixArray(colfunc,location,values,count,type,cols,rows) \
    if (location == -1 || count <= 0) \
        return; \
    if (sizeof(type) == sizeof(GLfloat) * cols * rows) { \
        const GLfloat *data = reinterpret_cast<const GLfloat *> \
            (values[0].constData());  \
        colfunc(location, count * cols, data); \
    } else { \
        QVarLengthArray<GLfloat> temp(cols * rows * count); \
        for (int index = 0; index < count; ++index) { \
            for (int index2 = 0; index2 < (cols * rows); ++index2) { \
                temp.data()[cols * rows * index + index2] = \
                    values[index].constData()[index2]; \
            } \
        } \
        colfunc(location, count * cols, temp.constData()); \
    }


/*!
    Sets the uniform variable array at \a location in the current
    context to the \a count 2x2 matrix elements of \a values.

    \sa setAttributeValue()
*/
void QGLShaderProgram::setUniformValueArray(int location, const QMatrix2x2 *values, int count)
{
   Q_D(QGLShaderProgram);

   setUniformMatrixArray
   (d->glfuncs->glUniformMatrix2fv, location, values, count, QMatrix2x2, 2, 2);
}

/*!
    \overload

    Sets the uniform variable array called \a name in the current
    context to the \a count 2x2 matrix elements of \a values.

    \sa setAttributeValue()
*/
void QGLShaderProgram::setUniformValueArray(const char *name, const QMatrix2x2 *values, int count)
{
   setUniformValueArray(uniformLocation(name), values, count);
}

/*!
    Sets the uniform variable array at \a location in the current
    context to the \a count 2x3 matrix elements of \a values.

    \sa setAttributeValue()
*/
void QGLShaderProgram::setUniformValueArray(int location, const QMatrix2x3 *values, int count)
{
   Q_D(QGLShaderProgram);
   Q_UNUSED(d);
   setUniformGenericMatrixArray
   (d->glfuncs->glUniform3fv, location, values, count,
      QMatrix2x3, 2, 3);
}

/*!
    \overload

    Sets the uniform variable array called \a name in the current
    context to the \a count 2x3 matrix elements of \a values.

    \sa setAttributeValue()
*/
void QGLShaderProgram::setUniformValueArray(const char *name, const QMatrix2x3 *values, int count)
{
   setUniformValueArray(uniformLocation(name), values, count);
}

/*!
    Sets the uniform variable array at \a location in the current
    context to the \a count 2x4 matrix elements of \a values.

    \sa setAttributeValue()
*/
void QGLShaderProgram::setUniformValueArray(int location, const QMatrix2x4 *values, int count)
{
   Q_D(QGLShaderProgram);
   Q_UNUSED(d);
   setUniformGenericMatrixArray
   (d->glfuncs->glUniform4fv, location, values, count,
      QMatrix2x4, 2, 4);
}

/*!
    \overload

    Sets the uniform variable array called \a name in the current
    context to the \a count 2x4 matrix elements of \a values.

    \sa setAttributeValue()
*/
void QGLShaderProgram::setUniformValueArray(const char *name, const QMatrix2x4 *values, int count)
{
   setUniformValueArray(uniformLocation(name), values, count);
}

/*!
    Sets the uniform variable array at \a location in the current
    context to the \a count 3x2 matrix elements of \a values.

    \sa setAttributeValue()
*/
void QGLShaderProgram::setUniformValueArray(int location, const QMatrix3x2 *values, int count)
{
   Q_D(QGLShaderProgram);
   Q_UNUSED(d);
   setUniformGenericMatrixArray
   (d->glfuncs->glUniform2fv, location, values, count,
      QMatrix3x2, 3, 2);
}

/*!
    \overload

    Sets the uniform variable array called \a name in the current
    context to the \a count 3x2 matrix elements of \a values.

    \sa setAttributeValue()
*/
void QGLShaderProgram::setUniformValueArray(const char *name, const QMatrix3x2 *values, int count)
{
   setUniformValueArray(uniformLocation(name), values, count);
}

/*!
    Sets the uniform variable array at \a location in the current
    context to the \a count 3x3 matrix elements of \a values.

    \sa setAttributeValue()
*/
void QGLShaderProgram::setUniformValueArray(int location, const QMatrix3x3 *values, int count)
{
   Q_D(QGLShaderProgram);
   Q_UNUSED(d);
   setUniformMatrixArray
   (d->glfuncs->glUniformMatrix3fv, location, values, count, QMatrix3x3, 3, 3);
}

/*!
    \overload

    Sets the uniform variable array called \a name in the current
    context to the \a count 3x3 matrix elements of \a values.

    \sa setAttributeValue()
*/
void QGLShaderProgram::setUniformValueArray(const char *name, const QMatrix3x3 *values, int count)
{
   setUniformValueArray(uniformLocation(name), values, count);
}

/*!
    Sets the uniform variable array at \a location in the current
    context to the \a count 3x4 matrix elements of \a values.

    \sa setAttributeValue()
*/
void QGLShaderProgram::setUniformValueArray(int location, const QMatrix3x4 *values, int count)
{
   Q_D(QGLShaderProgram);
   Q_UNUSED(d);
   setUniformGenericMatrixArray
   (d->glfuncs->glUniform4fv, location, values, count,
      QMatrix3x4, 3, 4);
}

/*!
    \overload

    Sets the uniform variable array called \a name in the current
    context to the \a count 3x4 matrix elements of \a values.

    \sa setAttributeValue()
*/
void QGLShaderProgram::setUniformValueArray(const char *name, const QMatrix3x4 *values, int count)
{
   setUniformValueArray(uniformLocation(name), values, count);
}

/*!
    Sets the uniform variable array at \a location in the current
    context to the \a count 4x2 matrix elements of \a values.

    \sa setAttributeValue()
*/
void QGLShaderProgram::setUniformValueArray(int location, const QMatrix4x2 *values, int count)
{
   Q_D(QGLShaderProgram);
   Q_UNUSED(d);
   setUniformGenericMatrixArray
   (d->glfuncs->glUniform2fv, location, values, count,
      QMatrix4x2, 4, 2);
}

/*!
    \overload

    Sets the uniform variable array called \a name in the current
    context to the \a count 4x2 matrix elements of \a values.

    \sa setAttributeValue()
*/
void QGLShaderProgram::setUniformValueArray(const char *name, const QMatrix4x2 *values, int count)
{
   setUniformValueArray(uniformLocation(name), values, count);
}

/*!
    Sets the uniform variable array at \a location in the current
    context to the \a count 4x3 matrix elements of \a values.

    \sa setAttributeValue()
*/
void QGLShaderProgram::setUniformValueArray(int location, const QMatrix4x3 *values, int count)
{
   Q_D(QGLShaderProgram);
   Q_UNUSED(d);
   setUniformGenericMatrixArray
   (d->glfuncs->glUniform3fv, location, values, count,
      QMatrix4x3, 4, 3);
}

/*!
    \overload

    Sets the uniform variable array called \a name in the current
    context to the \a count 4x3 matrix elements of \a values.

    \sa setAttributeValue()
*/
void QGLShaderProgram::setUniformValueArray(const char *name, const QMatrix4x3 *values, int count)
{
   setUniformValueArray(uniformLocation(name), values, count);
}

/*!
    Sets the uniform variable array at \a location in the current
    context to the \a count 4x4 matrix elements of \a values.

    \sa setAttributeValue()
*/
void QGLShaderProgram::setUniformValueArray(int location, const QMatrix4x4 *values, int count)
{
   Q_D(QGLShaderProgram);

   setUniformMatrixArray(d->glfuncs->glUniformMatrix4fv, location, values, count, QMatrix4x4, 4, 4);
}

/*!
    \overload

    Sets the uniform variable array called \a name in the current
    context to the \a count 4x4 matrix elements of \a values.

    \sa setAttributeValue()
*/
void QGLShaderProgram::setUniformValueArray(const char *name, const QMatrix4x4 *values, int count)
{
   setUniformValueArray(uniformLocation(name), values, count);
}

#undef ctx

/*!
    Returns the hardware limit for how many vertices a geometry shader
    can output.

    \since 4.7

    \sa setGeometryOutputVertexCount()
*/
int QGLShaderProgram::maxGeometryOutputVertices() const
{
   GLint n = 0;
#if !defined(QT_OPENGL_ES_2)
   Q_D(const QGLShaderProgram);
   if (!QOpenGLContext::currentContext()->isOpenGLES()) {
      d->glfuncs->glGetIntegerv(GL_MAX_GEOMETRY_OUTPUT_VERTICES_EXT, &n);
   }
#endif
   return n;
}

/*!
    Sets the maximum number of vertices the current geometry shader
    program will produce, if active, to \a count.

    \since 4.7

    This parameter takes effect the next time the program is linked.
*/
void QGLShaderProgram::setGeometryOutputVertexCount(int count)
{
#ifndef QT_NO_DEBUG
   int max = maxGeometryOutputVertices();
   if (count > max) {
      qWarning("QGLShaderProgram::setGeometryOutputVertexCount: count: %d higher than maximum: %d",
         count, max);
   }
#endif
   d_func()->geometryVertexCount = count;
}


/*!
    Returns the maximum number of vertices the current geometry shader
    program will produce, if active.

    \since 4.7

    This parameter takes effect the ntext time the program is linked.
*/
int QGLShaderProgram::geometryOutputVertexCount() const
{
   return d_func()->geometryVertexCount;
}


/*!
    Sets the input type from \a inputType.

    This parameter takes effect the next time the program is linked.
*/
void QGLShaderProgram::setGeometryInputType(GLenum inputType)
{
   d_func()->geometryInputType = inputType;
}


/*!
    Returns the geometry shader input type, if active.

    This parameter takes effect the next time the program is linked.

    \since 4.7
 */

GLenum QGLShaderProgram::geometryInputType() const
{
   return d_func()->geometryInputType;
}


/*!
    Sets the output type from the geometry shader, if active, to
    \a outputType.

    This parameter takes effect the next time the program is linked.

    \since 4.7
*/
void QGLShaderProgram::setGeometryOutputType(GLenum outputType)
{
   d_func()->geometryOutputType = outputType;
}


/*!
    Returns the geometry shader output type, if active.

    This parameter takes effect the next time the program is linked.

    \since 4.7
 */
GLenum QGLShaderProgram::geometryOutputType() const
{
   return d_func()->geometryOutputType;
}


/*!
    Returns true if shader programs written in the OpenGL Shading
    Language (GLSL) are supported on this system; false otherwise.

    The \a context is used to resolve the GLSL extensions.
    If \a context is null, then QGLContext::currentContext() is used.
*/
bool QGLShaderProgram::hasOpenGLShaderPrograms(const QGLContext *context)
{
#if !defined(QT_OPENGL_ES_2)
   if (!context) {
      context = QGLContext::currentContext();
   }

   if (!context) {
      return false;
   }

   QOpenGLFunctions functions(context->contextHandle());
   return functions.hasOpenGLFeature(QOpenGLFunctions::Shaders);
#else
   Q_UNUSED(context);
   return true;
#endif
}

/*!
    \internal
*/
void QGLShaderProgram::shaderDestroyed()
{
   Q_D(QGLShaderProgram);
   QGLShader *shader = qobject_cast<QGLShader *>(sender());
   if (shader && !d->removingShaders) {
      removeShader(shader);
   }
}

#undef ctx
#undef context


bool QGLShader::hasOpenGLShaders(ShaderType type, const QGLContext *context)
{
   if (! context) {
      context = QGLContext::currentContext();
   }

   if (! context) {
      return false;
   }

   if ((type & ~(Geometry | Vertex | Fragment)) || type == 0) {
      return false;
   }

   QOpenGLFunctions functions(context->contextHandle());
   bool resolved = functions.hasOpenGLFeature(QOpenGLFunctions::Shaders);

   if (!resolved) {
      return false;
   }
   if ((type & Geometry) && !QByteArray((const char *) functions.glGetString(GL_EXTENSIONS)).contains("GL_EXT_geometry_shader4")) {
      return false;
   }

   return true;
}


