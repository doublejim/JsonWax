#include "JsonWaxData.h"

namespace JsonWaxInternals {
    thread_local StringInOut STR_IN_OUT;

    static void indent(const int &indentation)
    {
        for (int i = 0; i < indentation; ++i)
            for (int j = 0; j < 4; ++j)         // 4 spaces.
                STR_IN_OUT.TO_STRING.append(' ');
    }
    static void toJsonString(const QString &input)
    {
        for (const QChar& ch : input)
            switch( ch.unicode())
            {
            case '\\': STR_IN_OUT.TO_STRING.append('\\'); STR_IN_OUT.TO_STRING.append('\\');  break;    // Breaks special characters up into two characters.
            case '\"': STR_IN_OUT.TO_STRING.append('\\'); STR_IN_OUT.TO_STRING.append('\"');  break;
            case '\b': STR_IN_OUT.TO_STRING.append('\\'); STR_IN_OUT.TO_STRING.append('b');   break;
            case '\f': STR_IN_OUT.TO_STRING.append('\\'); STR_IN_OUT.TO_STRING.append('f');   break;
            case '\n': STR_IN_OUT.TO_STRING.append('\\'); STR_IN_OUT.TO_STRING.append('n');   break;
            case '\r': STR_IN_OUT.TO_STRING.append('\\'); STR_IN_OUT.TO_STRING.append('r');   break;
            case '\t': STR_IN_OUT.TO_STRING.append('\\'); STR_IN_OUT.TO_STRING.append('t');   break;
            default:
                if (!STR_IN_OUT.CONVERT_TO_CODE_POINTS)
                {
                    STR_IN_OUT.TO_STRING.append(ch);
                } else {
                    if ( ch.unicode() > 31 && ch.unicode() < 127)
                    {
                        STR_IN_OUT.TO_STRING.append( ch);
                    } else {
                        STR_IN_OUT.TO_STRING.append( '\\');
                        STR_IN_OUT.TO_STRING.append( 'u');
                        STR_IN_OUT.TO_STRING.append( QString::number( ch.unicode(), 16).rightJustified(4, '0'));
                    }
                }
            }
    }

    void ReferenceCounted::incRefCount()
    {
        ++REFERENCES;
    }

    int ReferenceCounted::decRefCount()
    {
        --REFERENCES;
        return REFERENCES;
    }
}

JsonWaxInternals::JsonValue::JsonValue()
{
    //qDebug() << "constructing value" << this;
    hasType = Type::Value;
}

JsonWaxInternals::JsonValue::JsonValue(const QVariant &value)
{
    //qDebug() << "constructing value from qvariant" << this;
    hasType = Type::Value;
    setValue({}, value);
}

void JsonWaxInternals::JsonValue::destroy()
{
    //qDebug() << "destroying value" << VALUE << '(' << this << ')';
    delete this;
}

void JsonWaxInternals::JsonValue::toString(JsonWaxInternals::StringStyle style, int indentation) const
{
    Q_UNUSED(style);
    Q_UNUSED(indentation);

    switch( static_cast<QMetaType::Type>( VALUE.type()))
    {
    case QVariant::String: case QMetaType::QChar:
    {
        STR_IN_OUT.TO_STRING.append('\"');
        toJsonString( VALUE.toString());
        STR_IN_OUT.TO_STRING.append('\"');
        break;
    }
    case QMetaType::Int: case QMetaType::UInt:
    case QMetaType::Double: case QMetaType::Float:
    case QMetaType::LongLong: case QMetaType::ULongLong:
    case QMetaType::Bool:
        STR_IN_OUT.TO_STRING.append( VALUE.toString());
        break;
    case QVariant::Invalid:
        STR_IN_OUT.TO_STRING.append("null");
        break;
    default:
        STR_IN_OUT.TO_STRING.append("ERROR");
    }
}

void JsonWaxInternals::JsonValue::setValue(const QVariant &key, const QVariant &value)
{
    // qDebug() << "json set value";
    Q_UNUSED(key);
    VALUE = value;
}

JsonWaxInternals::JsonType *JsonWaxInternals::JsonValue::insertWeak(const QVariant &key, JsonWaxInternals::JsonType *fresh_element)
{
    Q_UNUSED(key);
    // delete fresh_element;
    fresh_element->destroy();
    return this;
}

