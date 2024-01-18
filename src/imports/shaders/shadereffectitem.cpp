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

#include "shadereffectitem.h"
#include "shadereffect.h"
#include "glfunctions.h"

#include <QPainter>
#include <QtOpenGL>

static const char qt_default_vertex_code[] =
        "uniform highp mat4 qt_ModelViewProjectionMatrix;\n"
        "attribute highp vec4 qt_Vertex;\n"
        "attribute highp vec2 qt_MultiTexCoord0;\n"
        "varying highp vec2 qt_TexCoord0;\n"
        "void main(void)\n"
        "{\n"
            "qt_TexCoord0 = qt_MultiTexCoord0;\n"
            "gl_Position = qt_ModelViewProjectionMatrix * qt_Vertex;\n"
        "}\n";

static const char qt_default_fragment_code[] =
        "varying highp vec2 qt_TexCoord0;\n"
        "uniform lowp sampler2D source;\n"
        "void main(void)\n"
        "{\n"
            "gl_FragColor = texture2D(source, qt_TexCoord0.st);\n"
        "}\n";

static const char qt_postion_attribute_name[] = "qt_Vertex";
static const char qt_texcoord_attribute_name[] = "qt_MultiTexCoord0";
static const char qt_emptyAttributeName[] = "";


/*!
    \qmlclass ShaderEffectItem ShaderEffectItem
    \ingroup qml-shader-elements
    \brief The ShaderEffectItem object alters the output of given item with OpenGL shaders.
    \inherits Item

    ShaderEffectItem is available in the \bold{Qt.labs.shaders 1.0} module.
    \e {Elements in the Qt.labs module are not guaranteed to remain compatible
    in future versions.}

    This element provides preliminary support for embedding OpenGL shader code into QML,
    and may be heavily changed or removed in later versions.

    Requirement for the use of shaders is that the application is either using
    Qt OpenGL graphicssystem or is using OpenGL by setting QGLWidget as the viewport to QDeclarativeView (depending on which one is the recommened way in the targeted platform).

    ShaderEffectItem internal behaviour is such that during the paint event it first renders its
    ShaderEffectSource items into a OpenGL framebuffer object which can be used as a texture. If the ShaderEffectSource is defined to be an image,
    it is directly uploaded as a texture. The texture(s) containing the source pixelcontent are then bound to graphics
    pipeline texture units. Finally a textured mesh is passed to the vertex- and fragmentshaders which
    then produce the final output for the ShaderEffectItem. It is possible to alter the mesh structure by defining
    the amount vertices it contains, but currently it is not possible to import complex 3D-models to be used as the mesh.

    It is possible to define one or more ShaderEffectItems to be a ShaderEffectSource for other ShaderEffectItems, but ShaderEffectItem
    should never be declared as a child element of its source item(s) because it would cause circular loop in the painting.

    A standard set of vertex attributes are provided for the shaders:

    \list
    \o qt_Vertex - The primary position of the vertex.
    \o qt_MultiTexCoord0 - The texture co-ordinate at each vertex for texture unit 0.
    \endlist

    Additionally following uniforms are available for shaders:

    \list
    \o qt_Opacity - Effective opacity of the item.
    \o qt_ModelViewProjectionMatrix - current 4x4 transformation matrix of the item.
    \endlist

    Furthermore, it is possible to utilize automatic QML propertybinding into vertex- and fragment shader
    uniforms. Conversions are done according to the table below:

    \table
    \header
        \o QML property
        \o GLSL uniform
    \row
        \o property double foo: 1.0
        \o uniform highp float foo
    \row
        \o property real foo: 1.0
        \o uniform highp float foo
    \row
        \o property bool foo: true
        \o uniform bool foo
    \row
        \o property int foo: 1
        \o uniform int foo
    \row
        \o property variant foo: Qt.point(1,1)
        \o uniform highp vec2 foo
    \row
        \o property variant foo: Qt.size(1, 1)
        \o uniform highp vec2 foo
    \row
        \o property variant foo: Qt.rect(1, 1, 2, 2)
        \o uniform highp vec4 foo
    \row
        \o property color foo: "#00000000"
        \o uniform lowp vec4 foo
    \row
        \o property variant foo: Qt.vector3d(1.0, 2.0, 0.0)
        \o uniform highp vec3 foo
    \row
        \o property variant foo: ShaderEffectSource { SourceItem: bar }
        \o uniform lowp sampler2D foo
    \endtable
    \note
    The uniform precision definitions in the above table are not strict, it is possible to choose the uniform
    precision based on what is the most suitable for the shader code for that particular uniform.


    The below example uses fragment shader to create simple wiggly effect to a text label.
    Automatic property binding takes care of binding the properties to the uniforms if their
    names are identical. ShaderEffectSource referring to textLabel is bound to sampler2D uniform inside the fragment
    shader code.

    \qml
import QtQuick 1.0
import Qt.labs.shaders 1.0

Rectangle {
    width: 300
    height: 300
    color: "black"

    Text {
        id: textLabel
        text: "Hello World"
        anchors.centerIn: parent
        font.pixelSize: 32
        color: "white"

    }

    ShaderEffectItem {
        property variant source: ShaderEffectSource { sourceItem: textLabel; hideSource: true }
        property real wiggleAmount: 0.005
        anchors.fill: textLabel

        fragmentShader: "
        varying highp vec2 qt_TexCoord0;
        uniform sampler2D source;
        uniform highp float wiggleAmount;
        void main(void)
        {
            highp vec2 wiggledTexCoord = qt_TexCoord0;
            wiggledTexCoord.s += sin(4.0 * 3.141592653589 * wiggledTexCoord.t) * wiggleAmount;
            gl_FragColor = texture2D(source, wiggledTexCoord.st);
        }
        "
    }
}
    \endqml
    \image shaderexample.png

*/

