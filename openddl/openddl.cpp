#pragma once

// TODO
// - Make configuration to auto-skip properties that get overwritten by later ones? There are a few fixme's in opegex.cpp
//   that would get fixed by that
// - Make each goto LError also store some information about what the error *was*... Take a holistic look
//   at how we are doing errors/bubbling!
// - Write a test for local references. The spec isn't super clear to me what this looks like and doesn't
//   give any good examples. And I can't find any good examples in the wild, so I'm just going to assume
//   my implementation works and if it ends up blowing up, then at least that means I have a file that
//   uses a local reference, which can be the basis of a test for me to write!

//
// Dependencies

#include "core/core.h"
#include "lib/simple_scan/simple_scan.cpp"

//
// Implementation

namespace OpenDdl // !SkipNamespace
{

#include "_generated.h"
#include "openddl.h"
#include "_generated.cpp"

#include "scan.cpp"
#include "parse.cpp"

// @CopyPaste - below function...
internal Structure *
ResolveFirstNameInReferencePathGlobal(
    String path,
    Slice<Structure> globalRoot,
    String * poPathAfterResolve) // NOTE - Allowed to be the address of the path arg if you want to update in place
{
    Assert(path.cBytes > 0);
    Assert(path[0] == '$');
    
    //  @Slow - Might want to track which structures at each level *have* names, so when we are looking for
    //  a target to resolve to, we don't waste time looking at unnamed structures

    *poPathAfterResolve = path;
        
    Structure * result = nullptr;
    for (Structure * structure : ByPtr(globalRoot))
    {
        if (!structure->name.bytes)
            continue;
        
        if (StringHasPrefix(path, structure->name))
        {
            result = structure;
            poPathAfterResolve->bytes = path.bytes + structure->name.cBytes;
            poPathAfterResolve->cBytes = path.cBytes - structure->name.cBytes;
            break;
        }
    }

    return result;
}

// @CopyPaste - above function...
internal Structure *
ResolveFirstNameInReferencePathLocal(
    String path,
    DerivedStructure * parent,
    String * poPathAfterResolve) // NOTE - Allowed to be the address of the path arg if you want to update in place
{
    Assert(path.cBytes > 0);
    Assert(path[0] == '%');
    Assert(parent);

    //  @Slow - Might want to track which structures at each level *have* names, so when we are looking for
    //  a target to resolve to, we don't waste time looking at unnamed structures

    *poPathAfterResolve = path;
    
    Structure * result = nullptr;
    for (Structure * structure : ByPtr(parent->children))
    {
        if (!structure->name.bytes)
            continue;
        
        if (StringHasPrefix(path, structure->name))
        {
            result = structure;
            poPathAfterResolve->bytes = path.bytes + structure->name.cBytes;
            poPathAfterResolve->cBytes = path.cBytes - structure->name.cBytes;
            break;
        }
    }

    return result;
}

internal bool
ResolveReference(
    RefValue * ref,
    DerivedStructure * parent,
    Slice<Structure> globalRoot)
{
    Assert(ref->refString.bytes);
    
    if (AreStringsEqual(ref->refString, "null"))
    {
        Assert(ref->refPtr == nullptr);
    }
    else if (ref->refString[0] == '$')
    {
        // @CopyPaste below..
        String nextLocalName;
        Structure * target = ResolveFirstNameInReferencePathGlobal(ref->refString, globalRoot, &nextLocalName);
        if (!target)
            goto LError;

        while (nextLocalName.cBytes)
        {
            if (!IsDerivedType(target->type))
                goto LError;
            
            target = ResolveFirstNameInReferencePathLocal(nextLocalName, &target->derived, &nextLocalName);
            if (!target)
                goto LError;
        }

        ref->refPtr = target;
    }
    else
    {
        if (!parent)
            goto LError;

        // @CopyPaste above...
        String nextLocalName;
        Structure * target = ResolveFirstNameInReferencePathLocal(ref->refString, parent, &nextLocalName);
        if (!target)
            goto LError;

        while (nextLocalName.cBytes)
        {
            if (!IsDerivedType(target->type))
                goto LError;
            
            target = ResolveFirstNameInReferencePathLocal(nextLocalName, &target->derived, &nextLocalName);
            if (!target)
                goto LError;
        }

        ref->refPtr = target;
    }

    return true;
    
LError:
    return false;
}

internal bool
ResolveAllReferences(
    DerivedStructure * parent, // NOTE - null parent will resolve all references from globalRoot. non-null will resolve all from parent down
    Slice<Structure> globalRoot)
{
    // TODO - We don't check for duplicate names, but we probably should. Right now, we'll just resolve to
    //  the first matching one we find.
    
    // @Slow - Should probably keep a list of all our unresloved references so we don't have to walk the
    //  entire structure tree to find them!
    // NOTE - It is somewhat worthwhile keeping it this way for now, as it increases my confidence that
    //  we can actually fully walk the tree!

    Slice<Structure> search = globalRoot;
    if (parent)
    {
        search = parent->children;
    }
    
    for (Structure * child : ByPtr(search))
    {
        if (child->type == StructureType::OpenDdlPrimitive)
        {
            PrimitiveStructure * primitive = &child->primitive;
            if (primitive->dataType != PrimitiveDataType::Ref)
                continue;
            
            RefValue * values = (RefValue *)primitive->values;
            for (int iValue = 0; iValue < primitive->cntValue; iValue++)
            {
                RefValue * value = values + iValue;
                if (!ResolveReference(value, parent, globalRoot))
                    goto LError;
            }
        }
        else
        {
            Assert(IsDerivedType(child->type));
            DerivedStructure * derived = &child->derived;
            if (!ResolveAllReferences(derived, globalRoot))
                goto LError;
        }
    }

    return true;
    
LError:
    return false;
}

internal void
SetParentPointers(Structure * parent, Slice<Structure> children)
{
    // HMM - Existence of this function as a separate step is kinda weak!
    //  Maybe use a single global table of structures indexed by ID?
    //  Children lists would then be a list of IDs and parent "pointers" would just be an ID...

    if (parent)
    {
        Assert(IsDerivedType(parent->type));
        Assert(parent->derived.children == children);
    }

    for (Structure * child : ByPtr(children))
    {
        child->parent = parent;
        
        if (!IsDerivedType(child->type))
            continue;

        SetParentPointers(child, child->derived.children);
    }
}

internal OpenDdlResult
ImportOpenDdlFileContents(
    char * fileContents,
    MemoryRegion memory,
    DerivedMappings derivedMappings)
{
    OpenDdlResult result = {};
    
    Scanner scanner = {};
    scanner.s.contents = fileContents;
    scanner.s.iLine = 1;

    // Parse structures

    DynArray<Structure> topLevel = {};
    topLevel.memory = memory;

    StructureId idNext = StructureId::Nil + 1;
    
    while (!IsAtEofToken(&scanner))
    {
        Structure structure = ParseStructure(&scanner, &idNext, memory, &derivedMappings);
        if (structure.type == StructureType::Nil)
            goto LError;

        if (derivedMappings.skipUnrecognizedTypes && structure.type == StructureType::UnrecognizedExtension)
            continue;

        Append(&topLevel, structure);
    }

    result.topLevel = MakeSlice(topLevel);

    SetParentPointers(nullptr, result.topLevel);
    
    if (!ResolveAllReferences(nullptr, result.topLevel))
        goto LError;

    result.cntStructureTotal = (int)idNext - 1;
    
    return result;

LError:
    
    result.cntStructureTotal = -1;
    result.topLevel = {};
    return result;
}

} // namespace OpenDdl
