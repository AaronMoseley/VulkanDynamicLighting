#include "VulkanInterface.h"
#include "VulkanWindow.h"
#include "WindowManager.h"

VulkanInterface::VulkanInterface(WindowManager* windowManager)
{
    m_windowManager = windowManager;
}

void VulkanInterface::InitializeVulkan()
{
    m_vulkanWindow = m_windowManager->GetVulkanWindow();

    instance = m_vulkanWindow->vulkanInstance()->vkInstance();
    surface = m_vulkanWindow->vulkanInstance()->surfaceForWindow(m_vulkanWindow);
    physicalDevice = m_vulkanWindow->physicalDevice();
    device = m_vulkanWindow->device();
    commandPool = m_vulkanWindow->graphicsCommandPool();
    graphicsQueue = m_vulkanWindow->graphicsQueue();
    CreateVMAAllocator();
	UpdateTextureResources(kDefaultTexturePath, false);
    CreateDescriptorSetLayout();
    CreateGraphicsPipeline();
    CreateUniformBuffers();
    CreateDescriptorPool();
    CreateDescriptorSets();
}

void VulkanInterface::CreateTextureSampler(std::string textureFilePath)
{
    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(physicalDevice, &properties);

	textureImages[textureFilePath]->CreateTextureSampler(properties.limits.maxSamplerAnisotropy);
}

void VulkanInterface::CreateTextureImageView(std::string textureFilePath) {
	textureImages[textureFilePath]->CreateImageView(VK_IMAGE_ASPECT_COLOR_BIT);
}

void VulkanInterface::CreateDepthResources() {
    VkFormat depthFormat = FindDepthFormat();

	GraphicsImage::GraphicsImageCreateInfo depthImageCreateInfo{};
	depthImageCreateInfo.imageSize = { m_vulkanWindow->swapChainImageSize().width(), m_vulkanWindow->swapChainImageSize().height()};
	depthImageCreateInfo.format = depthFormat;
	depthImageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	depthImageCreateInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	depthImageCreateInfo.properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	depthImageCreateInfo.allocator = allocator;
	depthImageCreateInfo.device = device;
	depthImageCreateInfo.commandPool = commandPool;
	depthImageCreateInfo.graphicsQueue = graphicsQueue;
	depthImage = std::make_shared<GraphicsImage>(depthImageCreateInfo);

	depthImage->CreateImageView(VK_IMAGE_ASPECT_DEPTH_BIT);

	depthImage->TransitionImageLayout(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
}

VkFormat VulkanInterface::FindDepthFormat() {
    return FindSupportedFormat(
        { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
    );
}

VkFormat VulkanInterface::FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) {
    for (VkFormat format : candidates) {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);

        if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
            return format;
        }
        else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
            return format;
        }
    }

    throw std::runtime_error("failed to find supported format!");
}

