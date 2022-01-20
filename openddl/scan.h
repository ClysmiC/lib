namespace SCAN
{
static constexpr char eof = '\0';
static constexpr char numberSeparator = '_';
}

enum class TokenType : u8
{
    Nil = 0,

    Eof,
    
    // NOTE - literals don't include preceding signs
    StringLiteral,
    DecimalLiteral,
    HexLiteral,
    OctalLiteral,
    BinaryLiteral,
    CharLiteral,
    DecimalFloatLiteral,
    
    Name,
    NamePath, // NOTE - Only for paths with >= 2 names. If path is just a single name, it'll scan as a Name.
    Identifier,

    LParen,
    RParen,
    LBrace,
    RBrace,
    LBracket,
    RBracket,
    
    Equal,
    Star,
    Comma,
    Plus,
    Minus,

    // Reserved words
    
    True,
    False,
    Null,

    // Reserved words for types. Aliases map to the corresponding type as you'd expect.
    // @Sync - Order of PrimitiveDataType
    
    Bool,
    Int8,
    Int16,
    Int32,
    Int64,
    UInt8,
    UInt16,
    UInt32,
    UInt64,
    // Float16,
    Float32,
    Float64,
    String,
    Ref,
    Type,
    // Base64,

    EnumCount,

    ReservedWordsStart = True,
    ReservedWordsCount = EnumCount - ReservedWordsStart,

    ReservedWordsTypesStart = Bool,
    ReservedWordsTypesCount = EnumCount - ReservedWordsTypesStart,
};
DefineEnumOps(TokenType, u8);

struct Token
{
    TokenType type;
    String lexeme;
};

struct Scanner
{
    SimpleScan::ZStringScanner s;
    
    struct CachedPeek
    {
        Token token;
        int iByteAfterPeek;
        int iLineAfterPeek;
    };
    
    CachedPeek peeked;
};
