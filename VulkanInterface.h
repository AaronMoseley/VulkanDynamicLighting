#pragma once

#include "RenderObject.h"
#include "Camera.h"
#include "VulkanCommonFunctions.h"
#include "GraphicsBuffer.h"
#include "GraphicsImage.h"
#include "TextureImage.h"
#include "LightSource.h"
#include "GraphicsPipeline.h"
#include "UIImage.h"

#include <map>
#include <vector>
#include <optional>
#include <set>
#include <iostream>
#include <fstream>
#include <algorithm>

class VulkanWindow;
class WindowManager;

class VulkanInterface {
public:
    VulkanInterface(WindowManager* windowManager);

    void DrawFrame(float deltaTime, std::shared_ptr<Scene> scene);

    bool HasRenderedFirstFrame() { return renderedFirstFrame; };

    void Cleanup();

    std::shared_ptr<GraphicsBuffer> CreateVertexBuffer(std::shared_ptr<MeshRenderer> object);
    std::shared_ptr<GraphicsBuffer> CreateIndexBuffer(std::shared_ptr<MeshRenderer>  object);

    std::shared_ptr<GraphicsBuffer> CreateUIVertexBuffer(std::shared_ptr<UIImage> imageObject);
    std::shared_ptr<GraphicsBuffer> CreateUIIndexBuffer(std::shared_ptr<UIImage> imageObject);

    void CreateInstanceBuffer(std::shared_ptr<MeshRenderer> object);
	std::shared_ptr<GraphicsBuffer> CreateInstanceBuffer(size_t maxObjects);
    void UpdateObjectBuffers(std::shared_ptr<MeshRenderer> objectMesh);
    bool HasTexture(std::string textureFilePath) { return std::find(textureFilePaths.begin(), textureFilePaths.end(), textureFilePath) != textureFilePaths.end(); };
    void UpdateTextureResources(std::string newTextureFilePath, bool alreadyInitialized=true);
    void CreateDepthResources();

    void InitializeVulkan();

    void CleanupSwapChain();

private:
    void CreateVMAAllocator();

	void CreateDescriptorSetLayouts();
    void CreatePrimaryDescriptorSetLayout();
	void CreateUIDescriptorSetLayout();

    void CreateDescriptorPools();
	void CreatePrimaryDescriptorPool();
	void CreateUIDescriptorPool();

    void CreateAllDescriptorSets();
	void CreatePrimaryDescriptorSets();
	void CreateUIDescriptorSets();

    void CreateGraphicsPipelines();
    void CreatePrimaryGraphicsPipeline();
	void CreateUIGraphicsPipeline();

    void CreateTextureImage(std::string textureFilePath);
    void CreateTextureImageView(std::string textureFilePath);
    void CreateTextureSampler(std::string textureFilePath);
    void CreateUniformBuffers();

    VkFormat FindDepthFormat();
    VkFormat FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
    void BeginDrawFrameCommandBuffer(VkCommandBuffer commandBuffer);
    void DrawInstancedObjectCommandBuffer(VkCommandBuffer commandBuffer, std::string objectName, size_t objectCount);
    void DrawSingleObjectCommandBuffer(VkCommandBuffer commandBuffer, std::shared_ptr<RenderObject> currentObject);
    void SwitchToUIPipeline(VkCommandBuffer commandBuffer);
	void DrawUIElementCommandBuffer(VkCommandBuffer commandBuffer, std::shared_ptr<RenderObject> currentObject);
    void EndDrawFrameCommandBuffer(VkCommandBuffer commandBuffer);
    void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
    static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);
    bool CheckValidationLayerSupport();
    void UpdateInstanceBuffer(std::string objectName, std::set<VulkanCommonFunctions::ObjectHandle> objectHandles, std::map<VulkanCommonFunctions::ObjectHandle, std::shared_ptr<RenderObject>> objects);
    void UpdateUniformBuffer(uint32_t currentImage, std::map<VulkanCommonFunctions::ObjectHandle, std::shared_ptr<RenderObject>> objects);

    static const int MAX_FRAMES_IN_FLIGHT = 3;

    VkInstance instance;
    VkDebugUtilsMessengerEXT debugMessenger;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice device;
    VkQueue graphicsQueue;
    VkSurfaceKHR surface;
    VkQueue presentQueue;
    VkCommandPool commandPool;

    std::shared_ptr<GraphicsPipeline> m_mainGraphicsPipeline = VK_NULL_HANDLE;
	std::shared_ptr<GraphicsPipeline> m_uiGraphicsPipeline = VK_NULL_HANDLE;

    std::map<std::string, std::shared_ptr<GraphicsBuffer>> vertexBuffers;
    std::map<std::string, std::shared_ptr<GraphicsBuffer>> indexBuffers;

    std::map<std::string, uint16_t> vertexBufferSizes;
    std::map<std::string, uint16_t> indexBufferSizes;

	std::vector<std::shared_ptr<GraphicsBuffer>> uniformBuffers;
	std::vector<std::shared_ptr<GraphicsBuffer>> lightInfoBuffers;
	std::vector<std::shared_ptr<GraphicsBuffer>> uiUniformBuffers;

    std::vector<std::string> textureFilePaths;
    std::map<std::string, size_t> texturePathToIndex;
    std::map<std::string, std::shared_ptr<TextureImage>> textureImages;

	std::string kDefaultTexturePath = "textures\\DefaultTexture.png";

    size_t maxLightCount = 200;

    uint32_t currentFrame = 0;

    VkDescriptorSetLayout m_primaryDescriptorSetLayout = VK_NULL_HANDLE;
    VkDescriptorSetLayout m_uiDescriptorSetLayout = VK_NULL_HANDLE;

    VkDescriptorPool m_primaryDescriptorPool = VK_NULL_HANDLE;
	VkDescriptorPool m_uiDescriptorPool = VK_NULL_HANDLE;

    std::vector<VkDescriptorSet> primaryDescriptorSets;
	std::vector<VkDescriptorSet> uiDescriptorSets;

    std::shared_ptr<GraphicsImage> depthImage;

    bool framebufferResized = false;

    std::array<std::map<std::string, std::shared_ptr<GraphicsBuffer>>, MAX_FRAMES_IN_FLIGHT> instanceBuffers;

    VmaAllocator allocator;

    WindowManager* m_windowManager;
    VulkanWindow* m_vulkanWindow;

	const std::string customMeshName = "CustomMesh";


    const std::vector<const char*> validationLayers = {
        "VK_LAYER_KHRONOS_validation"
    };

    const std::vector<const char*> deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

#ifdef NDEBUG
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif

    bool renderedFirstFrame = false;
};