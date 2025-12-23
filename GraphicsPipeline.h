#pragma once

#include <fstream>

#include "VulkanCommonFunctions.h"

class VulkanWindow;

struct GraphicsPipelineCreateInfo {
	std::string vertexShaderFilePath;
	std::string fragmentShaderFilePath;
	VkDescriptorSetLayout descriptorSetLayout;
	VkDevice device;
	VulkanWindow* vulkanWindow;
	bool uiBasedPipeline = false;
};

class GraphicsPipeline {
public: 
	GraphicsPipeline(GraphicsPipelineCreateInfo pipelineCreateInfo);

	void CreatePipeline();
	void DestroyPipeline();

	void SetDescriptorSetLayout(VkDescriptorSetLayout descriptorSetLayout) { m_descriptorSetLayout = descriptorSetLayout; }

	VkPipeline GetVkPipeline() { return m_graphicsPipeline; }
	VkPipelineLayout GetVkPipelineLayout() { return m_pipelineLayout; }

private:
	void CreatePipelineLayout();
	VkShaderModule CreateShaderModule(const std::vector<char>& code);
	std::vector<char> ReadFile(const std::string& filename);

	VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
	VkPipeline m_graphicsPipeline = VK_NULL_HANDLE;
	VkDescriptorSetLayout m_descriptorSetLayout = VK_NULL_HANDLE;
	VkDevice m_device = VK_NULL_HANDLE;

	std::string m_vertexShaderFilePath;
	std::string m_fragmentShaderFilePath;

	bool m_uiBasedPipeline = false;

	VulkanWindow* m_vulkanWindow;
};