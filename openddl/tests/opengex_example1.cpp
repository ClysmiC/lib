bool TestOpenGexExample1(MemoryRegion memory)
{
    // @Cleanup - Hard-coded path
    char * file = ReadEntireFile("W:/RTS/code/openddl/tests/opengex_example1.oddl");
    DoTest(file);
    Defer(free(file));

    // This is a smoke-test that just makes sure things parse properly and we get
    //  the expected # of structures in the output. Drilling down into each structure
    //  is beyond the scope of this test.

    DerivedMappings mappings = {};
    
    OpenDdlResult openDdl = ImportOpenDdlFileContents(file, memory, mappings);
    DoTest(openDdl.topLevel.count == 8);
    DoTest(openDdl.cntStructureTotal == 43);

    AllTestsPass();
}
