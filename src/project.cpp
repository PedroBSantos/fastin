#include "../include/project/project.h"

Project::Project(
    std::string createdAt, 
    std::string runtimeVersion, 
    std::string entrypoint, 
    std::string name, 
    ProjectType type)
{
    this->createdAt = createdAt;
    this->runtimeVersion = runtimeVersion;
    this->entrypoint = entrypoint;
    this->name = name;
    this->type = type;
}

Project::Project(const Project& project)
{
    this->createdAt = project.createdAt;
    this->runtimeVersion = project.runtimeVersion;
    this->entrypoint = project.entrypoint;
    this->name = project.name;
    this->type = project.type;
}

Project Project::loadFrom(std::string fastinFile)
{
    std::ifstream file(fastinFile);
    nlohmann::json projectJson;
    file >> projectJson;
    std::string createdAt = projectJson["createdAt"];
    std::string dotnetVersion = projectJson["dotnetVersion"];
    std::string projectEntrypoint = projectJson["projectEntrypoint"];
    std::string projectName = projectJson["projectName"];
    ProjectType projectType = (ProjectType) projectJson["projectType"];
    Project project(createdAt, dotnetVersion, projectEntrypoint, projectName, projectType);
    return project;
}

std::string Project::getCreatedAt() { return this->createdAt; }

std::string Project::getRuntimeVersion() { return this->runtimeVersion; }

std::string Project::getEntrypoint() { return this->entrypoint; }

std::string Project::getName() { return this->name; }

std::string Project::getType()
{
    switch (this->type)
    {
    case WEBAPI:
        return "WEBAPI";
    case WORKER:
        return "WORKER";
    default:
        return "CONSOLE";
    }
}

bool Project::isConsole()
{
    std::string projectType = this->getType();
    return "CONSOLE" == projectType;
}

bool Project::isWorker()
{
    std::string projectType = this->getType();
    return "WORKER" == projectType;
}

bool Project::isWebApi()
{
    std::string projectType = this->getType();
    return "WEBAPI" == projectType;
}