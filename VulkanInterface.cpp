#include "VulkanInterface.h"

VulkanInterface::VulkanInterface(WindowManager* windowManager, Camera* camera)
{
    m_windowManager = windowManager;
    m_camera = camera;
    InitializeVulkan();
}

void VulkanInterface::InitializeVulkan()
{
    Factory objectClasses = FACTORY(MeshRenderer);

    std::vector<std::string> objectClassNames = objectClasses.GetRegisteredClasses();

    for (size_t i = 0; i < objectClassNames.size(); i++)
    {
        meshNameToObjectMap[objectClassNames[i]] = {};
    }

    meshNameToObjectMap[customMeshName] = {};

    CreateInstance();
    SetupDebugMessenger();
    CreateSurface();
    PickPhysicalDevice();
    CreateLogicalDevice();
    CreateVMAAllocator();
    CreateSwapChain();
    CreateRenderPass();
    CreateDescriptorSetLayout();
    CreateGraphicsPipeline();
    CreateCommandPool();
    CreateDepthResources();
    CreateTextureImage();
    CreateTextureImageView();
    CreateTextureSampler();
	swapChain->CreateFrameBuffers(depthImage, renderPass);
    CreateVertexBuffers();
    CreateIndexBuffers();
    CreateInstanceBuffers();
    CreateUniformBuffers();
    CreateDescriptorPool();
    CreateDescriptorSets();
    CreateCommandBuffers();
    CreateSyncObjects();
}

void VulkanInterface::CreateTextureSampler()
{
    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(physicalDevice, &properties);

    for (size_t i = 0; i < textureFiles.size(); i++)
    {
		textureImages[i]->CreateTextureSampler(properties.limits.maxSamplerAnisotropy);
    }
}

VulkanInterface::ObjectHandle VulkanInterface::AddObject(RenderObject* newObject)
{
    m_currentObjectHandle++;

    objects[m_currentObjectHandle] = newObject;

	MeshRenderer* meshComponent = newObject->GetComponent<MeshRenderer>();

    if (meshComponent == nullptr)
    {
		return m_currentObjectHandle;
    }
	
    std::string objectName = meshComponent->GetMeshName();
    meshNameToObjectMap[objectName].insert(m_currentObjectHandle);

    return m_currentObjectHandle;
}

bool VulkanInterface::RemoveObject(ObjectHandle objectToRemove)
{
    RenderObject* currentObject = GetRenderObject(objectToRemove);

    if (currentObject == nullptr)
    {
        return false;
    }

    bool removalSuccessful = true;

    removalSuccessful = objects.erase(objectToRemove);

    MeshRenderer* meshComponent = currentObject->GetComponent<MeshRenderer>();

    if (meshComponent == nullptr)
    {
        return removalSuccessful;
    }

    std::string objectName = meshComponent->GetMeshName();

    if (!meshNameToObjectMap.contains(objectName))
        return false;

    removalSuccessful = removalSuccessful && meshNameToObjectMap[objectName].erase(objectToRemove);

    return removalSuccessful;
}

RenderObject* VulkanInterface::GetRenderObject(VulkanInterface::ObjectHandle handle)
{
    if (handle == INVALID_OBJECT_HANDLE)
    {
        return nullptr;
    }

    if (!objects.contains(handle))
    {
        return nullptr;
    }

    return objects[handle];
}

void VulkanInterface::CreateTextureImageView() {
    for (size_t i = 0; i < textureFiles.size(); i++)
    {
		textureImages[i]->CreateImageView(VK_IMAGE_ASPECT_COLOR_BIT);
    }
}