JsonWaxInternals::JsonType *JsonWaxInternals::JsonValue::insertStrong(const QVariant &key, JsonWaxInternals::JsonType *fresh_element)
{
    Q_UNUSED(key);
    // delete fresh_element;
    fresh_element->destroy();
    return this;
}

QVariantList JsonWaxInternals::JsonValue::keys()
{
    return QVariantList();
}

JsonWaxInternals::JsonType *JsonWaxInternals::JsonValue::value(const QVariant &key)
{
    Q_UNUSED(key);
    return nullptr;
}

bool JsonWaxInternals::JsonValue::remove(const QVariant &key)
{
    Q_UNUSED(key);
    return false;
}

bool JsonWaxInternals::JsonValue::removeWeak(const QVariant &key)
{
    Q_UNUSED(key);
    return false;
}

bool JsonWaxInternals::JsonValue::contains(const QVariant &key)
{
    Q_UNUSED(key);
    return false;
}

int JsonWaxInternals::JsonValue::size()
{
    return 1;
}

bool JsonWaxInternals::JsonObject::insertBase(const QVariant &key, JsonWaxInternals::JsonType *fresh_element)
{
    JsonType* value = MAP.value( key.toString(), 0);

    if (value != nullptr)
    {
        if (value->hasType != fresh_element->hasType)       // The value is of a wrong type.
        {                                                   // Delete its data and use the fresh_element.
            // delete value;
            value->destroy();
        } else {
            INSERTED_ELEMENT = value;
            return false;
        }
    }
    MAP.insert( key.toString(), fresh_element);             // There was no value at the key.
    INSERTED_ELEMENT = fresh_element;
    return true;
}

JsonWaxInternals::JsonObject::JsonObject()
{
    hasType = Type::Object;
}

void JsonWaxInternals::JsonObject::destroy()
{
    //qDebug() << "json object destroy. refcount:" << REFERENCES;
    //qDebug() << "refcount before attempt destroy jsonarray." << this << REFERENCES;

    if (decRefCount() < 1)                      // If there are no more references to this container.
    {
        //qDebug() << "destroying json object" << MAP.keys() << '(' << "size" << MAP.size() << ')' << '(' << this << ')';

        for (JsonType* jt : MAP.values())       // Attempt to destroy the sub-containers.
            /*if (jt->hasType == Type::Value)
            {
                delete jt;
            } else {
                if (static_cast<ReferenceCounted*>(jt)->decRefCount() < 1)
                    delete jt;
            }*/
            jt->destroy();

        delete this;
    }                                           // If there is at least one reference left, don't delete this container.
}

QVariantList JsonWaxInternals::JsonObject::keys()
{
    QVariantList result;

    for (QString str : MAP.keys())
        result.append( str);

    return result;
}

void JsonWaxInternals::JsonObject::toString(JsonWaxInternals::StringStyle style, int indentation) const
{
    STR_IN_OUT.TO_STRING.append('{');

    switch( style)
    {
    case StringStyle::Readable:
        STR_IN_OUT.TO_STRING.append('\n');

        for( QMap<QString, JsonType*>::const_iterator it = MAP.cbegin(); it != MAP.cend(); ++it)
        {
            indent( indentation);

            STR_IN_OUT.TO_STRING.append('\"');
            toJsonString( it.key());
            STR_IN_OUT.TO_STRING.append('\"');
            STR_IN_OUT.TO_STRING.append(':');
            STR_IN_OUT.TO_STRING.append(' ');
            it.value()->toString( style, indentation + 1);
            STR_IN_OUT.TO_STRING.append(',');
            STR_IN_OUT.TO_STRING.append('\n');
        }

        if (MAP.isEmpty())
            STR_IN_OUT.TO_STRING.chop(1);      // Chop off '\n'
        else
            STR_IN_OUT.TO_STRING.chop(2);      // Chop off ",\n"

        STR_IN_OUT.TO_STRING.append('\n');
        indent( indentation - 1);
        break;
    case StringStyle::Compact:
        for( QMap<QString, JsonType*>::const_iterator it = MAP.cbegin(); it != MAP.cend(); ++it)
        {
            STR_IN_OUT.TO_STRING.append('\"');
            toJsonString( it.key());
            STR_IN_OUT.TO_STRING.append('\"');
            STR_IN_OUT.TO_STRING.append(':');
            it.value()->toString(style);
            STR_IN_OUT.TO_STRING.append(',');
        }

        if (!MAP.isEmpty())
            STR_IN_OUT.TO_STRING.chop(1);      // Chop off ','
        break;
    }
    STR_IN_OUT.TO_STRING.append('}');
}