ShaderEffectItem::ShaderEffectItem(QDeclarativeItem *parent)
    : QDeclarativeItem(parent)
    , m_program(0)
    , m_meshResolution(1, 1)
    , m_geometry(QSGGeometry::defaultAttributes_TexturedPoint2D(), 4)
    , m_blending(true)
    , m_program_dirty(true)
    , m_active(true)
    , m_respectsMatrix(false)
    , m_respectsOpacity(false)
    , m_checkedViewportUpdateMode(false)
    , m_checkedOpenGL(false)
    , m_checkedShaderPrograms(false)
    , m_hasShaderPrograms(false)
    , m_mirrored(false)
    , m_defaultVertexShader(true)
{
    setFlag(QGraphicsItem::ItemHasNoContents, false);
    connect(this, SIGNAL(visibleChanged()), this, SLOT(handleVisibilityChange()));
    m_active = isVisible();
}

ShaderEffectItem::~ShaderEffectItem()
{
    reset();
}


/*!
    \qmlproperty string ShaderEffectItem::fragmentShader
    This property holds the OpenGL fragment shader code.

    The default fragment shader is following:

    \code
        varying highp vec2 qt_TexCoord0;
        uniform sampler2D source;
        void main(void)
        {
            gl_FragColor = texture2D(source, qt_TexCoord0.st);
        }
    \endcode

*/

void ShaderEffectItem::setFragmentShader(const QString &code)
{
    if (m_fragment_code.constData() == code.constData())
        return;

    m_fragment_code = code;
    if (isComponentComplete()) {
        reset();
        updateProperties();
    }
    emit fragmentShaderChanged();
}

/*!
    \qmlproperty string ShaderEffectItem::vertexShader
    This property holds the OpenGL vertex shader code.

    The default vertex shader is following:

    \code
        uniform highp mat4 qt_ModelViewProjectionMatrix;
        attribute highp vec4 qt_Vertex;
        attribute highp vec2 qt_MultiTexCoord0;
        varying highp vec2 qt_TexCoord0;
        void main(void)
        {
            qt_TexCoord0 = qt_MultiTexCoord0;
            gl_Position = qt_ModelViewProjectionMatrix * qt_Vertex;
        }
    \endcode

*/

void ShaderEffectItem::setVertexShader(const QString &code)
{
    if (m_vertex_code.constData() == code.constData())
        return;

    m_vertex_code = code;
    m_defaultVertexShader = false;
    if (isComponentComplete()) {
        reset();
        updateProperties();
    }
    emit vertexShaderChanged();
}

