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

#ifndef QPDF_P_H
#define QPDF_P_H

#include <qglobal.h>

#ifndef QT_NO_PDF

#include <qmatrix.h>
#include <qstring.h>
#include <qvector.h>

#include <qfontengine_faceid_p.h>
#include <qfontsubset_p.h>
#include <qstroker_p.h>
#include <qpaintengine_p.h>
#include <qpagelayout.h>

const char *qt_real_to_string(qreal val, char *buf);
const char *qt_int_to_string(int val, char *buf);

namespace QPdf {

    class ByteStream
    {
    public:
        // fileBacking means that ByteStream will buffer the contents on disk
        // if the size exceeds a certain threshold. In this case, if a byte
        // array was passed in, its contents may no longer correspond to the
        // ByteStream contents.

        explicit ByteStream(bool fileBacking = false);
        explicit ByteStream(QByteArray *ba, bool fileBacking = false);
        ~ByteStream();

        ByteStream &operator <<(char chr);
        ByteStream &operator <<(const char *str);
        ByteStream &operator <<(const QByteArray &str);
        ByteStream &operator <<(const ByteStream &src);

        ByteStream &operator <<(const QString &str);

        ByteStream &operator <<(qreal val);
        ByteStream &operator <<(int val);
        ByteStream &operator <<(const QPointF &p);

        // Note that the stream may be invalidated by calls that insert data.
        QIODevice *stream();
        void clear();

        static inline int maxMemorySize() { return 100000000; }
        static inline int chunkSize()     { return 10000000; }

    protected:
        void constructor_helper(QIODevice *dev);
        void constructor_helper(QByteArray *ba);

    private:
        void prepareBuffer();

        QIODevice *dev;
        QByteArray ba;
        bool fileBackingEnabled;
        bool fileBackingActive;
        bool handleDirty;
    };

    enum PathFlags {
        ClipPath,
        FillPath,
        StrokePath,
        FillAndStrokePath
    };
    QByteArray generatePath(const QPainterPath &path, const QTransform &matrix, PathFlags flags);
    QByteArray generateMatrix(const QTransform &matrix);
    QByteArray generateDashes(const QPen &pen);
    QByteArray patternForBrush(const QBrush &b);

    struct Stroker {
        Stroker();
        void setPen(const QPen &pen, QPainter::RenderHints hints);
        void strokePath(const QPainterPath &path);
        ByteStream *stream;
        bool first;
        QTransform matrix;
        bool cosmeticPen;

    private:
        QStroker basicStroker;
        QDashStroker dashStroker;
        QStrokerOps *stroker;
    };

    QByteArray ascii85Encode(const QByteArray &input);

    const char *toHex(ushort u, char *buffer);
    const char *toHex(uchar u, char *buffer);

}

class QPdfPage : public QPdf::ByteStream
{
public:
    QPdfPage();

    QVector<uint> images;
    QVector<uint> graphicStates;
    QVector<uint> patterns;
    QVector<uint> fonts;
    QVector<uint> annotations;

    void streamImage(int w, int h, int object);

    QSize pageSize;
};

class QPdfWriter;
class QPdfEnginePrivate;

class Q_GUI_EXPORT QPdfEngine : public QPaintEngine
{
    Q_DECLARE_PRIVATE(QPdfEngine)
    friend class QPdfWriter;

public:
    QPdfEngine();
    QPdfEngine(QPdfEnginePrivate &d);
    ~QPdfEngine() {}

    void setOutputFilename(const QString &filename);

    void setResolution(int resolution);
    int resolution() const;

    bool begin(QPaintDevice *pdev) override;
    bool end() override;

    void drawPoints(const QPointF *points, int pointCount) override;
    void drawLines(const QLineF *lines, int lineCount) override;
    void drawRects(const QRectF *rects, int rectCount) override;
    void drawPolygon(const QPointF *points, int pointCount, PolygonDrawMode mode) override;
    void drawPath (const QPainterPath & path) override;

    void drawTextItem(const QPointF &p, const QTextItem &textItem) override;

