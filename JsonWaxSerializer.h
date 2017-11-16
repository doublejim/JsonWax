#ifndef JSONWAX_SERIALIZER_H
#define JSONWAX_SERIALIZER_H

/* Original author: Nikolai S | https://github.com/doublejim
 *
 * You may use this file under the terms of any of these licenses:
 * GNU General Public License version 2.0       https://www.gnu.org/licenses/gpl-2.0.html
 * GNU General Public License version 3         https://www.gnu.org/licenses/gpl-3.0.html
 */

#include <QDebug>
#include <QtGlobal>
#include <QVariantList>
#include <QObject>
#include <QDataStream>
#include <QTextStream>
#include <QMetaObject>
#include <QMetaProperty>
#include <QSize>
#include <QDate>
#include <QTimeZone>
#include <QRect>
#include <QUrl>
#include <QLine>
#ifdef QT_GUI_LIB
#include <QColor>
#endif
#include "JsonWaxEditors.h"

namespace JsonWaxInternals {

class MainEditor;

// If this was just static, and not thread_local, the program would crash
// when using the Serializer-class simultaneously from separate threads.
struct SerialInOut
{
    bool SERIALIZE_TO_EDITOR = false;
    JsonWaxInternals::MainEditor* SERIALIZE_EDITOR = 0;
    JsonWaxInternals::MainEditor* DESERIALIZE_EDITOR = 0;
    QVariantList SERIALIZE_KEYS;
    QVariantList DESERIALIZE_KEYS;
    QTextStream READ_STREAM;
    QTextStream WRITE_STREAM;
};
extern thread_local SerialInOut SERIAL_IN_OUT;

class Serializer
{
private:
    void prepareEditor();

public:
    Serializer();
    ~Serializer();

    template <class T>
    QString serializeToBytes( const T& input)
    {
        QByteArray bytes;
        QDataStream stream( &bytes, QIODevice::WriteOnly);
        //stream.setVersion( QDataStream::Qt_5_7);                          // <--- Uncomment this line, if you want to lock the
        stream << input;                                                    //      serialization to a specific version of Qt.
        return QString( bytes.toBase64());
    }

    template <class T>
    JsonWaxInternals::MainEditor* serializeToJson( const T& input);

    template <class T>
    void deserializeBytes( QByteArray serializedBytes, T& outputHere)       // This function is for if you don't want to create a copy constructor
    {                                                                       // for your QObject.
        QByteArray bytes = QByteArray::fromBase64( serializedBytes);
        QDataStream stream( &bytes, QIODevice::ReadOnly);
        //stream.setVersion( QDataStream::Qt_5_7);                          // <--- Uncomment this line, if you want to lock the
        stream >> outputHere;                                               //      deserialization to a specific version of Qt.
    }

    template <class T>
    T deserializeBytes( QByteArray serializedBytes)
    {
        QByteArray bytes = QByteArray::fromBase64( serializedBytes);
        T result;
        QDataStream stream( &bytes, QIODevice::ReadOnly);
        //stream.setVersion( QDataStream::Qt_5_7);                          // <--- Uncomment this line, if you want to lock the
        stream >> result;                                                   //      deserialization to a specific version of Qt.
        return result;
    }

