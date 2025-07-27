#include "../include/project/project.h"

using namespace project;

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
    this->initialized = true;
}

Project::Project(const Project& project)
{
    this->createdAt = project.createdAt;
    this->runtimeVersion = project.runtimeVersion;
    this->entrypoint = project.entrypoint;
    this->name = project.name;
    this->type = project.type;
    this->initialized = project.initialized;
    this->ciBranchs = project.ciBranchs;
}

Project Project::loadFrom(std::string fastinFile)
{
    spdlog::info("Carregando dados do projeto a partir do arquivo " + fastinFile);
    if (!fs::exists("fastin.json"))
    {
        spdlog::error("Não foi possível encontrar o arquivo fastin.json no diretório atual");
        return Project();
    }
    std::ifstream file(fastinFile);
    nlohmann::json projectJson;
    file >> projectJson;
    file.close();
    std::string createdAt = projectJson["createdAt"];
    std::string dotnetVersion = projectJson["dotnetVersion"];
    std::string projectEntrypoint = projectJson["projectEntrypoint"];
    std::string projectName = projectJson["projectName"];
    ProjectType projectType = (ProjectType) projectJson["projectType"];
    std::vector<std::string> ciBranchs = (std::vector<std::string>) projectJson["ci_branchs"];
    Project project(createdAt, dotnetVersion, projectEntrypoint, projectName, projectType);
    for (std::string ciBranch : ciBranchs)
        project.addCiBranch(ciBranch);
    spdlog::info("Dados do projeto carregados com sucesso");
    return project;
}

void Project::saveProject(const Project& project, std::string fastinFile)
{
    spdlog::info("Salvando dados do projeto no arquivo fastin.json");
    nlohmann::json projectJson = { { "projectName", project.name },
                                   { "projectType", project.type },
                                   { "projectEntrypoint", project.entrypoint },
                                   { "dotnetVersion", project.runtimeVersion },
                                   { "createdAt", project.createdAt },
                                   { "ci_branchs", project.ciBranchs } };
    std::ofstream lockFile(fastinFile);
    lockFile << projectJson.dump(4);
    lockFile.close();
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

bool Project::isInitialized() { return this->initialized; }

bool Project::containsCiPipelineForBranch(std::string branch)
{
    return std::find(this->ciBranchs.begin(), this->ciBranchs.end(), branch) != this->ciBranchs.end();
}

void Project::addCiBranch(std::string branch)
{ 
    if (!branch.empty())
        this->ciBranchs.push_back(branch);
}