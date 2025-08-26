#pragma once

#include <iostream>
#include <vector>

#include <SFML/Graphics.hpp>

#include "ColorfulLog.h"

struct  ShaderList
{
	std::unordered_map<std::string, int> shaderMap;
	std::vector<sf::Shader> shaders;

    void addShader(const std::string& name,
        const std::filesystem::path& vertexPath,
        const std::filesystem::path& geometryPath,
        const std::filesystem::path& fragmentPath)
    {
        if (shaderMap.count(name) > 0) {
            ERROR("Shader with name '%s' already exists.", name.c_str());
            return;
        }

        sf::Shader tempShader;
        bool loaded = false;

        if (!vertexPath.empty() && !geometryPath.empty() && !fragmentPath.empty()) {
            loaded = tempShader.loadFromFile(vertexPath.string(), geometryPath.string(), fragmentPath.string());
        }
        else if (!vertexPath.empty() && !fragmentPath.empty()) {
            loaded = tempShader.loadFromFile(vertexPath.string(), fragmentPath.string());
        }
        else if (!geometryPath.empty() && !fragmentPath.empty()) {
            loaded = tempShader.loadFromFile(geometryPath.string(), fragmentPath.string());
        }
        else if (!vertexPath.empty()) {
            loaded = tempShader.loadFromFile(vertexPath.string(), sf::Shader::Type::Vertex);
        }
        else if (!geometryPath.empty()) {
            loaded = tempShader.loadFromFile(geometryPath.string(), sf::Shader::Type::Geometry);
        }
        else if (!fragmentPath.empty()) {
            loaded = tempShader.loadFromFile(fragmentPath.string(), sf::Shader::Type::Fragment);
        }

        if (!loaded) {
            ERROR("Failed to load shader: %s", name.c_str());
            return;
        }

        shaders.emplace_back(std::move(tempShader));

        shaderMap[name] = shaders.size() - 1;

        SUCCESS("Loaded shader: %s", name.c_str());
    }
        
    sf::Shader* getShader(const std::string& name) {
        auto it = shaderMap.find(name);
        if (it != shaderMap.end()) {
            return &shaders[it->second];
        }
        else {
            ERROR("Shader not found: %s", name.c_str());
            return nullptr;
        }
    }
};

struct Resourses
{
    ShaderList shaderlist;
};

class ResourceLoader {
	public :

};