void VulkanInterface::CreateDepthResources() {
    VkFormat depthFormat = FindDepthFormat();

	GraphicsImage::GraphicsImageCreateInfo depthImageCreateInfo{};
	depthImageCreateInfo.imageSize = { swapChain->GetSwapChainExtent().width, swapChain->GetSwapChainExtent().height};
	depthImageCreateInfo.format = depthFormat;
	depthImageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	depthImageCreateInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	depthImageCreateInfo.properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	depthImageCreateInfo.allocator = allocator;
	depthImageCreateInfo.device = device;
	depthImageCreateInfo.commandPool = commandPool;
	depthImageCreateInfo.graphicsQueue = graphicsQueue;
	depthImage = new GraphicsImage(depthImageCreateInfo);

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

void VulkanInterface::CreateTextureImage() {
    for (size_t i = 0; i < textureFiles.size(); i++)
    {
        int texWidth, texHeight, texChannels;
        stbi_uc* pixels = stbi_load(textureFiles[i].c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
        VkDeviceSize imageSize = texWidth * texHeight * 4;

        if (!pixels) {
            throw std::runtime_error("failed to load texture image!");
        }

        GraphicsBuffer::BufferCreateInfo stagingBufferCreateInfo{};
		stagingBufferCreateInfo.allocator = allocator;
		stagingBufferCreateInfo.size = imageSize;
		stagingBufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		stagingBufferCreateInfo.properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
		stagingBufferCreateInfo.device = device;
		stagingBufferCreateInfo.commandPool = commandPool;
		stagingBufferCreateInfo.graphicsQueue = graphicsQueue;
		GraphicsBuffer stagingBuffer(stagingBufferCreateInfo);

		stagingBuffer.LoadData(pixels, static_cast<size_t>(imageSize));

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

		TextureImage* currentImage = new TextureImage(textureImageCreateInfo);
		textureImages.push_back(currentImage);

		currentImage->TransitionImageLayout(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		currentImage->CopyFromBuffer(&stagingBuffer);
		currentImage->TransitionImageLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		stagingBuffer.DestroyBuffer();
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

        for (size_t i = 0; i < textureFiles.size(); i++)
        {
            VkDescriptorImageInfo imageInfo{};
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfo.imageView = textureImages[i]->GetImageView();
            imageInfo.sampler = textureImages[i]->GetTextureSampler();

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
    std::array<VkDescriptorPoolSize, 3> poolSizes{};
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    poolSizes[1].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
    poolSizes[2].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[2].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT) * textureFiles.size();

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
        GraphicsBuffer* uniformBuffer = new GraphicsBuffer(uniformBufferCreateInfo);
		GraphicsBuffer* lightBuffer = new GraphicsBuffer(lightBufferCreateInfo);

		uniformBuffers[i] = uniformBuffer;
		lightInfoBuffers[i] = lightBuffer;
    }
}

void VulkanInterface::CreateDescriptorSetLayout() {
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
    samplerLayoutBinding.descriptorCount = textureFiles.size();
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

void VulkanInterface::RecreateSwapChain() {
    int width = 0, height = 0;
    glfwGetFramebufferSize(m_windowManager->GetWindow(), &width, &height);
    while (width == 0 || height == 0) {
        glfwGetFramebufferSize(m_windowManager->GetWindow(), &width, &height);
        glfwWaitEvents();
    }

    vkDeviceWaitIdle(device);

	swapChain->DestroySwapChain();

    CreateSwapChain();
    CreateDepthResources();

	swapChain->CreateFrameBuffers(depthImage, renderPass);
}

void VulkanInterface::CreateSyncObjects() {
    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (uint32_t frameIndex = 0; frameIndex < MAX_FRAMES_IN_FLIGHT; frameIndex++)
    {
        VkFence currentFence;
        VkResult result = vkCreateFence(device, &fenceInfo, nullptr, &currentFence);

        if (result != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create fences!");
        }

        inFlightFences[frameIndex] = currentFence;

        VkSemaphore imageAvailableSemaphore;
        VkSemaphore renderFinishedSemaphore;

        if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphore) != VK_SUCCESS ||
            vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphore) != VK_SUCCESS) {
            throw std::runtime_error("failed to create semaphores!");
        }

        imageAvailableSemaphores[frameIndex] = imageAvailableSemaphore;
        renderFinishedSemaphores[frameIndex] = renderFinishedSemaphore;
    }
}

void VulkanInterface::BeginDrawFrameCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex)
{
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0; // Optional
    beginInfo.pInheritanceInfo = nullptr; // Optional

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("failed to begin recording command buffer!");
    }

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = renderPass;
    renderPassInfo.framebuffer = swapChain->GetFrameBuffer(imageIndex);
    renderPassInfo.renderArea.offset = { 0, 0 };
    renderPassInfo.renderArea.extent = swapChain->GetSwapChainExtent();

    std::array<VkClearValue, 2> clearValues{};

    //clearValues[0].color = { {0.345098039f, 0.52156862f, 0.6862745098039216f, 1.0f} };
    clearValues[0].color = { {0.01f, 0.01f, 0.01f, 1.0f} };
    clearValues[1].depthStencil = { 1.0f, 0 };

    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)swapChain->GetSwapChainExtent().width;
    viewport.height = (float)swapChain->GetSwapChainExtent().height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = { 0, 0 };
    scissor.extent = swapChain->GetSwapChainExtent();
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSets[currentFrame], 0, nullptr);
}

