bool TestProperties(MemoryRegion memory)
{
    // @Cleanup - Hard-coded path
    char * file = ReadEntireFile("W:/RTS/code/openddl/tests/properties.oddl");
    DoTest(file);
    Defer(free(file));

    enum class CustomType : u32
    {
        Nil                   = StructureType::Nil,

        OpenDdlPrimitive      = StructureType::OpenDdlPrimitive,
        UnrecognizedExtension = StructureType::UnrecognizedExtension,

        Mesh                  = StructureType::DerivedTypeStart,
    };

    DerivedTypeMapping typeMappings[] = {
        { StringFromLit("Mesh"),  (u32)CustomType::Mesh },
    };

    enum class CustomProperty : u32
    {
        Nil                   = PropertyName::Nil,

        UnrecognizedExtension = PropertyName::UnrecognizedExtension,

        Lod                   = PropertyName::DerivedPropertyStart,
        Part,
        BoolProperty,
        OtherBoolProperty,
        IntProperty,
    };
    
    DerivedPropertyMapping propertyMappings[] = {
        { StringFromLit("lod"),               (u32)CustomProperty::Lod,               PrimitiveDataType::Int32 },
        { StringFromLit("part"),              (u32)CustomProperty::Part,              PrimitiveDataType::String },
        { StringFromLit("boolProperty"),      (u32)CustomProperty::BoolProperty,      PrimitiveDataType::Bool },
        { StringFromLit("otherBoolProperty"), (u32)CustomProperty::OtherBoolProperty, PrimitiveDataType::Bool },
        { StringFromLit("intProperty"),       (u32)CustomProperty::IntProperty,       PrimitiveDataType::Int32 },
    };
    
    DerivedMappings mappings;
    mappings.typeMappings = typeMappings;
    mappings.cntTypeMapping = ArrayLen(typeMappings);
    mappings.propertyMappings = propertyMappings;
    mappings.cntPropertyMapping = ArrayLen(propertyMappings);
    
    OpenDdlResult openDdl = ImportOpenDdlFileContents(file, memory, mappings);
    DoTest(openDdl.topLevel.count == 3);
    DoTest(openDdl.cntStructureTotal == 3);

    Slice<Structure> topLevel = openDdl.topLevel;

    {
        Structure structure = topLevel[0];
        DoTest((CustomType)structure.type == CustomType::Mesh);
        DoTest(structure.name.bytes == nullptr);
        
        DerivedStructure derived = structure.derived;
        DoTest(AreStringsEqual(derived.derivedType, "Mesh"));
        DoTest(derived.children.count == 0);
        DoTest(derived.properties.count == 2);

        Property * lod = derived.properties + 0;
        DoTest((CustomProperty)lod->name == CustomProperty::Lod);
        DoTest(AreStringsEqual(lod->nameString, "lod"));
        DoTest(lod->valueType == PrimitiveDataType::Int32);
        DoTest(lod->as.i32Value == 2);

        Property * part = derived.properties + 1;
        DoTest((CustomProperty)part->name == CustomProperty::Part);
        DoTest(AreStringsEqual(part->nameString, "part"));
        DoTest(part->valueType == PrimitiveDataType::String);
        DoTest(AreStringsEqual(part->as.stringValue, "Left Hand"));
    }

    {
        Structure structure = topLevel[1];
        DoTest((CustomType)structure.type == CustomType::Mesh);
        DoTest(structure.name.bytes == nullptr);
        
        DerivedStructure derived = structure.derived;
        DoTest(AreStringsEqual(derived.derivedType, "Mesh"));
        DoTest(derived.children.count == 0);
        DoTest(derived.properties.count == 0);
    }

    {
        Structure structure = topLevel[2];
        DoTest((CustomType)structure.type == CustomType::Mesh);
        DoTest(AreStringsEqual(structure.name, "$NamedMesh"));
        
        DerivedStructure derived = structure.derived;
        DoTest(AreStringsEqual(derived.derivedType, "Mesh"));
        DoTest(derived.children.count == 0);
        DoTest(derived.properties.count == 3);

        Property * boolProperty = derived.properties + 0;
        DoTest((CustomProperty)boolProperty->name == CustomProperty::BoolProperty);
        DoTest(AreStringsEqual(boolProperty->nameString, "boolProperty"));
        DoTest(boolProperty->valueType == PrimitiveDataType::Bool);
        DoTest(boolProperty->as.boolValue == true);

        Property * otherBoolProperty = derived.properties + 1;
        DoTest((CustomProperty)otherBoolProperty->name == CustomProperty::OtherBoolProperty);
        DoTest(AreStringsEqual(otherBoolProperty->nameString, "otherBoolProperty"));
        DoTest(otherBoolProperty->valueType == PrimitiveDataType::Bool);
        DoTest(otherBoolProperty->as.boolValue == true);
        
        Property * intProperty = derived.properties + 2;
        DoTest((CustomProperty)intProperty->name == CustomProperty::IntProperty);
        DoTest(AreStringsEqual(intProperty->nameString, "intProperty"));
        DoTest(intProperty->valueType == PrimitiveDataType::Int32);
        DoTest(intProperty->as.i32Value == 12);
    }
    
    AllTestsPass();
}
