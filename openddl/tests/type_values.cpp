bool TestTypeValues(MemoryRegion memory)
{
    // @Cleanup - Hard-coded path
    char * file = ReadEntireFile("W:/RTS/code/openddl/tests/type_values.oddl");
    DoTest(file);
    Defer(free(file));

    DerivedMappings mappings = {};
    
    OpenDdlResult openDdl = ImportOpenDdlFileContents(file, memory, mappings);
    DoTest(openDdl.topLevel.count == 2);
    DoTest(openDdl.cntStructureTotal == 2);

    StructureListReader topLevel = CreateStructureListReader(openDdl.topLevel);

    {
        Structure * structure = NextStructure(&topLevel);
        DoTest(structure->type == StructureType::OpenDdlPrimitive);
        DoTest(AreStringsEqual(structure->name, "$types1"));
        
        PrimitiveStructure * primitive = &structure->primitive;
        DoTest(primitive->primitiveType == PrimitiveStructureType::Flat);
        DoTest(primitive->dataType == PrimitiveDataType::Type);
        DoTest(primitive->cntValue == 3);

        PrimitiveDataType * values = (PrimitiveDataType *)primitive->values;
        DoTest(values[0] == PrimitiveDataType::Bool);
        DoTest(values[1] == PrimitiveDataType::Int16);
        DoTest(values[2] == PrimitiveDataType::Type);
    }

    {
        Structure * structure = NextStructure(&topLevel);
        DoTest(structure->type == StructureType::OpenDdlPrimitive);
        DoTest(AreStringsEqual(structure->name, "$types2"));
        
        PrimitiveStructure * primitive = &structure->primitive;
        DoTest(primitive->primitiveType == PrimitiveStructureType::Flat);
        DoTest(primitive->dataType == PrimitiveDataType::Type);
        DoTest(primitive->cntValue == 3);

        PrimitiveDataType * values = (PrimitiveDataType *)primitive->values;
        DoTest(values[0] == PrimitiveDataType::Ref);
        DoTest(values[1] == PrimitiveDataType::String);
        DoTest(values[2] == PrimitiveDataType::Float64);
    }

    DoTest(NextStructure(&topLevel) == nullptr);

    AllTestsPass();
}
