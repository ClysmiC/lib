bool TestDerived(MemoryRegion memory)
{
    // @Cleanup - Hard-coded path
    char * file = ReadEntireFile("W:/RTS/code/openddl/tests/derived.oddl");
    DoTest(file);
    Defer(free(file));

    enum class CustomType : u32
    {
        Nil                   = StructureType::Nil,

        OpenDdlPrimitive      = StructureType::OpenDdlPrimitive,
        UnrecognizedExtension = StructureType::UnrecognizedExtension,

        Vertex                = StructureType::DerivedTypeStart,
        Vector3,
    };

    DerivedTypeMapping mappingArray[] = {
        { StringFromLit("Vertex"),  (u32)CustomType::Vertex },
        { StringFromLit("Vector3"), (u32)CustomType::Vector3 },
    };
    
    DerivedMappings mappings = {};
    mappings.typeMappings = mappingArray;
    mappings.cntTypeMapping = ArrayLen(mappingArray);
    
    OpenDdlResult openDdl = ImportOpenDdlFileContents(file, memory, mappings);
    DoTest(openDdl.topLevel.count == 1);
    DoTest(openDdl.cntStructureTotal == 5);

    StructureListReader topLevel = CreateStructureListReader(openDdl.topLevel);

    // Vertex
    {
        Structure * structure = NextStructure(&topLevel);
        DoTest(structure);
        DoTest((CustomType)structure->type == CustomType::Vertex);
        DoTest(structure->parent == nullptr);
        DoTest(structure->name.bytes == nullptr);
        
        DerivedStructure vertex = structure->derived;
        DoTest(AreStringsEqual(vertex.derivedType, "Vertex"));

        Slice<Structure> children = vertex.children;
        DoTest(children.count == 2);

        // Pos
        {
            Structure * child = children + 0;
            DoTest((CustomType)child->type == CustomType::Vector3);
            DoTest(child->parent == structure);
            DoTest(AreStringsEqual(child->name, "%pos"));

            DerivedStructure pos = child->derived;
            DoTest(AreStringsEqual(pos.derivedType, "Vector3"));

            // Primitive data
            {
                DoTest(pos.children.count == 1);
                
                Structure * grandChild = pos.children + 0;
                DoTest(grandChild->type == StructureType::OpenDdlPrimitive);
                DoTest(grandChild->parent == child);
                DoTest(grandChild->name.bytes == nullptr);

                PrimitiveStructure primitive = grandChild->primitive;
                DoTest(primitive.primitiveType == PrimitiveStructureType::Flat);
                DoTest(primitive.dataType == PrimitiveDataType::Float32);
                DoTest(primitive.cntValue == 3);

                f32 * values = (f32 *)primitive.values;
                DoTest(values[0] == 1.1f);
                DoTest(values[1] == 2.2f);
                DoTest(values[2] == 3.3f);
            }
        }

        // Normal
        {
            Structure * child = children + 1;
            DoTest((CustomType)child->type == CustomType::Vector3);
            DoTest(child->parent == structure);
            DoTest(AreStringsEqual(child->name, "%normal"));

            DerivedStructure normal = child->derived;
            DoTest(AreStringsEqual(normal.derivedType, "Vector3"));

            // Primitive data
            {
                DoTest(normal.children.count == 1);
                
                Structure * grandChild = normal.children + 0;
                DoTest(grandChild->type == StructureType::OpenDdlPrimitive);
                DoTest(grandChild->parent == child);
                DoTest(grandChild->name.bytes == nullptr);

                PrimitiveStructure primitive = grandChild->primitive;
                DoTest(primitive.primitiveType == PrimitiveStructureType::Flat);
                DoTest(primitive.dataType == PrimitiveDataType::Float32);
                DoTest(primitive.cntValue == 3);

                f32 * values = (f32 *)primitive.values;
                DoTest(values[0] == 0.0f);
                DoTest(values[1] == 0.0f);
                DoTest(values[2] == -1.0f);
            }
        }
    }

    DoTest(NextStructure(&topLevel) == nullptr);

    AllTestsPass();
}
