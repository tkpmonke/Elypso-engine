//Copyright(C) 2024 Lost Empire Entertainment
//This program comes with ABSOLUTELY NO WARRANTY.
//This is free software, and you are welcome to redistribute it under certain conditions.
//Read LICENSE.md for more information.
#if ENGINE_MODE
#include <type_ptr.hpp>
#include <cmath>
#include <filesystem>

//external
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "imgui_internal.h"
#include "magic_enum.hpp"
#include "glm.hpp"

//engine
#include "gui_settings.hpp"
#include "gui_engine.hpp"
#include "gui_console.hpp"
#include "render.hpp"
#include "input.hpp"
#include "console.hpp"
#include "grid.hpp"
#include "sceneFile.hpp"
#include "core.hpp"
#include "fileexplorer.hpp"
#include "configFile.hpp"
#include "stringUtils.hpp"
#include "gui_projectitemslist.hpp"
#include "skybox.hpp"

using std::to_string;
using std::stof;
using std::round;
using glm::value_ptr;
using std::exception;
using std::filesystem::path;

using EngineFile::SceneFile;
using Graphics::GUI::GUIConsole;
using Graphics::Render;
using Graphics::Grid;
using Core::Input;
using Core::ConsoleManager;
using Caller = Core::ConsoleManager::Caller;
using Type = Core::ConsoleManager::Type;
using Core::Engine;
using EngineFile::FileExplorer;
using EngineFile::ConfigFile;
using Utils::String;
using Graphics::Shape::Skybox;

namespace Graphics::GUI
{
	void GUISettings::RenderSettings()
	{
		ImVec2 windowSize = ImVec2(600.0f, 600.0f);
		ImGui::SetNextWindowSize(windowSize, ImGuiCond_Appearing);

		ImVec2 windowPos = EngineGUI::CenterWindow(windowSize);
		ImGui::SetNextWindowPos(ImVec2(windowPos), ImGuiCond_Appearing);

		ImGuiWindowFlags windowFlags =
			ImGuiWindowFlags_NoCollapse
			| ImGuiWindowFlags_NoDocking
			| ImGuiWindowFlags_NoSavedSettings;

		if (renderSettings
			&& ImGui::Begin("Settings", NULL, windowFlags))
		{
			ImGui::SameLine();
			ImGui::SetCursorPosX(ImGui::GetWindowWidth() - 40);
			if (ImGui::Button("X"))
			{
				renderSettings = false;
			}

			if (ImGui::BeginTabBar("Settings"))
			{
				if (ImGui::BeginTabItem("Input"))
				{
					InputSettings();
					ImGui::EndTabItem();
				}
				if (ImGui::BeginTabItem("GUI style"))
				{
					GUIStyleSettings();
					ImGui::EndTabItem();
				}
				if (ImGui::BeginTabItem("Graphics"))
				{
					GraphicsSettings();
					ImGui::EndTabItem();
				}
				if (ImGui::BeginTabItem("Other"))
				{
					OtherSettings();
					ImGui::EndTabItem();
				}
				ImGui::EndTabBar();
			}

			ImGui::End();
		}
	}
	void GUISettings::InputSettings()
	{
		ImGui::Text("Forwards: W");
		ImGui::Text("Backwards: S");
		ImGui::Text("Left: A");
		ImGui::Text("Right: D");
		ImGui::Text("Up: Space");
		ImGui::Text("Down: Left Control");
		ImGui::Text("Sprint: Left Shift");
		ImGui::Text("Enable camera movement: Hold right mouse button");
		ImGui::Text("Compile game: Ctrl + B");
		ImGui::Text("Run compiled game: Ctrl + R");
		ImGui::Text("Select GameObject: Left Mouse Button");
		ImGui::Text("Delete selected GameObject or node: Delete");
		ImGui::Text("Copy selected object: Ctrl + C");
		ImGui::Text("Paste copied object: Ctrl + V");
		ImGui::Text("Switch to X axis: X");
		ImGui::Text("Switch to Y axis: Y");
		ImGui::Text("Switch to Z axis: Z");
		ImGui::Text("Move selected object:");
		ImGui::Text("Press W and scroll up to increase or down to decrease value");
		ImGui::Text("Rotate selected object:");
		ImGui::Text("Hold E and scroll up to increase or down to decrease value");
		ImGui::Text("Scale selected object:");
		ImGui::Text("Hold R and scroll up to increase or down to decrease value");
	}

