#ifndef JSONWAX_UNIT_TESTS_H
#define JSONWAX_UNIT_TESTS_H

/* Original author: Nikolai S | https://github.com/doublejim
 *
 * You may use this file under the terms of any of these licenses:
 * GNU General Public License version 2.0       https://www.gnu.org/licenses/gpl-2.0.html
 * GNU General Public License version 3         https://www.gnu.org/licenses/gpl-3.0.html
 */

#include <QElapsedTimer>
#include <QDirIterator>
#include <QJsonDocument>
#include <QDebug>
#include <QMetaProperty>
#include <QVariant>
#include <QSize>
#include <QTimeZone>
#include <QLine>
#include <QObject>
#include <QDate>
#include <QColor>
#include <QImage>
#include <QFont>
#include <QJsonArray>
#include <QJsonObject>
#include "JsonWax.h"

namespace JsonWaxInternals {

// ------- QObjects for serializer tests -----------------------------

class SerializerClass1 : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString name MEMBER m_name)
    Q_PROPERTY(int superNumber MEMBER m_number)
    Q_PROPERTY(Enume1 walker MEMBER m_enume1)
    Q_PROPERTY(Enume2 fish MEMBER m_enume2)
    Q_PROPERTY(QDate bestDate MEMBER m_date)
    Q_PROPERTY(QColor coolColor MEMBER m_color)
    // Q_PROPERTY(QImage imageAttempt MEMBER m_image)          // Can't save its data.
    Q_ENUMS(Enume1 Enume2)                                  // Required to store the enums.

public:
    explicit SerializerClass1( QObject* parent = 0);

    enum Enume1 {Crab, Spider};
    enum Enume2 {Salmon, Trout, Mackerel};

private:
    QString m_name = "One\nWonderful Name";
    int m_number = 1900000;
    Enume1 m_enume1 = Crab;
    Enume2 m_enume2 = Trout;
    QDate m_date = QDate(1950,10,10);
    QColor m_color = QColor(15,16,16,255);
    // QImage m_image = QImage(512,512,QImage::Format_Mono);
};

class SerializerClass2 : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name WRITE setName)
    Q_PROPERTY(int superNumber READ number WRITE setNumber)
    Q_PROPERTY(Enume1 walker READ enume1 WRITE setEnume1)
    Q_PROPERTY(Enume2 fish READ enume2 WRITE setEnume2)
    Q_PROPERTY(QDate bestDate READ date WRITE setDate)
    Q_PROPERTY(QColor coolColor READ color WRITE setColor)
    Q_PROPERTY(QImage imageAttempt READ image WRITE setImage)           // Can't save anything.
    Q_ENUMS(Enume1 Enume2)                                              // Required to store the enums.

public:
    explicit SerializerClass2( QObject* parent = 0);

    enum Enume1 {Crab, Spider};
    enum Enume2 {Salmon, Trout, Mackerel};

    void setName( QString const& name)
    {
        m_name = name;
    }

    void setNumber(const int number)
    {
        m_number = number;
    }

    void setEnume1(Enume1 value)
    {
        m_enume1 = value;
    }

    void setEnume2(Enume2 value)
    {
        m_enume2 = value;
    }

    void setDate(const QDate& date)
    {
        m_date = date;
    }

    void setColor(const QColor& color)
    {
        m_color = color;
    }

    void setImage(const QImage& image)
    {
        m_image = image;
    }

    int number() const
    {
        return m_number;
    }

    Enume1 enume1() const
    {
        return m_enume1;
    }

    Enume2 enume2() const
    {
        return m_enume2;
    }

    QDate date() const
    {
        return m_date;
    }

    QString name() const
    {
        return m_name;
    }

    QColor color() const
    {
        return m_color;
    }

    QImage image() const
    {
        return m_image;
    }

private:
    QString m_name = "One\nWonderful Name";
    int m_number = 1900000;
    Enume1 m_enume1 = Crab;
    Enume2 m_enume2 = Trout;
    QDate m_date = QDate(1950,10,10);
    QColor m_color = QColor(15,16,16,255);
    QImage m_image = QImage(512,512,QImage::Format_Mono);
};

// ----------------------------------------------------------------

class Tests
{
public:
    enum Validity {INVALID, VALID};

private:
    static void run( QString& input, QString& expectedString, Validity expectedValidity, int& passCount, int& failCount, QString& description)
    {
        JsonWax json;
        bool isCorrect = json.fromByteArray( input.toUtf8());

        Validity result = INVALID;

        if (isCorrect)
            result = VALID;

        if (result == expectedValidity && json.toString(JsonWax::Compact) == expectedString)
        {
            ++passCount;
        } else {
            qDebug() << "Failed at: " << description;
            qDebug() << "output: " << json.toString(JsonWax::Compact);
            ++failCount;
        }

        if (!isCorrect)
            qDebug() << QString::number(passCount + failCount) << ": " << json.errorMsg();
    }

    static void checkWax( JsonWax& json, QString expectedString, QString& description, int& passCount, int& failCount)
    {
        bool isCorrect = (json.toString(JsonWax::Compact) == expectedString);

        if (isCorrect)
        {
            ++passCount;
        } else {
            qDebug() << "Failed at: " << description;
            qDebug() << "output: " << json.toString(JsonWax::Compact);
            ++failCount;
        }
    }

    static void checkWax( bool isCorrect, QString& description, int& passCount, int& failCount)
    {
        if (isCorrect)
        {
            ++passCount;
        } else {
            qDebug() << "Failed at: " << description;
            ++failCount;
        }
    }

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

public:
    static void speedTest()
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

        {
            // Measure time difference.
            int totalJsonWaxTime = 0;
            int totalJsonWaxFails = 0;
            int totalQtTime = 0;
            int totalQtFails = 0;
            int docNumber = 0;

            for (QByteArray& bytes : fileContents)
            {
                QElapsedTimer timer;
                QElapsedTimer timer2;

                JsonWax json;
                timer.start();                              // TIME START
                bool res1 = json.fromByteArray( bytes);
                totalJsonWaxTime += timer.nsecsElapsed();   // TIME END

                if (res1 != true)
                {
                    ++totalJsonWaxFails;
                    qDebug() << "It failed at doc: " << fileNames.at( docNumber);
                    qDebug() << json.errorMsg();
                }

                QJsonDocument doc;
                QJsonParseError* isok = new QJsonParseError();
                timer2.start();                             // TIME START
                doc.fromJson( bytes, isok);
                totalQtTime += timer2.nsecsElapsed();       // TIME END

                if (isok->error != 0)
                    ++totalQtFails;

                delete isok;

                ++docNumber;
            }

            qDebug() << "----- Parsing/internalizing speed -----";
            qDebug() << "Read:" << fileContents.size() << "files.";
            qDebug() << "JsonWax invalid doc count:" << totalJsonWaxFails;
            qDebug() << "JsonWax spent time:" << totalJsonWaxTime * 1e-6<< "ms";
            qDebug() << "Qt invalid doc count:" << totalQtFails;
            qDebug() << "Qt spent time:" << totalQtTime * 1e-6 << "ms";
            qDebug() << "JsonWax vs Qt:" << 100.0 * totalJsonWaxTime / totalQtTime << "%\n";
        }

        {   // CHANGING VALUES OF OBJECTS (DEPTH 0)
            JsonWax json;

            for (int i = 0; i < 20000; ++i)
                json.setValue({i}, 160);

            QString documentAsString = json.toString( JsonWax::Compact);

            // Qt change value of elements.
            int qtTimeSpent = 0;
            QJsonDocument qtjson = QJsonDocument::fromJson( documentAsString.toUtf8() );
            QElapsedTimer timer;
            timer.start();
            QJsonArray array = qtjson.array();

            for (int i = 0; i < array.size(); ++i)
                array.replace(i, QJsonValue(170));

            qtjson.setArray( array);
            qtTimeSpent = timer.nsecsElapsed();

            // JsonWax change value of elements.
            int jsonWaxTimeSpent = 0;
            QElapsedTimer timer2;
            timer2.start();
            for (int i = 0; i < json.size(); ++i)
                json.setValue({i}, 170);
            jsonWaxTimeSpent = timer2.nsecsElapsed();

            qDebug() << "----- Changing values (depth 0) speed -----";
            qDebug() << "JsonWax spent time:" << jsonWaxTimeSpent * 1e-6<< "ms";
            qDebug() << "Qt spent time:" << qtTimeSpent * 1e-6<< "ms";
            qDebug() << "JsonWax vs Qt:" << 100.0 * jsonWaxTimeSpent / qtTimeSpent << "%\n";
        }

