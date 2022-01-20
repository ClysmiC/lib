#pragma once

// Spec:
// - https://openddl.org/openddl-spec.pdf
// - https://openddl.org/openddl.pdf

#include "scan.h"
#include "parse.h"

//
// Metadata/mappings provided by derivative type
//

enum class StructureType : u32
{
    // The idea here is that the derivative importer should have a "type" enum that it exposes
    //  to OpenDDL via DerivedTypeMappings. The first few values in that enum should be shared
    //  with the values below to allow meaningful communication from the OpenDDL layer
    
    // Derived type enum should start by duplicating these:
    
    Nil = 0,
    
    OpenDdlPrimitive      = 1,
    UnrecognizedExtension = 2, // Any type name without a mapping is considered unrecognized, and won't be in the result

    // The first recognized type in the derived type enum should start here:
    
    DerivedTypeStart = 3
};

inline bool IsDerivedType(StructureType type)
{
    bool result = (type >= StructureType::UnrecognizedExtension);
    return result;
}

inline bool IsRecognizedDerivedType(StructureType type)
{
    bool result = (type >= StructureType::DerivedTypeStart);
    return result;
}

struct DerivedTypeMapping
{
    // Map a type name to a user-specified Type enum in the derived format (e.g., OPENGEX::StructureType)
    
    String identifier;
    u32 derivedType;   // Corresponds to StructureType
};

//
// Outputs from importer
//

enum class PrimitiveDataType : u8
{
    // @Sync - Order of TokenType reserved words for types
    
    Nil = 0,
    
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
    
    EnumCount
};
DefineEnumOps(PrimitiveDataType, u8);

enum class PrimitiveStructureType : u8
{
    Nil = 0,

    Flat,
    Subarray,
    SubarrayWithStates,

    EnumCount
};

struct Subarray
{
    int cntValuePerSubarray; // aka subarray length, the value in the brackets []
    int cntSubarray;         // number of subarrays
};

struct SubarrayWithStates
{
    int cntValuePerSubarray; // aka subarray length, the value in the brackets []
    int cntSubarray;
    String * states;          // length = cntSubarray
};

struct PrimitiveStructure
{
    PrimitiveStructureType primitiveType;
    PrimitiveDataType dataType;

    // TODO - Use byte buffer?
    int cntValue; // Total value count. For subarrays, this is cntValuePerSubarray * cntSubarray.
    void * values; // Tightly packed (subarrays are tightly packed too)
    
    union
    {
        Subarray subarray;
        SubarrayWithStates subarrayWithStates;
    };
};

enum class PropertyName : u32
{
    // This functions pretty much the same way as StructureType

    // Derived property enum should start by duplicating these:
    
    Nil = 0,

    UnrecognizedExtension = 1,

    // The first recognized property in the derived property enum should start here:

    DerivedPropertyStart = 2
};

// NOTE - We assume that a given property name will always be of a given type, regardless of which
//  derived structure it appears in. OpenDDL doesn't require this, but it is true for all OpenGEX
//  properties. If we find that we need a property mapping to be more context-sensitive, we will cross
//  that bridge when we come to it :)

struct DerivedPropertyMapping
{
    String nameString;
    u32 name;                    // Corresponds to PropertyName
    PrimitiveDataType valueType; // Nil = don't type check (since type checking properties isn't strictly OpenDDL's responsibility)
};

struct DerivedMappings
{
    int cntTypeMapping;
    const DerivedTypeMapping * typeMappings;
    bool skipUnrecognizedTypes;

    int cntPropertyMapping;
    const DerivedPropertyMapping * propertyMappings;
    bool skipUnrecognizedProperties;
};

struct RefValue
{
    String refString;   // Not strictly necessary to keep around once we've resolved the pointer, but it may be convenient for the user
    struct Structure * refPtr;
};

union RawPrimitiveValue
{
    bool              boolValue;
    s8                i8Value;
    s16               i16Value;
    s32               i32Value;
    s64               i64Value;
    u8                u8Value;
    u16               u16Value;
    u32               u32Value;
    u64               u64Value;
    f32               f32Value;
    f64               f64Value;
    String            stringValue;
    RefValue          refValue;
    PrimitiveDataType typeValue;
};

struct PrimitiveValue
{
    PrimitiveDataType type;
    RawPrimitiveValue as;
};

struct Property
{
    PropertyName name;
    String nameString;
    
    PrimitiveDataType valueType; // from DerivedPropertyMapping
    union
    {
        Token rawToken;    // PrimitiveDataType::Nil
        RawPrimitiveValue as;
    };
};

struct DerivedStructure
{
    String derivedType;
    Slice<Structure> children;
    Slice<Property> properties;
};

enum class StructureId : u32
{
    Nil = 0,
    EnumCount = U32::max,
};
DefineEnumOps(StructureId, u32);

struct Structure
{
    StructureId id;
    StructureType type;
    union
    {
        PrimitiveStructure primitive;
        DerivedStructure derived;
    };

    String name; // optional
    Structure * parent;
};

//
// Convenience utilities for derived file formats to give us a little bit of metadata and let us handle the
//  scanning of lists of structures/properties while silently ignoring ones that are unrecognized
//

struct StructureListReader
{
    Slice<Structure> structures;
    int iNext;
};

inline StructureListReader
CreateStructureListReader(Slice<Structure> structures)
{
    StructureListReader result;
    result.structures = structures;
    result.iNext = 0;
    return result;
}

inline Structure *
PeekStructure(StructureListReader * reader)
{
    if (reader->iNext >= reader->structures.count)
        return nullptr;
    
    Structure * result = reader->structures.items + reader->iNext;
    return result;
}

inline Structure *
NextStructure(StructureListReader * reader)
{
    if (reader->iNext >= reader->structures.count)
        return nullptr;
    
    Structure * result = reader->structures.items + reader->iNext;
    reader->iNext++;
    return result;
}

inline Structure *
MatchPeekStructure(StructureListReader * reader, StructureType type)
{
    Structure * result = PeekStructure(reader);
    if (result && result->type == type)
        return result;

    return nullptr;
}

inline Structure *
MatchNextStructure(StructureListReader * reader, StructureType type)
{
    Structure * result = PeekStructure(reader);
    if (result && result->type == type)
    {
        reader->iNext++;
        return result;
    }

    return nullptr;
}

struct PropertyListReader
{
    Slice<Property> properties;
    int iNext;
};

inline PropertyListReader
CreatePropertyListReader(Slice<Property> properties)
{
    PropertyListReader result;
    result.iNext = 0;
    result.properties = properties;
    return result;
}

Property *
PeekProperty(PropertyListReader * reader)
{
    if (reader->iNext >= reader->properties.count)
        return nullptr;
    
    Property * result = reader->properties + reader->iNext;
    return result;
}

Property *
NextProperty(PropertyListReader * reader)
{
    if (reader->iNext >= reader->properties.count)
        return nullptr;
    
    Property * result = reader->properties + reader->iNext;
    reader->iNext++;
    return result;
}

struct OpenDdlResult
{
    int cntStructureTotal; // StructureId's range from [1, 1 + structureIdCount)
    Slice<Structure> topLevel;
};