/*!
    \qmlproperty bool ShaderEffectItem::blending
    This property defines whether item is drawn using blending.

    If true, the RGBA pixel output from the fragment shader is blended with
    the pixel RGBA-values already in the framebuffer.

    If false, fragment shader output is written to framebuffer as such.

    Usually drawing without blending is slightly faster, thus disabling blending
    might be a good choice when item is used as a background element.

    \note
    By default the pixel data in textures is stored in 32-bit premultiplied alpha format.
    This should be taken into account when blending or reading the pixel values
    in the fragment shader code.

    The default value is true.
*/

void ShaderEffectItem::setBlending(bool enable)
{
    if (m_blending == enable)
        return;

    m_blending = enable;
    m_changed = true;
    emit blendingChanged();
}


/*!
    \qmlproperty QSize ShaderEffectItem::meshResolution
    This property defines to how many triangles the item is divided into before its
    vertices are passed to the vertex shader.

    Triangles are defined as triangle strips and the amount of triangles can be controlled
    separately for x and y-axis.

    The default value is QSize(1,1).
*/

void ShaderEffectItem::setMeshResolution(const QSize &size)
{
    if (size == m_meshResolution)
        return;

    m_meshResolution = size;
    emit meshResolutionChanged();
    updateGeometry();
}

void ShaderEffectItem::componentComplete()
{
    updateProperties();
    QDeclarativeItem::componentComplete();
}

void ShaderEffectItem::checkViewportUpdateMode()
{
    if (!m_checkedViewportUpdateMode) {
        QGraphicsScene *s = scene();
        if (s){
            QList<QGraphicsView*> views = s->views();
            for (int i = 0; i < views.count(); i++) {
                if (views[i]->viewportUpdateMode() != QGraphicsView::FullViewportUpdate) {
                    qWarning() << "ShaderEffectItem::checkViewportUpdateMode - consider setting QGraphicsView::FullViewportUpdate mode with OpenGL!";
                }
            }
        }
        m_checkedViewportUpdateMode = true;
    }
}

void ShaderEffectItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    if (!m_active) return;

    const QGLContext *context = QGLContext::currentContext();

    if (context) {
        if (!m_checkedShaderPrograms) {
            m_hasShaderPrograms = QGLShaderProgram::hasOpenGLShaderPrograms(context);
            m_checkedShaderPrograms = true;

            if (!m_hasShaderPrograms)
                qWarning() << "ShaderEffectItem::paint - Shader programs are not supported";
        }

        if ( !m_hasShaderPrograms )
            return;

        checkViewportUpdateMode();
        painter->save();
        painter->beginNativePainting();
        QMatrix4x4 combinedMatrix = QMatrix4x4(painter->transform());
        renderEffect(painter, combinedMatrix);
        painter->endNativePainting();
        painter->restore();
    } else {
        if (!m_checkedOpenGL) {
            qWarning() << "ShaderEffectItem::paint - OpenGL not available";
            m_checkedOpenGL = true;
        }
    }
}

void ShaderEffectItem::renderEffect(QPainter *painter, const QMatrix4x4 &matrix)
{
    if (!painter || !painter->device())
        return;

    if (!m_program || !m_program->programId()) {
        // Deleted due to deactivation, to save GPU memory,
        // or invalidated due to GL context change.
        delete m_program;
        if (QGLContext::currentContext())
            m_program = new QGLShaderProgram(this);
        if (!m_program)
            qWarning() << "ShaderEffectItem::renderEffect - Creating QGLShaderProgram failed!";
    }

    if (!m_program)
        return;

    if (!m_program->isLinked() || m_program_dirty)
        updateShaderProgram();

    m_program->bind();

    QMatrix4x4 combinedMatrix;
    combinedMatrix.scale(2.0 / painter->device()->width(), -2.0 / painter->device()->height(), 1.0);
    combinedMatrix.translate(-painter->device()->width() / 2.0, -painter->device()->height() / 2.0 );
    combinedMatrix *= matrix;
    updateEffectState(combinedMatrix);

    for (int i = 0; i < m_attributeNames.size(); ++i) {
        m_program->enableAttributeArray(m_geometry.attributes()[i].position);
    }

    bindGeometry();

    // Optimization, disable depth test when we know we don't need it.
    if (m_defaultVertexShader) {
        glDepthMask(false);
        glDisable(GL_DEPTH_TEST);
    } else {
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_GREATER);
        glDepthMask(true);
#if defined(QT_OPENGL_ES)
        glClearDepthf(0);
#else
        glClearDepth(0);
#endif
        glClearColor(0, 0, 0, 0);
        glClear(GL_DEPTH_BUFFER_BIT);
    }

    if (m_blending){
        glEnable(GL_BLEND);
        glBlendFunc (GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    } else {
        glDisable(GL_BLEND);
    }

    if (m_geometry.indexCount())
        glDrawElements(m_geometry.drawingMode(), m_geometry.indexCount(), m_geometry.indexType(), m_geometry.indexData());
    else
        glDrawArrays(m_geometry.drawingMode(), 0, m_geometry.vertexCount());

    glDepthMask(false);
    glDisable(GL_DEPTH_TEST);

   for (int i = 0; i < m_attributeNames.size(); ++i)
        m_program->disableAttributeArray(m_geometry.attributes()[i].position);
}

