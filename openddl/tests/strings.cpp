bool TestStrings(MemoryRegion memory)
{
    // @Cleanup - Hard-coded path
    char * file = ReadEntireFile("W:/RTS/code/openddl/tests/strings.oddl");
    DoTest(file);
    Defer(free(file));

    DerivedMappings mappings = {};
    
    OpenDdlResult openDdl = ImportOpenDdlFileContents(file, memory, mappings);
    DoTest(openDdl.topLevel.count == 1);
    DoTest(openDdl.cntStructureTotal == 1);
    
    StructureListReader topLevel = CreateStructureListReader(openDdl.topLevel);

    {
        Structure * structure = NextStructure(&topLevel);
        DoTest(structure);
        DoTest(structure->type == StructureType::OpenDdlPrimitive);
        DoTest(structure->name.bytes == nullptr);
        
        PrimitiveStructure * primitive = &structure->primitive;
        DoTest(primitive->primitiveType == PrimitiveStructureType::Flat);
        DoTest(primitive->dataType == PrimitiveDataType::String);
        DoTest(primitive->cntValue == 5);

        String * values = (String *)primitive->values;
        DoTest(AreStringsEqual(values[0], "Hello, world!"));
        DoTest(AreStringsEqual(values[1], ""));
        DoTest(AreStringsEqual(values[2], "This\nhas\nnew\nlines"));
        DoTest(AreStringsEqual(values[3], "Cost: €5")); // NOTE - This relies on compiler encoding this as UTF-8, which isn't a guarantee! (but it's passing, so I'm satisfied)
        DoTest(AreStringsEqual(values[4], "Random byte: \xAB"));
    }

    DoTest(NextStructure(&topLevel) == nullptr);

    AllTestsPass();
}
