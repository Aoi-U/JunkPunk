#include "Shader.h"

Shader::Shader(const char* vertPath, const char* fragPath)
{
	std::string vertexCode = readFile(vertPath);
	std::string fragmentCode = readFile(fragPath);
	
	const char* vertexSource = vertexCode.c_str();
	const char* fragmentSource = fragmentCode.c_str();

	if (!compile(vertexSource, fragmentSource))
	{
		std::cout << "ERROR::SHADER::COMPILATION_FAILED" << std::endl;
	}
}

void Shader::use()
{
	glUseProgram(ID);
}

unsigned int Shader::getID()
{
	return ID;
}

void Shader::setBool(const std::string& name, bool value) const
{
	glUniform1i(glGetUniformLocation(ID, name.c_str()), static_cast<int>(value));
}

void Shader::setInt(const std::string& name, int value) const
{
	glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
}

void Shader::setFloat(const std::string& name, float value) const
{
	glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
}

void Shader::setMat4(const std::string& name, const float* mat) const
{
	glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, mat);
}

void Shader::setVec3(const std::string& name, const float* vec) const
{
	glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, vec);
}

void Shader::setVec4(const std::string& name, const float* vec) const
{
	glUniform4fv(glGetUniformLocation(ID, name.c_str()), 1, vec);
}

std::string Shader::readFile(const char* filePath)
{
	std::ifstream in(filePath, std::ios::binary);
	if (in)
	{
		std::string contents;
		in.seekg(0, std::ios::end);
		contents.resize(in.tellg());
		in.seekg(0, std::ios::beg);
		in.read(&contents[0], contents.size());
		in.close();
		return(contents);
	}
	throw(errno);

	/*try {
		std::stringstream sstr;
		sstr << in.rdbuf();
		return sstr.str();
	} 
	catch (std::ifstream::failure& e) {
		std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ: " << filePath << std::endl;
		return "";
	}*/
}

bool Shader::compile(const char* vertexSource, const char* fragmentSource)
{
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexSource, nullptr);
	glCompileShader(vertexShader);

	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentSource, nullptr);
	glCompileShader(fragmentShader);

	ID = glCreateProgram();
	glAttachShader(ID, vertexShader);
	glAttachShader(ID, fragmentShader);

	glLinkProgram(ID);

	int success;
	char infolog[512];
	glGetProgramiv(ID, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(ID, 512, nullptr, infolog);
		std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infolog << std::endl;
		return false;
	}

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	return true;
}

