#ifndef JSONWAX_H
#define JSONWAX_H

/* Original author: Nikolai S | https://github.com/doublejim
 *
 * You may use this file under the terms of any of these licenses:
 * GNU General Public License version 2.0       https://www.gnu.org/licenses/gpl-2.0.html
 * GNU General Public License version 3         https://www.gnu.org/licenses/gpl-3.0.html
 */

#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <QCoreApplication>
#include <QDir>
#include "JsonWaxEditors.h"
#include "JsonWaxParser.h"
#include "JsonWaxSerializer.h"

class JWObject;
class JWArray;

// ===============================================================================================

class JsonWax : public JsonWaxInternals::MainEditor
{
    friend class JWObject;                                                     // So that JsonObject / JsonArray can convert to JsonWax.
    friend class JWArray;
public:
    typedef JsonWaxInternals::StringStyle StringStyle;
    typedef JsonWaxInternals::Type Type;

    static const StringStyle Compact = JsonWaxInternals::StringStyle::Compact;
    static const StringStyle Readable = JsonWaxInternals::StringStyle::Readable;
    static const Type Array = JsonWaxInternals::Type::Array;
    static const Type Null = JsonWaxInternals::Type::Null;
    static const Type Object = JsonWaxInternals::Type::Object;
    static const Type Value = JsonWaxInternals::Type::Value;

private:
    QString PROGRAM_PATH, FILENAME;
    JsonWaxInternals::Parser PARSER;
    JsonWaxInternals::Serializer SERIALIZER;
    void increaseRefCountOfRoot();

public:
    JsonWax();
    JsonWax& operator= (JsonWax other);
    JsonWax(MainEditor other);

    void copy( const QVariantList& keysFrom, QVariantList keysTo);
    void copy( const QVariantList& keysFrom, JsonWax& jsonTo, const QVariantList& keysTo);

    template <class T>
    T deserializeBytes( const QVariantList& keys, const T defaultValue = T())
    {
        if (keys.isEmpty())                                                     // Can't deserialize from root, since it's not a value.
            return defaultValue;

        JsonWaxInternals::JsonType* element = MainEditor::getPointer( keys);

        if (element == nullptr || element->hasType != Type::Value)              // Return default value if the found JsonType is not of type Value.
            return defaultValue;

        QVariant value = static_cast<JsonWaxInternals::JsonValue*>(element)->VALUE;
        return SERIALIZER.deserializeBytes<T>( value.toString().toUtf8());
    }

    template <class T>
    void deserializeBytes( T& outputHere, const QVariantList& keys)
    {
        if (keys.isEmpty())                                                     // Can't deserialize from root, since it's not a value.
            return;

        JsonWaxInternals::JsonType* element = MainEditor::getPointer( keys);

        if (element == nullptr || element->hasType != Type::Value)
            return;

        QVariant value = static_cast<JsonWaxInternals::JsonValue*>(element)->VALUE;
        SERIALIZER.deserializeBytes<T>( value.toString().toUtf8(), outputHere);
    }

    template <class T>
    T deserializeJson( const QVariantList& keys, T defaultValue = T())
    {
        JsonWaxInternals::JsonType* element = MainEditor::getPointer( keys);

        if (element == nullptr)
            return defaultValue;

        T value;
        SERIALIZER.deserializeJson<T>( this, keys, value);
        return value;                                                           // A fresh value with the deserialized data inserted.
    }

    template <class T>
    void deserializeJson( T& outputHere, const QVariantList& keys)
    {
        JsonWaxInternals::JsonType* element = MainEditor::getPointer( keys);

        if (element == nullptr)
            return;

        SERIALIZER.deserializeJson<T>( this, keys, outputHere);
    }

    int errorCode();
    QString errorMsg();
    int errorPos();
    bool fromByteArray( const QByteArray& bytes);
    bool loadFile( const QString& fileName);
    void move( const QVariantList& keysFrom, const QVariantList& keysTo);
    void move( const QVariantList& keysFrom, JsonWax& jsonTo, const QVariantList& keysTo);
    bool save( StringStyle style = Readable, bool convertToCodePoints = false);
    bool saveAs( const QString& fileName, StringStyle style = Readable, bool convertToCodePoints = false, bool overwriteAllowed = true);

    template <class T>
    void serializeToBytes( const QVariantList& keys, const T& object)
    {
        MainEditor::setValue( keys, SERIALIZER.serializeToBytes<T>(object));
    }

    template <class T>
    void serializeToJson( const QVariantList& keys, const T& object)
    {
        MainEditor* serializedJson = SERIALIZER.serializeToJson<T>(object);                // Serialize QObject or other data type as a JSON-document
        serializedJson->move({0}, this, keys);                                              // with the data located at the first array position.
    }                                                                                       // Move result to this editor.

    void setNull( const QVariantList& keys);
    JWArray toArray( const QVariantList& keys = {});
    JWObject toObject( const QVariantList& keys = {});
    JsonWax toJsonWax( const QVariantList& keys = {});

private:
    using JsonWaxInternals::MainEditor::forgetRoot;                                        // Force it to be private.
};

// ==================================== WAX ARRAY =============================================

class JWArray : public JsonWaxInternals::ArrayEditor
{
public:
    JWArray(ArrayEditor* other);

    JsonWax toJsonWax();
    JsonWax toJsonWax( const int& key);
    JWObject toObject(const int &key);
    JWArray toArray(const int &key);
private:
    using JsonWaxInternals::ArrayEditor::getRoot;                                  // Force it to be private.
};

// ==================================== WAX OBJECT =============================================

class JWObject : public JsonWaxInternals::ObjectEditor
{
public:
    JWObject(ObjectEditor *other);

    JsonWax toJsonWax();
    JsonWax toJsonWax( const QString& key);
    JWObject toObject( const QString& key);
    JWArray toArray( const QString& key);

private:
    using JsonWaxInternals::ObjectEditor::getRoot;                                 // Force it to be private.
};

#endif // JSONWAX_H

