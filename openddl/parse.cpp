internal u64
ParseDecimalLiteral(Scanner * scanner, bool * poSuccess)
{
    Token matched;
    if (!MatchNextToken(scanner, TokenType::DecimalLiteral, &matched))
    {
        *poSuccess = false;
        return 0;
    }

    Assert(matched.lexeme.cBytes > 0);

    // HMM - Factor below out into util?
    
    u64 result = 0;
    *poSuccess = TryParseU64FromEntireString(matched.lexeme, &result, SCAN::numberSeparator);
    return result;
}


internal f64
ParseDecimalFloatLiteral(Scanner * scanner, bool * poSuccess)
{
    Token matched;
    if (!MatchNextToken(scanner, TokenType::DecimalFloatLiteral, &matched))
    {
        *poSuccess = false;
        return 0;
    }

    f64 result;
    *poSuccess = TryParseF64FromEntireString(matched.lexeme, &result, SCAN::numberSeparator);
    return result;
}

internal u64
ParseHexLiteral(Scanner * scanner, bool * poSuccess)
{
    Token matched;
    if (!MatchNextToken(scanner, TokenType::HexLiteral, &matched))
        goto LError;

    // HMM - Factor below out into util?
    
    u64 result = 0;
    
    int cntDigit = 0;
    for (int iChar = 2; iChar < matched.lexeme.cBytes; iChar++)
    {
        char c = matched.lexeme[iChar];
        if (IsNumberSeparator(c))
            continue;
        
        Assert(IsHexDigit(c));
        
        result <<= 4;

        if (IsDecimalDigit(c))
        {
            result |= (c - '0');
        }
        else if (c >= 'a' && c <= 'f')
        {
            result |= (c - 'a');
        }
        else if (c >= 'A' && c <= 'F')
        {
            result |= (c - 'A');
        }
        
        cntDigit++;
    }

    if (cntDigit == 0)
        goto LError;

    if (cntDigit > 16)
        goto LError; // overflow

    *poSuccess = true;
    return result;
    
LError:
    *poSuccess = false;
    return 0;
}

#if 0
// Currently unsupported
internal u64
ParseOctalLiteral(Scanner * scanner, bool * poSuccess)
{
    Token matched;
    if (!MatchNextToken(scanner, TokenType::OctalLiteral, &matched))
        goto LError;

    // HMM - Factor below out into util?
    
    u64 result = 0;
    
    int cntDigit = 0;
    for (int iChar = 2; iChar < matched.cBytesLexeme; iChar++)
    {
        char c = matched.lexeme[iChar];
        if (IsNumberSeparator(c))
            continue;
        
        Assert(IsOctalDigit(c));

        result << 3;
        result |= (c - '0');
        cntDigit++;
    }

    if (cntDigit == 0)
        goto LError;

    // Check for overflow. We can't just count # of digits like we do for hex, since each octal digit is 3 bits and 64 doesn't divide by 3
    AssertTodo;

    *poSuccess = true;
    return result;

LError:
    *poSuccess = false;
    return 0;
}
#endif

internal u64
ParseBinaryLiteral(Scanner * scanner, bool * poSuccess)
{
    Token matched;
    if (!MatchNextToken(scanner, TokenType::BinaryLiteral, &matched))
        goto LError;

    // HMM - Factor below out into util?
    
    u64 result = 0;
    
    int cntDigit = 0;
    for (int iChar = 2; iChar < matched.lexeme.cBytes; iChar++)
    {
        char c = matched.lexeme[iChar];
        if (IsNumberSeparator(c))
            continue;
        
        Assert(IsBinaryDigit(c));

        result <<= 1;
        result |= (c - '0');
        
        cntDigit++;
    }

    if (cntDigit == 0)
        goto LError;

    if (cntDigit > 64)
        goto LError; // overflow

    *poSuccess = true;
    return result;

LError:
    *poSuccess = false;
    return 0;
}