    template <class T>
    void deserializeJson( MainEditor* editor, const QVariantList& keys, T& output);
};

// =============================== READ/WRITE JSON ENTRIES ===============================

template <class T>
static T readFromDeEditor( QString entryName);

template <class T>
static void writeToSeEditor( QString entryName, T value);

// =============================== TEXTSTREAM OVERLOAD ===============================

// QColor (only if project has QT += gui)
#ifdef QT_GUI_LIB
QTextStream& operator << (QTextStream &stream, const QColor &color);
QTextStream& operator >> (QTextStream &stream, QColor &color);
#endif
// QDate

QTextStream& operator << (QTextStream &stream, const QDate &date);
QTextStream& operator >> (QTextStream &stream, QDate &date);

// QTime
QTextStream& operator << (QTextStream &stream, const QTime &time);
QTextStream& operator >> (QTextStream &stream, QTime &time);

// QDateTime
QTextStream& operator << (QTextStream &stream, const QDateTime &dateTime);
QTextStream& operator >> (QTextStream &stream, QDateTime &dateTime);

// QLine
QTextStream& operator << (QTextStream &stream, const QLine &line);
QTextStream& operator >> (QTextStream &stream, QLine &line);

// QLineF
QTextStream& operator << (QTextStream &stream, const QLineF &line);
QTextStream& operator >> (QTextStream &stream, QLineF &line);

// QObject
QTextStream& operator << (QTextStream &stream, const QObject &obj);
QTextStream& operator >> (QTextStream &stream, QObject &obj);

// QPoint
QTextStream& operator << (QTextStream &stream, const QPoint &point);
QTextStream& operator >> (QTextStream &stream, QPoint &point);

// QPointF
QTextStream& operator << (QTextStream &stream, const QPointF &point);
QTextStream& operator >> (QTextStream &stream, QPointF &point);

// QRect
QTextStream& operator << (QTextStream &stream, const QRect &rect);
QTextStream& operator >> (QTextStream &stream, QRect &rect);

// QRectF
QTextStream& operator << (QTextStream &stream, const QRectF &rect);
QTextStream& operator >> (QTextStream &stream, QRectF &rect);

// QSize
QTextStream& operator << (QTextStream &stream, const QSize &size);
QTextStream& operator >> (QTextStream &stream, QSize &size);

// QUrl
QTextStream& operator << (QTextStream &stream, const QUrl &url);
QTextStream& operator >> (QTextStream &stream, QUrl &url);

// QVariant
QTextStream& operator << (QTextStream &stream, const QVariant &variant);
QTextStream& operator >> (QTextStream &stream, QVariant &variant);

// QList
template <class T>
inline QTextStream& operator << (QTextStream &stream, const QList<T>& list)
{
    SERIAL_IN_OUT.SERIALIZE_TO_EDITOR = true;

    for (int i = 0; i < list.count(); ++i)
    {
        SERIAL_IN_OUT.SERIALIZE_KEYS.append(i);

        QString value;
        QTextStream stream2( &value, QIODevice::WriteOnly);
        stream2 << list.at(i);                                              // This can be self-referential.

        if (!SERIAL_IN_OUT.SERIALIZE_EDITOR->exists( SERIAL_IN_OUT.SERIALIZE_KEYS))                     // This check is so that it doesn't overwrite with an empty value.
            SERIAL_IN_OUT.SERIALIZE_EDITOR->setValue( SERIAL_IN_OUT.SERIALIZE_KEYS, value);

        SERIAL_IN_OUT.SERIALIZE_KEYS.removeLast();
    }

    return stream;
}

template <class T>
inline QTextStream& operator >> (QTextStream &stream, QList<T>& list)
{
    ArrayEditor arrayEditor = SERIAL_IN_OUT.DESERIALIZE_EDITOR->toArrayEditor( SERIAL_IN_OUT.DESERIALIZE_KEYS );

    for (int key = 0; key < arrayEditor.size(); ++key)
    {
        SERIAL_IN_OUT.DESERIALIZE_KEYS.append( key);

        QString strValue = arrayEditor.value( key).toString();
        QTextStream stream2( &strValue, QIODevice::ReadOnly);
        T value;
        stream2 >> value;                                                   // This can be self-referential.
        list.append( value);

        SERIAL_IN_OUT.DESERIALIZE_KEYS.removeLast();
    }
    return stream;
}

// QMap
template <class T>
inline QTextStream& operator << (QTextStream &stream, const QMap<QString, T>& map)
{
    SERIAL_IN_OUT.SERIALIZE_TO_EDITOR = true;

    for (const QString& key : map.keys())
    {
        SERIAL_IN_OUT.SERIALIZE_KEYS.append( key);

        QString value;
        QTextStream stream2( &value, QIODevice::WriteOnly);
        stream2 << map.value( key);                                         // This can be self-referential.

        if (!SERIAL_IN_OUT.SERIALIZE_EDITOR->exists( SERIAL_IN_OUT.SERIALIZE_KEYS))                     // This check is so that it doesn't overwrite with an empty value.
            SERIAL_IN_OUT.SERIALIZE_EDITOR->setValue( SERIAL_IN_OUT.SERIALIZE_KEYS, value);

        SERIAL_IN_OUT.SERIALIZE_KEYS.removeLast();
    }

    return stream;
}

template <class T>
inline QTextStream& operator >> (QTextStream &stream, QMap<QString, T>& map)
{
    ObjectEditor objectEditor = SERIAL_IN_OUT.DESERIALIZE_EDITOR->toObjectEditor( SERIAL_IN_OUT.DESERIALIZE_KEYS);

    for (const QString& key : objectEditor.keys())
    {
        SERIAL_IN_OUT.DESERIALIZE_KEYS.append( key);

        QString strValue = objectEditor.value( key).toString();
        QTextStream stream2( &strValue, QIODevice::ReadOnly);
        T value;
        stream2 >> value;                                                   // This can be self-referential.
        map.insert( key, value);

        SERIAL_IN_OUT.DESERIALIZE_KEYS.removeLast();
    }
    return stream;
}

// =============================== DATASTREAM OVERLOAD ===============================

// QObject
QDataStream& operator << (QDataStream &stream, const QObject &obj);
QDataStream& operator >> (QDataStream &stream, QObject &obj);

// =========================================== DEFINED LATE ===========================================

template <class T>
static T readFromDeEditor( QString entryName)
{
    ObjectEditor waxobject = SERIAL_IN_OUT.DESERIALIZE_EDITOR->toObjectEditor( SERIAL_IN_OUT.DESERIALIZE_KEYS);
    QString strValue = waxobject.value( entryName).toString();
    SERIAL_IN_OUT.READ_STREAM.setString( &strValue, QIODevice::ReadOnly);
    T value;
    SERIAL_IN_OUT.READ_STREAM >> value;
    return value;
}

template <class T>
static void writeToSeEditor( QString entryName, T value)
{
    QString strValue;
    SERIAL_IN_OUT.WRITE_STREAM.setString( &strValue, QIODevice::WriteOnly);
    SERIAL_IN_OUT.WRITE_STREAM << value;

    ObjectEditor waxobject = SERIAL_IN_OUT.SERIALIZE_EDITOR->toObjectEditor( SERIAL_IN_OUT.SERIALIZE_KEYS);
    waxobject.setValue( entryName, strValue);
}

template <class T>
inline JsonWaxInternals::MainEditor* Serializer::serializeToJson( const T& input)
{
    prepareEditor();

    QString serialized;
    QTextStream stream( &serialized, QIODevice::WriteOnly);
    stream << input;

    // If the input was serialized to editor (fx. a QObject), "serialized" is still empty.
    // All the JSON data has been stored in the SERIALIZE_EDITOR.

    if (!SERIAL_IN_OUT.SERIALIZE_TO_EDITOR)
        SERIAL_IN_OUT.SERIALIZE_EDITOR->setValue({0}, serialized);

    return SERIAL_IN_OUT.SERIALIZE_EDITOR;
}

template <class T>
inline void Serializer::deserializeJson( MainEditor* editor, const QVariantList& keys, T& output)
{
    SERIAL_IN_OUT.DESERIALIZE_EDITOR = editor;                                                    // QObject uses the data in the DESERIALIZE_EDITOR to set its properties.
    SERIAL_IN_OUT.DESERIALIZE_KEYS = keys;                                                        // The keys are used to locate the properties, if it's a QObject,
                                                                                    // or the location, if it's a QMap or a QList.
    QString serializedValue;                                                        // serializedValue is used if the serialized data is just a string.

    if (SERIAL_IN_OUT.DESERIALIZE_EDITOR->isValue( keys))                                         // In case it's a one-line string value.
        serializedValue = SERIAL_IN_OUT.DESERIALIZE_EDITOR->value( keys, QString()).toString();

    QTextStream stream( &serializedValue, QIODevice::ReadOnly);
    stream >> output;
}

}

#endif // JSONWAX_SERIALIZER_H