bool JsonWaxInternals::JsonObject::isValidKey(const QVariant &key)
{
    if (key.type() == QVariant::String)
        return true;
    return false;
}

JsonWaxInternals::JsonType *JsonWaxInternals::JsonObject::insertWeak(const QVariant &key, JsonWaxInternals::JsonType *fresh_element)
{
    if (!insertBase( key, fresh_element))
        fresh_element->destroy();
        // delete fresh_element;

    return INSERTED_ELEMENT;
}

JsonWaxInternals::JsonType *JsonWaxInternals::JsonObject::insertStrong(const QVariant &key, JsonWaxInternals::JsonType *fresh_element)
{
    if (!insertBase( key, fresh_element))
    {
        INSERTED_ELEMENT->destroy();
        // delete INSERTED_ELEMENT;
        MAP.insert( key.toString(), fresh_element);
    }
    return fresh_element;
}

void JsonWaxInternals::JsonObject::setValue(const QVariant &key, const QVariant &value)              // key is expected to be a string.
{
    if (MAP.contains( key.toString()))
    {
        if (MAP[ key.toString()]->hasType != Type::Value)
        {
            remove( key);                                               // It deletes any existing object or array.
            MAP.insert( key.toString(), new JsonValue(value));
        } else {
            MAP[ key.toString()]->setValue( {}, value);
        }
    } else {
        MAP.insert( key.toString(), new JsonValue(value));
    }
}

JsonWaxInternals::JsonType *JsonWaxInternals::JsonObject::value(const QVariant &key)
{
    if (isValidKey(key))
        return MAP.value( key.toString(), nullptr);
    return nullptr;
}

bool JsonWaxInternals::JsonObject::contains(const QVariant &key)
{
    if (key.type() != QVariant::String)
        return false;
    return MAP.contains( key.toString());
}

bool JsonWaxInternals::JsonObject::remove(const QVariant &key)
{
    JsonType* jsonType = MAP.value( key.toString(), nullptr);

    if (jsonType != nullptr)
        jsonType->destroy();

    int i = MAP.remove( key.toString());

    return (i == 0) ? false : true;
}

bool JsonWaxInternals::JsonObject::removeWeak(const QVariant &key)
{
    int i = MAP.remove( key.toString());
    return i==0 ? false : true;
}

int JsonWaxInternals::JsonObject::size()
{
    return MAP.size();
}

bool JsonWaxInternals::JsonArray::insertBase(const QVariant &key, JsonWaxInternals::JsonType *fresh_element)
{
    inflate( key.toInt() + 1);
    JsonType* val = value( key);

    if (val != nullptr)     // will it ever be a nullptr? I just resized the array.
    {
        if (val->hasType != fresh_element->hasType)
        {
            val->destroy();
            // delete val;
        } else {
            INSERTED_ELEMENT = val;
            return false;
        }
    }
    ARRAY[ key.toInt()] = fresh_element;
    INSERTED_ELEMENT = fresh_element;
    return true;
}

JsonWaxInternals::JsonArray::JsonArray()
{
    hasType = Type::Array;
}

void JsonWaxInternals::JsonArray::destroy()
{
    //qDebug() << "json array destroy. refcount:" << REFERENCES;
    //qDebug() << "refcount before attempt destroy jsonarray." << this << REFERENCES;

    if (decRefCount() < 1)              // If there are no more references to this container.
    {
        //qDebug() << "destroying json array (size" << ARRAY.size() << ')' << ARRAY.toVector() << '(' << this << ')';
        for (JsonType* jt : ARRAY)      // Attempt to destroy the sub-containers.
            jt->destroy();
        delete this;
    }                                   // If there is at least one reference left, don't delete this container.
}

