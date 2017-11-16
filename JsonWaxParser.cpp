#include "JsonWaxParser.h"

JsonWaxInternals::EscapedCharacter::EscapedCharacter(JsonWaxInternals::EscapedCharacter::Type type, int position)
    :TYPE(type), POS(position){}

QString JsonWaxInternals::Parser::errorToString()
{
    switch( LAST_ERROR)
    {
    case OK:                                        return "No errors occured.";
    case UNEXPECTED_CHARACTER:                      return "Unexpected character.";
    case EXPECTED_BOOLEAN_OR_NULL:                  return "Expected boolean or null.";
    case EXPECTED_COMMA_OR_END_BRACE:               return "Expected comma or closing curly bracket.";
    case EXPECTED_COMMA_OR_END_SQUARE_BRACKET:      return "Expected comma or closing square bracket.";
    case EXPECTED_STARTING_CURLY_OR_SQUARE_BRACKET: return "Expected opening curly or square bracket.";
    case SUDDEN_END_OF_DOCUMENT:                    return "Document ended unexpectedly.";
    case NOT_A_NUMBER:                              return "Not a number.";
    case CHARACTER_AFTER_END_OF_DOCUMENT:           return "Character after end of document.";
    case NOT_A_HEX_VALUE:                           return "Not a hexadecimal value.";
    case EXPECTED_QUOTE_OR_END_BRACE:               return "Expected quote or closing curly bracket.";
    case INVALID_STRING:                            return "Invalid string.";
    default:                                        return "";
    }
}

void JsonWaxInternals::Parser::editorCreateAndOpenRoot(JsonWaxInternals::JsonType *jsonType)
{
    EDITOR->ROOT = jsonType;
    LOCATION.append( jsonType);
}

void JsonWaxInternals::Parser::editorCreateDir(const QVariant &key, JsonWaxInternals::JsonType *jsonType)
{
    LOCATION.last()->insertStrong( key, jsonType);
}

void JsonWaxInternals::Parser::editorCreateAndOpenDir(const QVariant &key, JsonWaxInternals::JsonType *jsonType)
{
    JsonType* fresh = LOCATION.last()->insertStrong( key, jsonType);
    LOCATION.append( fresh);
}

void JsonWaxInternals::Parser::editorExitLocation()
{
    if (!LOCATION.isEmpty())
        LOCATION.removeLast();
}

void JsonWaxInternals::Parser::editorSaveValue(const QVariant &value)
{
    LOCATION.last()->setValue( KEYS.last(), value);
}

