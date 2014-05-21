<!DOCTYPE xsl:stylesheet [
     <!ENTITY endl "&#10;">
]>
<xsl:stylesheet version="1.0"
                xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
                xmlns:xs="http://www.w3.org/2001/XMLSchema">

<!-- Hack to make names camel case
     All names in UI files are lowercase, while the element names are
     capital case. To make the UI files conforming to the XSD file + keep
     the DOM interface we rename them here -->
    <xsl:template name="camel-case">
        <xsl:param name="text"/>
    <xsl:choose>
        <xsl:when test="$text='exportmacro'">exportMacro</xsl:when>
        <xsl:when test="$text='layoutdefault'">layoutDefault</xsl:when>
        <xsl:when test="$text='layoutfunction'">layoutFunction</xsl:when>
        <xsl:when test="$text='pixmapfunction'">pixmapFunction</xsl:when>
        <xsl:when test="$text='customwidgets'">customWidgets</xsl:when>
        <xsl:when test="$text='tabstops'">tabStops</xsl:when>
        <xsl:when test="$text='tabstop'">tabStop</xsl:when>
        <xsl:when test="$text='buttongroups'">buttonGroups</xsl:when>
        <xsl:when test="$text='exportmacro'">exportMacro</xsl:when>
        <xsl:when test="$text='actiongroup'">actionGroup</xsl:when>
        <xsl:when test="$text='buttongroup'">buttonGroup</xsl:when>
        <xsl:when test="$text='customwidget'">customWidget</xsl:when>
        <xsl:when test="$text='sizehint'">sizeHint</xsl:when>
        <xsl:when test="$text='addpagemethod'">addPageMethod</xsl:when>
        <xsl:when test="$text='sizepolicy'">sizePolicy</xsl:when>
        <xsl:when test="$text='hordata'">horData</xsl:when>
        <xsl:when test="$text='verdata'">verData</xsl:when>
        <xsl:when test="$text='rowspan'">rowSpan</xsl:when>
        <xsl:when test="$text='colspan'">colSpan</xsl:when>
        <xsl:when test="$text='addaction'">addAction</xsl:when>
        <xsl:when test="$text='zorder'">zOrder</xsl:when>
        <xsl:when test="$text='startx'">startX</xsl:when>
        <xsl:when test="$text='starty'">startY</xsl:when>
        <xsl:when test="$text='endx'">endX</xsl:when>
        <xsl:when test="$text='endy'">endY</xsl:when>
        <xsl:when test="$text='centralx'">centralX</xsl:when>
        <xsl:when test="$text='centraly'">centralY</xsl:when>
        <xsl:when test="$text='focalx'">focalX</xsl:when>
        <xsl:when test="$text='focaly'">focalY</xsl:when>
        <xsl:when test="$text='widgetdata'">widgetData</xsl:when>
        <xsl:when test="$text='coordinatemode'">coordinateMode</xsl:when>
        <xsl:when test="$text='brushstyle'">brushStyle</xsl:when>
        <xsl:when test="$text='colorrole'">colorRole</xsl:when>
        <xsl:when test="$text='pointsize'">pointSize</xsl:when>
        <xsl:when test="$text='strikeout'">strikeOut</xsl:when>
        <xsl:when test="$text='stylestrategy'">styleStrategy</xsl:when>
        <xsl:when test="$text='hsizetype'">hSizeType</xsl:when>
        <xsl:when test="$text='vsizetype'">vSizeType</xsl:when>
        <xsl:when test="$text='horstretch'">horStretch</xsl:when>
        <xsl:when test="$text='verstretch'">verStretch</xsl:when>
        <xsl:when test="$text='normaloff'">normalOff</xsl:when>
        <xsl:when test="$text='normalon'">normalOn</xsl:when>
        <xsl:when test="$text='disabledoff'">disabledOff</xsl:when>
        <xsl:when test="$text='disabledon'">disabledOn</xsl:when>
        <xsl:when test="$text='activeoff'">activeOff</xsl:when>
        <xsl:when test="$text='activeon'">activeOn</xsl:when>
        <xsl:when test="$text='selectedoff'">selectedOff</xsl:when>
        <xsl:when test="$text='selectedon'">selectedOn</xsl:when>
        <xsl:when test="$text='cursorshape'">cursorShape</xsl:when>
        <xsl:when test="$text='iconset'">iconSet</xsl:when>
        <xsl:when test="$text='stringlist'">stringList</xsl:when>
        <xsl:when test="$text='datetime'">dateTime</xsl:when>
        <xsl:when test="$text='pointf'">pointF</xsl:when>
        <xsl:when test="$text='rectf'">rectF</xsl:when>
        <xsl:when test="$text='sizef'">sizeF</xsl:when>
        <xsl:when test="$text='longlong'">longLong</xsl:when>
        <xsl:when test="$text='uint'">UInt</xsl:when>
        <xsl:when test="$text='ulonglong'">uLongLong</xsl:when>
        <xsl:when test="$text='rowstretch'">rowStretch</xsl:when>
        <xsl:when test="$text='columnstretch'">columnStretch</xsl:when>
        <xsl:when test="$text='rowminimumheight'">rowMinimumHeight</xsl:when>
        <xsl:when test="$text='columnminimumwidth'">columnMinimumWidth</xsl:when>
        <xsl:when test="$text='extracomment'">extraComment</xsl:when>
       <xsl:otherwise><xsl:value-of select="$text"/></xsl:otherwise>
    </xsl:choose>
    </xsl:template>


