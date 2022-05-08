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

    // ||| this fills the descriptorLayoutBindings  |||
    Cmn::addStorage(task.bindings, 0); //                           |||
    Cmn::addStorage(task.bindings, 1); //                           |||
    Cmn::addStorage(task.bindings, 2); //                           |||
    // ||| ======================================== |||

    Cmn::createDescriptorSetLayout(app.device, task.bindings, task.descriptorSetLayout);
    Cmn::createDescriptorPool(app.device, task.bindings, task.descriptorPool);
    Cmn::allocateDescriptorSet(app.device, task.descriptorSet, task.descriptorPool, task.descriptorSetLayout);
    // ### DescriptorSet is created but not filled yet ###

    // ### create buffers, get their index in the task.buffers[] array ###
    using BFlag = vk::BufferUsageFlagBits;
    auto makeDLocalBuffer = [this](vk::BufferUsageFlags usage, vk::DeviceSize size, std::string name) -> Buffer
    {
        Buffer b;
        createBuffer(app.pDevice, app.device, size, usage, vk::MemoryPropertyFlagBits::eDeviceLocal, name, b.buf, b.mem);
        return b;
    };

    this->inBuffer1 = makeDLocalBuffer(BFlag::eTransferDst | BFlag::eStorageBuffer, workloadSize * sizeof(unsigned int), "buffer_in1");
    this->inBuffer2 = makeDLocalBuffer(BFlag::eTransferDst | BFlag::eStorageBuffer, workloadSize * sizeof(unsigned int), "buffer_in2");
    this->outBuffer = makeDLocalBuffer(BFlag::eTransferSrc | BFlag::eStorageBuffer, workloadSize * sizeof(unsigned int), "buffer_out");

    // ### Fills inBuffer1 and inBuffer2 ###
    this->defaultValues();

    // fillDeviceWithStagingBuffer must be used with appropriate flag: transferDst
    // fillDeviceWithStagingBuffer(app.pDevice, app.device, app.transferCommandPool, app.transferQueue, inBuffer1, inputVec);
    // fillDeviceWithStagingBuffer(app.pDevice, app.device, app.transferCommandPool, app.transferQueue, inBuffer2, inputVec2);
    //fillDeviceBuffer(app.device, in1Buffer.mem, inputVec);
    //fillDeviceBuffer(app.device, in2Buffer.mem, inputVec2);

    // ### Bind buffers to descriptor set ### (calls update several times)
    Cmn::bindBuffers(app.device, inBuffer1.buf, task.descriptorSet, 0);
    Cmn::bindBuffers(app.device, inBuffer2.buf, task.descriptorSet, 1);
    Cmn::bindBuffers(app.device, outBuffer.buf, task.descriptorSet, 2);

    // ### Create Pipeline Layout ###
    vk::PushConstantRange pcr(vk::ShaderStageFlagBits::eCompute, 0, sizeof(PushStruct));
    vk::PipelineLayoutCreateInfo pipInfo(vk::PipelineLayoutCreateFlags(), 1U, &task.descriptorSetLayout, 1U, &pcr);
    task.pipelineLayout = app.device.createPipelineLayout(pipInfo);

    // ### And we're *almost* done! ###
    // we need to actually create the pipeline one we know about the workgroup size.
    // each time we want to change the work group size, we need to recreate it
}

void A1_Task1::compute(uint32_t dx, uint32_t dy, uint32_t dz, std::string file)
{

    std::string compute = "./shaders/" + file + ".comp.spv";
    app.device.destroyShaderModule(task.cShader);
    Cmn::createShader(app.device, task.cShader, compute);

    // ### Create Pipeline ###
    // constantID, offset, sizeof(type)
    std::array<vk::SpecializationMapEntry, 1> specEntries = std::array<vk::SpecializationMapEntry, 1>{
        {{0U, 0U, sizeof(int)}}};
    std::array<int, 1> specValues = {int(dx)}; //for workgroup sizes

    vk::SpecializationInfo specInfo = vk::SpecializationInfo(CAST(specEntries), specEntries.data(),
                                                             CAST(specValues) * sizeof(int), specValues.data());

    // in case a pipeline was already created, destroy it
    app.device.destroyPipeline(task.pipeline);
    vk::PipelineShaderStageCreateInfo stageInfo(vk::PipelineShaderStageCreateFlags(),
                                                vk::ShaderStageFlagBits::eCompute, task.cShader,
                                                "main", &specInfo);

    vk::ComputePipelineCreateInfo computeInfo(vk::PipelineCreateFlags(), stageInfo, task.pipelineLayout);
    task.pipeline = app.device.createComputePipeline(nullptr, computeInfo, nullptr).value;

    //Cmn::createPipeline(app.device, task.pipeline, task.pipelineLayout, specInfo, task.cShader);

    uint32_t groupCount = (workloadSize + dx) / (dx);
    PushStruct push{workloadSize};
    // ### finally do the compute ###
    this->dispatchWork(groupCount, 1U, 1U, push);
    // ### END CHECK ###
    // now you need to call checkDefaultValues(); or appropriate
}

void A1_Task1::dispatchWork(uint32_t dx, uint32_t dy, uint32_t dz, PushStruct &pushConstant)
{
    vk::CommandBufferAllocateInfo allocInfo(
        app.computeCommandPool, vk::CommandBufferLevel::ePrimary, 1U);
    vk::CommandBuffer cb = app.device.allocateCommandBuffers(allocInfo)[0];

    vk::CommandBufferBeginInfo beginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

    cb.begin(beginInfo);
    cb.resetQueryPool(app.queryPool, 0, 2);
    cb.writeTimestamp(vk::PipelineStageFlagBits::eAllCommands, app.queryPool, 0);
    cb.pushConstants(task.pipelineLayout, vk::ShaderStageFlagBits::eCompute,
                     0, sizeof(PushStruct), &pushConstant);
    cb.bindPipeline(vk::PipelineBindPoint::eCompute, task.pipeline);
    cb.bindDescriptorSets(vk::PipelineBindPoint::eCompute, task.pipelineLayout,
                          0U, 1U, &task.descriptorSet, 0U, nullptr);
    cb.dispatch(dx, dy, dz);
    cb.writeTimestamp(vk::PipelineStageFlagBits::eAllCommands, app.queryPool, 1);
    cb.end();

    // submit the command buffer to the queue and set up a fence.
    vk::SubmitInfo submitInfo = vk::SubmitInfo(0, nullptr, nullptr, 1, &cb); // submit a single command buffer
    vk::Fence fence = app.device.createFence(vk::FenceCreateInfo());         // fence makes sure the control is not returned to CPU till command buffer is depleted

    app.computeQueue.submit({submitInfo}, fence);
    vk::Result haveIWaited = app.device.waitForFences({fence}, true, uint64_t(-1)); // wait for the fence indefinitely
    app.device.destroyFence(fence);

    uint64_t timestamps[2];
    vk::Result result = app.device.getQueryPoolResults(app.queryPool, 0, 2, sizeof(timestamps), &timestamps, sizeof(timestamps[0]), vk::QueryResultFlagBits::e64);
    assert(result == vk::Result::eSuccess);
    uint64_t timediff = timestamps[1] - timestamps[0];
    vk::PhysicalDeviceProperties properties = app.pDevice.getProperties();
    uint64_t nanoseconds = properties.limits.timestampPeriod * timediff;

    mstime = nanoseconds / 1000000.f;
    app.device.freeCommandBuffers(app.computeCommandPool, 1U, &cb);
}