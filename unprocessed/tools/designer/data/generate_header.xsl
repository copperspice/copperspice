<!DOCTYPE xsl:stylesheet [
     <!ENTITY endl "&#10;">
]>
<xsl:stylesheet version="1.0"
                xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
                xmlns:xs="http://www.w3.org/2001/XMLSchema">

    <xsl:output method="text"/>

    <xsl:include href="generate_shared.xsl"/>

<!-- Forward declaration -->

    <xsl:template name="class-forward-declaration">
        <xsl:param name="node"/>
        <xsl:variable name="name" select="concat('Dom', $node/attribute::name)"/>

        <xsl:text>class </xsl:text>
        <xsl:value-of select="$name"/>
        <xsl:text>;&endl;</xsl:text>
    </xsl:template>

<!-- Class declaration: child element accessors -->

    <xsl:template name="child-element-accessors">
        <xsl:param name="node"/>

        <xsl:variable name="isChoice" select="name($node)='xs:choice'"/>

        <xsl:if test="$isChoice">
            <xsl:text>    enum Kind { Unknown = 0</xsl:text>
            <xsl:for-each select="$node/xs:element">
                <xsl:variable name="camel-case-name">
                    <xsl:call-template name="camel-case">
                        <xsl:with-param name="text" select="@name"/>
                    </xsl:call-template>
                </xsl:variable>

                <xsl:variable name="cap-name">
                    <xsl:call-template name="cap-first-char">
                        <xsl:with-param name="text" select="$camel-case-name"/>
                    </xsl:call-template>
                </xsl:variable>
                <xsl:text>, </xsl:text>
                <xsl:value-of select="$cap-name"/>
            </xsl:for-each>
            <xsl:text> };&endl;</xsl:text>
            <xsl:text>    inline Kind kind() const { return m_kind; }&endl;&endl;</xsl:text>
        </xsl:if>

        <xsl:for-each select="$node/xs:element">
            <xsl:variable name="array" select="@maxOccurs='unbounded'"/>
            <xsl:variable name="camel-case-name">
                <xsl:call-template name="camel-case">
                    <xsl:with-param name="text" select="@name"/>
                </xsl:call-template>
            </xsl:variable>
            <xsl:variable name="cap-name">
                <xsl:call-template name="cap-first-char">
                    <xsl:with-param name="text" select="$camel-case-name"/>
                </xsl:call-template>
            </xsl:variable>
            <xsl:variable name="return-cpp-type">
                <xsl:call-template name="xs-type-to-cpp-return-type">
                    <xsl:with-param name="xs-type" select="@type"/>
                    <xsl:with-param name="array" select="$array"/>
                </xsl:call-template>
            </xsl:variable>
            <xsl:variable name="argument-cpp-type">
                <xsl:call-template name="xs-type-to-cpp-argument-type">
                    <xsl:with-param name="xs-type" select="@type"/>
                    <xsl:with-param name="array" select="$array"/>
                </xsl:call-template>
            </xsl:variable>
            <xsl:variable name="xs-type-cat">
                <xsl:call-template name="xs-type-category">
                    <xsl:with-param name="xs-type" select="@type"/>
                    <xsl:with-param name="array" select="$array"/>
                </xsl:call-template>
            </xsl:variable>

            <xsl:text>    inline </xsl:text>
            <xsl:value-of select="$return-cpp-type"/>
            <xsl:text> element</xsl:text>
            <xsl:value-of select="$cap-name"/>
            <xsl:text>() const { return m_</xsl:text>
            <xsl:value-of select="$camel-case-name"/>
            <xsl:text>; }&endl;</xsl:text>

            <xsl:if test="$xs-type-cat = 'pointer'">
                <xsl:text>    </xsl:text>
                <xsl:value-of select="$return-cpp-type"/>
                <xsl:text> takeElement</xsl:text>
                <xsl:value-of select="$cap-name"/>
                <xsl:text>();&endl;</xsl:text>
            </xsl:if>

            <xsl:text>    void setElement</xsl:text>
            <xsl:value-of select="$cap-name"/>
            <xsl:text>(</xsl:text>
            <xsl:value-of select="$argument-cpp-type"/>
            <xsl:text> a);&endl;</xsl:text>

            <xsl:if test="not($isChoice) and not(@maxOccurs='unbounded')">
                <xsl:text>    inline bool hasElement</xsl:text>
                <xsl:value-of select="$cap-name"/>
                <xsl:text>() const { return m_children &amp; </xsl:text>
                <xsl:value-of select="$cap-name"/>
                <xsl:text>; }&endl;</xsl:text>
                <xsl:text>    void clearElement</xsl:text>
                <xsl:value-of select="$cap-name"/>
                <xsl:text>();&endl;</xsl:text>
            </xsl:if>
            <xsl:text>&endl;</xsl:text>

        </xsl:for-each>
    </xsl:template>

