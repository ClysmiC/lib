#include "lib/LL/ll.h"

enum class OpenGexType : u32
{
    // @Sync - order
    
    Nil              = StructureType::Nil,
    
    OpenDdlPrimitive = StructureType::OpenDdlPrimitive,
    UnknownExtension = StructureType::UnrecognizedExtension,
    
    Animation        = StructureType::DerivedTypeStart,
    Atten,
    BoneCountArray,
    BoneIndexArray,
    BoneNode,        // top level allowed
    BoneRefArray,
    BoneWeightArray,
    CameraNode,      // top level allowed
    CameraObject,    // top level only
    Clip,            // top level only
    Color,
    GeometryNode,    // top level allowed
    GeometryObject,  // top level only
    IndexArray,
    Key,
    LightNode,       // top level allowed
    LightObject,     // top level only
    Material,        // top level only
    MaterialRef,
    Mesh,
    Metric,          // top level only
    Morph,
    MorphWeight,
    Name,
    Node,            // top level allowed
    ObjectRef,
    Param,
    Rotation,
    Scale,
    Skeleton,
    Skin,
    Spectrum,
    Texture,
    Time,
    Track,
    Transform,
    Translation,
    Value,
    VertexArray,

    EnumCount
};

namespace OPENGEX
{

static const DerivedTypeMapping typeMappings[] = {
    { StringFromLit("Animation"),       (u32)OpenGexType::Animation },
    { StringFromLit("Atten"),           (u32)OpenGexType::Atten },
    { StringFromLit("BoneCountArray"),  (u32)OpenGexType::BoneCountArray },
    { StringFromLit("BoneIndexArray"),  (u32)OpenGexType::BoneIndexArray },
    { StringFromLit("BoneNode"),        (u32)OpenGexType::BoneNode },
    { StringFromLit("BoneRefArray"),    (u32)OpenGexType::BoneRefArray },
    { StringFromLit("BoneWeightArray"), (u32)OpenGexType::BoneWeightArray },
    { StringFromLit("CameraNode"),      (u32)OpenGexType::CameraNode },
    { StringFromLit("CameraObject"),    (u32)OpenGexType::CameraObject },
    { StringFromLit("Clip"),            (u32)OpenGexType::Clip },
    { StringFromLit("Color"),           (u32)OpenGexType::Color },
    { StringFromLit("GeometryNode"),    (u32)OpenGexType::GeometryNode },
    { StringFromLit("GeometryObject"),  (u32)OpenGexType::GeometryObject },
    { StringFromLit("IndexArray"),      (u32)OpenGexType::IndexArray },
    { StringFromLit("Key"),             (u32)OpenGexType::Key },
    { StringFromLit("LightNode"),       (u32)OpenGexType::LightNode },
    { StringFromLit("LightObject"),     (u32)OpenGexType::LightObject },
    { StringFromLit("Material"),        (u32)OpenGexType::Material },
    { StringFromLit("MaterialRef"),     (u32)OpenGexType::MaterialRef },
    { StringFromLit("Mesh"),            (u32)OpenGexType::Mesh },
    { StringFromLit("Metric"),          (u32)OpenGexType::Metric },
    { StringFromLit("Morph"),           (u32)OpenGexType::Morph },
    { StringFromLit("MorphWeight"),     (u32)OpenGexType::MorphWeight },
    { StringFromLit("Name"),            (u32)OpenGexType::Name },
    { StringFromLit("Node"),            (u32)OpenGexType::Node },
    { StringFromLit("ObjectRef"),       (u32)OpenGexType::ObjectRef },
    { StringFromLit("Param"),           (u32)OpenGexType::Param },
    { StringFromLit("Rotation"),        (u32)OpenGexType::Rotation },
    { StringFromLit("Scale"),           (u32)OpenGexType::Scale },
    { StringFromLit("Skeleton"),        (u32)OpenGexType::Skeleton },
    { StringFromLit("Skin"),            (u32)OpenGexType::Skin },
    { StringFromLit("Spectrum"),        (u32)OpenGexType::Spectrum },
    { StringFromLit("Texture"),         (u32)OpenGexType::Texture },
    { StringFromLit("Time"),            (u32)OpenGexType::Time },
    { StringFromLit("Track"),           (u32)OpenGexType::Track },
    { StringFromLit("Transform"),       (u32)OpenGexType::Transform },
    { StringFromLit("Translation"),     (u32)OpenGexType::Translation },
    { StringFromLit("Value"),           (u32)OpenGexType::Value },
    { StringFromLit("VertexArray"),     (u32)OpenGexType::VertexArray },
};
StaticAssert(ArrayLen(typeMappings) == (u32)OpenGexType::EnumCount - (u32)StructureType::DerivedTypeStart);

} // namespace OPENGEX

enum class OpenGexProperty : u32
{
    Nil                   = PropertyName::Nil,

    UnrecognizedExtension = PropertyName::UnrecognizedExtension,

