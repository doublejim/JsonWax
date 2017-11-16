#ifndef JSONWAX_PARSER_H
#define JSONWAX_PARSER_H

/* Original author: Nikolai S | https://github.com/doublejim
 *
 * You may use this file under the terms of any of these licenses:
 * GNU General Public License version 2.0       https://www.gnu.org/licenses/gpl-2.0.html
 * GNU General Public License version 3         https://www.gnu.org/licenses/gpl-3.0.html
 */

#include "JsonWaxEditors.h"
#include <QByteArray>
#include <QVariantList>
#include <QDebug>

namespace JsonWaxInternals {

class EscapedCharacter
{
public:
    enum Type {CODE_POINT, ESCAPED_CHARACTER};
    Type TYPE = ESCAPED_CHARACTER;
    int POS = 0;
    EscapedCharacter( Type type, int position);
};

class Parser
{
public:
    enum ErrorCode {OK, UNEXPECTED_CHARACTER, EXPECTED_BOOLEAN_OR_NULL, SUDDEN_END_OF_DOCUMENT, NOT_A_NUMBER,
                    CHARACTER_AFTER_END_OF_DOCUMENT, NOT_A_HEX_VALUE, EXPECTED_QUOTE_OR_END_BRACE,
                    INVALID_STRING, EXPECTED_COMMA_OR_END_BRACE, EXPECTED_COMMA_OR_END_SQUARE_BRACKET,
                    EXPECTED_STARTING_CURLY_OR_SQUARE_BRACKET};

    ErrorCode LAST_ERROR = OK;
    int LAST_ERROR_POS = -1;
    QString errorToString();
    MainEditor* EDITOR = nullptr;          // Doesn't need to be deleted, because it's a pointer to the primary JsonWax instance.
                                           // Through this pointer, the parser inserts the data.
private:
    const QByteArray* BYTES;
    bool ERROR_REPORTED = false;

    // ============================ MAKES IT POSSIBLE TO FEED DATA TO EDITOR ==========================================
    QList<JsonType*> LOCATION;
    QVariantList KEYS;
    int POS_A, POSITION, SIZE;
    bool CONTAINS_ESCAPED_CHARACTERS = false;
    bool NUMBER_CONTAINS_DOT_OR_E = false;
    QList<EscapedCharacter> ESCAPED_CHARACTERS;

    void editorCreateAndOpenRoot( JsonType* jsonType);
    void editorCreateDir( const QVariant& key, JsonType* jsonType);
    void editorCreateAndOpenDir( const QVariant& key, JsonType* jsonType);
    void editorExitLocation();
    void editorSaveValue( const QVariant& value);
    QVariant rangeToVariant( QMetaType::Type saveAsType);
    // ========================= END OF FEED DATA TO EDITOR CODE =============================================
    bool error( ErrorCode code);
    bool checkHex(int length);
    // ------------ START OF VERIFY NUMBER ------------
    bool number8();
    bool number7();
    bool number6();
    bool number5();
    bool number4();
    bool number3();
    bool number2();
    bool number1();
    bool parseNumber();
    // ------------ END OF VERIFY NUMBER ------------
    void skipSpace();
    bool expectChar( QChar character);
    bool expectExactStr( QString toExpect);
    bool parseInnerObject();
    bool parseObject();
    bool parseInnerArray();
    bool parseArray();
    bool parseString();
    bool parseValue();
public:
    bool parseAndLoad( const QByteArray& bytes);
};
}

#endif // JSONWAX_PARSER_H
