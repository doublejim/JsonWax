#ifndef JSONWAX_EDITOR_H
#define JSONWAX_EDITOR_H

#include <QByteArray>

/* TODO:
 * - serialization write.
 * - serialization read.
 * - Problem: Make sure that documents are stored using UTF-8 codec, before loading them.
 */

namespace JsonWaxInternals {

    enum Type {Value, Object, Array};
    enum StringStyle {Compact, Readable};

    static bool CONVERT_TO_CODE_POINTS = false;

static void indent( QString& str, int indentation)
{
    for (int i = 0; i < indentation; ++i)
        str.append("    ");
}

static QString toJsonString( const QString& input)
{
    QString result;

    for (const QChar& ch : input)
        switch( ch.unicode())
        {
        case '\\': result.append('\\'); result.append('\\');  break;    // Breaks special characters up into two characters.
        case '\"': result.append("\\"); result.append('\"');  break;
        case '\b': result.append('\\'); result.append('b');   break;
        case '\f': result.append('\\'); result.append('f');   break;
        case '\n': result.append('\\'); result.append('n');   break;
        case '\r': result.append('\\'); result.append('r');   break;
        case '\t': result.append('\\'); result.append('t');   break;
        default:
            if (!CONVERT_TO_CODE_POINTS)
            {
                result.append(ch);
            } else {
                if ( ch.unicode() > 31 && ch.unicode() < 127)
                {
                    result.append( ch);
                } else {
                    result.append( "\\u");
                    result.append( QString::number( ch.unicode(), 16).rightJustified(4, '0'));
                }
            }
        }
    return result;
}

// -------------------------------------------------

class JsonType
{
public:
    Type hasType;

    JsonType(){}

    JsonType( Type type)
    {
        setType( type);
    }

    void setType( Type type)
    {
        hasType = type;
    }

    virtual ~JsonType(){}
    virtual QString toString( StringStyle style, int indentation = 0) = 0;
    virtual JsonType* insertWeak( const QVariant& key, JsonType* fresh_element) = 0;
    virtual JsonType* insertStrong( const QVariant& key, JsonType* fresh_element) = 0;
    virtual void setValue( const QVariant& key, const QVariant& value) = 0;
    virtual JsonType* value( const QVariant& key) = 0;
    virtual bool remove( const QVariant& key) = 0;
    virtual bool removeWeak( const QVariant& key) = 0;
    virtual bool contains( const QVariant& key) = 0;
    virtual int size() = 0;

    virtual QVariantList keys()
    {
        return QVariantList();
    }
};

class JsonValue : public JsonType
{
public:
    JsonValue()
    {
        setType( Type::Value);
    }

    JsonValue( const QVariant& value)
    {
        setType( Type::Value);
        setValue({}, value);
    }

    ~JsonValue(){}

    QVariant VALUE;

    QString toString( StringStyle style, int indentation = 0)
    {
        Q_UNUSED(style);
        Q_UNUSED(indentation);

        QString result;

        switch(static_cast<QMetaType::Type>(VALUE.type()))
        {
        case QVariant::String:
        {
            result.append('\"');
            result.append( toJsonString( VALUE.toString()));
            result.append('\"');
            break;
        }
        case QMetaType::Int: case QMetaType::Double: case QMetaType::Float: case QMetaType::Bool:
            result.append( VALUE.toString());
            break;
        case QVariant::Invalid:
            result.append("null");
            break;
        default:
            result.append("ERROR");
        }

        return result;
    }

    void setValue(const QVariant& key, const QVariant& value)
    {
        Q_UNUSED(key);
        VALUE = value;
    }

    JsonType* insertWeak( const QVariant& key, JsonType* fresh_element)
    {
        Q_UNUSED(key);
        delete fresh_element;
        return this;
    }

    JsonType* insertStrong( const QVariant& key, JsonType* fresh_element)
    {
        Q_UNUSED(key);
        delete fresh_element;
        return this;
    }

    JsonType* value( const QVariant& key)
    {
        Q_UNUSED(key);
        return 0;
    }

    bool remove( const QVariant& key)
    {
        Q_UNUSED(key);
        return false;
    }

    bool removeWeak( const QVariant& key)
    {
        Q_UNUSED(key);
        return false;
    }

    bool contains( const QVariant& key)
    {
        Q_UNUSED(key);
        return false;
    }

