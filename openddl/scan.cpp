SIMPLESCAN_DefineScannerWrappers(Scanner, s);

internal void
MaybeTransformIdentifierIntoReservedWord(Token * token)
{
    struct ReservedWord
    {
        String word;
        TokenType tokenType;
    };

    // @Slow - Probably worth sorting/binary searching this
    static const ReservedWord reservedWords[] =
        {
#define MakeReservedWord(STRING, TOKENTYPE) { StringFromLit(#STRING), TokenType::##TOKENTYPE }
            // Types

            MakeReservedWord(bool,   Bool),    MakeReservedWord(b,       Bool),
            MakeReservedWord(int8,   Int8),    MakeReservedWord(i8,      Int8),
            MakeReservedWord(int16,  Int16),   MakeReservedWord(i16,     Int16),
            MakeReservedWord(int32,  Int32),   MakeReservedWord(i32,     Int32),
            MakeReservedWord(int64,  Int64),   MakeReservedWord(i64,     Int64),   // NOTE - unsigned_int is not in spec 3.0, but is supported for backwards compatibility
            MakeReservedWord(uint8,  UInt8),   MakeReservedWord(u8,      UInt8),   MakeReservedWord(unsigned_int8,      UInt8),
            MakeReservedWord(uint16, UInt16),  MakeReservedWord(u16,     UInt16),  MakeReservedWord(unsigned_int16,     UInt16),
            MakeReservedWord(uint32, UInt32),  MakeReservedWord(u32,     UInt32),  MakeReservedWord(unsigned_int32,     UInt32),
            MakeReservedWord(uint64, UInt64),  MakeReservedWord(u64,     UInt64),  MakeReservedWord(unsigned_int64,     UInt64),
            MakeReservedWord(f32,    Float32), MakeReservedWord(float32, Float32), MakeReservedWord(float,  Float32), MakeReservedWord(f, Float32),
            MakeReservedWord(f64,    Float64), MakeReservedWord(float64, Float64), MakeReservedWord(double, Float64), MakeReservedWord(d, Float64),
            MakeReservedWord(string, String),  MakeReservedWord(s,       String),
            MakeReservedWord(ref,    Ref),     MakeReservedWord(r,       Ref),
            MakeReservedWord(type,   Type),    MakeReservedWord(t,       Type),

            // Non-type
            MakeReservedWord(true,  True),
            MakeReservedWord(false, False),
            MakeReservedWord(null,  Null),
#undef MakeReservedWord
        };

    if (token->type != TokenType::Identifier) return;

    // Optimization to avoid copying super long identifiers (too long to be reserved words)
    if (token->lexeme.cBytes > 16) return;

    for (int iReservedWord = 0; iReservedWord < ArrayLen(reservedWords); iReservedWord++)
    {
        ReservedWord reservedWord = reservedWords[iReservedWord];
        if (AreStringsEqual(token->lexeme, reservedWord.word))
        {
            token->type = reservedWord.tokenType;
            return;
        }
    }
}

inline int
ConsumeWhitespaceAndComments(Scanner * scanner)
{
    using namespace SimpleScan;
    
    int cntConsumed = 0;

    while (true)
    {
        // Consume whitespace
        
        while (IsWhitespace(PeekChar(scanner)))
        {
            NextChar(scanner);
            cntConsumed++;
        }

        // If there are comments, consume them and then repeat
        
        bool isLineComment = (PeekChar(scanner) == '/' && PeekChar(scanner, 1) == '/');
        bool isBlockComment = (PeekChar(scanner) == '/' && PeekChar(scanner, 1) == '*');
        if (!isLineComment && !isBlockComment)
            break;

        NextChar(scanner);
        NextChar(scanner);
        
        if (isLineComment)
        {
            ConsumeUntil(scanner, '\n');
            NextChar(scanner);
        }
        else
        {
            // Block comment

            while (true)
            {
                ConsumeUntil(scanner, '*');
                NextChar(scanner);

                char maybeSlash = NextChar(scanner);
                if (maybeSlash == '/')
                    break;
            }
        }
    }
    
    return cntConsumed;
}

