#pragma once
#include "../../config.h"

namespace vkMesh {

	class ObjMesh {
	public:
		std::vector<float> vertices;
		std::vector<uint32_t> indices;
		std::vector<glm::vec3> v, vn;
		std::vector<glm::vec2> vt;
		std::unordered_map<std::string, uint32_t> history;
		std::unordered_map<std::string, glm::vec3> colorLookup;
		glm::vec3 brushColor;
		glm::mat4 preTransform;

		void load(const char* objFilepath, const char* mtlFilepath, glm::mat4 preTransform);

		void readVertexData(const std::vector<std::string>& words);

		void readTexcoordData(const std::vector<std::string>& words);

		void readNormalData(const std::vector<std::string>& words);

		void readFaceData(const std::vector<std::string>& words);

		void readCorner(const std::string& vertex_description);
	};
}