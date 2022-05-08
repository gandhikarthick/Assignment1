#include <iostream>
#include <cstdlib>
#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1

#include <vulkan/vulkan.hpp>
#include <fstream>
#include <vector>
#include <algorithm>
#include "initialization.h"
#include "utils.h"
#include "task_common.h"
#include "A1task1.h"

void defaultVectors(std::vector<int> &in1, std::vector<int> &in2, size_t size)
{
    // === prepare data ===
    in1 = std::vector<int>(size, 0u);
    for (size_t i = 0; i < in1.size(); i++)
        in1[i] = static_cast<int>(i);
    in2 = std::vector<int>(in1);
    std::reverse(in2.begin(), in2.end());
}
/* requires to have called prepare() because we need the buffers to be correctly created*/
void A1_Task1::defaultValues()
{
    std::vector<int> inputVec, inputVec2;
    defaultVectors(inputVec, inputVec2, this->workloadSize);
    // === fill buffers ===
    fillDeviceWithStagingBuffer(app.pDevice, app.device, app.transferCommandPool, app.transferQueue, inBuffer1, inputVec);
    fillDeviceWithStagingBuffer(app.pDevice, app.device, app.transferCommandPool, app.transferQueue, inBuffer2, inputVec2);
}

void A1_Task1::checkDefaultValues()
{
    // ### gather the output data after having called compute() ###
    std::vector<unsigned int> result(this->workloadSize, 1u);

    fillHostWithStagingBuffer(app.pDevice, app.device, app.transferCommandPool, app.transferQueue, outBuffer, result);

    std::vector<int> inputVec, inputVec2;
    defaultVectors(inputVec, inputVec2, this->workloadSize);
    std::vector<int> outputVec(this->workloadSize, 0u);
    std::transform(inputVec.begin(), inputVec.end(), inputVec2.begin(), outputVec.begin(), std::plus<int>());

    if (std::equal(result.begin(), result.end(), outputVec.begin()))
        std::cout << "All is good it seems" << std::endl;
    else
        std::cout << " Oh no! We found errors!" << std::endl;
}

void A1_Task1::prepare(unsigned int size)
{
    this->workloadSize = size;
    
    // ### Fill the descriptorLayoutBindings  ###

    // ### Push Constant ###

    // ### Create Pipeline Layout ###

    // ### create buffers ###

    // ### Fills inBuffer1 and inBuffer2 ###
    // === prepare data ===
    std::vector<int> inputVec(workloadSize, 0u);
    for (size_t i = 0; i < inputVec.size(); i++)
        inputVec[i] = static_cast<int>(i);
    std::vector<int> inputVec2 = inputVec;
    std::reverse(inputVec2.begin(), inputVec2.end());
    // inputVec and inputVec2 are correctly filled

    // ### Create  structures ###
    // ### DescriptorSet is created but not filled yet ###
    // ### Bind buffers to descriptor set ### (calls update several times)


    // ### Preparation work done! ###
}

void A1_Task1::compute(uint32_t dx, uint32_t dy, uint32_t dz, std::string file)
{
    uint32_t groupCount; // todo: fill
    PushStruct push; // todo: fill
    // ### Create ShaderModule ###
    vk::PipelineShaderStageCreateInfo  stageInfo; // todo: fill 

    // ### Specialization constants
    // constantID, offset, sizeof(type)

    // ### Create Pipeline ###
    vk::ComputePipelineCreateInfo  computeInfo; // todo: fill

    // ### finally do the compute ###

}

void A1_Task1::dispatchWork(uint32_t dx, uint32_t dy, uint32_t dz, PushStruct &pushConstant)
{
    /* ### Create Command Buffer ### */

    /* ### Call Begin and register commands ### */

    /* ### End of Command Buffer, enqueue it and use a Fence ### */

    /* ### Collect data from the query Pool ### */
    /* Uncomment this once you've finished this function:
    ###
    uint64_t timestamps[2];
    vk::Result result = app.device.getQueryPoolResults(app.queryPool, 0, 2, sizeof(timestamps), &timestamps, sizeof(timestamps[0]), vk::QueryResultFlagBits::e64);
    assert(result == vk::Result::eSuccess);
    uint64_t timediff = timestamps[1] - timestamps[0];
    vk::PhysicalDeviceProperties properties = app.pDevice.getProperties();
    uint64_t nanoseconds = properties.limits.timestampPeriod * timediff;

    mstime = nanoseconds / 1000000.f;
    ###
    */
}