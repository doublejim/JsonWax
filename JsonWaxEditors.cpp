#include "JsonWaxEditors.h"

// SHOULD WRITE NAMESPACE  JsonWaxInternals HERE.
namespace JsonWaxInternals {

bool BaseEditor::base_exists(const QVariant &lastKey)
{
    if (FOCUS == nullptr)
        return false;
    return (FOCUS->contains( lastKey)) ? true : false;
}

JsonType *BaseEditor::base_insert(const QVariant key, Type type)
{
    switch( type)
    {
    case Type::Array:
        return FOCUS->insertStrong( key, new JsonArray);
    case Type::Object:
        return FOCUS->insertStrong( key, new JsonObject);
    default:                                                // Type::Value:
        return FOCUS->insertStrong( key, new JsonValue);
    }
}

bool BaseEditor::base_isArray()
{
    if (FOCUS != nullptr && FOCUS->hasType == Type::Array)
        return true;
    return false;
}

bool BaseEditor::base_isObject()
{
    if (FOCUS != nullptr && FOCUS->hasType == Type::Object)
        return true;
    return false;
}

bool BaseEditor::base_isValue()
{
    if (FOCUS != nullptr && FOCUS->hasType == Type::Value)
        return true;
    return false;
}

int BaseEditor::base_size()
{
    if (FOCUS == nullptr)
        return -1;

    return FOCUS->size();
}

QByteArray BaseEditor::base_toByteArray(StringStyle style, bool &convertToCodePoints)
{
    STR_IN_OUT.CONVERT_TO_CODE_POINTS = convertToCodePoints;

    if (FOCUS == nullptr)
        return QByteArray();

    STR_IN_OUT.TO_STRING.clear();
    FOCUS->toString( style, 1);
    return STR_IN_OUT.TO_STRING.toUtf8();
}

QString BaseEditor::base_toString(StringStyle style, bool &convertToCodePoints)
{
    STR_IN_OUT.CONVERT_TO_CODE_POINTS = convertToCodePoints;

    if (FOCUS == nullptr)
        return QString();

    STR_IN_OUT.TO_STRING.clear();
    FOCUS->toString( style, 1);
    return STR_IN_OUT.TO_STRING;
}

Type BaseEditor::base_type()
{
    if (FOCUS == nullptr)
        return Type::Null;

    return FOCUS->hasType;
}

QVariant BaseEditor::base_value(const QVariant &defaultValue)
{
    if (FOCUS == nullptr || FOCUS->hasType != Type::Value)          // Return default value if the found JsonType is not of type Value.
        return defaultValue;                                        // (Root can't have a value, since it's either an Object or Array.)

    return static_cast<JsonValue*>(FOCUS)->VALUE;                   // Else cast to JsonValue and return its VALUE.
}

MainEditor ArrayEditor::toMainEditor()
{
    FOCUS = ROOT;
    return BaseEditor::base_toWax();
}

MainEditor ArrayEditor::toMainEditor(const int &key)
{
    FOCUS = ARRAY->value( key, nullptr);
    return BaseEditor::base_toWax();
}

ArrayEditor::ArrayEditor(JsonType *location)
{
    if (location->hasType == Type::Array)
    {
        ROOT = location;                                                 // Pointer to the JsonType containing the ARRAY.
        JsonArray* array = static_cast<JsonArray*>(ROOT);
        ARRAY = &array->ARRAY;                                          // This link will continuously work, since the references are counted.
        array->incRefCount();                                           // Can only be done with JsonArray / JsonObject.
    } else {
        ROOT = new JsonArray;
    }
}

ArrayEditor::~ArrayEditor()
{
    ROOT->destroy();
}

void ArrayEditor::append(const QVariant &value)
{
    ARRAY->append( new JsonValue(value));
}

QVariant ArrayEditor::at(const int &key)
{
    return static_cast<JsonValue*>(ARRAY->at(key))->VALUE;
}

bool ArrayEditor::exists(const int &key)
{
    if (key >= 0 && key < ARRAY->size())
        return true;
    return false;
}

JsonType *ArrayEditor::getRoot()             // Should be hidden from users.
{
    return ROOT;
}

bool ArrayEditor::isArray(const int &key)
{
    FOCUS = ARRAY->value( key, nullptr);
    return BaseEditor::base_isArray();
}

bool ArrayEditor::isNull(const int &key)
{
    return value( key).isNull();
}

bool ArrayEditor::isNullValue(const int &key)
{
    if (isValue( key) && value( key).isNull())
        return true;
    return false;
}

bool ArrayEditor::isObject(const int &key)
{
    FOCUS = ARRAY->value( key, nullptr);
    return BaseEditor::base_isObject();
}

bool ArrayEditor::isValid()  // May not be necessary anymore.
{
    if (ROOT == nullptr)
        return false;
    return true;
}

bool ArrayEditor::isValue(const int &key)
{
    FOCUS = ARRAY->value( key, nullptr);
    return BaseEditor::base_isValue();
}

void ArrayEditor::prepend(const QVariant &value)
{
    ARRAY->prepend( new JsonValue(value));
}

void ArrayEditor::remove(const int &key)
{
    ROOT->remove( key);
}

void ArrayEditor::removeFirst(const int &removeTimes)
{
    for (int i = 0; i < removeTimes; ++i)
        ARRAY->removeFirst();
}

void ArrayEditor::removeLast(const int &removeTimes)
{
    for (int i = 0; i < removeTimes; ++i)
        ARRAY->removeLast();
}

void ArrayEditor::setEmptyArray(const int &key)
{
    FOCUS = ROOT;
    BaseEditor::base_insert( key, Type::Array);
}

void ArrayEditor::setEmptyObject(const int &key)
{
    FOCUS = ROOT;
    BaseEditor::base_insert( key, Type::Object);
}

void ArrayEditor::setValue(const int &key, const QVariant &value)
{
    ROOT->setValue( key, value);
}

int ArrayEditor::size()
{
    return ARRAY->size();
}

int ArrayEditor::size(const int &key)
{
    FOCUS = ARRAY->value( key, nullptr);
    return BaseEditor::base_size();
}

ArrayEditor ArrayEditor::toArrayEditor(const int &key)
{
    FOCUS = ARRAY->value( key, nullptr);

    if (FOCUS == nullptr || FOCUS->hasType != Type::Array)
    {
        FOCUS = ROOT;
        FOCUS = BaseEditor::base_insert( key, Type::Array);
    }

    return BaseEditor::base_toArray();
}

QByteArray ArrayEditor::toByteArray(StringStyle style, bool convertToCodePoints)
{
    FOCUS = ROOT;
    return BaseEditor::base_toByteArray( style, convertToCodePoints);
}

QByteArray ArrayEditor::toByteArray(const int &key, StringStyle style, bool convertToCodePoints)
{
    FOCUS = ARRAY->value( key, nullptr);
    return BaseEditor::base_toByteArray( style, convertToCodePoints);
}

QString ArrayEditor::toString(StringStyle style, bool convertToCodePoints)
{
    FOCUS = ROOT;
    return BaseEditor::base_toString( style, convertToCodePoints);
}

QString ArrayEditor::toString(const int &key, StringStyle style, bool convertToCodePoints)
{
    FOCUS = ARRAY->value( key, nullptr);
    return BaseEditor::base_toString( style, convertToCodePoints);
}

Type ArrayEditor::type(const int &key)
{
    FOCUS = ARRAY->value( key, nullptr);
    return BaseEditor::base_type();
}

QVariant ArrayEditor::value(const int &key, const QVariant &defaultValue)
{
    FOCUS = ARRAY->value( key, nullptr);
    return BaseEditor::base_value( defaultValue);
}

MainEditor ObjectEditor::toMainEditor()
{
    //qDebug() << "internalwax-object to wax.";
    FOCUS = ROOT;
    return BaseEditor::base_toWax();
}

MainEditor ObjectEditor::toMainEditor(const QString &key)
{
    FOCUS = MAP->value( key, nullptr);
    return BaseEditor::base_toWax();
}

ObjectEditor::ObjectEditor(JsonType *location)
{
    if (location->hasType == Type::Object)
    {
        ROOT = location;                                                 // Pointer to the JsonType containing the ARRAY.
        JsonObject* object = static_cast<JsonObject*>(ROOT);
        MAP = &object->MAP;
        object->incRefCount();
    } else {
        ROOT = new JsonObject;
    }
}

ObjectEditor::~ObjectEditor()
{
    ROOT->destroy();
}

bool ObjectEditor::exists(const QString &key)
{
    return MAP->contains( key);
}

JsonType *ObjectEditor::getRoot()         // Should be hidden from users.
{
    return ROOT;
}

bool ObjectEditor::isArray(const QString &key)
{
    FOCUS = MAP->value( key, nullptr);
    return BaseEditor::base_isArray();
}

bool ObjectEditor::isNull(const QString &key)
{
    return value( key).isNull();
}

bool ObjectEditor::isNullValue(const QString &key)
{
    if (isValue( key) && value( key).isNull())
        return true;
    return false;
}

bool ObjectEditor::isObject(const QString &key)
{
    FOCUS = MAP->value( key, nullptr);
    return BaseEditor::base_isObject();
}

bool ObjectEditor::isValid() // May not be necessary anymore.
{
    if (ROOT == nullptr)
        return false;
    return true;
}

bool ObjectEditor::isValue(const QString &key)
{
    FOCUS = MAP->value( key, nullptr);
    return BaseEditor::base_isValue();
}

QList<QString> ObjectEditor::keys()
{
    return MAP->keys();
}

void ObjectEditor::remove(const QString &key)
{
    ROOT->remove( key);
}

void ObjectEditor::setEmptyArray(const QString &key)
{
    FOCUS = ROOT;
    BaseEditor::base_insert( key, Type::Array);
}

void ObjectEditor::setEmptyObject(const QString &key)
{
    FOCUS = ROOT;
    BaseEditor::base_insert( key, Type::Object);
}

void ObjectEditor::setValue(const QString &key, const QVariant &value)
{
    ROOT->setValue( key, value);
}

int ObjectEditor::size()
{
    return MAP->size();
}

int ObjectEditor::size(const QString &key)
{
    FOCUS = MAP->value( key, nullptr);
    return BaseEditor::base_size();
}

QByteArray ObjectEditor::toByteArray(StringStyle style, bool convertToCodePoints)
{
    FOCUS = ROOT;
    return BaseEditor::base_toByteArray( style, convertToCodePoints);
}

QByteArray ObjectEditor::toByteArray(const QString &key, StringStyle style, bool convertToCodePoints)
{
    FOCUS = MAP->value( key, nullptr);
    return BaseEditor::base_toByteArray( style, convertToCodePoints);
}

QString ObjectEditor::toString(StringStyle style, bool convertToCodePoints)
{
    FOCUS = ROOT;
    return BaseEditor::base_toString( style, convertToCodePoints);
}

QString ObjectEditor::toString(const QString &key, StringStyle style, bool convertToCodePoints)
{
    FOCUS = MAP->value( key, nullptr);
    return BaseEditor::base_toString( style, convertToCodePoints);
}

ArrayEditor ObjectEditor::toArrayEditor(const QString &key)
{
    FOCUS = MAP->value( key, nullptr);
    if (FOCUS == nullptr || FOCUS->hasType != Type::Array)
    {
        FOCUS = ROOT;
        FOCUS = BaseEditor::base_insert( key, Type::Array);
    }
    return BaseEditor::base_toArray();
}

ObjectEditor ObjectEditor::toObjectEditor(const QString &key)
{
    FOCUS = MAP->value( key, nullptr);
    if (FOCUS == nullptr || FOCUS->hasType != Type::Object)
    {
        FOCUS = ROOT;
        FOCUS = BaseEditor::base_insert( key, Type::Object);
    }
    return BaseEditor::base_toObject();          // increases reference count.
}

Type ObjectEditor::type(const QString &key)
{
    FOCUS = MAP->value( key, nullptr);
    return BaseEditor::base_type();
}

QVariant ObjectEditor::value(const QString key, const QVariant &defaultValue)
{
    FOCUS = MAP->value( key, nullptr);
    return BaseEditor::base_value( defaultValue);
}

JsonType *MainEditor::createJsonTypeForKey(const QVariant &key)        // Will provide the correct JsonType* object, if you have the key.
{
    switch (key.type())
    {
    case QVariant::String:
        return new JsonObject();
    case QVariant::Int:
        return new JsonArray();
    default:
        return nullptr;
    }
}

bool MainEditor::keyMatchesJsonType(const QVariant &key, JsonType *jsonType)
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

void MainEditor::base_insert(const QVariantList &keys, JsonType *input)                    // This was the most difficult-to-create function.
{
    if (keys.isEmpty())
    {
        if (input->hasType == Type::Value)                                      // Root can't be set to a value. Nothing should happen.
        {
            qWarning("JsonWax-insert error: you can't save a value to root.");
            delete input;
            return;
        }
        swapRoot( input);
        return;
    }

    if (!keyMatchesJsonType( keys.first(), ROOT))                               // The root element is of a wrong type.
    {
        swapRoot( createJsonTypeForKey( keys.first()));
    }

    JsonType* parent = ROOT;
    JsonType* fresh_element = nullptr;

    for (int i = 0; i < keys.size() - 1; ++i)                                   // All but the last key.
    {
        fresh_element = createJsonTypeForKey( keys.at( i + 1));                 // This object could be deleted immediately below, which is a waste.
        parent = parent->insertWeak( keys.at(i), fresh_element);                // Reuses existing arrays and objects (deletes fresh_element if unused).

        if (parent == nullptr)                                                  // Abort in case of failure -- This really can't happen if the jsontype
        {
            qWarning("JsonWax-insert error: invalid key.");
            return;                                                             // was created specifically for the key. Can it?
        }
    }
    parent->insertStrong( keys.last(), input);                                  // Overwrites the last location.
}

bool MainEditor::copyData( JsonType *jsonFrom, MainEditor &jsonTo, QVariantList &keysTo)
{
    if (jsonFrom == nullptr)
        return false;

    if (jsonFrom->hasType == Type::Value)                               // If it's a Value.
    {
        QVariant sourceValue = static_cast<JsonValue*>(jsonFrom)->VALUE;
        JsonType* jsonValue = new JsonValue( sourceValue);
        jsonTo.base_insert( keysTo, jsonValue);

    } else if (jsonFrom->size() == 0) {                                 // If it's not a Value, it could be an empty Array or Object.
        switch( jsonFrom->hasType)
        {
        case Type::Object: jsonTo.setEmptyObject( keysTo);  break;
        case Type::Array:  jsonTo.setEmptyArray( keysTo);   break;
        default: break;     // Can't happen. We've determined that it's not a Value.
        }
    } else {                                                            // Else it's a non-empty Array or Object.
        for (const QVariant& key : jsonFrom->keys())                    // Go through each of its children recursively.
        {
            keysTo.append( key);
            copyData( jsonFrom->value( key), jsonTo, keysTo);
        }
    }
    if (!keysTo.isEmpty())
        keysTo.removeLast();

    return true;
}

void MainEditor::swapRoot(JsonType *root)      // Delete old root, and replace it with root.
{
    ROOT->destroy();
    ROOT = root;
}

/*void InternalWax::setRoot(JsonType *root)       // Without deleting the old one.
{
    ROOT = root;
}*/

/*void InternalWax::deleteRoot()
{
    //if (ROOT != nullptr)
    ROOT->destroy();            // This deletes everything that isn't referenced any more.
}*/

MainEditor::MainEditor()
{
    //qDebug() << "# internal wax construct";
    ROOT = new JsonObject;
}

MainEditor::MainEditor(JsonType *root)
{
    //qDebug() << "# internal wax construct";
    ROOT = root;

    if (ROOT->hasType == Type::Array)
    {
        static_cast<JsonArray*>(ROOT)->incRefCount();
    } else if (ROOT->hasType == Type::Object)
    {
        static_cast<JsonObject*>(ROOT)->incRefCount();
    }
}

MainEditor::MainEditor(const MainEditor &other)
{
   // qDebug() << "# internal wax construct";
    ROOT = other.ROOT;

    if (ROOT->hasType == Type::Array)
    {
        static_cast<JsonArray*>(ROOT)->incRefCount();
    } else if (ROOT->hasType == Type::Object)
    {
        static_cast<JsonObject*>(ROOT)->incRefCount();
    }
}

MainEditor::~MainEditor()
{
    //qDebug() << "# internalwax destruct";
    ROOT->destroy();
}

void MainEditor::clear()
{
    ROOT->destroy();
    ROOT = new JsonObject;
}

void MainEditor::copy(const QVariantList &keysFrom, MainEditor *editorTo, const QVariantList &keysTo) // Copy from this to a position in another Editor.
{
    QVariantList tempKeysTo;
    MainEditor tempEditor;
    JsonType* jsonFrom = getPointer( keysFrom);

    if (jsonFrom == nullptr || (jsonFrom->hasType == Type::Value && keysTo.isEmpty()))      // This is because you can't copy a Value to root.
        return;

    if (copyData( jsonFrom, tempEditor, tempKeysTo))                                        // Copy into tempEditor
        tempEditor.move({}, editorTo, keysTo);                                              // Move to destination (overwrite if keysTo already exists).
}

bool MainEditor::exists(const QVariantList &keys)
{
    if (keys.isEmpty())                                                                     // The root object always exists.
        return true;

    FOCUS = getPointer( keys.mid(0, keys.size() - 1));                                      // Uses all keys except the last.

    return BaseEditor::base_exists( keys.last());
}

void MainEditor::forgetRoot()
{
    ROOT = 0;
}

JsonType *MainEditor::getPointer(const QVariantList &keys)
{
    JsonType* element = ROOT;                                                               // Sets the starting point to root.

    for (int i = 0; i < keys.size(); ++i)
    {
        element = element->value( keys.at(i));

        if (element == nullptr)
            break;
    }
    return element;
}

JsonType *MainEditor::getRoot() const         // Should be hidden from users.
{
    return ROOT;
}

bool MainEditor::isArray(const QVariantList &keys)
{
    FOCUS = getPointer( keys);
    return BaseEditor::base_isArray();
}

bool MainEditor::isNull(const QVariantList &keys)
{
    return value( keys, QVariant()).isNull();
}

bool MainEditor::isNullValue(const QVariantList &keys)
{
    if (isValue( keys) && value( keys, QVariant()).isNull())
        return true;
    return false;
}

bool MainEditor::isObject(const QVariantList &keys)
{
    FOCUS = getPointer( keys);
    return BaseEditor::base_isObject();
}

bool MainEditor::isValue(const QVariantList &keys)
{
    FOCUS = getPointer( keys);
    return BaseEditor::base_isValue();
}

QVariantList MainEditor::keys(const QVariantList &keys)
{
    JsonType* element = getPointer( keys);

    if (element == nullptr)
        return QVariantList();
    return element->keys();
}

void MainEditor::move(const QVariantList &keysFrom, MainEditor *editorTo, const QVariantList &keysTo)
{
    QVariantList keysFrom_short = keysFrom.mid( 0, keysFrom.length() - 1);  // Keys except the last.
    JsonType* parent = getPointer( keysFrom_short);
    JsonType* child = getPointer( keysFrom);

    if (child == nullptr)
        return;

    if (child->hasType == Type::Value && keysTo.isEmpty())                  // A value can't be set to root. Abort and quit.
        return;

    // Remove from source.
    if (keysFrom.isEmpty())
        ROOT = new JsonObject;                                              // Not deleting, just creating new JsonObject.
    else
        parent->removeWeak( keysFrom.last());                               // Remove from map, or replace with null in array
    // (the weak version doesn't 'delete' the data).
    // Put in destination.
    if (keysTo.isEmpty())
        editorTo->swapRoot( child);
    else
        editorTo->base_insert( keysTo, child);
}

void MainEditor::remove(const QVariantList &keys)
{
    if (keys.isEmpty())
    {
        swapRoot( new JsonObject);
        return;
    }

    FOCUS = getPointer( keys.mid(0, keys.size() - 1));          // Uses all keys except the last.

    if (FOCUS == nullptr)
        return;

    FOCUS->remove( keys.last());
}

void MainEditor::setEmptyArray(const QVariantList &keys)
{
    base_insert( keys, new JsonArray);
}

void MainEditor::setEmptyObject(const QVariantList &keys)
{
    base_insert( keys, new JsonObject);
}

void MainEditor::setValue(const QVariantList &keys, const QVariant &value)
{
    base_insert( keys, new JsonValue(value));
}

int MainEditor::size(const QVariantList &keys)
{
    FOCUS = getPointer( keys);
    return BaseEditor::base_size();
}

ArrayEditor MainEditor::toArrayEditor(const QVariantList &keys)
{
    FOCUS = getPointer( keys);
    if (FOCUS == nullptr || FOCUS->hasType != Type::Array)
    {
        setEmptyArray( keys);
        FOCUS = getPointer( keys);
    }
    return BaseEditor::base_toArray();
}

QByteArray MainEditor::toByteArray(MainEditor::StringStyle style, bool convertToCodePoints, const QVariantList &keys)
{
    FOCUS = getPointer( keys);
    return BaseEditor::base_toByteArray( style, convertToCodePoints);
}

ObjectEditor MainEditor::toObjectEditor(const QVariantList &keys)
{
    FOCUS = getPointer( keys);

    if (FOCUS == nullptr || FOCUS->hasType != Type::Object)
    {
        setEmptyObject( keys);
        FOCUS = getPointer( keys);
    }
    return BaseEditor::base_toObject();          // increases reference count.
}

QString MainEditor::toString(MainEditor::StringStyle style, bool convertToCodePoints, const QVariantList &keys)
{
    FOCUS = getPointer( keys);
    return BaseEditor::base_toString( style, convertToCodePoints);
}

MainEditor MainEditor::toMainEditor(const QVariantList &keys)
{
    FOCUS = getPointer( keys);
    return BaseEditor::base_toWax();
}

Type MainEditor::type(const QVariantList &keys)
{
    FOCUS = getPointer( keys);
    return BaseEditor::base_type();
}

QVariant MainEditor::value(const QVariantList &keys, const QVariant &defaultValue)
{
    FOCUS = getPointer( keys);
    return BaseEditor::base_value( defaultValue);
}

ArrayEditor BaseEditor::base_toArray()
{
    return ArrayEditor( FOCUS);                // increases reference count.
}

ObjectEditor BaseEditor::base_toObject()
{
    return ObjectEditor( FOCUS);           // increases reference count.
}

MainEditor BaseEditor::base_toWax()
{
    if (FOCUS == nullptr)
    {
        qWarning("JsonWax-toWax error: location didn't exist.");
        return MainEditor();             // Return 'unlinked' Editor.
    }
    if (FOCUS->hasType == Type::Value)
    {
        qWarning("JsonWax-toWax error: can't make a new JsonWax-object with a value as root.");
        return MainEditor();         // Return 'unlinked' Editor.
    }
    return MainEditor( FOCUS);         // Increases reference count.
}

ObjectEditor ArrayEditor::toObjectEditor(const int &key)
{
    FOCUS = ARRAY->value( key, nullptr);

    if (FOCUS == nullptr || FOCUS->hasType != Type::Object)
    {
        FOCUS = ROOT;
        FOCUS = BaseEditor::base_insert( key, Type::Object);
    }
    return BaseEditor::base_toObject();
}

//int ReferenceCounted::decObjectRefCount(JsonType *container)
//{
    /*JsonObject* object = static_cast<JsonObject*>(container);


    qDebug() << "ref count object:" << container << object->REFERENCES;

    return object->REFERENCES;*/
//}

//int ReferenceCounted::decArrayRefCount(JsonType *container)
//{
    /*JsonArray* array = static_cast<JsonArray*>(container);
    array->REFERENCES -= 1;

    qDebug() << "ref count array:" << container << array->REFERENCES;

    return array->REFERENCES;*/
//}
}
