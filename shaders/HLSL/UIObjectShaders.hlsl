struct UIVSInputVertex
{
    //Vertex attributes
    [[vk::location(0)]] float3 position : POSITION; // Vertex position
    [[vk::location(1)]] float2 texCoord : TEXCOORD0; // Vertex texture coordinates
    
    //Instance attributes
    [[vk::location(2)]] float3 objectPosition : TEXCOORD1;
    [[vk::location(3)]] float3 scale : TEXCOORD2;
    [[vk::location(4)]] float3 color : COLOR3;
    
    [[vk::location(5)]] float2 textureOffset : TEXTCOORD8;
    [[vk::location(6)]] float2 characterTextureSize : TEXTCOORD9;
    [[vk::location(7)]] float2 characterOffset : TEXTCOORD10;
    
    [[vk::location(8)]] float opacity : COLOR4;
    
    [[vk::location(9)]] uint textured : TEXTCOORD5;
    [[vk::location(10)]] uint textureIndex : TEXCOORD6;
    
    [[vk::location(11)]] uint isTextCharacter : TEXTCOORD7;
};

//Vertex shader output to fragment shader input
struct VSOutput
{
    [[vk::location(0)]] float4 position : SV_POSITION;
    [[vk::location(1)]] float2 texCoord : TEXCOORD0;
    
    [[vk::location(2)]] float3 color : COLOR2;
    [[vk::location(3)]] float opacity : COLOR3;
    
    [[vk::location(4)]] uint textured : TEXTCOORD4;
    [[vk::location(5)]] uint textureIndex : TEXCOORD5;
};

// Uniform buffer (constant buffer)
cbuffer UIGlobalInfo : register(b0)
{
    uint screenWidth;
    uint screenHeight;
}

Texture2D textures[] : register(t1);
SamplerState textureSamplers[] : register(s1);

VSOutput VSMain(UIVSInputVertex vertexInput)
{
    VSOutput output;
    
    float2 uiPos = vertexInput.position.xy;
    
    if (screenWidth > screenHeight)
    {
        float ratio = (float) screenHeight / (float) screenWidth;
        uiPos.x = uiPos.x * ratio;
    }
    else if (screenHeight > screenWidth)
    {
        float ratio = (float) screenWidth / (float) screenHeight;
        uiPos.y = uiPos.y * ratio;
    }
    
    uiPos = uiPos * vertexInput.scale.xy;
    
    if (vertexInput.isTextCharacter == 1)
    {
        uiPos = uiPos - vertexInput.characterOffset;
    }
    
    uiPos = uiPos + vertexInput.objectPosition.xy;
    
    float3 clipPos;
    clipPos.x = uiPos.x;
    clipPos.y = 1 - ((uiPos.y + 1.0));
    clipPos.z = vertexInput.objectPosition.z;
    output.position = float4(clipPos, 1.0);
    
    if (vertexInput.isTextCharacter == 0)
    {
        output.texCoord = vertexInput.texCoord;
    }
    else
    {
        float2 texCoord = vertexInput.textureOffset;
        texCoord = texCoord + (vertexInput.texCoord * vertexInput.characterTextureSize);
        output.texCoord = texCoord;
    }
    
    output.color = vertexInput.color;
    output.opacity = vertexInput.opacity;
    output.textured = vertexInput.textured;
    output.textureIndex = vertexInput.textureIndex;
    
    return output;
}

float4 PSMain(VSOutput input) : SV_TARGET
{
    float4 texColor = float4(1.0, 1.0, 1.0, 1.0);
    
    if (input.textured == 1)
    {
        texColor = textures[NonUniformResourceIndex(input.textureIndex)].Sample(textureSamplers[input.textureIndex], input.texCoord);
    }
    
    return float4(input.color, input.opacity) * texColor;
}