void ShaderEffectItem::updateEffectState(const QMatrix4x4 &matrix)
{
    if (!m_program)
        return;

    for (int i = m_sources.size() - 1; i >= 0; --i) {
        const ShaderEffectItem::SourceData &source = m_sources.at(i);
        if (!source.source)
            continue;

        glActiveTexture(GL_TEXTURE0 + i);
        source.source->bind();
    }

    if (m_respectsOpacity)
        m_program->setUniformValue("qt_Opacity", static_cast<float> (effectiveOpacity()));

    if (m_respectsMatrix){
        m_program->setUniformValue("qt_ModelViewProjectionMatrix", matrix);
    }

    QSet<QByteArray>::const_iterator it;
    for (it = m_uniformNames.begin(); it != m_uniformNames.end(); ++it) {
        const QByteArray &name = *it;
        QVariant v = property(name.constData());

        switch (v.type()) {
        case QVariant::Color:
            m_program->setUniformValue(name.constData(), qvariant_cast<QColor>(v));
            break;
        case QVariant::Double:
            m_program->setUniformValue(name.constData(), (float) qvariant_cast<double>(v));
            break;
        case QVariant::Transform:
            m_program->setUniformValue(name.constData(), qvariant_cast<QTransform>(v));
            break;
        case QVariant::Int:
            m_program->setUniformValue(name.constData(), GLint(v.toInt()));
            break;
        case QVariant::Bool:
            m_program->setUniformValue(name.constData(), GLint(v.toBool()));
            break;
        case QVariant::Size:
        case QVariant::SizeF:
            m_program->setUniformValue(name.constData(), v.toSizeF());
            break;
        case QVariant::Point:
        case QVariant::PointF:
            m_program->setUniformValue(name.constData(), v.toPointF());
            break;
        case QVariant::Rect:
        case QVariant::RectF:
        {
            QRectF r = v.toRectF();
            m_program->setUniformValue(name.constData(), r.x(), r.y(), r.width(), r.height());
        }
            break;
        case QVariant::Vector3D:
            m_program->setUniformValue(name.constData(), qvariant_cast<QVector3D>(v));
            break;
        default:
            break;
        }
    }
}

static inline int size_of_type(GLenum type)
{
    static int sizes[] = {
        sizeof(char),
        sizeof(unsigned char),
        sizeof(short),
        sizeof(unsigned short),
        sizeof(int),
        sizeof(unsigned int),
        sizeof(float),
        2,
        3,
        4,
        sizeof(double)
    };
    return sizes[type - GL_BYTE];
}

void ShaderEffectItem::bindGeometry()
{
    if (!m_program)
        return;

    char const *const *attrNames = m_attributeNames.constData();
    int offset = 0;
        for (int j = 0; j < m_attributeNames.size(); ++j) {
        if (!*attrNames[j])
            continue;
        Q_ASSERT_X(j < m_geometry.attributeCount(), "ShaderEffectItem::bindGeometry()", "Geometry lacks attribute required by material");
        const QSGGeometry::Attribute &a = m_geometry.attributes()[j];
        Q_ASSERT_X(j == a.position, "ShaderEffectItem::bindGeometry()", "Geometry does not have continuous attribute positions");
#if defined(QT_OPENGL_ES_2)
        GLboolean normalize = a.type != GL_FLOAT;
#else
        GLboolean normalize = a.type != GL_FLOAT && a.type != GL_DOUBLE;
#endif
        if (normalize)
            qWarning() << "ShaderEffectItem::bindGeometry() - non supported attribute type!";

        m_program->setAttributeArray(a.position, (GLfloat*) (((char*) m_geometry.vertexData()) + offset), a.tupleSize, m_geometry.stride());
        //glVertexAttribPointer(a.position, a.tupleSize, a.type, normalize, m_geometry.stride(), (char *) m_geometry.vertexData() + offset);
        offset += a.tupleSize * size_of_type(a.type);
    }
}

