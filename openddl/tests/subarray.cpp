bool TestSubarray(MemoryRegion memory)
{
    // @Cleanup - Hard-coded path
    char * file = ReadEntireFile("W:/RTS/code/openddl/tests/subarray.oddl");
    DoTest(file);
    Defer(free(file));

    DerivedMappings mappings = {};

    OpenDdlResult openDdl = ImportOpenDdlFileContents(file, memory, mappings);
    DoTest(openDdl.topLevel.count == 2);
    DoTest(openDdl.cntStructureTotal == 2);
    
    StructureListReader topLevel = CreateStructureListReader(openDdl.topLevel);

    {
        Structure * structure = NextStructure(&topLevel);
        DoTest(structure);
        DoTest(structure->type == StructureType::OpenDdlPrimitive);
        DoTest(structure->name.bytes == nullptr);
        
        PrimitiveStructure * primitive = &structure->primitive;
        DoTest(primitive->primitiveType == PrimitiveStructureType::Subarray);
        DoTest(primitive->dataType == PrimitiveDataType::Int32);

        Subarray subarray = primitive->subarray;
        DoTest(subarray.cntValuePerSubarray == 4);
        DoTest(subarray.cntSubarray == 1);
        DoTest(primitive->cntValue == subarray.cntValuePerSubarray * subarray.cntSubarray);

        s32 * values = (s32 *)primitive->values;
        for (int i = 0; i < primitive->cntValue; i++)
        {
            DoTest(values[i] == (s32)(i + 1));
        }
    }

    {
        Structure * structure = NextStructure(&topLevel);
        DoTest(structure);
        DoTest(structure->type == StructureType::OpenDdlPrimitive);
        DoTest(structure->name.bytes == nullptr);
        
        PrimitiveStructure * primitive = &structure->primitive;
        DoTest(primitive->primitiveType == PrimitiveStructureType::Subarray);
        DoTest(primitive->dataType == PrimitiveDataType::Int32);

        Subarray subarray = primitive->subarray;
        DoTest(subarray.cntValuePerSubarray == 3);
        DoTest(subarray.cntSubarray == 3);
        DoTest(primitive->cntValue == subarray.cntValuePerSubarray * subarray.cntSubarray);

        s32 * values = (s32 *)primitive->values;
        for (int i = 0; i < primitive->cntValue; i++)
        {
            DoTest(values[i] == (s32)(i + 1));
        }
    }

    DoTest(NextStructure(&topLevel) == nullptr);

    AllTestsPass();
}
