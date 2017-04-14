#ifndef JSONWAX_SPEEDTEST_H
#define JSONWAX_SPEEDTEST_H

#include <QDir>
#include <QDirIterator>
#include <QElapsedTimer>
#include <QFile>
#include <QJsonDocument>
#include <QTextStream>
#include <QList>
#include <QDebug>
#include "JsonWax.h"

namespace JsonWaxInternals {

class Speedtest
{
public:

    static QList<QString> searchInDirectory(QString searchDirectory, QStringList fileFilter)
    {
        QDir dir = searchDirectory;

        if (dir.exists() == false)
            return QList<QString>();

        QList<QString> foundFiles;

        QDirIterator directories(searchDirectory, QDir::Files | QDir::NoSymLinks | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
        while(directories.hasNext())
        {
            directories.next();

            if (fileFilter.contains( directories.fileInfo().suffix(), Qt::CaseInsensitive)) // test each file suffix
            {
                QString path (directories.fileInfo().absoluteFilePath());
                foundFiles.append( path);
            }
        }
        return foundFiles;
    }

    static void runTest()
    {
        // Search for json files.
        QString searchDirectory = qApp->applicationDirPath() + "/speedtest/";
        QStringList fileFilter;
        fileFilter << "json";
        QList<QString> fileNames = searchInDirectory( searchDirectory, fileFilter);

        // Load file contents.
        QList<QByteArray> fileContents;

        for (QString& fileName : fileNames)
        {
            QFile qfile( fileName);
            QTextStream in (&qfile);
            in.setCodec( "UTF-8");
            qfile.open( QIODevice::ReadOnly);
            fileContents.append( in.readAll().toUtf8());
        }

        //Measure time difference.
        int totalOwnTime = 0;
        int totalOwnFails = 0;
        int totalQtTime = 0;
        int totalQtFails = 0;
        int docNumber = 0;

        for (QByteArray& bytes : fileContents)
        {
            QElapsedTimer timer;
            QElapsedTimer timer2;

            JsonWax json;
            timer.start();                          // TIME START
            bool res1 = json.fromByteArray( bytes);
            totalOwnTime += timer.nsecsElapsed();   // TIME END

            if (res1 != true)
            {
                ++totalOwnFails;
                qDebug() << "It failed at doc: " << fileNames.at( docNumber);
                qDebug() << json.errorMsg();
            }

            QJsonDocument doc;
            QJsonParseError* isok = new QJsonParseError();
            timer2.start();                         // TIME START
            doc.fromJson( bytes, isok);
            totalQtTime += timer2.nsecsElapsed();   // TIME END

            if (isok->error != 0)
                ++totalQtFails;

            delete isok;

            ++docNumber;
        }

        qDebug() << "Results: " << fileContents.size() << "files.";
        qDebug() << "JsonWax FAILS: " << totalOwnFails;
        qDebug() << "JsonWax spent time: " << totalOwnTime << "ns";
        qDebug() << "Qt FAILS: " << totalQtFails;
        qDebug() << "Qt spent time: " << totalQtTime << "ns\n";
        qDebug() << "JsonWax vs qt: " << 100.0 * totalOwnTime / totalQtTime << "%";
    }
};
}

#endif // JSONWAX_SPEEDTEST_H
