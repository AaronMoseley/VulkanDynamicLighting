#pragma once

#define VK_USE_PLATFORM_WIN32_KHR

#include "vk_mem_alloc.h"
#include "stb_image.h"

#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include "RenderObject.h"
#include "Camera.h"
#include "VulkanCommonFunctions.h"
#include "GraphicsBuffer.h"
#include "GraphicsImage.h"
#include "TextureImage.h"
#include "Swapchain.h"
#include "LightSource.h"

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
    VulkanInterface(std::shared_ptr<WindowManager> windowManager);

    void DrawFrame(float deltaTime, std::shared_ptr<Scene> scene);

    bool HasRenderedFirstFrame() { return renderedFirstFrame; };

    void Cleanup();

    std::shared_ptr<GraphicsBuffer> CreateVertexBuffer(std::shared_ptr<MeshRenderer> object);
    std::shared_ptr<GraphicsBuffer> CreateIndexBuffer(std::shared_ptr<MeshRenderer>  object);
    void CreateInstanceBuffer(std::shared_ptr<MeshRenderer> object);
	std::shared_ptr<GraphicsBuffer> CreateInstanceBuffer(size_t maxObjects);
    void UpdateObjectBuffers(std::shared_ptr<MeshRenderer> objectMesh);
    bool HasTexture(std::string textureFilePath) { return std::find(textureFilePaths.begin(), textureFilePaths.end(), textureFilePath) != textureFilePaths.end(); };
    void UpdateTextureResources(std::string newTextureFilePath, bool alreadyInitialized=true);

    void InitializeVulkan();
    void InitializeSwapChain();

    void CleanupSwapChain();

private:
    void CreateVMAAllocator();
    void CreateDescriptorSetLayout();
    void CreateGraphicsPipeline();
    void CreateDepthResources();
    void CreateTextureImage(std::string textureFilePath);
    void CreateTextureImageView(std::string textureFilePath);
    void CreateTextureSampler(std::string textureFilePath);
    void CreateUniformBuffers();
    void CreateDescriptorPool();
    void CreateDescriptorSets();

    VkFormat FindDepthFormat();
    VkFormat FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
    void BeginDrawFrameCommandBuffer(VkCommandBuffer commandBuffer);
    void DrawInstancedObjectCommandBuffer(VkCommandBuffer commandBuffer, std::string objectName, size_t objectCount);
    void DrawSingleObjectCommandBuffer(VkCommandBuffer commandBuffer, std::shared_ptr<RenderObject> currentObject);
    void EndDrawFrameCommandBuffer(VkCommandBuffer commandBuffer);
    VkShaderModule CreateShaderModule(const std::vector<char>& code);
    void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
    static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);
    bool CheckValidationLayerSupport();
    std::vector<char> ReadFile(const std::string& filename);
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
    std::shared_ptr<SwapChain> swapChain;
    VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
    VkPipeline graphicsPipeline = VK_NULL_HANDLE;
    VkCommandPool commandPool;

    std::map<std::string, std::shared_ptr<GraphicsBuffer>> vertexBuffers;
    std::map<std::string, std::shared_ptr<GraphicsBuffer>> indexBuffers;

    std::map<std::string, uint16_t> vertexBufferSizes;
    std::map<std::string, uint16_t> indexBufferSizes;

	std::vector< std::shared_ptr<GraphicsBuffer>> uniformBuffers;
	std::vector< std::shared_ptr<GraphicsBuffer>> lightInfoBuffers;

    std::vector<std::string> textureFilePaths;
    std::map<std::string, size_t> texturePathToIndex;
    std::map<std::string, std::shared_ptr<TextureImage>> textureImages;

	std::string kDefaultTexturePath = "textures\\DefaultTexture.png";

    size_t maxLightCount = 200;

    uint32_t currentFrame = 0;

    VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
    std::vector<VkDescriptorSet> descriptorSets;

    std::shared_ptr<GraphicsImage> depthImage;

    bool framebufferResized = false;

    std::array<std::map<std::string, std::shared_ptr<GraphicsBuffer>>, MAX_FRAMES_IN_FLIGHT> instanceBuffers;

    VmaAllocator allocator;

    std::shared_ptr<WindowManager> m_windowManager;
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