    int size()
    {
        return 1;
    }
};

class JsonObject : public JsonType
{
private:
    JsonType* INSERTED_ELEMENT = 0;

    bool insertBase( const QVariant& key, JsonType* fresh_element)
    {
        JsonType* value = MAP.value( key.toString(), 0);

        if (value != nullptr)
        {
            if (value->hasType != fresh_element->hasType)       // The value is of a wrong type.
            {                                                   // Delete its data and use the fresh_element.
                delete value;
            } else {
                INSERTED_ELEMENT = value;
                return false;
            }
        }
        MAP.insert( key.toString(), fresh_element);         // There was no value at the key.
        INSERTED_ELEMENT = fresh_element;
        return true;
    }

public:
    QMap<QString, JsonType*> MAP;

    JsonObject()
    {
        setType( Type::Object);
    }

    ~JsonObject()
    {
        for (JsonType* jt : MAP.values())
            delete jt;
    }

    QVariantList keys()
    {
        QVariantList result;

        for (QString str : MAP.keys())
            result.append( str);

        return result;
    }

    QString toString( StringStyle style, int indentation = 0)
    {
        QString result;

        result.append('{');

        switch( style)
        {
        case StringStyle::Readable:
            result.append('\n');

            for (QString& key : MAP.keys())
            {
                indent( result, indentation);

                result.append('\"');
                result.append( toJsonString( key));
                result.append('\"');
                result.append(':');
                result.append(' ');
                result.append( MAP.value(key)->toString( style, indentation + 1));
                result.append(",\n");
            }

            if (!MAP.isEmpty())
                result.chop(2);

            result.append('\n');
            indent( result, indentation - 1);
            break;
        case StringStyle::Compact:

            for (QString& key : MAP.keys())
            {
                result.append('\"');
                result.append( toJsonString( key));
                result.append('\"');
                result.append(':');
                result.append( MAP.value(key)->toString( style));
                result.append(',');
            }

            if (!MAP.isEmpty())
                result.chop(1);
            break;
        }
        result.append('}');

        return result;
    }

    JsonType* insertWeak( const QVariant& key, JsonType* fresh_element)
    {
        if (!insertBase( key, fresh_element))
            delete fresh_element;

        return INSERTED_ELEMENT;
    }

    JsonType* insertStrong( const QVariant& key, JsonType* fresh_element)
    {
        if (!insertBase( key, fresh_element))
        {
            delete INSERTED_ELEMENT;
            MAP.insert( key.toString(), fresh_element);
        }
        return fresh_element;
    }

    void setValue( const QVariant& key, const QVariant& value)              // key is expected to be a string.
    {
        if (MAP.contains( key.toString()))
        {
            if (MAP[ key.toString()]->hasType != Type::Value)
            {
                remove( key);                                               // It deletes any existing object or array.
                MAP.insert( key.toString(), new JsonValue(value));
            }
            else
                MAP[ key.toString()]->setValue( {}, value);
        }
        else MAP.insert( key.toString(), new JsonValue(value));
    }

    JsonType* value( const QVariant& key)
    {
        return MAP.value( key.toString(), 0);
    }

    bool contains( const QVariant& key)
    {
        if (key.type() != QVariant::String)
            return false;
        else
            return MAP.contains( key.toString());
    }

    bool remove( const QVariant& key)
    {
        delete MAP.value( key.toString(), 0);
        int i = MAP.remove( key.toString());
        return i==0 ? false : true;
    }

    bool removeWeak( const QVariant& key)
    {
        int i = MAP.remove( key.toString());
        return i==0 ? false : true;
    }

    int size()
    {
        return MAP.size();
    }
};

class JsonArray : public JsonType
{
private:
    JsonType* INSERTED_ELEMENT = 0;

    bool insertBase( const QVariant& key, JsonType* fresh_element)
    {
        inflate( key.toInt() + 1);
        JsonType* val = value( key);

        if (val != nullptr)
        {
            if (val->hasType != fresh_element->hasType)
            {
                delete val;
            } else {
                INSERTED_ELEMENT = val;
                return false;
            }
        }
        ARRAY[ key.toInt()] = fresh_element;
        INSERTED_ELEMENT = fresh_element;
        return true;
    }

public:
    JsonArray()
    {
        setType( Type::Array);
    }

