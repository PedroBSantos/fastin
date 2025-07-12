#include "../include/project/project_api.h"

ProjectApi::ProjectApi(std::string projectPath, std::string projectName, ProjectType projectType)
{
    this->projectPath = projectPath;
    this->projectName = projectName;
    this->projectType = projectType;
}

ProjectApi::ProjectApi(const ProjectApi& projectApi)
{
    this->projectPath = projectApi.projectPath;
    this->projectName = projectApi.projectName;
    this->projectType = projectApi.projectType;
}

void ProjectApi::initialize()
{
    if (!fs::exists(this->projectPath))
    {
        spdlog::error("O diretório " + this->projectPath + " não existe");
        return;
    }
    if (this->projectName.empty())
    {
        spdlog::error("O nome do projeto não pode ser vazio");
        return;
    }
    std::string projectFullPath = this->projectPath + "/" + this->projectName;
    if (fs::exists(projectFullPath))
    {
        spdlog::error("O diretório " + projectFullPath + " já existe");
        return;
    }
    spdlog::info("Criando estrutura básica do projeto");
    fs::create_directories(projectFullPath);
    fs::create_directories(projectFullPath + "/src");
    fs::create_directories(projectFullPath + "/tests");
    std::string createSlnFileCommand = "dotnet new sln -o " + this->projectPath + "/" + this->projectName;
    std::system(createSlnFileCommand.c_str());
}

void ProjectApi::markAsInitialized()
{
    spdlog::info("Gerando o arquivo fastin.json");
    std::string projectFullPath = this->projectPath + "/" + this->projectName;
    time_t timestamp;
    time(&timestamp);
    struct tm datetime = *localtime(&timestamp);
    char currentDate[90];
    strftime(currentDate, 90, "%Y-%m-%d %H:%M:%S", &datetime);
    std::string lockFilePath = projectFullPath + "/fastin.json";
    std::string projectEntrypoint = "";
    switch (this->projectType)
    {
    case WEBAPI:
        projectEntrypoint = this->projectName + ".WebApi";
        break;
    case WORKER:
        projectEntrypoint = this->projectName + ".Worker";
        break;
    default:
        projectEntrypoint = this->projectName + ".Console";
        break;
    }
    std::string projectEntrypointCsProjPath = projectFullPath + "/src/" + projectEntrypoint + "/" + projectEntrypoint + ".csproj";
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file(projectEntrypointCsProjPath.c_str());
    pugi::xpath_node targetFrameworkNode = doc.select_node("//PropertyGroup/TargetFramework");
    std::string targetFramework = targetFrameworkNode.node().child_value();
    std::string dotnetVersion = targetFramework.substr(3, 5);

    nlohmann::json projectJson = { { "projectName", this->projectName },
                                   { "projectType", this->projectType },
                                   { "projectEntrypoint", projectEntrypoint },
                                   { "dotnetVersion", dotnetVersion },
                                   { "createdAt", currentDate } };
    std::ofstream lockFile(lockFilePath);
    lockFile << projectJson.dump(4);
    lockFile.close();
    spdlog::info("Arquivo fastin.json gerado com sucesso");
}