void VulkanInterface::DrawObjectCommandBuffer(VkCommandBuffer commandBuffer, std::string objectName) {
    if (meshNameToObjectMap[objectName].size() <= 0)
        return;
    
    VkBuffer objectVertexBuffer[] = { vertexBuffers[objectName]->GetVkBuffer(), instanceBuffers[currentFrame][objectName]->GetVkBuffer()};
    VkDeviceSize offsets[] = { 0, 0 };
    vkCmdBindVertexBuffers(commandBuffer, 0, 2, objectVertexBuffer, offsets);

    if (indexBufferSizes[objectName] > 0)
    {
        vkCmdBindIndexBuffer(commandBuffer, indexBuffers[objectName]->GetVkBuffer(), 0, VK_INDEX_TYPE_UINT16);

        vkCmdDrawIndexed(commandBuffer, indexBufferSizes[objectName], meshNameToObjectMap[objectName].size(), 0, 0, 0);
        //vkCmdDrawIndexed(commandBuffer, indexBufferSizes[objectName], meshNameToObjectMap[objectName].size(), 0, 0, 0);
    }
    else {
        vkCmdDraw(commandBuffer, vertexBufferSizes[objectName], meshNameToObjectMap[objectName].size(), 0, 0);
    }
}

void VulkanInterface::EndDrawFrameCommandBuffer(VkCommandBuffer commandBuffer)
{
    vkCmdEndRenderPass(commandBuffer);

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to record command buffer!");
    }
}

void VulkanInterface::CreateCommandBuffers() {
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        if (vkAllocateCommandBuffers(device, &allocInfo, &frameCommandBuffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate command buffers!");
        }
    }
}

void VulkanInterface::CreateCommandPool() {
    VulkanCommonFunctions::QueueFamilyIndices queueFamilyIndices = VulkanCommonFunctions::FindQueueFamilies(physicalDevice, surface);

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = queueFamilyIndices.m_graphicsFamily.value();

    if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create command pool!");
    }
}

void VulkanInterface::CreateRenderPass() {
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = swapChain->GetImage(0)->GetImageFormat();
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcAccessMask = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    VkAttachmentDescription depthAttachment{};
    depthAttachment.format = FindDepthFormat();
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachmentRef{};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;

    std::array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };
    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
        throw std::runtime_error("failed to create render pass!");
    }
}

