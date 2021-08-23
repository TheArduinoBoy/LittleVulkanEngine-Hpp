#pragma once

#include "Pipeline.hpp"
#include "Window.hpp"

namespace Engine {
    class Engine {
        public:
        static constexpr int WIDTH = 800;
        static constexpr int HEIGHT = 600;

        void run();

        private:
        Window window{WIDTH, HEIGHT, "Hello Vulkan!"};
        Device device{window};
        vk::PipelineLayout pipelineLayout;
        Pipeline lvePipeline{
            device,
            "Shaders/Shader.vert.spv",
            "Shaders/Shader.frag.spv",
            Pipeline::defaultPipelineConfigInfo(WIDTH, HEIGHT)};
        };
}