    Clip                  = PropertyName::DerivedPropertyStart,
    Begin,
    End,
    Kind,
    Curve,
    Index,
    Attrib,
    Visible,
    Shadow,
    MotionBlur,
    Material,
    Restart,
    Front,
    Type,
    TwoSided,
    Lod,
    Primitive,
    Key,
    Base,
    Object,
    Min,
    Max,
    TexCoord,
    Swizzle,
    XAddress,
    YAddress,
    ZAddress,
    Border,
    Morph,

    EnumCount
};

namespace OPENGEX
{

static const DerivedPropertyMapping propertyMappings[] = {
    { StringFromLit("clip"),        (u32)OpenGexProperty::Clip,       PrimitiveDataType::UInt32 },
    { StringFromLit("begin"),       (u32)OpenGexProperty::Begin,      PrimitiveDataType::Float32 },
    { StringFromLit("end"),         (u32)OpenGexProperty::End,        PrimitiveDataType::Float32 },
    { StringFromLit("kind"),        (u32)OpenGexProperty::Kind,       PrimitiveDataType::String },
    { StringFromLit("curve"),       (u32)OpenGexProperty::Curve,      PrimitiveDataType::String },
    { StringFromLit("index"),       (u32)OpenGexProperty::Index,      PrimitiveDataType::UInt32 },
    { StringFromLit("attrib"),      (u32)OpenGexProperty::Attrib,     PrimitiveDataType::String },
    { StringFromLit("visible"),     (u32)OpenGexProperty::Visible,    PrimitiveDataType::Bool },
    { StringFromLit("shadow"),      (u32)OpenGexProperty::Shadow,     PrimitiveDataType::Bool },
    { StringFromLit("motion_blur"), (u32)OpenGexProperty::MotionBlur, PrimitiveDataType::Bool },
    { StringFromLit("material"),    (u32)OpenGexProperty::Material,   PrimitiveDataType::UInt32 },
    { StringFromLit("restart"),     (u32)OpenGexProperty::Restart,    PrimitiveDataType::UInt64 },
    { StringFromLit("front"),       (u32)OpenGexProperty::Front,      PrimitiveDataType::String },
    { StringFromLit("type"),        (u32)OpenGexProperty::Type,       PrimitiveDataType::String },
    { StringFromLit("two_sided"),   (u32)OpenGexProperty::TwoSided,   PrimitiveDataType::Bool },
    { StringFromLit("lod"),         (u32)OpenGexProperty::Lod,        PrimitiveDataType::UInt32 },
    { StringFromLit("primitive"),   (u32)OpenGexProperty::Primitive,  PrimitiveDataType::String },
    { StringFromLit("key"),         (u32)OpenGexProperty::Key,        PrimitiveDataType::String },
    { StringFromLit("base"),        (u32)OpenGexProperty::Base,       PrimitiveDataType::UInt32 },
    { StringFromLit("object"),      (u32)OpenGexProperty::Object,     PrimitiveDataType::Bool },
    { StringFromLit("min"),         (u32)OpenGexProperty::Min,        PrimitiveDataType::UInt32 },
    { StringFromLit("max"),         (u32)OpenGexProperty::Max,        PrimitiveDataType::UInt32 },
    { StringFromLit("texcoord"),    (u32)OpenGexProperty::TexCoord,   PrimitiveDataType::UInt32 },
    { StringFromLit("swizzle"),     (u32)OpenGexProperty::Swizzle,    PrimitiveDataType::String },
    { StringFromLit("x_address"),   (u32)OpenGexProperty::XAddress,   PrimitiveDataType::String },
    { StringFromLit("y_address"),   (u32)OpenGexProperty::YAddress,   PrimitiveDataType::String },
    { StringFromLit("z_address"),   (u32)OpenGexProperty::ZAddress,   PrimitiveDataType::String },
    { StringFromLit("border"),      (u32)OpenGexProperty::Border,     PrimitiveDataType::String },
    { StringFromLit("morph"),       (u32)OpenGexProperty::Morph,      PrimitiveDataType::UInt32 },
};
StaticAssert(ArrayLen(propertyMappings) == (u32)OpenGexProperty::EnumCount - (u32)PropertyName::DerivedPropertyStart);

} // namespace OPENGEX
    
struct Metric
{
    enum class Key : u8
    {
        Nil = 0,

        Distance,
        Angle,
        Time,
        Up,
        Forward,
        Red,
        Green,
        Blue,
        White,

        EnumCount
    };

    enum class Up : u8
    {
        Nil = 0,

        Y,
        Z,

        EnumCount
    };

    enum class Forward : u8
    {
        Nil = 0,
        
        XPos,
        XNeg,
        YPos,
        YNeg,
        ZPos,
        ZNeg,

        EnumCount
    };

    Key key;
    union
    {
        f32 distanceValue;
        f32 angleValue;
        f32 timeValue;
        Up upValue;
        Forward forwardValue;
        f32 redValue[2];
        f32 greenValue[2];
        f32 blueValue[2];
        f32 whiteValue[2];
    };
};
DefineEnumOps(Metric::Key, u8);