<!-- Class declaration: child element data -->

    <xsl:template name="child-element-data">
        <xsl:param name="node"/>

        <xsl:variable name="isChoice" select="$node[name()='xs:choice']"/>

        <xsl:for-each select="$node/xs:element">
            <xsl:variable name="camel-case-name">
                <xsl:call-template name="camel-case">
                    <xsl:with-param name="text" select="@name"/>
                </xsl:call-template>
            </xsl:variable>
            <xsl:variable name="cpp-type">
                <xsl:call-template name="xs-type-to-cpp-return-type">
                    <xsl:with-param name="xs-type" select="@type"/>
                    <xsl:with-param name="array" select="@maxOccurs='unbounded'"/>
                </xsl:call-template>
            </xsl:variable>
            <xsl:text>    </xsl:text>
            <xsl:value-of select="$cpp-type"/>
            <xsl:text> m_</xsl:text>
            <xsl:value-of select="$camel-case-name"/>
            <xsl:text>;&endl;</xsl:text>
        </xsl:for-each>

        <xsl:if test="not($isChoice) and not(@macOccurs='unbounded')">
            <xsl:text>    enum Child {&endl;</xsl:text>
            <xsl:for-each select="$node/xs:element">
            <xsl:variable name="camel-case-name">
                    <xsl:call-template name="camel-case">
                        <xsl:with-param name="text" select="@name"/>
                    </xsl:call-template>
                </xsl:variable>

                <xsl:text>        </xsl:text>
                <xsl:call-template name="cap-first-char">
                    <xsl:with-param name="text" select="$camel-case-name"/>
                </xsl:call-template>
                <xsl:text> = </xsl:text>
                <xsl:call-template name="powers-of-two">
                    <xsl:with-param name="num" select="position() - 1"/>
                </xsl:call-template>
                <xsl:if test="position()!=last()">
                    <xsl:text>,</xsl:text>
                </xsl:if>
                <xsl:text>&endl;</xsl:text>
                                     
            </xsl:for-each>
            <xsl:text>    };&endl;</xsl:text>
        </xsl:if>
    </xsl:template>

<!-- Class declaration: attribute accessors -->

    <xsl:template name="attribute-accessors">
        <xsl:param name="node"/>

        <xsl:for-each select="$node/xs:attribute">
            <xsl:variable name="camel-case-name">
                <xsl:call-template name="camel-case">
                    <xsl:with-param name="text" select="@name"/>
                </xsl:call-template>
            </xsl:variable>
            <xsl:variable name="cap-name">
                <xsl:call-template name="cap-first-char">
                    <xsl:with-param name="text" select="$camel-case-name"/>
                </xsl:call-template>
            </xsl:variable>
            <xsl:variable name="cpp-return-type">
                <xsl:call-template name="xs-type-to-cpp-return-type">
                    <xsl:with-param name="xs-type" select="@type"/>
                </xsl:call-template>
            </xsl:variable>
            <xsl:variable name="cpp-argument-type">
                <xsl:call-template name="xs-type-to-cpp-argument-type">
                    <xsl:with-param name="xs-type" select="@type"/>
                </xsl:call-template>
            </xsl:variable>

            <xsl:text>    inline bool hasAttribute</xsl:text>
            <xsl:value-of select="$cap-name"/>
            <xsl:text>() const { return m_has_attr_</xsl:text>
            <xsl:value-of select="$camel-case-name"/>
            <xsl:text>; }&endl;</xsl:text>

            <xsl:text>    inline </xsl:text>
            <xsl:value-of select="$cpp-return-type"/>
            <xsl:text> attribute</xsl:text>
            <xsl:value-of select="$cap-name"/>
            <xsl:text>() const { return m_attr_</xsl:text>
            <xsl:value-of select="$camel-case-name"/>
            <xsl:text>; }&endl;</xsl:text>

            <xsl:text>    inline void setAttribute</xsl:text>
            <xsl:value-of select="$cap-name"/>
            <xsl:text>(</xsl:text>
            <xsl:value-of select="$cpp-argument-type"/>
            <xsl:text> a) { m_attr_</xsl:text>
            <xsl:value-of select="$camel-case-name"/>
            <xsl:text> = a; m_has_attr_</xsl:text>
            <xsl:value-of select="$camel-case-name"/>
            <xsl:text> = true; }&endl;</xsl:text>

            <xsl:text>    inline void clearAttribute</xsl:text>
            <xsl:value-of select="$cap-name"/>
            <xsl:text>() { m_has_attr_</xsl:text>
            <xsl:value-of select="$camel-case-name"/>
            <xsl:text> = false; }&endl;&endl;</xsl:text>
        </xsl:for-each>
    </xsl:template>

