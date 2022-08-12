#pragma once

// Blender > Obj > Engine pipeline issues
// - Forward direction... I don't quite understand what Blender's concept of "forward" means, but to get things exporting correctly, we should export with Z up and -X forward (with units facing -Y in Blender)
// - Unit origin. In the master.blend file, each model gets exported relative to the scene origin (not the object's own origin!)
// - Unit size. Need to figure out what we want the scale to be... 1 unit = 1 meter seems reasonable. Need to experiment w/ how many meters a tile should be
// - Texture... not quite sure how texture exporting is happening... the mtl files referenced by the obj files are just a gray-ish color
//

//
// Dependencies

#include "core/core.h"

#include "lib/simple_scan/simple_scan.cpp"

//
// Implementation

namespace Obj // !SkipNamespace
{

#include "_generated.h"
#include "obj.h"
#include "_generated.cpp"

internal u32
HashObjVertex(const ObjVertex & vertex)
{
    u32 result = 31 * (u32)vertex.iPos + 11 * (u32)vertex.iTex + 3 * (u32)vertex.iNorm;
    return result;
}

internal bool
AreObjVerticesEqual(const ObjVertex & v0, const ObjVertex & v1)
{
    bool result = 
        v0.iPos == v1.iPos &&
        v0.iTex == v1.iTex &&
        v0.iNorm == v1.iNorm;

    return result;
}

struct SubmeshBuilder
{
    String usemtl;
    DynArray<Vertex> vertices;
    HashMap<ObjVertex, u32> mapVertexToI;
    DynArray<u32> indices;
};

internal SubmeshBuilder
StartSubmesh(ObjResult * obj, MemoryRegion memory)
{
    SubmeshBuilder result;
    result.vertices = DynArray<Vertex>(memory);
    Init(&result.mapVertexToI, HashObjVertex, AreObjVerticesEqual);
    result.indices = DynArray<u32>(memory);

    return result;
}

internal void
EndSubmesh(ObjResult * obj, SubmeshBuilder * sb, DynArray<Submesh> * allSubmeshes)
{
    if (sb->indices.count > 0)
    {
        Submesh * submesh = AppendNew(allSubmeshes);
        submesh->usemtl = sb->usemtl;
        submesh->vertices = MakeSlice(sb->vertices);
        submesh->indices = MakeSlice(sb->indices);
    }
}

internal ObjResult
ImportObjFileContents(
    String fileContents,
    MemoryRegion memory)
{
    using namespace SimpleScan;
    
    ObjResult result = {};

    // Global bookkeeping
    DynArray<Vec3>    allPositions(memory);
    DynArray<Vec2>    allTexCoords(memory);
    DynArray<Vec3>    allNormals(memory);
    DynArray<Submesh> allSubmeshes(memory);
    DynArray<String>  allMtllibs(memory);

    // Per-submesh bookkeeping
    SubmeshBuilder sb = StartSubmesh(&result, memory);
    
    StringScanner scanner;
    InitScanner(&scanner, fileContents);
    
    while (HasNextChar(&scanner))
    {
        String line = ConsumePast(&scanner, '\n');
        if (line.cBytes == 0)
            break;

        if (line.bytes[0] == '#')
            continue;

        StringScanner lineScanner;
        InitScanner(&lineScanner, line);

        String token = ConsumeUntil(&lineScanner, ' ');
        ConsumeWhitespace(&lineScanner);

        if (AreStringsEqual(token, "o")) continue; // No special meaning assigned to sub-objects
        else if (AreStringsEqual(token, "mtllib"))
        {
            while (HasNextChar(&lineScanner))
            {
                String lib = ConsumeUntilWhitespace(&lineScanner); ConsumeWhitespace(&lineScanner);
                Append(&allMtllibs, DuplicateString(lib, memory));
            }

            if (allMtllibs.count == 0) goto LError;
        }
        else if (AreStringsEqual(token, "usemtl"))
        {
            String mtl = ConsumeUntilWhitespace(&lineScanner); ConsumeWhitespace(&lineScanner);
            if (mtl.cBytes <= 0) goto LError;

            EndSubmesh(&result, &sb, &allSubmeshes);
            
            sb = StartSubmesh(&result, memory);
            sb.usemtl = DuplicateString(mtl, memory);
        }
        else if (AreStringsEqual(token, "s")) continue; // TODO
        else if (AreStringsEqual(token, "v"))
        {
            f64 x, y, z;
            String number;
            
            number = ConsumeUntilWhitespace(&lineScanner); ConsumeWhitespace(&lineScanner);
            if (!TryParseF64FromEntireString(number, &x)) goto LError;

            number = ConsumeUntilWhitespace(&lineScanner); ConsumeWhitespace(&lineScanner);
            if (!TryParseF64FromEntireString(number, &y)) goto LError;

            number = ConsumeUntilWhitespace(&lineScanner); ConsumeWhitespace(&lineScanner);
            if (!TryParseF64FromEntireString(number, &z)) goto LError;

            if (HasNextChar(&lineScanner)) goto LError;

            Append(&allPositions, Vec3((f32)x, (f32)y, (f32)z));
        }
        else if (AreStringsEqual(token, "vt"))
        {
            f64 u, v;
            String number;
            
            number = ConsumeUntilWhitespace(&lineScanner); ConsumeWhitespace(&lineScanner);
            if (!TryParseF64FromEntireString(number, &u)) goto LError;

            number = ConsumeUntilWhitespace(&lineScanner); ConsumeWhitespace(&lineScanner);
            if (!TryParseF64FromEntireString(number, &v)) goto LError;

            if (HasNextChar(&lineScanner)) goto LError;

            Append(&allTexCoords, Vec2((f32)u, (f32)v));
        }
        else if (AreStringsEqual(token, "vn"))
        {
            f64 x, y, z;
            String number;
            
            number = ConsumeUntilWhitespace(&lineScanner); ConsumeWhitespace(&lineScanner);
            if (!TryParseF64FromEntireString(number, &x)) goto LError;

            number = ConsumeUntilWhitespace(&lineScanner); ConsumeWhitespace(&lineScanner);
            if (!TryParseF64FromEntireString(number, &y)) goto LError;

            number = ConsumeUntilWhitespace(&lineScanner); ConsumeWhitespace(&lineScanner);
            if (!TryParseF64FromEntireString(number, &z)) goto LError;

            if (HasNextChar(&lineScanner)) goto LError;
            
            Append(&allNormals, NormalizeSafeXAxis(Vec3((f32)x, (f32)y, (f32)z)));
        }
        else if (AreStringsEqual(token, "f"))
        {
            int cntVertex = 0;
            u32 faceVertexIndices[4];
            
            while (HasNextChar(&lineScanner))
            {
                if (cntVertex >= ArrayLen(faceVertexIndices)) goto LError;
                
                String vertexString = ConsumeUntilWhitespace(&lineScanner);
                ConsumeWhitespace(&lineScanner);

                StringScanner vertexScanner;
                InitScanner(&vertexScanner, vertexString);

                ObjVertex objVertex;
                
                String indexString = ConsumeUntil(&vertexScanner, '/'); NextChar(&vertexScanner);
                if (!TryParseU64FromEntireString(indexString, &objVertex.iPos)) goto LError;

                indexString = ConsumeUntil(&vertexScanner, '/'); NextChar(&vertexScanner);
                if (!TryParseU64FromEntireString(indexString, &objVertex.iTex)) goto LError;

                indexString = ConsumeUntilWhitespace(&vertexScanner); ConsumeWhitespace(&vertexScanner);
                if (!TryParseU64FromEntireString(indexString, &objVertex.iNorm)) goto LError;

                // NOTE - .obj indices are 1-based...
                objVertex.iPos--;
                objVertex.iTex--;
                objVertex.iNorm--;                
                if (objVertex.iPos >= allPositions.count ||
                    objVertex.iTex >= allTexCoords.count ||
                    objVertex.iNorm >= allNormals.count)
                    goto LError;
                
                u32 * pIndex = Lookup(sb.mapVertexToI, objVertex);
                if (pIndex)
                {
                    Assert((int)*pIndex < sb.vertices.count);
                }
                else
                {
                    pIndex = InsertNew(&sb.mapVertexToI, objVertex);
                    *pIndex = sb.vertices.count;

                    Vertex * vertex = AppendNew(&sb.vertices);
                    vertex->position = allPositions[(int)objVertex.iPos];
                    vertex->normal = allNormals[(int)objVertex.iNorm];
                    vertex->uv = allTexCoords[(int)objVertex.iTex];
                }

                faceVertexIndices[cntVertex] = *pIndex;
                cntVertex++;
            }

            // NOTE - We only support importing tri's and quad's. Triangulate meshes on export!
            if (cntVertex < 3 || cntVertex > 4) goto LError;
            
            // Tri 1
            Append(&sb.indices, faceVertexIndices[0]);
            Append(&sb.indices, faceVertexIndices[1]);
            Append(&sb.indices, faceVertexIndices[2]);

            if (cntVertex == 4)
            {
                // Tri 2 (for quad)
                Append(&sb.indices, faceVertexIndices[2]);
                Append(&sb.indices, faceVertexIndices[3]);
                Append(&sb.indices, faceVertexIndices[0]);
            }
        }
        else
        {
            AssertTodo;
            goto LError;
        }
    }

    EndSubmesh(&result, &sb, &allSubmeshes);

    result.submeshes = MakeSlice(allSubmeshes);
    result.mtllibs = MakeSlice(allMtllibs);

    result.success = true;
    return result;

LError:
    result.success = false;
    return result;
}

internal bool
ReadMaterials(
    char * mtlFileContents,
    ObjResult * obj,
    MemoryRegion memory)
{
    using namespace SimpleScan;
    
    ZStringScanner scanner;
    InitScanner(&scanner, mtlFileContents);

    DynArray<Material> allMaterials(memory);
    Material * materialCur = nullptr;

    while (HasNextChar(&scanner))
    {
        ConsumeWhitespace(&scanner);
        
        String line = ConsumePast(&scanner, '\n');
        if (line.cBytes == 0)
            break;

        if (line.bytes[0] == '#')
            continue;

        StringScanner lineScanner;
        InitScanner(&lineScanner, line);

        String token = ConsumeUntil(&lineScanner, ' ');
        ConsumeWhitespace(&lineScanner);

        // @Slow - Checking this every iteration
        if (!materialCur && !AreStringsEqual(token, "newmtl"))
            goto LError;

        if (AreStringsEqual(token, "newmtl"))
        {
            materialCur = AppendNew(&allMaterials);
            *materialCur = {};

            String name = ConsumeUntilWhitespace(&lineScanner); ConsumeWhitespace(&lineScanner);
            materialCur->name = DuplicateString(name, memory);
            if (HasNextChar(&lineScanner)) goto LError;
        }
        else if (AreStringsEqual(token, "Ns"))
        {
            String number;
            
            number = ConsumeUntilWhitespace(&lineScanner); ConsumeWhitespace(&lineScanner);
            if (!TryParseF32FromEntireString(number, &materialCur->Ns)) goto LError;
            
            if (HasNextChar(&lineScanner)) goto LError;
        }
        else if (AreStringsEqual(token, "Ka") ||
                 AreStringsEqual(token, "Kd") ||
                 AreStringsEqual(token, "Ks"))
        {
            f32 r, g, b;
            String number;
            
            number = ConsumeUntilWhitespace(&lineScanner); ConsumeWhitespace(&lineScanner);
            if (!TryParseF32FromEntireString(number, &r)) goto LError;

            number = ConsumeUntilWhitespace(&lineScanner); ConsumeWhitespace(&lineScanner);
            if (!TryParseF32FromEntireString(number, &g)) goto LError;

            number = ConsumeUntilWhitespace(&lineScanner); ConsumeWhitespace(&lineScanner);
            if (!TryParseF32FromEntireString(number, &b)) goto LError;
            
            if (HasNextChar(&lineScanner)) goto LError;

            // @Slow - Checking string twice
            if (AreStringsEqual(token, "Ka"))
            {
                materialCur->Ka = Vec3(r, g, b);
            }
            else if (AreStringsEqual(token, "Kd"))
            {
                materialCur->Kd = Vec3(r, g, b);
            }
            else
            {
                Assert(AreStringsEqual(token, "Ks"));
                materialCur->Ks = Vec3(r, g, b);
            }
        }
        else if (AreStringsEqual(token, "map_Ka") ||
                 AreStringsEqual(token, "map_Kd") ||
                 AreStringsEqual(token, "map_Ks"))
        {
            String file = ConsumeUntilWhitespace(&lineScanner); ConsumeWhitespace(&lineScanner);
            file = DuplicateString(file, memory);
            
            if (HasNextChar(&lineScanner)) goto LError;

            // @Slow - Checking string twice
            if (AreStringsEqual(token, "map_Ka"))
            {
                materialCur->map_Ka = file;
            }
            else if (AreStringsEqual(token, "map_Kd"))
            {
                materialCur->map_Kd = file;
            }
            else
            {
                Assert(AreStringsEqual(token, "map_Ks"));
                materialCur->map_Ks = file;
            }            
        }
        else
        {
            // Unsupported parameter. Don't error, since there are a lot
            //  of obscure things we don't support, but might show up in files!
            
            continue;
        }
    }

    if (allMaterials.count <= 0) goto LError;

    obj->materials = MakeSlice(allMaterials);
    return true;
    
LError:
    return false;
}

} // namespace Obj

