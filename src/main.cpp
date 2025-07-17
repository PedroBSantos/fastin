#include <iostream>
#include <map>
#include "../include/project/project_api.h"
#include "../include/docker/docker_api.h"
#include "../include/project/project.h"
#include "CLI/CLI.hpp"

#define FASTIN_VERSION "1.0.0"

using namespace std;

int main(int argc, char const* argv[])
{
    CLI::App app("Fastin fast inicialization");
    app.set_version_flag("--version", std::string(FASTIN_VERSION));
    app.set_help_flag("");
    app.set_help_all_flag("-h, --help");
    CLI::App* projectApi = app.add_subcommand("project", "Project Api");
    CLI::App* initProject = projectApi->add_subcommand("init-project", "Inicializa um projeto");
    std::string projectPath;
    std::string projectName;
    ProjectType projectType;
    std::string dockerImageTag;
    initProject->add_option("-p,--path", projectPath, "Path em que o projeto será inicializado")
        ->required();
    initProject->add_option("-n,--name", projectName, "Nome do projeto")
        ->required();
    std::map<std::string, ProjectType> map
    {
        {"webapi", ProjectType::WEBAPI},
        {"worker", ProjectType::WORKER},
        {"console", ProjectType::CONSOLE}
    };
    initProject->add_option("-t,--type", projectType, "Tipo do projeto [webapi, worker, console]")
        ->required()
        ->transform(CLI::CheckedTransformer(map, CLI::ignore_case));
    initProject->callback([&]() {
        ProjectApi projectApi(projectPath, projectName, projectType);
        projectApi.initialize();
        projectApi.addLayersStructure();
        projectApi.createReferenceBetweenFolders();
        projectApi.markAsInitialized();
        });
    CLI::App* dockerApi = app.add_subcommand("docker", "Api Docker");
    CLI::App* initDockerfile = dockerApi->add_subcommand("init-dockerfile", "Adiciona o dockerfile ao projeto");
    initDockerfile->callback([]() {
        Project project = Project::loadFrom("./fastin.json");
        DockerApi dockerApi(project);
        dockerApi.createDockerfile();
        });
    CLI11_PARSE(app, argc, argv);
    return 0;
}
