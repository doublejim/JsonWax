#ifndef JSONWAX_SERIALIZER_H
#define JSONWAX_SERIALIZER_H

/* Original author: Nikolai S | https://github.com/doublejim
 *
 * You may use this file under the terms of any of these licenses:
 * GNU General Public License version 2.0       https://www.gnu.org/licenses/gpl-2.0.html
 * GNU General Public License version 3         https://www.gnu.org/licenses/gpl-3.0.html
 */

#include <QVariantList>
#include <QObject>
#include <QDataStream>
#include <QTextStream>
#include <QMetaObject>
#include <QMetaProperty>
#include <QColor>
#include <QSize>
#include <QDate>
#include <QTimeZone>
#include <QRect>
#include <QUrl>
#include <QLine>
#include "JsonWaxEditor.h"

namespace JsonWaxInternals {

static bool SERIALIZE_TO_EDITOR = false;
static JsonWaxInternals::Editor* SERIALIZE_EDITOR = 0;
static JsonWaxInternals::Editor* DESERIALIZE_EDITOR = 0;
static QVariantList SERIALIZE_KEYS;
static QVariantList DESERIALIZE_KEYS;

class Serializer
{
public:
    Serializer(){
        SERIALIZE_EDITOR = new JsonWaxInternals::Editor;
    }

    ~Serializer(){
        delete SERIALIZE_EDITOR;
    }

    template <class T>
    QString serializeToBytes( const T& input)
    {
        QByteArray bytes;
        QDataStream stream( &bytes, QIODevice::WriteOnly);
        stream.setVersion( QDataStream::Qt_5_8);
        stream << input;
        return QString( bytes.toBase64());
    }

    template <class T>
    JsonWaxInternals::Editor* serializeToJson( const T& input)
    {
        SERIALIZE_TO_EDITOR = false;
        SERIALIZE_EDITOR->clear();
        SERIALIZE_KEYS = {0};

        QString serialized;
        QTextStream stream( &serialized, QIODevice::WriteOnly);
        stream << input;

        // If the input was serialized to editor (fx. a QObject), "serialized" is still empty.
        // All the JSON data has been stored in the SERIALIZE_EDITOR.

        if (!SERIALIZE_TO_EDITOR)
            SERIALIZE_EDITOR->setValue({0}, serialized);

        return SERIALIZE_EDITOR;
    }

    template <class T>
    void deserializeBytes( QByteArray& serializedBytes, T& outputHere)      // If you don't want to create a copy constructor
    {                                                                       // for your QObject.
        QByteArray bytes = QByteArray::fromBase64( serializedBytes);
        QDataStream stream( &bytes, QIODevice::ReadOnly);
        stream >> outputHere;
    }

    template <class T>
    T deserializeBytes( QByteArray& serializedBytes)
    {
        QByteArray bytes = QByteArray::fromBase64( serializedBytes);
        T result;
        QDataStream stream( &bytes, QIODevice::ReadOnly);
        stream >> result;
        return result;
    }

