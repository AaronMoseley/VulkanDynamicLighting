struct VSInputVertex
{
    [[vk::location(0)]] float3 position : POSITION; // Vertex position
    [[vk::location(1)]] float3 normal : NORMAL; // Vertex normal
    
    [[vk::location(2)]] float4x4 model : TEXCOORD0;
    [[vk::location(6)]] float4x4 modelMatrixInverted : TEXCOORD1;
    
    [[vk::location(10)]] float3 ambient : COLOR2;
    [[vk::location(11)]] float3 diffuse : COLOR3;
    [[vk::location(12)]] float3 specular : COLOR4;
    
    [[vk::location(13)]] float shininess : COLOR5;
    [[vk::location(14)]] uint lit : COLOR6;
};

// Define output structure for the vertex shader
struct VSOutput
{
    [[vk::location(0)]] float4 position : SV_POSITION; // Transformed position
    [[vk::location(1)]] float3 fragPos : TEXCOORD1;
    [[vk::location(2)]] float3 normal : NORMAL2; // Pass-through normal
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
    float4x4 proj;
    float3 cameraPosition;
    
    float3 lightPosition;
    float3 lightColor;
    
    float3 lightAmbient;
    float3 lightDiffuse;
    float3 lightSpecular;
}

VSOutput VSMain(VSInputVertex vertexInput)
{
    VSOutput output;
    
    float4 worldPos = mul(vertexInput.model, float4(vertexInput.position, 1.0));
    float4 viewPos = mul(view, worldPos);
    float4 clipPos = mul(proj, viewPos);
    
    output.position = clipPos;
    output.fragPos = worldPos.xyz;
    
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
    
    // ambient
    float3 ambient = lightAmbient * input.ambient;

        // diffuse 
    float3 norm = normalize(input.normal);
    float3 lightDir = normalize(lightPosition - input.fragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    float3 diffuse = lightDiffuse * (diff * input.diffuse);

        // specular
    float3 viewDir = normalize(cameraPosition - input.fragPos);
    float3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), input.shininess);
    float3 specular = lightSpecular * (spec * input.specular);

    float3 result = ambient + diffuse + specular;
    
    return float4(result, 1.0);
}