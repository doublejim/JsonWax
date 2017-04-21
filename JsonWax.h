#ifndef JSONWAX_H
#define JSONWAX_H

#include <QFile>
#include <QTextStream>
#include <QCoreApplication>
#include <QDir>
#include "JsonWaxParser.h"
#include "JsonWaxEditor.h"

class JsonWax
{
private:
    JsonWaxInternals::Parser PARSER;
    JsonWaxInternals::Editor* EDITOR = 0;
    QString PROGRAM_PATH;
    QString FILENAME;

public:
    typedef JsonWaxInternals::StringStyle StringStyle;
    static const StringStyle Compact = JsonWaxInternals::StringStyle::Compact;
    static const StringStyle Readable = JsonWaxInternals::StringStyle::Readable;

    JsonWax()
    {
        EDITOR = new JsonWaxInternals::Editor();
        PROGRAM_PATH = qApp->applicationDirPath();
    }

    JsonWax( const QString& fileName) : JsonWax()
    {
        loadFile( fileName);
    }

    ~JsonWax()
    {
        delete EDITOR;
    }

    QString errorMsg()
    {
        return PARSER.errorToString();
    }

    int errorCode()
    {
        return PARSER.LAST_ERROR;
    }

    int errorPos()  // May not be 100% accurate.
    {
        return PARSER.LAST_ERROR_POS;
    }

    bool fromByteArray( const QByteArray& bytes)
    {
        bool isWellFormed = PARSER.isWellformed( bytes);
        delete EDITOR;
        EDITOR = PARSER.getEditorObject();
        return isWellFormed;
    }

    bool loadFile( const QString& fileName)
    {
        FILENAME = fileName;
        QDir dir (fileName);
        QFile qfile;
        if (dir.isRelative())
            qfile.setFileName( PROGRAM_PATH + '/' + fileName);
        else qfile.setFileName( fileName);

        if (!qfile.exists())
        {
            qWarning("JsonWax-loadFile warning: the file doesn't exist: \"" + fileName.toUtf8() + "\"");
            return false;
        }

        QTextStream in (&qfile);
        in.setCodec("UTF-8");
        /*
            TODO: Determine correct codec. UTF-8 setting invalidates some ansi characters like "æ,ø,å".
            So make sure the read file is UTF-8 encoded, then everything will work perfectly.
        */
        qfile.open(QIODevice::ReadOnly);
        return fromByteArray( in.readAll().toUtf8());
    }

    bool saveAs( const QString& fileName, StringStyle style = Readable, bool convertToCodePoints = false, bool overwriteAllowed = true)
    {
        QDir dir (fileName);
        QFile qfile;
        if (dir.isRelative())
            qfile.setFileName( PROGRAM_PATH + '/' + fileName);
        else qfile.setFileName( fileName);

        if (qfile.exists() && !overwriteAllowed)
            return false;

        qfile.open( QIODevice::WriteOnly | QIODevice::Text);
        QByteArray bytes = EDITOR->toByteArray( style, convertToCodePoints);
        qint64 bytesWritten = qfile.write( bytes);
        qfile.close();
        return (bytesWritten == bytes.size());
    }

    bool save( StringStyle style = Readable, bool convertToCodePoints = false)
    {
        if (FILENAME.isEmpty())
        {
            qWarning("JsonWax-save warning: use saveAs() if you haven't loaded a .json file. This document wasn't saved.");
            return false;
        } else {
            return saveAs( FILENAME, style, convertToCodePoints, true);
        }
    }

    void remove( const QVariantList& keys)
    {
        EDITOR->remove( keys);
    }

    void setValue( const QVariantList& keys, const QVariant& value)
    {
        EDITOR->setValue( keys, value);
    }

    bool exists( const QVariantList& keys)
    {
        return EDITOR->exists( keys);
    }

    void setNull( const QVariantList& keys)
    {
        EDITOR->setValue( keys, QVariant());
    }

    int size( const QVariantList& keys)
    {
        return EDITOR->size( keys);
    }

    int append( const QVariantList& keys, const QVariant& value)
    {
        return EDITOR->append( keys, value);
    }

    void prepend( const QVariantList& keys, const QVariant& value)
    {
        EDITOR->prepend( keys, value);
    }

    void popFirst( const QVariantList& keys, int removeTimes = 1)
    {
        EDITOR->popFirst( keys, removeTimes);
    }

    void popLast( const QVariantList& keys, int removeTimes = 1)
    {
        EDITOR->popLast( keys, removeTimes);
    }

    void copy( const QVariantList& keysFrom, QVariantList keysTo)
    {
        EDITOR->copy( keysFrom, EDITOR, keysTo);
    }

    void copy( const QVariantList& keysFrom, JsonWax& jsonTo, const QVariantList& keysTo)
    {
        EDITOR->copy( keysFrom, jsonTo.EDITOR, keysTo);
    }

    void move( const QVariantList& keysFrom, const QVariantList& keysTo)
    {
        EDITOR->move( keysFrom, EDITOR, keysTo);
    }

    void move( const QVariantList& keysFrom, JsonWax& jsonTo, const QVariantList& keysTo)
    {
        EDITOR->move( keysFrom, jsonTo.EDITOR, keysTo);
    }

    QVariant value( const QVariantList& keys)
    {
        return EDITOR->value( keys);
    }

    QVariantList keys( const QVariantList& keys)
    {
        return EDITOR->keys( keys);
    }

    QString toString( StringStyle style = Readable, bool convertToCodePoints = false)
    {
        return EDITOR->toString( style, convertToCodePoints);
    }
};

#endif // JSONWAX_H
