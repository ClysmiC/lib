#pragma once

struct Submesh
{
    Slice<u32> indices;
    Slice<Vertex> vertices; // @Gross - Unity build dependency on Vertex
    
    String usemtl; // Empty string for unspecified material (default)
};

struct Material
{
    String name;
    f32 Ns;        // spectral exponent
    Vec3 Ka;       // ambient
    Vec3 Kd;       // diffuse
    Vec3 Ks;       // spectral
    String map_Ka; // ambient map
    String map_Kd; // diffuse map
    String map_Ks; // spectral map
};

struct ObjResult
{
    Slice<String> mtllibs;
    Slice<Submesh> submeshes;
    Slice<Material> materials;

    bool success;
};

struct ObjVertex
{
    u64 iPos;
    u64 iTex;
    u64 iNorm;
};