void ShaderEffectItem::updateGeometry()
{
    QRectF srcRect(0, 1, 1, -1);

    if (m_mirrored)
        srcRect = QRectF(0, 0, 1, 1);

    QRectF dstRect = QRectF(0,0, width(), height());

    int vmesh = m_meshResolution.height();
    int hmesh = m_meshResolution.width();

    QSGGeometry *g = &m_geometry;
    if (vmesh == 1 && hmesh == 1) {
        if (g->vertexCount() != 4)
            g->allocate(4);
        QSGGeometry::updateTexturedRectGeometry(g, dstRect, srcRect);
        return;
    }

    g->allocate((vmesh + 1) * (hmesh + 1), vmesh * 2 * (hmesh + 2));

    QSGGeometry::TexturedPoint2D *vdata = g->vertexDataAsTexturedPoint2D();

    for (int iy = 0; iy <= vmesh; ++iy) {
        float fy = iy / float(vmesh);
        float y = float(dstRect.top()) + fy * float(dstRect.height());
        float ty = float(srcRect.top()) + fy * float(srcRect.height());
        for (int ix = 0; ix <= hmesh; ++ix) {
            float fx = ix / float(hmesh);
            vdata->x = float(dstRect.left()) + fx * float(dstRect.width());
            vdata->y = y;
            vdata->tx = float(srcRect.left()) + fx * float(srcRect.width());
            vdata->ty = ty;
            ++vdata;
        }
    }

    quint16 *indices = (quint16 *)g->indexDataAsUShort();
    int i = 0;
    for (int iy = 0; iy < vmesh; ++iy) {
        *(indices++) = i + hmesh + 1;
        for (int ix = 0; ix <= hmesh; ++ix, ++i) {
            *(indices++) = i + hmesh + 1;
            *(indices++) = i;
        }
        *(indices++) = i - 1;
    }
}

void ShaderEffectItem::setActive(bool enable)
{
    if (m_active == enable)
        return;

    if (m_active) {
        for (int i = 0; i < m_sources.size(); ++i) {
            ShaderEffectSource *source = m_sources.at(i).source;
            if (!source)
                continue;
            disconnect(source, SIGNAL(repaintRequired()), this, SLOT(markDirty()));
            source->derefFromEffectItem();
        }
    }

    m_active = enable;

    if (m_active) {
        for (int i = 0; i < m_sources.size(); ++i) {
            ShaderEffectSource *source = m_sources.at(i).source;
            if (!source)
                continue;
            source->refFromEffectItem();
            connect(source, SIGNAL(repaintRequired()), this, SLOT(markDirty()));
        }
    }

    // QGLShaderProgram is deleted when not active (to minimize GPU memory usage).
    if (!m_active && m_program) {
        delete m_program;
        m_program = 0;
    }

    emit activeChanged();
    markDirty();
}

void ShaderEffectItem::preprocess()
{
    for (int i = 0; i < m_sources.size(); ++i) {
        ShaderEffectSource *source = m_sources.at(i).source;
        if (source)
            source->updateBackbuffer();
    }
}

void ShaderEffectItem::geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry)
{
    if (newGeometry.size() != oldGeometry.size())
        updateGeometry();
    QDeclarativeItem::geometryChanged(newGeometry, oldGeometry);
}

void ShaderEffectItem::changeSource(int index)
{
    Q_ASSERT(index >= 0 && index < m_sources.size());
    QVariant v = property(m_sources.at(index).name.constData());
    setSource(v, index);
}

void ShaderEffectItem::markDirty() {
    update();
}

