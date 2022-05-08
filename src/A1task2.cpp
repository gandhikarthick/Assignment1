#include <iostream>
#include <cstdlib>
#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1

#include <vulkan/vulkan.hpp>
#include <fstream>
#include <vector>
#include "initialization.h"
#include "utils.h"
#include "task_common.h"
#include "A1task2.h"

/* requires to have called prepare() because we need the buffers to be correctly created*/
std::vector<int> A1_Task2::incArray()
{
    // === prepare data ===
    std::vector<int> inputVec(workloadSize, 0u);
    for (size_t i = 0; i < inputVec.size(); i++)
    {
        inputVec[i] = i;
    }
    return inputVec;
}
void A1_Task2::defaultValues()
{
    std::vector<int> inputVec = incArray();
    // === fill buffers ===
    fillDeviceWithStagingBuffer(app.pDevice, app.device, app.transferCommandPool, app.transferQueue, inBuffer, inputVec);
}

void A1_Task2::checkDefaultValues()
{
    // ### gather the output data after having called compute() ###
    std::vector<int> result(workloadSize, 1u);

    fillHostWithStagingBuffer(app.pDevice, app.device, app.transferCommandPool, app.transferQueue, outBuffer, result);
    std::vector<int> input = incArray();

    std::vector<int> rotate = rotateCPU(input, workloadSize_w, workloadSize_h);
    int errors = 0;
    for(int i = 0 ; i < rotate.size(); i++)
        if(rotate[i] != result[i])
            errors++;
    if(errors>0)
        std::cout<<std::endl<<"=== There were " << errors << " error(s). ===" << std::endl;

/*
    std::cout << std::endl
              << std::endl
              << "========================" << std::endl
              << std::endl;
    // ### CHECK RESULT IS VALID ###
    for (unsigned int i = 0; i < workloadSize; i++)
    {
        if (i % workloadSize_h == 0 && i != 0)
            std::cout << std::endl;
        std::cout << result[i] << " ";
    }*/
}

void A1_Task2::prepare(unsigned int size_w, unsigned int size_h)
{
    this->workloadSize_w = size_w;
    this->workloadSize_h = size_h;
    this->workloadSize = size_h * size_w;

    // ||| this fills the descriptorLayoutBindings  |||
    Cmn::addStorage(task.bindings, 0); //                           |||
    Cmn::addStorage(task.bindings, 1); //                           |||
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

    this->inBuffer = makeDLocalBuffer(BFlag::eTransferDst | BFlag::eStorageBuffer, workloadSize * sizeof(unsigned int), "buffer_in");
    this->outBuffer = makeDLocalBuffer(BFlag::eTransferSrc | BFlag::eStorageBuffer, workloadSize * sizeof(unsigned int), "buffer_out");
    /*
    Todo : create descriptorsetlayout pool set...
    */

    /*
    TODO: create the storage input and output buffers
    */
    this->defaultValues();

    // ### Bind buffers to descriptor set ### (calls update several times)
    Cmn::bindBuffers(app.device, inBuffer.buf, task.descriptorSet, 0);
    Cmn::bindBuffers(app.device, outBuffer.buf, task.descriptorSet, 1);

    // ### Create Pipeline Layout ###
    vk::PushConstantRange pcr(vk::ShaderStageFlagBits::eCompute, 0, sizeof(PushStruct));
    vk::PipelineLayoutCreateInfo pipInfo(vk::PipelineLayoutCreateFlags(), 1U, &task.descriptorSetLayout, 1U, &pcr);
    task.pipelineLayout = app.device.createPipelineLayout(pipInfo);

    // ### And we're *almost* done! ###
    // we need to actually create the pipeline one we know about the workgroup size.
    // each time we want to change the work group size, we need to recreate it
}

void A1_Task2::compute(uint32_t dx, uint32_t dy, uint32_t dz, std::string file)
{
    int ngx = (workloadSize_w + dx - 1) / dx;
    int ngy = (workloadSize_h + dy - 1) / dy;
    PushStruct push{workloadSize_w, workloadSize_h};

    app.device.destroyShaderModule(task.cShader);
    std::string compute = "./build/shaders/"+file+".comp.spv";
    Cmn::createShader(app.device, task.cShader, compute);

    // ### Create Pipeline ###
    // constantID, offset, sizeof(type)
    std::array<vk::SpecializationMapEntry, 2> specEntries = std::array<vk::SpecializationMapEntry, 2>{
        vk::SpecializationMapEntry{0U, 0U, sizeof(int)},
        vk::SpecializationMapEntry{1U, sizeof(int), sizeof(int)}};
    std::array<int, 2> specValues = {int(dx), int(dy)}; //for workgroup sizes

    vk::SpecializationInfo specInfo = vk::SpecializationInfo(CAST(specEntries), specEntries.data(),
                                                             CAST(specValues) * sizeof(int), specValues.data());
    // in case a pipeline was already created, destroy it
    app.device.destroyPipeline( task.pipeline );
    vk::PipelineShaderStageCreateInfo stageInfo(vk::PipelineShaderStageCreateFlags(),
                                            vk::ShaderStageFlagBits::eCompute, task.cShader,
                                            "main", &specInfo);
    
    vk::ComputePipelineCreateInfo computeInfo(vk::PipelineCreateFlags(), stageInfo, task.pipelineLayout);
    task.pipeline = app.device.createComputePipeline(nullptr, computeInfo, nullptr).value;

    // ### finally do the compute ###
    this->dispatchWork((workloadSize_w + dx - 1) / dx, (workloadSize_h + dy - 1) / dy, 1U, push);
    // ### END CHECK ###
}

void A1_Task2::dispatchWork(uint32_t dx, uint32_t dy, uint32_t dz, PushStruct &pushConstant)
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
    app.device.freeCommandBuffers(app.computeCommandPool, 1U, &cb);

    uint64_t timestamps[2];
    vk::Result result = app.device.getQueryPoolResults(app.queryPool, 0, 2, sizeof(timestamps), &timestamps, sizeof(timestamps[0]), vk::QueryResultFlagBits::e64);
    assert(result == vk::Result::eSuccess);
    uint64_t timediff = timestamps[1] - timestamps[0];
    vk::PhysicalDeviceProperties properties = app.pDevice.getProperties();
    uint64_t nanoseconds = properties.limits.timestampPeriod * timediff;

    mstime = nanoseconds / 1000000.f;
}