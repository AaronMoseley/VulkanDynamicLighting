struct VSInputVertex
{
    //Vertex attributes
    [[vk::location(0)]] float3 position : POSITION; // Vertex position
    [[vk::location(1)]] float3 normal : NORMAL; // Vertex normal
    
    //Instance attributes
    [[vk::location(2)]] float4x4 model : TEXCOORD0; //4 locations for 4 * float4
    [[vk::location(6)]] float4x4 modelMatrixInverted : TEXCOORD1; //4 locations for 4 * float4
    
    [[vk::location(10)]] float3 ambient : COLOR2;
    [[vk::location(11)]] float3 diffuse : COLOR3;
    [[vk::location(12)]] float3 specular : COLOR4;
    
    [[vk::location(13)]] float shininess : COLOR5;
    [[vk::location(14)]] uint lit : COLOR6;
};

//Vertex shader output to fragment shader input
struct VSOutput
{
    [[vk::location(0)]] float4 position : SV_POSITION;
    [[vk::location(1)]] float3 worldPosition : TEXCOORD1;
    
    [[vk::location(2)]] float3 normal : NORMAL2;
    [[vk::location(3)]] float3 ambient : COLOR3;
    [[vk::location(4)]] float3 diffuse : COLOR4;
    [[vk::location(5)]] float3 specular : COLOR5;
    [[vk::location(6)]] float shininess : COLOR6;
    [[vk::location(7)]] uint lit : COLOR7;
};

// Uniform buffer (constant buffer)
cbuffer GlobalInfo : register(b0)
{
    float4x4 view;
    float4x4 projection;
    float3 cameraPosition;
    uint lightCount;
}

struct LightInfo
{
    float4 lightPosition;
    float4 lightColor;
   
    float4 lightAmbient;
    float4 lightDiffuse;
    float4 lightSpecular;
};

StructuredBuffer<LightInfo> lights : register(t1);

VSOutput VSMain(VSInputVertex vertexInput)
{
    VSOutput output;
    
    float4 worldPos = mul(vertexInput.model, float4(vertexInput.position, 1.0));
    float4 viewPos = mul(view, worldPos);
    float4 clipPos = mul(projection, viewPos);
    
    output.position = clipPos;
    output.worldPosition = worldPos.xyz;
    
    float3x3 normalMatrix = (float3x3)transpose(vertexInput.modelMatrixInverted);
    
    output.normal = mul(normalMatrix, vertexInput.normal);
    output.ambient = vertexInput.ambient;
    output.diffuse = vertexInput.diffuse;
    output.specular = vertexInput.specular;
    output.shininess = vertexInput.shininess;
    output.lit = vertexInput.lit;
    
    return output;
}

float4 PSMain(VSOutput input) : SV_TARGET
{   
    if (input.lit == 0)
    {
        return float4(input.diffuse, 1.0);
    }
    
    float3 result = float3(0, 0, 0);
    
    for (uint i = 0; i < lightCount; i++)
    {
        // ambient
        float3 ambient = lights[i].lightAmbient.xyz * input.ambient;

        // diffuse 
        float3 norm = normalize(input.normal);
        float3 lightDir = normalize(lights[i].lightPosition.xyz - input.worldPosition);
        float diff = max(dot(norm, lightDir), 0.0);
        float3 diffuse = lights[i].lightDiffuse.xyz * (diff * input.diffuse);

        // specular
        float3 viewDir = normalize(cameraPosition - input.worldPosition);
        float3 reflectDir = reflect(-lightDir, norm);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), input.shininess);
        float3 specular = lights[i].lightSpecular.xyz * (spec * input.specular);
     
        float distance = length(lights[i].lightPosition.xyz - input.worldPosition);
        float lerpT = distance / 100.0;
    
        lerpT = min(lerpT, 1.0);
    
        float3 currentResult = ambient + diffuse + specular;
        currentResult = currentResult * (1.0 - lerpT);

        result += currentResult;
    }
    
    return float4(result, 1.0);
}