void VulkanInterface::CreateTextureImage(std::string textureFilePath) {
    int texWidth, texHeight, texChannels;
    stbi_uc* pixels = stbi_load(textureFilePath.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    VkDeviceSize imageSize = texWidth * texHeight * 4;

    if (!pixels) {
        throw std::runtime_error("failed to load texture image: " + textureFilePath);
    }

    GraphicsBuffer::BufferCreateInfo stagingBufferCreateInfo{};
	stagingBufferCreateInfo.allocator = allocator;
	stagingBufferCreateInfo.size = imageSize;
	stagingBufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	stagingBufferCreateInfo.properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	stagingBufferCreateInfo.device = device;
	stagingBufferCreateInfo.commandPool = commandPool;
	stagingBufferCreateInfo.graphicsQueue = graphicsQueue;
	std::unique_ptr<GraphicsBuffer> stagingBuffer = std::make_unique<GraphicsBuffer>(stagingBufferCreateInfo);

	stagingBuffer->LoadData(pixels, static_cast<size_t>(imageSize));

    stbi_image_free(pixels);

	GraphicsImage::GraphicsImageCreateInfo textureImageCreateInfo{};
	textureImageCreateInfo.imageSize = { static_cast<size_t>(texWidth), static_cast<size_t>(texHeight) };
	textureImageCreateInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
	textureImageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	textureImageCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	textureImageCreateInfo.properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	textureImageCreateInfo.allocator = allocator;
	textureImageCreateInfo.device = device;
	textureImageCreateInfo.commandPool = commandPool;
	textureImageCreateInfo.graphicsQueue = graphicsQueue;

	std::shared_ptr<TextureImage> currentImage = std::make_shared<TextureImage>(textureImageCreateInfo);
	textureImages[textureFilePath] = currentImage;

	currentImage->TransitionImageLayout(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	currentImage->CopyFromBuffer(stagingBuffer.get());
	currentImage->TransitionImageLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	stagingBuffer->DestroyBuffer();
}

void VulkanInterface::UpdateTextureResources(std::string textureFilePath, bool alreadyInitialized)
{
	textureFilePaths.push_back(textureFilePath);
	texturePathToIndex[textureFilePath] = textureFilePaths.size() - 1;
	CreateTextureImage(textureFilePath);

    CreateTextureSampler(textureFilePath);
	CreateTextureImageView(textureFilePath);

    if (alreadyInitialized)
    {
        CreateDescriptorPool();
        CreateDescriptorSetLayout();
        CreateDescriptorSets();
        CreateGraphicsPipeline();
    }
}

void VulkanInterface::CreateDescriptorSets() {
    std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, descriptorSetLayout);
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
    allocInfo.pSetLayouts = layouts.data();

    descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
    if (vkAllocateDescriptorSets(device, &allocInfo, descriptorSets.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate descriptor sets!");
    }

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = uniformBuffers[i]->GetVkBuffer();
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(VulkanCommonFunctions::GlobalInfo);

        VkDescriptorBufferInfo lightBufferInfo{};
        lightBufferInfo.buffer = lightInfoBuffers[i]->GetVkBuffer();
        lightBufferInfo.offset = 0;
        lightBufferInfo.range = sizeof(VulkanCommonFunctions::LightInfo) * maxLightCount;

        std::vector<VkDescriptorImageInfo> imageInfos;

        for (auto it = textureFilePaths.begin(); it != textureFilePaths.end(); it++)
        {
            VkDescriptorImageInfo imageInfo{};
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfo.imageView = textureImages[*it]->GetImageView();
            imageInfo.sampler = textureImages[*it]->GetTextureSampler();

            imageInfos.push_back(imageInfo);
        }

        std::array<VkWriteDescriptorSet, 3> descriptorWrites{};

        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet = descriptorSets[i];
        descriptorWrites[0].dstBinding = 0;
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].pBufferInfo = &bufferInfo;

        descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[1].dstSet = descriptorSets[i];
        descriptorWrites[1].dstBinding = 1;
        descriptorWrites[1].dstArrayElement = 0;
        descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        descriptorWrites[1].descriptorCount = 1;
        descriptorWrites[1].pBufferInfo = &lightBufferInfo;

        descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[2].dstSet = descriptorSets[i];
        descriptorWrites[2].dstBinding = 2;
        descriptorWrites[2].dstArrayElement = 0;
        descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[2].descriptorCount = imageInfos.size();
        descriptorWrites[2].pImageInfo = imageInfos.data();

        vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
    }
}

void VulkanInterface::CreateDescriptorPool() {
    if (descriptorPool != VK_NULL_HANDLE)
    {
		vkDestroyDescriptorPool(device, descriptorPool, nullptr);
    }
    
    std::array<VkDescriptorPoolSize, 3> poolSizes{};
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    poolSizes[1].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
    poolSizes[2].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[2].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT) * textureFilePaths.size();

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

    if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor pool!");
    }
}

void VulkanInterface::CreateUniformBuffers() {
    VkDeviceSize uniformBufferSize = sizeof(VulkanCommonFunctions::GlobalInfo);

    uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);

    VkDeviceSize lightBufferSize = sizeof(VulkanCommonFunctions::LightInfo) * maxLightCount;

    lightInfoBuffers.resize(MAX_FRAMES_IN_FLIGHT);

	GraphicsBuffer::BufferCreateInfo uniformBufferCreateInfo{};
	uniformBufferCreateInfo.allocator = allocator;
	uniformBufferCreateInfo.size = uniformBufferSize;
	uniformBufferCreateInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	uniformBufferCreateInfo.properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	uniformBufferCreateInfo.device = device;
	uniformBufferCreateInfo.commandPool = commandPool;
	uniformBufferCreateInfo.graphicsQueue = graphicsQueue;

	GraphicsBuffer::BufferCreateInfo lightBufferCreateInfo{};
	lightBufferCreateInfo.allocator = allocator;
	lightBufferCreateInfo.size = lightBufferSize;
	lightBufferCreateInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
	lightBufferCreateInfo.properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	lightBufferCreateInfo.device = device;
	lightBufferCreateInfo.commandPool = commandPool;
	lightBufferCreateInfo.graphicsQueue = graphicsQueue;

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        std::shared_ptr<GraphicsBuffer> uniformBuffer = std::make_shared<GraphicsBuffer>(uniformBufferCreateInfo);
        std::shared_ptr<GraphicsBuffer> lightBuffer = std::make_shared<GraphicsBuffer>(lightBufferCreateInfo);

		uniformBuffers[i] = uniformBuffer;
		lightInfoBuffers[i] = lightBuffer;
    }
}