inline u8
HexDigitToByte(char c)
{
    // @Slow... Use lookup table?
    if      (c >= '0' && c <= '9') return c - '0';
    else if (c >= 'A' && c <= 'F') return 10 + (c - 'A');
    else if (c >= 'a' && c <= 'f') return 10 + (c - 'a');
    else
    {
        AssertFalse;
        return 0;
    }
}
internal bool
WriteEscapedCharAndAdvanceCursors(
    char ** src,
    char ** dst,
    bool allowExtendedByteSequences, // \u and \U (allowed in string literals)
    bool allow_x00)                  // fine in character literals, but doesn't make any sense in strings
{
    Assert(*(*src - 1) == '\\');

    char escaped = **src;
    (*src)++;

    switch (escaped)
    {
        case '"':  **dst = 0x22; (*dst)++; break;
        case '\'': **dst = 0x27; (*dst)++; break;
        case '?':  **dst = 0x3F; (*dst)++; break;
        case '\\': **dst = 0x5C; (*dst)++; break;
        case 'a':  **dst = 0x07; (*dst)++; break;
        case 'b':  **dst = 0x08; (*dst)++; break;
        case 'f':  **dst = 0x0C; (*dst)++; break;
        case 'n':  **dst = 0x0A; (*dst)++; break;
        case 'r':  **dst = 0x0D; (*dst)++; break;
        case 't':  **dst = 0x09; (*dst)++; break;
        case 'v':  **dst = 0x0B; (*dst)++; break;
        case 'x':
        {
            char hex1 = **src;
            (*src)++;
                    
            if (!IsHexDigit(hex1))
                goto LError;

            char hex2 = **src;
            (*src)++;
                    
            if (!IsHexDigit(hex2))
                goto LError;

            char nib1 = HexDigitToByte(hex1);
            char nib2 = HexDigitToByte(hex2);
            if (!allow_x00 && nib1 == 0 && nib2 == 0)
                goto LError;
            
            **dst = (nib1 << 4) | nib2;
            (*dst)++;
        } break;

        case 'u':
        case 'U':
        {
            AssertTodo;
            goto LError;
        } break;
        
        // TODO - The \u and \U cases below are flawed. They write raw bytes as one would for \x.
        //  What I *actually* need to do is encode the codepoint in UTF-8 (marker bits and all) and
        //  write *those* bytes.
#if 0
        case 'u':
        {
            if (!allowExtendedByteSequences)
                goto LError;
            
            char hex1 = **src;
            (*src)++;
                    
            if (!IsHexDigit(hex1))
                goto LError;

            char hex2 = **src;
            (*src)++;
                    
            if (!IsHexDigit(hex2))
                goto LError;

            char hex3 = **src;
            (*src)++;
                    
            if (!IsHexDigit(hex3))
                goto LError;

            char hex4 = **src;
            (*src)++;
                    
            if (!IsHexDigit(hex4))
                goto LError;

            char nib1 = HexDigitToByte(hex1);
            char nib2 = HexDigitToByte(hex2);
            char nib3 = HexDigitToByte(hex3);
            char nib4 = HexDigitToByte(hex4);
            if (nib1 == 0 && nib2 == 0 && nib3 == 0 && nib4 == 0)
                goto LError;

            **dst = (nib1 << 4) | nib2;
            (*dst)++;

            **dst = (nib3 << 4) | nib4;
            (*dst)++;
        } break;

        case 'U':
        {
            if (!allowExtendedByteSequences)
                goto LError;
            
            char hex1 = **src;
            (*src)++;
                    
            if (!IsHexDigit(hex1))
                goto LError;

            char hex2 = **src;
            (*src)++;
                    
            if (!IsHexDigit(hex2))
                goto LError;

            char hex3 = **src;
            (*src)++;
                    
            if (!IsHexDigit(hex3))
                goto LError;

            char hex4 = **src;
            (*src)++;
                    
            if (!IsHexDigit(hex4))
                goto LError;

            char hex5 = **src;
            (*src)++;
                    
            if (!IsHexDigit(hex5))
                goto LError;

            char hex6 = **src;
            (*src)++;
                    
            if (!IsHexDigit(hex6))
                goto LError;

            char nib1 = HexDigitToByte(hex1);
            char nib2 = HexDigitToByte(hex2);
            char nib3 = HexDigitToByte(hex3);
            char nib4 = HexDigitToByte(hex4);
            char nib5 = HexDigitToByte(hex5);
            char nib6 = HexDigitToByte(hex6);
            if (nib1 == 0 && nib2 == 0 && nib3 == 0 && nib4 == 0 && nib5 == 0 && nib6 == 0)
                goto LError;
            
            **dst = (nib1 << 4) | nib2;
            (*dst)++;

            **dst = (nib3 << 4) | nib4;
            (*dst)++;

            **dst = (nib5 << 4) | nib6;
            (*dst)++;
        } break;
#endif

        default:
            goto LError;
    }

    return true;
    
LError:
    return false;
}

// --- UTF 8
// TODO - move out to common/utf.h

internal bool
IsValidUtf8CodePoint(u32 codePoint)
{
    bool result =
        (codePoint >= 0x20 && codePoint <= 0x21) ||
        (codePoint >= 0x23 && codePoint <= 0x5B) ||
        (codePoint >= 0x5D && codePoint <= 0x7E) ||
        (codePoint >= 0xA0 && codePoint <= 0xD7FF) ||
        (codePoint >= 0xE000 && codePoint <= 0xFFFD) ||
        (codePoint >= 0x0100000 && codePoint <= 0x10FFFF);

    return result;
}