void VulkanInterface::CreateGraphicsPipeline() {
    auto vertShaderCode = ReadFile("shaders/HLSL/VertexShader.spv");
    auto fragShaderCode = ReadFile("shaders/HLSL/PixelShader.spv");

    VkShaderModule vertShaderModule = CreateShaderModule(vertShaderCode);
    VkShaderModule fragShaderModule = CreateShaderModule(fragShaderCode);

    VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertShaderModule;
    vertShaderStageInfo.pName = "VSMain";

    VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.pName = "PSMain";

    VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

    auto bindingDescription = VulkanCommonFunctions::Vertex::GetBindingDescriptions();
    auto attributeDescriptions = VulkanCommonFunctions::Vertex::GetAttributeDescriptions();

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

    vertexInputInfo.vertexBindingDescriptionCount = 2;
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
    vertexInputInfo.pVertexBindingDescriptions = bindingDescription.data();
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    //can be set staticly when creating pipeline, or dynamically at draw time
    //the only attributes of the graphics pipeline that can be modified while drawing
    /*
    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)swapChainExtent.width;
    viewport.height = (float)swapChainExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor{};
    scissor.offset = { 0, 0 };
    scissor.extent = swapChainExtent;
    */

    std::vector<VkDynamicState> dynamicStates = {
    VK_DYNAMIC_STATE_VIEWPORT,
    VK_DYNAMIC_STATE_SCISSOR
    };

    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicState.pDynamicStates = dynamicStates.data();

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;

    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0f; // Optional
    rasterizer.depthBiasClamp = 0.0f; // Optional
    rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading = 1.0f; // Optional
    multisampling.pSampleMask = nullptr; // Optional
    multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
    multisampling.alphaToOneEnable = VK_FALSE; // Optional

    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

    //if we want to blend between each frame and the new frame
    /*
    colorBlendAttachment.blendEnable = VK_TRUE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
    */

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f; // Optional
    colorBlending.blendConstants[1] = 0.0f; // Optional
    colorBlending.blendConstants[2] = 0.0f; // Optional
    colorBlending.blendConstants[3] = 0.0f; // Optional

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout; // Optional
    pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
    pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

    if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline layout!");
    }

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = nullptr; // Optional
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.layout = pipelineLayout;
    pipelineInfo.renderPass = renderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
    pipelineInfo.basePipelineIndex = -1; // Optional

    VkPipelineDepthStencilStateCreateInfo depthStencil{};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.minDepthBounds = 0.0f; // Optional
    depthStencil.maxDepthBounds = 1.0f; // Optional
    depthStencil.stencilTestEnable = VK_FALSE;
    depthStencil.front = {}; // Optional
    depthStencil.back = {}; // Optional

    pipelineInfo.pDepthStencilState = &depthStencil;

    if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS) {
        throw std::runtime_error("failed to create graphics pipeline!");
    }

    vkDestroyShaderModule(device, fragShaderModule, nullptr);
    vkDestroyShaderModule(device, vertShaderModule, nullptr);
}

VkShaderModule VulkanInterface::CreateShaderModule(const std::vector<char>& code) {
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
        throw std::runtime_error("failed to create shader module!");
    }

    return shaderModule;
}

void VulkanInterface::CreateSwapChain() {
	SwapChain::SwapChainCreateInfo swapChainCreateInfo{};
	swapChainCreateInfo.allocator = allocator;
	swapChainCreateInfo.device = device;
	swapChainCreateInfo.physicalDevice = physicalDevice;
	swapChainCreateInfo.commandPool = commandPool;
	swapChainCreateInfo.graphicsQueue = graphicsQueue;
	swapChainCreateInfo.surface = surface;
	swapChainCreateInfo.windowManager = m_windowManager;
    
    swapChain = new SwapChain(swapChainCreateInfo);
}

void VulkanInterface::CreateInstance() {
    if (enableValidationLayers && !CheckValidationLayerSupport()) {
        throw std::runtime_error("validation layers requested, but not available!");
    }

    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Vulkan Demo";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_3;

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    auto extensions = GetRequiredExtensions();
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
    if (enableValidationLayers) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();

        PopulateDebugMessengerCreateInfo(debugCreateInfo);
        createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
    }
    else {
        createInfo.enabledLayerCount = 0;

        createInfo.pNext = nullptr;
    }

    if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
        throw std::runtime_error("failed to create instance!");
    }
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

std::vector<const char*> VulkanInterface::GetRequiredExtensions() {
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    if (enableValidationLayers) {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return extensions;
}

void VulkanInterface::SetupDebugMessenger() {
    if (!enableValidationLayers) return;

    VkDebugUtilsMessengerCreateInfoEXT createInfo;
    PopulateDebugMessengerCreateInfo(createInfo);

    if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
        throw std::runtime_error("failed to set up debug messenger!");
    }
}

