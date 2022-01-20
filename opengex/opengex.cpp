#pragma once

//
// Dependencies

#include "core/core.h"
#include "lib/openddl/openddl.cpp"

//
// Implementation

namespace OpenGex // !SkipNamespace
{

using namespace OpenDdl;

#include "_generated.h"
#include "opengex.h"
#include "_generated.cpp"

internal bool
ReadMetric(DerivedStructure * structure, Metric * poResult)
{
    // FIXME - "The same property can be specified more than once in the same property list, and in such a case, all but the final value specified for the same property must be ignored"
    if (structure->properties.count != 1)
        goto LError;

    Property * property = structure->properties + 0;    
    if ((u32)property->name != (u32)OpenGexProperty::Key)
        goto LError;

    if (structure->children.count != 1)
        goto LError;

    PrimitiveStructure * primitive = &structure->children[0].primitive;
    if (AreStringsEqual(property->as.stringValue, "distance"))
    {
        poResult->key = Metric::Key::Distance;
            
        if (primitive->primitiveType != PrimitiveStructureType::Flat ||
            primitive->dataType != PrimitiveDataType::Float32 ||
            primitive->cntValue != 1)
            goto LError;

        poResult->distanceValue = ((f32 *)primitive->values)[0];
    }
    else if (AreStringsEqual(property->as.stringValue, "up"))
    {
        poResult->key = Metric::Key::Up;
            
        if (primitive->primitiveType != PrimitiveStructureType::Flat ||
            primitive->dataType != PrimitiveDataType::String ||
            primitive->cntValue != 1)
            goto LError;

        String value = ((String *)primitive->values)[0];
        if      (AreStringsEqual(value, "y")) poResult->upValue = Metric::Up::Y;
        else if (AreStringsEqual(value, "z")) poResult->upValue = Metric::Up::Z;
        else goto LError;

    }
    else if (AreStringsEqual(property->as.stringValue, "angle"))
    {
        poResult->key = Metric::Key::Angle;
            
        if (primitive->primitiveType != PrimitiveStructureType::Flat ||
            primitive->dataType != PrimitiveDataType::Float32 ||
            primitive->cntValue != 1)
            goto LError;

        poResult->angleValue = ((f32 *)primitive->values)[0];
    }
    else if (AreStringsEqual(property->as.stringValue, "time"))
    {
        poResult->key = Metric::Key::Time;
            
        if (primitive->primitiveType != PrimitiveStructureType::Flat ||
            primitive->dataType != PrimitiveDataType::Float32 ||
            primitive->cntValue != 1)
            goto LError;

        poResult->timeValue = ((f32 *)primitive->values)[0];
    }
    else
    {
        // TODO, or unrecognized key
        
        AssertTodo;
        goto LError;
    }

    Assert(poResult->key != Metric::Key::Nil);
    return true;
    
LError:
    return false;
}

internal bool
ReadName(DerivedStructure * structure, String * poResult, MemoryRegion memory)
{
    if (structure->properties.count != 0)
        goto LError;

    if (structure->children.count != 1)
        goto LError;

    Structure * child = structure->children + 0;
    if (child->type != StructureType::OpenDdlPrimitive)
        goto LError;

    PrimitiveStructure * primitive = &child->primitive;
    if (primitive->primitiveType != PrimitiveStructureType::Flat)
        goto LError;
    
    if (primitive->dataType != PrimitiveDataType::String)
        goto LError;

    if (primitive->cntValue != 1)
        goto LError;
    
    *poResult = ((String *)primitive->values)[0];

    return true;
    
LError:
    return false;
}

internal bool
ReadColor(DerivedStructure * structure, Color * poResult, OpenGexType containingType)
{
    Assert(containingType == OpenGexType::Material || containingType == OpenGexType::LightObject);
    bool isInMaterial = (containingType == OpenGexType::Material);

    //
    // Initialize

    *poResult = {};

    //
    // Properties
    
    if (isInMaterial)
    {
        PropertyListReader propReader = CreatePropertyListReader(structure->properties);
        for (Property * property = NextProperty(&propReader);
             property;
             property = NextProperty(&propReader))
        {
            switch ((u32)property->name)
            {
                case OpenGexProperty::Attrib:
                {
                    if (AreStringsEqual(property->as.stringValue, "diffuse"))
                    {
                        poResult->materialAttrib = MaterialAttrib::Diffuse;
                    }
                    else if (AreStringsEqual(property->as.stringValue, "specular"))
                    {
                        poResult->materialAttrib = MaterialAttrib::Specular;
                    }
                    else if (AreStringsEqual(property->as.stringValue, "emission"))
                    {
                        poResult->materialAttrib = MaterialAttrib::Emission;
                    }
                    else if (AreStringsEqual(property->as.stringValue, "transparency"))
                    {
                        poResult->materialAttrib = MaterialAttrib::Transparency;
                    }
                    else if (AreStringsEqual(property->as.stringValue, "clearcoat"))
                    {
                        poResult->materialAttrib = MaterialAttrib::ClearCoat;
                    }
                    else if (AreStringsEqual(property->as.stringValue, "sheen"))
                    {
                        poResult->materialAttrib = MaterialAttrib::Sheen;
                    }
                    else
                        goto LError; // FIXME - I guess technically you could put a junk value here as long as you repeat the "attrib" property with the correct one sometime after
                } break;

                default:
                    goto LError;
            }
        }
    }
    else
    {
        bool isAttribLight = false;
        
        PropertyListReader propReader = CreatePropertyListReader(structure->properties);
        for (Property * property = NextProperty(&propReader);
             property;
             property = NextProperty(&propReader))
        {
            switch ((u32)property->name)
            {
                case OpenGexProperty::Attrib:
                {
                    if (!AreStringsEqual(property->as.stringValue, "light"))
                        goto LError; // FIXME - I guess technically you could put a junk value here as long as you repeat the "attrib" property with the correct one sometime after

                    isAttribLight = true;
                } break;

                default:
                    goto LError;
            }
        }

        if (!isAttribLight)
            goto LError;
    }

    //
    // Substructures

    if (structure->children.count != 1)
        goto LError;

    Structure * child = structure->children + 0;
    if (child->type != StructureType::OpenDdlPrimitive)
        goto LError;

    PrimitiveStructure * primitive = &child->primitive;
    if (primitive->primitiveType != PrimitiveStructureType::Subarray)
        goto LError;
    
    if (primitive->dataType != PrimitiveDataType::Float32)
        goto LError;

    f32 * values = (f32 *)primitive->values;
    Subarray * subarray = &primitive->subarray;
    if (subarray->cntValuePerSubarray == 3)
    {
        poResult->type = Color::Type::Rgb;
        poResult->rgb = Vec3(values[0], values[1], values[2]);
    }
    else if (subarray->cntValuePerSubarray == 4)
    {
        poResult->type = Color::Type::Rgba;
        poResult->rgba = Vec4(values[0], values[1], values[2], values[3]);
    }
    else
        goto LError;

    return true;
    
LError:
    return false;
}

internal bool
ReadParam(
    DerivedStructure * structure,
    Param * poResult,
    OpenGexType containingType)
{
    //
    // Initialize

    *poResult = {};
    
    if (containingType != OpenGexType::Material)
    {
        AssertTodo;
        goto LError;
    }

    //
    // Properties
    
    PropertyListReader propReader = CreatePropertyListReader(structure->properties);
    for (Property * property = NextProperty(&propReader);
         property;
         property = NextProperty(&propReader))
    {
        switch ((u32)property->name)
        {
            case OpenGexProperty::Attrib:
            {
                if (AreStringsEqual(property->as.stringValue, "specular_power"))
                {
                    poResult->materialAttrib = MaterialAttrib::SpecularPower;
                }
                else if (AreStringsEqual(property->as.stringValue, "opacity"))
                {
                    poResult->materialAttrib = MaterialAttrib::Opacity;
                }
                else if (AreStringsEqual(property->as.stringValue, "height_scale"))
                {
                    poResult->materialAttrib = MaterialAttrib::HeightScale;
                }
                else if (AreStringsEqual(property->as.stringValue, "roughness"))
                {
                    poResult->materialAttrib = MaterialAttrib::Roughness;
                }
                else if (AreStringsEqual(property->as.stringValue, "metalness"))
                {
                    poResult->materialAttrib = MaterialAttrib::Metalness;
                }
                else if (AreStringsEqual(property->as.stringValue, "clearcoat_roughness"))
                {
                    poResult->materialAttrib = MaterialAttrib::ClearCoatRoughness;
                }
                else if (AreStringsEqual(property->as.stringValue, "sheen_roughness"))
                {
                    poResult->materialAttrib = MaterialAttrib::SheenRoughness;
                }
                else if (AreStringsEqual(property->as.stringValue, "ior"))
                {
                    poResult->materialAttrib = MaterialAttrib::Ior;
                }
                else
                    goto LError; // FIXME - I guess technically you could put a junk value here as long as you repeat the "attrib" property with the correct one sometime after
            } break;

            default:
                goto LError;
        }
    }

    //
    // Data

    if (structure->children.count != 1)
        goto LError;

    if (structure->children[0].type != StructureType::OpenDdlPrimitive)
        goto LError;

    PrimitiveStructure * primitive = &structure->children[0].primitive;
    if (primitive->primitiveType != PrimitiveStructureType::Flat)
        goto LError;

    if (primitive->dataType != PrimitiveDataType::Float32)
        goto LError;

    if (primitive->cntValue != 1)
        goto LError;

    poResult->value = ((float *)primitive->values)[0];
    
    return true;
    
LError:
    return false;
}

internal bool
ReadMaterial(
    DerivedStructure * structure,
    Material * poResult,
    MemoryRegion memory)
{
    //
    // Initialize

    *poResult = {};

    poResult->diffuseColor = { MaterialAttrib::Diffuse, Color::Type::Rgb, Vec3Fill(1) };
    poResult->specularColor = { MaterialAttrib::Specular, Color::Type::Rgb, Vec3Fill(0) };
    poResult->specularPowerParam = { MaterialAttrib::SpecularPower, 1.0f };

    PropertyListReader propReader = CreatePropertyListReader(structure->properties);
    for (Property * property = NextProperty(&propReader);
         property;
         property = NextProperty(&propReader))
    {
        switch ((u32)property->name)
        {
            case OpenGexProperty::TwoSided:
            {
                poResult->isTwoSided = property->as.boolValue;
            } break;

            default:
                goto LError;
        }
    }

    //
    // Substructures

    StructureListReader childReader = CreateStructureListReader(structure->children);
    for (Structure * child = NextStructure(&childReader);
         child;
         child = NextStructure(&childReader))
    {
        switch ((u32)child->type)
        {
            case OpenGexType::Name:
            {
                if (poResult->name.bytes)
                    goto LError;
                
                if (!ReadName(&child->derived, &poResult->name, memory))
                    goto LError;
            } break;

            case OpenGexType::Color:
            {
                Color color;
                if (!ReadColor(&child->derived, &color, OpenGexType::Material))
                    goto LError;

                switch ((u32)color.materialAttrib)
                {
                    case MaterialAttrib::Diffuse:  poResult->diffuseColor  = color; break;
                    case MaterialAttrib::Specular: poResult->specularColor = color; break;
                        
                    case MaterialAttrib::Emission:
                    case MaterialAttrib::Transparency:
                    case MaterialAttrib::ClearCoat:
                    case MaterialAttrib::Sheen:
                        AssertTodo; // fallthrough
                        
                    default:
                        goto LError;
                }
            } break;

            case OpenGexType::Param:
            {
                Param param;
                if (!ReadParam(&child->derived, &param, OpenGexType::Material))
                    goto LError;

                switch ((u32)param.materialAttrib)
                {
                    case MaterialAttrib::SpecularPower:  poResult->specularPowerParam = param; break;
                    case MaterialAttrib::Opacity:
                    case MaterialAttrib::HeightScale:
                    case MaterialAttrib::Roughness:
                    case MaterialAttrib::Metalness:
                    case MaterialAttrib::ClearCoatRoughness:
                    case MaterialAttrib::SheenRoughness:
                    case MaterialAttrib::Ior:
                        AssertTodo; // fallthrough
                        
                    default:
                        goto LError;
                }
            } break;

            case OpenGexType::Spectrum:
            {
                AssertTodo;
                goto LError;
            } break;

            case OpenGexType::Texture:
            {
                AssertTodo;
                goto LError;
            } break;

            default:
                goto LError;
        }
    }

    return true;

LError:
    return false;
}

internal bool
ReadTransform(DerivedStructure * structure, Transform * poResult)
{
    //
    // Initialize
    
    poResult->isObjectTransform = false;
    poResult->matrix = Mat4Identity();

    //
    // Properties

    // FIXME - "The same property can be specified more than once in the same property list, and in such a case, all but the final value specified for the same property must be ignored"
    if (structure->properties.count > 1)
        goto LError;

    if (structure->properties.count == 1)
    {
        Property * property = structure->properties + 0;
        if ((OpenGexProperty)property->name != OpenGexProperty::Object)
            goto LError;

        poResult->isObjectTransform = property->as.boolValue;
    }

    //
    // Substructures
    
    if (structure->children.count != 1)
        goto LError;

    Structure * child = structure->children + 0;
    if (child->type != StructureType::OpenDdlPrimitive)
        goto LError;

    PrimitiveStructure * primitive = &child->primitive;
    if (primitive->primitiveType != PrimitiveStructureType::Subarray)
        goto LError;

    if (primitive->dataType != PrimitiveDataType::Float32)
        goto LError;

    Subarray * subarray = &primitive->subarray;
    if (subarray->cntSubarray != 1)
        goto LError;

    f32 * values = (f32 *)primitive->values;
    switch (primitive->cntValue)
    {
        case 4:
        {
            poResult->matrix = {{
                    { values[0], values[2], 0, 0 },
                    { values[1], values[3], 0, 0 },
                    { 0,         0,         1, 0 },
                    { 0,         0,         0, 1 },
                }};
            
        } break;
        case 6:
        {
            poResult->matrix = {{
                    { values[0], values[2], 0, values[4] },
                    { values[1], values[3], 0, values[5] },
                    { 0,         0,         1, 0 },
                    { 0,         0,         0, 1 },
                }};
            
        } break;
        
        case 9:
        {
            poResult->matrix = {{
                    { values[0], values[3], values[6], 0 },
                    { values[1], values[4], values[7], 0 },
                    { values[2], values[5], values[8], 0 },
                    { 0,         0,         0,         1 },
                }};
        } break;
        
        case 12:
        {
            poResult->matrix = {{
                    { values[0], values[3], values[6], values[9] },
                    { values[1], values[4], values[7], values[10] },
                    { values[2], values[5], values[8], values[11] },
                    { 0,         0,         0,         1 },
                }};
        } break;
        
        case 16:
        {
            poResult->matrix = {{
                    { values[0], values[4], values[8],  values[12] },
                    { values[1], values[5], values[9],  values[13] },
                    { values[2], values[6], values[10], values[14] },
                    { values[3], values[7], values[11], values[15] },
                }};
        } break;
            
        default:
            goto LError;
    }

    return true;
    
LError:
    return false;
}

internal bool
ReadIndexArray(DerivedStructure * structure, IndexArray * poResult, Mesh::PrimitiveType containingMeshPrimitiveType)
{
    //
    // Initialize
    
    *poResult = {};
    poResult->winding = WindingOrder::Ccw;

    //
    // Properties
    
    PropertyListReader propReader = CreatePropertyListReader(structure->properties);
    for (Property * property = NextProperty(&propReader);
         property;
         property = NextProperty(&propReader))
    {
        switch ((u32)property->name)
        {
            case OpenGexProperty::Material:
            {
                poResult->materialIndex = property->as.u32Value;
            } break;
            
            case OpenGexProperty::Restart:
            {
                if (containingMeshPrimitiveType != Mesh::PrimitiveType::TriangleStrip &&
                    containingMeshPrimitiveType != Mesh::PrimitiveType::LineStrip)
                    goto LError;
                
                poResult->stripRestartIndex = property->as.u64Value;
            } break;

            case OpenGexProperty::Front:
            {
                if (AreStringsEqual(property->as.stringValue, "ccw"))
                {
                    poResult->winding = WindingOrder::Ccw;
                }
                else if (AreStringsEqual(property->as.stringValue, "cw"))
                {
                    poResult->winding = WindingOrder::Cw;
                }
                else
                    goto LError;
            } break;
            
            default:
                goto LError;
        }
    }


    if (structure->children.count != 1)
        goto LError;

    Structure * child = structure->children + 0;
    if (child->type != StructureType::OpenDdlPrimitive)
        goto LError;

    poResult->data = &child->primitive;
    if (poResult->data->dataType < PrimitiveDataType::UInt8 ||
        poResult->data->dataType > PrimitiveDataType::UInt64)
        goto LError;

    if (poResult->data->primitiveType != PrimitiveStructureType::Flat &&
        poResult->data->primitiveType != PrimitiveStructureType::Subarray)
        goto LError;

    if (poResult->data->primitiveType == PrimitiveStructureType::Subarray)
    {
        Subarray * subarray = &poResult->data->subarray;
        if (subarray->cntValuePerSubarray < 2 || subarray->cntValuePerSubarray > 4)
            goto LError;
    }

    return true;
    
LError:
    return false;
}

internal bool
ReadVertexArray(DerivedStructure * structure, VertexArray * poResult)
{
    //
    // Initialize
    
    *poResult = {};

    //
    // Properties
    
    PropertyListReader propReader = CreatePropertyListReader(structure->properties);
    for (Property * property = NextProperty(&propReader);
         property;
         property = NextProperty(&propReader))
    {
        switch ((u32)property->name)
        {
            case OpenGexProperty::Attrib:
            {
                if (AreStringsEqual(property->as.stringValue, "position"))
                {
                    poResult->attributeType = VertexArray::AttributeType::Position;
                }
                else if (AreStringsEqual(property->as.stringValue, "normal"))
                {
                    poResult->attributeType = VertexArray::AttributeType::Normal;
                }
                else if (AreStringsEqual(property->as.stringValue, "tangent"))
                {
                    poResult->attributeType = VertexArray::AttributeType::Tangent;
                }
                else if (AreStringsEqual(property->as.stringValue, "bitangent"))
                {
                    poResult->attributeType = VertexArray::AttributeType::Bitangent;
                }
                else if (AreStringsEqual(property->as.stringValue, "color"))
                {
                    poResult->attributeType = VertexArray::AttributeType::Color;
                }
                else if (AreStringsEqual(property->as.stringValue, "texcoord"))
                {
                    poResult->attributeType = VertexArray::AttributeType::TexCoord;
                }
                else
                    goto LNoOp;
            } break;
            
            case OpenGexProperty::Index:
            {
                poResult->attributeIndex = property->as.u32Value;
            } break;

            case OpenGexProperty::Morph:
            {
                poResult->morphIndex = property->as.u32Value;
            } break;
            
            default:
                goto LError;
        }
    }

    // NOTE - Spec isn't clear on how to handle missing attrib property...
    if (poResult->attributeType == VertexArray::AttributeType::Nil)
        goto LNoOp;

    if (structure->children.count != 1)
        goto LError;

    Structure * child = structure->children + 0;
    if (child->type != StructureType::OpenDdlPrimitive)
        goto LError;

    poResult->data = &child->primitive;
    if (poResult->data->dataType != PrimitiveDataType::Float32 &&
        poResult->data->dataType != PrimitiveDataType::Float64)
        goto LError;

    if (poResult->data->primitiveType != PrimitiveStructureType::Flat &&
        poResult->data->primitiveType != PrimitiveStructureType::Subarray)
        goto LError;

    if (poResult->data->primitiveType == PrimitiveStructureType::Subarray)
    {
        Subarray * subarray = &poResult->data->subarray;
        if (subarray->cntValuePerSubarray < 2 || subarray->cntValuePerSubarray > 4)
            goto LError;
    }

    return true;

LNoOp:
    // Nil data/attribute + true return value signals that this vertex array should be ignored without error
    *poResult = {};
     return true;
    
LError:
    return false;
}

internal bool
ReadMesh(DerivedStructure * structure, Mesh * poResult, MemoryRegion memory)
{
    //
    // Initialize

    *poResult = {};
    poResult->primitiveType = Mesh::PrimitiveType::Triangles;

    //
    // Properties

    PropertyListReader propReader = CreatePropertyListReader(structure->properties);
    for (Property * property = NextProperty(&propReader);
         property;
         property = NextProperty(&propReader))
    {
        switch ((u32)property->name)
        {
            case OpenGexProperty::Lod:
            {
                poResult->lod = property->as.u32Value;
            } break;
            
            case OpenGexProperty::Primitive:
            {
                if (AreStringsEqual(property->as.stringValue, "triangles"))
                {
                    poResult->primitiveType = Mesh::PrimitiveType::Triangles;
                }
                else if (AreStringsEqual(property->as.stringValue, "triangle_strip"))
                {
                    poResult->primitiveType = Mesh::PrimitiveType::TriangleStrip;
                }
                else if (AreStringsEqual(property->as.stringValue, "lines"))
                {
                    poResult->primitiveType = Mesh::PrimitiveType::Lines;
                }
                else if (AreStringsEqual(property->as.stringValue, "line_strip"))
                {
                    poResult->primitiveType = Mesh::PrimitiveType::LineStrip;
                }
                else if (AreStringsEqual(property->as.stringValue, "points"))
                {
                    poResult->primitiveType = Mesh::PrimitiveType::Points;
                }
                else if (AreStringsEqual(property->as.stringValue, "quads"))
                {
                    poResult->primitiveType = Mesh::PrimitiveType::Quads;
                }
                else
                    goto LError;
            } break;
            
            default:
                goto LError;
        }
    }

    //
    // Substructures

    DynArray<VertexArray> vertexArrays(memory);
    DynArray<IndexArray> indexArrays(memory);
    
    StructureListReader childReader = CreateStructureListReader(structure->children);
    for (Structure * child = NextStructure(&childReader);
         child;
         child = NextStructure(&childReader))
    {
        switch ((u32)child->type)
        {
            case OpenGexType::VertexArray:
            {
                VertexArray * va = AppendNew(&vertexArrays);
                if (!ReadVertexArray(&child->derived, va))
                    goto LError;
            } break;

            case OpenGexType::IndexArray:
            {
                IndexArray * ia = AppendNew(&indexArrays);
                if (!ReadIndexArray(&child->derived, ia, poResult->primitiveType))
                    goto LError;
            } break;

            case OpenGexType::Skin:
            {
                AssertTodo;
                goto LError;
            } break;

            default:
                goto LError;
        }
    }

    if (vertexArrays.count < 1)
        goto LError;
    
    poResult->vertexArrays = MakeSlice(vertexArrays);
    poResult->indexArrays = MakeSlice(indexArrays);
    return true;

LError:
    return false;
}

internal bool
ReadGeometryProperties(DerivedStructure * structure, GeometryFlags * poResult)
{
    PropertyListReader propReader = CreatePropertyListReader(structure->properties);
    for (Property * property = NextProperty(&propReader);
         property;
         property = NextProperty(&propReader))
    {
        GeometryFlags flag = {};
        switch ((u32)property->name)
        {
            case OpenGexProperty::Visible:    flag = GeometryFlags::Visible;    break;
            case OpenGexProperty::Shadow:     flag = GeometryFlags::Shadow;     break;
            case OpenGexProperty::MotionBlur: flag = GeometryFlags::MotionBlur; break;
            default:
                goto LError;
        }

        bool isSet = property->as.boolValue;
        if (isSet)
        {
            *poResult |= flag;
        }
        else
        {
            *poResult &= ~flag;
        }
    }

    return true;

LError:
    return false;
}

internal bool
ReadGeometryNode(
    DerivedStructure * structure,
    GeometryNode * poResult,
    ReferenceTracker * refTracker, // super-hack!
    MemoryRegion memory)
{
    //
    // Initialize
    
    *poResult = {};
    poResult->nodeTransform = Mat4Identity();
    poResult->objectTransform = Mat4Identity();

    //
    // Properties

    if (!ReadGeometryProperties(structure, &poResult->flags))
        goto LError;

    //
    // Substructures

    bool hasObjectRef = false;

    struct MaterialRefFixup
    {
        uint iUref;
        uint iMaterial;
    };
    
    DynArray<Material *> materials(memory);
    DynArray<MaterialRefFixup> materialRefFixups(memory);
    
    StructureListReader childReader = CreateStructureListReader(structure->children);
    for (Structure * child = NextStructure(&childReader);
         child;
         child = NextStructure(&childReader))
    {
        switch ((u32)child->type)
        {
            case OpenGexType::Name:
            {
                if (poResult->name.bytes)
                    goto LError;
                
                if (!ReadName(&child->derived, &poResult->name, memory))
                    goto LError;
            } break;

            case OpenGexType::ObjectRef:
            case OpenGexType::MaterialRef:
            {
                bool isObjectRef = ((OpenGexType)child->type == OpenGexType::ObjectRef);
                if (isObjectRef && hasObjectRef)
                    goto LError;
                
                if (child->derived.children.count != 1)
                    goto LError;

                if (child->derived.children[0].type != StructureType::OpenDdlPrimitive)
                    goto LError;

                PrimitiveStructure * ref = &child->derived.children[0].primitive;
                if (ref->primitiveType != PrimitiveStructureType::Flat)
                    goto LError;

                if (ref->dataType != PrimitiveDataType::Ref)
                    goto LError;

                if (ref->cntValue != 1)
                    goto LError;

                RefValue refValue = ((RefValue *)ref->values)[0];

                uint iUref = refTracker->urefs.count;
                UnresolvedReference * uref = AppendNew(&refTracker->urefs);
                uref->refType = (OpenGexType)child->type;
                uref->refValue = refValue;
                uref->next = {};

                // @HACK, FIXME - This is wrong! We are assuming that the GeometryNode passed into us is the final
                //  address that it will be at, and we are setting the unresolved reference accordingly. However,
                //  the GeometryNode might be in a DynArray, which means the ptrToResolve we are setting might
                //  actually move! I'm totally punting on this right now, since my test file only has one node
                //  so it won't break. But 2 geometry nodes will definitely break things until I decide a better
                //  way to resolve refs!
                
                if (isObjectRef)
                {
                    hasObjectRef = true;
                    uref->ptrToResolve = (void **)(&poResult->geometryObject);
                }
                else
                {
                    // @Gross - Can't set the ref ptr to resolve to until we are done building the Material * DynArray...
                    //  so we keep track of the fixup and defer it until the Material pointers are in place
                    //  This is like a mini-version of the FIXME problem above...

                    uint iMaterial = materials.count;
                    AppendNew(&materials);

                    MaterialRefFixup fixup;
                    fixup.iUref = iUref;
                    fixup.iMaterial = iMaterial;
                    Append(&materialRefFixups, fixup);
                }
            } break;

            case OpenGexType::MorphWeight:
            {
                AssertTodo;
                goto LError;
            } break;

            case OpenGexType::Transform:
            {
                Transform transform;
                if (!ReadTransform(&child->derived, &transform))
                    goto LError;

                if (transform.isObjectTransform)
                {
                    poResult->nodeTransform = transform.matrix * poResult->nodeTransform;
                }
                else
                {
                    poResult->objectTransform = transform.matrix * poResult->objectTransform;
                }
            } break;

            case OpenGexType::Translation:
            {
                AssertTodo;
                goto LError;
            } break;

            case OpenGexType::Rotation:
            {
                AssertTodo;
                goto LError;
            } break;

            case OpenGexType::Scale:
            {
                AssertTodo;
                goto LError;
            } break;

            case OpenGexType::Animation:
            {
                AssertTodo;
                goto LError;
            } break;

            case OpenGexType::Node:
            {
                AssertTodo;
                goto LError;
            } break;

            case OpenGexType::BoneNode:
            {
                AssertTodo;
                goto LError;
            } break;

            case OpenGexType::GeometryNode:
            {
                AssertTodo;
                goto LError;
            } break;

            case OpenGexType::CameraNode:
            {
                AssertTodo;
                goto LError;
            } break;

            case OpenGexType::LightNode:
            {
                AssertTodo;
                goto LError;
            } break;

            default:
                goto LError;
        }
    }

    if (!hasObjectRef)
        goto LError;
    
    poResult->materials = MakeSlice(materials);

    // Fix-up material ref pointers

    for (MaterialRefFixup fixup : materialRefFixups)
    {
        refTracker->urefs[fixup.iUref].ptrToResolve = (void **)(poResult->materials + fixup.iMaterial);
    }
    
    return true;
    
LError:
    return false;
}

internal bool
ReadGeometryObject(
    DerivedStructure * structure,
    GeometryObject * poResult,
    MemoryRegion memory)
{
    //
    // Initialize
    
    *poResult = {};

    //
    // Properties

    if (!ReadGeometryProperties(structure, &poResult->flags))
        goto LError;

    //
    // Substructures

    DynArray<Mesh> meshes(memory);
    
    StructureListReader childReader = CreateStructureListReader(structure->children);
    for (Structure * child = NextStructure(&childReader);
         child;
         child = NextStructure(&childReader))
    {
        switch ((u32)child->type)
        {
            case OpenGexType::Mesh:
            {
                Mesh * mesh = AppendNew(&meshes);
                if (!ReadMesh(&child->derived, mesh, memory))
                    goto LError;
            } break;

            case OpenGexType::Morph:
            {
                AssertTodo;
                goto LError;
            } break;

            default:
                goto LError;
        }
    }

    if (meshes.count < 1)
        goto LError;

    poResult->meshes = MakeSlice(meshes);
    return true;
    
LError:
    return false;
}

internal OpenGexResult
ImportOpenGexFileContents(char * fileContents, MemoryRegion memory)
{
    OpenGexResult result = {};
    
    ReferenceTracker refTracker = {};
    refTracker.urefs = DynArray<UnresolvedReference>(memory);

    //
    // Import
    
    DerivedMappings mappings;
    mappings.typeMappings = OPENGEX::typeMappings;
    mappings.cntTypeMapping = ArrayLen(OPENGEX::typeMappings);
    mappings.skipUnrecognizedTypes = true;
    mappings.propertyMappings = OPENGEX::propertyMappings;
    mappings.cntPropertyMapping = ArrayLen(OPENGEX::propertyMappings);
    mappings.skipUnrecognizedProperties = true;
    
    OpenDdlResult openDdl = ImportOpenDdlFileContents(fileContents, memory, mappings);
    if (openDdl.topLevel.count <= 0)
        goto LError;

    //
    // Set up reference tracker
    
    refTracker.mapStructureIdToRefTarget = (void **)Allocate(memory, sizeof(void *) * (openDdl.cntStructureTotal + 1));
    ZeroMemory(refTracker.mapStructureIdToRefTarget, sizeof(void *) * (openDdl.cntStructureTotal + 1));
        
    //
    // Read metrics first
        
    ForEnum(Metric::Key, key)
    {
        result.metrics[(u32)key] = DefaultMetric(key);
    }
    
    StructureListReader reader = CreateStructureListReader(openDdl.topLevel);
    for (Structure * structure = MatchNextStructure(&reader, (StructureType)OpenGexType::Metric);
         structure;
         structure = MatchNextStructure(&reader, (StructureType)OpenGexType::Metric))
    {
        DerivedStructure * metricStructure = &structure->derived;
        Metric metric;
        if (!ReadMetric(metricStructure, &metric))
            goto LError;
        
        result.metrics[(u32)metric.key] = metric;
    }

    //
    // Read other top level structures

    DynArray<GeometryNode> geometryNodes(memory);
    DynArray<GeometryObject> geometryObjects(memory);
    DynArray<Material> materials(memory);

    // @Gross
    DynArray<StructureId> mapIGeometryNodeToStructureId(memory);
    DynArray<StructureId> mapIGeometryObjectToStructureId(memory);
    DynArray<StructureId> mapIMaterialToStructureId(memory);
    
    for (Structure * structure = NextStructure(&reader);
         structure;
         structure = NextStructure(&reader))
    {
        switch ((u32)structure->type)
        {
            case OpenGexType::GeometryNode:
            {
                GeometryNode * geometryNode = AppendNew(&geometryNodes);
                Assert(geometryNodes.count == 1); // See @HACK, FIXME comment in ReadGeometryNode
                if (!ReadGeometryNode(&structure->derived, geometryNode, &refTracker, memory))
                    goto LError;

                Append(&mapIGeometryNodeToStructureId, structure->id);
            } break;

            case OpenGexType::GeometryObject:
            {
                GeometryObject * geometryObject = AppendNew(&geometryObjects);
                if (!ReadGeometryObject(&structure->derived, geometryObject, memory))
                    goto LError;

                Append(&mapIGeometryObjectToStructureId, structure->id);
            } break;

            case OpenGexType::Material:
            {
                Material * material = AppendNew(&materials);
                if (!ReadMaterial(&structure->derived, material, memory))
                    goto LError;

                Append(&mapIMaterialToStructureId, structure->id);
            } break;

            default:
            {
                // TODO (or maybe malformed file)
                AssertTodo;
                goto LError;
            } break;
        }
    }

    //
    // Put targets in reference tracker
    
    result.geometryNodes = MakeSlice(geometryNodes);
    result.geometryObjects = MakeSlice(geometryObjects);
    result.materials = MakeSlice(materials);

    for (int iGeoNode = 0; iGeoNode < geometryNodes.count; iGeoNode++)
    {
        StructureId structureId = mapIGeometryNodeToStructureId[iGeoNode];
        refTracker.mapStructureIdToRefTarget[(int)structureId] = geometryNodes + iGeoNode;
    }
    
    for (int iGeoObj = 0; iGeoObj < geometryObjects.count; iGeoObj++)
    {
        StructureId structureId = mapIGeometryObjectToStructureId[iGeoObj];
        refTracker.mapStructureIdToRefTarget[(int)structureId] = geometryObjects + iGeoObj;
    }
    
    for (int iMaterial = 0; iMaterial < materials.count; iMaterial++)
    {
        StructureId structureId = mapIMaterialToStructureId[iMaterial];
        refTracker.mapStructureIdToRefTarget[(int)structureId] = materials + iMaterial;
    }
    
    //
    // Resolve references
    
    for (UnresolvedReference uref : refTracker.urefs)
    {
        // TODO - Type check that e.g. GeometryNode's object reference is a GeometryObject. At resolve
        //  time, we don't know the context of the reference, so we probably want to do this check when
        //  pushing the unresolved reference
        
        Structure * openDdlTarget = uref.refValue.refPtr;
        void * openGexTarget = refTracker.mapStructureIdToRefTarget[(u32)openDdlTarget->id];
        if (!openGexTarget)
            goto LError;

        *uref.ptrToResolve = openGexTarget;
    }

    result.success = true;
    return result;

LError:
    result.success = false;
    return result;
}

}
