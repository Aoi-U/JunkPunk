#pragma once

#include <string>

#include <glm/glm.hpp>



class Shader
{
public:
	Shader(const char* vertPath, const char* fragPath);
	Shader(const char* vertPath, const char* fragPath, const char* geomPath);

	void use() const;
	unsigned int getID() const { return ID; }

	void setBool(const std::string& name, bool value) const;
	void setInt(const std::string& name, int value) const;
	void setFloat(const std::string& name, float value) const;
	void setMat4(const std::string& name, const glm::mat4& mat) const;
	void setVec3(const std::string& name, const float* vec) const;
	void setVec4(const std::string& name, const float* vec) const;
	

	void Delete() const;


private:
	unsigned int ID;

	std::string readFile(const char* filePath);
	bool compile(const char* vertexSource, const char* fragmentSource);
	bool compile(const char* vertexSource, const char* fragmentSource, const char* geomSource);
};