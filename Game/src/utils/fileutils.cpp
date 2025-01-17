//Copyright(C) 2024 Lost Empire Entertainment
//This program comes with ABSOLUTELY NO WARRANTY.
//This is free software, and you are welcome to redistribute it under certain conditions.
//Read LICENSE.md for more information.

#include <Windows.h>
#include <iostream>
#include <memory>
#include <string>

//game
#include "fileutils.hpp"
#include "console.hpp"
#include "core.hpp"
#include "stringutils.hpp"

using std::exception;
using std::runtime_error;
using std::wstring;
using std::filesystem::exists;
using std::filesystem::copy_file;
using std::filesystem::copy;
using std::filesystem::remove;
using std::filesystem::remove_all;
using std::filesystem::copy_options;
using std::filesystem::is_directory;
using std::filesystem::is_regular_file;
using std::filesystem::create_directory;
using std::filesystem::create_directories;
using std::filesystem::rename;
using std::filesystem::recursive_directory_iterator;
using std::filesystem::directory_iterator;
using std::to_string;

using Core::ConsoleManager;
using Caller = Core::ConsoleManager::Caller;
using Type = Core::ConsoleManager::Type;
using Core::Game;
using Utils::String;

namespace Utils
{
    string File::GetOutputFromBatFile(const char* file)
    {
        char buffer[128];
        string result = "";
        string command = "\"" + string(file) + "\"";
        FILE* pipe = _popen(command.c_str(), "r");

        if (!pipe) throw runtime_error("popen() failed!");

        try
        {
            while (fgets(buffer, sizeof(buffer), pipe) != nullptr)
            {
                result += buffer;
            }
        }
        catch (...)
        {
            _pclose(pipe);
            throw;
        }

        _pclose(pipe);

        return result;
    }

    int File::RunBatFile(const string& file, bool runSeparate, BatType type)
    {
        string command = runSeparate == true
            ? "start cmd /c \"" + file + "\""
            : "\"" + file + "\"";

        if (type == BatType::compile)
        {
            string param1 = " build";

            command += param1;
        }

        return system(command.c_str());
    }

    void File::RunApplication(const string& parentFolderPath, const string& exePath, const string& commands)
    {
        wstring wParentFolderPath(parentFolderPath.begin(), parentFolderPath.end());
        wstring wExePath(exePath.begin(), exePath.end());
        wstring wCommands(commands.begin(), commands.end());

        //initialize structures for process creation
        STARTUPINFOW si;
        PROCESS_INFORMATION pi;
        ZeroMemory(&si, sizeof(si));
        ZeroMemory(&pi, sizeof(pi));
        si.cb = sizeof(si);

        // Create the new process
        if (!CreateProcessW
        (
            wExePath.c_str(),          //path to the executable
            const_cast<LPWSTR>(wCommands.c_str()), //command line arguments
            nullptr,                   //process handle not inheritable
            nullptr,                   //thread handle not inheritable
            FALSE,                     //handle inheritance
            0,                         //creation flags
            nullptr,                   //use parent's environment block
            wParentFolderPath.c_str(), //use parent's starting directory
            &si,                       //pointer to STARTUPINFO structure
            &pi                        //pointer to PROCESS_INFORMATION structure
        ))
        {
            //retrieve the error code and print a descriptive error message
            LPVOID lpMsgBuf = nullptr;
            DWORD dw = GetLastError();
            FormatMessageW(
                FORMAT_MESSAGE_ALLOCATE_BUFFER
                | FORMAT_MESSAGE_FROM_SYSTEM
                | FORMAT_MESSAGE_IGNORE_INSERTS,
                nullptr,
                dw,
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                (LPWSTR)&lpMsgBuf, 0, nullptr);
            std::wcout << L"Error: " << reinterpret_cast<LPCWSTR>(lpMsgBuf) << L"\n\n";
            LocalFree(lpMsgBuf);
        }

        // Close process and thread handles
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }

    void File::MoveOrRenameFileOrFolder(const path& sourcePath, const path& destinationPath, const bool isRenaming)
    {
        string output;

        if (!exists(sourcePath))
        {
            ConsoleManager::WriteConsoleMessage(
                Caller::FILE,
                Type::EXCEPTION,
                "Source path " + sourcePath.string() + " does not exist!\n\n");
            return;
        }
        if (exists(destinationPath))
        {
            string outputType = isRenaming ? "Cannot rename source " : "Cannot move source ";
            output = outputType + sourcePath.string() + " to destination " + destinationPath.string() + " because destination already exists!\n\n";
            ConsoleManager::WriteConsoleMessage(
                Caller::FILE,
                Type::EXCEPTION,
                output);
            return;
        }

        try
        {
            rename(sourcePath, destinationPath);

            string outputType = isRenaming ? "Renamed " : "Moved ";
            output = outputType + sourcePath.string() + " to " + destinationPath.string() + ".\n\n";
            ConsoleManager::WriteConsoleMessage(
                Caller::FILE,
                Type::DEBUG,
                output);
        }
        catch (const exception& e)
        {
            output = string(e.what()) + ".\n\n";
            ConsoleManager::WriteConsoleMessage(
                Caller::FILE,
                Type::EXCEPTION,
                output);
        }
    }

