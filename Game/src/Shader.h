#pragma once

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

#include <glad/glad.h>

class Shader
{
public:
	Shader(const char* vertPath, const char* fragPath);

	void use();
	unsigned int getID();

	void setBool(const std::string& name, bool value) const;
	void setInt(const std::string& name, int value) const;
	void setFloat(const std::string& name, float value) const;
	void setMat4(const std::string& name, const float* mat) const;
	void setVec3(const std::string& name, const float* vec) const;
	void setVec4(const std::string& name, const float* vec) const;


private:
	unsigned int ID;

	std::string readFile(const char* filePath);
	bool compile(const char* vertPath, const char* fragPath);
};