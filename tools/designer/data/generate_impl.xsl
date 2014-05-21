<!DOCTYPE xsl:stylesheet [
     <!ENTITY endl "&#10;">
]>
<xsl:stylesheet version="1.0"
                xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
                xmlns:xs="http://www.w3.org/2001/XMLSchema">

    <xsl:output method="text"/>

    <xsl:include href="generate_shared.xsl"/>

<!-- Implementation: constructor -->

    <xsl:template name="ctor-init-attributes">
        <xsl:param name="node"/>
        <xsl:for-each select="$node/xs:attribute">
            <xsl:variable name="camel-case-name">
                <xsl:call-template name="camel-case">
                    <xsl:with-param name="text" select="@name"/>
                </xsl:call-template>
            </xsl:variable>
            <xsl:text>    m_has_attr_</xsl:text>
            <xsl:value-of select="$camel-case-name"/>
            <xsl:text> = false;&endl;</xsl:text>
            <xsl:choose>
                <xsl:when test="@type = 'xs:integer'">
                    <xsl:text>    m_attr_</xsl:text>
                    <xsl:value-of select="$camel-case-name"/>
                    <xsl:text> = 0;&endl;</xsl:text>
                </xsl:when>
                <xsl:when test="@type = 'xs:double'">
                    <xsl:text>    m_attr_</xsl:text>
                    <xsl:value-of select="$camel-case-name"/>
                    <xsl:text> = 0.0;&endl;</xsl:text>
                </xsl:when>
                <xsl:when test="@type = 'xs:float'">
                    <xsl:text>    m_attr_</xsl:text>
                    <xsl:value-of select="$camel-case-name"/>
                    <xsl:text> = 0.0;&endl;</xsl:text>
                </xsl:when>
                <xsl:when test="@type = 'xs:boolean'">
                    <xsl:text>    m_attr_</xsl:text>
                    <xsl:value-of select="$camel-case-name"/>
                    <xsl:text> = false;&endl;</xsl:text>
                </xsl:when>
            </xsl:choose>
        </xsl:for-each>
    </xsl:template>

    <xsl:template name="ctor-init-child-elements">
        <xsl:param name="node"/>
        <xsl:for-each select="$node/xs:element">
            <xsl:variable name="array" select="@maxOccurs='unbounded'"/>
            <xsl:if test="not($array)">
                <xsl:variable name="cpp-type">
                    <xsl:call-template name="xs-type-to-cpp-type">
                        <xsl:with-param name="xs-type" select="@type"/>
                        <xsl:with-param name="array" select="$array"/>
                    </xsl:call-template>
                </xsl:variable>
                <xsl:variable name="camel-case-name">
                    <xsl:call-template name="camel-case">
                         <xsl:with-param name="text" select="@name"/>
                    </xsl:call-template>
                </xsl:variable>
                <xsl:choose>
                    <xsl:when test="@type = 'xs:integer'">
                        <xsl:text>    m_</xsl:text>
                        <xsl:value-of select="$camel-case-name"/>
                        <xsl:text> = 0;&endl;</xsl:text>
                    </xsl:when>
                    <xsl:when test="@type = 'xs:float'">
                         <xsl:text>    m_</xsl:text>
                         <xsl:value-of select="$camel-case-name"/>
                         <xsl:text> = 0.0;&endl;</xsl:text>
                    </xsl:when>
                    <xsl:when test="@type = 'xs:boolean'">
                        <xsl:text>    m_</xsl:text>
                        <xsl:value-of select="$camel-case-name"/>
                        <xsl:text> = false;&endl;</xsl:text>
                    </xsl:when>
                    <xsl:when test="@type = 'xs:string'"></xsl:when>
                    <xsl:otherwise>
                        <xsl:text>    m_</xsl:text>
                        <xsl:value-of select="$camel-case-name"/>
                        <xsl:text> = 0;&endl;</xsl:text>
                    </xsl:otherwise>
                </xsl:choose>
            </xsl:if>
        </xsl:for-each>
    </xsl:template>

    <xsl:template name="ctor-init-members">
        <xsl:param name="node"/>

        <xsl:if test="boolean($node/xs:choice)">
            <xsl:text>    m_kind = Unknown;&endl;&endl;</xsl:text>
        </xsl:if>

        <xsl:if test="not($node/xs:choice)">
            <xsl:text>    m_children = 0;&endl;</xsl:text>
        </xsl:if>

        <xsl:call-template name="ctor-init-attributes">
            <xsl:with-param name="node" select="."/>
        </xsl:call-template>

        <xsl:if test="$node[@mixed='true']">
            <xsl:text>    m_text = QLatin1String("");&endl;</xsl:text>
        </xsl:if>

        <xsl:for-each select="$node//xs:sequence | $node//xs:choice | $node//xs:all">
            <xsl:call-template name="ctor-init-child-elements">
                <xsl:with-param name="node" select="."/>
            </xsl:call-template>
        </xsl:for-each>
    </xsl:template>

    <xsl:template name="ctor-impl">
        <xsl:param name="node"/>
        <xsl:variable name="name" select="concat('Dom', $node/@name)"/>

        <xsl:value-of select="$name"/>
        <xsl:text>::</xsl:text>
        <xsl:value-of select="$name"/>
        <xsl:text>()&endl;</xsl:text>
        <xsl:text>{&endl;</xsl:text>
        <xsl:call-template name="ctor-init-members">
            <xsl:with-param name="node" select="$node"/>
        </xsl:call-template>
        <xsl:text>}&endl;&endl;</xsl:text>
    </xsl:template>