    ~JsonArray()
    {
        for (JsonType* jt : ARRAY)
            delete jt;
    }

    QList<JsonType*> ARRAY;

    bool isValidKey( const QVariant& key)
    {
        if (key.type() == QVariant::Int && key.toInt() >= 0)
            return true;
        return false;
    }

    void inflate( int elementCount)
    {
        while (ARRAY.size() < elementCount)
            ARRAY.append( new JsonValue());
    }

    QVariantList keys()
    {
        QVariantList result;

        for (int i = 0; i < ARRAY.size(); ++i)
            result.append(i);

        return result;
    }

    QString toString( StringStyle style, int indentation = 0)
    {
        QString result;
        result.append('[');

        switch(style)
        {
        case StringStyle::Readable:
            for (JsonType* jt : ARRAY)
            {
                result.append('\n');
                indent( result, indentation);
                result.append( jt->toString( style, indentation + 1));
                result.append(",");
            }

            if (!ARRAY.isEmpty())
                result.chop(1);

            result.append('\n');
            indent( result, indentation - 1);
            break;
        case StringStyle::Compact:
            for (JsonType* jt : ARRAY)
            {
                result.append( jt->toString( style));
                result.append(',');
            }

            if (!ARRAY.isEmpty())
                result.chop(1);
            break;
        }

        result.append(']');
        return result;
    }

    JsonType* insertWeak( const QVariant& key, JsonType* fresh_element)
    {
        if (!isValidKey( key))
        {
            delete fresh_element;
            return 0;
        }

        if (!insertBase( key, fresh_element))
            delete fresh_element;

        return INSERTED_ELEMENT;
    }

    JsonType* insertStrong( const QVariant& key, JsonType* fresh_element)
    {
        if (!isValidKey( key))
        {
            delete fresh_element;
            return 0;
        }

        if (!insertBase( key, fresh_element))
        {
            delete INSERTED_ELEMENT;
            ARRAY[ key.toInt()] = fresh_element;
        }
        return fresh_element;
    }

    void setValue( const QVariant& key, const QVariant& value)    // Key is required to be an int.
    {
        if (!isValidKey( key))
            return;

        int intkey = key.toInt();
        inflate( intkey + 1);

        if (ARRAY[ intkey]->hasType != Type::Value)
        {
            delete ARRAY.at( intkey);
            ARRAY[ intkey] = new JsonValue(value);
        } else {
            ARRAY[ intkey]->setValue( {}, value);
        }
    }

    JsonType* value( const QVariant& key)
    {
        if (!contains( key.toInt()))
            return 0;
        return ARRAY.at( key.toInt());
    }

    bool contains( const QVariant& key)
    {
        if (isValidKey(key) && key.toInt() < ARRAY.size())
            return true;
        return false;
    }

    bool remove( const QVariant& key)
    {
        if (!contains( key))
            return false;
        delete ARRAY.at( key.toInt());
        ARRAY.removeAt( key.toInt());
        return true;
    }

    bool removeWeak( const QVariant& key)
    {
        if (key.isNull())
            ARRAY[ key.toInt()] = new JsonValue();

        if (!contains( key))
            return false;
        ARRAY[ key.toInt()] = new JsonValue();
        return true;
    }

    int size()
    {
        return ARRAY.size();
    }
};

// ---------------------------------------------------------

class Cache
{
public:
    QList<JsonType*> CACHE;
    QVariantList CACHE_KEYS;

    JsonType* getCacheAt( int level)
    {
        return CACHE.at( level);
    }

    int getCacheLevel( const QVariantList& keys)
    {
        int i = 0;
        while (i < keys.size() && i < CACHE_KEYS.size() && keys.at(i) == CACHE_KEYS.at(i))
            ++i;

        return i - 1;                           // -1 means no cache. 0 means one level of cache.
    }

    void dropCacheAfterLevel( int level)
    {
        while (level < CACHE.size() - 1)
        {
            CACHE.removeLast();
            CACHE_KEYS.removeLast();
        }
    }

    void appendCache( const QVariant& key, JsonType* jsonType)
    {
        CACHE_KEYS.append( key);
        CACHE.append( jsonType);
    }
};

class KeysAndValues
{
public:
    QVariantList KEYS;
    QVariantList VALUES;
};

class Editor
{
private:
    JsonType* DATA = 0;
    Cache CACHE;

