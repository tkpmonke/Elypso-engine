//Copyright(C) 2025 Lost Empire Entertainment
//This program comes with ABSOLUTELY NO WARRANTY.
//This is free software, and you are welcome to redistribute it under certain conditions.
//Read LICENSE.md for more information.
#if ENGINE_MODE

#include <filesystem>

//external
#include "glad.h"
#include "quaternion.hpp"
#include "matrix_transform.hpp"

//engine
#include "selectedobjectborder.hpp"
#include "shader.hpp"
#include "core.hpp"
#include "render.hpp"
#include "selectobject.hpp"
#include "console.hpp"
#include "transformcomponent.hpp"
#include "meshcomponent.hpp"
#include "materialcomponent.hpp"
#include "lightcomponent.hpp"

using glm::translate;
using glm::rotate;
using glm::radians;
using glm::quat;
using glm::scale;
using std::filesystem::path;
using std::filesystem::exists;

using Graphics::Shader;
using Graphics::Components::TransformComponent;
using Graphics::Components::MeshComponent;
using Graphics::Components::MaterialComponent;
using Graphics::Components::LightComponent;
using Graphics::Components::AssimpVertex;
using MeshType = Graphics::Components::MeshComponent::MeshType;
using Graphics::Shape::GameObjectManager;
using Core::Engine;
using Graphics::Render;
using Core::Select;
using Core::ConsoleManager;
using Caller = Core::ConsoleManager::Caller;
using ConsoleType = Core::ConsoleManager::Type;

namespace Graphics::Shape
{
	shared_ptr<GameObject> Border::InitializeBorder(
		const vec3& pos, 
		const vec3& rot, 
		const vec3& scale)
	{
		auto obj = make_shared<GameObject>("SelectedObjectBorder", 10000001);
		auto transform = obj->AddComponent<TransformComponent>();
		transform->SetPosition(pos);
		transform->SetRotation(rot);
		transform->SetScale(scale);
		obj->SetEnableState(false);

		float vertices[] =
		{
			//edges of the cube
			-0.5f, -0.5f, -0.5f,
			 0.5f, -0.5f, -0.5f,

			 0.5f, -0.5f, -0.5f,
			 0.5f,  0.5f, -0.5f,

			 0.5f,  0.5f, -0.5f,
			-0.5f,  0.5f, -0.5f,

			-0.5f,  0.5f, -0.5f,
			-0.5f, -0.5f, -0.5f,

			-0.5f, -0.5f,  0.5f,
			 0.5f, -0.5f,  0.5f,

			 0.5f, -0.5f,  0.5f,
			 0.5f,  0.5f,  0.5f,

			 0.5f,  0.5f,  0.5f,
			-0.5f,  0.5f,  0.5f,

			-0.5f,  0.5f,  0.5f,
			-0.5f, -0.5f,  0.5f,

			//connecting edges
			-0.5f, -0.5f, -0.5f,
			-0.5f, -0.5f,  0.5f,

			 0.5f, -0.5f, -0.5f,
			 0.5f, -0.5f,  0.5f,

			 0.5f,  0.5f, -0.5f,
			 0.5f,  0.5f,  0.5f,

			-0.5f,  0.5f, -0.5f,
			-0.5f,  0.5f,  0.5f,
		};

		GLuint vao, vbo, ebo;

		glGenVertexArrays(1, &vao);
		glGenBuffers(1, &vbo);
		glBindVertexArray(vao);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);

		glBindVertexArray(0);

		auto mesh = obj->AddComponent<MeshComponent>(
			MeshType::border, 
			vao, 
			vbo, 
			ebo);

		string vert = (path(Engine::filesPath) / "shaders" / "Basic_model.vert").string();
		string frag = (path(Engine::filesPath) / "shaders" / "Basic.frag").string();

		if (!exists(vert)
			|| !exists(frag))
		{
			Engine::CreateErrorPopup("One of the shader paths for selected object border is invalid!");
		}

		Shader borderShader = Shader::LoadShader(vert, frag);

		auto mat = obj->AddComponent<MaterialComponent>();
		mat->AddShader(vert, frag, borderShader);

		GameObjectManager::SetBorder(obj);

		ConsoleManager::WriteConsoleMessage(
			Caller::FILE,
			ConsoleType::DEBUG,
			"Successfully initialized selected object border gameobject!\n");

		return obj;
	}

	void Border::RenderBorder(
		const shared_ptr<GameObject>& obj, 
		const mat4& view, 
		const mat4& projection)
	{
		if (obj == nullptr) Engine::CreateErrorPopup("Selected object border gameobject is invalid.");

		auto mat = obj->GetComponent<MaterialComponent>();

		Shader shader = mat->GetShader();

		shader.Use();
		shader.SetMat4("projection", projection);
		shader.SetMat4("view", view);

		shader.SetVec3("color", vec3(1.0));

		mat4 model = mat4(1.0f);

		if (Select::isObjectSelected)
		{
			shader.SetFloat("transparency", 0.5f);

			auto mesh = Select::selectedObj->GetComponent<MeshComponent>();
			if (mesh->GetMeshType() == MeshComponent::MeshType::model)
			{
				//retrieve vertices and calculate bounding box
				const vector<AssimpVertex>& vertices = mesh->GetVertices();
				vec3 minBound, maxBound;
				vec3 position = Select::selectedObj->GetComponent<TransformComponent>()->GetPosition();
				vec3 initialScale = Select::selectedObj->GetComponent<TransformComponent>()->GetScale();

				//calculate the bounding box based on vertices
				Select::CalculateInteractionBoxFromVertices(vertices, minBound, maxBound, position, initialScale);

				//compute the center and scale of the bounding box
				vec3 boxCenter = (minBound + maxBound) * 0.5f;
				vec3 boxScale = maxBound - minBound;

				//translate to the center of the bounding box
				model = translate(model, boxCenter);

				//apply rotation
				quat newRot = quat(radians(Select::selectedObj->GetComponent<TransformComponent>()->GetRotation()));
				model *= mat4_cast(newRot);

				//scale based on the bounding box size with the margin included
				model = scale(model, boxScale);
			}
			else
			{
				shader.SetFloat("transparency", 0.0f);

				//move the border out of view when no object is selected
				model = translate(model, vec3(0.0f, -100.0f, 0.0f));
				model = scale(model, vec3(0.01f));
			}
		}
		else
		{
			shader.SetFloat("transparency", 0.0f);

			//move the border out of view when no object is selected
			model = translate(model, vec3(0.0f, -100.0f, 0.0f));
			model = scale(model, vec3(0.01f));
		}

		shader.SetMat4("model", model);

		auto thisMesh = obj->GetComponent<MeshComponent>();
		GLuint VAO = thisMesh->GetVAO();
		glBindVertexArray(VAO);
		glDrawArrays(GL_LINES, 0, 24);
	}
}
#endif