void VulkanInterface::PickPhysicalDevice() {
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

    if (deviceCount == 0) {
        throw std::runtime_error("failed to find GPUs with Vulkan support!");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

    for (const auto& device : devices) {
        if (IsDeviceSuitable(device)) {
            physicalDevice = device;
            break;
        }
    }

    if (physicalDevice == VK_NULL_HANDLE) {
        throw std::runtime_error("failed to find a suitable GPU!");
    }
}

bool VulkanInterface::IsDeviceSuitable(VkPhysicalDevice physicalDevice) {
    VulkanCommonFunctions::QueueFamilyIndices indices = VulkanCommonFunctions::FindQueueFamilies(physicalDevice, surface);

    bool extensionsSupported = CheckDeviceExtensionSupport(physicalDevice);

    bool swapChainAdequate = false;
    if (extensionsSupported) {
        swapChainAdequate = swapChain->IsSwapChainAdequate(physicalDevice, surface);
    }

    VkPhysicalDeviceFeatures supportedFeatures;
    vkGetPhysicalDeviceFeatures(physicalDevice, &supportedFeatures);

    VkPhysicalDeviceDescriptorIndexingFeatures indexingFeatures = {};
    indexingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES;
    indexingFeatures.pNext = nullptr;

    VkPhysicalDeviceFeatures2 deviceFeatures2 = {};
    deviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    deviceFeatures2.pNext = &indexingFeatures;

    vkGetPhysicalDeviceFeatures2(physicalDevice, &deviceFeatures2);

    return indices.IsComplete() && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy && indexingFeatures.runtimeDescriptorArray;
}

bool VulkanInterface::CheckDeviceExtensionSupport(VkPhysicalDevice device) {
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

    std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

    for (const auto& extension : availableExtensions) {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}

void VulkanInterface::CreateLogicalDevice() {
    VulkanCommonFunctions::QueueFamilyIndices indices = VulkanCommonFunctions::FindQueueFamilies(physicalDevice, surface);

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = { indices.m_graphicsFamily.value(), indices.m_presentFamily.value() };

    float queuePriority = 1.0f;
    for (uint32_t queueFamily : uniqueQueueFamilies) {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures{};

    deviceFeatures.samplerAnisotropy = VK_TRUE;

    VkPhysicalDeviceDescriptorIndexingFeatures indexingFeatures = {};
    indexingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES;
    indexingFeatures.runtimeDescriptorArray = VK_TRUE;
    indexingFeatures.shaderSampledImageArrayNonUniformIndexing = VK_TRUE; // Usually needed too
    indexingFeatures.descriptorBindingVariableDescriptorCount = VK_TRUE;  // Optional if using variable count
    indexingFeatures.descriptorBindingPartiallyBound = VK_TRUE; // Useful for sparse arrays

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());

    createInfo.pEnabledFeatures = &deviceFeatures;

    createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();

    createInfo.pNext = &indexingFeatures;

    if (enableValidationLayers) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
    }
    else {
        createInfo.enabledLayerCount = 0;
    }

    if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
        throw std::runtime_error("failed to create logical device!");
    }

    vkGetDeviceQueue(device, indices.m_graphicsFamily.value(), 0, &graphicsQueue);
    vkGetDeviceQueue(device, indices.m_presentFamily.value(), 0, &presentQueue);
}

void VulkanInterface::CreateSurface() {
    if (glfwCreateWindowSurface(instance, m_windowManager->GetWindow(), nullptr, &surface) != VK_SUCCESS) {
        throw std::runtime_error("failed to create window surface!");
    }
}

void VulkanInterface::CreateVertexBuffers() {
    Factory objectClasses = FACTORY(MeshRenderer);

    for (auto it = meshNameToObjectMap.begin(); it != meshNameToObjectMap.end(); it++)
    {
        MeshRenderer* classInstance = objectClasses.create(it->first);

        if (classInstance == nullptr)
        {
            continue;
        }

        CreateVertexBuffer(it->first, classInstance);
        vertexBufferSizes[it->first] = static_cast<uint16_t>(classInstance->GetVertices().size());
    }
}

void VulkanInterface::CreateVertexBuffer(std::string name, MeshRenderer* meshInfo) {
    VkDeviceSize bufferSize = sizeof(meshInfo->GetVertices()[0]) * meshInfo->GetVertices().size();

    GraphicsBuffer::BufferCreateInfo stagingBufferCreateInfo = {};
	stagingBufferCreateInfo.size = bufferSize;
	stagingBufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    stagingBufferCreateInfo.properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	stagingBufferCreateInfo.allocator = allocator;
	stagingBufferCreateInfo.commandPool = commandPool;
	stagingBufferCreateInfo.graphicsQueue = graphicsQueue;
    stagingBufferCreateInfo.device = device;

	GraphicsBuffer* stagingBuffer = new GraphicsBuffer(stagingBufferCreateInfo);
    stagingBuffer->LoadData(meshInfo->GetVertices().data(), (size_t)bufferSize);

    GraphicsBuffer::BufferCreateInfo vertexBufferCreateInfo = {};
    vertexBufferCreateInfo.size = bufferSize;
    vertexBufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    vertexBufferCreateInfo.properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    vertexBufferCreateInfo.allocator = allocator;
    vertexBufferCreateInfo.commandPool = commandPool;
    vertexBufferCreateInfo.graphicsQueue = graphicsQueue;
    vertexBufferCreateInfo.device = device;

	GraphicsBuffer* vertexBuffer = new GraphicsBuffer(vertexBufferCreateInfo);

    stagingBuffer->CopyBuffer(vertexBuffer, bufferSize);
    stagingBuffer->DestroyBuffer();

    vertexBuffers[name] = vertexBuffer;
}