internal String
ParseStringLiteral(Scanner * scanner, MemoryRegion memory)
{
    //
    // Validate all characters are valid according to the spec
    
    struct Utf8ByteMetadata
    {
        u8 markerMask;
        u8 markerValue;
        u8 markerCntBit;
        u8 codePointMask;
        u8 codePointCntBit;
        u32 maxCodePoint; // Used to test for "overlong" encoding. Only valid for leading byte metadata.
    };

    // Leading byte
    Utf8ByteMetadata oneByte           = { 0b10000000, 0b00000000, 1, 0b01111111, 7, 0b1111111 };
    Utf8ByteMetadata twoBytes          = { 0b11100000, 0b11000000, 3, 0b00011111, 5, 0b11111111111 };
    Utf8ByteMetadata threeBytes        = { 0b11110000, 0b11100000, 4, 0b00001111, 4, 0b1111111111111111 };
    Utf8ByteMetadata fourBytes         = { 0b11111000, 0b11110000, 5, 0b00000111, 3, 0b111111111111111111111 };

    // Continuation bytes
    Utf8ByteMetadata continuationByte  = { 0b11000000, 0b10000000, 2, 0b00111111, 6 };
    String result = {}; // @GoToWarning
    
    Token matched;
    if (!MatchNextToken(scanner, TokenType::StringLiteral, &matched))
        goto LError;
    
    char * src = matched.lexeme.bytes;
    Assert(*src == '"');
    src++;

    // NOTE - Conservative over-allocation. Needed bytes may be shorter due to escaped characters
    // TODO - replace this with a StringBuilder (once it's implemented...)
    
    result.bytes = (char *)Allocate(memory, matched.lexeme.cBytes);
    
    char * dst = result.bytes;
    while (*src != '"')
    {
        u8 b1 = *src;
        src++;
        
        Assert(b1 != '"');

        u32 codePoint = 0;
        if ((b1 & oneByte.markerMask) == oneByte.markerValue)
        {
            codePoint = b1;

            if (codePoint == '\\')
            {
                bool allowExtendedByteSequences = true;
                bool allow_x00 = false;
                if (!WriteEscapedCharAndAdvanceCursors(&src, &dst, allowExtendedByteSequences, allow_x00))
                    goto LError;
            }
            else
            {
                if (!IsValidUtf8CodePoint(codePoint))
                    goto LError;
                
                *dst = b1;
                dst++;
            }
        }
        else if ((b1 & twoBytes.markerMask) == twoBytes.markerValue)
        {
            u8 b2 = *src;
            src++;
            
            if ((b2 & continuationByte.markerMask) != continuationByte.markerValue)
                goto LError;

            codePoint =
                ((u32)(b1 & twoBytes.codePointMask) << (1 * continuationByte.codePointCntBit)) |
                ((u32)(b2 & continuationByte.codePointMask));

            if (codePoint <= oneByte.maxCodePoint)
                goto LError;

            if (!IsValidUtf8CodePoint(codePoint))
                goto LError;
                
            *dst = b1;
            dst++;

            *dst = b2;
            dst++;
        }
        else if ((b1 & threeBytes.markerMask) == threeBytes.markerValue)
        {
            u8 b2 = *src;
            src++;
            
            if ((b2 & continuationByte.markerMask) != continuationByte.markerValue)
                goto LError;
                    
            u8 b3 = *src;
            src++;
            
            if ((b3 & continuationByte.markerMask) != continuationByte.markerValue)
                goto LError;

            codePoint =
                ((u32)(b1 & threeBytes.codePointMask)       << (2 * continuationByte.codePointCntBit)) |
                ((u32)(b2 & continuationByte.codePointMask) << (1 * continuationByte.codePointCntBit)) |
                ((u32)(b3 & continuationByte.codePointMask));

            if (codePoint <= twoBytes.maxCodePoint)
                goto LError;

            if (!IsValidUtf8CodePoint(codePoint))
                goto LError;
                
            *dst = b1;
            dst++;

            *dst = b2;
            dst++;

            *dst = b3;
            dst++;
        }
        else if ((b1 & fourBytes.markerMask) == fourBytes.markerValue)
        {
            u8 b2 = *src;
            src++;
            
            if ((b2 & continuationByte.markerMask) != continuationByte.markerValue)
                goto LError;
                    
            u8 b3 = *src;
            src++;
            
            if ((b3 & continuationByte.markerMask) != continuationByte.markerValue)
                goto LError;
                    
            u8 b4 = *src;
            src++;
            
            if ((b4 & continuationByte.markerMask) != continuationByte.markerValue)
                goto LError;

            codePoint =
                ((u32)(b1 & fourBytes.codePointMask)        << (3 * continuationByte.codePointCntBit)) |
                ((u32)(b2 & continuationByte.codePointMask) << (2 * continuationByte.codePointCntBit)) |
                ((u32)(b3 & continuationByte.codePointMask) << (1 * continuationByte.codePointCntBit)) |
                ((u32)(b4 & continuationByte.codePointMask));

            if (codePoint <= threeBytes.maxCodePoint)
                goto LError;

            if (!IsValidUtf8CodePoint(codePoint))
                goto LError;
                
            *dst = b1;
            dst++;

            *dst = b2;
            dst++;

            *dst = b3;
            dst++;

            *dst = b4;
            dst++;
        }
        else
        {
            goto LError;
        }
    }

    Assert((dst - result.bytes) < matched.lexeme.cBytes);
    result.cBytes = (uint)(dst - result.bytes);
    
    return result;

LError:
    return {};
}

internal u64
ParseCharLiteral(Scanner * scanner, bool * poSuccess)
{
    Token matched;
    if (!MatchNextToken(scanner, TokenType::CharLiteral, &matched))
        goto LError;
    
    char * src = matched.lexeme.bytes;
    Assert(*src == '\'');
    src++;

    int iByte = 0;
    u64 result = 0;
    while (*src != '\'')
    {
        // NOTE - Assuming that >16 characters (8 bytes) overflows, even though you
        //  *could* technically pad with 0's...

        if (iByte >= 8)
            goto LError;
        
        char c = *src;
        src++;
        
        if (c == '\\')
        {
            char b;
            char * dummy = &b;
            bool allowExtendedByteSequences = false;
            bool allow_x00 = true;
            WriteEscapedCharAndAdvanceCursors(&src, &dummy, allowExtendedByteSequences, allow_x00);

            result <<= 8;
            result |= b;
        }
        else
        {
            if (c < 0x20 || c > 0x7E || c == 0x5C || c == 0x27)
                goto LError;

            result <<= 8;
            result |= c;
        }

        iByte++;
    }

    *poSuccess = true;
    return result;

LError:
    *poSuccess = false;
    return 0;
}

internal u64
ParseUInt(Scanner * scanner, PrimitiveDataType dataType, bool * poSuccess)
{
    // @CopyPaste - mostly the same as ParseInt(..)
    
    // NOTE - Returns u64 for convenience, with the expectation that the caller will cast
    //  to the appropriately sized int. If we don't report an error, the caller does not
    //  need to check for overflow
    
    Assert(dataType >= PrimitiveDataType::UInt8  && dataType <= PrimitiveDataType::UInt64);

    MatchNextToken(scanner, TokenType::Plus); // No-op

    u64 result = 0;
    if (MatchPeekToken(scanner, TokenType::DecimalLiteral))
    {
        result = ParseDecimalLiteral(scanner, poSuccess);
    }
    else if (MatchPeekToken(scanner, TokenType::HexLiteral))
    {
        result = ParseHexLiteral(scanner, poSuccess);
    }
    else if (MatchPeekToken(scanner, TokenType::BinaryLiteral))
    {
        result = ParseBinaryLiteral(scanner, poSuccess);
    }
    else if (MatchPeekToken(scanner, TokenType::OctalLiteral))
    {
        AssertTodo; // Currently unsupported
        // result = ParseOctalLiteral(scanner, poSuccess);
    }
    else if (MatchPeekToken(scanner, TokenType::CharLiteral))
    {
        result = ParseCharLiteral(scanner, poSuccess);
    }
    else
    {
        goto LError;
    }
    
    if (!poSuccess)
        goto LError;

    // @Slow - Can probably do some bit mask stuff to check this faster?
    
    switch ((int)dataType)
    {
        case PrimitiveDataType::UInt8:
        {
            if (result > U8::max)
                goto LError;
        } break;

        case PrimitiveDataType::UInt16:
        {
            if (result > U16::max)
                goto LError;
        } break;

        case PrimitiveDataType::UInt32:
        {
            if (result > U32::max)
                goto LError;
        } break;
        
        // NOTE - 64 bit overflow already checked by the Parse___ function we called into

        DefaultInvalid;
    }
    
    *poSuccess = true;
    return result;

LError:
    *poSuccess = false;
    return 0;
}

