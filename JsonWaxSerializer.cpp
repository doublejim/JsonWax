#include "JsonWaxSerializer.h"

namespace JsonWaxInternals {
    thread_local SerialInOut SERIAL_IN_OUT;
}

void JsonWaxInternals::Serializer::prepareEditor()
{
    SERIAL_IN_OUT.SERIALIZE_TO_EDITOR = false;        // This value will change if the type was serialized to the Editor.
    SERIAL_IN_OUT.SERIALIZE_KEYS = {0};

    if (SERIAL_IN_OUT.SERIALIZE_EDITOR == nullptr)
        SERIAL_IN_OUT.SERIALIZE_EDITOR = new JsonWaxInternals::MainEditor;
    else
        SERIAL_IN_OUT.SERIALIZE_EDITOR->clear();
}

JsonWaxInternals::Serializer::Serializer(){}

JsonWaxInternals::Serializer::~Serializer(){
    delete SERIAL_IN_OUT.SERIALIZE_EDITOR;
    SERIAL_IN_OUT.SERIALIZE_EDITOR = nullptr;
}

// QColor (only if project has QT += gui)
#ifdef QT_GUI_LIB
QTextStream &JsonWaxInternals::operator <<(QTextStream &stream, const QColor &color)
{
    stream << "#";
    stream << QString::number( color.alpha(), 16).rightJustified(2, '0');
    stream << QString::number( color.red(), 16).rightJustified(2, '0');
    stream << QString::number( color.green(), 16).rightJustified(2, '0');
    stream << QString::number( color.blue(), 16).rightJustified(2, '0');
    return stream;
}

QTextStream &JsonWaxInternals::operator >>(QTextStream &stream, QColor &color)
{
    QString sColor;
    stream >> sColor;
    color.setNamedColor( sColor);
    return stream;
}
#endif

QTextStream &JsonWaxInternals::operator <<(QTextStream &stream, const QDate &date)
{
    stream << date.toString(Qt::ISODate);
    return stream;
}

QTextStream &JsonWaxInternals::operator >>(QTextStream &stream, QDate &date)
{
    QString sDate;
    stream >> sDate;
    date = QDate::fromString( sDate, Qt::ISODate);
    return stream;
}

QTextStream &JsonWaxInternals::operator <<(QTextStream &stream, const QTime &time)
{
#if (QT_VERSION >= 0x050800)
    stream << time.toString(Qt::ISODateWithMs);             // This format is only available from Qt 5.8.
#else
    stream << time.toString(Qt::ISODate) << '.' << time.msec();
#endif
    //}
    return stream;
}



QTextStream &JsonWaxInternals::operator >>(QTextStream &stream, QTime &time)
{
    QString value;
    stream >> value;
#if (QT_VERSION >= 0x050800)
    time = QTime::fromString( value, Qt::ISODateWithMs);    // This format is only available from Qt 5.8.
#else
    time = QTime::fromString( value, Qt::ISODate);
#endif
    return stream;
}

