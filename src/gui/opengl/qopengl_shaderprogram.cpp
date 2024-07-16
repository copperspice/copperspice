/***********************************************************************
*
* Copyright (c) 2012-2024 Barbara Geller
* Copyright (c) 2012-2024 Ansel Sermersheim
*
* Copyright (c) 2013 Klarälvdalens Datakonsult AB, a KDAB Group company
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

#include <qopengl_shaderprogram.h>

#include <qdebug.h>
#include <qcolor.h>
#include <qfile.h>
#include <qopenglfunctions.h>
#include <qvarlengtharray.h>
#include <qvector.h>
#include <qregularexpression.h>
#include <qtransform.h>
#include <qsurfaceformat.h>

#if ! defined(QT_OPENGL_ES_2)
#include <qopenglfunctions_4_0_core.h>
#endif

#include <qopenglcontext_p.h>

#include <algorithm>

class QOpenGLShaderPrivate
{
    Q_DECLARE_PUBLIC(QOpenGLShader)

public:
    QOpenGLShaderPrivate(QOpenGLContext *ctx, QOpenGLShader::ShaderType type)
        : shaderGuard(nullptr), shaderType(type), compiled(false), glfuncs(new QOpenGLFunctions(ctx))
#ifndef QT_OPENGL_ES_2
        , supportsGeometryShaders(false), supportsTessellationShaders(false)
#endif

    {
#ifndef QT_OPENGL_ES_2
        if (! ctx->isOpenGLES()) {
            QSurfaceFormat f = ctx->format();

            // Geometry shaders require OpenGL >= 3.2
            if (shaderType & QOpenGLShader::Geometry) {
                supportsGeometryShaders = (f.version() >= qMakePair<int, int>(3, 2));
            } else if (shaderType & (QOpenGLShader::TessellationControl | QOpenGLShader::TessellationEvaluation))  {
                supportsTessellationShaders = (f.version() >= qMakePair<int, int>(4, 0));
            }
        }
#endif
    }

    ~QOpenGLShaderPrivate();

    QOpenGLSharedResourceGuard *shaderGuard;
    QOpenGLShader::ShaderType shaderType;
    bool compiled;
    QString log;

    QOpenGLFunctions *glfuncs;

#ifndef QT_OPENGL_ES_2
    // Support for geometry shaders
    bool supportsGeometryShaders;

    // Support for tessellation shaders
    bool supportsTessellationShaders;
#endif

    bool create();
    bool compile(QOpenGLShader *q);
    void deleteShader();

 protected:
    QOpenGLShader *q_ptr;

};

namespace {
    void freeShaderFunc(QOpenGLFunctions *funcs, GLuint id) {
        funcs->glDeleteShader(id);
    }
}

QOpenGLShaderPrivate::~QOpenGLShaderPrivate()
{
    delete glfuncs;

    if (shaderGuard) {
        shaderGuard->free();
    }
}

bool QOpenGLShaderPrivate::create()
{
    QOpenGLContext *context = const_cast<QOpenGLContext *>(QOpenGLContext::currentContext());

    if (! context) {
        return false;
    }

    GLuint shader;

    if (shaderType == QOpenGLShader::Vertex) {
        shader = glfuncs->glCreateShader(GL_VERTEX_SHADER);

#if defined(QT_OPENGL_3_2)
    } else if (shaderType == QOpenGLShader::Geometry && supportsGeometryShaders) {
        shader = glfuncs->glCreateShader(GL_GEOMETRY_SHADER);
#endif

#if defined(QT_OPENGL_4)
    } else if (shaderType == QOpenGLShader::TessellationControl && supportsTessellationShaders) {
        shader = glfuncs->glCreateShader(GL_TESS_CONTROL_SHADER);

    } else if (shaderType == QOpenGLShader::TessellationEvaluation && supportsTessellationShaders) {
        shader = glfuncs->glCreateShader(GL_TESS_EVALUATION_SHADER);
#endif

#if defined(QT_OPENGL_4_3)
    } else if (shaderType == QOpenGLShader::Compute) {
        shader = glfuncs->glCreateShader(GL_COMPUTE_SHADER);
#endif

    } else {
        shader = glfuncs->glCreateShader(GL_FRAGMENT_SHADER);
    }

    if (! shader) {
        qWarning("QOpenGLShader::create() Unable to create shader");
        return false;
    }

    shaderGuard = new QOpenGLSharedResourceGuard(context, shader, freeShaderFunc);

    return true;
}

bool QOpenGLShaderPrivate::compile(QOpenGLShader *q)
{
    GLuint shader = shaderGuard ? shaderGuard->id() : 0;

    if (! shader) {
        return false;
    }

    // Try to compile shader
    glfuncs->glCompileShader(shader);
    GLint value = 0;

    // Get compilation status
    glfuncs->glGetShaderiv(shader, GL_COMPILE_STATUS, &value);
    compiled = (value != 0);

    if (! compiled) {
        // Compilation failed, try to provide some information about the failure
        QString name = q->objectName();

        const char *types[] = {
            "Fragment",
            "Vertex",
            "Geometry",
            "Tessellation Control",
            "Tessellation Evaluation",
            "Compute",
            ""
        };

        const char *type = types[6];
        switch (shaderType) {
        case QOpenGLShader::Fragment:
            type = types[0]; break;
        case QOpenGLShader::Vertex:
            type = types[1]; break;
        case QOpenGLShader::Geometry:
            type = types[2]; break;
        case QOpenGLShader::TessellationControl:
            type = types[3]; break;
        case QOpenGLShader::TessellationEvaluation:
            type = types[4]; break;
        case QOpenGLShader::Compute:
            type = types[5]; break;
        }

        // Get info and source code lengths
        GLint infoLogLength = 0;
        GLint sourceCodeLength = 0;

        char *logBuffer        = nullptr;
        char *sourceCodeBuffer = nullptr;

        // Get the compilation info log
        glfuncs->glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);

        if (infoLogLength > 1) {
            GLint temp;
            logBuffer = new char [infoLogLength];
            glfuncs->glGetShaderInfoLog(shader, infoLogLength, &temp, logBuffer);
        }

        // Get the source code
        glfuncs->glGetShaderiv(shader, GL_SHADER_SOURCE_LENGTH, &sourceCodeLength);

        if (sourceCodeLength > 1) {
            GLint temp;
            sourceCodeBuffer = new char [sourceCodeLength];
            glfuncs->glGetShaderSource(shader, sourceCodeLength, &temp, sourceCodeBuffer);
        }

        if (logBuffer) {
            log = QString::fromLatin1(logBuffer);
        } else {
            log = QString("failed");
        }

        if (name.isEmpty()) {
            qWarning("QOpenGLShader::compile(%s) %s", type, csPrintable(log));
        } else {
            qWarning("QOpenGLShader::compile(%s)[%s] %s", type, csPrintable(name), csPrintable(log));
        }

        // Dump the source code if we got it
        if (sourceCodeBuffer) {
            qWarning("*** Problem %s shader source code ***", type);
            qWarning() << csPrintable(QString::fromLatin1(sourceCodeBuffer));
            qWarning("***");
        }

        // Cleanup
        delete [] logBuffer;
        delete [] sourceCodeBuffer;
    }

    return compiled;
}

void QOpenGLShaderPrivate::deleteShader()
{
    if (shaderGuard) {
        shaderGuard->free();
        shaderGuard = nullptr;
    }
}

QOpenGLShader::QOpenGLShader(QOpenGLShader::ShaderType type, QObject *parent)
    : QObject(parent), d_ptr(new QOpenGLShaderPrivate(QOpenGLContext::currentContext(), type))
{
   d_ptr->q_ptr = this;

   Q_D(QOpenGLShader);
   d->create();
}

QOpenGLShader::~QOpenGLShader()
{
}

QOpenGLShader::ShaderType QOpenGLShader::shaderType() const
{
    Q_D(const QOpenGLShader);
    return d->shaderType;
}

static const char qualifierDefines[] =
    "#define lowp\n"
    "#define mediump\n"
    "#define highp\n";

#if defined(QT_OPENGL_ES) && !defined(QT_OPENGL_FORCE_SHADER_DEFINES)
// The "highp" qualifier doesn't exist in fragment shaders
// on all ES platforms.  When it doesn't exist, use "mediump".
#define QOpenGL_REDEFINE_HIGHP 1
static const char redefineHighp[] =
    "#ifndef GL_FRAGMENT_PRECISION_HIGH\n"
    "#define highp mediump\n"
    "#endif\n";
#endif

struct QVersionDirectivePosition
{
    constexpr QVersionDirectivePosition(int position = 0, int line = -1)
        : position(position)
        , line(line)
    {
    }

    constexpr bool hasPosition() const
    {
        return position > 0;
    }

    const int position;
    const int line;
};

static QVersionDirectivePosition findVersionDirectivePosition(const char *source)
{
    Q_ASSERT(source);

    // According to the GLSL spec the #version directive must not be
    // preceded by anything but whitespace and comments.
    // In order to not get confused by #version directives within a
    // multiline comment, we need to run a minimal preprocessor first.
    enum {
        Normal,
        CommentStarting,
        MultiLineComment,
        SingleLineComment,
        CommentEnding
    } state = Normal;

    QString tmpStr = QString::fromUtf8(source);
    QString retval;

    for (QChar c : tmpStr) {

      switch (state) {
        case Normal:
            if (c == '/') {
                state = CommentStarting;
            }
            break;

        case CommentStarting:
            if (c == '*') {
                state = MultiLineComment;

            } else if (c == '/') {
                state = SingleLineComment;

            } else {
                state = Normal;

            }

            break;

        case MultiLineComment:
            if (c == '*') {
                state = CommentEnding;

            } else if (c == '#') {
                c = '_';
            }

            break;

        case SingleLineComment:
            if (c == '\n') {
                state = Normal;

            } else if (c == '#') {
                c = '_';
            }

            break;

        case CommentEnding:
            if (c == '/') {
                state = Normal;

            } else {
                if (c == '#')
                    c = '_';

                if (c != '*') {
                    state = MultiLineComment;
                }
            }

            break;
        }

        retval.append(c);
    }

    // Search for #version directive
    int splitPosition = 0;
    int linePosition  = 1;

    static const QRegularExpression pattern("^\\s*#\\s*version.*(\\n)?", QPatternOption::MultilineOption );

    QRegularExpressionMatch match = pattern.match(retval);

    if (match.hasMatch()) {
        splitPosition = match.capturedEnd() - retval.begin();
        linePosition += int(std::count(retval.begin(), retval.begin() + splitPosition, '\n'));
    }

    return QVersionDirectivePosition(splitPosition, linePosition);
}


bool QOpenGLShader::compileSourceCode(const char *source)
{
    Q_D(QOpenGLShader);

    // This method breaks the shader code into two parts:
    // 1. Up to and including an optional #version directive.
    // 2. The rest.
    // If a #version directive exists, qualifierDefines and redefineHighp
    // are inserted after. Otherwise they are inserted right at the start.
    // In both cases a #line directive is appended in order to compensate
    // for line number changes in case of compiler errors.

    if (d->shaderGuard && d->shaderGuard->id() && source) {
        const QVersionDirectivePosition versionDirectivePosition = findVersionDirectivePosition(source);

        QVarLengthArray<const char *, 5> sourceChunks;
        QVarLengthArray<GLint, 5> sourceChunkLengths;
        QOpenGLContext *ctx = QOpenGLContext::currentContext();

        if (versionDirectivePosition.hasPosition()) {
            // Append source up to and including the #version directive
            sourceChunks.append(source);
            sourceChunkLengths.append(GLint(versionDirectivePosition.position));

        } else {
            // QTBUG-55733: Intel on Windows with Compatibility profile requires a #version always
            if (ctx->format().profile() == QSurfaceFormat::CompatibilityProfile) {
                const char *vendor = reinterpret_cast<const char *>(ctx->functions()->glGetString(GL_VENDOR));

                if (vendor && !strcmp(vendor, "Intel")) {
                    static const char version110[] = "#version 110\n";
                    sourceChunks.append(version110);
                    sourceChunkLengths.append(GLint(sizeof(version110)) - 1);
                }
            }
        }

        // The precision qualifiers are useful on OpenGL/ES systems,
        // but usually not present on desktop systems.
        const QSurfaceFormat currentSurfaceFormat = ctx->format();
        QOpenGLContextPrivate *ctx_d = QOpenGLContextPrivate::get(QOpenGLContext::currentContext());

        if (currentSurfaceFormat.renderableType() == QSurfaceFormat::OpenGL
              || ctx_d->workaround_missingPrecisionQualifiers
#ifdef QT_OPENGL_FORCE_SHADER_DEFINES
              || true
#endif
              ) {
            sourceChunks.append(qualifierDefines);
            sourceChunkLengths.append(GLint(sizeof(qualifierDefines) - 1));
        }

#ifdef QOpenGL_REDEFINE_HIGHP
        if (d->shaderType == Fragment && !ctx_d->workaround_missingPrecisionQualifiers
              && QOpenGLContext::currentContext()->isOpenGLES()) {
            sourceChunks.append(redefineHighp);
            sourceChunkLengths.append(GLint(sizeof(redefineHighp) - 1));
        }
#endif

        QByteArray lineDirective;
        // #line is rejected by some drivers:
        // "2.1 Mesa 8.1-devel (git-48a3d4e)" or "MESA 2.1 Mesa 8.1-devel"
        const char *version = reinterpret_cast<const char *>(ctx->functions()->glGetString(GL_VERSION));

        if (! version || ! strstr(version, "2.1 Mesa 8")) {
            // Append #line directive in order to compensate for text insertion
            lineDirective = QString("#line %1\n").formatArg(versionDirectivePosition.line).toUtf8();
            sourceChunks.append(lineDirective.constData());
            sourceChunkLengths.append(GLint(lineDirective.length()));
        }

        // Append rest of shader code
        sourceChunks.append(source + versionDirectivePosition.position);
        sourceChunkLengths.append(GLint(qstrlen(source + versionDirectivePosition.position)));

        d->glfuncs->glShaderSource(d->shaderGuard->id(), sourceChunks.size(), sourceChunks.data(), sourceChunkLengths.data());
        return d->compile(this);

    } else {
        return false;
    }
}

bool QOpenGLShader::compileSourceCode(const QByteArray& source)
{
    return compileSourceCode(source.constData());
}

bool QOpenGLShader::compileSourceCode(const QString& source)
{
    return compileSourceCode(source.toLatin1().constData());
}

bool QOpenGLShader::compileSourceFile(const QString& fileName)
{
    QFile file(fileName);

    if (! file.open(QFile::ReadOnly)) {
        qWarning() << "QOpenGLShader::compileSourceFile() Unable to open file" << fileName;
        return false;
    }

    QByteArray contents = file.readAll();

    return compileSourceCode(contents.constData());
}

QByteArray QOpenGLShader::sourceCode() const
{
    Q_D(const QOpenGLShader);

    GLuint shader = d->shaderGuard ? d->shaderGuard->id() : 0;

    if (! shader) {
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

bool QOpenGLShader::isCompiled() const
{
    Q_D(const QOpenGLShader);
    return d->compiled;
}

QString QOpenGLShader::log() const
{
    Q_D(const QOpenGLShader);
    return d->log;
}

GLuint QOpenGLShader::shaderId() const
{
    Q_D(const QOpenGLShader);
    return d->shaderGuard ? d->shaderGuard->id() : 0;
}

class QOpenGLShaderProgramPrivate
{
    Q_DECLARE_PUBLIC(QOpenGLShaderProgram)

public:
    QOpenGLShaderProgramPrivate()
        : programGuard(nullptr), linked(false), inited(false), removingShaders(false), glfuncs(new QOpenGLFunctions)

#ifndef QT_OPENGL_ES_2
        , tessellationFuncs(nullptr)
#endif

    {
    }

    ~QOpenGLShaderProgramPrivate();

    QOpenGLSharedResourceGuard *programGuard;
    bool linked;
    bool inited;
    bool removingShaders;

    QString log;
    QList<QOpenGLShader *> shaders;
    QList<QOpenGLShader *> anonShaders;

    QOpenGLFunctions *glfuncs;

#ifndef QT_OPENGL_ES_2
    // Tessellation shader support
    QOpenGLFunctions_4_0_Core *tessellationFuncs;
#endif

    bool hasShader(QOpenGLShader::ShaderType type) const;

protected:
   QOpenGLShaderProgram *q_ptr;
};

namespace {
    void freeProgramFunc(QOpenGLFunctions *funcs, GLuint id) {
        funcs->glDeleteProgram(id);
    }
}

QOpenGLShaderProgramPrivate::~QOpenGLShaderProgramPrivate()
{
    delete glfuncs;

    if (programGuard) {
        programGuard->free();
    }
}

bool QOpenGLShaderProgramPrivate::hasShader(QOpenGLShader::ShaderType type) const
{
    for (QOpenGLShader *shader : shaders) {
        if (shader->shaderType() == type) {
            return true;
        }
    }
    return false;
}

QOpenGLShaderProgram::QOpenGLShaderProgram(QObject *parent)
    : QObject(parent), d_ptr(new QOpenGLShaderProgramPrivate)
{
   d_ptr->q_ptr = this;
}

QOpenGLShaderProgram::~QOpenGLShaderProgram()
{
}

bool QOpenGLShaderProgram::create()
{
    return init();
}

bool QOpenGLShaderProgram::init()
{
    Q_D(QOpenGLShaderProgram);

    if ((d->programGuard && d->programGuard->id()) || d->inited) {
        return true;
    }

    d->inited = true;

    QOpenGLContext *context = const_cast<QOpenGLContext *>(QOpenGLContext::currentContext());

    if (! context) {
        return false;
    }

    d->glfuncs->initializeOpenGLFunctions();

#ifndef QT_OPENGL_ES_2
    // Resolve OpenGL 4 functions for tessellation shader support
    QSurfaceFormat format = context->format();

    if (! context->isOpenGLES() && format.version() >= qMakePair<int, int>(4, 0)) {
        d->tessellationFuncs = context->versionFunctions<QOpenGLFunctions_4_0_Core>();
        d->tessellationFuncs->initializeOpenGLFunctions();
    }
#endif

    GLuint program = d->glfuncs->glCreateProgram();
    if (! program) {
        qWarning("QOpenGLShaderProgram::init() Unable to compile shader program");
        return false;
    }

    if (d->programGuard) {
        delete d->programGuard;
    }

    d->programGuard = new QOpenGLSharedResourceGuard(context, program, freeProgramFunc);

    return true;
}

bool QOpenGLShaderProgram::addShader(QOpenGLShader *shader)
{
    Q_D(QOpenGLShaderProgram);

    if (! init()) {
        return false;
    }

    if (d->shaders.contains(shader)) {
        return true;    // Already added to this shader program
    }

    if (d->programGuard && d->programGuard->id() && shader) {
        if (! shader->d_func()->shaderGuard || !shader->d_func()->shaderGuard->id()) {
            return false;
        }

        if (d->programGuard->group() != shader->d_func()->shaderGuard->group()) {
            qWarning("QOpenGLShaderProgram::addShader() Program and shader are not associated with same context");
            return false;
        }

        d->glfuncs->glAttachShader(d->programGuard->id(), shader->d_func()->shaderGuard->id());
        d->linked = false;  // Program needs to be relinked.
        d->shaders.append(shader);
        connect(shader, &QOpenGLShader::destroyed, this, &QOpenGLShaderProgram::shaderDestroyed);

        return true;

    } else {
        return false;
    }
}

bool QOpenGLShaderProgram::addShaderFromSourceCode(QOpenGLShader::ShaderType type, const char *source)
{
    Q_D(QOpenGLShaderProgram);

    if (! init()) {
        return false;
    }

    QOpenGLShader *shader = new QOpenGLShader(type, this);
    if (! shader->compileSourceCode(source)) {
        d->log = shader->log();
        delete shader;
        return false;
    }

    d->anonShaders.append(shader);
    return addShader(shader);
}

bool QOpenGLShaderProgram::addShaderFromSourceCode(QOpenGLShader::ShaderType type, const QByteArray& source)
{
    return addShaderFromSourceCode(type, source.constData());
}

bool QOpenGLShaderProgram::addShaderFromSourceCode(QOpenGLShader::ShaderType type, const QString& source)
{
    return addShaderFromSourceCode(type, source.toLatin1().constData());
}

bool QOpenGLShaderProgram::addShaderFromSourceFile
    (QOpenGLShader::ShaderType type, const QString& fileName)
{
    Q_D(QOpenGLShaderProgram);

    if (! init()) {
        return false;
    }

    QOpenGLShader *shader = new QOpenGLShader(type, this);
    if (! shader->compileSourceFile(fileName)) {
        d->log = shader->log();
        delete shader;
        return false;
    }

    d->anonShaders.append(shader);
    return addShader(shader);
}

void QOpenGLShaderProgram::removeShader(QOpenGLShader *shader)
{
    Q_D(QOpenGLShaderProgram);

    if (d->programGuard && d->programGuard->id() && shader && shader->d_func()->shaderGuard) {
        d->glfuncs->glDetachShader(d->programGuard->id(), shader->d_func()->shaderGuard->id());
    }

    d->linked = false;  // Program needs to be relinked.

    if (shader) {
        d->shaders.removeAll(shader);
        d->anonShaders.removeAll(shader);
        disconnect(shader, &QOpenGLShader::destroyed, this, &QOpenGLShaderProgram::shaderDestroyed);
    }
}

QList<QOpenGLShader *> QOpenGLShaderProgram::shaders() const
{
    Q_D(const QOpenGLShaderProgram);
    return d->shaders;
}

void QOpenGLShaderProgram::removeAllShaders()
{
    Q_D(QOpenGLShaderProgram);
    d->removingShaders = true;

    for (QOpenGLShader *shader : d->shaders) {
        if (d->programGuard && d->programGuard->id() && shader && shader->d_func()->shaderGuard) {
            d->glfuncs->glDetachShader(d->programGuard->id(), shader->d_func()->shaderGuard->id());
        }
    }

    for (QOpenGLShader *shader : d->anonShaders) {
        // Delete shader objects that were created anonymously.
        delete shader;
    }

    d->shaders.clear();
    d->anonShaders.clear();
    d->linked = false;  // Program needs to be relinked.
    d->removingShaders = false;
}

bool QOpenGLShaderProgram::link()
{
    Q_D(QOpenGLShaderProgram);

    GLuint program = d->programGuard ? d->programGuard->id() : 0;
    if (! program) {
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

        if (! d->linked) {
            QString name = objectName();

            if (name.isEmpty()) {
                qWarning() << "QOpenGLShader::link()" << d->log;
            } else {
                qWarning() << "QOpenGLShader::link() name =" << name << " " << d->log;
            }
        }

        delete [] logbuf;
    }

    return d->linked;
}

bool QOpenGLShaderProgram::isLinked() const
{
    Q_D(const QOpenGLShaderProgram);
    return d->linked;
}

QString QOpenGLShaderProgram::log() const
{
    Q_D(const QOpenGLShaderProgram);
    return d->log;
}

bool QOpenGLShaderProgram::bind()
{
    Q_D(QOpenGLShaderProgram);

    GLuint program = d->programGuard ? d->programGuard->id() : 0;
    if (! program) {
        return false;
    }

    if (! d->linked && ! link()) {
        return false;
    }

#if defined(CS_SHOW_DEBUG_GUI_OPENGL)
    if (d->programGuard->group() != QOpenGLContextGroup::currentContextGroup()) {
       qDebug("QOpenGLShaderProgram::bind() Program is not valid in the current context");
       return false;
    }
#endif

    d->glfuncs->glUseProgram(program);

    return true;
}

void QOpenGLShaderProgram::release()
{
    Q_D(QOpenGLShaderProgram);

#if defined(CS_SHOW_DEBUG_GUI_OPENGL)
    if (d->programGuard && d->programGuard->group() != QOpenGLContextGroup::currentContextGroup()) {
       qDebug("QOpenGLShaderProgram::release() Program is not valid in the current context");
    }
#endif

    d->glfuncs->glUseProgram(0);
}

GLuint QOpenGLShaderProgram::programId() const
{
    Q_D(const QOpenGLShaderProgram);

    GLuint id = d->programGuard ? d->programGuard->id() : 0;

    if (id) {
        return id;
    }

    // Create the identifier if we don't have one yet.  This is for
    // applications that want to create the attached shader configuration
    // themselves, particularly those using program binaries.
    if (! const_cast<QOpenGLShaderProgram *>(this)->init()) {
        return 0;
    }

    return d->programGuard ? d->programGuard->id() : 0;
}

void QOpenGLShaderProgram::bindAttributeLocation(const char *name, int location)
{
    Q_D(QOpenGLShaderProgram);

    if (! init() || ! d->programGuard || ! d->programGuard->id()) {
        return;
    }

    d->glfuncs->glBindAttribLocation(d->programGuard->id(), location, name);
    d->linked = false;  // Program needs to be relinked.
}

void QOpenGLShaderProgram::bindAttributeLocation(const QByteArray& name, int location)
{
    bindAttributeLocation(name.constData(), location);
}

void QOpenGLShaderProgram::bindAttributeLocation(const QString &name, int location)
{
    bindAttributeLocation(name.toLatin1().constData(), location);
}

int QOpenGLShaderProgram::attributeLocation(const char *name) const
{
    Q_D(const QOpenGLShaderProgram);

    if (d->linked && d->programGuard && d->programGuard->id()) {
        return d->glfuncs->glGetAttribLocation(d->programGuard->id(), name);

    } else {
        qWarning() << "QOpenGLShaderProgram::attributeLocation(" << name
                   << ") Shader program is not linked";
        return -1;
    }
}

int QOpenGLShaderProgram::attributeLocation(const QByteArray &name) const
{
    return attributeLocation(name.constData());
}

int QOpenGLShaderProgram::attributeLocation(const QString &name) const
{
    return attributeLocation(name.toLatin1().constData());
}

void QOpenGLShaderProgram::setAttributeValue(int location, GLfloat value)
{
    Q_D(QOpenGLShaderProgram);

    if (location != -1) {
        d->glfuncs->glVertexAttrib1fv(location, &value);
    }
}

void QOpenGLShaderProgram::setAttributeValue(const char *name, GLfloat value)
{
    setAttributeValue(attributeLocation(name), value);
}

void QOpenGLShaderProgram::setAttributeValue(int location, GLfloat x, GLfloat y)
{
    Q_D(QOpenGLShaderProgram);

    if (location != -1) {
        GLfloat values[2] = {x, y};
        d->glfuncs->glVertexAttrib2fv(location, values);
    }
}

void QOpenGLShaderProgram::setAttributeValue(const char *name, GLfloat x, GLfloat y)
{
    setAttributeValue(attributeLocation(name), x, y);
}

void QOpenGLShaderProgram::setAttributeValue(int location, GLfloat x, GLfloat y, GLfloat z)
{
    Q_D(QOpenGLShaderProgram);

    (void) d;

    if (location != -1) {
        GLfloat values[3] = {x, y, z};
        d->glfuncs->glVertexAttrib3fv(location, values);
    }
}

void QOpenGLShaderProgram::setAttributeValue(const char *name, GLfloat x, GLfloat y, GLfloat z)
{
    setAttributeValue(attributeLocation(name), x, y, z);
}

void QOpenGLShaderProgram::setAttributeValue(int location, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
    Q_D(QOpenGLShaderProgram);

    if (location != -1) {
        GLfloat values[4] = {x, y, z, w};
        d->glfuncs->glVertexAttrib4fv(location, values);
    }
}

void QOpenGLShaderProgram::setAttributeValue(const char *name, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
    setAttributeValue(attributeLocation(name), x, y, z, w);
}

void QOpenGLShaderProgram::setAttributeValue(int location, const QVector2D& value)
{
    Q_D(QOpenGLShaderProgram);

    if (location != -1) {
        d->glfuncs->glVertexAttrib2fv(location, reinterpret_cast<const GLfloat *>(&value));
    }
}

void QOpenGLShaderProgram::setAttributeValue(const char *name, const QVector2D& value)
{
    setAttributeValue(attributeLocation(name), value);
}

void QOpenGLShaderProgram::setAttributeValue(int location, const QVector3D& value)
{
    Q_D(QOpenGLShaderProgram);
    (void) d;

    if (location != -1) {
        d->glfuncs->glVertexAttrib3fv(location, reinterpret_cast<const GLfloat *>(&value));
    }
}

void QOpenGLShaderProgram::setAttributeValue(const char *name, const QVector3D& value)
{
    setAttributeValue(attributeLocation(name), value);
}

void QOpenGLShaderProgram::setAttributeValue(int location, const QVector4D& value)
{
    Q_D(QOpenGLShaderProgram);
    (void) d;

    if (location != -1) {
        d->glfuncs->glVertexAttrib4fv(location, reinterpret_cast<const GLfloat *>(&value));
    }
}

void QOpenGLShaderProgram::setAttributeValue(const char *name, const QVector4D& value)
{
    setAttributeValue(attributeLocation(name), value);
}

void QOpenGLShaderProgram::setAttributeValue(int location, const QColor& value)
{
    Q_D(QOpenGLShaderProgram);
    (void) d;

    if (location != -1) {
        GLfloat values[4] = {GLfloat(value.redF()), GLfloat(value.greenF()),
                             GLfloat(value.blueF()), GLfloat(value.alphaF())};

        d->glfuncs->glVertexAttrib4fv(location, values);
    }
}

void QOpenGLShaderProgram::setAttributeValue(const char *name, const QColor& value)
{
    setAttributeValue(attributeLocation(name), value);
}

void QOpenGLShaderProgram::setAttributeValue(int location, const GLfloat *values, int columns, int rows)
{
    Q_D(QOpenGLShaderProgram);

    (void) d;

    if (rows < 1 || rows > 4) {
        qWarning() << "QOpenGLShaderProgram::setAttributeValue() rows" << rows << "not supported";
        return;
    }

    if (location != -1) {
        while ((columns--) > 0) {
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

void QOpenGLShaderProgram::setAttributeValue(const char *name, const GLfloat *values, int columns, int rows)
{
    setAttributeValue(attributeLocation(name), values, columns, rows);
}

void QOpenGLShaderProgram::setAttributeArray(int location, const GLfloat *values, int tupleSize, int stride)
{
    Q_D(QOpenGLShaderProgram);
    (void) d;

    if (location != -1) {
        d->glfuncs->glVertexAttribPointer(location, tupleSize, GL_FLOAT, GL_FALSE, stride, values);
    }
}

void QOpenGLShaderProgram::setAttributeArray(int location, const QVector2D *values, int stride)
{
    Q_D(QOpenGLShaderProgram);
    (void) d;

    if (location != -1) {
        d->glfuncs->glVertexAttribPointer(location, 2, GL_FLOAT, GL_FALSE, stride, values);
    }
}

void QOpenGLShaderProgram::setAttributeArray(int location, const QVector3D *values, int stride)
{
    Q_D(QOpenGLShaderProgram);
    (void) d;

    if (location != -1) {
        d->glfuncs->glVertexAttribPointer(location, 3, GL_FLOAT, GL_FALSE, stride, values);
    }
}

void QOpenGLShaderProgram::setAttributeArray(int location, const QVector4D *values, int stride)
{
    Q_D(QOpenGLShaderProgram);
    (void) d;

    if (location != -1) {
        d->glfuncs->glVertexAttribPointer(location, 4, GL_FLOAT, GL_FALSE, stride, values);
    }
}

void QOpenGLShaderProgram::setAttributeArray(int location, GLenum type, const void *values, int tupleSize, int stride)
{
    Q_D(QOpenGLShaderProgram);
    (void) d;

    if (location != -1) {
        d->glfuncs->glVertexAttribPointer(location, tupleSize, type, GL_TRUE, stride, values);
    }
}

void QOpenGLShaderProgram::setAttributeArray(const char *name, const GLfloat *values, int tupleSize, int stride)
{
    setAttributeArray(attributeLocation(name), values, tupleSize, stride);
}

void QOpenGLShaderProgram::setAttributeArray(const char *name, const QVector2D *values, int stride)
{
    setAttributeArray(attributeLocation(name), values, stride);
}

void QOpenGLShaderProgram::setAttributeArray(const char *name, const QVector3D *values, int stride)
{
    setAttributeArray(attributeLocation(name), values, stride);
}

void QOpenGLShaderProgram::setAttributeArray(const char *name, const QVector4D *values, int stride)
{
    setAttributeArray(attributeLocation(name), values, stride);
}

void QOpenGLShaderProgram::setAttributeArray(const char *name, GLenum type, const void *values, int tupleSize, int stride)
{
    setAttributeArray(attributeLocation(name), type, values, tupleSize, stride);
}

void QOpenGLShaderProgram::setAttributeBuffer(int location, GLenum type, int offset, int tupleSize, int stride)
{
    Q_D(QOpenGLShaderProgram);
    (void) d;

    if (location != -1) {
        d->glfuncs->glVertexAttribPointer(location, tupleSize, type, GL_TRUE, stride,
              reinterpret_cast<const void *>(qintptr(offset)));
    }
}

void QOpenGLShaderProgram::setAttributeBuffer(const char *name, GLenum type, int offset, int tupleSize, int stride)
{
    setAttributeBuffer(attributeLocation(name), type, offset, tupleSize, stride);
}

void QOpenGLShaderProgram::enableAttributeArray(int location)
{
    Q_D(QOpenGLShaderProgram);
    (void) d;

    if (location != -1) {
        d->glfuncs->glEnableVertexAttribArray(location);
    }
}

void QOpenGLShaderProgram::enableAttributeArray(const char *name)
{
    enableAttributeArray(attributeLocation(name));
}

void QOpenGLShaderProgram::disableAttributeArray(int location)
{
    Q_D(QOpenGLShaderProgram);
    (void) d;

    if (location != -1) {
        d->glfuncs->glDisableVertexAttribArray(location);
    }
}

void QOpenGLShaderProgram::disableAttributeArray(const char *name)
{
    disableAttributeArray(attributeLocation(name));
}

int QOpenGLShaderProgram::uniformLocation(const char *name) const
{
    Q_D(const QOpenGLShaderProgram);

    if (d->linked && d->programGuard && d->programGuard->id()) {
        return d->glfuncs->glGetUniformLocation(d->programGuard->id(), name);

    } else {
        (void) d;

        qWarning() << "QOpenGLShaderProgram::uniformLocation() Shader program is not linked " << name;
        return -1;
    }
}

int QOpenGLShaderProgram::uniformLocation(const QByteArray& name) const
{
    return uniformLocation(name.constData());
}

int QOpenGLShaderProgram::uniformLocation(const QString& name) const
{
    return uniformLocation(name.toLatin1().constData());
}

void QOpenGLShaderProgram::setUniformValue(int location, GLfloat value)
{
    Q_D(QOpenGLShaderProgram);
    (void) d;

    if (location != -1) {
        d->glfuncs->glUniform1fv(location, 1, &value);
    }
}

void QOpenGLShaderProgram::setUniformValue(const char *name, GLfloat value)
{
    setUniformValue(uniformLocation(name), value);
}

void QOpenGLShaderProgram::setUniformValue(int location, GLint value)
{
    Q_D(QOpenGLShaderProgram);
    (void) d;

    if (location != -1) {
        d->glfuncs->glUniform1i(location, value);
    }
}

void QOpenGLShaderProgram::setUniformValue(const char *name, GLint value)
{
    setUniformValue(uniformLocation(name), value);
}

void QOpenGLShaderProgram::setUniformValue(int location, GLuint value)
{
    Q_D(QOpenGLShaderProgram);
    (void) d;

    if (location != -1) {
        d->glfuncs->glUniform1i(location, value);
    }
}

void QOpenGLShaderProgram::setUniformValue(const char *name, GLuint value)
{
    setUniformValue(uniformLocation(name), value);
}

void QOpenGLShaderProgram::setUniformValue(int location, GLfloat x, GLfloat y)
{
    Q_D(QOpenGLShaderProgram);
    (void) d;

    if (location != -1) {
        GLfloat values[2] = {x, y};
        d->glfuncs->glUniform2fv(location, 1, values);
    }
}

void QOpenGLShaderProgram::setUniformValue(const char *name, GLfloat x, GLfloat y)
{
    setUniformValue(uniformLocation(name), x, y);
}

void QOpenGLShaderProgram::setUniformValue(int location, GLfloat x, GLfloat y, GLfloat z)
{
    Q_D(QOpenGLShaderProgram);
    (void) d;

    if (location != -1) {
        GLfloat values[3] = {x, y, z};
        d->glfuncs->glUniform3fv(location, 1, values);
    }
}

void QOpenGLShaderProgram::setUniformValue(const char *name, GLfloat x, GLfloat y, GLfloat z)
{
    setUniformValue(uniformLocation(name), x, y, z);
}

void QOpenGLShaderProgram::setUniformValue(int location, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
    Q_D(QOpenGLShaderProgram);
    (void) d;

    if (location != -1) {
        GLfloat values[4] = {x, y, z, w};
        d->glfuncs->glUniform4fv(location, 1, values);
    }
}

void QOpenGLShaderProgram::setUniformValue(const char *name, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
    setUniformValue(uniformLocation(name), x, y, z, w);
}

void QOpenGLShaderProgram::setUniformValue(int location, const QVector2D& value)
{
    Q_D(QOpenGLShaderProgram);
    (void) d;

    if (location != -1) {
        d->glfuncs->glUniform2fv(location, 1, reinterpret_cast<const GLfloat *>(&value));
    }
}

void QOpenGLShaderProgram::setUniformValue(const char *name, const QVector2D& value)
{
    setUniformValue(uniformLocation(name), value);
}

void QOpenGLShaderProgram::setUniformValue(int location, const QVector3D& value)
{
    Q_D(QOpenGLShaderProgram);
    (void) d;

    if (location != -1) {
        d->glfuncs->glUniform3fv(location, 1, reinterpret_cast<const GLfloat *>(&value));
    }
}

void QOpenGLShaderProgram::setUniformValue(const char *name, const QVector3D& value)
{
    setUniformValue(uniformLocation(name), value);
}

void QOpenGLShaderProgram::setUniformValue(int location, const QVector4D& value)
{
    Q_D(QOpenGLShaderProgram);
    (void) d;

    if (location != -1) {
        d->glfuncs->glUniform4fv(location, 1, reinterpret_cast<const GLfloat *>(&value));
    }
}

void QOpenGLShaderProgram::setUniformValue(const char *name, const QVector4D& value)
{
    setUniformValue(uniformLocation(name), value);
}

void QOpenGLShaderProgram::setUniformValue(int location, const QColor& color)
{
    Q_D(QOpenGLShaderProgram);
    (void) d;

    if (location != -1) {
        GLfloat values[4] = {GLfloat(color.redF()), GLfloat(color.greenF()),
                             GLfloat(color.blueF()), GLfloat(color.alphaF())};
        d->glfuncs->glUniform4fv(location, 1, values);
    }
}

void QOpenGLShaderProgram::setUniformValue(const char *name, const QColor& color)
{
    setUniformValue(uniformLocation(name), color);
}

void QOpenGLShaderProgram::setUniformValue(int location, const QPoint& point)
{
    Q_D(QOpenGLShaderProgram);
    (void) d;

    if (location != -1) {
        GLfloat values[4] = {GLfloat(point.x()), GLfloat(point.y())};
        d->glfuncs->glUniform2fv(location, 1, values);
    }
}

void QOpenGLShaderProgram::setUniformValue(const char *name, const QPoint& point)
{
    setUniformValue(uniformLocation(name), point);
}

void QOpenGLShaderProgram::setUniformValue(int location, const QPointF& point)
{
    Q_D(QOpenGLShaderProgram);
    (void) d;

    if (location != -1) {
        GLfloat values[4] = {GLfloat(point.x()), GLfloat(point.y())};
        d->glfuncs->glUniform2fv(location, 1, values);
    }
}

void QOpenGLShaderProgram::setUniformValue(const char *name, const QPointF& point)
{
    setUniformValue(uniformLocation(name), point);
}

void QOpenGLShaderProgram::setUniformValue(int location, const QSize& size)
{
    Q_D(QOpenGLShaderProgram);
    (void) d;

    if (location != -1) {
        GLfloat values[4] = {GLfloat(size.width()), GLfloat(size.height())};
        d->glfuncs->glUniform2fv(location, 1, values);
    }
}

void QOpenGLShaderProgram::setUniformValue(const char *name, const QSize& size)
{
    setUniformValue(uniformLocation(name), size);
}

void QOpenGLShaderProgram::setUniformValue(int location, const QSizeF& size)
{
    Q_D(QOpenGLShaderProgram);
    (void) d;

    if (location != -1) {
        GLfloat values[4] = {GLfloat(size.width()), GLfloat(size.height())};
        d->glfuncs->glUniform2fv(location, 1, values);
    }
}

void QOpenGLShaderProgram::setUniformValue(const char *name, const QSizeF& size)
{
    setUniformValue(uniformLocation(name), size);
}

void QOpenGLShaderProgram::setUniformValue(int location, const QMatrix2x2& value)
{
    Q_D(QOpenGLShaderProgram);

    constexpr int max = 4;
    GLfloat tmp[max];

    for (int i = 0; i < max; ++i) {
      tmp[i] = value.constData()[i];
    }

    d->glfuncs->glUniformMatrix2fv(location, 1, GL_FALSE, tmp);
}

void QOpenGLShaderProgram::setUniformValue(const char *name, const QMatrix2x2& value)
{
    setUniformValue(uniformLocation(name), value);
}

void QOpenGLShaderProgram::setUniformValue(int location, const QMatrix2x3& value)
{
    Q_D(QOpenGLShaderProgram);

    constexpr int max = 6;
    GLfloat tmp[max];

    for (int i = 0; i < max; ++i) {
      tmp[i] = value.constData()[i];
    }

    d->glfuncs->glUniform3fv(location, 2, tmp);
}

void QOpenGLShaderProgram::setUniformValue(const char *name, const QMatrix2x3& value)
{
    setUniformValue(uniformLocation(name), value);
}

void QOpenGLShaderProgram::setUniformValue(int location, const QMatrix2x4& value)
{
    Q_D(QOpenGLShaderProgram);

    constexpr int max = 8;
    GLfloat tmp[max];

    for (int i = 0; i < max; ++i) {
      tmp[i] = value.constData()[i];
    }

    d->glfuncs->glUniform4fv(location, 2, tmp);
}

void QOpenGLShaderProgram::setUniformValue(const char *name, const QMatrix2x4& value)
{
    setUniformValue(uniformLocation(name), value);
}

void QOpenGLShaderProgram::setUniformValue(int location, const QMatrix3x2& value)
{
    Q_D(QOpenGLShaderProgram);

    constexpr int max = 6;
    GLfloat tmp[max];

    for (int i = 0; i < max; ++i) {
      tmp[i] = value.constData()[i];
    }

    d->glfuncs->glUniform2fv(location, 3, tmp);
}

void QOpenGLShaderProgram::setUniformValue(const char *name, const QMatrix3x2& value)
{
    setUniformValue(uniformLocation(name), value);
}

void QOpenGLShaderProgram::setUniformValue(int location, const QMatrix3x3& value)
{
    Q_D(QOpenGLShaderProgram);

    constexpr int max = 9;
    GLfloat tmp[max];

    for (int i = 0; i < max; ++i) {
      tmp[i] = value.constData()[i];
    }

    d->glfuncs->glUniformMatrix3fv(location, 1, GL_FALSE, tmp);
}

void QOpenGLShaderProgram::setUniformValue(const char *name, const QMatrix3x3& value)
{
    setUniformValue(uniformLocation(name), value);
}

void QOpenGLShaderProgram::setUniformValue(int location, const QMatrix3x4 &value)
{
    Q_D(QOpenGLShaderProgram);

    constexpr int max = 12;
    GLfloat tmp[max];

    for (int i = 0; i < max; ++i) {
      tmp[i] = value.constData()[i];
    }

    d->glfuncs->glUniform4fv(location, 3, tmp);
}

void QOpenGLShaderProgram::setUniformValue(const char *name, const QMatrix3x4& value)
{
    setUniformValue(uniformLocation(name), value);
}

void QOpenGLShaderProgram::setUniformValue(int location, const QMatrix4x2& value)
{
    Q_D(QOpenGLShaderProgram);

    constexpr int max = 8;
    GLfloat tmp[max];

    for (int i = 0; i < max; ++i) {
      tmp[i] = value.constData()[i];
    }

    d->glfuncs->glUniform2fv(location, 4, tmp);
}

void QOpenGLShaderProgram::setUniformValue(const char *name, const QMatrix4x2& value)
{
    setUniformValue(uniformLocation(name), value);
}

void QOpenGLShaderProgram::setUniformValue(int location, const QMatrix4x3& value)
{
    Q_D(QOpenGLShaderProgram);

    constexpr int max = 12;
    GLfloat tmp[max];

    for (int i = 0; i < max; ++i) {
      tmp[i] = value.constData()[i];
    }

    d->glfuncs->glUniform3fv(location, 4, tmp);
}

void QOpenGLShaderProgram::setUniformValue(const char *name, const QMatrix4x3& value)
{
    setUniformValue(uniformLocation(name), value);
}

void QOpenGLShaderProgram::setUniformValue(int location, const QMatrix4x4& value)
{
    Q_D(QOpenGLShaderProgram);

    constexpr int max = 16;
    GLfloat tmp[max];

    for (int i = 0; i < max; ++i) {
      tmp[i] = value.constData()[i];
    }

    d->glfuncs->glUniformMatrix4fv(location, 1, GL_FALSE, tmp);
}

void QOpenGLShaderProgram::setUniformValue(const char *name, const QMatrix4x4& value)
{
    setUniformValue(uniformLocation(name), value);
}

void QOpenGLShaderProgram::setUniformValue(int location, const GLfloat value[2][2])
{
    Q_D(QOpenGLShaderProgram);

    if (location != -1) {
        d->glfuncs->glUniformMatrix2fv(location, 1, GL_FALSE, value[0]);
    }
}

void QOpenGLShaderProgram::setUniformValue(int location, const GLfloat value[3][3])
{
    Q_D(QOpenGLShaderProgram);

    if (location != -1) {
        d->glfuncs->glUniformMatrix3fv(location, 1, GL_FALSE, value[0]);
    }
}

void QOpenGLShaderProgram::setUniformValue(int location, const GLfloat value[4][4])
{
    Q_D(QOpenGLShaderProgram);
    (void) d;

    if (location != -1) {
        d->glfuncs->glUniformMatrix4fv(location, 1, GL_FALSE, value[0]);
    }
}

void QOpenGLShaderProgram::setUniformValue(const char *name, const GLfloat value[2][2])
{
    setUniformValue(uniformLocation(name), value);
}

void QOpenGLShaderProgram::setUniformValue(const char *name, const GLfloat value[3][3])
{
    setUniformValue(uniformLocation(name), value);
}

void QOpenGLShaderProgram::setUniformValue(const char *name, const GLfloat value[4][4])
{
    setUniformValue(uniformLocation(name), value);
}

void QOpenGLShaderProgram::setUniformValue(int location, const QTransform& value)
{
    Q_D(QOpenGLShaderProgram);
    (void) d;

    if (location != -1) {
        GLfloat mat[3][3] = {
            {GLfloat(value.m11()), GLfloat(value.m12()), GLfloat(value.m13())},
            {GLfloat(value.m21()), GLfloat(value.m22()), GLfloat(value.m23())},
            {GLfloat(value.m31()), GLfloat(value.m32()), GLfloat(value.m33())}
        };
        d->glfuncs->glUniformMatrix3fv(location, 1, GL_FALSE, mat[0]);
    }
}

void QOpenGLShaderProgram::setUniformValue(const char *name, const QTransform& value)
{
    setUniformValue(uniformLocation(name), value);
}

void QOpenGLShaderProgram::setUniformValueArray(int location, const GLint *values, int count)
{
    Q_D(QOpenGLShaderProgram);
    (void) d;

    if (location != -1) {
        d->glfuncs->glUniform1iv(location, count, values);
    }
}

void QOpenGLShaderProgram::setUniformValueArray(const char *name, const GLint *values, int count)
{
    setUniformValueArray(uniformLocation(name), values, count);
}

void QOpenGLShaderProgram::setUniformValueArray(int location, const GLuint *values, int count)
{
    Q_D(QOpenGLShaderProgram);
    (void) d;

    if (location != -1) {
        d->glfuncs->glUniform1iv(location, count, reinterpret_cast<const GLint *>(values));
    }
}

void QOpenGLShaderProgram::setUniformValueArray(const char *name, const GLuint *values, int count)
{
    setUniformValueArray(uniformLocation(name), values, count);
}

void QOpenGLShaderProgram::setUniformValueArray(int location, const GLfloat *values, int count, int tupleSize)
{
    Q_D(QOpenGLShaderProgram);

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
            (void) d;
            qWarning() << "QOpenGLShaderProgram::setUniformValue() size" << tupleSize << "not supported";
        }
    }
}

void QOpenGLShaderProgram::setUniformValueArray(const char *name, const GLfloat *values, int count, int tupleSize)
{
    setUniformValueArray(uniformLocation(name), values, count, tupleSize);
}

void QOpenGLShaderProgram::setUniformValueArray(int location, const QVector2D *values, int count)
{
    Q_D(QOpenGLShaderProgram);
    (void) d;

    if (location != -1) {
        d->glfuncs->glUniform2fv(location, count, reinterpret_cast<const GLfloat *>(values));
    }
}

void QOpenGLShaderProgram::setUniformValueArray(const char *name, const QVector2D *values, int count)
{
    setUniformValueArray(uniformLocation(name), values, count);
}

void QOpenGLShaderProgram::setUniformValueArray(int location, const QVector3D *values, int count)
{
    Q_D(QOpenGLShaderProgram);
    (void) d;

    if (location != -1) {
        d->glfuncs->glUniform3fv(location, count, reinterpret_cast<const GLfloat *>(values));
    }
}

void QOpenGLShaderProgram::setUniformValueArray(const char *name, const QVector3D *values, int count)
{
    setUniformValueArray(uniformLocation(name), values, count);
}

void QOpenGLShaderProgram::setUniformValueArray(int location, const QVector4D *values, int count)
{
    Q_D(QOpenGLShaderProgram);
    (void) d;

    if (location != -1) {
        d->glfuncs->glUniform4fv(location, count, reinterpret_cast<const GLfloat *>(values));
    }
}

void QOpenGLShaderProgram::setUniformValueArray(const char *name, const QVector4D *values, int count)
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

void QOpenGLShaderProgram::setUniformValueArray(int location, const QMatrix2x2 *values, int count)
{
    Q_D(QOpenGLShaderProgram);
    (void) d;

    setUniformMatrixArray(d->glfuncs->glUniformMatrix2fv, location, values, count, QMatrix2x2, 2, 2);
}

void QOpenGLShaderProgram::setUniformValueArray(const char *name, const QMatrix2x2 *values, int count)
{
    setUniformValueArray(uniformLocation(name), values, count);
}

void QOpenGLShaderProgram::setUniformValueArray(int location, const QMatrix2x3 *values, int count)
{
    Q_D(QOpenGLShaderProgram);
    (void) d;

    setUniformGenericMatrixArray(d->glfuncs->glUniform3fv, location, values, count, QMatrix2x3, 2, 3);
}

void QOpenGLShaderProgram::setUniformValueArray(const char *name, const QMatrix2x3 *values, int count)
{
    setUniformValueArray(uniformLocation(name), values, count);
}

void QOpenGLShaderProgram::setUniformValueArray(int location, const QMatrix2x4 *values, int count)
{
    Q_D(QOpenGLShaderProgram);
    (void) d;

    setUniformGenericMatrixArray(d->glfuncs->glUniform4fv, location, values, count, QMatrix2x4, 2, 4);
}

void QOpenGLShaderProgram::setUniformValueArray(const char *name, const QMatrix2x4 *values, int count)
{
    setUniformValueArray(uniformLocation(name), values, count);
}

void QOpenGLShaderProgram::setUniformValueArray(int location, const QMatrix3x2 *values, int count)
{
    Q_D(QOpenGLShaderProgram);
    (void) d;

    setUniformGenericMatrixArray(d->glfuncs->glUniform2fv, location, values, count, QMatrix3x2, 3, 2);
}

void QOpenGLShaderProgram::setUniformValueArray(const char *name, const QMatrix3x2 *values, int count)
{
    setUniformValueArray(uniformLocation(name), values, count);
}

void QOpenGLShaderProgram::setUniformValueArray(int location, const QMatrix3x3 *values, int count)
{
    Q_D(QOpenGLShaderProgram);
    (void) d;

    setUniformMatrixArray(d->glfuncs->glUniformMatrix3fv, location, values, count, QMatrix3x3, 3, 3);
}

void QOpenGLShaderProgram::setUniformValueArray(const char *name, const QMatrix3x3 *values, int count)
{
    setUniformValueArray(uniformLocation(name), values, count);
}

void QOpenGLShaderProgram::setUniformValueArray(int location, const QMatrix3x4 *values, int count)
{
    Q_D(QOpenGLShaderProgram);
    (void) d;

    setUniformGenericMatrixArray(d->glfuncs->glUniform4fv, location, values, count, QMatrix3x4, 3, 4);
}

void QOpenGLShaderProgram::setUniformValueArray(const char *name, const QMatrix3x4 *values, int count)
{
    setUniformValueArray(uniformLocation(name), values, count);
}

void QOpenGLShaderProgram::setUniformValueArray(int location, const QMatrix4x2 *values, int count)
{
    Q_D(QOpenGLShaderProgram);
    (void) d;

    setUniformGenericMatrixArray(d->glfuncs->glUniform2fv, location, values, count, QMatrix4x2, 4, 2);
}

void QOpenGLShaderProgram::setUniformValueArray(const char *name, const QMatrix4x2 *values, int count)
{
    setUniformValueArray(uniformLocation(name), values, count);
}

void QOpenGLShaderProgram::setUniformValueArray(int location, const QMatrix4x3 *values, int count)
{
    Q_D(QOpenGLShaderProgram);
    (void) d;

    setUniformGenericMatrixArray(d->glfuncs->glUniform3fv, location, values, count, QMatrix4x3, 4, 3);
}

void QOpenGLShaderProgram::setUniformValueArray(const char *name, const QMatrix4x3 *values, int count)
{
    setUniformValueArray(uniformLocation(name), values, count);
}

void QOpenGLShaderProgram::setUniformValueArray(int location, const QMatrix4x4 *values, int count)
{
    Q_D(QOpenGLShaderProgram);
    (void) d;

    setUniformMatrixArray(d->glfuncs->glUniformMatrix4fv, location, values, count, QMatrix4x4, 4, 4);
}

void QOpenGLShaderProgram::setUniformValueArray(const char *name, const QMatrix4x4 *values, int count)
{
    setUniformValueArray(uniformLocation(name), values, count);
}

int QOpenGLShaderProgram::maxGeometryOutputVertices() const
{
    GLint n = 0;

#if defined(QT_OPENGL_3_2)
    Q_D(const QOpenGLShaderProgram);
    d->glfuncs->glGetIntegerv(GL_MAX_GEOMETRY_OUTPUT_VERTICES, &n);
#endif

    return n;
}

void QOpenGLShaderProgram::setPatchVertexCount(int count)
{
#if defined(QT_OPENGL_4)
    Q_D(QOpenGLShaderProgram);

    if (d->tessellationFuncs) {
        d->tessellationFuncs->glPatchParameteri(GL_PATCH_VERTICES, count);
    }
#else
    (void) count;
#endif
}
int QOpenGLShaderProgram::patchVertexCount() const
{
    int patchVertices = 0;

#if defined(QT_OPENGL_4)
    Q_D(const QOpenGLShaderProgram);

    if (d->tessellationFuncs) {
        d->tessellationFuncs->glGetIntegerv(GL_PATCH_VERTICES, &patchVertices);
    }
#endif

    return patchVertices;
}

void QOpenGLShaderProgram::setDefaultOuterTessellationLevels(const QVector<float> &levels)
{
#if defined(QT_OPENGL_4)
    QVector<float> tessLevels = levels;

    // Ensure we have the required 4 outer tessellation levels
    // Use default of 1 for missing entries (same as spec)
    const int argCount = 4;

    if (tessLevels.size() < argCount) {
        tessLevels.reserve(argCount);

        for (int i = tessLevels.size(); i < argCount; ++i) {
            tessLevels.append(1.0f);
        }
    }

    Q_D(QOpenGLShaderProgram);

    if (d->tessellationFuncs) {
        d->tessellationFuncs->glPatchParameterfv(GL_PATCH_DEFAULT_OUTER_LEVEL, tessLevels.data());
    }

#else
    (void) levels;
#endif
}

QVector<float> QOpenGLShaderProgram::defaultOuterTessellationLevels() const
{
    QVector<float> tessLevels(4, 1.0f);

#if defined(QT_OPENGL_4)
    Q_D(const QOpenGLShaderProgram);

    if (d->tessellationFuncs) {
        d->tessellationFuncs->glGetFloatv(GL_PATCH_DEFAULT_OUTER_LEVEL, tessLevels.data());
    }
#endif

    return tessLevels;
}

void QOpenGLShaderProgram::setDefaultInnerTessellationLevels(const QVector<float> &levels)
{
#if defined(QT_OPENGL_4)
    QVector<float> tessLevels = levels;

    // Ensure we have the required 2 inner tessellation levels
    // Use default of 1 for missing entries (same as spec)
    const int argCount = 2;

    if (tessLevels.size() < argCount) {
        tessLevels.reserve(argCount);

        for (int i = tessLevels.size(); i < argCount; ++i) {
            tessLevels.append(1.0f);
        }
    }

    Q_D(QOpenGLShaderProgram);

    if (d->tessellationFuncs) {
        d->tessellationFuncs->glPatchParameterfv(GL_PATCH_DEFAULT_INNER_LEVEL, tessLevels.data());
    }
#else
    (void) levels;
#endif
}

QVector<float> QOpenGLShaderProgram::defaultInnerTessellationLevels() const
{
    QVector<float> tessLevels(2, 1.0f);

#if defined(QT_OPENGL_4)
    Q_D(const QOpenGLShaderProgram);

    if (d->tessellationFuncs) {
        d->tessellationFuncs->glGetFloatv(GL_PATCH_DEFAULT_INNER_LEVEL, tessLevels.data());
    }
#endif

    return tessLevels;
}

bool QOpenGLShaderProgram::hasOpenGLShaderPrograms(QOpenGLContext *context)
{
#if ! defined(QT_OPENGL_ES_2)
    if (! context) {
        context = QOpenGLContext::currentContext();
    }

    if (! context) {
        return false;
    }

    return QOpenGLFunctions(context).hasOpenGLFeature(QOpenGLFunctions::Shaders);
#else
    (void) context;
    return true;
#endif
}

// internal
void QOpenGLShaderProgram::shaderDestroyed()
{
    Q_D(QOpenGLShaderProgram);
    QOpenGLShader *shader = qobject_cast<QOpenGLShader *>(sender());

    if (shader && ! d->removingShaders) {
        removeShader(shader);
    }
}

bool QOpenGLShader::hasOpenGLShaders(ShaderType type, QOpenGLContext *context)
{
    if (! context) {
        context = QOpenGLContext::currentContext();
    }

    if (! context) {
        return false;
    }

    if ((type & ~(Geometry | Vertex | Fragment | TessellationControl | TessellationEvaluation | Compute)) || type == 0) {
        return false;
    }

    QSurfaceFormat format = context->format();

    if (type == Geometry) {
#ifndef QT_OPENGL_ES_2
        // Geometry shaders require OpenGL 3.2 or newer
        QSurfaceFormat format = context->format();
        return (! context->isOpenGLES()) && (format.version() >= qMakePair<int, int>(3, 2));
#else
        // No geometry shader support in OpenGL ES2
        return false;
#endif

    } else if (type == TessellationControl || type == TessellationEvaluation) {
#if ! defined(QT_OPENGL_ES_2)
        return (!context->isOpenGLES()) && (format.version() >= qMakePair<int, int>(4, 0));
#else
        // No tessellation shader support in OpenGL ES2
        return false;
#endif

    } else if (type == Compute) {
#if defined(QT_OPENGL_4_3)
        return (format.version() >= qMakePair<int, int>(4, 3));
#else
        // No compute shader support without OpenGL 4.3 or newer
        return false;
#endif

    }

    // Unconditional support of vertex and fragment shaders implicitly assumes
    // a minimum OpenGL version of 2.0
    return true;
}