internal bool
ParseBool(Scanner * scanner, bool * poSuccess)
{
    bool result = false;
    
    Token matched;
    if (MatchNextToken(scanner, TokenType::True))
    {
        result = true;
    }
    else if (MatchNextToken(scanner, TokenType::False))
    {
        result = false;
    }
    else if (MatchPeekToken(scanner, TokenType::DecimalLiteral, &matched))
    {
        if (AreStringsEqual(matched.lexeme, "1"))
        {
            NextToken(scanner);
            result = true;
        }
        else if (AreStringsEqual(matched.lexeme, "0"))
        {
            NextToken(scanner);
            result = false;
        }
        else
        {
            goto LError;
        }
    }
    else if (MatchPeekToken(scanner, TokenType::False, &matched))
    {
        result = false;
    }
    else
    {
        goto LError;
    }

    *poSuccess = true;
    return result;

LError:
    *poSuccess = false;
    return false;
}

internal s64
ParseInt(Scanner * scanner, PrimitiveDataType dataType, bool * poSuccess)
{
    // NOTE - Returns s64 for convenience, with the expectation that the caller will cast
    //  to the appropriately sized int. If we don't report an error, the caller does not
    //  need to check for overflow/underflow.
    
    Assert(dataType >= PrimitiveDataType::Int8  && dataType <= PrimitiveDataType::Int64);
    
    bool isNegated = false;
    if (MatchNextToken(scanner, TokenType::Plus))
    {
        // No-op
    }
    else if (MatchNextToken(scanner, TokenType::Minus))
    {
        isNegated = true;
    }

    s64 result = 0;
    if (MatchPeekToken(scanner, TokenType::DecimalLiteral))
    {
        result = (s64)ParseDecimalLiteral(scanner, poSuccess);
    }
    else if (MatchPeekToken(scanner, TokenType::HexLiteral))
    {
        result = (s64)ParseHexLiteral(scanner, poSuccess);
    }
    else if (MatchPeekToken(scanner, TokenType::BinaryLiteral))
    {
        result = (s64)ParseBinaryLiteral(scanner, poSuccess);
    }
    else if (MatchPeekToken(scanner, TokenType::OctalLiteral))
    {
        AssertTodo; // Currently unsupported
        // result = (s64)ParseOctalLiteral(scanner, poSuccess);
    }
    else if (MatchPeekToken(scanner, TokenType::CharLiteral))
    {
        result = (s64)ParseCharLiteral(scanner, poSuccess);
    }
    else
    {
        goto LError;
    }
    
    if (!poSuccess)
        goto LError;

    // @Slow - Can probably do some bit mask stuff to check this faster?
    
    switch ((int)dataType)
    {
        case PrimitiveDataType::Int8:
        {
            if (result < S8::min || result > S8::max)
                goto LError;
        } break;

        case PrimitiveDataType::Int16:
        {
            if (result < S16::min || result > S16::max)
                goto LError;
        } break;

        case PrimitiveDataType::Int32:
        {
            if (result < S32::min || result > S32::max)
                goto LError;
        } break;
        
        // NOTE - 64 bit overflow already checked by the Parse___ function we called into

        DefaultInvalid;
    }

    if (isNegated)
    {
        result = -result;
    }
    
    Assert(*poSuccess);
    return result;

LError:
    *poSuccess = false;
    return 0;
}

internal f64
ParseFloat(Scanner * scanner, PrimitiveDataType dataType, bool * poSuccess)
{
    // NOTE - Returns s64 for convenience, with the expectation that the caller will cast
    //  to the appropriately sized float. If we don't report an error, the caller does not
    //  need to check for overflow.
    
    Assert(dataType >= PrimitiveDataType::Float32  && dataType <= PrimitiveDataType::Float64);
    
    bool isNegated = false;
    if (MatchNextToken(scanner, TokenType::Plus))
    {
        // No-op
    }
    else if (MatchNextToken(scanner, TokenType::Minus))
    {
        isNegated = true;
    }

    f64 result = 0;
    if (MatchPeekToken(scanner, TokenType::DecimalFloatLiteral))
    {
        result = ParseDecimalFloatLiteral(scanner, poSuccess);
    }
    else if (MatchPeekToken(scanner, TokenType::DecimalLiteral))
    {
        result = (f64)ParseDecimalLiteral(scanner, poSuccess);
    }
    else if (MatchPeekToken(scanner, TokenType::HexLiteral))
    {
        u64 hex = ParseHexLiteral(scanner, poSuccess);
        result = *(f64 *)&hex;
    }
    else if (MatchPeekToken(scanner, TokenType::BinaryLiteral))
    {
        u64 binary = ParseBinaryLiteral(scanner, poSuccess);
        result = *(f64 *)&binary;
    }
    else if (MatchPeekToken(scanner, TokenType::OctalLiteral))
    {
        AssertTodo; // Currently unsupported
        // result = (s64)ParseOctalLiteral(scanner, poSuccess);
    }
    else
    {
        goto LError;
    }
    
    if (!poSuccess)
        goto LError;

    if (isNegated)
    {
        result = -result;
    }

    // NOTE - 64 bit overflow already checked by the Parse___ function we called into
    if (dataType == PrimitiveDataType::Float32 && (result > F32::max || result < -F32::max))
        goto LError;

    Assert(*poSuccess);
    return result;

LError:
    *poSuccess = false;
    return 0;
}