void VulkanInterface::CreateDescriptorSetLayout() {
    if (descriptorSetLayout != VK_NULL_HANDLE)
    {
		vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
    }
    
    VkDescriptorSetLayoutBinding globalInfoLayoutBinding{};
    globalInfoLayoutBinding.binding = 0;
    globalInfoLayoutBinding.descriptorCount = 1;
    globalInfoLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    globalInfoLayoutBinding.pImmutableSamplers = nullptr;
    globalInfoLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutBinding lightInfoBinding{};
    lightInfoBinding.binding = 1;
    lightInfoBinding.descriptorCount = 1;
    lightInfoBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    lightInfoBinding.pImmutableSamplers = nullptr;
    lightInfoBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutBinding samplerLayoutBinding{};
    samplerLayoutBinding.binding = 2;
    samplerLayoutBinding.descriptorCount = textureFilePaths.size();
    samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerLayoutBinding.pImmutableSamplers = nullptr;
    samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    std::array<VkDescriptorSetLayoutBinding, 3> bindings = { globalInfoLayoutBinding, lightInfoBinding, samplerLayoutBinding };
    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();

    if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor set layout!");
    }
}

void VulkanInterface::BeginDrawFrameCommandBuffer(VkCommandBuffer commandBuffer)
{
    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    //renderPassInfo.renderPass = renderPass;
    renderPassInfo.renderPass = m_vulkanWindow->defaultRenderPass();
    renderPassInfo.framebuffer = m_vulkanWindow->currentFramebuffer();
    renderPassInfo.renderArea.offset = { 0, 0 };
    renderPassInfo.renderArea.extent = { (uint)m_vulkanWindow->swapChainImageSize().width(), (uint)m_vulkanWindow->swapChainImageSize().height()};

    std::array<VkClearValue, 2> clearValues{};

    //clearValues[0].color = { {0.345098039f, 0.52156862f, 0.6862745098039216f, 1.0f} };
    clearValues[0].color = { {0.1f, 0.1f, 0.1f, 1.0f} };
    clearValues[1].depthStencil = { 1.0f, 0 };

    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_mainGraphicsPipeline->GetVkPipeline());

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)m_vulkanWindow->swapChainImageSize().width();
    viewport.height = (float)m_vulkanWindow->swapChainImageSize().height();
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = { 0, 0 };
    scissor.extent = { (uint)m_vulkanWindow->swapChainImageSize().width(), (uint)m_vulkanWindow->swapChainImageSize().height() };
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_mainGraphicsPipeline->GetVkPipelineLayout(), 0, 1, &descriptorSets[currentFrame], 0, nullptr);
}

void VulkanInterface::DrawInstancedObjectCommandBuffer(VkCommandBuffer commandBuffer, std::string objectName, size_t objectCount) {
    if (objectCount <= 0)
        return;
    
    VkBuffer objectVertexBuffer[] = { vertexBuffers[objectName]->GetVkBuffer(), instanceBuffers[currentFrame][objectName]->GetVkBuffer()};
    VkDeviceSize offsets[] = { 0, 0 };
    vkCmdBindVertexBuffers(commandBuffer, 0, 2, objectVertexBuffer, offsets);

    if (indexBufferSizes[objectName] > 0)
    {
        vkCmdBindIndexBuffer(commandBuffer, indexBuffers[objectName]->GetVkBuffer(), 0, VK_INDEX_TYPE_UINT16);

        vkCmdDrawIndexed(commandBuffer, indexBufferSizes[objectName], objectCount, 0, 0, 0);
        //vkCmdDrawIndexed(commandBuffer, indexBufferSizes[objectName], meshNameToObjectMap[objectName].size(), 0, 0, 0);
    }
    else {
        vkCmdDraw(commandBuffer, vertexBufferSizes[objectName], objectCount, 0, 0);
    }
}

