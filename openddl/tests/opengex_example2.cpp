bool TestOpenGexExample2(MemoryRegion memory)
{
    // @Cleanup - Hard-coded path
    char * file = ReadEntireFile("W:/RTS/code/openddl/tests/opengex_example2.oddl");
    DoTest(file);
    Defer(free(file));

    // This is a smoke-test that just makes sure things parse properly and we get
    //  the expected # of structures in the output. Drilling down into each structure
    //  is beyond the scope of this test.

    DerivedMappings mappings = {};
    
    OpenDdlResult openDdl = ImportOpenDdlFileContents(file, memory, mappings);
    DoTest(openDdl.topLevel.count == 7);
    DoTest(openDdl.cntStructureTotal == 90);

    AllTestsPass();
}