	void GUISettings::GUIStyleSettings()
	{
		ImGuiStyle& style = ImGui::GetStyle();

		float fontScale = stof(ConfigFile::GetValue("gui_fontScale"));
		ImGui::Text("Font scale");

		if (ImGui::DragFloat("##fontScale", &fontScale, 0.01f, 0.1f, 2.0f))
		{
			if (fontScale < 1.0f) fontScale = 1.0f;
			if (fontScale > 2.0f) fontScale = 2.0f;

			ConfigFile::SetValue("gui_fontScale", to_string(fontScale));
			if (!SceneFile::unsavedChanges) Render::SetWindowNameAsUnsaved(true);
		}
		ImGui::SameLine();
		if (ImGui::Button("Reset##fontScale"))
		{
			ConfigFile::SetValue("gui_fontScale", "1.5");
			if (!SceneFile::unsavedChanges) Render::SetWindowNameAsUnsaved(true);
		}
	}

	void GUISettings::GraphicsSettings()
	{
		if (skyboxTextures.empty())
		{
			string texturesFolder = Engine::filesPath + "\\textures";
			skyboxTextures["right"] = texturesFolder + "\\skybox_default.png";
			skyboxTextures["left"] = texturesFolder + "\\skybox_default.png";
			skyboxTextures["top"] = texturesFolder + "\\skybox_default.png";
			skyboxTextures["bottom"] = texturesFolder + "\\skybox_default.png";
			skyboxTextures["front"] = texturesFolder + "\\skybox_default.png";
			skyboxTextures["back"] = texturesFolder + "\\skybox_default.png";

			vector<string> appliedSkyboxTextures
			{
				skyboxTextures["right"],
				skyboxTextures["left"],
				skyboxTextures["top"],
				skyboxTextures["bottom"],
				skyboxTextures["front"],
				skyboxTextures["back"]
			};
		}

		ImGui::Text("Skybox textures");

		ImGui::Spacing();

		if (ImGui::Button("Right"))
		{
			GUIProjectItemsList::type = GUIProjectItemsList::Type::SkyboxTexture_right;
			GUIProjectItemsList::selectSkyboxTexture = true;
			GUIProjectItemsList::renderProjectItemsList = true;
		}
		ImGui::SameLine();
		string rightName = path(skyboxTextures["right"]).filename().string();
		ImGui::Text(rightName.c_str());

		if (ImGui::Button("Left"))
		{
			GUIProjectItemsList::type = GUIProjectItemsList::Type::SkyboxTexture_left;
			GUIProjectItemsList::selectSkyboxTexture = true;
			GUIProjectItemsList::renderProjectItemsList = true;
		}
		ImGui::SameLine();
		string leftName = path(skyboxTextures["left"]).filename().string();
		ImGui::Text(leftName.c_str());

		if (ImGui::Button("Top"))
		{
			GUIProjectItemsList::type = GUIProjectItemsList::Type::SkyboxTexture_top;
			GUIProjectItemsList::selectSkyboxTexture = true;
			GUIProjectItemsList::renderProjectItemsList = true;
		}
		ImGui::SameLine();
		string topName = path(skyboxTextures["top"]).filename().string();
		ImGui::Text(topName.c_str());

		if (ImGui::Button("Bottom"))
		{
			GUIProjectItemsList::type = GUIProjectItemsList::Type::SkyboxTexture_bottom;
			GUIProjectItemsList::selectSkyboxTexture = true;
			GUIProjectItemsList::renderProjectItemsList = true;
		}
		ImGui::SameLine();
		string bottomName = path(skyboxTextures["bottom"]).filename().string();
		ImGui::Text(bottomName.c_str());

		if (ImGui::Button("Front"))
		{
			GUIProjectItemsList::type = GUIProjectItemsList::Type::SkyboxTexture_front;
			GUIProjectItemsList::selectSkyboxTexture = true;
			GUIProjectItemsList::renderProjectItemsList = true;
		}
		ImGui::SameLine();
		string frontName = path(skyboxTextures["front"]).filename().string();
		ImGui::Text(frontName.c_str());

		if (ImGui::Button("Back"))
		{
			GUIProjectItemsList::type = GUIProjectItemsList::Type::SkyboxTexture_back;
			GUIProjectItemsList::selectSkyboxTexture = true;
			GUIProjectItemsList::renderProjectItemsList = true;
		}
		ImGui::SameLine();
		string backName = path(skyboxTextures["back"]).filename().string();
		ImGui::Text(backName.c_str());

		if (ImGui::Button("Apply"))
		{
			vector<string> appliedSkyboxTextures
			{
				skyboxTextures["right"],
				skyboxTextures["left"],
				skyboxTextures["top"],
				skyboxTextures["bottom"],
				skyboxTextures["front"],
				skyboxTextures["back"]
			};
			Skybox::AssignSkyboxTextures(appliedSkyboxTextures, false);

			ConsoleManager::WriteConsoleMessage(
				Caller::INPUT,
				Type::INFO,
				"Successfully applied new skybox textures!");

			if (!SceneFile::unsavedChanges) Render::SetWindowNameAsUnsaved(true);
		}

		ImGui::SameLine();
		if (ImGui::Button("Reset"))
		{
			string texturesFolder = Engine::filesPath + "\\textures";
			skyboxTextures["right"] = texturesFolder + "\\skybox_default.png";
			skyboxTextures["left"] = texturesFolder + "\\skybox_default.png";
			skyboxTextures["top"] = texturesFolder + "\\skybox_default.png";
			skyboxTextures["bottom"] = texturesFolder + "\\skybox_default.png";
			skyboxTextures["front"] = texturesFolder + "\\skybox_default.png";
			skyboxTextures["back"] = texturesFolder + "\\skybox_default.png";

			vector<string> appliedSkyboxTextures
			{
				skyboxTextures["right"],
				skyboxTextures["left"],
				skyboxTextures["top"],
				skyboxTextures["bottom"],
				skyboxTextures["front"],
				skyboxTextures["back"]
			};
			Skybox::AssignSkyboxTextures(appliedSkyboxTextures, false);

			ConsoleManager::WriteConsoleMessage(
				Caller::INPUT,
				Type::INFO,
				"Successfully reset skybox textures to default values!");

			if (!SceneFile::unsavedChanges) Render::SetWindowNameAsUnsaved(true);
		}
	}

