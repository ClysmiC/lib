bool TestFlat(MemoryRegion memory)
{
    // @Cleanup - Hard-coded path
    char * file = ReadEntireFile("W:/RTS/code/openddl/tests/flat.oddl");
    DoTest(file);
    Defer(free(file));

    DerivedMappings mappings = {};
    
    OpenDdlResult openDdl = ImportOpenDdlFileContents(file, memory, mappings);
    DoTest(openDdl.cntStructureTotal == 3);
    DoTest(openDdl.topLevel.count == 3);

    StructureListReader topLevel = CreateStructureListReader(openDdl.topLevel);

    {
        Structure * structure = NextStructure(&topLevel);
        DoTest(structure);
        DoTest(structure->type == StructureType::OpenDdlPrimitive);
        DoTest(structure->name.bytes == nullptr);
        
        PrimitiveStructure * primitive = &structure->primitive;
        DoTest(primitive->primitiveType == PrimitiveStructureType::Flat);
        DoTest(primitive->dataType == PrimitiveDataType::Int32);
        DoTest(primitive->cntValue == 6);

        s32 * values = (s32 *)primitive->values;
        for (int i = 0; i < primitive->cntValue; i++)
        {
            s32 fibonacci = 1;
            if (i >= 2)
            {
                fibonacci = values[i - 2] + values[i - 1];
            }
            
            DoTest(values[i] == fibonacci);
        }
    }

    {
        Structure * structure = NextStructure(&topLevel);
        DoTest(structure);
        DoTest(structure->type == StructureType::OpenDdlPrimitive);
        DoTest(structure->name.bytes == nullptr);
        
        PrimitiveStructure * primitive = &structure->primitive;
        DoTest(primitive->primitiveType == PrimitiveStructureType::Flat);
        DoTest(primitive->dataType == PrimitiveDataType::Float32);
        DoTest(primitive->cntValue == 3);

        f32 * values = (f32 *)primitive->values;
        DoTest(values[0] == 0.25f);
        DoTest(values[1] == 0.5f);
        DoTest(values[2] == 1.0f);
    }

    {
        Structure * structure = NextStructure(&topLevel);
        DoTest(structure);
        DoTest(structure->type == StructureType::OpenDdlPrimitive);
        DoTest(structure->name.bytes == nullptr);
        
        PrimitiveStructure * primitive = &structure->primitive;
        DoTest(primitive->primitiveType == PrimitiveStructureType::Flat);
        DoTest(primitive->dataType == PrimitiveDataType::Bool);
        DoTest(primitive->cntValue == 4);

        bool * values = (bool *)primitive->values;
        DoTest(values[0] == false);
        DoTest(values[1] == true);
        DoTest(values[2] == false);
        DoTest(values[3] == true);
    }

    DoTest(NextStructure(&topLevel) == nullptr);
    
    AllTestsPass();
}