QVariant JsonWaxInternals::Parser::rangeToVariant(QMetaType::Type saveAsType)
{
    QVariant result;
    int POS_B = POSITION;

    switch( saveAsType)
    {
    case QMetaType::QString:                                                // Get rid of quotes, and replace \uXXXX unicode
    {                                                                       // code points, and other escaped characters, with
        if (!CONTAINS_ESCAPED_CHARACTERS)                                   // the proper characters. The escaped characters
        {                                                                   // were detected during parsing.
            result = QString( BYTES->mid( POS_A, POS_B - POS_A - 1));       // The last character is a closing quotation mark.
        } else {
            QString str;
            QString codepointAsHex;
            int left = POS_A;

            for (EscapedCharacter ch : ESCAPED_CHARACTERS)
            {
                switch(ch.TYPE)
                {
                case EscapedCharacter::Type::ESCAPED_CHARACTER:
                    str.append( BYTES->mid( left, ch.POS - left - 1));          // Until right before the back slash.
                    switch( BYTES->at( ch.POS))
                    {
                    case '\"':  str.append('\"');   break;
                    case '\\':  str.append('\\');   break;
                    case '/':   str.append('/');    break;
                    case 'b':   str.append('\b');   break;
                    case 'f':   str.append('\f');   break;
                    case 'n':   str.append('\n');   break;
                    case 'r':   str.append('\r');   break;
                    case 't':   str.append('\t');   break;
                    default: break; // Can't happen.
                    }
                    left = ch.POS + 1;
                    break;
                case EscapedCharacter::Type::CODE_POINT:
                {
                    str.append( BYTES->mid( left, ch.POS - left));
                    codepointAsHex = BYTES->mid( ch.POS + 2, 4);
                    int codepointAsInt = std::stoi (codepointAsHex.toStdString(), 0, 16);
                    str.append( QChar( codepointAsInt));
                    left = ch.POS + 6;
                    break;
                }
                default: break; // Can't happen.
                }
            }
            str.append( BYTES->mid( left, POS_B - left - 1));                   // The last character is a closing quotation mark.
            result = str;
            CONTAINS_ESCAPED_CHARACTERS = false;
        }
        break;
    }
    case QMetaType::Int:                                                        // It's not actually stored as an Int.
    {                                                                           // The type of the number is determined here.
        QByteArray bytesResult (BYTES->mid( POS_A, POS_B - POS_A));

        if (NUMBER_CONTAINS_DOT_OR_E)
        {
            result = bytesResult.toDouble();
        } else if (bytesResult.size() > 9) {
            if (bytesResult.toLongLong() > 2147483647 || bytesResult.toLongLong() < -2147483647)
                result = bytesResult.toLongLong();
            else
                result = bytesResult.toInt();
        } else {
            result = bytesResult.toInt();
        }
        break;
    }
    case QMetaType::Bool:
        if (BYTES->at(POS_A) == 't')
            result = true;
        else
            result = false;
        break;
    default: break; // Empty QVariant.
    }
    return result;
}

bool JsonWaxInternals::Parser::error(JsonWaxInternals::Parser::ErrorCode code)
{
    LAST_ERROR = code;
    LAST_ERROR_POS = POSITION;
    ERROR_REPORTED = true;
    return false;
}

bool JsonWaxInternals::Parser::checkHex(int length)
{
    int iteration = 0;
    while ( POSITION < SIZE && iteration < length)
        switch (BYTES->at( POSITION++))
        {
        case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
        case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
        case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
            ++iteration;
            continue;
        default:
            return error( NOT_A_HEX_VALUE);
        }
    return true;
}

bool JsonWaxInternals::Parser::number8()     // Acceptable position.
{
    while (POSITION < SIZE)
    {
        switch (BYTES->at( POSITION++))
        {
        case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
            return number8();
        default:
            --POSITION;
            return true;
        }
    }
    return error( SUDDEN_END_OF_DOCUMENT);
}

bool JsonWaxInternals::Parser::number7()     // Acceptable position.
{
    while (POSITION < SIZE)
    {
        switch (BYTES->at( POSITION++))
        {
        case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
            return number7();
        case 'e': case 'E':
            NUMBER_CONTAINS_DOT_OR_E = true;
            return number4();
        default:
            --POSITION;
            return true;
        }
    }
    return error( SUDDEN_END_OF_DOCUMENT);
}

bool JsonWaxInternals::Parser::number6()     // Acceptable position.
{
    while (POSITION < SIZE)
    {
        switch (BYTES->at( POSITION++))
        {
        case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
            return number6();
        case '.':
            NUMBER_CONTAINS_DOT_OR_E = true;
            return number3();
        case 'e': case 'E':
            NUMBER_CONTAINS_DOT_OR_E = true;
            return number4();
        default:
            --POSITION;
            return true;
        }
    }
    return error( SUDDEN_END_OF_DOCUMENT);
}

bool JsonWaxInternals::Parser::number5()
{
    while (POSITION < SIZE)
    {
        switch (BYTES->at( POSITION++))
        {
        case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
            return number8();
        default:
            --POSITION;
            return false;
        }
    }
    return error( SUDDEN_END_OF_DOCUMENT);
}

