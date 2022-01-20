bool TestRefGlobal(MemoryRegion memory)
{
    // @Cleanup - Hard-coded path
    char * file = ReadEntireFile("W:/RTS/code/openddl/tests/ref_global.oddl");
    DoTest(file);
    Defer(free(file));

    enum class CustomType : u32
    {
        Nil                   = StructureType::Nil,

        OpenDdlPrimitive      = StructureType::OpenDdlPrimitive,
        UnrecognizedExtension = StructureType::UnrecognizedExtension,

        Person                = StructureType::DerivedTypeStart,
        Name,
        Friends,
        ChildList
    };

    DerivedTypeMapping mappingArray[] = {
        { StringFromLit("Person"),    (u32)CustomType::Person },
        { StringFromLit("Name"),      (u32)CustomType::Name },
        { StringFromLit("Friends"),   (u32)CustomType::Friends },
        { StringFromLit("ChildList"), (u32)CustomType::ChildList },
    };

    DerivedMappings mappings = {};
    mappings.typeMappings = mappingArray;
    mappings.cntTypeMapping = ArrayLen(mappingArray);
    
    OpenDdlResult openDdl = ImportOpenDdlFileContents(file, memory, mappings);
    DoTest(openDdl.topLevel.count == 3);
    DoTest(openDdl.cntStructureTotal == 15);

    Slice<Structure> topLevel = openDdl.topLevel;

    Structure * alice = topLevel + 0;
    DoTest((CustomType)alice->type == CustomType::Person);
    DoTest(AreStringsEqual(alice->name, "$alice"));
    DoTest(AreStringsEqual(alice->derived.derivedType, "Person"));
    
    Structure * bob = topLevel + 1;
    DoTest((CustomType)bob->type == CustomType::Person);
    DoTest(AreStringsEqual(bob->name, "$bob"));
    DoTest(AreStringsEqual(bob->derived.derivedType, "Person"));
    
    Structure * charles = topLevel + 2;
    DoTest((CustomType)charles->type == CustomType::Person);
    DoTest(AreStringsEqual(charles->name, "$charles"));
    DoTest(AreStringsEqual(charles->derived.derivedType, "Person"));

    DoTest(charles->derived.children.count == 2);
    Structure * charlesChildren = charles->derived.children + 1;
    DoTest((CustomType)charlesChildren->type == CustomType::ChildList);
    DoTest(AreStringsEqual(charlesChildren->name, "%children"));
    DoTest(AreStringsEqual(charlesChildren->derived.derivedType, "ChildList"));
    DoTest(charlesChildren->derived.children.count == 1);

    Structure * debbie = charlesChildren->derived.children + 0;
    DoTest((CustomType)debbie->type == CustomType::Person);
    DoTest(AreStringsEqual(debbie->name, "%debbie"));
    DoTest(AreStringsEqual(debbie->derived.derivedType, "Person"));

    DoTest(alice->derived.children.count == 2);
    Structure * aliceFriends = alice->derived.children + 1;
    DoTest((CustomType)aliceFriends->type == CustomType::Friends);
    DoTest(AreStringsEqual(aliceFriends->derived.derivedType, "Friends"));

    DoTest(aliceFriends->derived.children.count == 1);
    Structure * friendRefs = aliceFriends->derived.children + 0;
    DoTest(friendRefs->type == StructureType::OpenDdlPrimitive);
    DoTest(friendRefs->primitive.primitiveType == PrimitiveStructureType::Flat);
    DoTest(friendRefs->primitive.dataType == PrimitiveDataType::Ref);

    DoTest(friendRefs->primitive.cntValue == 3);
    RefValue * refs = (RefValue *)friendRefs->primitive.values;

    DoTest(AreStringsEqual(refs[0].refString, "$bob"));
    DoTest(refs[0].refPtr == bob);

    DoTest(AreStringsEqual(refs[1].refString, "$charles"));
    DoTest(refs[1].refPtr == charles);

    DoTest(AreStringsEqual(refs[2].refString, "$charles%children%debbie"));
    DoTest(refs[2].refPtr == debbie);

    AllTestsPass();
}
