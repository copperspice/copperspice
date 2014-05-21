/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the documentation of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Nokia Corporation and its Subsidiary(-ies) nor
**     the names of its contributors may be used to endorse or promote
**     products derived from this software without specific prior written
**     permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QtGui>

#include <QAudioOutput>
#include <QAudioDeviceInfo>
#include <QAudioInput>

class Window2 : public QWidget
{
    Q_OBJECT

public:

//![0]
    void startRecording()
    {
        outputFile.setFileName("/tmp/test.raw");
        outputFile.open( QIODevice::WriteOnly | QIODevice::Truncate );

        QAudioFormat format;
        // set up the format you want, eg.
        format.setFrequency(8000);
        format.setChannels(1);
        format.setSampleSize(8);
        format.setCodec("audio/pcm");
        format.setByteOrder(QAudioFormat::LittleEndian);
        format.setSampleType(QAudioFormat::UnSignedInt);

        QAudioDeviceInfo info = QAudioDeviceInfo::defaultInputDevice();
        if (!info.isFormatSupported(format)) {
            qWarning()<<"default format not supported try to use nearest";
            format = info.nearestFormat(format);
        }

        audioInput = new QAudioInput(format, this);
        QTimer::singleShot(3000, this, SLOT(stopRecording()));
        audioInput->start(&outputFile);
        // Records audio for 3000ms
    }
//![0]

//![1]
    void stopRecording()
    {
        audioInput->stop();
        outputFile.close();
        delete audioInput;
    }
//![1]

public slots:
//![2]
    void stateChanged(QAudio::State newState)
    {
        switch(newState) {
            case QAudio::StoppedState:
            if (audioInput->error() != QAudio::NoError) {
                 // Perform error handling
            } else {

            }
            break;
//![2]
        default:
            ;
        }
    }

private:
//![3]
    QFile outputFile;         // class member.
    QAudioInput *audioInput;  // class member.
//![3]
};


class Window : public QWidget
{
    Q_OBJECT

public:
    Window()
    {
        audioOutput = new QAudioOutput;
        connect(audioOutput, SIGNAL(stateChanged(QAudio::State)),
            this, SLOT(stateChanged(QAudio::State)));
    }

public:

//![4]
    void startPlaying()
    {
        inputFile.setFileName("/tmp/test.raw");
        inputFile.open(QIODevice::ReadOnly);

        QAudioFormat format;
        // Set up the format, eg.
        format.setFrequency(8000);
        format.setChannels(1);
        format.setSampleSize(8);
        format.setCodec("audio/pcm");
        format.setByteOrder(QAudioFormat::LittleEndian);
        format.setSampleType(QAudioFormat::UnSignedInt);

        QAudioDeviceInfo info(QAudioDeviceInfo::defaultOutputDevice());
        if (!info.isFormatSupported(format)) {
            qWarning()<<"raw audio format not supported by backend, cannot play audio.";
            return;
        }

        audioOutput = new QAudioOutput(format, this);
        connect(audioOutput,SIGNAL(stateChanged(QAudio::State)),SLOT(finishedPlaying(QAudio::State)));
        audioOutput->start(&inputFile);
    }
//![4]

//![5]
    void finishedPlaying(QAudio::State state)
    {
        if (state == QAudio::IdleState) {
            audioOutput->stop();
            inputFile.close();
            delete audioOutput;
        }
    }
//![5]

private:

    void setupFormat()
    {
//![6]
        QAudioFormat format;
        format.setFrequency(44100);
//![6]
        format.setChannels(2);
        format.setSampleSize(16);
        format.setCodec("audio/pcm");
        format.setByteOrder(QAudioFormat::LittleEndian);
//![7]
        format.setSampleType(QAudioFormat::SignedInt);

        QAudioDeviceInfo info(QAudioDeviceInfo::defaultOutputDevice());

        if (!info.isFormatSupported(format))
            format = info.nearestFormat(format);
//![7]
    }

public slots:
//![8]
    void stateChanged(QAudio::State newState)
    {
        switch (newState) {
            case QAudio::StoppedState:
                if (audioOutput->error() != QAudio::NoError) {
                    // Perform error handling
                } else {
                    // Normal stop
                }
                break;
//![8]

            // Handle 
            case QAudio::ActiveState:
                // Handle active state...
                break;
            break;
        default:
            ;
        }
    }

private:

//![9]
    QFile inputFile;           // class member.
    QAudioOutput *audioOutput; // class member.
//![9]
};

int main(int argv, char **args)
{
    QApplication app(argv, args);

    Window window;
    window.show();

    return app.exec();        
}


#include "main.moc"