void VulkanInterface::DrawSingleObjectCommandBuffer(VkCommandBuffer commandBuffer, std::shared_ptr<RenderObject> renderObject) {
	std::shared_ptr<MeshRenderer> meshComponent = renderObject->GetComponent<MeshRenderer>();

    if (meshComponent == nullptr)
    {
        return;
    }

    if (meshComponent->GetMeshName() != MeshRenderer::kCustomMeshName)
    {
        return;
    }

    VkBuffer objectVertexBuffer[] = { meshComponent->GetVertexBuffer()->GetVkBuffer(), renderObject->GetInstanceBuffer()->GetVkBuffer()};
    VkDeviceSize offsets[] = { 0, 0 };
    vkCmdBindVertexBuffers(commandBuffer, 0, 2, objectVertexBuffer, offsets);

    if (meshComponent->IsIndexed())
    {
        vkCmdBindIndexBuffer(commandBuffer, meshComponent->GetIndexBuffer()->GetVkBuffer(), 0, VK_INDEX_TYPE_UINT16);

        vkCmdDrawIndexed(commandBuffer, meshComponent->GetIndexBufferSize(), 1, 0, 0, 0);
        //vkCmdDrawIndexed(commandBuffer, indexBufferSizes[objectName], meshNameToObjectMap[objectName].size(), 0, 0, 0);
    }
    else {
        vkCmdDraw(commandBuffer, meshComponent->GetVertexBufferSize(), 1, 0, 0);
    }
}

void VulkanInterface::EndDrawFrameCommandBuffer(VkCommandBuffer commandBuffer)
{
    vkCmdEndRenderPass(commandBuffer);
}

void VulkanInterface::CreateGraphicsPipeline() 
{
    if (m_mainGraphicsPipeline != VK_NULL_HANDLE)
    {
        m_mainGraphicsPipeline->SetDescriptorSetLayout(descriptorSetLayout);
        m_mainGraphicsPipeline->CreatePipeline();
        return;
    }

	GraphicsPipelineCreateInfo pipelineCreateInfo{};
	pipelineCreateInfo.vertexShaderFilePath = "shaders/HLSL/VertexShader.spv";
	pipelineCreateInfo.fragmentShaderFilePath = "shaders/HLSL/PixelShader.spv";
	pipelineCreateInfo.descriptorSetLayout = descriptorSetLayout;
	pipelineCreateInfo.device = device;
	pipelineCreateInfo.vulkanWindow = m_vulkanWindow;
	m_mainGraphicsPipeline = std::make_shared<GraphicsPipeline>(pipelineCreateInfo);
}

void VulkanInterface::PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
    createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = DebugCallback;
}

VKAPI_ATTR VkBool32 VKAPI_CALL VulkanInterface::DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
    std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

    return VK_FALSE;
}

bool VulkanInterface::CheckValidationLayerSupport() {
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const char* layerName : validationLayers) {
        bool layerFound = false;

        for (const auto& layerProperties : availableLayers) {
            if (strcmp(layerName, layerProperties.layerName) == 0) {
                layerFound = true;
                break;
            }
        }

        if (!layerFound) {
            return false;
        }
    }

    return true;
}

std::shared_ptr<GraphicsBuffer> VulkanInterface::CreateVertexBuffer(std::shared_ptr<MeshRenderer> meshInfo) {
    VkDeviceSize bufferSize = sizeof(VulkanCommonFunctions::Vertex) * meshInfo->GetVertices().size();

    GraphicsBuffer::BufferCreateInfo stagingBufferCreateInfo = {};
	stagingBufferCreateInfo.size = bufferSize;
	stagingBufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    stagingBufferCreateInfo.properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	stagingBufferCreateInfo.allocator = allocator;
	stagingBufferCreateInfo.commandPool = commandPool;
	stagingBufferCreateInfo.graphicsQueue = graphicsQueue;
    stagingBufferCreateInfo.device = device;

	const std::vector< VulkanCommonFunctions::Vertex>& vertices = meshInfo->GetVertices();

	std::shared_ptr<GraphicsBuffer> stagingBuffer = std::make_shared<GraphicsBuffer>(stagingBufferCreateInfo);
    stagingBuffer->LoadData((void*)vertices.data(), (size_t)bufferSize);

    GraphicsBuffer::BufferCreateInfo vertexBufferCreateInfo = {};
    vertexBufferCreateInfo.size = bufferSize;
    vertexBufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    vertexBufferCreateInfo.properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    vertexBufferCreateInfo.allocator = allocator;
    vertexBufferCreateInfo.commandPool = commandPool;
    vertexBufferCreateInfo.graphicsQueue = graphicsQueue;
    vertexBufferCreateInfo.device = device;

    std::shared_ptr<GraphicsBuffer> vertexBuffer = std::make_shared<GraphicsBuffer>(vertexBufferCreateInfo);

    stagingBuffer->CopyBuffer(vertexBuffer, bufferSize);
    stagingBuffer->DestroyBuffer();

    return vertexBuffer;
}

