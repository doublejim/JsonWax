#ifndef JSONWAX_UNIT_TESTS_H
#define JSONWAX_UNIT_TESTS_H

#include <QElapsedTimer>
#include <QDirIterator>
#include <QJsonDocument>
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

// =========================================================================================================

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

public:
    static void parserPositiveTests() // ============================================================
    {
        int passCount = 0;
        int failCount = 0;
        qDebug() << "----- Error messages from positive tests -----";

        {
            QString input = "{\n\n"
                            "   \"hello\"  \n :\n   \"omega\" \n"
                            "}";
            QString expectedString = "{\"hello\":\"omega\"}";
            QString description = "String key and string value in an object, with newlines and spaces.";
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
            QString input = "[[[[[[[[[[[[[[[[[[[[[[[[[[[[[true]]]]]]]]]]]]]]]]]]]]]]]]]]]]]";
            QString expectedString = "[[[[[[[[[[[[[[[[[[[[[[[[[[[[[true]]]]]]]]]]]]]]]]]]]]]]]]]]]]]";
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
            QString input = "[ 5e15, -554e1, -0.4E-67, 0.94, 7565E+56, 1E+0 ]";
            QString expectedString = "[5e+15,-5540,-4e-68,0.94,7.565e+59,1]";
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
            QString description = "Invalid number 0.";
            run( input, expectedString, INVALID, passCount, failCount, description);
        }

        {
            QString input = "{\n\"john\"  : .15}";
            QString expectedString = "{}"; // Can't read any values.
            QString description = "Invalid number 1.";
            run( input, expectedString, INVALID, passCount, failCount, description);
        }

        {
            QString input = "{\n\"john\"  : .789}";
            QString expectedString = "{}"; // Can't read any values.
            QString description = "Invalid number 2.";
            run( input, expectedString, INVALID, passCount, failCount, description);
        }

        {
            QString input = "{\n\"john\"  : 78.9.2}";
            QString expectedString = "{\"john\":78.9}";
            QString description = "Invalid number 3.";
            run( input, expectedString, INVALID, passCount, failCount, description);
        }

        {
            QString input = "{\n\"john\"  : ..123}";
            QString expectedString = "{}"; // Can't read any values.
            QString description = "Invalid number 4.";
            run( input, expectedString, INVALID, passCount, failCount, description);
        }

        {
            QString input = "{\n\"john\"  : e12}";
            QString expectedString = "{}"; // Can't read any values.
            QString description = "Invalid number 5.";
            run( input, expectedString, INVALID, passCount, failCount, description);
        }

        {
            QString input = "{\n\"john\"  : 5e1e5}";
            QString expectedString = "{\"john\":50}";
            QString description = "Invalid number 6.";
            run( input, expectedString, INVALID, passCount, failCount, description);
            // Note: it reads 5e, which is 50
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
            QString expectedString = "{}";
            QString description = "Illegal array after array.";
            run( input, expectedString, INVALID, passCount, failCount, description);
            // Note: an empty array always becomes: {}
        }

        {
            QString input = "[]{}";
            QString expectedString = "{}";
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

        qDebug() << "----- Error messages from failed Editor tests: -----";

        {
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

            json.remove({4});
            description = "size/remove test: remove array element decreases size by 1.";
            checkWax( json.size({}) == 5, description, passCount, failCount);
            checkWax( json, QString("[\"value\",null,null,null,\"valued space\"]"), description, passCount, failCount);

            json.append({}, "salmon");
            description = "size/append test: append increases size by 1.";
            checkWax( json.size({}) == 6, description, passCount, failCount);
            checkWax( json, QString("[\"value\",null,null,null,\"valued space\",\"salmon\"]"), description, passCount, failCount);

            json.pop_first({});
            description = "size/pop_first test: pop decreases size by 1.";
            checkWax( json.size({}) == 5, description, passCount, failCount);
            checkWax( json, QString("[null,null,null,\"valued space\",\"salmon\"]"), description, passCount, failCount);

            json.pop_last({});
            description = "size/pop_last test: pop decreases size by 1.";
            checkWax( json.size({}) == 4, description, passCount, failCount);
            checkWax( json, QString("[null,null,null,\"valued space\"]"), description, passCount, failCount);

            json.setValue({1},true);
            json.setValue({2},false);
            json.setValue({-1},"hi");
            description = "setValue bool test.";
            checkWax( json.size({}) == 4, description, passCount, failCount);
            checkWax( json, QString("[null,true,false,\"valued space\"]"), description, passCount, failCount);

            description = "size/value consistency confirmation.";
            checkWax( json.value({json.size({}) - 1}) == "valued space", description, passCount, failCount);

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

        qDebug() << "---------------------------------------------";
        qDebug() << "=====    Editor tests PASSED: " << passCount;
        qDebug() << "=====    Editor tests FAILED: " << failCount;
    }

    static void runTest()
    {
        parserPositiveTests();
        parserNegativeTests();
        editorTests();
    }

    /*static void run_test2()
    {
        JsonWax json;
        json.loadFile("before.json");

        QString value = json.value({"cheeks"}).toString();
        if (!value.isEmpty())
        {
            json.saveAs("after.json", JsonWax::Compact);
            json.loadFile("after.json");
            QString value2 = json.value({"cheeks"}).toString();
            qDebug() << "Do chinese characters work now? : " << (value == value2);

            QStringList stringsBef;
            for (QChar ch : value)
                stringsBef.append( QString::number(ch.unicode()));

            QStringList stringsAft;
            for (QChar ch : value2)
                stringsAft.append( QString::number(ch.unicode()));

            qDebug() << "Unicode code points before: " << stringsBef;
            qDebug() << "Unicode code points after: " << stringsAft;
        }
    }*/
};
}

#endif // JSONWAX_UNIT_TESTS_H