	void GUISettings::OtherSettings()
	{
		ImGui::Text("Set game name");
		if (gameName == "") gameName = "Game";
		strcpy_s(gameNameChar, bufferSize, gameName.c_str());
		if (ImGui::InputText("##objName", gameNameChar, bufferSize))
		{
			gameName = gameNameChar;
		}
		if (ImGui::IsItemHovered())
		{
			ImGui::SetTooltip(Engine::gameExePath.c_str());
		}

		ImGui::SameLine();

		if (ImGui::Button("Apply"))
		{
			bool canApply = true;

			for (char c : gameName)
			{
				if (!String::IsValidSymbolInPath(c))
				{
					ConsoleManager::WriteConsoleMessage(
						Caller::INPUT,
						Type::EXCEPTION,
						"Invalid character detected in game name! Please only use english letters, roman numbers and dash, dot or underscore symbol!");

					gameName = "Game";

					canApply = false;

					break;
				}
			}

			if (canApply)
			{
				string finalPath = Engine::gamePath + "/build/Release/" + gameName + ".exe";
				Engine::gameExePath = finalPath;

				ConsoleManager::WriteConsoleMessage(
					Caller::INPUT,
					Type::INFO,
					"Successfully set game name to '" + gameName + "' and game path as '" + Engine::gameExePath + "'!\n");

				if (!SceneFile::unsavedChanges) Render::SetWindowNameAsUnsaved(true);
			}
		}

		ImGui::Separator();

		ImGui::Text("Set first scene");
		if (ImGui::Button("Select start scene"))
		{
			GUIProjectItemsList::type = GUIProjectItemsList::Type::Scene;
			GUIProjectItemsList::selectStartScene = true;
			GUIProjectItemsList::renderProjectItemsList = true;
		}
	}
}
#endif