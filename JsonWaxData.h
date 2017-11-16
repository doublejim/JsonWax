#ifndef JSONWAX_DATA_H
#define JSONWAX_DATA_H

/* Original author: Nikolai S | https://github.com/doublejim
 *
 * You may use this file under the terms of any of these licenses:
 * GNU General Public License version 2.0       https://www.gnu.org/licenses/gpl-2.0.html
 * GNU General Public License version 3         https://www.gnu.org/licenses/gpl-3.0.html
 */
#include <QByteArray>
#include <QString>
#include <QVariant>

#include <QDebug>

/* TODO:
 * - Problem: Make sure that documents are stored using UTF-8 codec, before loading them.
 */

/* There are 3 JSON data classes and there are Editor classes.
 * The JSON classes are: Object, Array and Value, which implement functions from "JsonType".
 * Objects and Arrays use polymorphism to contain any of the 3 types.
 *
 * The difference between insertWeak and insertStrong:
 * - insertWeak is for when we want to insert a key, but if there already
 *   exists a key of that same type, instead of overwriting, we just
 *   take where that jsonType leads to, instead of creating the key.
 * - insertStrong REQUIRES that we insert the jsonType, no matter if there
 *   already exists a jsonType of the same type.
 * - So, insertWeak is for inserting keys, and insertStrong is for inserting
 *   values.
 *
 * Destroy works like this:
 * - It starts in the root-container (from the top of the current container).
 * - If there's only one reference to the container, it will attempt to delete
 *   as many sub-containers as possible, and end with deleting itself.
 *   The method is recursive.
 */

namespace JsonWaxInternals {

// -------------------------------- STATIC THINGS --------------------------------
enum Type {Value, Object, Array, Null};
enum StringStyle {Compact, Readable};

struct StringInOut
{
    bool CONVERT_TO_CODE_POINTS = false;
    QString TO_STRING;
};
extern thread_local StringInOut STR_IN_OUT;

// ------------------------- JSON TYPES -------------------------

class JsonType
{
public:
    Type hasType = Type::Null;

    virtual ~JsonType(){}
    virtual bool contains( const QVariant& key) = 0;
    virtual void destroy() = 0;
    virtual JsonType* insertWeak( const QVariant& key, JsonType* fresh_element) = 0;
    virtual JsonType* insertStrong( const QVariant& key, JsonType* fresh_element) = 0;
    virtual QVariantList keys() = 0;
    virtual bool remove( const QVariant& key) = 0;
    virtual bool removeWeak( const QVariant& key) = 0;
    virtual void setValue( const QVariant& key, const QVariant& value) = 0;
    virtual int size() = 0;
    virtual void toString( StringStyle style, int indentation = 0) const = 0;
    virtual JsonType* value( const QVariant& key) = 0;
};

class ReferenceCounted
{
public:
    int REFERENCES = 1;
    void incRefCount();
    int decRefCount();
};

class JsonValue : public JsonType
{
public:
    JsonValue();
    JsonValue( const QVariant& value);
    void destroy();
    QVariant VALUE;
    void toString( StringStyle style, int indentation = 0) const;
    void setValue(const QVariant& key, const QVariant& value);
    JsonType* insertWeak( const QVariant& key, JsonType* fresh_element);
    JsonType* insertStrong( const QVariant& key, JsonType* fresh_element);
    QVariantList keys();
    JsonType* value( const QVariant& key);
    bool remove( const QVariant& key);
    bool removeWeak( const QVariant& key);
    bool contains( const QVariant& key);
    int size();
};

class JsonObject : public JsonType, public ReferenceCounted
{
private:
    JsonType* INSERTED_ELEMENT = 0;
    bool insertBase( const QVariant& key, JsonType* fresh_element);

public:
    QMap<QString, JsonType*> MAP;
    JsonObject();
    void destroy();
    QVariantList keys();
    void toString( StringStyle style, int indentation = 0) const;
    bool isValidKey( const QVariant& key);
    JsonType* insertWeak( const QVariant& key, JsonType* fresh_element);
    JsonType* insertStrong( const QVariant& key, JsonType* fresh_element);
    void setValue( const QVariant& key, const QVariant& value);
    JsonType* value( const QVariant& key);
    bool contains( const QVariant& key);
    bool remove( const QVariant& key);
    bool removeWeak( const QVariant& key);
    int size();
};

class JsonArray : public JsonType, public ReferenceCounted
{
private:
    JsonType* INSERTED_ELEMENT = 0;
    bool insertBase( const QVariant& key, JsonType* fresh_element);

public:
    QList<JsonType*> ARRAY;
    JsonArray();
    void destroy();
    bool isValidKey( const QVariant& key);
    void inflate( int elementCount);
    QVariantList keys();
    void toString( StringStyle style, int indentation = 0) const;
    JsonType* insertWeak( const QVariant& key, JsonType* fresh_element);
    JsonType* insertStrong( const QVariant& key, JsonType* fresh_element);
    void setValue( const QVariant& key, const QVariant& value);
    JsonType* value( const QVariant& key);
    bool contains( const QVariant& key);
    bool remove( const QVariant& key);
    bool removeWeak( const QVariant& key);
    int size();
};
}

#endif // JSONWAX_DATA_H
