#include "shaders.h"
#include "../../control/logging.h"

std::vector<char> vkUtil::read_file(std::string filename)
{
  std::ifstream file(filename, std::ios::ate | std::ios::binary);

  if (!file.is_open())
  {
    std::stringstream message;
    message << "Failed to load \"" << filename << "\"";
    vkLogging::Logger::getLogger()->print(message.str());
  }

  size_t filesize{ static_cast<size_t>(file.tellg()) };

  std::vector<char> buffer(filesize);
  file.seekg(0);
  file.read(buffer.data(), filesize);

  file.close();
  return buffer;
}

vk::ShaderModule vkUtil::create_module(std::string filename, vk::Device device)
{
  std::vector<char> sourceCode = read_file(filename);

  vk::ShaderModuleCreateInfo moduleInfo = {};
  moduleInfo.flags = vk::ShaderModuleCreateFlags();
  moduleInfo.codeSize = sourceCode.size();
  moduleInfo.pCode = reinterpret_cast<const uint32_t*>(sourceCode.data());

  try
  {
    return device.createShaderModule(moduleInfo);
  }
  catch (vk::SystemError err)
  {
    std::stringstream message;
    message << "Failed to create shader module for \"" << filename << "\"";
    vkLogging::Logger::getLogger()->print(message.str());
  }
  return nullptr;
}