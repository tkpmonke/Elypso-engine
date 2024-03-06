//<Elypso engine>
//    Copyright(C) < 2024 > < Lost Empire Entertainment >
//
//    This program is free software : you can redistribute it and /or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License in LICENCE.md
//    and a copy of the EULA in EULA.md along with this program. 
//    If not, see < https://github.com/Lost-Empire-Entertainment/Elypso-engine >.

#include <iostream>

//external
#include "glad.h"
#include "quaternion.hpp"
#include "matrix_transform.hpp"

//engine
#include "billboard.hpp"
#include "shader.hpp"
#include "texture.hpp"
#include "core.hpp"
#include "render.hpp"
#include "selectobject.hpp"

using std::cout;
using glm::translate;
using glm::rotate;
using glm::radians;
using glm::quat;
using glm::scale;

using Graphics::Shader;
using Graphics::Texture;
using Graphics::Shape::Mesh;
using Type = Graphics::Shape::Mesh::MeshType;
using Graphics::Shape::Material;
using Graphics::Shape::GameObjectManager;
using Core::Engine;
using Graphics::Render;
using Physics::Select;

namespace Graphics::Shape
{
	shared_ptr<GameObject> Billboard::InitializeBillboard(
		const vec3& pos,
		const vec3& rot,
		const vec3& scale,
		const string& vertShader,
		const string& fragShader,
		const string& diffTexture,
		const float& shininess,
		string& name,
		unsigned int& id)
	{
		shared_ptr<Transform> transform = make_shared<Transform>(pos, rot, scale);

		float vertices[] = 
		{
			//positions             //normals            //texture coords
			-0.25f, -0.25f, -0.25f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,
			 0.25f, -0.25f, -0.25f,  0.0f,  0.0f, -1.0f,  1.0f,  0.0f,
			 0.25f,  0.25f, -0.25f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
			 0.25f,  0.25f, -0.25f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
			-0.25f,  0.25f, -0.25f,  0.0f,  0.0f, -1.0f,  0.0f,  1.0f,
			-0.25f, -0.25f, -0.25f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f
		};

		shared_ptr<Mesh> mesh = make_shared<Mesh>(Type::billboard);

		Shader billboardShader = Shader(
			Engine::enginePath + "/" + vertShader,
			Engine::enginePath + "/" + fragShader);

		GLuint vao, vbo;

		glGenVertexArrays(1, &vao);
		glGenBuffers(1, &vbo);
		glBindVertexArray(vao);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

		//position attribute
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		//normal attribute
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(1);
		//texture attribute
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
		glEnableVertexAttribArray(2);

		glBindVertexArray(0);

		shared_ptr<Material> mat = make_shared<Material>(vao, vbo);
		mat->AddShader(vertShader, fragShader, billboardShader);

		vector<shared_ptr<Component>> components;

		shared_ptr<BasicShape_Variables> basicShape = make_shared<BasicShape_Variables>(shininess);

		if (name == tempName) name = "Billboard";
		if (id == tempID) id = GameObject::nextID++;
		shared_ptr<GameObject> obj = make_shared<GameObject>(
			true,
			name,
			id,
			transform,
			mesh,
			mat,
			components,
			basicShape);

		Texture tex(Engine::enginePath);
		tex.LoadTexture(obj, diffTexture, true, GL_RGBA);

		Shader assignedShader = obj->GetMaterial()->GetShader();
		assignedShader.Use();
		assignedShader.SetInt("material.diffuse", 0);

		GameObjectManager::AddGameObject(obj);
		GameObjectManager::AddTransparentObject(obj);
		GameObjectManager::AddBillboard(obj);

		return obj;
	}

	void Billboard::RenderBillboard(const shared_ptr<GameObject>& obj, const mat4& view, const mat4& projection)
	{
		Shader shader = obj->GetMaterial()->GetShader();

		shader.Use();
		shader.SetMat4("projection", projection);
		shader.SetMat4("view", view);

		shader.SetFloat("transparency", 1.0f);
		shader.SetVec3("color", vec3(1));

		mat4 model = mat4(1.0f);

		vec3 pos = obj->GetParentBillboardHolder()->GetTransform()->GetPosition();
		obj->GetTransform()->SetPosition(pos);

		vec3 objectPos = obj->GetTransform()->GetPosition();
		vec3 cameraPos = Render::camera.GetCameraPosition();
		model = translate(model, objectPos);

		mat4 rotationMatrix = lookAt(objectPos, cameraPos, vec3(0.0f, 1.0f, 0.0f));
		rotationMatrix = inverse(rotationMatrix);
		model = rotationMatrix;

		model = scale(model, obj->GetTransform()->GetScale());

		//bind diffuse map
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, obj->GetMaterial()->GetTextureID(0));

		shader.SetMat4("model", model);
		GLuint VAO = obj->GetMaterial()->GetVAO();
		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);
	}
}