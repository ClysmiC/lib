bool TestSubarrayWithStates(MemoryRegion memory)
{
    // @Cleanup - Hard-coded path
    char * file = ReadEntireFile("W:/RTS/code/openddl/tests/subarray_with_states.oddl");
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
        DoTest(AreStringsEqual(structure->name, "$Vertex0"));
        
        PrimitiveStructure * primitive = &structure->primitive;
        DoTest(primitive->primitiveType == PrimitiveStructureType::SubarrayWithStates);
        DoTest(primitive->dataType == PrimitiveDataType::Float32);

        SubarrayWithStates * subarrayWithStates = &primitive->subarrayWithStates;
        DoTest(subarrayWithStates->cntValuePerSubarray == 3);
        DoTest(subarrayWithStates->cntSubarray == 3);
        DoTest(primitive->cntValue == subarrayWithStates->cntValuePerSubarray * subarrayWithStates->cntSubarray);

        Vec3 valuesExpected[] = {
            Vec3(1, 4, 9),
            Vec3(0, 0, 1),
            Vec3(0.6f, 0.1f, 0.0f)
        };
        
        char * statesExpected[] = {
            "Pos",
            "Normal",
            "Uvw"
        };

        f32 * values = (f32 *)primitive->values;
        for (int i = 0; i < subarrayWithStates->cntSubarray; i++)
        {
            DoTest(values[subarrayWithStates->cntValuePerSubarray * i + 0] == valuesExpected[i].e[0]);
            DoTest(values[subarrayWithStates->cntValuePerSubarray * i + 1] == valuesExpected[i].e[1]);
            DoTest(values[subarrayWithStates->cntValuePerSubarray * i + 2] == valuesExpected[i].e[2]);

            DoTest(AreStringsEqual(subarrayWithStates->states[i], statesExpected[i]));
        }
    }

    {
        Structure * structure = NextStructure(&topLevel);
        DoTest(structure);
        DoTest(structure->type == StructureType::OpenDdlPrimitive);
        DoTest(structure->name.bytes == nullptr);
        
        PrimitiveStructure * primitive = &structure->primitive;
        DoTest(primitive->primitiveType == PrimitiveStructureType::SubarrayWithStates);
        DoTest(primitive->dataType == PrimitiveDataType::Float32);

        SubarrayWithStates * subarrayWithStates = &primitive->subarrayWithStates;
        DoTest(subarrayWithStates->cntValuePerSubarray == 2);
        DoTest(subarrayWithStates->cntSubarray == 5);
        DoTest(primitive->cntValue == subarrayWithStates->cntValuePerSubarray * subarrayWithStates->cntSubarray);

        Vec2 valuesExpected[] = {
            Vec2(1, 2),
            Vec2(3, 4),
            Vec2(5, 6),
            Vec2(7, 8),
            Vec2(9, 10),
        };
        
        String statesExpected[] = {
            StringFromLit(""),
            StringFromLit(""),
            StringFromLit("Name1"),
            StringFromLit("Name1"),
            StringFromLit("Name2")
        };

        f32 * values = (f32 *)primitive->values;
        for (int i = 0; i < subarrayWithStates->cntSubarray; i++)
        {
            DoTest(values[subarrayWithStates->cntValuePerSubarray * i + 0] == valuesExpected[i].e[0]);
            DoTest(values[subarrayWithStates->cntValuePerSubarray * i + 1] == valuesExpected[i].e[1]);

            DoTest(AreStringsEqual(subarrayWithStates->states[i], statesExpected[i]));
        }
    }

    DoTest(NextStructure(&topLevel) == nullptr);

    AllTestsPass();
}