bool JsonWaxInternals::Parser::number4()
{
    while (POSITION < SIZE)
    {
        switch (BYTES->at( POSITION++))
        {
        case '+': case '-':
            return number5();
        case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
            return number8();
        default:
            --POSITION;
            return false;
        }
    }
    return error( SUDDEN_END_OF_DOCUMENT);
}

bool JsonWaxInternals::Parser::number3()
{
    while (POSITION < SIZE)
    {
        switch (BYTES->at( POSITION++))
        {
        case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
            return number7();
        default:
            --POSITION;
            return false;
        }
    }
    return error( SUDDEN_END_OF_DOCUMENT);
}

bool JsonWaxInternals::Parser::number2()      // Acceptable position.
{
    while (POSITION < SIZE)
    {
        switch (BYTES->at( POSITION++))
        {
        case 'e': case 'E':
            NUMBER_CONTAINS_DOT_OR_E = true;
            return number4();
        case '.':
            NUMBER_CONTAINS_DOT_OR_E = true;
            return number3();
        default:
            --POSITION;
            return true;
        }
    }
    return error( SUDDEN_END_OF_DOCUMENT);
}

bool JsonWaxInternals::Parser::number1()
{
    while (POSITION < SIZE)
    {
        switch (BYTES->at( POSITION++))
        {
        case '0':
            return number2();
        case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
            return number6();
        default:
            --POSITION;
            return false;
        }
    }
    return error( SUDDEN_END_OF_DOCUMENT);
}

bool JsonWaxInternals::Parser::parseNumber()
{
    NUMBER_CONTAINS_DOT_OR_E = false;

    while (POSITION < SIZE)
    {
        switch (BYTES->at( POSITION++))
        {
        case '-':
            if (!number1()) {
                if (!ERROR_REPORTED)
                    return error( NOT_A_NUMBER);
                return false;
            } else return true;
        case '0':
            if (!number2()) {
                if (!ERROR_REPORTED)
                    return error( NOT_A_NUMBER);
                return false;
            } else return true;
        case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
            if (!number6()) {
                if (!ERROR_REPORTED)
                    return error( NOT_A_NUMBER);
                return false;
            } else return true;
        default:
            --POSITION;
            return error( NOT_A_NUMBER);
        }
    }
    return error( SUDDEN_END_OF_DOCUMENT);
}

void JsonWaxInternals::Parser::skipSpace()
{
    while ( POSITION < SIZE )
        if (BYTES->at( POSITION) == ' ' || BYTES->at( POSITION) == '\n' || BYTES->at( POSITION) == '\r' || BYTES->at( POSITION) == '\t')
        {
            ++POSITION;
            continue;
        } else {
            return;
        }
}

bool JsonWaxInternals::Parser::expectChar(QChar character)
{
    skipSpace();
    while ( POSITION < SIZE )
    {
        if (BYTES->at( POSITION++) == character)
            return true;
        else
            return error( UNEXPECTED_CHARACTER);
    }
    return error( SUDDEN_END_OF_DOCUMENT);
}

bool JsonWaxInternals::Parser::expectExactStr(QString toExpect) // Except from the first character in the toExpect-string.
{
    int matchPos = 1;

    while ( POSITION < SIZE )
    {
        if (BYTES->at( POSITION++) == toExpect.at( matchPos))
        {
            if (++matchPos > toExpect.length() - 1)
                return true;
            continue;
        } else {
            return error( EXPECTED_BOOLEAN_OR_NULL);
        }
    }
    return error( SUDDEN_END_OF_DOCUMENT);
}

