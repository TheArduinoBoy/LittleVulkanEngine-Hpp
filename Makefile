CFLAGS = -std=c++17 -I. -I$(VULKAN_SDK_PATH)/include
LDFLAGS = -L$(VULKAN_SDK_PATH)/lib `pkg-config --static --libs glfw3` -lvulkan

VulkanEngine: *.cpp
	clang++ $(CFLAGS) -o VulkanEngine *.cpp $(LDFLAGS)

.PHONY: test clean

test: VulkanEngine
	./VulkanEngine

clean:
	rm -f VulkanEngine