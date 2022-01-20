// TODO
// - Add meaningful error codes to the OpenGex lib (basically each goto LError should have its own code)
// - Add tests with malformed input and test that we get the error code we'd expect

#include <cstdio>
#include <cstdlib>

#include "common/common.h"

#include "../opengex.cpp"
using namespace OpenGex;


//
// Convenience utilities for tests

#define DoTest(EXPRESSION)                                              \
    do {                                                                \
        if (!(EXPRESSION)) {                                            \
            fprintf(stderr, "FAILED: %s(%d): %s\n", __FILE__, __LINE__, #EXPRESSION); \
            return false;                                               \
        }                                                               \
    } while (0)

#define AllTestsPass() printf("PASSED: %s\n", __FILE__); return true;

#pragma warning (disable : 4996) // Absolutely insane that this is required to use fopen...
char * ReadEntireFile(char * filename)
{
    FILE * file = fopen(filename, "rb");
    if (!file)
        return nullptr;
    
    fseek(file, 0, SEEK_END);
    
    char * result = nullptr;
    long cBytesFile = ftell(file);

    fseek (file, 0, SEEK_SET);
    result = (char *)malloc(cBytesFile + 1);
    if (result)
    {
        fread(result, 1, cBytesFile, file);
    }

    fclose(file);

    result[cBytesFile] = '\0';
    return result;
}

//
// Test runner

internal bool
TestMech(MemoryRegion memory)
{
    // @Cleanup - Hard-coded path
    char * file = ReadEntireFile("W:/RTS/data/models/mech.ogex");
    DoTest(file);
    Defer(free(file));

    OpenGexResult openGex = ImportOpenGexFileContents(file, memory);
    DoTest(openGex.success);

    DoTest(openGex.metrics[(int)Metric::Key::Distance].distanceValue == 1.0f);
    DoTest(openGex.metrics[(int)Metric::Key::Up].upValue == Metric::Up::Z);
    DoTest(openGex.metrics[(int)Metric::Key::Angle].angleValue == 1.0f);
    DoTest(openGex.metrics[(int)Metric::Key::Time].timeValue == 1.0f);

    DoTest(openGex.geometryNodes.count == 1);
    DoTest(openGex.geometryObjects.count == 1);
    DoTest(openGex.materials.count == 1);

    GeometryNode * geometryNode = openGex.geometryNodes + 0;
    GeometryObject * geometryObject = openGex.geometryObjects + 0;
    Material * material = openGex.materials + 0;

    DoTest(geometryNode->geometryObject == geometryObject);
    DoTest(geometryNode->materials.count == 1);
    DoTest(geometryNode->materials[0] == material);
    
    AllTestsPass();
}

int main()
{
    printf("\n");
    fflush(stdout);

    uint cBytesMemory = Megabytes(1);
    u8 * memoryBytes = new u8[cBytesMemory];
        
    MemoryRegion memory = BeginRootMemoryRegion(memoryBytes, cBytesMemory);

    int cntTest = 0;
    int cntPass = 0;
    
#define RunTest(TEST) cntPass += TEST(memory); cntTest++; ResetMemoryRegion(memory);

    RunTest(TestMech);

#undef RunTest

    fflush(stderr);

    printf("\n");
    
    if (cntPass == cntTest)
    {
        printf("All tests PASSED\n");
    }
    else
    {
        printf("!! Some tests FAILED !!\n");
    }
    
    int cntFail = cntTest - cntPass;
    printf("(%d passed, %d failed, %d total)\n", cntPass, cntFail, cntTest);
        
    
    return 0;
}