bool JsonWaxInternals::Parser::parseInnerObject()
{
    if (!KEYS.isEmpty())                                                // [EDITOR]
        editorCreateAndOpenDir( KEYS.last(), new JsonObject);            // [EDITOR]

inner_begin:
    POS_A = POSITION;                                                   // [EDITOR]

    if (parseString())
    {
        KEYS.append( rangeToVariant(QMetaType::QString));                // [EDITOR: create key for this item]

        if (expectChar(':'))
        {
            if (parseValue())
            {
                skipSpace();
                while ( POSITION < SIZE )
                {
                    switch( BYTES->at( POSITION++))
                    {
                    case ',':
                        KEYS.removeLast();                              // [EDITOR: back one key]
                        if (expectChar('\"'))
                            goto inner_begin;
                        return false;
                    case '}':
                        KEYS.removeLast();                              // [EDITOR: back one key]
                        editorExitLocation();                           // [EDITOR: back to the parent location] // !KEYS.isEmpty()
                        return true;
                    default:
                        return error( EXPECTED_COMMA_OR_END_BRACE);
                    }
                }
                return error( SUDDEN_END_OF_DOCUMENT);
            }
        }
    }
    return false;
}

bool JsonWaxInternals::Parser::parseObject()
{
    while ( POSITION < SIZE )
    {
        skipSpace();
        if (POSITION < SIZE)
        {
            switch (BYTES->at( POSITION))
            {
            case '\"':
                ++POSITION;
                return parseInnerObject();
            case '}':
                ++POSITION;

                if (!KEYS.isEmpty())
                {
                    editorCreateAndOpenDir( KEYS.last(), new JsonObject);       // [EDITOR]
                    editorExitLocation();                                       // [EDITOR]
                }
                return true;
            default:
                return error( EXPECTED_QUOTE_OR_END_BRACE);
            }
        } else {
            return error( SUDDEN_END_OF_DOCUMENT);
        }
    }
    return error( SUDDEN_END_OF_DOCUMENT);
}

bool JsonWaxInternals::Parser::parseInnerArray()
{
    if (!KEYS.isEmpty())                                                // [EDITOR]
        editorCreateAndOpenDir( KEYS.last(), new JsonArray);            // [EDITOR]

    int elementPosition = 0;

inner_begin:
    skipSpace();
    POS_A = POSITION;                                                   // [EDITOR]
    KEYS.append( elementPosition);                                      // [EDITOR]

    if (parseValue())
    {
        KEYS.removeLast();                                              // [EDITOR]

        skipSpace();
        while ( POSITION < SIZE )
        {
            switch( BYTES->at( POSITION))
            {
            case ',':
                ++POSITION;
                ++elementPosition;
                goto inner_begin;
            case ']':                                                   // There's only one way to end the array: with a ]
                ++POSITION;
                editorExitLocation();                                   // [EDITOR: back to parent directory]
                return true;
            default:
                return error( EXPECTED_COMMA_OR_END_SQUARE_BRACKET);
            }
        }
        return error( SUDDEN_END_OF_DOCUMENT);
    } else {
        return false;                                                   // The error was already reported.
    }
}

bool JsonWaxInternals::Parser::parseArray()
{
    skipSpace();
    while ( POSITION < SIZE )
    {
        switch( BYTES->at( POSITION))
        {
        case ']':

            if (!KEYS.isEmpty())
            {
                editorCreateAndOpenDir( KEYS.last(), new JsonArray);        // [EDITOR]
                editorExitLocation();                                       // [EDITOR]
            }
            ++POSITION;
            return true;
        default:
            return parseInnerArray();
        }
    }
    return error( SUDDEN_END_OF_DOCUMENT);
}

