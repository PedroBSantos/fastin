#include "../include/project/project_api.h"

ProjectApi::ProjectApi(std::string projectPath, std::string projectName)
{
    this->projectPath = projectPath;
    this->projectName = projectName;
}

ProjectApi::ProjectApi(const ProjectApi& projectApi)
{
    this->projectPath = projectApi.projectPath;
    this->projectName = projectApi.projectName;
}

void ProjectApi::initialize()
{
    if (!fs::exists(this->projectPath))
        return;
    if (this->projectName.empty())
        return;
    std::string projectFullPath = this->projectPath + "/" + this->projectName;
    if (fs::exists(projectFullPath))
        return;
    fs::create_directories(projectFullPath);
    fs::create_directories(projectFullPath + "/src");
    fs::create_directories(projectFullPath + "/tests");
    std::string createSlnFileCommand = "dotnet new sln -o " + this->projectPath + "/" + this->projectName;
    std::system(createSlnFileCommand.c_str());
}

void ProjectApi::markAsInitialized()
{
    std::string projectFullPath = this->projectPath + "/" + this->projectName;
    std::string createFastInLockCommand = "touch " + projectFullPath + "/fastin.lock";
    std::system(createFastInLockCommand.c_str());
    time_t timestamp;
    time(&timestamp);
    struct tm datetime = *localtime(&timestamp);
    char currentDate[90];
    strftime(currentDate, 90, "%Y-%m-%d %H:%M:%S", &datetime);
    std::string fastInLockFileContent = "[project] = " + this->projectName + "\n[project_path] = " + projectFullPath + "\n[created_at] = " + currentDate + "\n";
    std::ofstream outfile;
    outfile.open(projectFullPath + "/fastin.lock", std::ios_base::app);
    outfile << fastInLockFileContent;
}

void ProjectApi::addLayersStructure()
{
    std::string projectFullPath = this->projectPath + "/" + this->projectName;
    std::string projectSrcPath = this->projectPath + "/" + this->projectName + "/src/";
    std::string projectTestsPath = this->projectPath + "/" + this->projectName + "/tests/";
    std::string createCoreClassLibCommand = "dotnet new classlib -o " + projectFullPath + "/src/" + this->projectName + ".Core";
    std::string createInfraClassLibCommand = "dotnet new classlib -o " + projectFullPath + "/src/" + this->projectName + ".Infra";
    std::string createWebApiCommand = "dotnet new webapi -o " + projectFullPath + "/src/" + this->projectName + ".WebApi";
    std::string createCoreTestsCommand = "dotnet new xunit -o " + projectFullPath + "/tests/" + this->projectName + ".Tests.Core";
    std::system(createCoreClassLibCommand.c_str());
    std::system(createInfraClassLibCommand.c_str());
    std::system(createWebApiCommand.c_str());
    std::system(createCoreTestsCommand.c_str());
    std::string linkSlnWithCoreClassLibCommand = "dotnet sln " + projectFullPath + " add " + projectSrcPath + this->projectName + ".Core";
    std::string linkSlnWithInfraClassLibCommand = "dotnet sln " + projectFullPath + " add " + projectSrcPath + this->projectName + ".Infra";
    std::string linkSlnWithWebApiCommand = "dotnet sln " + projectFullPath + " add " + projectSrcPath + this->projectName + ".WebApi";
    std::string linkSlnWithTestsCommand = "dotnet sln " + projectFullPath + " add " + projectTestsPath + this->projectName + ".Tests.Core";
    std::system(linkSlnWithCoreClassLibCommand.c_str());
    std::system(linkSlnWithInfraClassLibCommand.c_str());
    std::system(linkSlnWithWebApiCommand.c_str());
    std::system(linkSlnWithTestsCommand.c_str());
}

void ProjectApi::createReferenceBetweenFolders()
{
    std::string projectFullPath = this->projectPath + "/" + this->projectName;
    std::string projectCoreFolderPath = projectFullPath + "/src/" + this->projectName + ".Core";
    std::string projectInfraFolderPath = projectFullPath + "/src/" + this->projectName + ".Infra";
    std::string projectWebApiFolderPath = projectFullPath + "/src/" + this->projectName + ".WebApi";
    std::string projectTestsCoreFolderPath = projectFullPath + "/tests/" + this->projectName + ".Tests.Core";
    std::string dotnetReferenceCoreOnInfraCommand = "dotnet add " + projectInfraFolderPath + " reference " + projectCoreFolderPath;
    std::string dotnetReferenceCoreOnWebApiCommand = "dotnet add " + projectWebApiFolderPath + " reference " + projectCoreFolderPath;
    std::string dotnetReferenceInfraOnWebApiCommand = "dotnet add " + projectWebApiFolderPath + " reference " + projectInfraFolderPath;
    std::string dotnetReferenceCoreOnTestsCommand = "dotnet add " + projectTestsCoreFolderPath + " reference " + projectCoreFolderPath;
    std::system(dotnetReferenceCoreOnInfraCommand.c_str());
    std::system(dotnetReferenceCoreOnWebApiCommand.c_str());
    std::system(dotnetReferenceInfraOnWebApiCommand.c_str());
    std::system(dotnetReferenceCoreOnTestsCommand.c_str());
}