    void File::CopyFileOrFolder(const path& sourcePath, const path& destinationPath)
    {
        string output;

        if (!exists(sourcePath))
        {
            ConsoleManager::WriteConsoleMessage(
                Caller::FILE,
                Type::EXCEPTION,
                "Source path " + sourcePath.string() + " does not exist!\n\n");
            return;
        }

        try
        {
            if (is_directory(sourcePath))
            {
                copy(sourcePath, destinationPath, copy_options::recursive | copy_options::overwrite_existing);

                output = "Copied folder " + sourcePath.string() + " to " + destinationPath.string() + ".\n\n";
                ConsoleManager::WriteConsoleMessage(
                    Caller::FILE,
                    Type::DEBUG,
                    output);
            }
            else if (is_regular_file(sourcePath))
            {
                copy_file(sourcePath, destinationPath, copy_options::overwrite_existing);

                output = "Copied file " + sourcePath.string() + " to " + destinationPath.string() + ".\n\n";
                ConsoleManager::WriteConsoleMessage(
                    Caller::FILE,
                    Type::DEBUG,
                    output);
            }
        }
        catch (const exception& e)
        {
            output = string(e.what()) + ".\n\n";
            ConsoleManager::WriteConsoleMessage(
                Caller::FILE,
                Type::EXCEPTION,
                output);
        }
    }

    void File::DeleteFileOrfolder(const path& sourcePath)
    {
        string output;
        if (!exists(sourcePath))
        {
            ConsoleManager::WriteConsoleMessage(
                Caller::FILE,
                Type::EXCEPTION,
                "Cannot delete file or folder " + sourcePath.string() + " because it does not exist!\n\n");
            return;
        }

        try
        {
            if (is_regular_file(sourcePath)) remove(sourcePath);
            else if (is_directory(sourcePath))
            {
                bool hasChildDirectories = false;
                for (const auto& entry : directory_iterator(sourcePath))
                {
                    if (is_directory(entry))
                    {
                        hasChildDirectories = true;
                    }
                    else if (is_regular_file(entry))
                    {
                        remove(entry);
                    }
                }

                if (hasChildDirectories)
                {
                    for (const auto& entry : directory_iterator(sourcePath))
                    {
                        DeleteFileOrfolder(entry.path());
                    }
                    remove_all(sourcePath);
                }
                else remove_all(sourcePath);
            }

            output = "Deleted " + sourcePath.string() + ".\n\n";
            ConsoleManager::WriteConsoleMessage(
                Caller::FILE,
                Type::DEBUG,
                output);
        }
        catch (const exception& e)
        {
            output = string(e.what()) + ".\n\n";
            ConsoleManager::WriteConsoleMessage(
                Caller::FILE,
                Type::EXCEPTION,
                output);
        }
    }

    void File::CreateNewFolder(const path& folderPath)
    {
        string output;
        if (exists(folderPath))
        {
            output = folderPath.string() + " already exists!\n\n";
            ConsoleManager::WriteConsoleMessage(
                Caller::FILE,
                Type::EXCEPTION,
                output);

            return;
        }

        if (is_regular_file(folderPath))
        {
            output = folderPath.string() + " must be a folder!\n\n";
            ConsoleManager::WriteConsoleMessage(
                Caller::FILE,
                Type::EXCEPTION,
                output);

            return;
        }

        try
        {
            create_directory(folderPath);

            output = "Created new folder at " + folderPath.string() + ".\n\n";
            ConsoleManager::WriteConsoleMessage(
                Caller::FILE,
                Type::DEBUG,
                output);
        }
        catch (const exception& e)
        {
            output = string(e.what()) + ".\n\n";
            ConsoleManager::WriteConsoleMessage(
                Caller::FILE,
                Type::EXCEPTION,
                output);
        }
    }

    string File::AddIndex(const path& parentFolderPath, const string& fileName, const string& extension)
    {
        string newFilePath = parentFolderPath.string() + "/" + fileName + extension;

        if (exists(newFilePath))
        {
            newFilePath = parentFolderPath.string() + "/" + fileName + " (1)" + extension;
        }

        //try to create new file/folder with first highest available number
        if (exists(newFilePath))
        {
            int highestNumber = 1;
            for (const auto& entry : directory_iterator(parentFolderPath))
            {
                path entryPath = entry.path();
                string entryString = entry.path().string();

                if (is_directory(entryPath)
                    && entryString.find(fileName + " (") != string::npos)
                {
                    vector<string> split = String::Split(entryString, '(');
                    string cleanedNumberString = String::CharReplace(split[1], ')', '\0');
                    int number = stoi(cleanedNumberString);

                    if (number >= highestNumber) highestNumber = number + 1;
                }
            }

            newFilePath = parentFolderPath.string() + "\\" + fileName + " (" + to_string(highestNumber) + ")" + extension;
        }

        return newFilePath;
    }
}