inline bool
IsDecimalDigit(char c)
{
    bool result = (c >= '0' && c <= '9');
    return result;
}

inline bool
IsNumberSeparator(char c)
{
    bool result = (c == SCAN::numberSeparator);
    return result;
}

inline bool
IsDecimalDigitOrSeparator(char c)
{
    bool result = IsDecimalDigit(c) || IsNumberSeparator(c);
    return result;
}

inline bool
IsHexDigit(char c)
{
    bool result =
        (c >= '0' && c <= '9') ||
        (c >= 'a' && c <= 'f') ||
        (c >= 'A' && c <= 'F');

    return result;
}

inline bool
IsHexDigitOrSeparator(char c)
{
    bool result = IsHexDigit(c) || IsNumberSeparator(c);
    return result;
}

inline bool
IsOctalDigit(char c)
{
    bool result = (c >= '0' && c <= '8');
    return result;
}

inline bool
IsOctalDigitOrSeparator(char c)
{
    bool result = IsOctalDigit(c) || IsNumberSeparator(c);
    return result;
}

inline bool
IsBinaryDigit(char c)
{
    bool result = c == '0' || c == '1';
    return result;
}

inline bool
IsBinaryDigitOrSeparator(char c)
{
    bool result = IsBinaryDigit(c) || IsNumberSeparator(c);
    return result;
}

internal int
ConsumeDecimalDigitsAndSeparators(Scanner * scanner)
{
    int cntConsumed = 0;
    while (IsDecimalDigitOrSeparator(PeekChar(scanner)))
    {
        NextChar(scanner);
        cntConsumed++;
    }

    return cntConsumed;
}

internal int
ConsumeHexDigitsAndSeparators(Scanner * scanner)
{
    int cntConsumed = 0;
    while (IsHexDigitOrSeparator(PeekChar(scanner)))
    {
        NextChar(scanner);
        cntConsumed++;
    }

    return cntConsumed;
}

internal int
ConsumeOctalDigitsAndSeparators(Scanner * scanner)
{
    int cntConsumed = 0;
    while (IsOctalDigitOrSeparator(PeekChar(scanner)))
    {
        NextChar(scanner);
        cntConsumed++;
    }

    return cntConsumed;
}

internal int
ConsumeBinaryDigitsAndSeparators(Scanner * scanner)
{
    int cntConsumed = 0;
    while (IsBinaryDigitOrSeparator(PeekChar(scanner)))
    {
        NextChar(scanner);
        cntConsumed++;
    }

    return cntConsumed;
}

inline bool
IsLegalStartOfIdentifier(char c)
{
    bool result = (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c == '_');
    return result;
}

inline bool
IsLegalAfterStartOfIdentifier(char c)
{
    bool result = IsLegalStartOfIdentifier(c) || IsDecimalDigit(c);
    return result;
}

internal int
ConsumeLegalCharsAfterStartOfIdentifier(Scanner * scanner)
{
    int cntConsumed = 0;
    while (IsLegalAfterStartOfIdentifier(PeekChar(scanner)))
    {
        NextChar(scanner);
        cntConsumed++;
    }

    return cntConsumed;
}