bool JsonWaxInternals::JsonArray::isValidKey(const QVariant &key)
{
    if (key.type() == QVariant::Int && key.toInt() >= 0)
        return true;
    return false;
}

void JsonWaxInternals::JsonArray::inflate(int elementCount)
{
    while (ARRAY.size() < elementCount)
        ARRAY.append( new JsonValue);
}

QVariantList JsonWaxInternals::JsonArray::keys()
{
    QVariantList result;

    for (int i = 0; i < ARRAY.size(); ++i)
        result.append(i);

    return result;
}

void JsonWaxInternals::JsonArray::toString(JsonWaxInternals::StringStyle style, int indentation) const
{
    STR_IN_OUT.TO_STRING.append('[');

    switch(style)
    {
    case StringStyle::Readable:
        for (const JsonType* jt : ARRAY)
        {
            STR_IN_OUT.TO_STRING.append('\n');
            indent( indentation);
            jt->toString( style, indentation + 1);
            STR_IN_OUT.TO_STRING.append(',');
        }

        if (!ARRAY.isEmpty())
            STR_IN_OUT.TO_STRING.chop(1);          // Chop off ','

        STR_IN_OUT.TO_STRING.append('\n');
        indent( indentation - 1);
        break;
    case StringStyle::Compact:
        for (const JsonType* jt : ARRAY)
        {
            jt->toString( style);
            STR_IN_OUT.TO_STRING.append(',');
        }

        if (!ARRAY.isEmpty())
            STR_IN_OUT.TO_STRING.chop(1);          // Chop off ','
        break;
    }
    STR_IN_OUT.TO_STRING.append(']');
}

JsonWaxInternals::JsonType *JsonWaxInternals::JsonArray::insertWeak(const QVariant &key, JsonWaxInternals::JsonType *fresh_element)
{
    if (!isValidKey( key))
    {
        // delete fresh_element;
        fresh_element->destroy();
        return nullptr;
    }

    if (!insertBase( key, fresh_element))
        fresh_element->destroy();
        //delete fresh_element;

    return INSERTED_ELEMENT;
}

JsonWaxInternals::JsonType *JsonWaxInternals::JsonArray::insertStrong(const QVariant &key, JsonWaxInternals::JsonType *fresh_element)
{
    if (!isValidKey( key))
    {
        qWarning("JsonWax-insert error: invalid key.");
        //delete fresh_element;  // destroy?
        fresh_element->destroy();
        return nullptr;
    }

    if (!insertBase( key, fresh_element))
    {
        INSERTED_ELEMENT->destroy();
        // delete INSERTED_ELEMENT;
        ARRAY[ key.toInt()] = fresh_element;
    }
    return fresh_element;
}

void JsonWaxInternals::JsonArray::setValue(const QVariant &key, const QVariant &value)
{
    if (!isValidKey( key))
        return;

    int intkey = key.toInt();
    inflate( intkey + 1);

    if (ARRAY[ intkey]->hasType == Type::Value)
    {
        ARRAY[ intkey]->setValue( {}, value);
    } else {
        ARRAY.at( intkey)->destroy();
        ARRAY[ intkey] = new JsonValue(value);
    }
}

JsonWaxInternals::JsonType *JsonWaxInternals::JsonArray::value(const QVariant &key)
{
    if (contains(key))
        return ARRAY[key.toInt()];
    return nullptr;
}

bool JsonWaxInternals::JsonArray::contains(const QVariant &key)
{
    if (isValidKey(key) && key.toInt() < ARRAY.size())
        return true;
    return false;
}

bool JsonWaxInternals::JsonArray::remove(const QVariant &key)
{
    if (!contains( key))
        return false;

    // SPECIAL TEST FIX.
    ARRAY.at( key.toInt())->destroy();
    ARRAY.removeAt( key.toInt());
    return true;
}

bool JsonWaxInternals::JsonArray::removeWeak(const QVariant &key)
{
    if (key.isNull())
        ARRAY[ key.toInt()] = new JsonValue();

    if (!contains( key))
        return false;
    ARRAY[ key.toInt()] = new JsonValue();
    return true;
}

int JsonWaxInternals::JsonArray::size()
{
    return ARRAY.size();
}

#include "JsonWaxData.h"
