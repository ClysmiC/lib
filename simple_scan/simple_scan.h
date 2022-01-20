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

struct StringScanner
{
	String contents;
	int iByte;
	int iLine;
};

internal void
InitScanner(StringScanner * scanner, String contents)
{
	scanner->contents = contents;
	scanner->iByte = 0;
	scanner->iLine = 0;
}
