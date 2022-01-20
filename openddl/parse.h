struct DataListUnknownLength
{
    int cntValue;
    void * values;
};

struct DataArrayList
{
    int cntSubarray;
    void * values;   // tightly packed. length = cntSubarray * cntValuePerSubarray (known by caller)
    String * states; // length = cntSubarray
};
