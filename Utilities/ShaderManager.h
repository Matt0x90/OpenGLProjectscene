/******************************************************************************
 * ShaderManager.h
 * =================
 * Provides an interface for managing OpenGL shader programs, including the
 * loading, compilation, and setting of shader attributes and uniforms.
 *
 * PURPOSE:
 * - Facilitate the loading of GLSL vertex and fragment shaders.
 * - Provide utility methods for setting uniform values in shader programs.
 * - Manage the activation and interaction with shader programs during rendering.
 *
 * FEATURES:
 * - `LoadShaders`: Loads, compiles, and links vertex and fragment shaders.
 * - Uniform setter functions for various data types:
 *    - Boolean, integer, float
 *    - Vectors (2D, 3D, 4D)
 *    - Matrices (2x2, 3x3, 4x4)
 *    - Sampler2D for texture units.
 * - Inline functions for efficient and direct interaction with the OpenGL API.
 *
 * USAGE:
 * - Create an instance of `ShaderManager`.
 * - Use `LoadShaders` to initialize shader programs with file paths.
 * - Use `use()` to activate the shader program before rendering.
 * - Set shader uniform variables with the provided utility methods.
 *
 * AUTHOR:
 * - Brian Battersby - SNHU Instructor / Computer Science
 * - Created for CS-330: Computational Graphics and Visualization, Nov. 1st, 2023.
 * - LAST REVISED: December 1, 2024
 *
 ******************************************************************************/


#pragma once

#include <GL/glew.h>        // GLEW library

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

class ShaderManager
{
public:
	unsigned int m_programID;
	
	GLuint LoadShaders(
		const char* vertex_file_path, 
		const char* fragment_file_path);

	// activate the shader
	// ------------------------------------------------------------------------
	inline void use()
	{
		glUseProgram(m_programID);
	}

	// utility uniform functions
	// ------------------------------------------------------------------------
	inline void setBoolValue(const std::string &name, bool value) const
	{
		glUniform1i(glGetUniformLocation(m_programID, name.c_str()), (int)value);
	}

	// ------------------------------------------------------------------------
	inline void setIntValue(const std::string &name, int value) const
	{
		glUniform1i(glGetUniformLocation(m_programID, name.c_str()), value);
	}

	// ------------------------------------------------------------------------
	inline void setFloatValue(const std::string &name, float value) const
	{
		glUniform1f(glGetUniformLocation(m_programID, name.c_str()), value);
	}

	// ------------------------------------------------------------------------
	inline void setVec2Value(const std::string &name, const glm::vec2 &value) const
	{
		glUniform2fv(glGetUniformLocation(m_programID, name.c_str()), 1, &value[0]);
	}

	inline void setVec2Value(const std::string &name, float x, float y) const
	{
		glUniform2f(glGetUniformLocation(m_programID, name.c_str()), x, y);
	}

	// ------------------------------------------------------------------------
	inline void setVec3Value(const std::string &name, const glm::vec3 &value) const
	{
		glUniform3fv(glGetUniformLocation(m_programID, name.c_str()), 1, &value[0]);
	}
	inline void setVec3Value(const std::string &name, float x, float y, float z) const
	{
		glUniform3f(glGetUniformLocation(m_programID, name.c_str()), x, y, z);
	}

	// ------------------------------------------------------------------------
	inline void setVec4Value(const std::string &name, const glm::vec4 &value) const
	{
		glUniform4fv(glGetUniformLocation(m_programID, name.c_str()), 1, &value[0]);
	}
	inline void setVec4Value(const std::string &name, float x, float y, float z, float w)
	{
		glUniform4f(glGetUniformLocation(m_programID, name.c_str()), x, y, z, w);
	}

	// ------------------------------------------------------------------------
	inline void setMat2Value(const std::string &name, const glm::mat2 &mat) const
	{
		glUniformMatrix2fv(glGetUniformLocation(m_programID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
	}

	// ------------------------------------------------------------------------
	inline void setMat3Value(const std::string &name, const glm::mat3 &mat) const
	{
		glUniformMatrix3fv(glGetUniformLocation(m_programID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
	}

	// ------------------------------------------------------------------------
	inline void setMat4Value(const std::string &name, const glm::mat4 &mat) const
	{
		glUniformMatrix4fv(glGetUniformLocation(m_programID, name.c_str()), 1, GL_FALSE, glm::value_ptr(mat));
	}

	// ------------------------------------------------------------------------
	inline void setSampler2DValue(const std::string& name, const int &value) const
	{
		glUniform1i(glGetUniformLocation(m_programID, name.c_str()), value);
	}
};