void ProjectApi::addLayersStructure()
{
    spdlog::info("Adicionando o padrão em camadas");
    std::string projectFullPath = this->projectPath + "/" + this->projectName;
    std::string projectSrcPath = this->projectPath + "/" + this->projectName + "/src/";
    std::string projectTestsPath = this->projectPath + "/" + this->projectName + "/tests/";
    std::string createCoreClassLibCommand = "dotnet new classlib -o " + projectFullPath + "/src/" + this->projectName + ".Core";
    std::string createInfraClassLibCommand = "dotnet new classlib -o " + projectFullPath + "/src/" + this->projectName + ".Infra";
    std::string createProjectEntrypointCommand = "";
    std::string linkSlnWithEntrypointCommand = "";
    switch (this->projectType)
    {
    case WEBAPI:
        createProjectEntrypointCommand = "dotnet new webapi -o " + projectFullPath + "/src/" + this->projectName + ".WebApi";
        linkSlnWithEntrypointCommand = "dotnet sln " + projectFullPath + " add " + projectSrcPath + this->projectName + ".WebApi";
        break;
    case WORKER:
        createProjectEntrypointCommand = "dotnet new worker -o " + projectFullPath + "/src/" + this->projectName + ".Worker";
        linkSlnWithEntrypointCommand = "dotnet sln " + projectFullPath + " add " + projectSrcPath + this->projectName + ".Worker";
        break;
    default:
        createProjectEntrypointCommand = "dotnet new console -o " + projectFullPath + "/src/" + this->projectName + ".Console";
        linkSlnWithEntrypointCommand = "dotnet sln " + projectFullPath + " add " + projectSrcPath + this->projectName + ".Console";
        break;
    }
    std::string createCoreTestsCommand = "dotnet new xunit -o " + projectFullPath + "/tests/" + this->projectName + ".Tests.Core";
    std::system(createCoreClassLibCommand.c_str());
    std::system(createInfraClassLibCommand.c_str());
    std::system(createProjectEntrypointCommand.c_str());
    std::system(createCoreTestsCommand.c_str());
    std::string linkSlnWithCoreClassLibCommand = "dotnet sln " + projectFullPath + " add " + projectSrcPath + this->projectName + ".Core";
    std::string linkSlnWithInfraClassLibCommand = "dotnet sln " + projectFullPath + " add " + projectSrcPath + this->projectName + ".Infra";
    std::string linkSlnWithTestsCommand = "dotnet sln " + projectFullPath + " add " + projectTestsPath + this->projectName + ".Tests.Core";
    std::system(linkSlnWithCoreClassLibCommand.c_str());
    std::system(linkSlnWithInfraClassLibCommand.c_str());
    std::system(linkSlnWithEntrypointCommand.c_str());
    std::system(linkSlnWithTestsCommand.c_str());
}

void ProjectApi::createReferenceBetweenFolders()
{
    spdlog::info("Realizando o link entre as camadas do projeto");
    std::string projectFullPath = this->projectPath + "/" + this->projectName;
    std::string projectCoreFolderPath = projectFullPath + "/src/" + this->projectName + ".Core";
    std::string projectInfraFolderPath = projectFullPath + "/src/" + this->projectName + ".Infra";
    std::string projectEntrypointPath = projectFullPath + "/src/" + this->projectName;
    switch (this->projectType)
    {
    case WEBAPI:
        projectEntrypointPath = projectEntrypointPath.append(".WebApi");
        break;
    case WORKER:
        projectEntrypointPath = projectEntrypointPath.append(".Worker");
        break;
    default:
        projectEntrypointPath = projectEntrypointPath.append(".Console");
        break;
    }
    std::string projectTestsCoreFolderPath = projectFullPath + "/tests/" + this->projectName + ".Tests.Core";
    std::string dotnetReferenceCoreOnInfraCommand = "dotnet add " + projectInfraFolderPath + " reference " + projectCoreFolderPath;
    std::string dotnetReferenceCoreOnProjectEntrypointCommand = "dotnet add " + projectEntrypointPath + " reference " + projectCoreFolderPath;
    std::string dotnetReferenceInfraOnProjectEntrypointCommand = "dotnet add " + projectEntrypointPath + " reference " + projectInfraFolderPath;
    std::string dotnetReferenceCoreOnTestsCommand = "dotnet add " + projectTestsCoreFolderPath + " reference " + projectCoreFolderPath;
    std::system(dotnetReferenceCoreOnInfraCommand.c_str());
    std::system(dotnetReferenceCoreOnProjectEntrypointCommand.c_str());
    std::system(dotnetReferenceInfraOnProjectEntrypointCommand.c_str());
    std::system(dotnetReferenceCoreOnTestsCommand.c_str());
}