    JsonType* createJsonTypeForKey( const QVariant& key) // Will provide the correct JsonType* object.
    {
        switch (key.type())
        {
        case QVariant::String:
            return new JsonObject();
        case QVariant::Int:
            return new JsonArray();
        default:
            return 0;
        }
    }

    bool keyMatchesJsonType( const QVariant& key, JsonType* jsonType)
    {
        switch( key.type())
        {
        case QVariant::String:
            if (jsonType->hasType == Type::Object)
                return true;
            return false;
        case QVariant::Int:
            if (jsonType->hasType == Type::Array)
                return true;
            return false;
        default:
            return false;
        }
    }

    void appendPrepend( const QVariantList& keys, const QVariant& value, bool isAppend)
    {
        if (keys.isEmpty())
        {
            if (DATA->hasType != Type::Array)
            {
                delete DATA;
                DATA = new JsonArray();
            }
            if (isAppend)
                static_cast<JsonArray*>(DATA)->ARRAY.append( new JsonValue(value));
            else
                static_cast<JsonArray*>(DATA)->ARRAY.prepend( new JsonValue(value));
            return;
        }

        JsonType* parent = DATA;
        for (int i = 0; i < keys.size() - 1; ++i)
        {
            parent = parent->insertWeak( keys.at(i), createJsonTypeForKey( keys.at( i + 1)));
            if (parent == nullptr)                          // Abort if location doesn't exist.
                return;
        }

        JsonType* child = parent->value( keys.last());

        if ((child == nullptr) || (child->hasType != Type::Array))
        {
            parent = parent->insertStrong( keys.last(), new JsonArray());
            parent->setValue( 0, value);
        } else {
            if (isAppend)
                static_cast<JsonArray*>(child)->ARRAY.append( new JsonValue(value));
            else
                static_cast<JsonArray*>(child)->ARRAY.prepend( new JsonValue(value));
        }
    }

    JsonType* insert( const QVariantList& keys, JsonType* input)
    {
        if (keys.isEmpty())
            return DATA;

        JsonType* parent = 0;

        if (!keyMatchesJsonType( keys.first(), DATA))
        {
            delete DATA;
            DATA = createJsonTypeForKey( keys.first());
        }

        parent = DATA;
        JsonType* fresh_element = 0;

        for (int i = 0; i < keys.size() - 1; ++i)                       // All but the last key.
        {
            fresh_element = createJsonTypeForKey( keys.at( i + 1));     // This object could be deleted immediately below, which is a waste.
            parent = parent->insertWeak( keys.at(i), fresh_element);    // Reuses existing arrays and objects (deletes fresh_element if unused).

            if (parent == nullptr)                                      // Abort in case of failure.
                return 0;
        }
        parent->insertStrong( keys.last(), input);                      // Overwrites the last location.
        return parent;
    }

    bool copyData( JsonType* jsonFrom, Editor& jsonTo, QVariantList& keysTo)
    {
        if (jsonFrom == nullptr)
            return false;

        if (jsonFrom->hasType == Type::Value)
        {
            QVariant sourceValue = static_cast<JsonValue*>(jsonFrom)->VALUE;
            JsonType* jsonValue = new JsonValue( sourceValue);
            jsonTo.insert( keysTo, jsonValue);

            if (!keysTo.isEmpty())
                keysTo.removeLast();
        } else {
            for (QVariant key : jsonFrom->keys())
            {
                keysTo.append( key);
                copyData( jsonFrom->value( key), jsonTo, keysTo);
            }
            if (!keysTo.isEmpty())
                keysTo.removeLast();
        }
        return true;
    }

public:
    Editor()
    {
        DATA = new JsonObject();
    }

    ~Editor()
    {
        delete DATA;
    }

    typedef JsonWaxInternals::StringStyle StringStyle;

    void clear()
    {
        delete DATA;
        DATA = new JsonObject();
    }

    JsonType* getPointer( const QVariantList& keys)
    {
        JsonType* element = DATA;

        for (int i = 0; i < keys.size(); ++i)
        {
            if (element->contains( keys.at(i)))
                element = element->value( keys.at(i));
            else
                return nullptr;
        }
        return element;
    }

    void setValue( const QVariantList& keys, const QVariant& value)
    {
        insert( keys, new JsonValue(value));
    }