<!-- Implementation: destructor -->

    <xsl:template name="dtor-delete-members">
        <xsl:param name="node"/>

        <xsl:for-each select="$node/xs:element">
            <xsl:variable name="camel-case-name">
                <xsl:call-template name="camel-case">
                    <xsl:with-param name="text" select="@name"/>
                </xsl:call-template>
            </xsl:variable>
            <xsl:variable name="xs-type-cat">
                <xsl:call-template name="xs-type-category">
                    <xsl:with-param name="xs-type" select="@type"/>
                </xsl:call-template>
            </xsl:variable>
            <xsl:choose>
                <xsl:when test="@maxOccurs='unbounded'">
                    <xsl:if test="$xs-type-cat = 'pointer'">
                        <xsl:text>    qDeleteAll(m_</xsl:text>
                        <xsl:value-of select="$camel-case-name"/>
                        <xsl:text>);&endl;</xsl:text>
                    </xsl:if>
                    <xsl:text>    m_</xsl:text>
                    <xsl:value-of select="$camel-case-name"/>
                    <xsl:text>.clear();&endl;</xsl:text>
                </xsl:when>
                <xsl:otherwise>
                    <xsl:if test="$xs-type-cat = 'pointer'">
                        <xsl:text>    delete m_</xsl:text>
                        <xsl:value-of select="$camel-case-name"/>
                        <xsl:text>;&endl;</xsl:text>
                    </xsl:if>
                </xsl:otherwise>
            </xsl:choose>
        </xsl:for-each>
    </xsl:template>

    <xsl:template name="dtor-impl">
        <xsl:param name="node"/>
        <xsl:variable name="name" select="concat('Dom', $node/@name)"/>

        <xsl:value-of select="$name"/>
        <xsl:text>::~</xsl:text>
        <xsl:value-of select="$name"/>
        <xsl:text>()&endl;</xsl:text>
        <xsl:text>{&endl;</xsl:text>

        <xsl:for-each select="$node//xs:sequence | $node//xs:choice | $node//xs:all">
            <xsl:call-template name="dtor-delete-members">
                <xsl:with-param name="node" select="."/>
            </xsl:call-template>
        </xsl:for-each>

        <xsl:text>}&endl;&endl;</xsl:text>
    </xsl:template>

