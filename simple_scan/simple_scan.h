#pragma once

struct ZStringScanner
{
    char * contents;
    int iByte;
    int iLine;
};

internal void
InitScanner(ZStringScanner * scanner, char * contents)
{
    scanner->contents = contents;
    scanner->iByte = 0;
    scanner->iLine = 0;
}

enum class Endianness : u8
{
    Nil = 0,
    Little = Nil,

    Big
};

struct StringScanner
{
    String contents;
    int iByte;
    int iLine;

    Endianness endianness;
};

internal void
InitScanner(StringScanner * scanner, String contents)
{
    scanner->contents = contents;
    scanner->iByte = 0;
    scanner->iLine = 0;
}

namespace SimpleScan
{

// TODO - come up with a better solution that would support NUL bytes...
//  they are are totally valid in byte streams without necessarily ending the stream...
static constexpr char end = '\0';
}

