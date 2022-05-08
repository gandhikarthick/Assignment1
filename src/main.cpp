#include <iostream>
#include <cstdlib>
#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#include <vulkan/vulkan.hpp>
#include <fstream>
#include <vector>
#include "initialization.h"
#include "utils.h"
#include "A1task1.h"
#include "A1task2.h"
#include "renderdoc.h"
#include <CSVWriter.h>

int main()
{
    try
    {
        AppResources app;
        initApp(app);

        renderdoc::initialize();
        renderdoc::startCapture();

        CSVWriter csv;
        
        A1_Task1 task(app);
        
        csv.newRow() << "dx" << "dy" << "dz" << "vecsize" << "timing";
        task.prepare(1024*1024+31);
        
        task.compute(1024,1,1);
        task.checkDefaultValues();
        csv.newRow() << 1024 << 1 << 1 << 1024*1024 << task.mstime;

        task.compute(512,1,1);
        task.checkDefaultValues();
        csv.newRow() << 512 << 1 << 1 << 1024*1024 << task.mstime;

        task.compute(256,1,1);
        task.checkDefaultValues();
        csv.newRow() << 256 << 1 << 1 << 1024*1024 << task.mstime;

        task.compute(128,1,1);
        task.checkDefaultValues();
        csv.newRow() << 128 << 1 << 1 << 1024*1024 << task.mstime;

        task.compute(64,1,1);
        task.checkDefaultValues();
        csv.newRow() << 64 << 1 << 1 << 1024*1024 << task.mstime;
        
        task.compute(32,1,1);
        task.checkDefaultValues();
        csv.newRow() << 32 << 1 << 1 << 1024*1024 << task.mstime;
        task.cleanup();
        std::cout << csv << std::endl;
        csv.writeToFile("vectorAddition.csv" ,false);
        
      /*  A1_Task2 A1task2(app);
        A1task2.prepare(3200, 4000);

        A1task2.compute(32, 16, 1);
        A1task2.checkDefaultValues();

        std::cout <<"took: "<< A1task2.mstime<<" ms"<<std::endl;

        A1task2.compute(32, 16, 1, "matrixRotOpti");
        A1task2.checkDefaultValues();
        std::cout <<"took: "<< A1task2.mstime<<" ms"<<std::endl;

        A1task2.cleanup();*/
    
        renderdoc::endCapture();

        app.destroy();
    }
    catch (vk::SystemError &err)
    {
        std::cout << "vk::SystemError: " << err.what() << std::endl;
        exit(-1);
    }
    catch (std::exception &err)
    {
        std::cout << "std::exception: " << err.what() << std::endl;
        exit(-1);
    }
    catch (...)
    {
        std::cout << "unknown error\n";
        exit(-1);
    }
    return EXIT_SUCCESS;
}