    template <class T>
    void deserializeJson( JsonWaxInternals::Editor* editor, const QVariantList& keys, T& output)
    {
        DESERIALIZE_EDITOR = editor;                                        // QObject uses the data in the DESERIALIZE_EDITOR to set its properties.
        DESERIALIZE_KEYS = keys;                                            // The keys are used to locate the properties.

        QString serializedValue;                                            // serializedValue is used if the serialized data is not a QObject.

        if (editor->isValue( keys))                                         // In case it's a one-line string value.
            serializedValue = editor->value(keys, QVariant()).toString();

        QTextStream stream( &serializedValue, QIODevice::ReadOnly);
        stream >> output;
    }
};

// =============================== READ/WRITE JSON ENTRIES ===============================

template <class T>
static T readFromDeEditor( QString entryName)
{
    QString strValue = DESERIALIZE_EDITOR->value( DESERIALIZE_KEYS + QVariantList{entryName}, "").toString();
    QTextStream stream( &strValue, QIODevice::ReadOnly);
    T value;
    stream >> value;
    return value;
}

template <class T>
static void writeToSeEditor( QString entryName, T value)
{
    QString strValue;
    QTextStream stream( &strValue, QIODevice::WriteOnly);
    stream << value;
    SERIALIZE_EDITOR->setValue( SERIALIZE_KEYS + QVariantList{ entryName}, strValue);
}

// =============================== TEXTSTREAM OVERLOAD ===============================

// QColor

inline QTextStream& operator << (QTextStream &stream, const QColor &color)
{
    stream << "#";
    stream << QString::number( color.alpha(), 16).rightJustified(2, '0');
    stream << QString::number( color.red(), 16).rightJustified(2, '0');
    stream << QString::number( color.green(), 16).rightJustified(2, '0');
    stream << QString::number( color.blue(), 16).rightJustified(2, '0');
    return stream;
}

inline QTextStream& operator >> (QTextStream &stream, QColor &color)
{
    QString sColor;
    stream >> sColor;
    color.setNamedColor( sColor);
    return stream;
}

// QDate

inline QTextStream& operator << (QTextStream &stream, const QDate &date)
{
    stream << date.toString(Qt::ISODate);
    return stream;
}

inline QTextStream& operator >> (QTextStream &stream, QDate &date)
{
    QString sDate;
    stream >> sDate;
    date = QDate::fromString( sDate, Qt::ISODate);
    return stream;
}

// QTime

inline QTextStream& operator << (QTextStream &stream, const QTime &time)
{
    stream << time.toString(Qt::ISODateWithMs);
    return stream;
}

inline QTextStream& operator >> (QTextStream &stream, QTime &time)
{
    QString value;
    stream >> value;
    time = QTime::fromString( value, Qt::ISODateWithMs);
    return stream;
}

// QDateTime

inline QTextStream& operator << (QTextStream &stream, const QDateTime &dateTime)
{
    SERIALIZE_TO_EDITOR = true;

    writeToSeEditor("date", dateTime.date());
    writeToSeEditor("time", dateTime.time());
    writeToSeEditor("timeSpec", dateTime.timeSpec());

    switch(dateTime.timeSpec())
    {
    case Qt::LocalTime: case Qt::UTC:
        // Do nothing more.
        break;
    case Qt::OffsetFromUTC:
        writeToSeEditor("utcOffset", dateTime.offsetFromUtc());
        break;
    case Qt::TimeZone:
        writeToSeEditor("timeZoneId", dateTime.timeZone().id());
        break;
    }
    return stream;
}

inline QTextStream& operator >> (QTextStream &stream, QDateTime &dateTime)
{
    QDate date = readFromDeEditor<QDate>("date");
    QTime time = readFromDeEditor<QTime>("time");
    int timeSpec = readFromDeEditor<int>("timeSpec");

    dateTime.setDate(date);
    dateTime.setTime(time);

    switch(Qt::TimeSpec(timeSpec))
    {
    case Qt::LocalTime:
        // default time spec.
        break;
    case Qt::UTC:
        dateTime.setTimeSpec( Qt::UTC);
        break;
    case Qt::OffsetFromUTC:
    {
        int offset = readFromDeEditor<int>("utcOffset");
        dateTime.setOffsetFromUtc( offset);
        break;
    }
    case Qt::TimeZone:
    {
        QByteArray bytesTimeZone = readFromDeEditor<QByteArray>("timeZoneId");
        dateTime.setTimeZone( QTimeZone( bytesTimeZone));
    }
    }

    return stream;
}

// QLine

inline QTextStream& operator << (QTextStream &stream, const QLine &line)
{
    SERIALIZE_TO_EDITOR = true;
    writeToSeEditor("x1", line.x1());
    writeToSeEditor("y1", line.y1());
    writeToSeEditor("x2", line.x2());
    writeToSeEditor("y2", line.y2());
    return stream;
}

inline QTextStream& operator >> (QTextStream &stream, QLine &line)
{
    int x1 = readFromDeEditor<int>("x1");
    int y1 = readFromDeEditor<int>("y1");
    int x2 = readFromDeEditor<int>("x2");
    int y2 = readFromDeEditor<int>("y2");
    line.setLine( x1, y1, x2, y2);
    return stream;
}

// QLineF

inline QTextStream& operator << (QTextStream &stream, const QLineF &line)
{
    SERIALIZE_TO_EDITOR = true;
    writeToSeEditor("x1", line.x1());
    writeToSeEditor("y1", line.y1());
    writeToSeEditor("x2", line.x2());
    writeToSeEditor("y2", line.y2());
    return stream;
}

inline QTextStream& operator >> (QTextStream &stream, QLineF &line)
{
    qreal x1 = readFromDeEditor<qreal>("x1");
    qreal y1 = readFromDeEditor<qreal>("y1");
    qreal x2 = readFromDeEditor<qreal>("x2");
    qreal y2 = readFromDeEditor<qreal>("y2");
    line.setLine( x1, y1, x2, y2);
    return stream;
}

// QObject

inline QTextStream& operator << (QTextStream &stream, const QObject &obj)
{
    // This stores all the QObject properties in the SERIALIZE_EDITOR.

    SERIALIZE_TO_EDITOR = true;

    for (int i = 0; i < obj.metaObject()->propertyCount(); ++i)
        if (obj.metaObject()->property(i).isStored( &obj))
        {
            QString name = QString( obj.metaObject()->property(i).name());
            QString value = obj.metaObject()->property(i).read( &obj).toString();
            SERIALIZE_EDITOR->setValue({0, name}, value);
        }
    return stream;
}

inline QTextStream& operator >> (QTextStream &stream, QObject &obj)
{
    // This inserts property values from a JSON-document into the QObject.
    // The DESERIALIZE_EDITOR is a pointer to the main editor in "JsonWax.h".
    // We find all the subkeys in that location, see if they are stored
    // in the object, and insert them if they are.

    for (QVariant& key : DESERIALIZE_EDITOR->keys({ DESERIALIZE_KEYS}))
    {
        int index = obj.metaObject()->indexOfProperty( key.toString().toUtf8());

        if (index == -1)
            return stream;

        if (obj.metaObject()->property( index).isStored( &obj))
        {
            QVariant value = DESERIALIZE_EDITOR->value( DESERIALIZE_KEYS + QVariantList{key}, QVariant());
            obj.metaObject()->property( index).write( &obj, value);
        }
    }
    return stream;
}

// QPoint

inline QTextStream& operator << (QTextStream &stream, const QPoint &point)
{
    SERIALIZE_TO_EDITOR = true;
    writeToSeEditor("x", point.x());
    writeToSeEditor("y", point.y());
    return stream;
}

inline QTextStream& operator >> (QTextStream &stream, QPoint &point)
{
    int x = readFromDeEditor<int>("x");
    int y = readFromDeEditor<int>("y");
    point.setX( x);
    point.setY( y);
    return stream;
}

// QPointF

inline QTextStream& operator << (QTextStream &stream, const QPointF &point)
{
    SERIALIZE_TO_EDITOR = true;
    writeToSeEditor("x", point.x());
    writeToSeEditor("y", point.y());
    return stream;
}

inline QTextStream& operator >> (QTextStream &stream, QPointF &point)
{
    qreal x = readFromDeEditor<qreal>("x");
    qreal y = readFromDeEditor<qreal>("y");
    point.setX( x);
    point.setY( y);
    return stream;
}

// QRect

inline QTextStream& operator << (QTextStream &stream, const QRect &rect)
{
    SERIALIZE_TO_EDITOR = true;
    writeToSeEditor("left", rect.left());
    writeToSeEditor("top", rect.top());
    writeToSeEditor("right", rect.right());
    writeToSeEditor("bottom", rect.bottom());
    return stream;
}

inline QTextStream& operator >> (QTextStream &stream, QRect &rect)
{
    int left = readFromDeEditor<int>("left");
    int top = readFromDeEditor<int>("top");
    int right = readFromDeEditor<int>("right");
    int bottom = readFromDeEditor<int>("bottom");
    rect.setLeft( left);
    rect.setTop( top);
    rect.setRight( right);
    rect.setBottom( bottom);
    return stream;
}

// QRectF

inline QTextStream& operator << (QTextStream &stream, const QRectF &rect)
{
    SERIALIZE_TO_EDITOR = true;
    writeToSeEditor("left", rect.left());
    writeToSeEditor("top", rect.top());
    writeToSeEditor("right", rect.right());
    writeToSeEditor("bottom", rect.bottom());
    return stream;
}

inline QTextStream& operator >> (QTextStream &stream, QRectF &rect)
{
    qreal left = readFromDeEditor<qreal>("left");
    qreal top = readFromDeEditor<qreal>("top");
    qreal right = readFromDeEditor<qreal>("right");
    qreal bottom = readFromDeEditor<qreal>("bottom");
    rect.setLeft( left);
    rect.setTop( top);
    rect.setRight( right);
    rect.setBottom( bottom);
    return stream;
}

// QSize

inline QTextStream& operator << (QTextStream &stream, const QSize &size)
{
    SERIALIZE_TO_EDITOR = true;
    writeToSeEditor("width", size.width());
    writeToSeEditor("height", size.height());
    return stream;
}

inline QTextStream& operator >> (QTextStream &stream, QSize &size)
{
    int width = readFromDeEditor<int>("width");
    int height = readFromDeEditor<int>("height");
    size.setWidth( width);
    size.setHeight( height);
    return stream;
}

// QUrl

inline QTextStream& operator << (QTextStream &stream, const QUrl &url)
{
    stream << url.toString();
    return stream;
}

inline QTextStream& operator >> (QTextStream &stream, QUrl &url)
{
    QString strUrl;
    stream >> strUrl;
    url = QUrl(strUrl);
    return stream;
}

// QVariant

inline QTextStream& operator << (QTextStream &stream, const QVariant &variant)
{
    SERIALIZE_TO_EDITOR = true;
    writeToSeEditor("type", variant.type());
    if (!variant.toString().isEmpty())
         writeToSeEditor("value", variant.toString());
    return stream;
}

inline QTextStream& operator >> (QTextStream &stream, QVariant &variant)
{
    int type = readFromDeEditor<int>({"type"});
    QString value = readFromDeEditor<QString>({"value"});;
    variant.setValue( value);
    variant.convert( QVariant::Type( type));
    return stream;
}

// QList

template <class T>
inline QTextStream& operator << (QTextStream &stream, const QList<T>& list)
{
    SERIALIZE_TO_EDITOR = true;

    for (int i = 0; i < list.count(); ++i)
    {
        SERIALIZE_KEYS.append(i);

        QString value;
        QTextStream stream2( &value, QIODevice::WriteOnly);
        stream2 << list.at(i);                                              // This can be self-referential.

        if (!SERIALIZE_EDITOR->exists( SERIALIZE_KEYS))                     // Check here, so that it doesn't overwrite (with an empty value).
            SERIALIZE_EDITOR->setValue( SERIALIZE_KEYS, value);

        SERIALIZE_KEYS.removeLast();
    }

    return stream;
}

template <class T>
inline QTextStream& operator >> (QTextStream &stream, QList<T>& list)
{
    for (QVariant& key : DESERIALIZE_EDITOR->keys({ DESERIALIZE_KEYS}))
    {
        DESERIALIZE_KEYS.append( key);

        QString strValue = DESERIALIZE_EDITOR->value( DESERIALIZE_KEYS, QVariant()).toString();
        QTextStream stream2( &strValue, QIODevice::ReadOnly);
        T value;
        stream2 >> value;                                                   // This can be self-referential.
        list.append( value);

        DESERIALIZE_KEYS.removeLast();
    }
    return stream;
}

// QMap

template <class T>
inline QTextStream& operator << (QTextStream &stream, const QMap<QString, T>& map)
{
    SERIALIZE_TO_EDITOR = true;

    for (const QString& key : map.keys())
    {
        SERIALIZE_KEYS.append( key);

        QString value;
        QTextStream stream2( &value, QIODevice::WriteOnly);
        stream2 << map.value( key);                                         // This can be self-referential.

        if (!SERIALIZE_EDITOR->exists( SERIALIZE_KEYS))                     // Check here, so that it doesn't overwrite (with an empty value).
            SERIALIZE_EDITOR->setValue( SERIALIZE_KEYS, value);

        SERIALIZE_KEYS.removeLast();
    }

    return stream;
}

template <class T>
inline QTextStream& operator >> (QTextStream &stream, QMap<QString, T>& map)
{
    for (const QVariant& key : DESERIALIZE_EDITOR->keys({ DESERIALIZE_KEYS}))
    {
        DESERIALIZE_KEYS.append( key.toString());

        QString strValue = DESERIALIZE_EDITOR->value( DESERIALIZE_KEYS, QVariant()).toString();
        QTextStream stream2( &strValue, QIODevice::ReadOnly);
        T value;
        stream2 >> value;                                                   // This can be self-referential.
        map.insert( key.toString(), value);

        DESERIALIZE_KEYS.removeLast();
    }
    return stream;
}

// =============================== DATASTREAM OVERLOAD ===============================

// QObject

inline QDataStream& operator << (QDataStream &stream, const QObject &obj)
{
    for (int i = 1; i < obj.metaObject()->propertyCount(); ++i)
        if (obj.metaObject()->property(i).isStored( &obj))
            stream << obj.metaObject()->property(i).read( &obj);
    return stream;
}

inline QDataStream& operator >> (QDataStream &stream, QObject &obj)
{
    QVariant value;
    for (int i = 1; i < obj.metaObject()->propertyCount(); ++i)
        if (obj.metaObject()->property(i).isStored( &obj))
        {
            stream >> value;
            obj.metaObject()->property(i).write( &obj, value);
        }
    return stream;
}

}

#endif // JSONWAX_SERIALIZER_H