internal Token
NextToken(Scanner * scanner)
{
    if (scanner->peeked.token.type != TokenType::Nil)
    {
        Token result = scanner->peeked.token;
        scanner->s.iByte = scanner->peeked.iByteAfterPeek;
        scanner->s.iLine = scanner->peeked.iLineAfterPeek;
        scanner->peeked = {};
        
        return result;
    }
    
    ConsumeWhitespaceAndComments(scanner);

    Token result = {};
    result.lexeme.bytes = scanner->s.contents + scanner->s.iByte;

    char c = PeekChar(scanner);
    switch (c)
    {
        case '(':
        {
            NextChar(scanner);
            result.type = TokenType::LParen;
        } break;

        case ')':
        {
            NextChar(scanner);
            result.type = TokenType::RParen;
        } break;

        case '{':
        {
            NextChar(scanner);
            result.type = TokenType::LBrace;
        } break;

        case '}':
        {
            NextChar(scanner);
            result.type = TokenType::RBrace;
        } break;

        case '[':
        {
            NextChar(scanner);
            result.type = TokenType::LBracket;
        } break;

        case ']':
        {
            NextChar(scanner);
            result.type = TokenType::RBracket;
        } break;

        case '$':
        case '%':
        {
            NextChar(scanner);

            if (!IsLegalStartOfIdentifier(PeekChar(scanner)))
                goto LError;
            
            NextChar(scanner);
            ConsumeLegalCharsAfterStartOfIdentifier(scanner);
                
            result.type = TokenType::Name;

            if (PeekChar(scanner) == '%')
            {
                while (PeekChar(scanner) == '%')
                {
                    NextChar(scanner);
                
                    if (!IsLegalStartOfIdentifier(PeekChar(scanner)))
                        goto LError;
            
                    NextChar(scanner);
                    ConsumeLegalCharsAfterStartOfIdentifier(scanner);
                }
                
                result.type = TokenType::NamePath;
            }
            else
            {
                result.type = TokenType::Name;
            }
        } break;

        case '=':
        {
            NextChar(scanner);
            result.type = TokenType::Equal;
        } break;

        case '*':
        {
            NextChar(scanner);
            result.type = TokenType::Star;
        } break;

        case ',':
        {
            NextChar(scanner);
            result.type = TokenType::Comma;
        } break;
        
        case '+':
        {
            NextChar(scanner);
            result.type = TokenType::Plus;
        } break;

        case '-':
        {
            NextChar(scanner);
            result.type = TokenType::Minus;
        } break;

        case '\'':
        {
            NextChar(scanner);
            
            // NOTE - char literal lexexeme includes the containing ' characters
            //  We are lax while scanning the lexeme. We make sure the char literal
            //  actually conforms to spec when we parse it

            bool isEmpty = true;
            while (!MatchNextChar(scanner, '\''))
            {
                if (!HasNextChar(scanner))
                    goto LError;

                isEmpty = false;
                
                char charLiteralChar = NextChar(scanner);
                if (charLiteralChar == '\\')
                {
                    // Make sure we don't prematurely end the string on a \'
                    NextChar(scanner);
                }
            }

            if (isEmpty)
                goto LError;
            
            result.type = TokenType::CharLiteral;
        } break;

        case '"':
        {
            NextChar(scanner);

            // NOTE - string lexexeme includes the containing " characters
            //  We are lax while scanning the lexeme. We make sure the string
            //  actually conforms to spec when we parse it
            
            while (!MatchNextChar(scanner, '"'))
            {
                if (!HasNextChar(scanner))
                    goto LError;
                
                char stringChar = NextChar(scanner);
                if (stringChar == '\\')
                {
                    // Make sure we don't prematurely end the string on a \"
                    NextChar(scanner);
                }
            }

            // TODO - Concatenate adjacent strings (e.g. "hello" " world")
            //  The spec allows this. It should be pretty easy in the scanner,
            //  but might take a tad more work in the parser where we actually
            //  interpret the string.
            
            result.type = TokenType::StringLiteral;
        } break;

        case '\0':
        {
            result.type = TokenType::Eof;
        } break;

        default:
        {
            if (IsDecimalDigit(c) || c == '.')
            {
                NextChar(scanner);

                if (c == '0' && (MatchNextChar(scanner, 'x') || MatchNextChar(scanner, 'X')))
                {
                    if (!IsHexDigit(PeekChar(scanner)))
                        goto LError;

                    ConsumeHexDigitsAndSeparators(scanner);
                    result.type = TokenType::HexLiteral;
                }
                else if (c == '0' && (MatchNextChar(scanner, 'b') || MatchNextChar(scanner, 'B')))
                {
                    if (!IsBinaryDigit(PeekChar(scanner)))
                        goto LError;

                    ConsumeBinaryDigitsAndSeparators(scanner);
                    result.type = TokenType::BinaryLiteral;
                }
                else if (c == '0' && (MatchNextChar(scanner, 'o') || MatchNextChar(scanner, 'O')))
                {
                    if (!IsOctalDigit(PeekChar(scanner)))
                        goto LError;

                    ConsumeOctalDigitsAndSeparators(scanner);
                    result.type = TokenType::OctalLiteral;
                }
                else
                {
                    // Decimal (int or float)

                    bool hasDot = (c == '.');
                    if (hasDot)
                    {
                        if (!IsDecimalDigit(PeekChar(scanner)))
                            goto LError;

                        NextChar(scanner);
                    }
                
                    // NOTE - Guaranteed that one digit has already been consumed by the time we get here,
                    //  so we can go ahead and start consuming separators too.
                    
                    ConsumeDecimalDigitsAndSeparators(scanner);

                    if (!hasDot && MatchNextChar(scanner, '.'))
                    {
                        hasDot = true;

                        if (IsDecimalDigit(PeekChar(scanner)))
                        {
                            NextChar(scanner);

                            ConsumeDecimalDigitsAndSeparators(scanner);
                        }
                    }

                    bool hasExponent = false;
                    if (MatchNextChar(scanner, 'e') || MatchNextChar(scanner, 'E'))
                    {
                        hasExponent = true;

                        // @Hack - Abusing short-circuit eval the same way we are in many if's in this file... we
                        //  just happen to not care about the result of this one (we want to match either + or - or neither)
                        MatchNextChar(scanner, '+') || MatchNextChar(scanner, '-');

                        if (!IsDecimalDigit(PeekChar(scanner)))
                            goto LError;

                        ConsumeDecimalDigitsAndSeparators(scanner);
                    }

                    if (hasDot || hasExponent)
                    {
                        result.type = TokenType::DecimalFloatLiteral;
                    }
                    else
                    {
                        result.type = TokenType::DecimalLiteral;
                    }
                }
            }
            else if (IsLegalStartOfIdentifier(c))
            {
                NextChar(scanner);
                ConsumeLegalCharsAfterStartOfIdentifier(scanner);

                // NOTE - Reserved words are checked for after the fact
                
                result.type = TokenType::Identifier;
            }
            else
            {
                goto LError;
            }
        } break;
    }

    result.lexeme.cBytes = scanner->s.iByte - (int)(result.lexeme.bytes - scanner->s.contents);
    MaybeTransformIdentifierIntoReservedWord(&result);
    return result;

LError:
    result.type = TokenType::Nil;
    result.lexeme.cBytes = scanner->s.iByte - (int)(result.lexeme.bytes - scanner->s.contents); // Maybe useful to let caller know how far we got before getting confused?
    return result;
}

internal Token
PeekToken(Scanner * scanner)
{
    if (scanner->peeked.token.type != TokenType::Nil)
    {
        return scanner->peeked.token;
    }
    
    Scanner probe = *scanner;
    scanner->peeked.token = NextToken(&probe);
    scanner->peeked.iByteAfterPeek = probe.s.iByte;
    scanner->peeked.iLineAfterPeek = probe.s.iLine;
    return scanner->peeked.token;
}

internal bool
MatchPeekToken(Scanner * scanner, TokenType type, Token * poToken) // !optional =nullptr
{
    Token peeked = PeekToken(scanner);
    if (peeked.type == type)
    {
        if (poToken) *poToken = peeked;
        
        return true;
    }

    return false;
}

internal bool
MatchNextToken(Scanner * scanner, TokenType type, Token * poToken) // !optional =nullptr
{
    if (MatchPeekToken(scanner, type, poToken))
    {
        NextToken(scanner);
        return true;
    }

    return false;
}


inline bool
IsAtEofToken(Scanner * scanner)
{
    bool result = MatchPeekToken(scanner, TokenType::Eof);
    return result;
}