void VulkanInterface::CreateIndexBuffers() {
    Factory objectClasses = FACTORY(MeshRenderer);
    for (auto it = meshNameToObjectMap.begin(); it != meshNameToObjectMap.end(); it++)
    {
        MeshRenderer* classInstance = objectClasses.create(it->first);

        if (classInstance == nullptr)
        {
            continue;
        }

        if (!classInstance->IsIndexed())
            return;

        CreateIndexBuffer(it->first, classInstance);
        indexBufferSizes[it->first] = static_cast<uint16_t>(classInstance->GetIndices().size());
    }
}

void VulkanInterface::CreateIndexBuffer(std::string name, MeshRenderer* meshInfo) {
    VkDeviceSize bufferSize = sizeof(meshInfo->GetIndices()[0]) * meshInfo->GetIndices().size();

    GraphicsBuffer::BufferCreateInfo stagingBufferCreateInfo = {};
    stagingBufferCreateInfo.size = bufferSize;
    stagingBufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    stagingBufferCreateInfo.properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    stagingBufferCreateInfo.allocator = allocator;
    stagingBufferCreateInfo.commandPool = commandPool;
    stagingBufferCreateInfo.graphicsQueue = graphicsQueue;
    stagingBufferCreateInfo.device = device;

    GraphicsBuffer* stagingBuffer = new GraphicsBuffer(stagingBufferCreateInfo);
    stagingBuffer->LoadData(meshInfo->GetIndices().data(), (size_t)bufferSize);

    GraphicsBuffer::BufferCreateInfo indexBufferCreateInfo = {};
    indexBufferCreateInfo.size = bufferSize;
    indexBufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    indexBufferCreateInfo.properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    indexBufferCreateInfo.allocator = allocator;
    indexBufferCreateInfo.commandPool = commandPool;
    indexBufferCreateInfo.graphicsQueue = graphicsQueue;
    indexBufferCreateInfo.device = device;

	GraphicsBuffer* indexBuffer = new GraphicsBuffer(indexBufferCreateInfo);

	stagingBuffer->CopyBuffer(indexBuffer, bufferSize);

	stagingBuffer->DestroyBuffer();

    indexBuffers[name] = indexBuffer;
}

uint32_t VulkanInterface::FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    throw std::runtime_error("failed to find suitable memory type!");
}

std::vector<char> VulkanInterface::ReadFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        throw std::runtime_error("failed to open file!");
    }

    size_t fileSize = (size_t)file.tellg();
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);

    file.close();

    return buffer;
}

VkResult VulkanInterface::CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    }
    else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void VulkanInterface::DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance, debugMessenger, pAllocator);
    }
}

void VulkanInterface::CreateVMAAllocator()
{
    VmaVulkanFunctions vulkanFunctions = {};
    vulkanFunctions.vkGetInstanceProcAddr = &vkGetInstanceProcAddr;
    vulkanFunctions.vkGetDeviceProcAddr = &vkGetDeviceProcAddr;

    VmaAllocatorCreateInfo allocatorCreateInfo = {};
    allocatorCreateInfo.flags = VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT;
    allocatorCreateInfo.vulkanApiVersion = VK_API_VERSION_1_2;
    allocatorCreateInfo.physicalDevice = physicalDevice;
    allocatorCreateInfo.device = device;
    allocatorCreateInfo.instance = instance;
    allocatorCreateInfo.pVulkanFunctions = &vulkanFunctions;

    vmaCreateAllocator(&allocatorCreateInfo, &allocator);
}

