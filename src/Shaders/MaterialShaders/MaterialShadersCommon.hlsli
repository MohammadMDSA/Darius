#ifndef __MATERIAL_SHADERS_COMMON_HLSLI__
#define __MATERIAL_SHADERS_COMMON_HLSLI__


#define ParamDeclaration(type) \
ConstantBuffer<MaterialConstantsType> MaterialConstants; \
float __main__() : SV_TARGET \
{ \
    (MaterialConstants); \
    return 0.f; \
}

#endif