    void drawPixmap (const QRectF & rectangle, const QPixmap & pixmap, const QRectF & sr) override;
    void drawImage(const QRectF &r, const QImage &pm, const QRectF &sr,
                   Qt::ImageConversionFlags flags = Qt::AutoColor) override;
    void drawTiledPixmap (const QRectF & rectangle, const QPixmap & pixmap, const QPointF & point) override;

    void drawHyperlink(const QRectF &r, const QUrl &url);

    void updateState(const QPaintEngineState &state) override;

    int metric(QPaintDevice::PaintDeviceMetric metricType) const;
    Type type() const override;

    // Printer stuff
    bool newPage();

    // Page layout stuff
    void setPageLayout(const QPageLayout &pageLayout);
    void setPageSize(const QPageSize &pageSize);
    void setPageOrientation(QPageLayout::Orientation orientation);
    void setPageMargins(const QMarginsF &margins, QPageSize::Unit units = QPageSize::Unit::Point);

    QPageLayout pageLayout() const;

    void setPen();
    void setBrush();
    void setupGraphicsState(QPaintEngine::DirtyFlags flags);

private:
    void updateClipPath(const QPainterPath & path, Qt::ClipOperation op);
};

class Q_GUI_EXPORT QPdfEnginePrivate : public QPaintEnginePrivate
{
    Q_DECLARE_PUBLIC(QPdfEngine)

public:
    QPdfEnginePrivate();
    ~QPdfEnginePrivate();

    inline uint requestObject() { return currentObject++; }

    void writeHeader();
    void writeTail();

    int addImage(const QImage &image, bool *bitmap, qint64 serial_no);
    int addConstantAlphaObject(int brushAlpha, int penAlpha = 255);
    int addBrushPattern(const QTransform &matrix, bool *specifyColor, int *gStateObject);

    void drawTextItem(const QPointF &p, const QTextItemInt &ti);

    QTransform pageMatrix() const;

    void newPage();

    int currentObject;

    QPdfPage* currentPage;
    QPdf::Stroker stroker;

    QPointF brushOrigin;
    QBrush brush;
    QPen pen;
    QVector<QPainterPath> clips;
    bool clipEnabled;
    bool allClipped;
    bool hasPen;
    bool hasBrush;
    bool simplePen;
    qreal opacity;

    QHash<QFontEngine_FaceId, QFontSubset *> fonts;
    QPaintDevice *pdev;

    // the device the output is in the end streamed to.
    QIODevice *outDevice;
    bool ownsDevice;

    // printer options
    QString outputFileName;
    QString title;
    QString creator;
    bool embedFonts;
    int resolution;
    bool grayscale;

    // Page layout: size, orientation and margins
    QPageLayout m_pageLayout;

private:
    int gradientBrush(const QBrush &b, const QTransform &matrix, int *gStateObject);
    int generateGradientShader(const QGradient *gradient, const QTransform &matrix, bool alpha = false);
    int generateLinearGradientShader(const QLinearGradient *lg, const QTransform &matrix, bool alpha);
    int generateRadialGradientShader(const QRadialGradient *gradient, const QTransform &matrix, bool alpha);
    int createShadingFunction(const QGradient *gradient, int from, int to, bool reflect, bool alpha);

    void writeInfo();
    void writePageRoot();
    void writeFonts();
    void embedFont(QFontSubset *font);

    QVector<int> xrefPositions;
    QDataStream* stream;
    int streampos;

    int writeImage(const QByteArray &data, int width, int height, int depth,
                   int maskObject, int softMaskObject, bool dct = false, bool isMono = false);
    void writePage();

    int addXrefEntry(int object, bool printostr = true);
    void printString(const QString &string);
    void xprintf(const char* fmt, ...);

    inline void write(const QByteArray &data) {
        stream->writeRawData(data.constData(), data.size());
        streampos += data.size();
    }

    int writeCompressed(const char *src, int len);
    inline int writeCompressed(const QByteArray &data) { return writeCompressed(data.constData(), data.length()); }
    int writeCompressed(QIODevice *dev);

    // various PDF objects
    int pageRoot, catalog, info, graphicsState, patternColorSpace;
    QVector<uint> pages;
    QHash<qint64, uint> imageCache;
    QHash<QPair<uint, uint>, uint > alphaCache;
};

#endif // QT_NO_PDF

#endif // QPDF_P_H