void VulkanInterface::CreateInstanceBuffers()
{
    Factory objectClasses = FACTORY(MeshRenderer);

    for (uint32_t frameIndex = 0; frameIndex < MAX_FRAMES_IN_FLIGHT; frameIndex++)
    {
        for (auto it = meshNameToObjectMap.begin(); it != meshNameToObjectMap.end(); it++)
        {
            VkDeviceSize bufferSize = sizeof(VulkanCommonFunctions::InstanceInfo) * MAX_OBJECTS;

			GraphicsBuffer::BufferCreateInfo instanceBufferCreateInfo = {};
			instanceBufferCreateInfo.size = bufferSize;
			instanceBufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
			instanceBufferCreateInfo.properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
			instanceBufferCreateInfo.allocator = allocator;
			instanceBufferCreateInfo.commandPool = commandPool;
			instanceBufferCreateInfo.graphicsQueue = graphicsQueue;
			instanceBufferCreateInfo.device = device;

			GraphicsBuffer* instanceBuffer = new GraphicsBuffer(instanceBufferCreateInfo);
			instanceBuffers[frameIndex][it->first] = instanceBuffer;
        }
    }
}

void VulkanInterface::CreateInstanceBuffer(std::string objectName)
{
    vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

    std::vector<VulkanCommonFunctions::InstanceInfo> objectInfo;
    std::set<ObjectHandle> objectHandles = meshNameToObjectMap[objectName];

    for (auto it = objectHandles.begin(); it != objectHandles.end(); it++)
    {
        ObjectHandle currentHandle = *it;

        if (currentHandle == INVALID_OBJECT_HANDLE || !objects.contains(currentHandle))
        {
            continue;
        }

        RenderObject* object = objects[currentHandle];

		MeshRenderer* meshRenderer = object->GetComponent<MeshRenderer>();

        if (meshRenderer == nullptr)
            continue;

        VulkanCommonFunctions::InstanceInfo info = object->GetInstanceInfo();
        objectInfo.push_back(info);
    }

    VkDeviceSize bufferSize = meshNameToObjectMap[objectName].size() * sizeof(VulkanCommonFunctions::InstanceInfo);

	instanceBuffers[currentFrame][objectName]->LoadData(objectInfo.data(), (size_t)bufferSize);
}

void VulkanInterface::DrawFrame() {
    for (auto it = meshNameToObjectMap.begin(); it != meshNameToObjectMap.end(); it++)
    {
        CreateInstanceBuffer(it->first);
    }

    vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

    vkResetFences(device, 1, &inFlightFences[currentFrame]);

    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(device, swapChain->GetVkSwapChain(), UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        RecreateSwapChain();
        return;
    }
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("failed to acquire swap chain image!");
    }

    UpdateUniformBuffer(currentFrame);

    vkResetCommandBuffer(frameCommandBuffers[currentFrame], 0);

    BeginDrawFrameCommandBuffer(frameCommandBuffers[currentFrame], imageIndex);

    for (auto it = meshNameToObjectMap.begin(); it != meshNameToObjectMap.end(); it++)
    {
        DrawObjectCommandBuffer(frameCommandBuffers[currentFrame], it->first);
    }

    EndDrawFrameCommandBuffer(frameCommandBuffers[currentFrame]);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    std::vector<VkSemaphore> waitSemaphores;
    std::vector<VkPipelineStageFlags> waitStages;
    waitSemaphores.push_back(imageAvailableSemaphores[currentFrame]);
    waitStages.push_back(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);

    submitInfo.waitSemaphoreCount = waitSemaphores.size();
    submitInfo.pWaitSemaphores = waitSemaphores.data();
    submitInfo.pWaitDstStageMask = waitStages.data();
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &frameCommandBuffers[currentFrame];

    VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[currentFrame] };
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    VkResult temp = vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]);

    if (temp != VK_SUCCESS) {
        throw std::runtime_error("failed to submit draw command buffer!");
    }

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = { swapChain->GetVkSwapChain()};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;
    presentInfo.pResults = nullptr; // Optional
    result = vkQueuePresentKHR(presentQueue, &presentInfo);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized) {
        framebufferResized = false;
        RecreateSwapChain();
    }
    else if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to present swap chain image!");
    }

    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    renderedFirstFrame = true;
}

