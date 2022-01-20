bool TestInts(MemoryRegion memory)
{
    // @Cleanup - Hard-coded path
    char * file = ReadEntireFile("W:/RTS/code/openddl/tests/ints.oddl");
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
        DoTest(primitive->dataType == PrimitiveDataType::UInt32);
        DoTest(primitive->cntValue == 4);

        u32 * values = (u32 *)primitive->values;
        for (int i = 0; i < primitive->cntValue; i++)
        {
            DoTest(values[i] == 1094861636);
        }
    }

    DoTest(NextStructure(&topLevel) == nullptr);

    AllTestsPass();
}
