#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>
#include <string>

class Shader {
public:
	Shader();
	~Shader();

	int32_t BuildFromFile(std::string& vertPath, std::string& geomPath, std::string& fragPath);

	void Use();

	void UnUse();

//private:
	GLuint mId = 0;

};


#endif // !SHADER_H
