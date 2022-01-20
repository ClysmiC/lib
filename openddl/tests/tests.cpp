// TODO
// - Add meaningful error codes to the OpenDdl lib (basically each goto LError should have its own code)
// - Add tests with malformed input and test that we get the error code we'd expect

#include <cstdio>
#include <cstdlib>

#include "common/common.h"

#include "../openddl.cpp"
using namespace OpenDdl;

//
// Convenience utilities for tests
//

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
//

#include "flat.cpp"
#include "subarray.cpp"
#include "subarray_with_states.cpp"
#include "ints.cpp"
#include "strings.cpp"
#include "derived.cpp"
#include "type_values.cpp"
#include "ref_global.cpp"
#include "properties.cpp"
#include "opengex_example1.cpp"
#include "opengex_example2.cpp"

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

    RunTest(TestFlat);
    RunTest(TestSubarray);
    RunTest(TestSubarrayWithStates);
    RunTest(TestInts);
    RunTest(TestStrings);
    RunTest(TestDerived);
    RunTest(TestTypeValues);
    RunTest(TestRefGlobal);
    RunTest(TestProperties);
    RunTest(TestOpenGexExample1);
    RunTest(TestOpenGexExample2);

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
