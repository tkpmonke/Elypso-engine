//Copyright(C) 2025 Lost Empire Entertainment
//This program comes with ABSOLUTELY NO WARRANTY.
//This is free software, and you are welcome to redistribute it under certain conditions.
//Read LICENSE.md for more information.
#if ENGINE_MODE
#include <vector>
#include <algorithm>
#include <filesystem>

//external
#include "glad.h"

//engine
#include "grid.hpp"
#include "shader.hpp"
#include "core.hpp"
#include "configFile.hpp"
#include "stringUtils.hpp"
#include "render.hpp"

using glm::mat4;
using std::vector;
using std::clamp;
using std::filesystem::path;

using Graphics::Shader;
using Core::Engine;
using EngineFile::ConfigFile;
using Utils::String;
using Graphics::Render;

namespace Graphics
{
	void Grid::InitializeGrid()
	{
		float lineLength = static_cast<float>(lineCount) / 2;

		for (int i = 0; i < lineCount; ++i)
		{
			float offset = 
				lineDistance * static_cast<float>(i) 
				- (lineDistance * static_cast<float>(lineCount) * 0.5f);

			// horizontal lines
			vertices[i * 4 * 3] = offset;
			vertices[i * 4 * 3 + 1] = 0.0f;
			vertices[i * 4 * 3 + 2] = lineLength;

			vertices[i * 4 * 3 + 3] = offset;
			vertices[i * 4 * 3 + 4] = 0.0f;
			vertices[i * 4 * 3 + 5] = -lineLength;

			// vertical lines
			vertices[i * 4 * 3 + 6] = lineLength;
			vertices[i * 4 * 3 + 7] = 0.0f;
			vertices[i * 4 * 3 + 8] = offset;

			vertices[i * 4 * 3 + 9] = -lineLength;
			vertices[i * 4 * 3 + 10] = 0.0f;
			vertices[i * 4 * 3 + 11] = offset;
		}

		shader = Shader::LoadShader(
			(path(Engine::filesPath) / "shaders" / "Grid.vert").string(),
			(path(Engine::filesPath) / "shaders" / "Grid.frag").string());

		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);

		glBindVertexArray(VAO);

		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}

	void Grid::RenderGrid(const mat4& view, const mat4& projection)
	{
		shader.Use();
		shader.SetMat4("projection", projection);
		shader.SetMat4("view", view);

		float transparency = stof(ConfigFile::GetValue("grid_transparency"));
		shader.SetFloat("transparency", transparency);

		float maxDistance = stof(ConfigFile::GetValue("grid_maxDistance"));
		fadeDistance = static_cast<float>(maxDistance * 0.9);
		shader.SetFloat("fadeDistance", fadeDistance);
		shader.SetFloat("maxDistance", maxDistance);
		shader.SetVec3("center", Render::camera.GetCameraPosition());

		string gridColorString = ConfigFile::GetValue("grid_color");
		vector<string> gridColorSplit = String::Split(gridColorString, ',');
		vec3 color = vec3(
			stof(gridColorSplit[0]), 
			stof(gridColorSplit[1]),
			stof(gridColorSplit[2]));
		shader.SetVec3("color", color);

		glBindVertexArray(VAO);

		glDrawArrays(GL_LINES, 0, lineCount * 4);

		glBindVertexArray(0);
	}
}
#endif