void ShaderEffectItem::setSource(const QVariant &var, int index)
{
    Q_ASSERT(index >= 0 && index < m_sources.size());

    SourceData &source = m_sources[index];

    source.source = 0;
    source.item = 0;
    if (var.isNull()) {
        return;
    } else if (!qVariantCanConvert<QObject *>(var)) {
        qWarning("Could not assign source of type '%s' to property '%s'.", var.typeName(), source.name.constData());
        return;
    }

    QObject *obj = qVariantValue<QObject *>(var);

    source.source = qobject_cast<ShaderEffectSource *>(obj);
    source.item = qobject_cast<QDeclarativeItem *>(obj);

    if (!source.item)
        qWarning("Could not assign property '%s', did not implement QDeclarativeItem.", source.name.constData());

    if (!source.source)
        qWarning("Could not assign property '%s', did not implement ShaderEffectSource.", source.name.constData());

    // TODO: Find better solution.
    // 'source.item' needs a canvas to get a scenegraph node.
    // The easiest way to make sure it gets a canvas is to
    // make it a part of the same item tree as 'this'.
    if (source.item && source.item->parentItem() == 0) {
        source.item->setParentItem(this);
        // Unlike in scenegraph, we cannot set item invisible here because qgraphicsview would optimize it away.
    }

    // Unlike in scenegraph, ref counting is used to optimize memory consumption. Sources themself may free fbos when not referenced.
    if (m_active && source.source) {
        source.source->refFromEffectItem();
        connect(source.source, SIGNAL(repaintRequired()), this, SLOT(markDirty()));
     }
}

void ShaderEffectItem::disconnectPropertySignals()
{
    disconnect(this, 0, this, SLOT(markDirty()));
    for (int i = 0; i < m_sources.size(); ++i) {
        SourceData &source = m_sources[i];
        disconnect(this, 0, source.mapper, 0);
        disconnect(source.mapper, 0, this, 0);
    }
}

void ShaderEffectItem::connectPropertySignals()
{
    QSet<QByteArray>::const_iterator it;
    for (it = m_uniformNames.begin(); it != m_uniformNames.end(); ++it) {
        int pi = metaObject()->indexOfProperty(it->constData());
        if (pi >= 0) {
            QMetaProperty mp = metaObject()->property(pi);
            if (!mp.hasNotifySignal())
                qWarning("ShaderEffectItem: property '%s' does not have notification method!", it->constData());
            QByteArray signalName("2");
            signalName.append(mp.notifySignal().signature());
            connect(this, signalName, this, SLOT(markDirty()));
        } else {
            qWarning("ShaderEffectItem: '%s' does not have a matching property!", it->constData());
        }
    }
    for (int i = 0; i < m_sources.size(); ++i) {
        SourceData &source = m_sources[i];
        int pi = metaObject()->indexOfProperty(source.name.constData());
        if (pi >= 0) {
            QMetaProperty mp = metaObject()->property(pi);
            QByteArray signalName("2");
            signalName.append(mp.notifySignal().signature());
            connect(this, signalName, source.mapper, SLOT(map()));
            source.mapper->setMapping(this, i);
            connect(source.mapper, SIGNAL(mapped(int)), this, SLOT(changeSource(int)));
        } else {
            qWarning("ShaderEffectItem: '%s' does not have a matching source!", source.name.constData());
        }
    }
}

void ShaderEffectItem::reset()
{
    disconnectPropertySignals();

    if (m_program)
        m_program->removeAllShaders();

    m_attributeNames.clear();
    m_uniformNames.clear();
    for (int i = 0; i < m_sources.size(); ++i) {
        const SourceData &source = m_sources.at(i);
        if (m_active && source.source)
            source.source->derefFromEffectItem();
        delete source.mapper;
    }

    m_sources.clear();
    m_program_dirty = true;
}

void ShaderEffectItem::updateProperties()
{
    QString vertexCode = m_vertex_code;
    QString fragmentCode = m_fragment_code;

    if (vertexCode.isEmpty())
        vertexCode = qt_default_vertex_code;

    if (fragmentCode.isEmpty())
        fragmentCode = qt_default_fragment_code;

    lookThroughShaderCode(vertexCode);
    lookThroughShaderCode(fragmentCode);

    if (!m_attributeNames.contains(qt_postion_attribute_name))
        qWarning("ShaderEffectItem: Missing reference to \'%s\'.", qt_postion_attribute_name);
    if (!m_attributeNames.contains(qt_texcoord_attribute_name))
        qWarning("ShaderEffectItem: Missing reference to \'%s\'.", qt_texcoord_attribute_name);
    if (!m_respectsMatrix)
        qWarning("ShaderEffectItem: Missing reference to \'qt_ModelViewProjectionMatrix\'.");

    for (int i = 0; i < m_sources.size(); ++i) {
        QVariant v = property(m_sources.at(i).name);
        setSource(v, i); // Property exists.
    }

    connectPropertySignals();
}