void VulkanInterface::UpdateObjectBuffers(std::shared_ptr<MeshRenderer> objectMesh)
{
    if (objectMesh->GetMeshName() == MeshRenderer::kCustomMeshName)
    {
        return;
    }

    if (vertexBuffers.contains(objectMesh->GetMeshName()) || (objectMesh->IsIndexed() && indexBuffers.contains(objectMesh->GetMeshName())))
    {
        return;
    }

    std::shared_ptr<GraphicsBuffer> indexBuffer = CreateIndexBuffer(objectMesh);
    indexBuffers[objectMesh->GetMeshName()] = indexBuffer;
    indexBufferSizes[objectMesh->GetMeshName()] = static_cast<uint16_t>(objectMesh->GetIndices().size());

    std::shared_ptr<GraphicsBuffer> vertexBuffer = CreateVertexBuffer(objectMesh);
    vertexBuffers[objectMesh->GetMeshName()] = vertexBuffer;
    vertexBufferSizes[objectMesh->GetMeshName()] = static_cast<uint16_t>(objectMesh->GetVertices().size());

    CreateInstanceBuffer(objectMesh);
}

std::shared_ptr<GraphicsBuffer> VulkanInterface::CreateIndexBuffer(std::shared_ptr<MeshRenderer>  meshInfo) {
    VkDeviceSize bufferSize = sizeof(uint16_t) * meshInfo->GetIndices().size();

    GraphicsBuffer::BufferCreateInfo stagingBufferCreateInfo = {};
    stagingBufferCreateInfo.size = bufferSize;
    stagingBufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    stagingBufferCreateInfo.properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    stagingBufferCreateInfo.allocator = allocator;
    stagingBufferCreateInfo.commandPool = commandPool;
    stagingBufferCreateInfo.graphicsQueue = graphicsQueue;
    stagingBufferCreateInfo.device = device;

	const std::vector<uint16_t>& indices = meshInfo->GetIndices();

    std::shared_ptr<GraphicsBuffer> stagingBuffer = std::make_shared<GraphicsBuffer>(stagingBufferCreateInfo);
    stagingBuffer->LoadData((void*)indices.data(), (size_t)bufferSize);

    GraphicsBuffer::BufferCreateInfo indexBufferCreateInfo = {};
    indexBufferCreateInfo.size = bufferSize;
    indexBufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    indexBufferCreateInfo.properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    indexBufferCreateInfo.allocator = allocator;
    indexBufferCreateInfo.commandPool = commandPool;
    indexBufferCreateInfo.graphicsQueue = graphicsQueue;
    indexBufferCreateInfo.device = device;

    std::shared_ptr<GraphicsBuffer> indexBuffer = std::make_shared<GraphicsBuffer>(indexBufferCreateInfo);

	stagingBuffer->CopyBuffer(indexBuffer, bufferSize);

	stagingBuffer->DestroyBuffer();

	return indexBuffer;
}

void VulkanInterface::CreateVMAAllocator()
{
    VmaVulkanFunctions vulkanFunctions = {};
    vulkanFunctions.vkGetInstanceProcAddr = &vkGetInstanceProcAddr;
    vulkanFunctions.vkGetDeviceProcAddr = &vkGetDeviceProcAddr;

    VmaAllocatorCreateInfo allocatorCreateInfo = {};
    allocatorCreateInfo.flags = VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT;
    allocatorCreateInfo.vulkanApiVersion = VK_API_VERSION_1_3;
    allocatorCreateInfo.physicalDevice = physicalDevice;
    allocatorCreateInfo.device = device;
    allocatorCreateInfo.instance = instance;
    allocatorCreateInfo.pVulkanFunctions = &vulkanFunctions;

    vmaCreateAllocator(&allocatorCreateInfo, &allocator);
}