inline Metric
DefaultMetric(Metric::Key key)
{
    Metric result;
    result.key = key;
    switch (result.key)
    {
        case Metric::Key::Distance: result.distanceValue = 1.0f; break;
        case Metric::Key::Angle:    result.angleValue = 1.0f; break;
        case Metric::Key::Time:     result.timeValue = 1.0f; break;
        case Metric::Key::Up:       result.upValue = Metric::Up::Z; break;
        case Metric::Key::Forward:  result.forwardValue = Metric::Forward::XPos; break;
        case Metric::Key::Red:      result.redValue[0] = 0.64f; result.redValue[1] = 0.33f; break;
        case Metric::Key::Green:    result.greenValue[0] = 0.3f; result.greenValue[1] = 0.6f; break;
        case Metric::Key::Blue:     result.blueValue[0] = 0.15f; result.blueValue[1] = 0.06f; break;
        case Metric::Key::White:    result.whiteValue[0] = 0.3127f; result.whiteValue[1] = 0.329f; break;

        case Metric::Key::Nil:
        case Metric::Key::EnumCount:
        default:
        {
            AssertFalseWarn;
            result.key = Metric::Key::Nil;
        } break;
    }

    return result;
}

struct Transform
{
    bool isObjectTransform;
    Mat4 matrix;
};

struct VertexArray
{
    enum class AttributeType : u8
    {
        Nil = 0,

        Position,
        Normal,
        Tangent,
        Bitangent,
        Color,
        TexCoord,

        EnumCount
    };

    //
    // Properties
    
    AttributeType attributeType;
    u32 attributeIndex; // Distinguishes between two VertexArrays with the same attribute type. E.g., a mesh with two sets of TexCoords
    u32 morphIndex;     // Unimplemented

    //
    // Data

    PrimitiveStructure * data;
};

struct IndexArray
{
    //
    // Properties

    u32 materialIndex;
    u64 stripRestartIndex; // 0 = don't split
    WindingOrder winding;
    
    //
    // Data

    PrimitiveStructure * data;
};

struct Mesh
{
    enum class PrimitiveType : u8
    {
        Nil = 0,

        Points,
        Lines,
        LineStrip,
        Triangles,
        TriangleStrip,
        Quads,

        EnumCount
    };

    u32 lod;
    PrimitiveType primitiveType;

    Slice<VertexArray> vertexArrays;
    Slice<IndexArray> indexArrays;
};

enum class GeometryFlags : u8
{
    Nil = 0,

    Visible    = 1 << 0,
    Shadow     = 1 << 1,
    MotionBlur = 1 << 2
};
DefineFlagOps(GeometryFlags, u8);

struct GeometryObject
{
    GeometryFlags flags;
    Slice<Mesh> meshes;
};

// TODO - Move defn into Material struct?
enum class MaterialAttrib : u8
{
    Nil = 0,
                        // C = Color, P = Param, S = Spectrum, T = Texture
    
    Diffuse,            // C, S, T
    Specular,           // C, S, T
    SpecularPower,      // P, T
    Emission,           // C, S, T
    Opacity,            // P, T
    Transparency,       // C, S, T
    HeightScale,        // P
    Roughness,          // P, T
    Metalness,          // P, T
    ClearCoat,          // C, S, T
    ClearCoatRoughness, // P, T
    Sheen,              // C, S, T
    SheenRoughness,     // P, T
    Ior,                // P

    EnumCount
};

struct Param
{
    MaterialAttrib materialAttrib;

    f32 value;
};

struct Color
{
    enum class Type : u8
    {
        Nil = 0,

        Rgb,
        Rgba,

        EnumCount
    };

    MaterialAttrib materialAttrib; // Nil if contained by non-Material
    
    Type type;
    union
    {
        Vec3 rgb;
        Vec4 rgba;
    };
};

struct Material
{
    //
    // Propertias

    bool isTwoSided;

    //
    // Data

    String name;

    Color diffuseColor;
    Color specularColor;
    Param specularPowerParam;
};

struct GeometryNode
{
    String name;
    GeometryFlags flags;

    Mat4 nodeTransform;
    Mat4 objectTransform;
    GeometryObject * geometryObject;

    Slice<Material *> materials;
};

struct OpenGexResult
{
    bool success;
    
    // All fields are top level
    //  Children structures can be accessed by following pointers in the top level structures

    // BoneNode * boneNodes;
    // uint cntBoneNode;

    // CameraNode * cameraNodes;
    // uint cntCameraNode;

    // CameraObject * cameraObjects;
    // uint cntCameraObject;

    // Clip * clips;
    // uint cntClip;
    
    Slice<GeometryNode> geometryNodes;
    
    Slice<GeometryObject> geometryObjects;
    
    // LightNode * lightNodes;
    // uint cntLightNode;

    // LightObject * lightObjects;
    // uint cntLightObject;

    Slice<Material> materials;

    Metric metrics[Metric::Key::EnumCount];

    // Node * nodes;
    // uint cntNode;
};

struct UnresolvedReference
{
    OpenGexType refType;
    RefValue refValue;
    void ** ptrToResolve;

    DefineLL1Node(UnresolvedReference);
    LL1Node next;
};

struct ReferenceTracker
{
    DynArray<UnresolvedReference> urefs;
    void ** mapStructureIdToRefTarget;
};