void ShaderEffectItem::updateShaderProgram()
{
    if (!m_program)
        return;

    QString vertexCode = m_vertex_code;
    QString fragmentCode = m_fragment_code;

    if (vertexCode.isEmpty())
        vertexCode = QString::fromLatin1(qt_default_vertex_code);

    if (fragmentCode.isEmpty())
        fragmentCode = QString::fromLatin1(qt_default_fragment_code);

    m_program->addShaderFromSourceCode(QGLShader::Vertex, vertexCode);
    m_program->addShaderFromSourceCode(QGLShader::Fragment, fragmentCode);

    for (int i = 0; i < m_attributeNames.size(); ++i) {
        m_program->bindAttributeLocation(m_attributeNames.at(i), m_geometry.attributes()[i].position);
    }

    if (!m_program->link()) {
        qWarning("ShaderEffectItem: Shader compilation failed:");
        qWarning() << m_program->log();
    }

    if (!m_attributeNames.contains(qt_postion_attribute_name))
        qWarning("ShaderEffectItem: Missing reference to \'qt_Vertex\'.");
    if (!m_attributeNames.contains(qt_texcoord_attribute_name))
        qWarning("ShaderEffectItem: Missing reference to \'qt_MultiTexCoord0\'.");
    if (!m_respectsMatrix)
        qWarning("ShaderEffectItem: Missing reference to \'qt_ModelViewProjectionMatrix\'.");

    if (m_program->isLinked()) {
        m_program->bind();
        for (int i = 0; i < m_sources.size(); ++i)
            m_program->setUniformValue(m_sources.at(i).name.constData(), (GLint) i);
    }

    m_program_dirty = false;
}

void ShaderEffectItem::lookThroughShaderCode(const QString &code)
{
    // Regexp for matching attributes and uniforms.
    // In human readable form: attribute|uniform [lowp|mediump|highp] <type> <name>
    static QRegExp re(QLatin1String("\\b(attribute|uniform)\\b\\s*\\b(?:lowp|mediump|highp)?\\b\\s*\\b(\\w+)\\b\\s*\\b(\\w+)"));
    Q_ASSERT(re.isValid());

    int pos = -1;

    //QString wideCode = QString::fromLatin1(code.constData(), code.size());
    QString wideCode = code;

    while ((pos = re.indexIn(wideCode, pos + 1)) != -1) {
        QByteArray decl = re.cap(1).toLatin1(); // uniform or attribute
        QByteArray type = re.cap(2).toLatin1(); // type
        QByteArray name = re.cap(3).toLatin1(); // variable name

        if (decl == "attribute") {
            if (name == qt_postion_attribute_name) {
                m_attributeNames.insert(0, qt_postion_attribute_name);
            } else if (name == "qt_MultiTexCoord0") {
                if (m_attributeNames.at(0) == 0) {
                    m_attributeNames.insert(0, qt_emptyAttributeName);
                }
                m_attributeNames.insert(1, qt_texcoord_attribute_name);
            } else {
                // TODO: Support user defined attributes.
                qWarning("ShaderEffectItem: Attribute \'%s\' not recognized.", name.constData());
            }
        } else {
            Q_ASSERT(decl == "uniform");

            if (name == "qt_ModelViewProjectionMatrix") {
                m_respectsMatrix = true;
            } else if (name == "qt_Opacity") {
                m_respectsOpacity = true;
            } else {
                m_uniformNames.insert(name);
                if (type == "sampler2D") {
                    SourceData d;
                    d.mapper = new QSignalMapper;
                    d.source = 0;
                    d.name = name;
                    d.item = 0;
                    m_sources.append(d);
                }
            }
        }
    }
}

void ShaderEffectItem::handleVisibilityChange()
{
    setActive(isVisible());
}