<!-- Class declaration -->

    <xsl:template name="class-declaration">
        <xsl:param name="node"/>
        <xsl:variable name="name" select="concat('Dom', $node/@name)"/>
<!--    <xsl:variable name="hasText" select="$node[@mixed='true']"/>-->
        <xsl:variable name="hasText" select="true()"/>

        <xsl:text>class QDESIGNER_UILIB_EXPORT </xsl:text>
        <xsl:value-of select="$name"/>
        <xsl:text> {&endl;</xsl:text>
        <xsl:text>public:&endl;</xsl:text>
        <xsl:text>    </xsl:text>
        <xsl:value-of select="$name"/>
        <xsl:text>();&endl;</xsl:text>
        <xsl:text>    ~</xsl:text>
        <xsl:value-of select="$name"/>
        <xsl:text>();&endl;&endl;</xsl:text>

        <xsl:text>    void read(QXmlStreamReader &amp;reader);&endl;</xsl:text>
        <xsl:text>#ifdef QUILOADER_QDOM_READ&endl;</xsl:text>
        <xsl:text>    void read(const QDomElement &amp;node);&endl;</xsl:text>
        <xsl:text>#endif&endl;</xsl:text>
        <xsl:text>    void write(QXmlStreamWriter &amp;writer, const QString &amp;tagName = QString()) const;&endl;</xsl:text>

        <xsl:if test="$hasText">
            <xsl:text>    inline QString text() const { return m_text; }&endl;</xsl:text>
            <xsl:text>    inline void setText(const QString &amp;s) { m_text = s; }&endl;</xsl:text>
        </xsl:if>

        <xsl:text>&endl;</xsl:text>

        <xsl:text>    // attribute accessors&endl;</xsl:text>
        <xsl:call-template name="attribute-accessors">
            <xsl:with-param name="node" select="$node"/>
        </xsl:call-template>

        <xsl:text>    // child element accessors&endl;</xsl:text>

        <xsl:for-each select="$node/xs:sequence | $node/xs:choice | $node/xs:all">
            <xsl:call-template name="child-element-accessors">
                <xsl:with-param name="node" select="."/>
            </xsl:call-template>
        </xsl:for-each>

        <xsl:text>private:&endl;</xsl:text>

        <xsl:if test="$hasText">
            <xsl:text>    QString m_text;&endl;</xsl:text>
        </xsl:if>

        <xsl:text>    void clear(bool clear_all = true);&endl;&endl;</xsl:text>

        <xsl:text>    // attribute data&endl;</xsl:text>
        <xsl:for-each select="$node/xs:attribute">
            <xsl:variable name="camel-case-name">
                <xsl:call-template name="camel-case">
                    <xsl:with-param name="text" select="@name"/>
                </xsl:call-template>
            </xsl:variable>
            <xsl:variable name="cpp-type">
                <xsl:call-template name="xs-type-to-cpp-type">
                    <xsl:with-param name="xs-type" select="@type"/>
                </xsl:call-template>
            </xsl:variable>
            <xsl:text>    </xsl:text>
            <xsl:value-of select="$cpp-type"/>
            <xsl:text> m_attr_</xsl:text>
            <xsl:value-of select="$camel-case-name"/>
            <xsl:text>;&endl;</xsl:text>
            <xsl:text>    bool m_has_attr_</xsl:text>
            <xsl:value-of select="$camel-case-name"/>
            <xsl:text>;&endl;&endl;</xsl:text>
        </xsl:for-each>

        <xsl:text>    // child element data&endl;</xsl:text>
        <xsl:if test="boolean($node/xs:choice)">
            <xsl:text>    Kind m_kind;&endl;</xsl:text>
        </xsl:if>
        <xsl:if test="not($node/xs:choice)">
            <!-- TODO: if there are no elements with maxOccurs='1', m_children is never used-->
            <xsl:text>    uint m_children;&endl;</xsl:text>
        </xsl:if>
        <xsl:for-each select="$node/xs:sequence | $node/xs:choice | $node/xs:all">
            <xsl:call-template name="child-element-data">
                <xsl:with-param name="node" select="."/>
            </xsl:call-template>
        </xsl:for-each>

        <xsl:text>&endl;</xsl:text>
        <xsl:text>    </xsl:text>
        <xsl:value-of select="$name"/>
        <xsl:text>(const </xsl:text>
        <xsl:value-of select="$name"/>
        <xsl:text> &amp;other);&endl;</xsl:text>
        <xsl:text>    void operator = (const </xsl:text>
        <xsl:value-of select="$name"/>
        <xsl:text>&amp;other);&endl;</xsl:text>

        <xsl:text>};&endl;&endl;</xsl:text>
    </xsl:template>

