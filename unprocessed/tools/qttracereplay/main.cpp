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

#include <QtGui>
#include <QtDebug>

#include <private/qpaintengineex_p.h>
#include <private/qpaintbuffer_p.h>

class ReplayWidget : public QWidget
{
    Q_OBJECT
public:
    ReplayWidget(const QString &filename, int from, int to, bool single, int frame);

    void paintEvent(QPaintEvent *event);
    void resizeEvent(QResizeEvent *event);

public slots:
    void updateRect();

public:
    QList<QRegion> updates;
    QPaintBuffer buffer;

    int currentFrame;
    int currentIteration;
    QTime timer;

    QList<uint> visibleUpdates;

    QVector<uint> iterationTimes;
    QString filename;

    int from;
    int to;

    bool single;

    int frame;
    int currentCommand;
};

void ReplayWidget::updateRect()
{
    if (frame >= 0 && !updates.isEmpty())
        update(updates.at(frame));
    else if (!visibleUpdates.isEmpty())
        update(updates.at(visibleUpdates.at(currentFrame)));
}

const int singleFrameRepeatsPerCommand = 100;
const int singleFrameIterations = 4;

void ReplayWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);

    QTimer::singleShot(0, this, SLOT(updateRect()));

//    p.setClipRegion(frames.at(currentFrame).updateRegion);

    if (frame >= 0) {
        int start = buffer.frameStartIndex(frame);
        int end = buffer.frameEndIndex(frame);

        iterationTimes.resize(end - start);

        int saveRestoreStackDepth = buffer.processCommands(&p, start, start + currentCommand);

        for (int i = 0; i < saveRestoreStackDepth; ++i)
            p.restore();

        const int repeats = currentIteration >= 3 ? singleFrameRepeatsPerCommand : 1;

        ++currentFrame;
        if (currentFrame == repeats) {
            currentFrame = 0;
            if (currentIteration >= 3) {
                iterationTimes[currentCommand - 1] = qMin(iterationTimes[currentCommand - 1], uint(timer.elapsed()));
                timer.restart();
            }

            if (currentIteration >= singleFrameIterations + 3) {
                printf(" #    | ms      | description\n");
                printf("------+---------+------------------------------------------------------------\n");

		qSort(iterationTimes);

		int sum = 0;
                for (int i = 0; i < iterationTimes.size(); ++i) {
                    int delta = iterationTimes.at(i);
                    if (i > 0)
                        delta -= iterationTimes.at(i-1);
		    sum += delta;
                    qreal deltaF = delta / qreal(repeats);
                    printf("%.5d | %.5f | %s\n", i, deltaF, qPrintable(buffer.commandDescription(start + i)));
                }
                printf("Total | %.5f | Total frame time\n", sum / qreal(repeats));
                deleteLater();
                return;
            }

            if (start + currentCommand >= end) {
                currentCommand = 1;
		++currentIteration;
                if (currentIteration == 3) {
                    timer.start();
                    iterationTimes.fill(uint(-1));
                }
		if (currentIteration >= 3 && currentIteration < singleFrameIterations + 3)
                    printf("Profiling iteration %d of %d\n", currentIteration - 2, singleFrameIterations);
            } else {
                ++currentCommand;
	    }
        }

        return;
    }

    buffer.draw(&p, visibleUpdates.at(currentFrame));

    ++currentFrame;
    if (currentFrame >= visibleUpdates.size()) {
        currentFrame = 0;
        ++currentIteration;

        if (single) {
            deleteLater();
            return;
        }

        if (currentIteration == 3)
            timer.start();
        else if (currentIteration > 3) {
            iterationTimes << timer.elapsed();
            timer.restart();

            if (iterationTimes.size() >= 3) {
                qreal mean = 0;
                qreal stddev = 0;
                uint min = INT_MAX;

                for (int i = 0; i < iterationTimes.size(); ++i) {
                    mean += iterationTimes.at(i);
                    min = qMin(min, iterationTimes.at(i));
                }

                mean /= qreal(iterationTimes.size());

                for (int i = 0; i < iterationTimes.size(); ++i) {
                    qreal delta = iterationTimes.at(i) - mean;
                    stddev += delta * delta;
                }

                stddev = qSqrt(stddev / iterationTimes.size());

                qSort(iterationTimes.begin(), iterationTimes.end());
                uint median = iterationTimes.at(iterationTimes.size() / 2);

                stddev = 100 * stddev / mean;

                if (iterationTimes.size() >= 10 || stddev < 4) {
                    printf("%s, iterations: %d, frames: %d, min(ms): %d, median(ms): %d, stddev: %f %%, max(fps): %f\n", qPrintable(filename),
                            iterationTimes.size(), visibleUpdates.size(), min, median, stddev, 1000. * visibleUpdates.size() / min);
                    deleteLater();
                    return;
                }
            }
        }
    }
}