<!-- Convenience templates -->

    <xsl:template name="cap-first-char">
        <xsl:param name="text"/>
        <xsl:value-of select="concat(translate(substring($text, 1, 1), 'abcdefghijklmnopqrstuvwxyz', 'ABCDEFGHIJKLMNOPQRSTUVWXYZ'), substring($text, 2))" />
    </xsl:template>

    <xsl:template name="lower-text">
        <xsl:param name="text"/>

        <xsl:if test="boolean($text)">
            <xsl:variable name="head" select="substring($text, 1, 1)"/>
            <xsl:variable name="tail" select="substring($text, 2)"/>
            <xsl:variable name="lower-head" select="translate($text, 'ABCDEFGHIJKLMNOPQRSTUVWXYZ', 'abcdefghijklmnopqrstuvwxyz')"/>
            <xsl:variable name="lower-tail">
                <xsl:call-template name="lower-text">
                    <xsl:with-param name="text" select="tail"/>
                </xsl:call-template>
            </xsl:variable>
            <xsl:value-of select='concat($lower-head, $lower-tail)'/>
        </xsl:if>
    </xsl:template>

    <xsl:template name="powers-of-two">
        <xsl:param name="num"/>

        <xsl:choose>
            <xsl:when test="$num=0">1</xsl:when>
            <xsl:otherwise>
                <xsl:variable name="x">
                    <xsl:call-template name="powers-of-two">
                        <xsl:with-param name="num" select="$num - 1"/>
                    </xsl:call-template>
                </xsl:variable>
                <xsl:value-of select="2*$x"/>
            </xsl:otherwise>
        </xsl:choose>
    </xsl:template>