        {   // CHANGING VALUES OF OBJECTS (DEPTH 2)
            JsonWax json;

            for (int i = 0; i < 20000; ++i)
                json.setValue({"hello","world",i}, "one thing");

            QString documentAsString = json.toString( JsonWax::Compact);

            // Qt change value of elements.
            int qtTimeSpent = 0;
            QJsonDocument qtjson = QJsonDocument::fromJson( documentAsString.toUtf8() );
            QElapsedTimer timer;
            timer.start();
            QJsonObject obj = qtjson.object().value("hello").toObject();
            QJsonArray array = obj.value("world").toArray();

            for (int i = 0; i < array.size(); ++i)
                array.replace(i, QJsonValue("another thing"));

            obj.insert("world", array);
            QJsonObject obj_root;
            obj_root.insert("hello", obj);
            qtjson.setObject( obj_root);
            qtTimeSpent = timer.nsecsElapsed();

            // JsonWax change value of elements.
            int jsonWaxTimeSpent = 0;
            QElapsedTimer timer2;
            timer2.start();
            for (int i = 0; i < json.size({"hello","world"}); ++i)
                json.setValue({"hello","world",i}, "another thing");
            jsonWaxTimeSpent = timer2.nsecsElapsed();

            qDebug() << "----- Changing values (depth 2) speed -----";
            qDebug() << "JsonWax spent time:" << jsonWaxTimeSpent * 1e-6<< "ms";
            qDebug() << "Qt spent time:" << qtTimeSpent * 1e-6<< "ms";
            qDebug() << "JsonWax vs Qt:" << 100.0 * jsonWaxTimeSpent / qtTimeSpent << "%\n";
        }

        {   // CHANGING VALUES OF OBJECTS (DEPTH 5)
            JsonWax json;

            for (int i = 0; i < 20000; ++i)
                json.setValue({"hello","world","this","is","a",i}, "false");

            QString documentAsString = json.toString( JsonWax::Compact);

            // Qt change value of elements.
            int qtTimeSpent = 0;
            QJsonDocument qtjson = QJsonDocument::fromJson( documentAsString.toUtf8() );
            QElapsedTimer timer;
            timer.start();
            QJsonObject obj = qtjson.object().value("hello").toObject();
            QJsonObject obj2 = obj.value("world").toObject();
            QJsonObject obj3 = obj2.value("this").toObject();
            QJsonObject obj4 = obj3.value("is").toObject();
            QJsonArray array = obj4.value("a").toArray();

            for (int i = 0; i < array.size(); ++i)
                array.replace(i, QJsonValue(true));

            obj4.insert("a", array);
            obj3.insert("is", obj4);
            obj2.insert("this", obj3);
            obj.insert("world", obj2);
            QJsonObject obj_root;
            obj_root.insert("hello", obj);
            qtjson.setObject( obj_root);
            qtTimeSpent = timer.nsecsElapsed();

            // JsonWax change value of elements.
            int jsonWaxTimeSpent = 0;
            QElapsedTimer timer2;
            timer2.start();
            for (int i = 0; i < json.size({"hello","world","this","is","a"}); ++i)
                json.setValue({"hello","world","this","is","a",i}, true);

            jsonWaxTimeSpent = timer2.nsecsElapsed();

            qDebug() << "----- Changing values (depth 5) speed -----";
            qDebug() << "JsonWax spent time:" << jsonWaxTimeSpent * 1e-6<< "ms";
            qDebug() << "Qt spent time:" << qtTimeSpent * 1e-6<< "ms";
            qDebug() << "JsonWax vs Qt:" << 100.0 * jsonWaxTimeSpent / qtTimeSpent << "%\n";
        }

        {   // READING VALUES (DEPTH 0)
            JsonWax json;

            for (int i = 0; i < 20000; ++i)
                json.setValue({i}, 160);

            QString documentAsString = json.toString( JsonWax::Compact);

            // Qt change read of elements.
            int qtTimeSpent = 0;
            QJsonDocument qtjson = QJsonDocument::fromJson( documentAsString.toUtf8() );
            QElapsedTimer timer;
            timer.start();
            QJsonArray array = qtjson.array();

            bool value1 = false;
            for (int i = 0; i < array.size(); ++i)
                value1 = array.at(i).toBool();

            qtjson.setArray( array);
            qtTimeSpent = timer.nsecsElapsed();

            // JsonWax read value of elements.
            int jsonWaxTimeSpent = 0;
            QElapsedTimer timer2;
            timer2.start();
            bool value2 = false;
            for (int i = 0; i < json.size(); ++i)
                value2 = json.value({i}).toBool();
            jsonWaxTimeSpent = timer2.nsecsElapsed();

            // Pure QList read.
            int pureQListTimeSpent = 0;
            QList<QVariant> justAList;
            for (int i = 0; i < 20000; ++i)
                justAList.append(true);
            QElapsedTimer timer3;
            timer3.start();
            bool value3 = false;
            for (int i = 0; i < 20000; ++i)
                value3 = justAList.at(i).toBool();
            pureQListTimeSpent = timer3.nsecsElapsed();

            if (value1 + value2 + value3)                               // Just using it for something, to avoid "unused" warning.
            qDebug() << "----- Reading values (depth 0) speed -----";
            qDebug() << "JsonWax spent time:" << jsonWaxTimeSpent * 1e-6 << "ms";
            qDebug() << "Qt spent time:" << qtTimeSpent * 1e-6 << "ms";
            qDebug() << "pure QList spent time:" << pureQListTimeSpent * 1e-6 << "ms";
            qDebug() << "JsonWax vs Qt:" << 100.0 * jsonWaxTimeSpent / qtTimeSpent << "%\n";
        }

        {   // READING VALUES (DEPTH 5)
            JsonWax json;

            for (int i = 0; i < 20000; ++i)
                json.setValue({"hello","world","this","is","a",i}, "false");

            QString documentAsString = json.toString( JsonWax::Compact);

            // Qt change value of elements.
            int qtTimeSpent = 0;
            QJsonDocument qtjson = QJsonDocument::fromJson( documentAsString.toUtf8() );
            QElapsedTimer timer;
            timer.start();
            QJsonObject obj = qtjson.object().value("hello").toObject();
            QJsonObject obj2 = obj.value("world").toObject();
            QJsonObject obj3 = obj2.value("this").toObject();
            QJsonObject obj4 = obj3.value("is").toObject();
            QJsonArray array = obj4.value("a").toArray();

            bool value1 = false;
            for (int i = 0; i < array.size(); ++i)
                value1 = array.at(i).toBool();

            obj4.insert("a", array);
            obj3.insert("is", obj4);
            obj2.insert("this", obj3);
            obj.insert("world", obj2);
            QJsonObject obj_root;
            obj_root.insert("hello", obj);
            qtjson.setObject( obj_root);
            qtTimeSpent = timer.nsecsElapsed();

            // JsonWax change value of elements.
            int jsonWaxTimeSpent = 0;
            QElapsedTimer timer2;
            timer2.start();
            bool value2 = false;
            for (int i = 0; i < json.size({"hello","world","this","is","a"}); ++i)
                value2 = json.value({"hello","world","this","is","a",i}).toBool();

            jsonWaxTimeSpent = timer2.nsecsElapsed();

            if (value1 + value2 == false)                               // Just using it for something, to avoid "unused" warning.
            qDebug() << "----- Reading values (depth 5) speed -----";
            qDebug() << "JsonWax spent time:" << jsonWaxTimeSpent * 1e-6<< "ms";
            qDebug() << "Qt spent time:" << qtTimeSpent * 1e-6<< "ms";
            qDebug() << "JsonWax vs Qt:" << 100.0 * jsonWaxTimeSpent / qtTimeSpent << "%\n";
        }

        {   // SERIALIZE TO BASE64 BYTE ARRAY.
            QList<QRect> list;
            for (int i = 0; i < 20000; ++i)
                list.append( QRect(15,16,16,16));

            // Serialize
            JsonWax json;
            QElapsedTimer timer;
            int jsonWaxTimeSpent1 = 0;
            timer.start();
            json.serializeToBytes({0}, list);
            jsonWaxTimeSpent1 = timer.nsecsElapsed();

            // Deserialize
            int jsonWaxTimeSpent2 = 0;
            QElapsedTimer timer2;
            timer2.start();
            list = json.deserializeBytes<QList<QRect>>({0});
            jsonWaxTimeSpent2 = timer2.nsecsElapsed();

            qDebug() << "----- serializeToBytes speed -----";
            qDebug() << "Serialize JsonWax spent time: " << jsonWaxTimeSpent1 * 1e-6 << "ms";
            qDebug() << "Deserialize JsonWax spent time: " << jsonWaxTimeSpent2 * 1e-6 << "ms\n";
        }