void ReplayWidget::resizeEvent(QResizeEvent *)
{
    visibleUpdates.clear();

    QRect bounds = rect();

    int first = qMax(0, from);
    int last = qMin(unsigned(to), unsigned(updates.size()));
    for (int i = first; i < last; ++i) {
        if (updates.at(i).intersects(bounds))
            visibleUpdates << i;
    }

    int range = last - first;

    if (visibleUpdates.size() != range)
        printf("Warning: skipped %d frames due to limited resolution\n", range - visibleUpdates.size());

}

ReplayWidget::ReplayWidget(const QString &filename_, int from_, int to_, bool single_, int frame_)
    : currentFrame(0)
    , currentIteration(0)
    , filename(filename_)
    , from(from_)
    , to(to_)
    , single(single_)
    , frame(frame_)
    , currentCommand(1)
{
    setWindowTitle(filename);
    QFile file(filename);

    if (!file.open(QIODevice::ReadOnly)) {
        printf("Failed to load input file '%s'\n", qPrintable(filename_));
        return;
    }

    QDataStream in(&file);

    char *data;
    uint size;
    in.readBytes(data, size);
    bool isTraceFile = size >= 7 && qstrncmp(data, "qttrace", 7) == 0;

    uint version = 0;
    if (size == 9 && qstrncmp(data, "qttraceV2", 9) == 0) {
        in.setFloatingPointPrecision(QDataStream::SinglePrecision);
        in >> version;
    }

    if (!isTraceFile) {
        printf("File '%s' is not a trace file\n", qPrintable(filename_));
        return;
    }

    in >> buffer >> updates;
    printf("Read paint buffer version %d with %d frames\n", version, buffer.numFrames());

    resize(buffer.boundingRect().size().toSize());

    setAutoFillBackground(false);
    setAttribute(Qt::WA_NoSystemBackground);

    QTimer::singleShot(10, this, SLOT(updateRect()));
}

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    if (argc <= 1 || qstrcmp(argv[1], "-h") == 0 || qstrcmp(argv[1], "--help") == 0) {
        printf("Replays a tracefile generated with '-graphicssystem trace'\n");
        printf("Usage:\n  > %s [OPTIONS] [traceFile]\n", argv[0]);
        printf("OPTIONS\n"
               "   --range=from-to to specify a frame range.\n"
               "   --singlerun to do only one run (without statistics)\n"
               "   --instrumentframe=frame to instrument a single frame\n");
        return 1;
    }

    QFile file(app.arguments().last());
    if (!file.exists()) {
        printf("%s does not exist\n", qPrintable(app.arguments().last()));
        return 1;
    }

    bool single = false;

    int frame = -1;

    int from = 0;
    int to = -1;
    for (int i = 1; i < app.arguments().size() - 1; ++i) {
        QString arg = app.arguments().at(i);
        if (arg.startsWith(QLatin1String("--range="))) {
            QString rest = arg.mid(8);
            QStringList components = rest.split(QLatin1Char('-'));

            bool ok1 = false;
            bool ok2 = false;
            int fromCandidate = 0;
            int toCandidate = 0;
            if (components.size() == 2) {
                fromCandidate = components.first().toInt(&ok1);
                toCandidate = components.last().toInt(&ok2);
            }

            if (ok1 && ok2) {
                from = fromCandidate;
                to = toCandidate;
            } else {
                printf("ERROR: malformed syntax in argument %s\n", qPrintable(arg));
            }
        } else if (arg == QLatin1String("--singlerun")) {
            single = true;
        } else if (arg.startsWith(QLatin1String("--instrumentframe="))) {
            QString rest = arg.mid(18);
            bool ok = false;
            int frameCandidate = rest.toInt(&ok);
            if (ok) {
                frame = frameCandidate;
            } else {
                printf("ERROR: malformed syntax in argument %s\n", qPrintable(arg));
            }
        } else {
            printf("Unrecognized argument: %s\n", qPrintable(arg));
            return 1;
        }
    }

    ReplayWidget *widget = new ReplayWidget(app.arguments().last(), from, to, single, frame);

    if (!widget->updates.isEmpty()) {
        widget->show();
        return app.exec();
    }

}
#include "main.moc"