void VulkanInterface::UpdateUniformBuffer(uint32_t currentImage) {
    if (objects.size() == 0)
        return;
    
    VulkanCommonFunctions::GlobalInfo globalInfo;

    globalInfo.view = m_camera->GetViewMatrix();

	float aspectRatio = swapChain->GetSwapChainExtent().width / (float)swapChain->GetSwapChainExtent().height;

    globalInfo.proj = glm::perspective(glm::radians(m_camera->Zoom), aspectRatio, 0.1f, 10000.0f);
    globalInfo.proj[1][1] *= -1;

    globalInfo.cameraPosition = m_camera->Position;

    globalInfo.lightCount = 2;

    glm::vec3 whiteLightColor = glm::vec3(1.0f, 1.0f, 1.0f);

    glm::vec3 partyLightColor = glm::vec3(1.0f, 1.0f, 1.0f);
    partyLightColor.x = (sin(glfwGetTime() * 2.0f) + 1.0f) / 2.0f;
    partyLightColor.y = (sin(glfwGetTime() * 0.7f) + 1.0f) / 2.0f;
    partyLightColor.z = (sin(glfwGetTime() * 1.3f) + 1.0f) / 2.0f;

    glm::vec3 mainLightColor = whiteLightColor;
    if (partyMode)
    {
        objects[lightObjectHandle]->GetComponent<MeshRenderer>()->SetColor(partyLightColor);
        mainLightColor = partyLightColor;
    }

    glm::vec3 mainDiffuseColor = mainLightColor * 0.5f;
    glm::vec3 mainAmbientColor = mainDiffuseColor * 0.2f;

    glm::vec3 whiteDiffuseColor = whiteLightColor * 0.5f;
    glm::vec3 whiteAmbientColor = whiteDiffuseColor * 0.2f;

    std::array<VulkanCommonFunctions::LightInfo, 2> lightInfos;

    lightInfos[0].lightColor = glm::vec4(mainLightColor, 1.0);
    lightInfos[0].lightAmbient = glm::vec4(mainAmbientColor, 1.0);
    lightInfos[0].lightDiffuse = glm::vec4(mainDiffuseColor, 1.0);
    lightInfos[0].lightSpecular = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    lightInfos[0].lightPosition = glm::vec4(objects[lightObjectHandle]->GetComponent<Transform>()->GetPosition(), 1.0);
    lightInfos[0].maxLightDistance = 25.0f;

    lightInfos[1].lightColor = glm::vec4(whiteLightColor, 1.0);
    lightInfos[1].lightAmbient = glm::vec4(whiteAmbientColor, 1.0);
    lightInfos[1].lightDiffuse = glm::vec4(whiteDiffuseColor, 1.0);
    lightInfos[1].lightSpecular = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    lightInfos[1].lightPosition = glm::vec4(m_camera->Position, 1.0);
    lightInfos[1].maxLightDistance = 10.0f;

	lightInfoBuffers[currentImage]->LoadData(lightInfos.data(), sizeof(lightInfos));
	uniformBuffers[currentImage]->LoadData(&globalInfo, sizeof(globalInfo));
}

void VulkanInterface::CleanupSwapChain() {
	depthImage->DestroyImage();

    swapChain->DestroySwapChain();
}

void VulkanInterface::Cleanup() {
    vkDeviceWaitIdle(device);

    CleanupSwapChain();

    vkDestroyPipeline(device, graphicsPipeline, nullptr);
    vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
    vkDestroyRenderPass(device, renderPass, nullptr);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		uniformBuffers[i]->DestroyBuffer();
		lightInfoBuffers[i]->DestroyBuffer();
    }

    for (size_t i = 0; i < textureFiles.size(); i++)
    {
		textureImages[i]->DestroyTextureImage();
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

    vkDestroyCommandPool(device, commandPool, nullptr);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
        vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
    }

    for (uint32_t frameIndex = 0; frameIndex < MAX_FRAMES_IN_FLIGHT; frameIndex++)
    {
        vkDestroyFence(device, inFlightFences[frameIndex], nullptr);
    }

    vmaDestroyAllocator(allocator);

    if (enableValidationLayers)
    {
        DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
    }

    vkDestroySurfaceKHR(instance, surface, nullptr);

    vkDeviceWaitIdle(device);
    vkDestroyDevice(device, nullptr);

    vkDestroyInstance(instance, nullptr);

    glfwDestroyWindow(m_windowManager->GetWindow());

    glfwTerminate();
}