bool JsonWaxInternals::Parser::parseString()
{
    ESCAPED_CHARACTERS.clear();                                         // [EDITOR]
    CONTAINS_ESCAPED_CHARACTERS = false;                                // [EDITOR]

    while ( POSITION < SIZE )
    {
        switch ( BYTES->at( POSITION++))                                // The loop is looking for backspace or "
        {
        case '\\':
            switch ( BYTES->at( POSITION++))                            // Inner test.
            {
            case '\"': case '\\': case '/': case 'b': case 'f': case 'n': case 'r': case 't':
                ESCAPED_CHARACTERS.append( EscapedCharacter( EscapedCharacter::Type::ESCAPED_CHARACTER, POSITION - 1));
                CONTAINS_ESCAPED_CHARACTERS = true;
                break;                                                  // Go to next character.
            case 'u':
                if (checkHex(4))
                {
                    ESCAPED_CHARACTERS.append( EscapedCharacter( EscapedCharacter::Type::CODE_POINT, POSITION - 6));
                    CONTAINS_ESCAPED_CHARACTERS = true;                 // A valid code point always has the same length.
                    --POSITION;
                    break;                                              // Get out of inner, go to next character.
                } else {
                    --POSITION;
                    return error( NOT_A_HEX_VALUE);
                }
            default:
                return error( INVALID_STRING);
            }
            break;
        case '\"':
            return true;                                                // End of string.
        }
    }
    return error( SUDDEN_END_OF_DOCUMENT);
}

bool JsonWaxInternals::Parser::parseValue()
{
    skipSpace();
    POS_A = POSITION;                                                   // [Editor]
    while ( POSITION < SIZE )
    {
        switch ( BYTES->at( POSITION++))
        {
        case '{':
            return parseObject();
        case '[':
            return parseArray();
        case '\"':
        {
            ++POS_A;                                                        // [EDITOR: skip the opening quotation mark]
            bool isValid = parseString();
            if (isValid)
                editorSaveValue( rangeToVariant( QMetaType::QString));
            return isValid;
        }
        case '-': case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
        {
            --POSITION;
            bool isValid = parseNumber();
            if (isValid)
                editorSaveValue( rangeToVariant( QMetaType::Int));             // [EDITOR: It's not necessarily an int, just any valid number]
            return isValid;
        }
        case 't':
        {
            bool isValid = expectExactStr("true");
            if (isValid)
                editorSaveValue( rangeToVariant( QMetaType::Bool));            // [EDITOR]
            return isValid;
        }
        case 'f':
        {
            bool isValid = expectExactStr("false");
            if (isValid)
                editorSaveValue( rangeToVariant( QMetaType::Bool));            // [EDITOR]
            return isValid;
        }
        case 'n':
        {
            bool isValid = expectExactStr("null");
            if (isValid)
                editorSaveValue( rangeToVariant( QMetaType::Void));            // [EDITOR]
            return isValid;
        }
        default:
            return error( UNEXPECTED_CHARACTER);
        }
    }
    return error( SUDDEN_END_OF_DOCUMENT);
}

bool JsonWaxInternals::Parser::parseAndLoad(const QByteArray &bytes)
{
    POSITION = 0;
    BYTES = &bytes;
    SIZE = bytes.size();
    EDITOR->ROOT->destroy();                                            // [EDITOR] This deletes all data in editor.
    KEYS.clear();                                                       // [EDITOR]

    while (POSITION < SIZE)
    {
        skipSpace();
        switch( bytes.at( POSITION++))
        {
        case '{':
            editorCreateAndOpenRoot( new JsonObject);
            if (!parseObject())                                        // Only return in case of failure.
                return false;
            break;
        case '[':
            editorCreateAndOpenRoot( new JsonArray);
            if (!parseArray())                                         // Only return in case of failure.
                return false;
            break;
        default:
            editorCreateAndOpenRoot( new JsonObject);                   // Create an empty Object in case of error.
            return error( EXPECTED_STARTING_CURLY_OR_SQUARE_BRACKET);
        }
        skipSpace();
        if (POSITION < SIZE)
        {
            return error( CHARACTER_AFTER_END_OF_DOCUMENT);
        }
        LAST_ERROR_POS = -1;
        LAST_ERROR = OK;
        return true;                                                    // We are at the end of the document,
        // and the object or array was valid.
    }
    editorCreateAndOpenRoot( new JsonObject);                           // Create an empty Object in case of error.
    return error( SUDDEN_END_OF_DOCUMENT);
}