internal String
ParseName(Scanner * scanner)
{
    Token matched;
    if (!MatchNextToken(scanner, TokenType::Name, &matched))
        return {}; // TODO - better way to signal error?

    return matched.lexeme;
}

internal String
ParseIdentifier(Scanner * scanner)
{
    Token matched;
    if (!MatchNextToken(scanner, TokenType::Identifier, &matched))
        return {}; // TODO - better way to signal error?

    return matched.lexeme;
}

internal PrimitiveDataType
PrimitiveDataTypeFromReservedWordToken(Token token)
{
    if (token.type < TokenType::ReservedWordsTypesStart ||
        token.type >= TokenType::ReservedWordsTypesStart + TokenType::ReservedWordsTypesCount)
        return PrimitiveDataType::Nil;

    int iReservedWordType = (int)(token.type - TokenType::ReservedWordsTypesStart);
    PrimitiveDataType result = (PrimitiveDataType)(iReservedWordType + 1); // +1 to step over Nil
    return result;
}

internal PrimitiveDataType
ParsePrimitiveDataType(Scanner * scanner)
{
    Token token = PeekToken(scanner);
    PrimitiveDataType result = PrimitiveDataTypeFromReservedWordToken(token);
    if (result != PrimitiveDataType::Nil)
    {
        NextToken(scanner);
    }

    return result;
}

internal RefValue
ParseRef(Scanner * scanner)
{
    RefValue result = {};
    
    Token matched;
    if (!MatchNextToken(scanner, TokenType::Null, &matched) &&
        !MatchNextToken(scanner, TokenType::Name, &matched) &&
        !MatchNextToken(scanner, TokenType::NamePath, &matched))
        goto LError;

    result.refString = matched.lexeme;
    return result;
    
LError:
    result = {};
    return result;
}

internal int
cBytesFromPrimitiveDataType(PrimitiveDataType dataType)
{
    Assert(IsEnumValid(dataType));
    
    int mapTypeTocBytes[] = {
        0,                         // Nil
        sizeof(bool),              // Bool
        sizeof(s8),                // Int8
        sizeof(s16),               // Int16
        sizeof(s32),               // Int32
        sizeof(s64),               // Int64
        sizeof(u8),                // UInt8
        sizeof(u16),               // UInt16
        sizeof(u32),               // UInt32
        sizeof(u64),               // UInt64
        sizeof(f32),               // Float32
        sizeof(f64),               // Float64
        sizeof(String),            // String
        sizeof(RefValue),          // Ref
        sizeof(PrimitiveDataType), // Type
    };
    StaticAssert(ArrayLen(mapTypeTocBytes) == (int)PrimitiveDataType::EnumCount);

    return mapTypeTocBytes[(int)dataType];
}

internal PrimitiveValue
ParsePrimitiveValue(Scanner * scanner, MemoryRegion memory, PrimitiveDataType dataType)
{
    PrimitiveValue result = {};
    result.type = dataType;

    switch (result.type)
    {
        case PrimitiveDataType::Bool:
        {
            bool success;
            result.as.boolValue = ParseBool(scanner, &success);
            if (!success)
                goto LError;
        } break;

        case PrimitiveDataType::Int8:
        case PrimitiveDataType::Int16:
        case PrimitiveDataType::Int32:
        case PrimitiveDataType::Int64:
        {
            bool success;
            s64 value64 = ParseInt(scanner, dataType, &success);
            if (!success)
                goto LError;

            switch ((int)dataType)
            {
                case PrimitiveDataType::Int8:  result.as.i8Value  = (s8)value64;  break;
                case PrimitiveDataType::Int16: result.as.i16Value = (s16)value64; break;
                case PrimitiveDataType::Int32: result.as.i32Value = (s32)value64; break;
                case PrimitiveDataType::Int64: result.as.i64Value = (s64)value64; break;
                DefaultInvalid;
            }
        } break;

        case PrimitiveDataType::UInt8:
        case PrimitiveDataType::UInt16:
        case PrimitiveDataType::UInt32:
        case PrimitiveDataType::UInt64:
        {
            bool success;
            u64 value64 = ParseUInt(scanner, dataType, &success);
            if (!success)
                goto LError;

            switch ((int)dataType)
            {
                case PrimitiveDataType::UInt8:  result.as.u8Value  = (u8)value64;  break;
                case PrimitiveDataType::UInt16: result.as.u16Value = (u16)value64; break;
                case PrimitiveDataType::UInt32: result.as.u32Value = (u32)value64; break;
                case PrimitiveDataType::UInt64: result.as.u64Value = (u64)value64; break;
                DefaultInvalid;
            }
        } break;

        case PrimitiveDataType::Float32:
        case PrimitiveDataType::Float64:
        {
            // NOTE - This relies on fact that we can losslessly convert f32 -> f64 -> f32
                
            bool success;
            f64 value64 = ParseFloat(scanner, dataType, &success);
            if (!success)
                goto LError;

            switch ((int)dataType)
            {
                case PrimitiveDataType::Float32:  result.as.f32Value = (f32)value64; break;
                case PrimitiveDataType::Float64:  result.as.f64Value = (f64)value64; break;
                DefaultInvalid;
            }
        } break;

        case PrimitiveDataType::String:
        {
            result.as.stringValue = ParseStringLiteral(scanner, memory);
            if (!result.as.stringValue.bytes)
                goto LError;
        } break;

        case PrimitiveDataType::Ref:
        {
            result.as.refValue = ParseRef(scanner);
            if (!result.as.refValue.refString.bytes) // NOTE - the ref string will literally be "null" in the case of a null ref
                goto LError;
        } break;

        case PrimitiveDataType::Type:
        {
            result.as.typeValue = ParsePrimitiveDataType(scanner);
            if (result.as.typeValue == PrimitiveDataType::Nil)
                goto LError;
        } break;

        case PrimitiveDataType::Nil:       // satisfy compiler
        case PrimitiveDataType::EnumCount: // ...
        default:
            goto LError;
    }

    return result;

LError:
    result.type = PrimitiveDataType::Nil;
    return result;
}