<!-- Root -->

    <xsl:template match="xs:schema">

<xsl:text>/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the tools applications of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this
** file. Please review the following information to ensure the GNU Lesser
** General Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights. These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU General
** Public License version 3.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of this
** file. Please review the following information to ensure the GNU General
** Public License version 3.0 requirements will be met:
** http://www.gnu.org/copyleft/gpl.html.
**
** Other Usage
** Alternatively, this file may be used in accordance with the terms and
** conditions contained in a signed written agreement between you and Nokia.
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of Qt Designer.  This header
// file may change from version to version without notice, or even be removed.
//
// We mean it.
//

// THIS FILE IS AUTOMATICALLY GENERATED

#ifndef UI4_H
#define UI4_H

#include &lt;QtCore/QList&gt;
#include &lt;QtCore/QString&gt;
#include &lt;QtCore/QStringList&gt;
#include &lt;QtCore/QXmlStreamReader&gt;
#include &lt;QtCore/QXmlStreamWriter&gt;
#include &lt;QtCore/qglobal.h&gt;

#if defined(QT_UIC3)
 #define QUILOADER_QDOM_READ
#endif

QT_BEGIN_NAMESPACE

#ifdef QUILOADER_QDOM_READ
 class QDomElement;
#endif


#define QDESIGNER_UILIB_EXTERN Q_DECL_EXPORT
#define QDESIGNER_UILIB_IMPORT Q_DECL_IMPORT

#if defined(QT_DESIGNER_STATIC) || defined(QT_UIC) || defined(QT_UIC3)
#  define QDESIGNER_UILIB_EXPORT
#elif defined(QDESIGNER_UILIB_LIBRARY)
#  define QDESIGNER_UILIB_EXPORT QDESIGNER_UILIB_EXTERN
#else
#  define QDESIGNER_UILIB_EXPORT QDESIGNER_UILIB_IMPORT
#endif

#ifndef QDESIGNER_UILIB_EXPORT
#    define QDESIGNER_UILIB_EXPORT
#endif

#ifdef QFORMINTERNAL_NAMESPACE
namespace QFormInternal
{
#endif

</xsl:text>

        <xsl:text>&endl;</xsl:text>
        <xsl:text>/*******************************************************************************&endl;</xsl:text>
        <xsl:text>** Forward declarations&endl;</xsl:text>
        <xsl:text>*/&endl;&endl;</xsl:text>

        <xsl:for-each select="xs:complexType">
            <xsl:call-template name="class-forward-declaration">
                <xsl:with-param name="node" select="."/>
            </xsl:call-template>
        </xsl:for-each>

        <xsl:text>&endl;</xsl:text>
        <xsl:text>/*******************************************************************************&endl;</xsl:text>
        <xsl:text>** Declarations&endl;</xsl:text>
        <xsl:text>*/&endl;&endl;</xsl:text>

        <xsl:for-each select="xs:complexType">
            <xsl:call-template name="class-declaration">
                <xsl:with-param name="node" select="."/>
            </xsl:call-template>
        </xsl:for-each>
        <xsl:text>
#ifdef QFORMINTERNAL_NAMESPACE
}
#endif

QT_END_NAMESPACE

#endif // UI4_H
</xsl:text>
    </xsl:template>
</xsl:stylesheet>