void VulkanInterface::CreateInstanceBuffer(std::shared_ptr<MeshRenderer> object)
{
    if (object->GetMeshName() == MeshRenderer::kCustomMeshName)
    {
        return;
    }

    for (uint32_t frameIndex = 0; frameIndex < MAX_FRAMES_IN_FLIGHT; frameIndex++)
    {
        std::shared_ptr<GraphicsBuffer> instanceBuffer = CreateInstanceBuffer(VulkanCommonFunctions::MAX_OBJECTS);
		instanceBuffers[frameIndex][object->GetMeshName()] = instanceBuffer;
    }
}

std::shared_ptr<GraphicsBuffer> VulkanInterface::CreateInstanceBuffer(size_t maxObjects)
{
    VkDeviceSize bufferSize = sizeof(VulkanCommonFunctions::InstanceInfo) * maxObjects;

    GraphicsBuffer::BufferCreateInfo instanceBufferCreateInfo = {};
    instanceBufferCreateInfo.size = bufferSize;
    instanceBufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    instanceBufferCreateInfo.properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    instanceBufferCreateInfo.allocator = allocator;
    instanceBufferCreateInfo.commandPool = commandPool;
    instanceBufferCreateInfo.graphicsQueue = graphicsQueue;
    instanceBufferCreateInfo.device = device;

    std::shared_ptr<GraphicsBuffer> instanceBuffer = std::make_shared<GraphicsBuffer>(instanceBufferCreateInfo);
    return instanceBuffer;
}

void VulkanInterface::UpdateInstanceBuffer(std::string objectName, std::set<VulkanCommonFunctions::ObjectHandle> objectHandles, std::map<VulkanCommonFunctions::ObjectHandle, std::shared_ptr<RenderObject>> objects)
{
    std::vector<VulkanCommonFunctions::InstanceInfo> objectInfo;

    for (auto it = objectHandles.begin(); it != objectHandles.end(); it++)
    {
        VulkanCommonFunctions::ObjectHandle currentHandle = *it;

        if (currentHandle == VulkanCommonFunctions::INVALID_OBJECT_HANDLE || !objects.contains(currentHandle))
        {
            continue;
        }

        std::shared_ptr<RenderObject> object = objects[currentHandle];

		std::shared_ptr<MeshRenderer> meshRenderer = object->GetComponent<MeshRenderer>();

        if (meshRenderer == nullptr)
            continue;

        if (!meshRenderer->IsEnabled())
            continue;

        VulkanCommonFunctions::InstanceInfo info = object->GetInstanceInfo();

        auto iterator = std::find(textureFilePaths.begin(), textureFilePaths.end(), meshRenderer->GetTexturePath());

        info.textureIndex = std::distance(textureFilePaths.begin(), iterator);
        if (info.textureIndex > textureFilePaths.size())
        {
            info.textureIndex = 0;
        }

        objectInfo.push_back(info);
    }

    if (objectInfo.size() == 0)
    {
        return; 
    }

    VkDeviceSize bufferSize = objectHandles.size() * sizeof(VulkanCommonFunctions::InstanceInfo);

	instanceBuffers[currentFrame][objectName]->LoadData(objectInfo.data(), (size_t)bufferSize);
}

void VulkanInterface::DrawFrame(float deltaTime, std::shared_ptr<Scene> scene) {
    std::map<std::string, std::set<VulkanCommonFunctions::ObjectHandle>> objectHandles = scene->GetMeshNameToObjectMap();
    std::map<VulkanCommonFunctions::ObjectHandle, std::shared_ptr<RenderObject>> objects = scene->GetObjects();
    
    for (auto it = objectHandles.begin(); it != objectHandles.end(); it++)
    {
        if (it->first != MeshRenderer::kCustomMeshName)
        {
            UpdateInstanceBuffer(it->first, it->second, objects);
        }
    }

    uint32_t imageIndex = m_vulkanWindow->currentSwapChainImageIndex();

    UpdateUniformBuffer(currentFrame, objects);

    VkCommandBuffer commandBuffer = m_vulkanWindow->currentCommandBuffer();
    VkFramebuffer frameBuffer = m_vulkanWindow->currentFramebuffer();
    QSize extent = m_vulkanWindow->swapChainImageSize();

    BeginDrawFrameCommandBuffer(commandBuffer);

    for (auto it = objectHandles.begin(); it != objectHandles.end(); it++)
    {
        if (it->first == MeshRenderer::kCustomMeshName)
        {
            for (auto it2 = it->second.begin(); it2 != it->second.end(); it2++)
            {
				VulkanCommonFunctions::ObjectHandle currentHandle = *it2;
				DrawSingleObjectCommandBuffer(commandBuffer, objects[currentHandle]);
            }
        }
        else {
            DrawInstancedObjectCommandBuffer(commandBuffer, it->first, it->second.size());
        }
    }

    EndDrawFrameCommandBuffer(commandBuffer);

    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    renderedFirstFrame = true;

    m_vulkanWindow->frameReady();
    m_vulkanWindow->requestUpdate();
}

