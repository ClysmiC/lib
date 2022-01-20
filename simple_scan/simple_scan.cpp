#pragma once

// --- Module Dependencies

#include "core/core.h"

// --- Module Implementation


namespace SimpleScan
{

#include "_generated.h"
#include "simple_scan.h"
#include "_generated.cpp"

inline bool
IsWhitespace(char c)
{
	// Entire range [1-32] is considered whitespace. This is a convenient model, borrowed from OpenDDL spec.

	bool result = (c >= 1 && c <= 32);
	return result;
}

inline char
PeekChar(ZStringScanner * scanner)
{
	char result = scanner->contents[scanner->iByte];
	return result;
}

inline char
PeekChar(ZStringScanner * scanner, int cBytesLookahead)
{
	cBytesLookahead = Max(0, cBytesLookahead);
	for (int iOffset = 0; iOffset < cBytesLookahead; iOffset++)
	{
		char c = scanner->contents[scanner->iByte + iOffset];
		if (!c)
			return c;
	}

	char result = scanner->contents[scanner->iByte + cBytesLookahead];
	return result;
}

inline bool
HasNextChar(ZStringScanner * scanner)
{
	bool result = PeekChar(scanner);
	return result;
}

internal char
NextChar(ZStringScanner * scanner)
{
	char result = PeekChar(scanner);
	if (result == '\n')
	{
		scanner->iLine++;
	}

	if (result)
	{
		scanner->iByte++;
	}

	return result;
}

inline bool
MatchNextChar(ZStringScanner * scanner, char c)
{
	if (PeekChar(scanner) == c)
	{
		NextChar(scanner);
		return true;
	}

	return false;
}

inline String
ConsumeUntil(ZStringScanner * scanner, char * candidates, int cntCandidate)
{
	String result;
	result.bytes = scanner->contents + scanner->iByte;
	result.cBytes = 0;

	while (HasNextChar(scanner))
	{
		char c = PeekChar(scanner);
		for (int iCandidate = 0; iCandidate < cntCandidate; iCandidate++)
		{
			char candidate = candidates[iCandidate];
			if (c == candidate)
				goto LDone;
		}

		NextChar(scanner);
		result.cBytes++;
	}

LDone:
	return result;
}

inline String
ConsumeUntil(ZStringScanner * scanner, char c)
{
	String result = ConsumeUntil(scanner, &c, 1);
	return result;
}

inline String
ConsumePast(ZStringScanner * scanner, char * candidates, int cntCandidate)
{
	String result = ConsumeUntil(scanner, candidates, cntCandidate);

	if (HasNextChar(scanner))
	{
		NextChar(scanner);
		result.cBytes++;
	}

	return result;
}

inline String
ConsumePast(ZStringScanner * scanner, char c)
{
	String result = ConsumePast(scanner, &c, 1);
	return result;
}

inline String
ConsumeWhitespace(ZStringScanner * scanner)
{
	String result;
	result.bytes = scanner->contents + scanner->iByte;
	result.cBytes = 0;

	while (HasNextChar(scanner) && IsWhitespace(PeekChar(scanner)))
	{
		NextChar(scanner);
		result.cBytes++;
	}

	return result;
}

inline String
ConsumeUntilWhitespace(ZStringScanner * scanner)
{
	String result;
	result.bytes = scanner->contents + scanner->iByte;
	result.cBytes = 0;

	while (HasNextChar(scanner))
	{
		char c = PeekChar(scanner);
		if (IsWhitespace(c))
			break;

		NextChar(scanner);
		result.cBytes++;
	}

	return result;
}

inline bool
HasNextChar(StringScanner * scanner)
{
	bool result = scanner->iByte < scanner->contents.cBytes;
	return result;
}

inline int
CBytesRemaining(StringScanner * scanner)
{
	int result = scanner->contents.cBytes - scanner->iByte;
	return result;
}

inline char
PeekChar(StringScanner * scanner)
{
	if (!HasNextChar(scanner))
	{
		AssertFalseWarn;
		return '\0';
	}

	char result = scanner->contents.bytes[scanner->iByte];
	return result;
}

inline char
PeekChar(StringScanner * scanner, int cBytesLookahead)
{
	cBytesLookahead = Max(0, cBytesLookahead);

	int iByteTarget = scanner->iByte + cBytesLookahead;
	if (iByteTarget < 0 || iByteTarget >= scanner->contents.cBytes)
	{
		AssertFalseWarn;
		return 0;
	}
	char result = scanner->contents.bytes[iByteTarget];
	return result;
}

inline char
NextChar(StringScanner * scanner)
{
	if (!HasNextChar(scanner))
	{
		AssertFalseWarn;
		return 0;
	}

	char result = PeekChar(scanner);
	if (result == '\n')
	{
		scanner->iLine++;
	}

	scanner->iByte++;

	return result;
}

inline bool
MatchNextChar(StringScanner * scanner, char c)
{
	if (PeekChar(scanner) == c)
	{
		NextChar(scanner);
		return true;
	}

	return false;
}

inline String
ConsumeUntil(StringScanner * scanner, char * candidates, int cntCandidate)
{
	String result;
	result.bytes = scanner->contents.bytes + scanner->iByte;
	result.cBytes = 0;

	while (HasNextChar(scanner))
	{
		char c = PeekChar(scanner);
		for (int iCandidate = 0; iCandidate < cntCandidate; iCandidate++)
		{
			char candidate = candidates[iCandidate];
			if (c == candidate)
				goto LDone;
		}

		NextChar(scanner);
		result.cBytes++;
	}

LDone:
	return result;
}

inline String
ConsumeUntil(StringScanner * scanner, char c)
{
	String result = ConsumeUntil(scanner, &c, 1);
	return result;
}

inline String
ConsumePast(StringScanner * scanner, char * candidates, int cntCandidate)
{
	String result = ConsumeUntil(scanner, candidates, cntCandidate);

	if (HasNextChar(scanner))
	{
		NextChar(scanner);
		result.cBytes++;
	}

	return result;
}

inline String
ConsumePast(StringScanner * scanner, char c)
{
	String result = ConsumePast(scanner, &c, 1);
	return result;
}

inline String
ConsumeWhitespace(StringScanner * scanner)
{
	String result;
	result.bytes = scanner->contents.bytes + scanner->iByte;
	result.cBytes = 0;

	while (HasNextChar(scanner) && IsWhitespace(PeekChar(scanner)))
	{
		NextChar(scanner);
		result.cBytes++;
	}

	return result;
}

inline String
ConsumeUntilWhitespace(StringScanner * scanner)
{
	String result;
	result.bytes = scanner->contents.bytes + scanner->iByte;
	result.cBytes = 0;

	while (HasNextChar(scanner))
	{
		char c = PeekChar(scanner);
		if (IsWhitespace(c))
			break;

		NextChar(scanner);
		result.cBytes++;
	}

	return result;
}

#define SIMPLESCAN_DefineScannerWrappers(WRAPTYPE, SSMEMBER)     \
	inline char PeekChar(WRAPTYPE * scanner)                 { return PeekChar(&scanner->SSMEMBER); } \
	inline char PeekChar(WRAPTYPE * scanner, int cBytesLookahead) { return PeekChar(&scanner->SSMEMBER, cBytesLookahead); } \
	inline char NextChar(WRAPTYPE * scanner)                 { return NextChar(&scanner->SSMEMBER); } \
	inline char MatchNextChar(WRAPTYPE * scanner, char c)    { return MatchNextChar(&scanner->SSMEMBER, c); } \
	inline bool HasNextChar(WRAPTYPE * scanner)              { return HasNextChar(&scanner->SSMEMBER); } \
	inline String ConsumeUntil(WRAPTYPE * scanner, char * candidates, int cntCandidate) { return ConsumeUntil(&scanner->SSMEMBER, candidates, cntCandidate); } \
	inline String ConsumeUntil(WRAPTYPE * scanner, char c)   { return ConsumeUntil(&scanner->SSMEMBER, c); } \
	inline String ConsumePast(WRAPTYPE * scanner, char * candidates, int cntCandidate)  { return ConsumePast(&scanner->SSMEMBER, candidates, cntCandidate); } \
	inline String ConsumePast(WRAPTYPE * scanner, char c)    { return ConsumePast(&scanner->SSMEMBER, c); } \
	inline String ConsumeWhitespace(WRAPTYPE * scanner)      { return ConsumeWhitespace(&scanner->SSMEMBER); } \
	inline String ConsumeUntilWhitespace(WRAPTYPE * scanner) { return ConsumeUntilWhitespace(&scanner->SSMEMBER); }

} // namespace SimpleScan
