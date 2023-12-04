#include "obj_mesh.h"

static std::vector<std::string> split(std::string line, std::string delimiter)
{
	std::vector<std::string> split_line;

	size_t pos = 0;
	std::string token;
	while ((pos = line.find(delimiter)) != std::string::npos)
	{
		token = line.substr(0, pos);
		split_line.push_back(token);
		line.erase(0, pos + delimiter.length());
	}
	if (!line.empty())
		split_line.push_back(line);

	return split_line;
}

void vkmesh::ObjMesh::load(const char* objFilepath, const char* mtlFilepath, glm::mat4 preTransform)
{
	this->preTransform = preTransform;

	std::ifstream file;
	file.open(mtlFilepath);
	std::string line;
	std::string materialName;
	std::vector<std::string> words;

	while (std::getline(file, line))
	{
		if (line.empty())
			continue;

		words = split(line, " ");

		if (!words[0].compare("newmtl"))
			materialName = words[1];

		if (!words[0].compare("Kd"))
		{
			brushColor = glm::vec3(std::stof(words[1]), std::stof(words[2]), std::stof(words[3]));
			colorLookup.insert({ materialName,brushColor });
		}
	}

	file.close();

	size_t vertex_count = 0;
	size_t texcoord_count = 0;
	size_t normal_count = 0;
	size_t corner_count = 0;

	file.open(objFilepath);

	while (std::getline(file, line))
	{
		if (line.empty())
			continue;

		words = split(line, " ");

		if (!words[0].compare("v"))
			vertex_count += 1;

		else if (!words[0].compare("vt"))
			texcoord_count += 1;

		else if (!words[0].compare("vn"))
			normal_count += 1;

		else if (!words[0].compare("f"))
		{
			size_t triangle_count = words.size() - 3;
			corner_count += 3 * triangle_count;
		}
	}

	file.close();

	v.reserve(vertex_count);
	vt.reserve(texcoord_count);
	vn.reserve(normal_count);
	vertices.reserve(corner_count);
	indices.reserve(corner_count);

	file.open(objFilepath);

	while (std::getline(file, line))
	{
		if (line.empty())
			continue;
			
		words = split(line, " ");

		if (!words[0].compare("v"))
			readVertexData(words);

		else if (!words[0].compare("vt"))
			readTexcoordData(words);

		else if (!words[0].compare("vn"))
			readNormalData(words);

		else if (!words[0].compare("usemtl"))
		{
			if (colorLookup.contains(words[1]))
				brushColor = colorLookup[words[1]];
			else
				brushColor = glm::vec3(1.f);
		}
		else if (!words[0].compare("f"))
			readFaceData(words);
	}

	file.close();
}

void vkmesh::ObjMesh::readVertexData(const std::vector<std::string>& words)
{
	glm::vec4 new_vertex = glm::vec4(std::stof(words[1]), std::stof(words[2]), std::stof(words[3]), 1.f);
	glm::vec3 transformed_vertex = glm::vec3(preTransform * new_vertex);
	v.push_back(transformed_vertex);
}

void vkmesh::ObjMesh::readTexcoordData(const std::vector<std::string>& words)
{
	// To account for difference between Vulkan and OBJ coordinate systems, we need to flip Y coordinates
	glm::vec2 new_texcoord = glm::vec2(std::stof(words[1]), 1.f - std::stof(words[2]));
	vt.push_back(new_texcoord);
}

void vkmesh::ObjMesh::readNormalData(const std::vector<std::string>& words)
{
	glm::vec4 new_normal = glm::vec4(std::stof(words[1]), std::stof(words[2]), std::stof(words[3]), 0.f);
	glm::vec3 transformed_normal = glm::vec3(preTransform * new_normal);
	vn.push_back(transformed_normal);
}

void vkmesh::ObjMesh::readFaceData(const std::vector<std::string>& words)
{
	size_t triangleCount = words.size() - 3;

	for (int i = 0; i < triangleCount; ++i)
	{
		readCorner(words[1]);
		readCorner(words[2 + i]);
		readCorner(words[3 + i]);
	}
}

void vkmesh::ObjMesh::readCorner(const std::string& vertex_description)
{
	if (history.contains(vertex_description))
	{
		indices.push_back(history[vertex_description]);
		return;
	}

	uint32_t index = static_cast<uint32_t>(history.size());
	history.insert({ vertex_description, index });
	indices.push_back(index);


	std::vector<std::string> v_vt_vn = split(vertex_description, "/");

	//position
	glm::vec3 pos = v[std::stol(v_vt_vn[0]) - 1];
	vertices.push_back(pos[0]);
	vertices.push_back(pos[1]);
	vertices.push_back(pos[2]);

	//color
	vertices.push_back(brushColor.r);
	vertices.push_back(brushColor.g);
	vertices.push_back(brushColor.b);

	//texcoord
	glm::vec2 texcoord = glm::vec2(0.f, 0.f);
	if (v_vt_vn.size() == 3 && v_vt_vn[1].size() > 0)
		texcoord = vt[std::stol(v_vt_vn[1]) - 1];

	vertices.push_back(texcoord[0]);
	vertices.push_back(texcoord[1]);

	//normal
	glm::vec3 normal = vn[std::stol(v_vt_vn[2]) - 1];
	vertices.push_back(normal[0]);
	vertices.push_back(normal[1]);
	vertices.push_back(normal[2]);
}