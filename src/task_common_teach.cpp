#include <iostream>
#include <cstdlib>
#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#include <vulkan/vulkan.hpp>
#include <fstream>
#include <vector>
#include "task_common.h"
#include "initialization.h"
#include "utils.h"

namespace Cmn
{
    void createDescriptorSetLayout(vk::Device &device,
                                   std::vector<vk::DescriptorSetLayoutBinding> &bindings, vk::DescriptorSetLayout &descLayout)
    {
        vk::DescriptorSetLayoutCreateInfo layoutInfo({}, CAST(bindings), bindings.data());
        descLayout = device.createDescriptorSetLayout(layoutInfo);
    }

    void addStorage(std::vector<vk::DescriptorSetLayoutBinding> &bindings, uint32_t binding)
    {
        bindings.push_back(vk::DescriptorSetLayoutBinding(
            binding, vk::DescriptorType::eStorageBuffer,
            1U, vk::ShaderStageFlagBits::eCompute));
    }

    void allocateDescriptorSet(vk::Device &device, vk::DescriptorSet &descSet, vk::DescriptorPool &descPool,
                               vk::DescriptorSetLayout &descLayout)
    {
        std::cout << "allocateDescriptorSet has not been defined yet, line: " << __LINE__ << " file " << __FILE__ << std::endl;
    }

    void bindBuffers(vk::Device &device, vk::Buffer &b, vk::DescriptorSet &set, uint32_t binding)
    {
        std::cout << "bindBuffers has not been defined yet, line: " << __LINE__ << " file " << __FILE__ << std::endl;
    }

    void createDescriptorPool(vk::Device &device,
                              std::vector<vk::DescriptorSetLayoutBinding> &bindings, vk::DescriptorPool &descPool, uint32_t numDescriptorSets)
    {
        std::cout << "createDescriptorPool has not been defined yet, line: " << __LINE__ << " file " << __FILE__ << std::endl;
    }

    void createPipeline(vk::Device &device, vk::Pipeline &pipeline,
                        vk::PipelineLayout &pipLayout, vk::SpecializationInfo &specInfo,
                        vk::ShaderModule &sModule)
    {
        std::cout << "createPipeline has not been defined yet, line: " << __LINE__ << " file " << __FILE__ << std::endl;
    }

    void createShader(vk::Device &device, vk::ShaderModule &shaderModule, const std::string &filename)
    {
        std::cout << "createShader has not been defined yet, line: " << __LINE__ << " file " << __FILE__ << std::endl;
    }
}

void TaskResources::destroy(vk::Device &device)
{
    std::cout << "file : " << __FILE__ << ", line: " << __LINE__ << " you need to destroy: " << std::endl
              << "- pipeline" << std::endl
              << "- pipelineLayout" << std::endl
              << "- descriptorPool" << std::endl
              << "- descriptorSetLayout" << std::endl
              << "- shaderModule" << std::endl;
    /*
    Destroy stuff
    */
}