        {   // SERIALIZE TO JSON.
            QList<QDateTime> dateTimeList;
            for (int i = 0; i < 20000; ++i)
                dateTimeList.append( QDateTime::currentDateTime());

            JsonWax json;
            QElapsedTimer timer;
            int jsonWaxTimeSpent = 0;
            timer.start();
            json.serializeToJson({}, dateTimeList);
            jsonWaxTimeSpent = timer.nsecsElapsed();
            qDebug() << "----- serializeToJson speed -----";
            qDebug() << "Serialize JsonWax spent time: " << jsonWaxTimeSpent * 1e-6 << "ms";

            QElapsedTimer timer2;
            timer2.start();
            dateTimeList = json.deserializeJson<QList<QDateTime>>({});
            jsonWaxTimeSpent = timer2.nsecsElapsed();
            qDebug() << "Deserialize JsonWax spent time: " << jsonWaxTimeSpent * 1e-6 << "ms\n";
        }

        {   // COPY
            JsonWax json;
            for (int i = 0; i < 20000; ++i)
                json.setValue({"a",i}, i);
            QElapsedTimer timer;
            timer.start();
            json.copy({"a"},{"b"});
            int jsonWaxTimeSpent = timer.nsecsElapsed();
            qDebug() << "----- Copy speed -----";
            qDebug() << "JsonWax copy spent time: " << jsonWaxTimeSpent * 1e-6 << "ms";
        }
    }

    static void parserPositiveTests() // ============================================================
    {
        int passCount = 0;
        int failCount = 0;
        qDebug() << "----- Error messages from positive tests -----";

        {
            QString input = "{\n\n"
                            "   \"hello\"  \r\n :\n   \"omega\" \t\n"
                            "}";
            QString expectedString = "{\"hello\":\"omega\"}";
            QString description = "String key and string value in an object, with newlines, tabs and spaces.";
            run( input, expectedString, VALID, passCount, failCount, description);
        }

        {
            QString input = "{"
                            "   \"obj1\":{},\"obj2\":[] \n"
                            "}";
            QString expectedString = "{\"obj1\":{},\"obj2\":[]}";
            QString description = "Empty object and array.";
            run( input, expectedString, VALID, passCount, failCount, description);
        }

        {
            QString input = "{\n"
                            "   \"filo\"  : \"omega\","
                            "   \"topia\" :\n15455,"
                            "   \"thing\" :   15.6486,"
                            "   \"tpk\"   :   true,"
                            "   \"uf\"    :   false,"
                            "   \"v\"     :   null"
                            "  \n\n}";
            QString expectedString = "{\"filo\":\"omega\",\"thing\":15.6486,\"topia\":15455,\"tpk\":true,"
                                     "\"uf\":false,\"v\":null}";
            QString description = "Six keys with six types of values in an object.";
            run( input, expectedString, VALID, passCount, failCount, description);
        }

        {
            QString input = "{\n"
                            "   \"array\" : [ \"hello\", 15656, 12.1, null, \"things\"]"
                            "}";
            QString expectedString = "{\"array\":[\"hello\",15656,12.1,null,\"things\"]}";
            QString description = "Object containing array containing various types of values.";
            run( input, expectedString, VALID, passCount, failCount, description);
        }

        {
            QString input = "{"
                            "\"0\":"
                            "   {\"fili\": [ 15,1150,200,null,84] },"
                            "\"grand\": \"scale\""
                            "}";
            QString expectedString = "{\"0\":{\"fili\":[15,1150,200,null,84]},\"grand\":\"scale\"}";
            QString description = "Array in object in object.";
            run( input, expectedString, VALID, passCount, failCount, description);
        }

        {
            QString input = "{}";
            QString expectedString = "{}";
            QString description = "The most minimal acceptable object.";
            run( input, expectedString, VALID, passCount, failCount, description);
        }

        {
            QString input = "{ \"hejsa\" : 15.1512, \"mega\": 15.545e-1531, \"don\": 0.1653}";
            QString expectedString = "{\"don\":0.1653,\"hejsa\":15.1512,\"mega\":0}";
            QString description = "Various allowed numbers.";
            run( input, expectedString, VALID, passCount, failCount, description);
            // Note: extreme values like 15.545e-1531 cannot be represented.
            // It is stored as a 0.
        }

        {
            QString input = "[[[[[[[[[[[[[[[[[[[[[[[[[[[[[]]]]]]]]]]]]]]]]]]]]]]]]]]]]]";
            QString expectedString = "[[[[[[[[[[[[[[[[[[[[[[[[[[[[[]]]]]]]]]]]]]]]]]]]]]]]]]]]]]";
            QString description = "Very nested arrays.";
            run( input, expectedString, VALID, passCount, failCount, description);
        }

        {
            QString input = "{ \"he\rjsa\" :"
                            "   [1512, 12.1, 16.8, "
                            "       {\"1蕤\t\n\b\f465\r\": { \"this\": \"is a test\"}}"
                            "   ],"
                            "   \"ᄡok\":{\"meᄡga\" : [ 14.6, \"\t\bhi th伒1en\re\"]"
                            "   }"
                            "}";
            QString expectedString = "{\"he\\rjsa\":[1512,12.1,16.8,{\"1蕤\\t\\n\\b\\f465\\r\":{\"this\":\"is a test\"}}"
                                     "],\"ᄡok\":{\"meᄡga\":[14.6,\"\\t\\bhi th伒1en\\re\"]}}";
            QString description = "Special characters in keys and values, arrays and objects.";
            run( input, expectedString, VALID, passCount, failCount, description);
        }

        {
            QString input = "{ \"15156\" : \"12me\\b\nga\rhej\", \"6\t465\\t4\"  :\"fin\b\\b\"}";
            QString expectedString = "{\"15156\":\"12me\\b\\nga\\rhej\",\"6\\t465\\t4\":\"fin\\b\\b\"}";
            QString description = "Various special characters.";
            run( input, expectedString, VALID, passCount, failCount, description);
        }

        {
            QString input = "{ \" \n \\n \" : \"12mende\nr\\nfor\"}";
            QString expectedString = "{\" \\n \\n \":\"12mende\\nr\\nfor\"}";
            QString description = "Newline as one and two characters, converted to the same two characters.";
            run( input, expectedString, VALID, passCount, failCount, description);
        }

        {
            QString input = "[{\"aa\\/\b\f\n\r\ta\": {\"aa\\/\b\f\n\r\ta\": \"aa\\/\b\f\n\r\ta\"}}]";
            QString expectedString = "[{\"aa/\\b\\f\\n\\r\\ta\":{\"aa/\\b\\f\\n\\r\\ta\":\"aa/\\b\\f\\n\\r\\ta\"}}]";
            QString description = "Escaped characters in keys and value";
            run( input, expectedString, VALID, passCount, failCount, description);
            // Note: JsonWax stores the character '\n' and the string "\n" as the SAME
            // character: '\n' internally. When written to a file, '\n' is converted
            // to the two characters: "\n". This is to prevent strings from distorting
            // the document, when viewing it in a text editor. This goes for all escaped
            // characters: \, ", \b, \f, \n, \r, \t
            // That is the intended purpose of being able to escape characters.
        }

        {
            QString input = "{  \"1\": \"\","
                            "   \"2\": \"\b\","
                            "   \"3\": \"\f\","
                            "   \"4\": \"\n\","
                            "   \"\": \"\r\","
                            "   \"6\": \"\t\""
                            "}";
            QString expectedString = "{\"\":\"\\r\",\"1\":\"\",\"2\":\"\\b\",\"3\":\"\\f\",\"4\":"
                                     "\"\\n\",\"6\":\"\\t\"}";
            QString description = "Short keys and values with escaped characters.";
            run( input, expectedString, VALID, passCount, failCount, description);
        }

        {
            QString input = "[ 5e15, -554e1, -0.4E-67, 0.94, 7565E+56, 1E+0, -0,"
                            "  -0.0, -0.1358, -8484682248465, 0.9876543210E+2, 9870e-1,"
                            "  0, 1234567890.0123456, 123.0123456789]";
            QString expectedString = "[5e+15,-5540,-4e-68,0.94,7.565e+59,1,0,0,-0.1358,-8484682248465,"
                                     "98.7654321,987,0,1234567890.0123456,123.0123456789]";
            QString description = "Valid numbers.";
            run( input, expectedString, VALID, passCount, failCount, description);
            // Note: QVariant.toString() will introduce a + after the e.
            // It makes various changes in the representation.
        }

        {
            QString input = "[{\"a\\u8564b\\u4f12c\\u1121d\" : \"a\\u8564b\\u4f12c\\u1121d\"},\"a\\u8564b\\u4f12c\\u1121d\"]";
            QString expectedString = "[{\"a蕤b伒cᄡd\":\"a蕤b伒cᄡd\"},\"a蕤b伒cᄡd\"]";
            QString description = "Unicode code points are interpreted as their respective characters in key and value.";
            run( input, expectedString, VALID, passCount, failCount, description);
        }

        {   // Number type interpretation.
            JsonWax json;
            json.fromByteArray("{\"test1\":15,\"test2\":16.5,\"test3\":15e0,\"test4\":15e-2,\"test5\":1234567890,"
                               "\"test6\":12345678901,\"test7\":12345678901e0,\"test8\":12345678901e80,"
                               "\"test9\":15.6e+46,\"test10\":2147483647,\"test11\":2147483648,"
                               "\"test12\":-2147483647,\"test13\":-2147483648}");
            QString description = "Number type interpretation.";
            checkWax((QString(json.value({"test1"}).typeName()) == "int"), description, passCount, failCount);
            checkWax((QString(json.value({"test2"}).typeName()) == "double"), description, passCount, failCount);
            checkWax((QString(json.value({"test3"}).typeName()) == "double"), description, passCount, failCount);
            checkWax((QString(json.value({"test4"}).typeName()) == "double"), description, passCount, failCount);
            checkWax((QString(json.value({"test5"}).typeName()) == "int"), description, passCount, failCount);
            checkWax((QString(json.value({"test6"}).typeName()) == "qlonglong"), description, passCount, failCount);
            checkWax((QString(json.value({"test7"}).typeName()) == "double"), description, passCount, failCount);
            checkWax((QString(json.value({"test8"}).typeName()) == "double"), description, passCount, failCount);
            checkWax((QString(json.value({"test9"}).typeName()) == "double"), description, passCount, failCount);
            checkWax((QString(json.value({"test10"}).typeName()) == "int"), description, passCount, failCount);
            checkWax((QString(json.value({"test11"}).typeName()) == "qlonglong"), description, passCount, failCount);
            checkWax((QString(json.value({"test12"}).typeName()) == "int"), description, passCount, failCount);
            checkWax((QString(json.value({"test13"}).typeName()) == "qlonglong"), description, passCount, failCount);
        }

        qDebug() << "---------------------------------------------";
        qDebug() << "=====    Parser: positive tests PASSED: " << passCount;
        qDebug() << "=====    Parser: positive tests FAILED: " << failCount;
    }

    static void parserNegativeTests() // ============================================================
    {
        int passCount = 0;
        int failCount = 0;

        qDebug() << "----- Error messages from negative test -----";

        {
            QString input = "{\n\"filo\"  : \"omega\",\"topia\" :\n\"hejsan\",\n}";
            QString expectedString = "{\"filo\":\"omega\",\"topia\":\"hejsan\"}";
            QString description = "Ending with a comma is not allowed.";
            run( input, expectedString, INVALID, passCount, failCount, description);
        }

        {
            QString input = "{\n\"filo\"  : \"omega\",\"topia\" :\n\"hejsan\"\n}}";
            QString expectedString = "{\"filo\":\"omega\",\"topia\":\"hejsan\"}";
            QString description = "Too many curly braces.";
            run( input, expectedString, INVALID, passCount, failCount, description);
        }

        {
            QString input = "\n\"john\"  : \"two\"";
            QString expectedString = "{}"; // Can't read any values.
            QString description = "Missing curly braces.";
            run( input, expectedString, INVALID, passCount, failCount, description);
        }

        {
            QString input = "{\n\"john\"  : 15.4.}";
            QString expectedString = "{\"john\":15.4}";
            QString description = "Invalid number 1.";
            run( input, expectedString, INVALID, passCount, failCount, description);
        }

        {
            QString input = "[ .159 ]";
            QString expectedString = "{}"; // Can't read the value.
            QString description = "Invalid number 2.";
            run( input, expectedString, INVALID, passCount, failCount, description);
        }

        {
            QString input = "{\n\"john\"  : -0ee0}";
            QString expectedString = "{}";
            QString description = "Invalid number 3.";
            run( input, expectedString, INVALID, passCount, failCount, description);
        }

        {
            QString input = "{\n\"john\"  : 78.9.2}";
            QString expectedString = "{\"john\":78.9}";
            QString description = "Invalid number 4.";
            run( input, expectedString, INVALID, passCount, failCount, description);
        }

        {
            QString input = "{\n\"john\"  : ..123}";
            QString expectedString = "{}"; // Can't read the value.
            QString description = "Invalid number 5.";
            run( input, expectedString, INVALID, passCount, failCount, description);
        }

        {
            QString input = "[ e12]";
            QString expectedString = "{}"; // Can't read the value.
            QString description = "Invalid number 6.";
            run( input, expectedString, INVALID, passCount, failCount, description);
        }

        {
            QString input = "[ 5e1e5]";
            QString expectedString = "[50]";
            QString description = "Invalid number 7.";
            run( input, expectedString, INVALID, passCount, failCount, description);
            // Note: it reads 5e1, which is 50.
        }

        {
            QString input = "[ -.9]";
            QString expectedString = "{}";
            QString description = "Invalid number 8.";
            run( input, expectedString, INVALID, passCount, failCount, description);
        }

        {
            QString input = "[ -0.9ea+1]";
            QString expectedString = "{}";
            QString description = "Invalid number 9.";
            run( input, expectedString, INVALID, passCount, failCount, description);
        }

        {
            QString input = "[ -0.9E9f]";
            QString expectedString = "[-9e+8]";
            QString description = "Invalid number 10.";
            run( input, expectedString, INVALID, passCount, failCount, description);
        }

        {
            QString input = "[ -9.5165-1]";
            QString expectedString = "[-9.5165]";
            QString description = "Invalid number 11.";
            run( input, expectedString, INVALID, passCount, failCount, description);
        }

        {
            QString input = "[ -9.5165+1]";
            QString expectedString = "[-9.5165]";
            QString description = "Invalid number 12.";
            run( input, expectedString, INVALID, passCount, failCount, description);
        }

        {
            QString input = "[ 077 ]";
            QString expectedString = "[0]";
            QString description = "Invalid number 13.";
            run( input, expectedString, INVALID, passCount, failCount, description);
            // Note: in JSON, the only number starting with 0, is 0.
        }

        {
            QString input = "[ -1a ]";
            QString expectedString = "[-1]";
            QString description = "Invalid number 14.";
            run( input, expectedString, INVALID, passCount, failCount, description);
        }

        {
            QString input = "[ +1 ]";
            QString expectedString = "{}";
            QString description = "Invalid number 15.";
            run( input, expectedString, INVALID, passCount, failCount, description);
        }

        {
            QString input = "[ 1+1 ]";
            QString expectedString = "[1]";
            QString description = "Invalid number 16.";
            run( input, expectedString, INVALID, passCount, failCount, description);
        }

        {
            QString input = "[ 100e ]";
            QString expectedString = "{}";
            QString description = "Invalid number 17.";
            run( input, expectedString, INVALID, passCount, failCount, description);
        }

        {
            QString input = "{\n\"john\"  : /}";
            QString expectedString = "{}"; // Can't read any values.
            QString description = "Slash can't be a value (without quotes).";
            run( input, expectedString, INVALID, passCount, failCount, description);
        }

        {
            QString input = "{\n\"john\"  : 15.3e";
            QString expectedString = "{}"; // Can't read any values.
            QString description = "Unexpected end of document.";
            run( input, expectedString, INVALID, passCount, failCount, description);
        }

        {
            QString input = "{\n\"filo\"  : \"omega\",\"topia\" :\n\"hejsan\"\n}{}";
            QString expectedString = "{\"filo\":\"omega\",\"topia\":\"hejsan\"}";
            QString description = "Illegal second object.";
            run( input, expectedString, INVALID, passCount, failCount, description);
        }

        {
            QString input = "{\n\"this\":\"is\", \"my\":\"life\", \"now\":\"ok\"}[]";
            QString expectedString = "{\"my\":\"life\",\"now\":\"ok\",\"this\":\"is\"}";
            QString description = "Illegal array after object.";
            run( input, expectedString, INVALID, passCount, failCount, description);
        }

        {
            QString input = "[][]";
            QString expectedString = "[]";
            QString description = "Illegal array after array.";
            run( input, expectedString, INVALID, passCount, failCount, description);
            // Note: an empty array always becomes: {}
        }

        {
            QString input = "[]{}";
            QString expectedString = "[]";
            QString description = "Illegal object after array.";
            run( input, expectedString, INVALID, passCount, failCount, description);
        }

        {
            QString input = "{ \"hejsa\"}";
            QString expectedString = "{}";
            QString description = "Missing colon and value.";
            run( input, expectedString, INVALID, passCount, failCount, description);
        }

        {
            QString input = "{ \"hejsa\"  :}";
            QString expectedString = "{}";
            QString description = "Missing value.";
            run( input, expectedString, INVALID, passCount, failCount, description);
        }

        {
            QString input = "{ \"hejsa\" : 0.1512, \"mega\": 0545e+1531}";
            QString expectedString = "{\"hejsa\":0.1512,\"mega\":0}";
            QString description = "The second value is not allowed, because it starts with 0.";
            run( input, expectedString, INVALID, passCount, failCount, description);
        }

        {
            QString input = "{ \"hejsa\" : [\"1512\", \"12e1\", \"16.8\\u15431\\u2\"], \"mega\" : 14.6}";
            QString expectedString = "{\"hejsa\":[\"1512\",\"12e1\"]}";
            QString description = "One character long unicode code point not allowed.";
            run( input, expectedString, INVALID, passCount, failCount, description);
        }

        {
            QString input = "   [1512, 12.1, 16.8,"
                            "   \"1\\u8564\t\n\b\f465\r\": { \"this\": \"is a test\"}"
                            "   ],";
            QString expectedString = "[1512,12.1,16.8,\"1蕤\\t\\n\\b\\f465\\r\"]";
            QString description = "Object without curly braces.";
            run( input, expectedString, INVALID, passCount, failCount, description);
        }

        {
            QString input = "";
            QString expectedString = "{}";
            QString description = "Empty string is invalid. It must be an object or array.";
            run( input, expectedString, INVALID, passCount, failCount, description);
        }

        qDebug() << "---------------------------------------------";
        qDebug() << "=====    Parser: negative tests PASSED: " << passCount;
        qDebug() << "=====    Parser: negative tests FAILED: " << failCount;
    }

    static void editorTests() // ============================================================
    {
        int passCount = 0;
        int failCount = 0;
        QString description;

        qDebug() << "----- Error messages from Editor tests: -----";

        {
            qDebug() << "expects error message:";
            JsonWax json;
            json.setValue({"this","is","a","test"},"okay!");
            json.setValue({"this","is","a","shirt"},"no!");
            json.setValue({"this","is","a","shirt"},"yes!");    // Overwrites existing.
            json.setValue({"this","is","a","test",0}, 15);
            json.setValue({"this","is","a","test",2}, 4);
            json.setValue({"this","is","a","test",-1}, 20);     // Doesn't do anything.
            json.setValue({"this","is","a","test",0}, 16);      // Overwrites existing.
            QString expectedString = "{\"this\":{\"is\":{\"a\":"
                                     "{\"shirt\":\"yes!\","
                                     "\"test\":[16,null,4]}}}}";
            description = "setValue tests.";
            checkWax( json, expectedString, description, passCount, failCount);

            json.setValue({"this","is","a","test"},"truely");
            expectedString = "{\"this\":{\"is\":{\"a\":"
                              "{\"shirt\":\"yes!\","
                              "\"test\":\"truely\"}}}}";
            description = "setValue overwrite array 1.";
            checkWax( json, expectedString, description, passCount, failCount);

            json.setValue({"this","is","a",0},"nothing");
            expectedString = "{\"this\":{\"is\":{\"a\":"
                              "[\"nothing\"]}}}";
            description = "setValue overwrite object 1.";
            checkWax( json, expectedString, description, passCount, failCount);
        }

        {
            JsonWax json;
            json.setValue({"alpha"}, 12345678901234567890ULL ); // unsigned long long
            json.setValue({"beta"},QChar('h'));
            json.setValue({"gamma"},char('h'));
            json.setValue({"hef"},uint(5));
            json.setValue({"ief"},uchar(30));
            json.setValue({"joo"}, 1234567890123456789LL ); // long long
            QString expectedString = "{\"alpha\":12345678901234567890,\"beta\":\"h\",\"gamma\":104,\"hef\":5,\"ief\":30,\"joo\":1234567890123456789}";
            description = "setValue with unusual input.";
            checkWax( json, expectedString, description, passCount, failCount);
        }

        {
            JsonWax json;
            json.setValue({"hej"},"yes");
            json.setValue({2}, "no");
            QString expectedString = "[null,null,\"no\"]";
            description = "setValue overwrite object 2.";
            checkWax( json, expectedString, description, passCount, failCount);

            json.setValue({"you lose"}, "okay");
            expectedString = "{\"you lose\":\"okay\"}";
            description = "setValue overwrite array 2.";
            checkWax( json, expectedString, description, passCount, failCount);
        }

        {
            qDebug() << "expects error message:";
            JsonWax json;
            json.setValue({}, "salmon");
            QString expectedString = "{}";
            description = "Set root to value. Nothing should happen.";
            checkWax( json, expectedString, description, passCount, failCount);
        }

        {
            qDebug() << "expects error message:";
            JsonWax json;
            json.setValue({"this","is","a","test"},"okay!");
            json.setValue({"this","is","a","shirt"},"no!");
            json.setValue({"this","is","a","shirt"},"yes!");
            json.setValue({"this","is","a","test",0}, 15);
            json.setValue({"this","is","a","test",2}, 4);
            json.setValue({"this","is","a","test",-1}, 20);
            json.setValue({"this","is","a","test",0}, 16);
            json.copy({"this","is","a"},{"this"});
            QString expectedString = "{\"this\":{\"shirt\":\"yes!\",\"test\":[16,null,4]}}";
            description = "copy test 1.";
            checkWax( json, expectedString, description, passCount, failCount);
        }

        {
            JsonWax json;
            json.setValue({"agora",0},"okay!");
            json.setValue({"agora",2}, 168648468468486464LL); // long long
            json.setEmptyArray({"agora",3,"mister1"});
            json.setEmptyObject({"agora",4,"mister2"});
            json.setNull({"french","toast"});
            json.copy({},{"result"});
            QString expectedString = "{\"agora\":[\"okay!\",null,168648468468486464,{\"mister1\":[]},{\"mister2\":{}}],\"french\":{\"toast\":null},"
                                     "\"result\":{\"agora\":[\"okay!\",null,168648468468486464,{\"mister1\":[]},{\"mister2\":{}}],\"french\":{\"toast\":null}}}";
            description = "copy test 2. with empty objects and arrays";
            checkWax( json, expectedString, description, passCount, failCount);
        }

        {
            JsonWax json;
            json.setValue({"one"},"thing");
            json.copy({"one"},{});
            QString expectedString = "{\"one\":\"thing\"}";
            description = "this is impossible. copying a value to root. nothing should happen.";
            checkWax( json, expectedString, description, passCount, failCount);
        }

        {
            JsonWax json;
            json.setValue({"one"},"thing");
            json.move({"one"},{});
            QString expectedString = "{\"one\":\"thing\"}";
            description = "this is impossible. moving a value to root. nothing should happen.";
            checkWax( json, expectedString, description, passCount, failCount);
        }

        {
            JsonWax json;
            json.copy({"great",0},{"koala"});
            QString expectedString = "{}";
            description = "copy nothing shouldn't create anything.";
            checkWax( json, expectedString, description, passCount, failCount);
        }

        {
            JsonWax json;
            json.setValue({1, "det", "virker"}, "DOOD");
            json.setValue({0, "hej",2,"lasgo"}, "Bring");
            json.setValue({1, "mmongo"}, "it");
            json.move({0}, {2,2});
            QString expectedString = "[null,{\"det\":{\"virker\":\"DOOD\"},\"mmongo\":\"it\"},"
                                     "[null,null,{\"hej\":[null,null,{\"lasgo\":\"Bring\"}]}]]";
            description = "move test 1.";
            checkWax( json, expectedString, description, passCount, failCount);
        }

        {
            JsonWax json;
            json.setValue({1, "det", "virker"}, "DOOD");
            json.setValue({0, "okay"}, "Bring");
            json.setValue({1, "mmongo"}, "it");
            json.move({0}, {2});
            json.prepend({}, "g_1");
            json.prepend({1}, "g_2");
            json.prepend({4}, "g_4");
            json.prepend({4}, "g_3");
            QString expectedString = "[\"g_1\",[\"g_2\"],{\"det\":{\"virker\":\"DOOD\"},"
                                     "\"mmongo\":\"it\"},{\"okay\":\"Bring\"},[\"g_3\",\"g_4\"]]";
            description = "move and prepend things.";
            checkWax( json, expectedString, description, passCount, failCount);
        }

        {
            JsonWax json;
            description = "exists test 1: empty document root object exists.";
            checkWax( json.exists({}) == true, description, passCount, failCount);

            json.setValue({2,"a","b"}, "value");
            description = "exists test 2: existing value exists.";
            checkWax( json.exists({2,"a","b"}) == true, description, passCount, failCount);

            description = "exists test 3: non-existing value doesn't exist.";
            checkWax( json.exists({2,"a","b",0}) == false, description, passCount, failCount);

            description = "exists test 4: non-existing value doesn't exist.";
            checkWax( json.exists({2,"a","c"}) == false, description, passCount, failCount);

            description = "exists test 5: key exists.";
            checkWax( json.exists({1}) == true, description, passCount, failCount);

            description = "exists test 6: key exists.";
            checkWax( json.exists({2}) == true, description, passCount, failCount);

            description = "exists test 7: key exists.";
            checkWax( json.exists({2,"a"}) == true, description, passCount, failCount);
        }

        {
            JsonWax json;
            description = "size test 1: empty document root object has size 0.";
            checkWax( json.size({}) == 0, description, passCount, failCount);

            description = "size test 2: non-existent item has size -1.";
            checkWax( json.size({"thing"}) == -1, description, passCount, failCount);

            json.setValue({"hello"},"value1");
            description = "size test 3: one object means size of 1.";
            checkWax( json.size({}) == 1, description, passCount, failCount);

            json.setValue({"hello2"},"value2");
            description = "size test 4: two objects means size of 2.";
            checkWax( json.size({}) == 2, description, passCount, failCount);

            json.setValue({0},"value");
            description = "size test 5: one array element means size of 1.";
            checkWax( json.size({}) == 1, description, passCount, failCount);
            checkWax( json, QString("[\"value\"]"), description, passCount, failCount);

            json.setValue({5},"valued space");
            description = "size test 6: six array elements means size of 6.";
            checkWax( json.size({}) == 6, description, passCount, failCount);
            checkWax( json, QString("[\"value\",null,null,null,null,\"valued space\"]"), description, passCount, failCount);

            description = "size test 7: value has size of 1.";
            checkWax( json.size({0}) == 1, description, passCount, failCount);

            json.remove({4});
            description = "size/remove test: remove array element decreases size by 1.";
            checkWax( json.size({}) == 5, description, passCount, failCount);
            checkWax( json, QString("[\"value\",null,null,null,\"valued space\"]"), description, passCount, failCount);

            json.append({}, "salmon");
            description = "size/append test: append increases size by 1.";
            checkWax( json.size({}) == 6, description, passCount, failCount);
            checkWax( json, QString("[\"value\",null,null,null,\"valued space\",\"salmon\"]"), description, passCount, failCount);

            json.popFirst({});
            description = "size/pop_first test: pop decreases size by 1.";
            checkWax( json.size({}) == 5, description, passCount, failCount);
            checkWax( json, QString("[null,null,null,\"valued space\",\"salmon\"]"), description, passCount, failCount);

            json.popLast({});
            description = "size/pop_last test: pop decreases size by 1.";
            checkWax( json.size({}) == 4, description, passCount, failCount);
            checkWax( json, QString("[null,null,null,\"valued space\"]"), description, passCount, failCount);

            json.setValue({1},true);
            json.setValue({2},false);
            qDebug() << "expects error message:";
            json.setValue({-1},"hi");
            description = "setValue bool test.";
            checkWax( json.size({}) == 4, description, passCount, failCount);
            checkWax( json, QString("[null,true,false,\"valued space\"]"), description, passCount, failCount);

            description = "size/value consistency confirmation.";
            checkWax( json.value({json.size() - 1}) == "valued space", description, passCount, failCount);

            description = "keys test 1.";
            checkWax( json.keys({}) == QVariantList{0,1,2,3}, description, passCount, failCount);

            json.setValue({"melt","b"}, "hello1");
            json.setValue({"melt","a"}, "hello2");
            json.setValue({"melt","c"}, "hello3");
            json.setValue({"melt","d"}, "hello4");
            description = "keys test 2.";
            checkWax( json.keys({"melt"}) == QVariantList{"a","b","c","d"}, description, passCount, failCount);

            json.remove({"melt","b"});
            json.remove({"melt","k"});
            description = "keys/remove test.";
            checkWax( json.keys({"melt"}) == QVariantList{"a","c","d"}, description, passCount, failCount);
            checkWax( json, QString("{\"melt\":{\"a\":\"hello2\",\"c\":\"hello3\",\"d\":\"hello4\"}}"), description, passCount, failCount);

            json.prepend({"melt","b"}, "consistency");
            description = "prepend test 1.";
            checkWax( json.keys({"melt"}) == QVariantList{"a","b","c","d"}, description, passCount, failCount);
            checkWax( json, QString("{\"melt\":{\"a\":\"hello2\",\"b\":[\"consistency\"],\"c\":\"hello3\",\"d\":\"hello4\"}}"), description, passCount, failCount);

            json.prepend({"melt","b"}, "consistency2");
            description = "prepend test 2.";
            checkWax( json, QString("{\"melt\":{\"a\":\"hello2\",\"b\":[\"consistency2\",\"consistency\"],\"c\":\"hello3\",\"d\":\"hello4\"}}"), description, passCount, failCount);

            json.append({"melt","b"}, "consistency3");
            description = "append to array in object.";
            checkWax( json, QString("{\"melt\":{\"a\":\"hello2\",\"b\":[\"consistency2\",\"consistency\",\"consistency3\"],\"c\":\"hello3\",\"d\":\"hello4\"}}"), description, passCount, failCount);

            json.move({"melt","b"}, {"melt","c"});
            description = "move test 2.";
            checkWax( json, QString("{\"melt\":{\"a\":\"hello2\",\"c\":[\"consistency2\",\"consistency\",\"consistency3\"],\"d\":\"hello4\"}}"), description, passCount, failCount);

            json.remove({});
            description = "remove with empty keys removes everything.";
            checkWax( json, QString("{}"), description, passCount, failCount);
        }

        {
            JsonWax json;
            json.setValue({0,"mega","mark"}, "1");
            json.setValue({1,"giga","mark"}, "2");
            json.setValue({2,"ultra","mark"}, "3");

            json.popLast({});
            description = "array popping test 1.";
            checkWax( json, QString("[{\"mega\":{\"mark\":\"1\"}},{\"giga\":{\"mark\":\"2\"}}]"), description, passCount, failCount);

            json.popFirst({});
            description = "array popping test 2.";
            checkWax( json, QString("[{\"giga\":{\"mark\":\"2\"}}]"), description, passCount, failCount);

            json.popLast({});
            description = "array popping test 3.";
            checkWax( json, QString("[]"), description, passCount, failCount);

            json.popLast({});
            description = "array popping test 4: pop empty document.";
            checkWax( json, QString("[]"), description, passCount, failCount);

            json.popFirst({});
            description = "array popping test 5: pop empty document.";
            checkWax( json, QString("[]"), description, passCount, failCount);
        }

        {
            JsonWax json;
            json.setValue({"3","1","2"},"okay!");
            json.setValue({3}, "mark");
            description = "array/object overwrites properly test 1.";
            checkWax( json, QString("[null,null,null,\"mark\"]"), description, passCount, failCount);

            json.setValue({"3"},"okay!");
            description = "array/object overwrites properly test 2.";
            checkWax( json, QString("{\"3\":\"okay!\"}"), description, passCount, failCount);

            json.setValue({3,"1","2"},"salomon!");
            description = "array/object overwrites properly test 3.";
            checkWax( json, QString("[null,null,null,{\"1\":{\"2\":\"salomon!\"}}]"), description, passCount, failCount);

            json.setValue({3,1,"2"},"fracture");
            description = "array/object overwrites properly test 4.";
            checkWax( json, QString("[null,null,null,[null,{\"2\":\"fracture\"}]]"), description, passCount, failCount);
        }

        {
            JsonWax json;
            json.setValue({"A", "B"}, "this");
            json.setValue({"A", "C"}, "that");
            json.setNull({"A"});
            checkWax( json, QString("{\"A\":null}"), description, passCount, failCount);
        }

        {
            JsonWax json;
            json.fromByteArray("[{\"one\":15},{\"two\":[16,17]}]");
            QString description = "Testing type 1";
            checkWax( json.type({}) == JsonWax::Array, description, passCount, failCount);
            checkWax( json.isArray({}), description, passCount, failCount);
            checkWax( json.type({0}) == JsonWax::Object, description, passCount, failCount);
            checkWax( json.type({0,"one"}) == JsonWax::Value, description, passCount, failCount);
            checkWax( json.type({0,"not"}) == JsonWax::Null, description, passCount, failCount);
            checkWax( json.isNullValue({0,"not"}) == false, description, passCount, failCount);
            checkWax( json.type({1}) == JsonWax::Object, description, passCount, failCount);
            checkWax( json.isObject({1}), description, passCount, failCount);
            checkWax( json.type({1,"two"}) == JsonWax::Array, description, passCount, failCount);
            checkWax( json.isArray({1,"two"}), description, passCount, failCount);
            checkWax( json.type({1,"two",1}) == JsonWax::Value, description, passCount, failCount);
            checkWax( json.isValue({1,"two",1}), description, passCount, failCount);
            checkWax( json.type({1,"two",2}) == JsonWax::Null, description, passCount, failCount);
            checkWax( json.type({"ding"}) == JsonWax::Null, description, passCount, failCount);
            json.setNull({2,"three"});
            checkWax( json.isNullValue({2,"three"}), description, passCount, failCount);
        }

        {
            JsonWax json;
            json.fromByteArray("{\"one\":15,\"two\":16}");
            QString description = "Testing type 2";
            checkWax( json.type({}) == JsonWax::Object, description, passCount, failCount);
            checkWax( json.type({"one"}) == JsonWax::Value, description, passCount, failCount);
        }

        {
            JsonWax json;
            json.fromByteArray("{\"one\":15,\"two\":16}");
            QString description = "setArray and setObject";
            json.setEmptyObject({"one"});
            checkWax( json, "{\"one\":{},\"two\":16}", description, passCount, failCount);
            json.setEmptyArray({"two"});
            checkWax( json, "{\"one\":{},\"two\":[]}", description, passCount, failCount);
            json.setEmptyArray({});
            checkWax( json, "[]", description, passCount, failCount);
            json.setEmptyObject({});
            checkWax( json, "{}", description, passCount, failCount);
        }

        qDebug() << "---------------------------------------------";
        qDebug() << "=====    Editor tests PASSED: " << passCount;
        qDebug() << "=====    Editor tests FAILED: " << failCount;
    }

    static void serializerTests()
    {
        int passCount = 0;
        int failCount = 0;
        QString description;

        qDebug() << "----- Error messages from failed Serializer tests: -----";
        {
            JsonWax json;
            QColor value (155, 0, 155, 201);
            QColor preValue = value;
            json.serializeToJson<QColor>({"free"}, value);
            value = QColor();
            value = json.deserializeJson<QColor>({"free"});
            description = "Serialize and deserialize QColor as JSON.";
            checkWax( value == preValue, description, passCount, failCount );

            QDate date(1980,10,10);
            json.serializeToJson<QDate>({"a date"}, date);
            QDate preDate = date;
            date = QDate();
            date = json.deserializeJson<QDate>({"a date"});
            description = "Serialize and deserialize QDate as JSON.";
            checkWax( date == preDate, description, passCount, failCount );

            QSize size(15,160);
            json.serializeToJson<QSize>({"planetary"}, size);
            QSize preSize = size;
            size = QSize();
            size = json.deserializeJson<QSize>({"planetary"});
            description = "Serialize and deserialize QSize as JSON.";
            checkWax( size == preSize, description, passCount, failCount);

            QColor color( 15,16,0,18);
            json.serializeToBytes<QColor>({"wow color"}, color);
            QColor preColor = color;
            color = QColor();
            color = json.deserializeBytes<QColor>({"wow color"});
            description = "Serialize and deserialize QColor as base64 byte array.";
            checkWax( preColor == color, description, passCount, failCount );

            QTime time = QTime::currentTime();
            json.serializeToJson<QTime>({"the time"}, time);
            QTime preTime = time;
            time = QTime();
            time = json.deserializeJson<QTime>({"the time"});
            description = "Serialize and deserialize QTime as JSON.";
            checkWax( preTime == time, description, passCount, failCount );

            QLine line (15, 16, 200, 10);
            json.serializeToJson<QLine>({"alpha","supermega"}, line);
            QLine preline = line;
            line = QLine();
            line = json.deserializeJson<QLine>({"alpha","supermega"});
            description = "Serialize and deserialize QLine as JSON.";
            checkWax( preline == line, description, passCount, failCount );

            QLineF linef (15.5, 16.9, 200.6, 0);
            json.serializeToJson<QLineF>({"alpha","supermega float"}, linef);
            QLineF prelinef = linef;
            linef = QLineF();
            linef = json.deserializeJson<QLineF>({"alpha","supermega float"});
            description = "Serialize and deserialize QLineF as JSON.";
            checkWax( prelinef == linef, description, passCount, failCount );

            QRect rect (15,16,12,0);
            json.serializeToJson<QRect>({"alpha","beta"},rect);
            QRect preRect = rect;
            rect = QRect();
            rect = json.deserializeJson<QRect>({"alpha","beta"});
            description = "Serialize and deserialize QRect as JSON.";
            checkWax( preRect == rect, description, passCount, failCount );

            QRectF rectf (15.6,16.4,12,0);
            json.serializeToJson({"alpha","ultramon"},rectf);
            QRectF preRectf = rectf;
            rectf = QRectF();
            rectf = json.deserializeJson<QRectF>({"alpha","ultramon"});
            description = "Serialize and deserialize QRectF as JSON.";
            checkWax( preRectf == rectf, description, passCount, failCount );

            QPoint point (500,600);
            json.serializeToJson<QPoint>({"alpha","gamma"},point);
            QPoint prePoint = point;
            point = QPoint();
            point = json.deserializeJson<QPoint>({"alpha","gamma"});
            description = "Serialize and deserialize QPoint as JSON.";
            checkWax( prePoint == point, description, passCount, failCount );

            QPointF pointf (500.65,600.46);
            json.serializeToJson<QPointF>({"alpha","zebra"},pointf);
            QPointF prePointf = pointf;
            pointf = QPointF();
            pointf = json.deserializeJson<QPointF>({"alpha","zebra"});
            description = "Serialize and deserialize QPointF as JSON.";
            checkWax( prePointf == pointf, description, passCount, failCount );

            QUrl url ("www.google.com");
            json.serializeToJson<QUrl>({"alpha","omega","url"}, url);
            QUrl preUrl = url;
            url = QUrl();
            url = json.deserializeJson<QUrl>({"alpha","omega","url"});
            description = "Serialize and deserialize QUrl as JSON.";
            checkWax( preUrl == url, description, passCount, failCount );
        }

        {   // Testing datetime.
            JsonWax json;

            QDateTime dateTime1 = QDateTime::currentDateTimeUtc();                                                  // UTC
            QDateTime dateTime2 = QDateTime::currentDateTime();                                                     // LocalTime
            QDateTime dateTime3 = QDateTime(QDate(2017, 05, 19), QTime(13, 35), QTimeZone("Asia/Yekaterinburg"));   // TimeZone
            QDateTime dateTime4 = QDateTime::currentDateTime();                                                     // OffsetFromUTC
            dateTime4.setOffsetFromUtc(5800);

            json.serializeToJson<QDateTime>({"alpha","date time","test1"}, dateTime1);
            json.serializeToJson<QDateTime>({"alpha","date time","test2"}, dateTime2);
            json.serializeToJson<QDateTime>({"alpha","date time","test3"}, dateTime3);
            json.serializeToJson<QDateTime>({"alpha","date time","test4"}, dateTime4);

            QDateTime preDateTime1 = dateTime1;
            QDateTime preDateTime2 = dateTime2;
            QDateTime preDateTime3 = dateTime3;
            QDateTime preDateTime4 = dateTime4;

            dateTime1 = QDateTime();
            dateTime2 = QDateTime();
            dateTime3 = QDateTime();
            dateTime4 = QDateTime();

            dateTime1 = json.deserializeJson<QDateTime>({"alpha","date time", "test1"}, dateTime1);
            dateTime2 = json.deserializeJson<QDateTime>({"alpha","date time", "test2"}, dateTime2);
            dateTime3 = json.deserializeJson<QDateTime>({"alpha","date time", "test3"}, dateTime3);
            dateTime4 = json.deserializeJson<QDateTime>({"alpha","date time", "test4"}, dateTime4);

            description = "Serialize and deserialize QDateTime as JSON.";

            checkWax( preDateTime1 == dateTime1, description, passCount, failCount );
            checkWax( preDateTime2 == dateTime2, description, passCount, failCount );
            checkWax( preDateTime3 == dateTime3, description, passCount, failCount );
            checkWax( preDateTime4 == dateTime4, description, passCount, failCount );
        }

        {   // Static out of scope test.
            JsonWax json1;
            QColor color (15,16,17,18);
            json1.serializeToJson({"no way"}, color);
            {
                JsonWax json2;
                json2.setValue({"yes way"}, "value2");
            }
            json1.serializeToJson({"better way"}, color);
            QString expectedString = "{\"better way\":\"#120f1011\",\"no way\":\"#120f1011\"}";
            description = "Static out of scope test.";
            checkWax( json1, expectedString, description, passCount, failCount);
        }

        {   // Attempt serialize QColor (which is a string value) directly to root. Nothing should happen.
            JsonWax json;
            QColor color (15,16,17,18);
            json.serializeToJson({}, color);
            description = "Attempt to serialize QColor directly to root. Nothing should happen.";
            QString expectedString = "{}";
            checkWax( json, expectedString, description, passCount, failCount);
        }

        {   // Serialize QObjects
            JsonWax json;
            SerializerClass1 obj;
            json.serializeToJson<SerializerClass1>({"perfect"}, obj);
            obj.setProperty("superNumber", 20000);
            json.deserializeJson<SerializerClass1>( obj, {"perfect"});
            description = "Serialize and deserialize QObject as JSON (member).";
            checkWax( obj.property("superNumber").toInt() == 1900000, description, passCount, failCount );
            checkWax( obj.property("bestDate").toString() == "1950-10-10", description, passCount, failCount );
            checkWax( obj.property("name").toString() == "One\nWonderful Name", description, passCount, failCount );

            SerializerClass2 obj2;
            json.serializeToJson<SerializerClass2>({"perfect2"}, obj2);
            obj2.setProperty("superNumber", 20000);
            json.deserializeJson<SerializerClass2>( obj2, {"perfect2"});
            description = "Serialize and deserialize QObject as JSON (read write).";
            checkWax( obj2.property("superNumber").toInt() == 1900000, description, passCount, failCount );

            json.serializeToBytes<SerializerClass1>({"super neat"}, obj);
            obj.setProperty("name", "not cool");
            json.deserializeBytes<SerializerClass1>(obj, {"super neat"});
            description = "Serialize and deserialize QObject as base64 byte array.";
            checkWax( obj.property("name").toString() == "One\nWonderful Name", description, passCount, failCount );
        }

        {   // Serialize QList

            JsonWax json;
            QList<QColor> colorList;
            colorList.append( QColor(20,20,20,21));
            colorList.append( QColor(30,30,30,31));
            json.serializeToJson<QList<QColor>>({}, colorList);
            QList<QColor> preColorList = colorList;
            colorList.clear();
            colorList = json.deserializeJson<QList<QColor>>({});
            description = "Serialize and deserialize QList<QColor> as JSON.";
            checkWax( preColorList == colorList, description, passCount, failCount );
        }

        {   // Serialize QList<QList<QRect>>
            JsonWax json;
            QList<QList<QRect>> rectListMain;
            QList<QRect> rectListSub;
            rectListSub.append( QRect(20,20,20,21));
            rectListSub.append( QRect(30,30,30,31));
            rectListMain.append( rectListSub);
            QList<QRect> rectListSub2;
            rectListSub2.append( QRect(0,1,2,3));
            rectListSub2.append( QRect(4,5,9,9));
            rectListMain.append( rectListSub2);
            json.serializeToJson<QList<QList<QRect>>>({"fantastic"}, rectListMain);
            QList<QList<QRect>> preRectList = rectListMain;
            rectListMain.clear();
            rectListMain = json.deserializeJson<QList<QList<QRect>>>({"fantastic"});
            description = "Serialize and deserialize QList<QList<QRect>> as JSON.";
            checkWax( preRectList == rectListMain, description, passCount, failCount );
        }

        {   // Serialize QMap<QColor>
            JsonWax json;
            QMap<QString,QColor> map;
            map.insert("122", QColor(15,16,17,0));
            map.insert("543", QColor(1,5,0,4));
            QMap<QString,QColor> preMap = map;
            json.serializeToJson<QMap<QString,QColor>>({"generation"}, map);
            map = json.deserializeJson<QMap<QString,QColor>>({"generation"});
            description = "Serialize and deserialize QMap<QColor> as JSON.";
            checkWax( preMap == map, description, passCount, failCount );
        }

        {   // Serialize QMap<QString,QMap<QString,QSize>>
            JsonWax json;
            QMap<QString,QMap<QString,QSize>> mainMap;
            QMap<QString,QSize> submap1;
            submap1.insert("12 2", QSize(15,16));
            submap1.insert("543", QSize(1200,16));
            QMap<QString,QSize> submap2;
            submap2.insert("7", QSize(140,12));
            submap2.insert("0", QSize(200,600));
            mainMap.insert("9",submap1);
            mainMap.insert("10",submap2);
            QMap<QString,QMap<QString,QSize>> preMainMap = mainMap;
            json.serializeToJson<QMap<QString,QMap<QString,QSize>>>({"trueplay"}, mainMap);
            mainMap = json.deserializeJson<QMap<QString,QMap<QString,QSize>>>({"trueplay"});
            description = "Serialize and deserialize QMap<QString,QMap<QString,QSize>> as JSON.";
            checkWax( preMainMap == mainMap, description, passCount, failCount );
        }

        {   // Serialize QString
            JsonWax json;
            QString prestring = "20 mega tændstikker";
            json.serializeToJson({"p lay"}, prestring);
            QString poststring = json.deserializeJson<QString>({"p lay"});
            description = "Serialize and deserialize QString as JSON.";
            checkWax( prestring == poststring, description, passCount, failCount );
        }

        {   // Serialize various QString and QDate
            JsonWax json;
            QDate predate = QDate::currentDate();
            json.serializeToJson({"EventData","EventDate"}, predate);
            json.serializeToJson({"EventData","EventLocation"}, "GolfClub\nXYZ 測試 æøå");
            json.serializeToJson({"EventData","ContactPerson"}, "Fr. Curie");
            json.serializeToJson({"EventData","ContactPhone"}, "0172 123 45 67");

            QDate postdate = json.deserializeJson<QDate>({"EventData","EventDate"});
            QString out1 = json.deserializeJson<QString>({"EventData","EventLocation"});
            QString out2 = json.deserializeJson<QString>({"EventData","ContactPerson"});
            QString out3 = json.deserializeJson<QString>({"EventData","ContactPhone"});

            checkWax( predate == postdate, description, passCount, failCount );
            checkWax( "GolfClub\nXYZ 測試 æøå" == out1, description, passCount, failCount );
            checkWax( "Fr. Curie" == out2, description, passCount, failCount );
            checkWax( "0172 123 45 67" == out3, description, passCount, failCount );
        }

        {   // Serialize QStringList
            JsonWax json;
            QStringList prelist = {"20 mega tændstikker", "gold ", "pain And sacrifice"};
            json.serializeToJson({"pepole"}, prelist);
            QStringList postlist = json.deserializeJson<QStringList>({"pepole"});
            description = "Serialize and deserialize QStringList as JSON.";
            checkWax( prelist == postlist, description, passCount, failCount );
        }

        {   // Combine QMap and QList
            QMap<QString,QList<int>> data;
            QList<int> subdata;
            subdata.append(15);
            subdata.append(200);
            QList<int> subdata2;
            subdata2.append(1544554);
            subdata2.append(4545455);
            data.insert("16", subdata);
            data.insert("19", subdata2);
            JsonWax json;
            QMap<QString,QList<int>> preData = data;
            json.serializeToJson<QMap<QString,QList<int>>>({"fully","functional"}, data);
            data.clear();
            data = json.deserializeJson<QMap<QString,QList<int>>>({"fully","functional"});
            description = "Serialize and deserialize QMap combined with QList as JSON.";
            checkWax( preData == data, description, passCount, failCount );
        }

        {   // Serialize QList<QVariant>

            QList<QVariant> prelist;
            prelist << "Mowgli test" << 5 << 456.5e+5;

            JsonWax json;
            json.serializeToJson({"hi t", "here"}, prelist);
            QList<QVariant> postlist = json.deserializeJson<QList<QVariant>>({"hi t", "here"});
            description = "Serialize QList<QVariant>.";
            checkWax( prelist == postlist, description, passCount, failCount );
        }

        {   // Serialize int, double

            JsonWax json;
            json.serializeToJson({"h 1"}, 5);
            json.serializeToJson({"h 2"}, 5.67);
            json.serializeToJson({"h 3"}, 5.67e+9);

            int post1 = json.deserializeJson<int>({"h 1"});
            double post2 = json.deserializeJson<double>({"h 2"});
            double post3 = json.deserializeJson<double>({"h 3"});
            description = "Serialize int, double.";

            checkWax( 5 == post1, description, passCount, failCount );
            checkWax( 5.67 == post2, description, passCount, failCount );
            checkWax( 5.67e+9 == post3, description, passCount, failCount );
        }

        qDebug() << "---------------------------------------------";
        qDebug() << "=====    Serializer tests PASSED: " << passCount;
        qDebug() << "=====    Serializer tests FAILED: " << failCount;
    }

    static void unitTests()
    {
        parserPositiveTests();
        parserNegativeTests();
        editorTests();
        serializerTests();
    }
};    
}


#endif // JSONWAX_UNIT_TESTS_H