internal bool
ParseDataListKnownLength(
    Scanner * scanner,
    MemoryRegion memory, // for intermediate per-value allocations
    PrimitiveDataType dataType,
    int cntValueExpected,
    u8 * poResult)       // output buffer size must be cntValueExpected * sizeof( the actual data type )
{
#define SetValueInResult(TYPE, INDEX, VALUE) ((TYPE *)poResult)[(INDEX)] = (VALUE);
    
    for (int iValue = 0; iValue < cntValueExpected; iValue++)
    {
        // @Slow - Repeatedly switching inside of a loop on a condition that doesn't change
        //  (ParsePrimitiveValue also does a switch!)

        PrimitiveValue value = ParsePrimitiveValue(scanner, memory, dataType);
        if (value.type != dataType)
        {
            Assert(value.type == PrimitiveDataType::Nil);
            goto LError;
        }

        switch (dataType)
        {
            case PrimitiveDataType::Bool:    SetValueInResult(bool,              iValue, value.as.boolValue);   break;
            case PrimitiveDataType::Int8:    SetValueInResult(s8,                iValue, value.as.i8Value);     break;
            case PrimitiveDataType::Int16:   SetValueInResult(s16,               iValue, value.as.i16Value);    break;
            case PrimitiveDataType::Int32:   SetValueInResult(s32,               iValue, value.as.i32Value);    break;
            case PrimitiveDataType::Int64:   SetValueInResult(s64,               iValue, value.as.i64Value);    break;
            case PrimitiveDataType::UInt8:   SetValueInResult(u8,                iValue, value.as.u8Value);     break;
            case PrimitiveDataType::UInt16:  SetValueInResult(u16,               iValue, value.as.u16Value);    break;
            case PrimitiveDataType::UInt32:  SetValueInResult(u32,               iValue, value.as.u32Value);    break;
            case PrimitiveDataType::UInt64:  SetValueInResult(u64,               iValue, value.as.u64Value);    break;
            case PrimitiveDataType::Float32: SetValueInResult(f32,               iValue, value.as.f32Value);    break;
            case PrimitiveDataType::Float64: SetValueInResult(f64,               iValue, value.as.f64Value);    break;
            case PrimitiveDataType::String:  SetValueInResult(String,            iValue, value.as.stringValue); break;
            case PrimitiveDataType::Ref:     SetValueInResult(RefValue,          iValue, value.as.refValue);    break;
            case PrimitiveDataType::Type:    SetValueInResult(PrimitiveDataType, iValue, value.as.typeValue);   break;
            DefaultNilInvalidEnum(PrimitiveDataType);
        }

        if (iValue != cntValueExpected - 1)
        {
            if (!MatchNextToken(scanner, TokenType::Comma))
                goto LError;
        }
    }

    return true;

LError:
    return false;
    
#undef SetValueInResult
}

// @CopyPaste - mostly the same as ParseDataListKnownLength
internal DataListUnknownLength
ParseDataListUnknownLength(
    Scanner * scanner,
    MemoryRegion memory,
    PrimitiveDataType dataType)
{
    DataListUnknownLength result = {};
    BufferBuilder builder(memory);
    
    int cBytesPerValue = cBytesFromPrimitiveDataType(dataType);
    
    while ((result.cntValue == 0) || MatchNextToken(scanner, TokenType::Comma))
    {
        // @Slow - Repeatedly switching inside of a loop on a condition that doesn't change
        //  (ParsePrimitiveValue also does a switch!)

        PrimitiveValue value = ParsePrimitiveValue(scanner, memory, dataType);
        if (value.type != dataType)
        {
            Assert(value.type == PrimitiveDataType::Nil);
            goto LError;
        }

        void * bytes = AppendNewBytes(&builder, cBytesPerValue);
        switch (dataType)
        {
            case PrimitiveDataType::Bool:    *(bool *)bytes =              value.as.boolValue;   break;
            case PrimitiveDataType::Int8:    *(s8 *)bytes =                value.as.i8Value;     break;
            case PrimitiveDataType::Int16:   *(s16 *)bytes =               value.as.i16Value;    break;
            case PrimitiveDataType::Int32:   *(s32 *)bytes =               value.as.i32Value;    break;
            case PrimitiveDataType::Int64:   *(s64 *)bytes =               value.as.i64Value;    break;
            case PrimitiveDataType::UInt8:   *(u8 *)bytes =                value.as.u8Value;     break;
            case PrimitiveDataType::UInt16:  *(u16 *)bytes =               value.as.u16Value;    break;
            case PrimitiveDataType::UInt32:  *(u32 *)bytes =               value.as.u32Value;    break;
            case PrimitiveDataType::UInt64:  *(u64 *)bytes =               value.as.u64Value;    break;
            case PrimitiveDataType::Float32: *(f32 *)bytes =               value.as.f32Value;    break;
            case PrimitiveDataType::Float64: *(f64 *)bytes =               value.as.f64Value;    break;
            case PrimitiveDataType::String:  *(String *)bytes =            value.as.stringValue; break;
            case PrimitiveDataType::Ref:     *(RefValue *)bytes =          value.as.refValue;    break;
            case PrimitiveDataType::Type:    *(PrimitiveDataType *)bytes = value.as.typeValue;   break;
            DefaultNilInvalidEnum(PrimitiveDataType);
        }
        
        result.cntValue++;
    }

    result.values = RawPtr(&builder);
    return result;
    
LError:
    
    result.cntValue = -1;
    return result;
}