    QVariant value( const QVariantList& keys, const QVariant& defaultValue)
    {
        if (keys.isEmpty())
            return defaultValue;

        JsonType* element = getPointer( keys);

        if (element == nullptr || element->hasType != Type::Value)      // Return default value if something is wrong with the found one.
            return defaultValue;

        return static_cast<JsonValue*>(element)->VALUE;                 // Else cast to JsonValue and return its VALUE.
    }

    QString toString( StringStyle style, bool convertToCodePoints)
    {
        CONVERT_TO_CODE_POINTS = convertToCodePoints;
        return DATA->toString( style, 1);
    }

    QByteArray toByteArray( StringStyle style, bool convertToCodePoints)
    {
        CONVERT_TO_CODE_POINTS = convertToCodePoints;
        return DATA->toString( style, 1).toUtf8();
    }

    int size( const QVariantList& keys)
    {
        JsonType* element = getPointer( keys);

        if (element == nullptr)
            return -1;
        else
            return element->size();
    }

    void remove( const QVariantList& keys)
    {
        if (keys.isEmpty())
        {
            delete DATA;
            DATA = new JsonObject();
            return;
        }

        JsonType* element = getPointer( keys.mid(0, keys.size() - 1));  // Uses all keys except the last.

        if (element == nullptr)
            return;
        else
            element->remove( keys.last());
    }

    bool exists( const QVariantList& keys)
    {
        if (keys.isEmpty())                                             // The root object always exists.
            return true;

        JsonType* element = getPointer( keys.mid(0, keys.size() - 1));  // Uses all keys except the last.

        if (element == nullptr)
            return false;
        else
            return (element->contains( keys.last())) ? true : false;
    }

    QVariantList keys( const QVariantList& keys)
    {
        JsonType* element = getPointer( keys);

        if (element == nullptr)
            return QVariantList();
        else
            return element->keys();
    }

    int append( const QVariantList& keys, const QVariant& value)
    {
        appendPrepend( keys, value, true);
        return getPointer(keys)->size() - 1;
    }

    void prepend( const QVariantList& keys, const QVariant& value)
    {
        appendPrepend( keys, value, false);
    }

    void popFirst( const QVariantList& keys, int removeTimes)      // Removes first element of array.
    {
        JsonType* element = getPointer( keys);

        if (element != nullptr)
            if (element->hasType == Type::Array)
                for (int i = 0; i < removeTimes; ++i)
                    element->remove(0);
    }

    void popLast( const QVariantList& keys, int removeTimes)       // Removes last element of array.
    {
        JsonType* element = getPointer( keys);

        if (element != nullptr)
            if (element->hasType == Type::Array)
                for (int i = 0; i < removeTimes; ++i)
                    element->remove( element->size() - 1);
    }

    void copy( const QVariantList& keysFrom, JsonWaxInternals::Editor* editor, const QVariantList& keysTo) // Copy from this to a position in another Editor.
    {
        QVariantList tempKeysTo;
        Editor tempEditor;
        JsonType* jsonFrom = getPointer(keysFrom);

        if (jsonFrom == nullptr || jsonFrom->hasType == Type::Value)
            return;

        if (copyData( jsonFrom, tempEditor, tempKeysTo))                        // Copy into tempEditor
            tempEditor.move({}, editor, keysTo);                                // Move to destination (overwrite if keysTo already exists).
    }

    void move( const QVariantList& keysFrom, Editor* editorTo, const QVariantList& keysTo)
    {
        QVariantList keysFrom_short = keysFrom.mid( 0, keysFrom.length() - 1);  // Keys except the last.
        JsonType* parent = getPointer( keysFrom_short);
        JsonType* child = getPointer( keysFrom);

        if (child == nullptr || child->hasType == Type::Value)
            return;

        // Remove from source.
        if (keysFrom.isEmpty())
        {
            DATA = new JsonObject();                                            // Not deleting.
        } else {
            parent->removeWeak( keysFrom.last());                               // Remove from map, or replace with null in array
        }                                                                       // (the weak version doesn't 'delete' the data).

        // Put in destination.
        if (keysTo.isEmpty())
        {
            delete editorTo->DATA;
            editorTo->DATA = child;
        } else {
            editorTo->insert( keysTo, child);
        }
    }
};
}

#endif // JSONWAX_EDITOR_H