<!-- Convenience templates: xs-types to c++ types conversions -->

    <xsl:template name="xs-type-from-qstring-func">
        <xsl:param name="xs-type"/>
        <xsl:param name="val"/>
        <xsl:choose>
            <xsl:when test="$xs-type='xs:string'">
                <xsl:value-of select="$val"/>
            </xsl:when>
            <xsl:when test="$xs-type='xs:integer'">
                <xsl:value-of select="$val"/>
                <xsl:text>.toInt()</xsl:text>
            </xsl:when>
            <xsl:when test="$xs-type='xs:float'">
                <xsl:value-of select="$val"/>
                <xsl:text>.toFloat()</xsl:text>
            </xsl:when>
            <xsl:when test="$xs-type='xs:double'">
                <xsl:value-of select="$val"/>
                <xsl:text>.toDouble()</xsl:text>
            </xsl:when>
            <xsl:when test="$xs-type='xs:boolean'">
                <xsl:text>(</xsl:text>
                <xsl:value-of select="$val"/>
                <xsl:text> == QLatin1String("true") ? true : false)</xsl:text>
            </xsl:when>
            <xsl:when test="$xs-type='xs:long'">
                <xsl:value-of select="$val"/>
                <xsl:text>.toLongLong()</xsl:text>
            </xsl:when>
            <xsl:when test="$xs-type='xs:unsignedInt'">
                <xsl:value-of select="$val"/>
                <xsl:text>.toUInt()</xsl:text>
            </xsl:when>
            <xsl:when test="$xs-type='xs:unsignedLong'">
                <xsl:value-of select="$val"/>
                <xsl:text>.toULongLong()</xsl:text>
            </xsl:when>
            <xsl:otherwise>### BZZZZT! ###</xsl:otherwise>
        </xsl:choose>
    </xsl:template>

    <xsl:template name="xs-type-to-qstring-func">
        <xsl:param name="xs-type"/>
        <xsl:param name="val"/>
        <xsl:choose>
            <xsl:when test="$xs-type='xs:string'">
                <xsl:value-of select="$val"/>
            </xsl:when>
            <xsl:when test="$xs-type='xs:integer'">
                <xsl:text>QString::number(</xsl:text>
                <xsl:value-of select="$val"/>
                <xsl:text>)</xsl:text>
            </xsl:when>
            <xsl:when test="$xs-type='xs:long'">
                <xsl:text>QString::number(</xsl:text>
                <xsl:value-of select="$val"/>
                <xsl:text>)</xsl:text>
            </xsl:when>
            <xsl:when test="$xs-type='xs:unsignedInt'">
                <xsl:text>QString::number(</xsl:text>
                <xsl:value-of select="$val"/>
                <xsl:text>)</xsl:text>
            </xsl:when>
            <xsl:when test="$xs-type='xs:unsignedLong'">
                <xsl:text>QString::number(</xsl:text>
                <xsl:value-of select="$val"/>
                <xsl:text>)</xsl:text>
            </xsl:when>
            <xsl:when test="$xs-type='xs:float'">
                <xsl:text>QString::number(</xsl:text>
                <xsl:value-of select="$val"/>
                <xsl:text>, 'f', 8)</xsl:text>
            </xsl:when>
            <xsl:when test="$xs-type='xs:double'">
                <xsl:text>QString::number(</xsl:text>
                <xsl:value-of select="$val"/>
                <xsl:text>, 'f', 15)</xsl:text>
            </xsl:when>
            <xsl:when test="$xs-type='xs:boolean'">
                <xsl:text>(</xsl:text>
                <xsl:value-of select="$val"/>
                <xsl:text> ? QLatin1String("true") : QLatin1String("false"))</xsl:text>
            </xsl:when>
            <xsl:otherwise>### BZZZZT! ###</xsl:otherwise>
        </xsl:choose>
    </xsl:template>

    <xsl:template name="xs-type-category">
        <xsl:param name="xs-type"/>
        <xsl:param name="array" select="false"/>
        <xsl:choose>
            <xsl:when test="$array">value</xsl:when>
            <xsl:otherwise>
                <xsl:choose>
                    <xsl:when test="$xs-type='xs:string'">value</xsl:when>
                    <xsl:when test="$xs-type='xs:integer'">value</xsl:when>
                    <xsl:when test="$xs-type='xs:float'">value</xsl:when>
                    <xsl:when test="$xs-type='xs:double'">value</xsl:when>
                    <xsl:when test="$xs-type='xs:boolean'">value</xsl:when>
                    <xsl:when test="$xs-type='xs:long'">value</xsl:when>
                    <xsl:when test="$xs-type='xs:unsignedInt'">value</xsl:when>
                    <xsl:when test="$xs-type='xs:unsignedLong'">value</xsl:when>
                    <xsl:otherwise>pointer</xsl:otherwise>
                </xsl:choose>
            </xsl:otherwise>
        </xsl:choose>
    </xsl:template>

    <xsl:template name="xs-type-to-cpp-type">
        <xsl:param name="xs-type"/>
        <xsl:param name="array" select="false"/>
        <xsl:choose>
            <xsl:when test="$array">
                <xsl:choose>
                    <xsl:when test="$xs-type='xs:string'">QStringList</xsl:when>
                    <xsl:when test="$xs-type='xs:integer'">QList&lt;int&gt;</xsl:when>
                    <xsl:when test="$xs-type='xs:float'">QList&lt;float&gt;</xsl:when>
                    <xsl:when test="$xs-type='xs:double'">QList&lt;double&gt;</xsl:when>
                    <xsl:when test="$xs-type='xs:boolean'">QList&lt;bool&gt;</xsl:when>
                    <xsl:when test="$xs-type='xs:long'">QList&lt;qlonglong&gt;</xsl:when>
                    <xsl:when test="$xs-type='xs:unsignedInt'">QList&lt;uint&gt;</xsl:when>
                    <xsl:when test="$xs-type='xs:unsignedLong'">QList&lt;qulonglong&gt;</xsl:when>
                    <xsl:otherwise>QList&lt;Dom<xsl:value-of select="$xs-type"/>*&gt;</xsl:otherwise>
                </xsl:choose>
            </xsl:when>
            <xsl:otherwise>
                <xsl:choose>
                    <xsl:when test="$xs-type='xs:string'">QString</xsl:when>
                    <xsl:when test="$xs-type='xs:integer'">int</xsl:when>
                    <xsl:when test="$xs-type='xs:float'">float</xsl:when>
                    <xsl:when test="$xs-type='xs:double'">double</xsl:when>
                    <xsl:when test="$xs-type='xs:boolean'">bool</xsl:when>
                    <xsl:when test="$xs-type='xs:long'">qlonglong</xsl:when>
                    <xsl:when test="$xs-type='xs:unsignedInt'">uint</xsl:when>
                    <xsl:when test="$xs-type='xs:unsignedLong'">qulonglong</xsl:when>
                    <xsl:otherwise>Dom<xsl:value-of select="$xs-type"/></xsl:otherwise>
                </xsl:choose>
            </xsl:otherwise>
        </xsl:choose>
    </xsl:template>

    <xsl:template name="xs-type-to-cpp-return-type">
        <xsl:param name="xs-type"/>
        <xsl:param name="array" select="false"/>
        <xsl:choose>
            <xsl:when test="$array">
                <xsl:choose>
                    <xsl:when test="$xs-type='xs:string'">QStringList</xsl:when>
                    <xsl:when test="$xs-type='xs:integer'">QList&lt;int&gt;</xsl:when>
                    <xsl:when test="$xs-type='xs:float'">QList&lt;float&gt;</xsl:when>
                    <xsl:when test="$xs-type='xs:double'">QList&lt;double&gt;</xsl:when>
                    <xsl:when test="$xs-type='xs:boolean'">QList&lt;bool&gt;</xsl:when>
                    <xsl:when test="$xs-type='xs:long'">QList&lt;qlonglong&gt;</xsl:when>
                    <xsl:when test="$xs-type='xs:unsignedInt'">QList&lt;uint&gt;</xsl:when>
                    <xsl:when test="$xs-type='xs:unsignedLong'">QList&lt;qulonglong&gt;</xsl:when>
                    <xsl:otherwise>QList&lt;Dom<xsl:value-of select="$xs-type"/>*&gt;</xsl:otherwise>
                </xsl:choose>
            </xsl:when>
            <xsl:otherwise>
                <xsl:choose>
                    <xsl:when test="$xs-type='xs:string'">QString</xsl:when>
                    <xsl:when test="$xs-type='xs:integer'">int</xsl:when>
                    <xsl:when test="$xs-type='xs:float'">float</xsl:when>
                    <xsl:when test="$xs-type='xs:double'">double</xsl:when>
                    <xsl:when test="$xs-type='xs:boolean'">bool</xsl:when>
                    <xsl:when test="$xs-type='xs:long'">qlonglong</xsl:when>
                    <xsl:when test="$xs-type='xs:unsignedInt'">uint</xsl:when>
                    <xsl:when test="$xs-type='xs:unsignedLong'">qulonglong</xsl:when>
                    <xsl:otherwise>Dom<xsl:value-of select="$xs-type"/>*</xsl:otherwise>
                </xsl:choose>
            </xsl:otherwise>
        </xsl:choose>
    </xsl:template>

    <xsl:template name="xs-type-to-cpp-argument-type">
        <xsl:param name="xs-type"/>
        <xsl:param name="array" select="false"/>
        <xsl:choose>
            <xsl:when test="$array">
                <xsl:choose>
                    <xsl:when test="$xs-type='xs:string'">const QStringList&amp;</xsl:when>
                    <xsl:when test="$xs-type='xs:integer'">const QList&lt;int&gt;&amp;</xsl:when>
                    <xsl:when test="$xs-type='xs:float'">const QList&lt;float&gt;&amp;</xsl:when>
                    <xsl:when test="$xs-type='xs:double'">const QList&lt;double&gt;&amp;</xsl:when>
                    <xsl:when test="$xs-type='xs:boolean'">const QList&lt;bool&gt;&amp;</xsl:when>
                    <xsl:when test="$xs-type='xs:long'">const QList&lt;qlonglong&gt;&amp;</xsl:when>
                    <xsl:when test="$xs-type='xs:unsignedInt'">const QList&lt;uint&gt;&amp;</xsl:when>
                    <xsl:when test="$xs-type='xs:unsignedLong'">const QList&lt;qulonglong&gt;&amp;</xsl:when>
                    <xsl:otherwise>const QList&lt;Dom<xsl:value-of select="$xs-type"/>*&gt;&amp;</xsl:otherwise>
                </xsl:choose>
            </xsl:when>
            <xsl:otherwise>
                <xsl:choose>
                    <xsl:when test="$xs-type='xs:string'">const QString&amp;</xsl:when>
                    <xsl:when test="$xs-type='xs:integer'">int</xsl:when>
                    <xsl:when test="$xs-type='xs:float'">float</xsl:when>
                    <xsl:when test="$xs-type='xs:double'">double</xsl:when>
                    <xsl:when test="$xs-type='xs:boolean'">bool</xsl:when>
                    <xsl:when test="$xs-type='xs:long'">qlonglong</xsl:when>
                    <xsl:when test="$xs-type='xs:unsignedInt'">uint</xsl:when>
                    <xsl:when test="$xs-type='xs:unsignedLong'">qulonglong</xsl:when>
                    <xsl:otherwise>Dom<xsl:value-of select="$xs-type"/>*</xsl:otherwise>
                </xsl:choose>
            </xsl:otherwise>
        </xsl:choose>
    </xsl:template>


</xsl:stylesheet>
