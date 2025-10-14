#pragma once

#define VK_USE_PLATFORM_WIN32_KHR

#include "vk_mem_alloc.h"
#include "stb_image.h"

#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include "RenderObject.h"
#include "Camera.h"
#include "WindowManager.h"
#include "VulkanCommonFunctions.h"
#include "GraphicsBuffer.h"
#include "GraphicsImage.h"

#include "Factory.h"

#include <map>
#include <vector>
#include <optional>
#include <set>
#include <iostream>
#include <fstream>
#include <algorithm>

class VulkanInterface {
public:
    using ObjectHandle = size_t;

    struct QueueFamilyIndices {
        std::optional<uint32_t> m_graphicsFamily;
        std::optional<uint32_t> m_presentFamily;

        bool IsComplete() {
            return m_graphicsFamily.has_value() && m_presentFamily.has_value();
        }
    };

    struct SwapChainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };

    VulkanInterface(WindowManager* windowManager, Camera* camera);

    void DrawFrame();

    ObjectHandle AddObject(RenderObject* newObject);
    bool RemoveObject(ObjectHandle objectToRemove);

    bool HasRenderedFirstFrame() { return renderedFirstFrame; };
    size_t GetObjectCount() { return objects.size(); };
    RenderObject* GetRenderObject(ObjectHandle handle);

    void Cleanup();

    static const ObjectHandle INVALID_OBJECT_HANDLE = 0;

private:
	void InitializeVulkan();

    void CreateInstance();
    void SetupDebugMessenger();
    void CreateSurface();
    void PickPhysicalDevice();
    void CreateLogicalDevice();
    void CreateVMAAllocator();
    void CreateSwapChain();
    void CreateImageViews();
    void CreateRenderPass();
    void CreateDescriptorSetLayout();
    void CreateGraphicsPipeline();
    void CreateCommandPool();
    void CreateDepthResources();
    void CreateTextureImage();
    void CreateTextureImageView();
    void CreateTextureSampler();
    void CreateFramebuffers();
    void CreateVertexBuffers();
    void CreateIndexBuffers();
    void CreateInstanceBuffers();
    void CreateUniformBuffers();
    void CreateDescriptorPool();
    void CreateDescriptorSets();
    void CreateCommandBuffers();
    void CreateSyncObjects();

    VkFormat FindDepthFormat();
    VkFormat FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
    //VkImageView CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);
    //void CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VmaAllocation& imageMemory);
    //void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
    //void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
    void RecreateSwapChain();
    void BeginDrawFrameCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
    void DrawObjectCommandBuffer(VkCommandBuffer commandBuffer, std::string objectName);
    void EndDrawFrameCommandBuffer(VkCommandBuffer commandBuffer);
    VkShaderModule CreateShaderModule(const std::vector<char>& code);
    void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
    static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);
    bool CheckValidationLayerSupport();
    std::vector<const char*> GetRequiredExtensions();
    bool IsDeviceSuitable(VkPhysicalDevice device);
    bool CheckDeviceExtensionSupport(VkPhysicalDevice device);
    QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);
    void CreateVertexBuffer(std::string name, RenderObject* object);
    void CreateIndexBuffer(std::string name, RenderObject* object);
    uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
    SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device);
    VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
    VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
    std::vector<char> ReadFile(const std::string& filename);
    VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
    void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);
    void CreateInstanceBuffer(std::string objectName);
    void UpdateUniformBuffer(uint32_t currentImage);
    void CleanupSwapChain();

    static const int MAX_FRAMES_IN_FLIGHT = 3;
    static const size_t MAX_OBJECTS = 10000;

    std::map<ObjectHandle, RenderObject*> objects;
    std::map<std::string, std::set<ObjectHandle>> nameToObjectMap;

    VkInstance instance;
    VkDebugUtilsMessengerEXT debugMessenger;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice device;
    VkQueue graphicsQueue;
    VkSurfaceKHR surface;
    VkQueue presentQueue;
    VkSwapchainKHR swapChain;
    std::vector<GraphicsImage*> swapChainImages;
    //VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;
    //std::vector<VkImageView> swapChainImageViews;
    VkRenderPass renderPass;
    VkDescriptorSetLayout descriptorSetLayout;
    VkPipelineLayout pipelineLayout;
    VkPipeline graphicsPipeline;
    std::vector<VkFramebuffer> swapChainFramebuffers;
    VkCommandPool commandPool;
    std::array<VkSemaphore, MAX_FRAMES_IN_FLIGHT> imageAvailableSemaphores;
    std::array<VkSemaphore, MAX_FRAMES_IN_FLIGHT> renderFinishedSemaphores;
    std::array<VkFence, MAX_FRAMES_IN_FLIGHT> inFlightFences;

    std::map<std::string, GraphicsBuffer*> vertexBuffers;
    std::map<std::string, GraphicsBuffer*> indexBuffers;

    std::map<std::string, uint16_t> vertexBufferSizes;
    std::map<std::string, uint16_t> indexBufferSizes;

    std::array<VkCommandBuffer, MAX_FRAMES_IN_FLIGHT> frameCommandBuffers;

	std::vector<GraphicsBuffer*> uniformBuffers;
	std::vector<GraphicsBuffer*> lightInfoBuffers;

    std::array<std::string, 2> textureFiles = { "textures/SandTexture.png", "textures/OtherTexture.png" };
    std::vector<GraphicsImage*> textureImages;
    //std::vector<VmaAllocation> textureImagesMemory;
    //std::vector<VkImageView> textureImageViews;
    std::vector<VkSampler> textureSamplers;

    size_t maxLightCount = 50;

    uint32_t currentFrame = 0;

    VkDescriptorPool descriptorPool;
    std::vector<VkDescriptorSet> descriptorSets;

    GraphicsImage* depthImage;
    //VmaAllocation depthImageMemory;
    //VkImageView depthImageView;

    ObjectHandle lightObjectHandle = 1;

    bool framebufferResized = false;

    std::array<std::map<std::string, GraphicsBuffer*>, MAX_FRAMES_IN_FLIGHT> instanceBuffers;

    VmaAllocator allocator;

    WindowManager* m_windowManager;
    Camera* m_camera;

    ObjectHandle m_currentObjectHandle = 0;

    bool partyMode = false;
    float lightOrbitRadius = 10.0f;


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