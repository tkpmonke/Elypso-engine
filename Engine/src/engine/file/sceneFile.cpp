//<Elypso engine>
//    Copyright(C) < 2024 > < Greenlaser >
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

//external
#include "magic_enum.hpp"

//engine
#include "sceneFile.hpp"
#include "gameobject.hpp"
#include "selectobject.hpp"
#include "console.hpp"
#include "core.hpp"
#include "fileUtils.hpp"

#include <vector>
#include <memory>
#include <iostream>
#include <fstream>
#include <filesystem>

using std::to_string;
using std::vector;
using std::shared_ptr;
using std::ofstream;
using std::filesystem::exists;
using std::filesystem::path;

using Graphics::Shape::GameObjectManager;
using Graphics::Shape::GameObject;
using Graphics::Shape::Mesh;
using Physics::Select;
using Core::Engine;
using Utils::File;
using Core::ConsoleManager;
using Caller = Core::ConsoleManager::Caller;
using Type = Core::ConsoleManager::Type;

namespace EngineFile
{
	void SceneFile::LoadScene(const string& filePath)
	{
		vector<shared_ptr<GameObject>> objects = GameObjectManager::GetObjects();
		if (objects.size() == 0)
		{
			ConsoleManager::WriteConsoleMessage(
				Caller::ENGINE,
				Type::INFO,
				"Successfully loaded " + filePath + "!\n");
			return;
		}

		Select::isObjectSelected = false;
		Select::selectedObj = nullptr;

		for (const auto& obj : objects)
		{
			GameObjectManager::DestroyGameObject(obj);
		}

		ConsoleManager::WriteConsoleMessage(
			Caller::ENGINE,
			Type::INFO,
			"Successfully loaded " + filePath + "!\n");
	}

	void SceneFile::SaveCurrentScene()
	{
		vector<shared_ptr<GameObject>> objects = GameObjectManager::GetObjects();
		if (objects.size() < 2)
		{
			ConsoleManager::WriteConsoleMessage(
				Caller::ENGINE,
				Type::INFO,
				"There is no content to save...\n");

			return;
		}

		string fullScenePath = Engine::filesPath + "/scenes/scene.txt";
		string tempFullScenePath = path(fullScenePath).replace_extension().string() + "_TEMP.txt";

		ofstream sceneFile(tempFullScenePath);

		if (!sceneFile.is_open())
		{
			ConsoleManager::WriteConsoleMessage(
				Caller::ENGINE,
				Type::EXCEPTION,
				"Couldn't open " + tempFullScenePath + "!\n");
			return;
		}

		for (const auto& obj : objects)
		{
			//ignore border gameobject
			if (obj->GetID() != -1)
			{
				sceneFile << "id: " << to_string(obj->GetID()) << "\n";

				sceneFile << "  name: " << obj->GetName() << "\n";

				string type = string(magic_enum::enum_name(obj->GetMesh()->GetMeshType()));
				sceneFile << "  type: " << type << "\n";

				//position
				float posX = obj->GetTransform()->GetPosition().x;
				float posY = obj->GetTransform()->GetPosition().y;
				float posZ = obj->GetTransform()->GetPosition().z;
				sceneFile << "  position: " << posX << ", " << posY << ", " << posX << "\n";

				//rotation
				float rotX = obj->GetTransform()->GetRotation().x;
				float rotY = obj->GetTransform()->GetRotation().y;
				float rotZ = obj->GetTransform()->GetRotation().z;
				sceneFile << "  rotation: " << rotX << ", " << rotY << ", " << rotZ << "\n";

				//scale
				float scaleX = obj->GetTransform()->GetScale().x;
				float scaleY = obj->GetTransform()->GetScale().y;
				float scaleZ = obj->GetTransform()->GetScale().z;
				sceneFile << "  scale: " << scaleX << ", " << scaleY << ", " << scaleZ << "\n";

				//object textures
				Mesh::MeshType meshType = obj->GetMesh()->GetMeshType();
				if (meshType == Mesh::MeshType::cube)
				{
					string diffuseTexture = obj->GetMaterial()->GetTextureName(0);
					string specularTexture = obj->GetMaterial()->GetTextureName(1);
					sceneFile << "  textures: " << diffuseTexture << ", " << specularTexture << "\n";
				}
				else if (meshType == Mesh::MeshType::billboard)
				{
					string diffuseTexture = obj->GetMaterial()->GetTextureName(0);
					sceneFile << "  textures: " << diffuseTexture << "\n";
				}

				//shaders
				string vertexShader = obj->GetMaterial()->GetShaderName(0);
				string fragmentShader = obj->GetMaterial()->GetShaderName(1);
				sceneFile << "  shaders: " << vertexShader << ", " << fragmentShader << "\n";

				//material variables
				if (meshType == Mesh::MeshType::cube)
				{
					sceneFile << "  shininess: " << obj->GetBasicShape()->GetShininess() << "\n";
				}
				else if (meshType == Mesh::MeshType::point_light)
				{
					float pointDiffuseX = obj->GetPointLight()->GetDiffuse().x;
					float pointDiffuseY = obj->GetPointLight()->GetDiffuse().y;
					float pointDiffuseZ = obj->GetPointLight()->GetDiffuse().z;
					sceneFile << "  diffuse: " << pointDiffuseX << ", " << pointDiffuseY << ", " << pointDiffuseZ << "\n";

					sceneFile << "  intensity: " << obj->GetPointLight()->GetIntensity() << "\n";

					sceneFile << "  distance: " << obj->GetPointLight()->GetDistance() << "\n";
				}
				else if (meshType == Mesh::MeshType::spot_light)
				{
					float spotDiffuseX = obj->GetSpotLight()->GetDiffuse().x;
					float spotDiffuseY = obj->GetSpotLight()->GetDiffuse().y;
					float spotDiffuseZ = obj->GetSpotLight()->GetDiffuse().z;
					sceneFile << "  diffuse: " << spotDiffuseX << ", " << spotDiffuseY << ", " << spotDiffuseZ << "\n";

					sceneFile << "  intensity: " << obj->GetSpotLight()->GetIntensity() << "\n";

					sceneFile << "  distance: " << obj->GetSpotLight()->GetDistance() << "\n";

					sceneFile << "  inner angle: " << obj->GetSpotLight()->GetInnerAngle() << "\n";

					sceneFile << "  outer angle: " << obj->GetSpotLight()->GetOuterAngle() << "\n";
				}
			}
		}

		sceneFile.close();

		path fsPath = fullScenePath;
		File::DeleteFileOrfolder(fsPath);

		path tempFSPath = tempFullScenePath;
		File::MoveOrRenameFileOrFolder(tempFSPath, fsPath, true);

		ConsoleManager::WriteConsoleMessage(
			Caller::ENGINE,
			Type::INFO,
			"Successfully saved " + fullScenePath + "!\n");
	}
}