void VulkanInterface::UpdateUniformBuffer(uint32_t currentImage, std::map<VulkanCommonFunctions::ObjectHandle, std::shared_ptr<RenderObject>> objects) {
    VulkanCommonFunctions::GlobalInfo globalInfo;
    float aspectRatio = (float)m_vulkanWindow->swapChainImageSize().width() / (float)m_vulkanWindow->swapChainImageSize().height();

	bool cameraFound = false;
    for (auto it = objects.begin(); it != objects.end(); it++)
    {
		std::shared_ptr<Camera> camera = it->second->GetComponent<Camera>();
        if (camera == nullptr)
        {
            continue;
        }

        if (!camera->IsMainCamera())
        {
            continue;
        }

		globalInfo.view = camera->GetViewMatrix();
		globalInfo.proj = glm::perspective(glm::radians(camera->GetFOV()), aspectRatio, camera->GetNearPlane(), camera->GetFarPlane());
        globalInfo.proj[1][1] *= -1;
		globalInfo.cameraPosition = glm::vec4(it->second->GetComponent<Transform>()->GetPosition(), 1.0f);
		cameraFound = true;
        break;
    }

    if (!cameraFound)
    {
		throw std::runtime_error("No camera found in the scene. Please add a camera to render the scene.");
    }

    std::vector<VulkanCommonFunctions::LightInfo> lightInfos;

    for (auto it = objects.begin(); it != objects.end(); it++)
    {
		std::shared_ptr<LightSource> light = it->second->GetComponent<LightSource>();

        if (light == nullptr)
        {
            continue;
        }

        if (lightInfos.size() >= maxLightCount)
        {
			std::cout << "Warning: Maximum light count exceeded, additional lights will be ignored in rendering." << std::endl;
            break;
        }

        if (!light->IsEnabled())
        {
            continue;
        }

        VulkanCommonFunctions::LightInfo lightInfo = light->GetLightInfo();
		lightInfos.push_back(lightInfo);
    }

    globalInfo.lightCount = lightInfos.size();

	lightInfoBuffers[currentImage]->LoadData(lightInfos.data(), lightInfos.size() * sizeof(VulkanCommonFunctions::LightInfo));
	uniformBuffers[currentImage]->LoadData(&globalInfo, sizeof(globalInfo));
}

void VulkanInterface::CleanupSwapChain() {
    vkDeviceWaitIdle(device);
	depthImage->DestroyImage();
}

void VulkanInterface::Cleanup() {
    vkDeviceWaitIdle(device);

	m_mainGraphicsPipeline->DestroyPipeline();

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		uniformBuffers[i]->DestroyBuffer();
		lightInfoBuffers[i]->DestroyBuffer();
    }

    for (auto it = textureFilePaths.begin(); it != textureFilePaths.end(); it++)
    {
		textureImages[*it]->DestroyTextureImage();
    }

    vkDestroyDescriptorPool(device, descriptorPool, nullptr);

    vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);

    for (auto it = indexBuffers.begin(); it != indexBuffers.end(); it++)
    {
		it->second->DestroyBuffer();
    }

    for (auto it = vertexBuffers.begin(); it != vertexBuffers.end(); it++)
    {
		it->second->DestroyBuffer();
    }

    for (uint32_t frameIndex = 0; frameIndex < MAX_FRAMES_IN_FLIGHT; frameIndex++)
    {
        for (auto it = instanceBuffers[frameIndex].begin(); it != instanceBuffers[frameIndex].end(); it++)
        {
            it->second->DestroyBuffer();
        }
    }

    vmaDestroyAllocator(allocator);
}