<!-- Implementation: clear() -->

    <xsl:template name="clear-impl">
        <xsl:param name="node"/>
        <xsl:variable name="name" select="concat('Dom', $node/@name)"/>

        <xsl:text>void </xsl:text><xsl:value-of select="$name"/>
        <xsl:text>::clear(bool clear_all)&endl;</xsl:text>
        <xsl:text>{&endl;</xsl:text>

        <xsl:for-each select="$node//xs:sequence | $node//xs:choice | $node//xs:all">
            <xsl:call-template name="dtor-delete-members">
                <xsl:with-param name="node" select="."/>
            </xsl:call-template>
        </xsl:for-each>

        <xsl:text>&endl;    if (clear_all) {&endl;</xsl:text>

        <xsl:choose>
            <xsl:when test="$node[@mixed='true']">
                <xsl:text>    m_text = QLatin1String("");&endl;</xsl:text>
            </xsl:when>
            <xsl:otherwise>
                <xsl:text>    m_text.clear();&endl;</xsl:text>
            </xsl:otherwise>
        </xsl:choose>

        <xsl:call-template name="ctor-init-attributes">
            <xsl:with-param name="node" select="."/>
        </xsl:call-template>
        <xsl:text>    }&endl;&endl;</xsl:text>

        <xsl:if test="boolean($node/xs:choice)">
            <xsl:text>    m_kind = Unknown;&endl;&endl;</xsl:text>
        </xsl:if>

        <xsl:if test="not($node/xs:choice)">
            <xsl:text>    m_children = 0;&endl;</xsl:text>
        </xsl:if>

        <xsl:for-each select="$node//xs:sequence | $node//xs:choice | $node//xs:all">
            <xsl:call-template name="ctor-init-child-elements">
                <xsl:with-param name="node" select="."/>
            </xsl:call-template>
        </xsl:for-each>

        <xsl:text>}&endl;&endl;</xsl:text>

    </xsl:template>

    <!-- Format a string constant as QString(QLatin1Char('X')) or QLatin1String("foo"), respectively -->
    <xsl:template name="string-constant">
    <xsl:param name="literal"/>
        <xsl:choose>
            <xsl:when test="string-length($literal) &lt; 2">
                  <xsl:text>QString(QLatin1Char('</xsl:text>
                <xsl:value-of select="$literal"/>
                <xsl:text>'))</xsl:text>
            </xsl:when>
            <xsl:otherwise>
                <xsl:text>QLatin1String("</xsl:text>
                    <xsl:value-of select="$literal"/>
                <xsl:text>")</xsl:text>
           </xsl:otherwise>
       </xsl:choose>
    </xsl:template>

<!-- Implementation: read(QXmlStreamReader) -->

    <xsl:template name="read-impl-load-attributes">
        <xsl:param name="node"/>

        <xsl:if test="$node/xs:attribute">
            <xsl:text>&endl;</xsl:text>
            <xsl:text>    foreach (const QXmlStreamAttribute &amp;attribute, reader.attributes()) {&endl;</xsl:text>
            <xsl:text>        QStringRef name = attribute.name();&endl;</xsl:text>

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
                <xsl:variable name="qstring-func">
                    <xsl:call-template name="xs-type-from-qstring-func">
                        <xsl:with-param name="xs-type" select="@type"/>
                        <xsl:with-param name="val">
                           <xsl:text>attribute.value().toString()</xsl:text>
                        </xsl:with-param>
                    </xsl:call-template>
                </xsl:variable>

                <xsl:text>        if (name == </xsl:text>
                <xsl:call-template name="string-constant">
                    <xsl:with-param name="literal" select="@name"/>
                </xsl:call-template>
                <xsl:text>) {&endl;</xsl:text>
                <xsl:text>            setAttribute</xsl:text>
                <xsl:value-of select="$cap-name"/>
                <xsl:text>(</xsl:text>
                <xsl:value-of select="$qstring-func"/>
                <xsl:text>);&endl;</xsl:text>
                <xsl:text>            continue;&endl;</xsl:text>
                <xsl:text>        }&endl;</xsl:text>
            </xsl:for-each>

            <xsl:text>        reader.raiseError(QLatin1String("Unexpected attribute ") + name.toString());&endl;</xsl:text>
            <xsl:text>    }&endl;</xsl:text>
        </xsl:if>
    </xsl:template>

    <xsl:template name="read-impl-load-child-element">
        <xsl:param name="node"/>

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
            <xsl:variable name="xs-type-cat">
                <xsl:call-template name="xs-type-category">
                    <xsl:with-param name="xs-type" select="@type"/>
                </xsl:call-template>
            </xsl:variable>
            <xsl:variable name="lower-name">
                <xsl:call-template name="lower-text">
                    <xsl:with-param name="text" select="@name"/>
                </xsl:call-template>
            </xsl:variable>
            <xsl:variable name="array" select="@maxOccurs = 'unbounded'"/>

            <xsl:text>            if (tag == </xsl:text>
            <xsl:call-template name="string-constant">
                <xsl:with-param name="literal" select="$lower-name"/>
            </xsl:call-template>
            <xsl:text>) {&endl;</xsl:text>

            <xsl:choose>
                <xsl:when test="not($array) and $xs-type-cat = 'value'">
                    <xsl:variable name="qstring-func">
                        <xsl:call-template name="xs-type-from-qstring-func">
                            <xsl:with-param name="xs-type" select="@type"/>
                            <xsl:with-param name="val" select="'reader.readElementText()'"/>
                        </xsl:call-template>
                    </xsl:variable>

                    <xsl:text>                setElement</xsl:text>
                    <xsl:value-of select="$cap-name"/>
                    <xsl:text>(</xsl:text>
                    <xsl:value-of select="$qstring-func"/>
                    <xsl:text>);&endl;</xsl:text>
                </xsl:when>
                <xsl:when test="@maxOccurs='unbounded' and $xs-type-cat = 'value'">
                    <xsl:variable name="qstring-func">
                        <xsl:call-template name="xs-type-from-qstring-func">
                            <xsl:with-param name="xs-type" select="@type"/>
                            <xsl:with-param name="val" select="'reader.readElementText()'"/>
                        </xsl:call-template>
                    </xsl:variable>

                    <xsl:text>                m_</xsl:text>
                    <xsl:value-of select="$camel-case-name"/>
                    <xsl:text>.append(</xsl:text>
                    <xsl:value-of select="$qstring-func"/>
                    <xsl:text>);&endl;</xsl:text>
                </xsl:when>
                <xsl:when test="not(@maxOccurs='unbounded') and $xs-type-cat = 'pointer'">
                    <xsl:text>                Dom</xsl:text>
                    <xsl:value-of select="@type"/>
                    <xsl:text> *v = new Dom</xsl:text>
                    <xsl:value-of select="@type"/>
                    <xsl:text>();&endl;</xsl:text>
                    <xsl:text>                v->read(reader);&endl;</xsl:text>
                    <xsl:text>                setElement</xsl:text>
                    <xsl:value-of select="$cap-name"/>
                    <xsl:text>(v);&endl;</xsl:text>
                </xsl:when>
                <xsl:when test="@maxOccurs='unbounded' and $xs-type-cat = 'pointer'">
                    <xsl:text>                Dom</xsl:text>
                    <xsl:value-of select="@type"/>
                    <xsl:text> *v = new Dom</xsl:text>
                    <xsl:value-of select="@type"/>
                    <xsl:text>();&endl;</xsl:text>
                    <xsl:text>                v->read(reader);&endl;</xsl:text>
                    <xsl:text>                m_</xsl:text>
                    <xsl:value-of select="$camel-case-name"/>
                    <xsl:text>.append(v);&endl;</xsl:text>
                </xsl:when>
            </xsl:choose>
            <xsl:text>                continue;&endl;</xsl:text>
            <xsl:text>            }&endl;</xsl:text>
        </xsl:for-each>
    </xsl:template>

    <xsl:template name="read-impl">
        <xsl:param name="node"/>
        <xsl:variable name="name" select="concat('Dom', $node/@name)"/>

        <xsl:text>void </xsl:text>
        <xsl:value-of select="$name"/>
        <xsl:text>::read(QXmlStreamReader &amp;reader)&endl;</xsl:text>

        <xsl:text>{&endl;</xsl:text>

        <xsl:call-template name="read-impl-load-attributes">
            <xsl:with-param name="node" select="$node"/>
        </xsl:call-template>

        <xsl:text>&endl;</xsl:text>

        <xsl:text>    for (bool finished = false; !finished &amp;&amp; !reader.hasError();) {&endl;</xsl:text>
        <xsl:text>        switch (reader.readNext()) {&endl;</xsl:text>
        <xsl:text>        case QXmlStreamReader::StartElement : {&endl;</xsl:text>
        <xsl:text>            const QString tag = reader.name().toString().toLower();&endl;</xsl:text>

        <xsl:for-each select="$node//xs:sequence | $node//xs:choice | $node//xs:all">
            <xsl:call-template name="read-impl-load-child-element">
                <xsl:with-param name="node" select="."/>
            </xsl:call-template>
        </xsl:for-each>

        <xsl:text>            reader.raiseError(QLatin1String("Unexpected element ") + tag);&endl;</xsl:text>
        <xsl:text>        }&endl;</xsl:text>
        <xsl:text>            break;&endl;</xsl:text>
        <xsl:text>        case QXmlStreamReader::EndElement :&endl;</xsl:text>
        <xsl:text>            finished = true;&endl;</xsl:text>
        <xsl:text>            break;&endl;</xsl:text>
        <xsl:text>        case QXmlStreamReader::Characters :&endl;</xsl:text>
        <xsl:text>            if (!reader.isWhitespace())&endl;</xsl:text>
        <xsl:text>                m_text.append(reader.text().toString());&endl;</xsl:text>
        <xsl:text>            break;&endl;</xsl:text>
        <xsl:text>        default :&endl;</xsl:text>
        <xsl:text>            break;&endl;</xsl:text>

        <xsl:text>        }&endl;</xsl:text>
        <xsl:text>    }&endl;</xsl:text>
        <xsl:text>}&endl;&endl;</xsl:text>
    </xsl:template>

<!-- Implementation: read(QDomElement) -->

    <xsl:template name="read-impl-qdom-load-attributes">
        <xsl:param name="node"/>

        <xsl:if test="$node/xs:attribute">
            <xsl:text>&endl;</xsl:text>

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
                <xsl:variable name="qstring-func">
                    <xsl:call-template name="xs-type-from-qstring-func">
                        <xsl:with-param name="xs-type" select="@type"/>
                        <xsl:with-param name="val">
                           <xsl:text>node.attribute(</xsl:text>
                           <xsl:call-template name="string-constant">
                               <xsl:with-param name="literal" select="@name"/>
                           </xsl:call-template>
                           <xsl:text>)</xsl:text>
                        </xsl:with-param>
                    </xsl:call-template>
                </xsl:variable>

                <xsl:text>    if (node.hasAttribute(</xsl:text>
                <xsl:call-template name="string-constant">
                    <xsl:with-param name="literal" select="@name"/>
                </xsl:call-template>
                <xsl:text>))&endl;</xsl:text>
                <xsl:text>        setAttribute</xsl:text>
                <xsl:value-of select="$cap-name"/>
                <xsl:text>(</xsl:text>
                <xsl:value-of select="$qstring-func"/>
                <xsl:text>);&endl;</xsl:text>
            </xsl:for-each>
        </xsl:if>
    </xsl:template>

    <xsl:template name="read-impl-qdom-load-child-element">
        <xsl:param name="node"/>

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
            <xsl:variable name="xs-type-cat">
                <xsl:call-template name="xs-type-category">
                    <xsl:with-param name="xs-type" select="@type"/>
                </xsl:call-template>
            </xsl:variable>
            <xsl:variable name="lower-name">
                <xsl:call-template name="lower-text">
                    <xsl:with-param name="text" select="@name"/>
                </xsl:call-template>
            </xsl:variable>
            <xsl:variable name="array" select="@maxOccurs = 'unbounded'"/>

            <xsl:text>            if (tag == </xsl:text>
            <xsl:call-template name="string-constant">
                <xsl:with-param name="literal" select="$lower-name"/>
            </xsl:call-template>
            <xsl:text>) {&endl;</xsl:text>

            <xsl:choose>
                <xsl:when test="not($array) and $xs-type-cat = 'value'">
                    <xsl:variable name="qstring-func">
                        <xsl:call-template name="xs-type-from-qstring-func">
                            <xsl:with-param name="xs-type" select="@type"/>
                            <xsl:with-param name="val" select="'e.text()'"/>
                        </xsl:call-template>
                    </xsl:variable>

                    <xsl:text>                setElement</xsl:text>
                    <xsl:value-of select="$cap-name"/>
                    <xsl:text>(</xsl:text>
                    <xsl:value-of select="$qstring-func"/>
                    <xsl:text>);&endl;</xsl:text>
                </xsl:when>
                <xsl:when test="@maxOccurs='unbounded' and $xs-type-cat = 'value'">
                    <xsl:variable name="qstring-func">
                        <xsl:call-template name="xs-type-from-qstring-func">
                            <xsl:with-param name="xs-type" select="@type"/>
                            <xsl:with-param name="val" select="'e.text()'"/>
                        </xsl:call-template>
                    </xsl:variable>

                    <xsl:text>                m_</xsl:text>
                    <xsl:value-of select="$camel-case-name"/>
                    <xsl:text>.append(</xsl:text>
                    <xsl:value-of select="$qstring-func"/>
                    <xsl:text>);&endl;</xsl:text>
                </xsl:when>
                <xsl:when test="not(@maxOccurs='unbounded') and $xs-type-cat = 'pointer'">
                    <xsl:text>                Dom</xsl:text>
                    <xsl:value-of select="@type"/>
                    <xsl:text> *v = new Dom</xsl:text>
                    <xsl:value-of select="@type"/>
                    <xsl:text>();&endl;</xsl:text>
                    <xsl:text>                v->read(e);&endl;</xsl:text>
                    <xsl:text>                setElement</xsl:text>
                    <xsl:value-of select="$cap-name"/>
                    <xsl:text>(v);&endl;</xsl:text>
                </xsl:when>
                <xsl:when test="@maxOccurs='unbounded' and $xs-type-cat = 'pointer'">
                    <xsl:text>                Dom</xsl:text>
                    <xsl:value-of select="@type"/>
                    <xsl:text> *v = new Dom</xsl:text>
                    <xsl:value-of select="@type"/>
                    <xsl:text>();&endl;</xsl:text>
                    <xsl:text>                v->read(e);&endl;</xsl:text>
                    <xsl:text>                m_</xsl:text>
                    <xsl:value-of select="$camel-case-name"/>
                    <xsl:text>.append(v);&endl;</xsl:text>
                </xsl:when>
            </xsl:choose>
            <xsl:text>                continue;&endl;</xsl:text>
            <xsl:text>            }&endl;</xsl:text>
        </xsl:for-each>
    </xsl:template>

    <xsl:template name="read-impl-qdom">
        <xsl:param name="node"/>
        <xsl:variable name="name" select="concat('Dom', $node/@name)"/>

        <xsl:text>#ifdef QUILOADER_QDOM_READ&endl;</xsl:text>

        <xsl:text>void </xsl:text>
        <xsl:value-of select="$name"/>
        <xsl:text>::read(const QDomElement &amp;node)&endl;</xsl:text>

        <xsl:text>{</xsl:text>

        <xsl:call-template name="read-impl-qdom-load-attributes">
            <xsl:with-param name="node" select="$node"/>
        </xsl:call-template>

        <xsl:text>&endl;</xsl:text>

        <xsl:text>    for (QDomNode n = node.firstChild(); !n.isNull(); n = n.nextSibling()) {&endl;</xsl:text>
        <xsl:text>        if (!n.isElement())&endl;</xsl:text>
        <xsl:text>            continue;&endl;</xsl:text>
        <xsl:text>        QDomElement e = n.toElement();&endl;</xsl:text>
        <xsl:text>        QString tag = e.tagName().toLower();&endl;</xsl:text>

        <xsl:for-each select="$node//xs:sequence | $node//xs:choice | $node//xs:all">
            <xsl:call-template name="read-impl-qdom-load-child-element">
                <xsl:with-param name="node" select="."/>
            </xsl:call-template>
        </xsl:for-each>

        <xsl:text>    }&endl;</xsl:text>

        <xsl:choose>
            <xsl:when test="$node[@mixed='true']">
                <xsl:text>    m_text = QLatin1String("");&endl;</xsl:text>
            </xsl:when>
            <xsl:otherwise>
                <xsl:text>    m_text.clear();&endl;</xsl:text>
            </xsl:otherwise>
        </xsl:choose>

        <xsl:text>    for (QDomNode child = node.firstChild(); !child.isNull(); child = child.nextSibling()) {&endl;</xsl:text>
        <xsl:text>        if (child.isText())&endl;</xsl:text>
        <xsl:text>            m_text.append(child.nodeValue());&endl;</xsl:text>
        <xsl:text>     }&endl;</xsl:text>

        <xsl:text>}&endl;</xsl:text>
        <xsl:text>#endif&endl;</xsl:text>
        <xsl:text>&endl;</xsl:text>
    </xsl:template>
<!-- Implementation: write() -->

    <xsl:template name="write-impl-save-attributes">
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
            <xsl:variable name="lower-name">
                <xsl:call-template name="lower-text">
                    <xsl:with-param name="text" select="@name"/>
                </xsl:call-template>
            </xsl:variable>

            <xsl:text>    if (hasAttribute</xsl:text>
            <xsl:value-of select="$cap-name"/>
            <xsl:text>())&endl;</xsl:text>
            <xsl:text>        writer.writeAttribute(</xsl:text>
            <xsl:call-template name="string-constant">
                <xsl:with-param name="literal" select="$lower-name"/>
            </xsl:call-template>

            <xsl:text>, </xsl:text>

            <xsl:call-template name="xs-type-to-qstring-func">
                <xsl:with-param name="xs-type" select="@type"/>
                <xsl:with-param name="val" select="concat('attribute', $cap-name, '()')"/>
            </xsl:call-template>

            <xsl:text>);&endl;&endl;</xsl:text>
        </xsl:for-each>
    </xsl:template>

    <xsl:template name="write-impl-save-choice-child-element">
        <xsl:param name="node"/>
        <xsl:variable name="have-kind" select="name($node) = 'xs:choice'"/>

        <xsl:text>    switch (kind()) {&endl;</xsl:text>

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
            <xsl:variable name="lower-name">
                <xsl:call-template name="lower-text">
                    <xsl:with-param name="text" select="@name"/>
                </xsl:call-template>
            </xsl:variable>
            <xsl:variable name="xs-type-cat">
                <xsl:call-template name="xs-type-category">
                    <xsl:with-param name="xs-type" select="@type"/>
                </xsl:call-template>
            </xsl:variable>

            <xsl:text>        case </xsl:text>
            <xsl:value-of select="$cap-name"/>
            <xsl:text>: {&endl;</xsl:text>
                <xsl:choose>
                    <xsl:when test="$xs-type-cat = 'value'">
                        <xsl:variable name="qstring-func">
                            <xsl:call-template name="xs-type-to-qstring-func">
                                <xsl:with-param name="xs-type" select="@type"/>
                                <xsl:with-param name="val" select="concat('element', $cap-name, '()')"/>
                            </xsl:call-template>
                        </xsl:variable>

                        <xsl:text>            writer.writeTextElement(</xsl:text>
                        <xsl:call-template name="string-constant">
                            <xsl:with-param name="literal" select="$camel-case-name"/>
                        </xsl:call-template>
                        <xsl:text>, </xsl:text>
                        <xsl:value-of select="$qstring-func"/>
                        <xsl:text>);&endl;</xsl:text>
                    </xsl:when>
                    <xsl:when test="$xs-type-cat = 'pointer'">
                        <xsl:variable name="cpp-return-type">
                            <xsl:call-template name="xs-type-to-cpp-return-type">
                                <xsl:with-param name="xs-type" select="@type"/>
                            </xsl:call-template>
                        </xsl:variable>

                        <xsl:text>            </xsl:text>
                        <xsl:value-of select="$cpp-return-type"/>
                        <xsl:text> v = element</xsl:text>
                        <xsl:value-of select="$cap-name"/>
                        <xsl:text>();&endl;</xsl:text>
                        <xsl:text>            if (v != 0) {&endl;</xsl:text>
                        <xsl:text>                v->write(writer, </xsl:text>
                        <xsl:call-template name="string-constant">
                            <xsl:with-param name="literal" select="$lower-name"/>
                        </xsl:call-template>
                        <xsl:text>);&endl;</xsl:text>
                        <xsl:text>            }&endl;</xsl:text>
                    </xsl:when>
                </xsl:choose>
            <xsl:text>            break;&endl;</xsl:text>
            <xsl:text>        }&endl;</xsl:text>
        </xsl:for-each>

        <xsl:text>        default:&endl;</xsl:text>
        <xsl:text>            break;&endl;</xsl:text>
        <xsl:text>    }&endl;</xsl:text>
    </xsl:template>

    <xsl:template name="write-impl-save-sequence-child-element">
        <xsl:param name="node"/>
        <xsl:variable name="name" select="concat('Dom', $node/@name)"/>
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
            <xsl:variable name="lower-name">
                <xsl:call-template name="lower-text">
                    <xsl:with-param name="text" select="@name"/>
                </xsl:call-template>
            </xsl:variable>
            <xsl:variable name="xs-type-cat">
                <xsl:call-template name="xs-type-category">
                    <xsl:with-param name="xs-type" select="@type"/>
                </xsl:call-template>
            </xsl:variable>
            <xsl:variable name="cpp-return-type">
                <xsl:call-template name="xs-type-to-cpp-return-type">
                    <xsl:with-param name="xs-type" select="@type"/>
                </xsl:call-template>
            </xsl:variable>

            <xsl:choose>
                <xsl:when test="@maxOccurs='unbounded'">
                    <xsl:text>    for (int i = 0; i &lt; m_</xsl:text>
                    <xsl:value-of select="$camel-case-name"/>
                    <xsl:text>.size(); ++i) {&endl;</xsl:text>
                    <xsl:text>        </xsl:text>
                    <xsl:value-of select="$cpp-return-type"/>
                    <xsl:text> v = m_</xsl:text>
                    <xsl:value-of select="$camel-case-name"/>
                    <xsl:text>[i];&endl;</xsl:text>
                    <xsl:choose>
                        <xsl:when test="$xs-type-cat = 'pointer'">
                            <xsl:text>        v->write(writer, </xsl:text>
                            <xsl:call-template name="string-constant">
                                <xsl:with-param name="literal" select="$lower-name"/>
                            </xsl:call-template>
                            <xsl:text>);&endl;</xsl:text>
                        </xsl:when>
                        <xsl:otherwise>
                            <xsl:variable name="qstring-func">
                                <xsl:call-template name="xs-type-to-qstring-func">
                                    <xsl:with-param name="xs-type" select="@type"/>
                                    <xsl:with-param name="val" select="'v'"/>
                                </xsl:call-template>
                            </xsl:variable>

                            <xsl:text>        writer.writeTextElement(</xsl:text>
                            <xsl:call-template name="string-constant">
                                <xsl:with-param name="literal" select="$lower-name"/>
                            </xsl:call-template>
                            <xsl:text>, </xsl:text>
                            <xsl:value-of select="$qstring-func"/>
                            <xsl:text>);&endl;</xsl:text>
                        </xsl:otherwise>
                    </xsl:choose>
                    <xsl:text>    }&endl;</xsl:text>
                </xsl:when>
                <xsl:otherwise>
                    <xsl:text>    if (m_children &amp; </xsl:text>
                    <xsl:value-of select="$cap-name"/>
                    <xsl:text>) {&endl;</xsl:text>
                    <xsl:choose>
                        <xsl:when test="$xs-type-cat = 'pointer'">
                            <xsl:text>        m_</xsl:text>
                            <xsl:value-of select="$camel-case-name"/>
                            <xsl:text>->write(writer, </xsl:text>
                            <xsl:call-template name="string-constant">
                                <xsl:with-param name="literal" select="$lower-name"/>
                            </xsl:call-template>
                            <xsl:text>);&endl;</xsl:text>
                        </xsl:when>
                        <xsl:otherwise>
                            <xsl:variable name="qstring-func">
                                <xsl:call-template name="xs-type-to-qstring-func">
                                    <xsl:with-param name="xs-type" select="@type"/>
                                    <xsl:with-param name="val" select="concat('m_', $camel-case-name)"/>
                                </xsl:call-template>
                            </xsl:variable>
                            <xsl:text>        writer.writeTextElement(</xsl:text>
                            <xsl:call-template name="string-constant">
                                <xsl:with-param name="literal" select="$lower-name"/>
                            </xsl:call-template>
                            <xsl:text>, </xsl:text>
                            <xsl:value-of select="$qstring-func"/>
                            <xsl:text>);&endl;</xsl:text>
                        </xsl:otherwise>
                    </xsl:choose>
                    <xsl:text>    }&endl;&endl;</xsl:text>
                </xsl:otherwise>
            </xsl:choose>
        </xsl:for-each>
    </xsl:template>

    <xsl:template name="write-impl">
        <xsl:param name="node"/>
        <xsl:variable name="name" select="concat('Dom', $node/@name)"/>
        <xsl:variable name="lower-name">
            <xsl:call-template name="lower-text">
                <xsl:with-param name="text" select="@name"/>
            </xsl:call-template>
        </xsl:variable>

        <xsl:text>void </xsl:text>
        <xsl:value-of select="$name"/>
        <xsl:text>::write(QXmlStreamWriter &amp;writer, const QString &amp;tagName) const&endl;</xsl:text>
        <xsl:text>{&endl;</xsl:text>

        <xsl:text>    writer.writeStartElement(tagName.isEmpty() ? QString::fromUtf8("</xsl:text>
        <xsl:value-of select="$lower-name"/>
        <xsl:text>") : tagName.toLower());&endl;&endl;</xsl:text>

        <xsl:call-template name="write-impl-save-attributes">
            <xsl:with-param name="node" select="$node"/>
        </xsl:call-template>

        <xsl:for-each select="$node//xs:choice">
            <xsl:call-template name="write-impl-save-choice-child-element">
                <xsl:with-param name="node" select="."/>
            </xsl:call-template>
        </xsl:for-each>

        <xsl:for-each select="$node//xs:sequence | $node//xs:all">
            <xsl:call-template name="write-impl-save-sequence-child-element">
                <xsl:with-param name="node" select="."/>
            </xsl:call-template>
        </xsl:for-each>

        <xsl:text>    if (!m_text.isEmpty())&endl;</xsl:text>
        <xsl:text>        writer.writeCharacters(m_text);&endl;&endl;</xsl:text>

        <xsl:text>    writer.writeEndElement();&endl;</xsl:text>
        <xsl:text>}&endl;&endl;</xsl:text>
    </xsl:template>

<!-- Implementation: child element setters -->

    <xsl:template name="child-setter-impl-helper">
        <xsl:param name="node"/>
        <xsl:param name="name"/>
        <xsl:variable name="make-kind-enum" select="name($node)='xs:choice'"/>
        <xsl:variable name="isChoice" select="name($node)='xs:choice'"/>

        <xsl:for-each select="$node/xs:element">
            <xsl:variable name="array" select="@maxOccurs = 'unbounded'"/>
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

            <xsl:if test="$xs-type-cat = 'pointer'">
                <xsl:value-of select="$return-cpp-type"/>
                <xsl:text> </xsl:text>
                <xsl:value-of select="$name"/>
                <xsl:text>::takeElement</xsl:text>
                <xsl:value-of select="$cap-name"/>
                <xsl:text>() &endl;{&endl;</xsl:text>
                <xsl:text>    </xsl:text>
                <xsl:value-of select="$return-cpp-type"/>
                <xsl:text> a = m_</xsl:text>
                <xsl:value-of select="$camel-case-name"/>
                <xsl:text>;&endl;</xsl:text>
                <xsl:text>    m_</xsl:text>
                <xsl:value-of select="$camel-case-name"/>
                <xsl:text> = 0;&endl;</xsl:text>
                <xsl:if test="not($isChoice)">
                    <xsl:text>    m_children ^= </xsl:text>
                    <xsl:value-of select="$cap-name"/>
                    <xsl:text>;&endl;</xsl:text>
                </xsl:if>
                <xsl:text>    return a;&endl;</xsl:text>
                <xsl:text>}&endl;&endl;</xsl:text>
            </xsl:if>

            <xsl:text>void </xsl:text>
            <xsl:value-of select="$name"/>
            <xsl:text>::setElement</xsl:text>
            <xsl:value-of select="$cap-name"/>
            <xsl:text>(</xsl:text>
            <xsl:value-of select="$argument-cpp-type"/>
            <xsl:text> a)&endl;</xsl:text>
            <xsl:text>{&endl;</xsl:text>
            <xsl:choose>
                <xsl:when test="$make-kind-enum">
                    <xsl:text>    clear(false);&endl;</xsl:text>
                    <xsl:text>    m_kind = </xsl:text>
                    <xsl:value-of select="$cap-name"/>
                    <xsl:text>;&endl;</xsl:text>
                </xsl:when>
                <xsl:when test="$xs-type-cat = 'pointer'">
                    <xsl:text>    delete </xsl:text>
                    <xsl:text>m_</xsl:text>
                    <xsl:value-of select="$camel-case-name"/>
                    <xsl:text>;&endl;</xsl:text>
                </xsl:when>
            </xsl:choose>
            <xsl:if test="not($isChoice)">
                <xsl:text>    m_children |= </xsl:text>
                <xsl:value-of select="$cap-name"/>
                <xsl:text>;&endl;</xsl:text>
            </xsl:if>
            <xsl:text>    m_</xsl:text>
            <xsl:value-of select="$camel-case-name"/>
            <xsl:text> = a;&endl;</xsl:text>
            <xsl:text>}&endl;&endl;</xsl:text>
        </xsl:for-each>
    </xsl:template>

    <xsl:template name="child-setter-impl">
        <xsl:param name="node"/>
        <xsl:variable name="name" select="concat('Dom', $node/@name)"/>

        <xsl:for-each select="$node/xs:sequence | $node/xs:choice | $node/xs:all">
            <xsl:call-template name="child-setter-impl-helper">
                <xsl:with-param name="node" select="."/>
                <xsl:with-param name="name" select="$name"/>
            </xsl:call-template>
        </xsl:for-each>
    </xsl:template>

    <xsl:template name="child-clear-impl">
        <xsl:param name="node"/>

       <xsl:variable name="name" select="concat('Dom', @name)"/>
       <xsl:for-each select="$node/xs:sequence | $node/xs:choice | $node/xs:all">
            <xsl:variable name="isChoice" select="name()='xs:choice'"/>
            <xsl:variable name="make-child-enum" select="boolean(xs:sequence) and not(@maxOccurs='unbounded')"/>

            <xsl:for-each select="xs:element">
                <xsl:if test="not($isChoice) and not(@maxOccurs='unbounded')">
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
                    <xsl:variable name="xs-type-cat">
                        <xsl:call-template name="xs-type-category">
                            <xsl:with-param name="xs-type" select="@type"/>
                            <xsl:with-param name="array" select="@maxOccurs='unbounded'"/>
                        </xsl:call-template>
                    </xsl:variable>

                    <xsl:text>void </xsl:text>
                    <xsl:value-of select="$name"/>
                    <xsl:text>::clearElement</xsl:text>
                    <xsl:value-of select="$cap-name"/>
                    <xsl:text>()&endl;</xsl:text>
                    <xsl:text>{&endl;</xsl:text>
                    <xsl:if test="$xs-type-cat = 'pointer'">
                        <xsl:text>    delete m_</xsl:text>
                        <xsl:value-of select="$camel-case-name"/>
                        <xsl:text>;&endl;</xsl:text>
                        <xsl:text>    m_</xsl:text>
                        <xsl:value-of select="$camel-case-name"/>
                        <xsl:text> = 0;&endl;</xsl:text>
                    </xsl:if>
                    <xsl:text>    m_children &amp;= ~</xsl:text>
                    <xsl:value-of select="$cap-name"/>
                    <xsl:text>;&endl;</xsl:text>
                    <xsl:text>}&endl;&endl;</xsl:text>
                </xsl:if>
            </xsl:for-each>
        </xsl:for-each>
    </xsl:template>


<!-- Implementation -->

    <xsl:template name="class-implementation">
        <xsl:param name="node"/>

        <xsl:call-template name="clear-impl">
            <xsl:with-param name="node" select="$node"/>
        </xsl:call-template>

        <xsl:call-template name="ctor-impl">
            <xsl:with-param name="node" select="$node"/>
        </xsl:call-template>

        <xsl:call-template name="dtor-impl">
            <xsl:with-param name="node" select="$node"/>
        </xsl:call-template>

        <xsl:call-template name="read-impl">
            <xsl:with-param name="node" select="$node"/>
        </xsl:call-template>

        <xsl:call-template name="read-impl-qdom">
            <xsl:with-param name="node" select="$node"/>
        </xsl:call-template>

        <xsl:call-template name="write-impl">
            <xsl:with-param name="node" select="$node"/>
        </xsl:call-template>

        <xsl:call-template name="child-setter-impl">
            <xsl:with-param name="node" select="$node"/>
        </xsl:call-template>

        <xsl:call-template name="child-clear-impl">
            <xsl:with-param name="node" select="$node"/>
        </xsl:call-template>

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
</xsl:text>
        <xsl:text>#include "ui4_p.h"&endl;</xsl:text>
        <xsl:text>&endl;</xsl:text>
        <xsl:text>#ifdef QUILOADER_QDOM_READ&endl;</xsl:text>
        <xsl:text>#include &lt;QtXml/QDomElement&gt;&endl;</xsl:text>
        <xsl:text>#endif&endl;</xsl:text>
        <xsl:text>&endl;</xsl:text>
        <xsl:text>QT_BEGIN_NAMESPACE&endl;</xsl:text>

        <xsl:text>#ifdef QFORMINTERNAL_NAMESPACE&endl;</xsl:text>
        <xsl:text>using namespace QFormInternal;&endl;</xsl:text>
        <xsl:text>#endif&endl;</xsl:text>
        <xsl:text>&endl;</xsl:text>

        <xsl:text>/*******************************************************************************&endl;</xsl:text>
        <xsl:text>** Implementations&endl;</xsl:text>
        <xsl:text>*/&endl;&endl;</xsl:text>

        <xsl:for-each select="xs:complexType">
            <xsl:call-template name="class-implementation">
                <xsl:with-param name="node" select="."/>
            </xsl:call-template>
        </xsl:for-each>
        <xsl:text>QT_END_NAMESPACE&endl;</xsl:text>

        <xsl:text>&endl;</xsl:text>
    </xsl:template>

</xsl:stylesheet>
