#include "JsonWax.h"

void JsonWax::increaseRefCountOfRoot()
{
    switch(ROOT->hasType)
    {
    case JsonWaxInternals::Array:
        static_cast<JsonWaxInternals::JsonArray*>(ROOT)->incRefCount();
        break;
    case JsonWaxInternals::Object:
        static_cast<JsonWaxInternals::JsonObject*>(ROOT)->incRefCount();
        break;
    default: break; // This can't happen. Root is either of the above.
    }
}

JsonWax::JsonWax()
{
    PROGRAM_PATH = qApp->applicationDirPath();
}

JsonWax &JsonWax::operator= (JsonWax other)
{
    if (this != &other)
    {
        ROOT->destroy();                // "this" always has a ROOT, so we need to destroy the data.
        ROOT = other.getRoot();         // Replace this ROOT with other ROOT.
        increaseRefCountOfRoot();
    }

    return *this;
}

JsonWax::JsonWax(JsonWaxInternals::MainEditor other)
{
    if (this != &other)
    {
        ROOT->destroy();                // this always has a ROOT, so we need to destroy it.
        ROOT = other.getRoot();         // Replace this ROOT with other ROOT.
        increaseRefCountOfRoot();
    }
}

void JsonWax::copy(const QVariantList &keysFrom, QVariantList keysTo)
{
    MainEditor::copy( keysFrom, this, keysTo);
}

void JsonWax::copy(const QVariantList &keysFrom, JsonWax &jsonTo, const QVariantList &keysTo)
{
    MainEditor::copy( keysFrom, &jsonTo, keysTo);
}

int JsonWax::errorCode()
{
    return PARSER.LAST_ERROR;
}

QString JsonWax::errorMsg()
{
    return PARSER.errorToString();
}

int JsonWax::errorPos()                                                              // May not be 100% accurate.
{
    return PARSER.LAST_ERROR_POS;
}

bool JsonWax::fromByteArray(const QByteArray &bytes)
{
    PARSER.EDITOR = this;
    return PARSER.parseAndLoad( bytes);
}

bool JsonWax::loadFile(const QString &fileName)
{
    FILENAME = fileName;
    QDir dir (fileName);
    QFile qfile;
    if (dir.isRelative())
        qfile.setFileName( PROGRAM_PATH + '/' + fileName);
    else
        qfile.setFileName( fileName);

    if (!qfile.exists())
        return false;

    QTextStream in (&qfile);
    in.setCodec("UTF-8");
    /*
            TODO: Determine correct codec. UTF-8 setting invalidates some ansi characters like "æ,ø,å".
            So make sure the read file is UTF-8 encoded, then everything will work perfectly.
        */
    qfile.open(QIODevice::ReadOnly);
    return fromByteArray( in.readAll().toUtf8());
}

void JsonWax::move(const QVariantList &keysFrom, const QVariantList &keysTo)
{
    MainEditor::move( keysFrom, this, keysTo);
}

void JsonWax::move(const QVariantList &keysFrom, JsonWax &jsonTo, const QVariantList &keysTo)
{
    MainEditor::move( keysFrom, &jsonTo, keysTo);
}

bool JsonWax::save(JsonWax::StringStyle style, bool convertToCodePoints)
{
    if (FILENAME.isEmpty())
    {
        qWarning("JsonWax-save error: use saveAs() if you haven't loaded a .json file. This document wasn't saved.");
        return false;
    } else {
        return saveAs( FILENAME, style, convertToCodePoints, true);
    }
}

bool JsonWax::saveAs(const QString &fileName, JsonWax::StringStyle style, bool convertToCodePoints, bool overwriteAllowed)
{
    QDir dir (fileName);
    QFile qfile;
    if (dir.isRelative())
        qfile.setFileName( PROGRAM_PATH + '/' + fileName);
    else
        qfile.setFileName( fileName);

    if (qfile.exists() && !overwriteAllowed)
        return false;

    qfile.open( QIODevice::WriteOnly | QIODevice::Text);
    QByteArray bytes = MainEditor::toByteArray( style, convertToCodePoints, {});
    qint64 bytesWritten = qfile.write( bytes);
    qfile.close();
    return (bytesWritten == bytes.size());
}

void JsonWax::setNull(const QVariantList &keys)
{
    setValue( keys, QVariant());
}

JsonWax JsonWax::toJsonWax(const QVariantList &keys)
{
    JsonWax result (toMainEditor( keys));
    result.PROGRAM_PATH = PROGRAM_PATH;
    return result;
}

JWArray::JWArray(ArrayEditor *other)
    : ArrayEditor( other->getRoot())
{}

JsonWax JWArray::toJsonWax()
{
    FOCUS = ROOT;
    JsonWax::MainEditor internalwax = base_toWax();
    return JsonWax(internalwax);
}

JsonWax JWArray::toJsonWax(const int &key)
{
    JsonWax result;
    result.ROOT = toMainEditor( key).getRoot();
    return result;
}

JWObject::JWObject(ObjectEditor* other)
    : ObjectEditor( other->getRoot())
{}

JsonWax JWObject::toJsonWax()
{
    JsonWax result( ObjectEditor::toMainEditor());                // Set editor to Editor from toWax conversion.
    return result;
}

JsonWax JWObject::toJsonWax(const QString &key)
{
    JsonWax result( ObjectEditor::toMainEditor( key));                              // Set editor to Editor from toWax conversion.
    return result;
}

JWObject JWObject::toObject(const QString &key)
{
    ObjectEditor editor = ObjectEditor::toObjectEditor(key);
    return JWObject( &editor);
}

JWArray JWObject::toArray(const QString &key)
{
    JWArray::ArrayEditor editor = ObjectEditor::toArrayEditor( key);
    return JWArray( &editor);
}

JWArray JsonWax::toArray(const QVariantList &keys)
{
    JWArray::ArrayEditor editor = MainEditor::toArrayEditor(keys);
    return JWArray( &editor);
}

JWObject JsonWax::toObject(const QVariantList &keys)
{
    JWObject::ObjectEditor editor = MainEditor::toObjectEditor( keys);
    return JWObject( &editor);
}

JWObject JWArray::toObject(const int &key)
{
    JWObject::ObjectEditor editor = ArrayEditor::toObjectEditor( key);
    return JWObject( &editor);
}

JWArray JWArray::toArray(const int &key)
{
    JWArray::ArrayEditor editor = ArrayEditor::toArrayEditor( key);
    return JWArray( &editor);
}