internal DataArrayList
ParseDataArrayList(
    Scanner * scanner,
    MemoryRegion memory,
    PrimitiveDataType dataType,
    int cntValuePerSubarray,
    PrimitiveStructureType structureType)
{
    Assert(structureType == PrimitiveStructureType::Subarray || structureType == PrimitiveStructureType::SubarrayWithStates);

    DataArrayList result = {};

    DynArray<String> stateNames(memory);
    BufferBuilder valueBuilder(memory);
    int cBytesPerValue = cBytesFromPrimitiveDataType(dataType);
    
    //
    // Subarrays

    String state = {};
    while ((result.cntSubarray == 0) || MatchNextToken(scanner, TokenType::Comma))
    {
        if (!MatchPeekToken(scanner, TokenType::LBrace) && structureType == PrimitiveStructureType::SubarrayWithStates)
        {
            state = ParseIdentifier(scanner);
            if (!state.bytes)
                goto LError;
        }

        if (!MatchNextToken(scanner, TokenType::LBrace))
            goto LError;

        u8 * dataListBuffer = AppendNewBytes(&valueBuilder, cBytesPerValue * cntValuePerSubarray);
        if (!ParseDataListKnownLength(scanner, memory, dataType, cntValuePerSubarray, dataListBuffer))
            goto LError;

        if (!MatchNextToken(scanner, TokenType::RBrace))
            goto LError;

        Append(&stateNames, state);
        result.cntSubarray++;
    }

    Assert(stateNames.count == result.cntSubarray);
    Assert(valueBuilder.bytes.count == result.cntSubarray * cBytesPerValue * cntValuePerSubarray);
    
    result.states = RawPtr(&stateNames);
    result.values = RawPtr(&valueBuilder);
    return result;
    
LError:
    result.cntSubarray = -1;
    return result;
}

internal bool
ParsePrimitiveStructure(Scanner * scanner, Structure * poStructure, MemoryRegion memory)
{
    poStructure->type = StructureType::OpenDdlPrimitive;
    PrimitiveStructure * primitive = &poStructure->primitive;
    
    primitive->dataType = ParsePrimitiveDataType(scanner);
    if (primitive->dataType == PrimitiveDataType::Nil)
        goto LError;
    
    if (MatchNextToken(scanner, TokenType::LBracket))
    {
        // Subarray (with or without states)
            
        bool success;
        uint cntValuePerSubarray = (uint)ParseUInt(scanner, PrimitiveDataType::UInt32, &success);
        if (!success)
            goto LError;
        
        if (!MatchNextToken(scanner, TokenType::RBracket))
            goto LError;

        if (MatchNextToken(scanner, TokenType::Star))
        {
            primitive->primitiveType = PrimitiveStructureType::SubarrayWithStates;
        }
        else
        {
            primitive->primitiveType = PrimitiveStructureType::Subarray;
        }

        // Name (optional)
        
        poStructure->name = ParseName(scanner);

        if (!MatchNextToken(scanner, TokenType::LBrace))
            goto LError;

        if (!MatchPeekToken(scanner, TokenType::RBrace))
        {
            DataArrayList dataArrayList = ParseDataArrayList(scanner, memory, primitive->dataType, cntValuePerSubarray, primitive->primitiveType);
            if (dataArrayList.cntSubarray < 0)
                goto LError;

            primitive->cntValue = cntValuePerSubarray * dataArrayList.cntSubarray;
            primitive->values = dataArrayList.values;
            
            if (primitive->primitiveType == PrimitiveStructureType::Subarray)
            {
                primitive->subarray.cntValuePerSubarray = cntValuePerSubarray;
                primitive->subarray.cntSubarray = dataArrayList.cntSubarray;
            }
            else
            {
                Assert(primitive->primitiveType == PrimitiveStructureType::SubarrayWithStates);
                primitive->subarrayWithStates.cntValuePerSubarray = cntValuePerSubarray;
                primitive->subarrayWithStates.cntSubarray = dataArrayList.cntSubarray;
                primitive->subarrayWithStates.states = dataArrayList.states;
            }
        }

        if (!MatchNextToken(scanner, TokenType::RBrace))
            goto LError;
    }
    else
    {
        // Flat

        primitive->primitiveType = PrimitiveStructureType::Flat;
        
        // Name (optional)
        
        poStructure->name = ParseName(scanner);

        if (!MatchNextToken(scanner, TokenType::LBrace))
            goto LError;

        if (!MatchPeekToken(scanner, TokenType::RBrace))
        {
            DataListUnknownLength dataList = ParseDataListUnknownLength(scanner, memory, primitive->dataType);
            if (dataList.cntValue < 0)
                goto LError;

            primitive->cntValue = dataList.cntValue;
            primitive->values = dataList.values;
        }

        if (!MatchNextToken(scanner, TokenType::RBrace))
            goto LError;
    }

    // Success!

    return true;

LError:
    *poStructure = {};
    return false;
}