QTextStream &JsonWaxInternals::operator <<(QTextStream &stream, const QDateTime &dateTime)
{
    SERIAL_IN_OUT.SERIALIZE_TO_EDITOR = true;

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

QTextStream &JsonWaxInternals::operator >>(QTextStream &stream, QDateTime &dateTime)
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

QTextStream &JsonWaxInternals::operator <<(QTextStream &stream, const QLine &line)
{
    SERIAL_IN_OUT.SERIALIZE_TO_EDITOR = true;
    writeToSeEditor("x1", line.x1());
    writeToSeEditor("y1", line.y1());
    writeToSeEditor("x2", line.x2());
    writeToSeEditor("y2", line.y2());
    return stream;
}

QTextStream &JsonWaxInternals::operator >>(QTextStream &stream, QLine &line)
{
    int x1 = readFromDeEditor<int>("x1");
    int y1 = readFromDeEditor<int>("y1");
    int x2 = readFromDeEditor<int>("x2");
    int y2 = readFromDeEditor<int>("y2");
    line.setLine( x1, y1, x2, y2);
    return stream;
}

QTextStream &JsonWaxInternals::operator <<(QTextStream &stream, const QLineF &line)
{
    SERIAL_IN_OUT.SERIALIZE_TO_EDITOR = true;
    writeToSeEditor("x1", line.x1());
    writeToSeEditor("y1", line.y1());
    writeToSeEditor("x2", line.x2());
    writeToSeEditor("y2", line.y2());
    return stream;
}

QTextStream &JsonWaxInternals::operator >>(QTextStream &stream, QLineF &line)
{
    qreal x1 = readFromDeEditor<qreal>("x1");
    qreal y1 = readFromDeEditor<qreal>("y1");
    qreal x2 = readFromDeEditor<qreal>("x2");
    qreal y2 = readFromDeEditor<qreal>("y2");
    line.setLine( x1, y1, x2, y2);
    return stream;
}

QTextStream &JsonWaxInternals::operator <<(QTextStream &stream, const QObject &obj)
{
    // This stores all the QObject properties in the SERIALIZE_EDITOR.

    SERIAL_IN_OUT.SERIALIZE_TO_EDITOR = true;

    // Dynamic properties:

    QList<QByteArray> dynamicProperties = obj.dynamicPropertyNames();

    for (int i = 0; i < dynamicProperties.size(); ++i)
    {
        QString name = QString( dynamicProperties.at(i));
        QString value = obj.property( dynamicProperties.at(i)).toString();
        SERIAL_IN_OUT.SERIALIZE_EDITOR->setValue({0, name}, value);
    }

    // Stored properties:

    for (int i = 0; i < obj.metaObject()->propertyCount(); ++i)
        if (obj.metaObject()->property(i).isStored( &obj))
        {
            QString name = QString( obj.metaObject()->property(i).name());
            QString value = obj.metaObject()->property(i).read( &obj).toString();
            SERIAL_IN_OUT.SERIALIZE_EDITOR->setValue({0, name}, value);
        }

    return stream;
}

QTextStream &JsonWaxInternals::operator >>(QTextStream &stream, QObject &obj)
{
    // This inserts property values from a JSON-document into the QObject.
    // The DESERIALIZE_EDITOR is a pointer to the main editor in "JsonWax.h".
    // It finds all the subkeys in that location, and inserts them as properties.

    ObjectEditor objectEditor = SERIAL_IN_OUT.DESERIALIZE_EDITOR->toObjectEditor( SERIAL_IN_OUT.DESERIALIZE_KEYS);

    for (QString& key : objectEditor.keys())
    {
        QVariant value = objectEditor.value( key);
        obj.setProperty( key.toUtf8(), value);
    }
    return stream;
}

QTextStream &JsonWaxInternals::operator <<(QTextStream &stream, const QPoint &point)
{
    SERIAL_IN_OUT.SERIALIZE_TO_EDITOR = true;
    writeToSeEditor("x", point.x());
    writeToSeEditor("y", point.y());
    return stream;
}

QTextStream &JsonWaxInternals::operator >>(QTextStream &stream, QPoint &point)
{
    int x = readFromDeEditor<int>("x");
    int y = readFromDeEditor<int>("y");
    point.setX( x);
    point.setY( y);
    return stream;
}

QTextStream &JsonWaxInternals::operator <<(QTextStream &stream, const QPointF &point)
{
    SERIAL_IN_OUT.SERIALIZE_TO_EDITOR = true;
    writeToSeEditor("x", point.x());
    writeToSeEditor("y", point.y());
    return stream;
}

QTextStream &JsonWaxInternals::operator >>(QTextStream &stream, QPointF &point)
{
    qreal x = readFromDeEditor<qreal>("x");
    qreal y = readFromDeEditor<qreal>("y");
    point.setX( x);
    point.setY( y);
    return stream;
}

QTextStream &JsonWaxInternals::operator <<(QTextStream &stream, const QRect &rect)
{
    SERIAL_IN_OUT.SERIALIZE_TO_EDITOR = true;
    writeToSeEditor("left", rect.left());
    writeToSeEditor("top", rect.top());
    writeToSeEditor("right", rect.right());
    writeToSeEditor("bottom", rect.bottom());
    return stream;
}

QTextStream &JsonWaxInternals::operator >>(QTextStream &stream, QRect &rect)
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

QTextStream &JsonWaxInternals::operator <<(QTextStream &stream, const QRectF &rect)
{
    SERIAL_IN_OUT.SERIALIZE_TO_EDITOR = true;
    writeToSeEditor("left", rect.left());
    writeToSeEditor("top", rect.top());
    writeToSeEditor("right", rect.right());
    writeToSeEditor("bottom", rect.bottom());
    return stream;
}

QTextStream &JsonWaxInternals::operator >>(QTextStream &stream, QRectF &rect)
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

QTextStream &JsonWaxInternals::operator <<(QTextStream &stream, const QSize &size)
{
    SERIAL_IN_OUT.SERIALIZE_TO_EDITOR = true;
    writeToSeEditor("width", size.width());
    writeToSeEditor("height", size.height());
    return stream;
}

QTextStream &JsonWaxInternals::operator >>(QTextStream &stream, QSize &size)
{
    int width = readFromDeEditor<int>("width");
    int height = readFromDeEditor<int>("height");
    size.setWidth( width);
    size.setHeight( height);
    return stream;
}

QTextStream &JsonWaxInternals::operator <<(QTextStream &stream, const QUrl &url)
{
    stream << url.toString();
    return stream;
}

QTextStream &JsonWaxInternals::operator >>(QTextStream &stream, QUrl &url)
{
    QString strUrl;
    stream >> strUrl;
    url = QUrl(strUrl);
    return stream;
}

QTextStream &JsonWaxInternals::operator <<(QTextStream &stream, const QVariant &variant)
{
    SERIAL_IN_OUT.SERIALIZE_TO_EDITOR = true;
    writeToSeEditor("type", variant.type());
    if (!variant.toString().isEmpty())
        writeToSeEditor("value", variant.toString());
    return stream;
}

QTextStream &JsonWaxInternals::operator >>(QTextStream &stream, QVariant &variant)
{
    int type = readFromDeEditor<int>("type");
    QString value = readFromDeEditor<QString>("value");
    variant.setValue( value);
    variant.convert( QVariant::Type( type));
    return stream;
}

QDataStream &JsonWaxInternals::operator <<(QDataStream &stream, const QObject &obj)
{
    // Stored properties:

    for (int i = 1; i < obj.metaObject()->propertyCount(); ++i)
        stream << obj.metaObject()->property(i).read( &obj);

    // Dynamic properties:

    for (QByteArray& name : obj.dynamicPropertyNames())
    {
        stream << name;
        stream << obj.property( name);
    }

    return stream;
}

QDataStream &JsonWaxInternals::operator >>(QDataStream &stream, QObject &obj)
{
    QVariant value;

    // Stored properties.

    for (int i = 1; i < obj.metaObject()->propertyCount(); ++i)
    {
        stream >> value;
        obj.metaObject()->property(i).write( &obj, value);
    }

    // Dynamic properties:

    QByteArray name;

    while (!stream.atEnd())
    {
        stream >> name;
        stream >> value;
        obj.setProperty( name, value);
    }

    return stream;
}

#include "JsonWaxSerializer.h"
