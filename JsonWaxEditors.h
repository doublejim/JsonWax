#ifndef JSONWAXEDITORS_H
#define JSONWAXEDITORS_H

/* Original author: Nikolai S | https://github.com/doublejim
 *
 * You may use this file under the terms of any of these licenses:
 * GNU General Public License version 2.0       https://www.gnu.org/licenses/gpl-2.0.html
 * GNU General Public License version 3         https://www.gnu.org/licenses/gpl-3.0.html
 */

#include "JsonWaxData.h"

namespace JsonWaxInternals {
class MainEditor;
class ObjectEditor;
class ArrayEditor;

// ================ BASE EDITOR ================

class BaseEditor
{
protected:
    JsonType* ROOT = nullptr, *FOCUS = nullptr;
    bool base_exists( const QVariant& lastKey);
    JsonType* base_insert( const QVariant key, Type type);
    bool base_isArray();
    bool base_isObject();
    bool base_isValue();
    int base_size();
    QByteArray base_toByteArray( StringStyle style, bool& convertToCodePoints);
    QString base_toString( StringStyle style, bool& convertToCodePoints);
    Type base_type();
    QVariant base_value( const QVariant& defaultValue);
    ArrayEditor base_toArray();
    ObjectEditor base_toObject();
    MainEditor base_toWax();
};

// ================ INTERNAL WAX ARRAY ================

class ArrayEditor : protected BaseEditor
{
private:
    QList<JsonType*>* ARRAY = nullptr;

protected:
    MainEditor toMainEditor();
    MainEditor toMainEditor( const int& key);
    ArrayEditor toArrayEditor( const int& key);
    ObjectEditor toObjectEditor( const int& key);

public:
    ArrayEditor( JsonType* location);
    ~ArrayEditor();

    void append( const QVariant& value);
    QVariant at( const int& key);
    bool exists( const int& key);
    JsonType* getRoot();
    bool isArray( const int& key);
    bool isNull( const int& key);
    bool isNullValue( const int& key);
    bool isObject( const int& key);
    bool isValid();
    bool isValue( const int& key);
    void prepend( const QVariant& value);
    void remove( const int& key);
    void removeFirst( const int& removeTimes);
    void removeLast( const int& removeTimes);
    void setEmptyArray( const int& key);
    void setEmptyObject( const int& key);
    void setValue( const int& key, const QVariant& value);
    int size();
    int size( const int& key);
    QByteArray toByteArray( StringStyle style = Readable, bool convertToCodePoints = false);
    QByteArray toByteArray( const int& key, StringStyle style = Readable, bool convertToCodePoints = false);
    QString toString( StringStyle style = Readable, bool convertToCodePoints = false);
    QString toString( const int& key, StringStyle style = Readable, bool convertToCodePoints = false);
    Type type( const int& key);
    QVariant value( const int& key, const QVariant& defaultValue = QVariant());
};

// ================ INTERNAL WAX OBJECT ================

class ObjectEditor : protected BaseEditor
{
private:
    QMap<QString,JsonType*>* MAP = nullptr;

protected:
    MainEditor toMainEditor();
    MainEditor toMainEditor( const QString& key);
    ArrayEditor toArrayEditor( const QString& key);
    ObjectEditor toObjectEditor( const QString& key);

public:
    ObjectEditor( JsonType* location);
    ~ObjectEditor();

    bool exists( const QString& key);
    JsonType* getRoot();
    bool isArray( const QString& key);
    bool isNull( const QString& key);
    bool isNullValue( const QString& key);
    bool isObject( const QString& key);
    bool isValid();
    bool isValue( const QString& key);
    QList<QString> keys();
    void remove( const QString& key);
    void setEmptyArray( const QString& key);
    void setEmptyObject( const QString& key);
    void setValue( const QString& key, const QVariant& value);
    int size();
    int size( const QString& key);
    QByteArray toByteArray( StringStyle style = Readable, bool convertToCodePoints = false);
    QByteArray toByteArray( const QString& key, StringStyle style = Readable, bool convertToCodePoints = false);
    QString toString( StringStyle style = Readable, bool convertToCodePoints = false);
    QString toString( const QString& key, StringStyle style = Readable, bool convertToCodePoints = false);
    Type type( const QString& key);
    QVariant value( const QString key, const QVariant& defaultValue = QVariant());
};

/*
 * ================ INTERNAL WAX ================
 */

class MainEditor : protected BaseEditor
{
    friend class Parser;                                        // The Parser uses some of the Editor functions.
private:
    JsonType* createJsonTypeForKey( const QVariant& key);
    bool keyMatchesJsonType( const QVariant& key, JsonType* jsonType);
    void base_insert( const QVariantList& keys, JsonType* input);
    bool copyData( JsonType* jsonFrom, MainEditor& jsonTo, QVariantList& keysTo);
    void swapRoot(JsonType* root);

protected:
    MainEditor toMainEditor( const QVariantList& keys = {});

public:
    typedef JsonWaxInternals::StringStyle StringStyle;
    MainEditor();
    MainEditor( JsonType* root);
    MainEditor( const MainEditor& other);
    ~MainEditor();

    void clear();
    void copy( const QVariantList& keysFrom, MainEditor* editorTo, const QVariantList& keysTo);
    bool exists( const QVariantList& keys);
    void forgetRoot();
    JsonType* getPointer( const QVariantList& keys);
    JsonType* getRoot() const;
    bool isArray( const QVariantList& keys);
    bool isNull( const QVariantList& keys);
    bool isNullValue( const QVariantList& keys);
    bool isObject( const QVariantList& keys);
    bool isValue( const QVariantList& keys);
    QVariantList keys( const QVariantList& keys);
    void move( const QVariantList& keysFrom, MainEditor* editorTo, const QVariantList& keysTo);
    void remove( const QVariantList& keys);
    void setEmptyArray( const QVariantList& keys = {});
    void setEmptyObject( const QVariantList& keys = {});
    void setValue( const QVariantList& keys, const QVariant& value);
    int size( const QVariantList& keys = {});
    QByteArray toByteArray( StringStyle style = Readable, bool convertToCodePoints = false, const QVariantList& keys = {});
    QString toString( StringStyle style = Readable, bool convertToCodePoints = false, const QVariantList& keys = {});
    Type type( const QVariantList& keys);
    QVariant value( const QVariantList& keys, const QVariant& defaultValue = QVariant());
    ObjectEditor toObjectEditor( const QVariantList& keys = {});    // Not for users.
    ArrayEditor toArrayEditor( const QVariantList& keys = {});      // Not for users.
};
}

#endif // JSONWAXEDITORS_H