internal Property
ParseProperty(Scanner * scanner, MemoryRegion memory, DerivedMappings * derivedMappings)
{
    Property result = {};

    result.nameString = ParseIdentifier(scanner);
    if (!result.nameString.bytes)
        goto LError;

    // Map to a particular propperty name
    
    result.name = PropertyName::UnrecognizedExtension;
    result.valueType = PrimitiveDataType::Nil;

    if (AreStringsEqual(result.nameString, "attrib"))
    {
        bool brk = true;
    }
    
    for (int iMapping = 0; iMapping < derivedMappings->cntPropertyMapping; iMapping++)
    {
        DerivedPropertyMapping mapping = derivedMappings->propertyMappings[iMapping];
        if (AreStringsEqual(mapping.nameString, result.nameString))
        {
            result.name = (PropertyName)mapping.name;
            result.valueType = mapping.valueType;
            break;
        }
    }

    // Parse value
    
    if (result.valueType == PrimitiveDataType::Nil)
    {
        // Mapping uspecified (or we don't recognize this property)
        
        if (MatchNextToken(scanner, TokenType::Equal))
        {
            TokenType propertyTokenType = PeekToken(scanner).type;
            switch ((int)propertyTokenType)
            {
                case TokenType::StringLiteral:
                case TokenType::DecimalLiteral:
                case TokenType::HexLiteral:
                case TokenType::OctalLiteral:
                case TokenType::BinaryLiteral:
                case TokenType::CharLiteral:
                case TokenType::DecimalFloatLiteral:
                case TokenType::Name:
                case TokenType::NamePath:
                    break; // Ok

                default:
                {
                    if (propertyTokenType >= TokenType::ReservedWordsTypesStart &&
                        propertyTokenType <= TokenType::ReservedWordsTypesCount)
                    {
                        // Ok
                    }
                    else
                    {
                        goto LError;
                    }
                } break;

            }

            result.rawToken = NextToken(scanner);
        }
        else
        {
            // Permitted to omit '='. Derived type presumably treats this is a bool, but since
            //  they didn't provide a mapping, we won't explicitly tell them it's a bool.
            
            Assert(result.rawToken.type == TokenType::Nil);
        }
    }
    else if (result.valueType == PrimitiveDataType::Bool)
    {
        // Maps to bool ('=' optional)
        
        if (MatchNextToken(scanner, TokenType::Equal))
        {
            bool success;
            result.as.boolValue = ParseBool(scanner, &success);
            if (!success)
                goto LError;
        }
        else
        {
            result.as.boolValue = true;
        }
    }
    else
    {
        // Maps to non-bool ('=' mandatory)
        
        if (!MatchNextToken(scanner, TokenType::Equal))
            goto LError;

        PrimitiveValue value = ParsePrimitiveValue(scanner, memory, result.valueType);
        if (value.type != result.valueType)
        {
            Assert(value.type == PrimitiveDataType::Nil);
            goto LError;
        }

        result.as = value.as;
    }

    return result;
    
LError:
    result = {};
    return result;
}

internal bool
ParseDerivedStructure(Scanner * scanner, Structure * poStructure, StructureId * idNext, MemoryRegion memory, DerivedMappings * derivedMappings)
{
    DerivedStructure * derived = &poStructure->derived;
    derived->derivedType = ParseIdentifier(scanner);
    if (!derived->derivedType.bytes)
        goto LError;

    //
    // Determine type

    poStructure->type = StructureType::UnrecognizedExtension;
    for (int iMapping = 0; iMapping < derivedMappings->cntTypeMapping; iMapping++)
    {
        DerivedTypeMapping  mapping = derivedMappings->typeMappings[iMapping];
        if (AreStringsEqual(derived->derivedType, mapping.identifier))
        {
            poStructure->type = (StructureType)mapping.derivedType;
            break;
        }
    }

    //
    // Name (optional)
     
    poStructure->name = ParseName(scanner);

    //
    // Properties

    { // @Hack - scope to avoid goto warning. @Cleanup the gotos!
        
        DynArray<Property> properties = {};
        properties.memory = memory;
    
        if (MatchNextToken(scanner, TokenType::LParen))
        {
            while (!MatchNextToken(scanner, TokenType::RParen))
            {
                if (!IsEmpty(properties) && !MatchNextToken(scanner, TokenType::Comma))
                    goto LError;

                Property property = ParseProperty(scanner, memory, derivedMappings);
                if (property.name == PropertyName::Nil)
                    goto LError;

                if (derivedMappings->skipUnrecognizedProperties && property.name == PropertyName::UnrecognizedExtension)
                    continue;

                Append(&properties, property);
            }
        }
        
        derived->properties = MakeSlice(properties);
    }
    
    if (!MatchNextToken(scanner, TokenType::LBrace))
        goto LError;
    
    //
    // Children
    // NOTE - Children's "parent" pointers are set after parsing, once we know where all the nodes are
    //  located in memory

    { // @Hack - scope to avoid goto warning. @Cleanup the gotos!
        
        DynArray<Structure> children = {};
        children.memory = memory;
    
        while (!MatchNextToken(scanner, TokenType::RBrace))
        {
            Structure child = ParseStructure(scanner, idNext, memory, derivedMappings);
            if (child.type == StructureType::Nil)
                goto LError;

            Append(&children, child);
        }
    
        derived->children = MakeSlice(children);
    }
    
    return true;

LError:
    *poStructure = {};
    return false;
}

internal Structure
ParseStructure(Scanner * scanner, StructureId * idNext, MemoryRegion memory, DerivedMappings * derivedMappings)
{
    Structure result = {};
        
    Token peek = PeekToken(scanner);
    PrimitiveDataType primitiveDataType = PrimitiveDataTypeFromReservedWordToken(peek);
    if (primitiveDataType == PrimitiveDataType::Nil)
    {
        if (!ParseDerivedStructure(scanner, &result, idNext, memory, derivedMappings))
            goto LError;
    }
    else
    {
        if (!ParsePrimitiveStructure(scanner, &result, memory))
            goto LError;
    }

    result.id = *idNext;
    (*idNext)++;
    
    return result;

LError:
    result.type = StructureType::Nil;
    return result;
}
