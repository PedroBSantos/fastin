#include <iostream>
#include <map>
#include "../include/project/project_api.h"
#include "../include/docker/docker_api.h"
#include "../include/project/project.h"
#include "../include/gitlab/gitlab.h"
#include "CLI/CLI.hpp"

#define FASTIN_VERSION "3.0.1"

int main(int argc, char const* argv[])
{
    CLI::App app("Fastin fast .NET CORE project's initialization");
    app.set_version_flag("--version", std::string(FASTIN_VERSION));
    app.set_help_flag("");
    app.set_help_all_flag("-h, --help");
    CLI::App* projectApi = app.add_subcommand("project", "Project Api");
    CLI::App* initProject = projectApi->add_subcommand("init-project", "Inicializa um projeto");
    std::string projectPath;
    std::string projectName;
    project::ProjectType projectType;
    gitlab::DeployBranch pipelineBranch;
    initProject->add_option("-p,--path", projectPath, "Path em que o projeto será inicializado")
        ->required();
    initProject->add_option("-n,--name", projectName, "Nome do projeto")
        ->required();
    std::map<std::string, project::ProjectType> projectTypesMap
    {
        {"webapi", project::ProjectType::WEBAPI},
        {"worker", project::ProjectType::WORKER},
        {"console", project::ProjectType::CONSOLE}
    };
    initProject->add_option("-t,--type", projectType, "Tipo do projeto [webapi, worker, console]")
        ->required()
        ->transform(CLI::CheckedTransformer(projectTypesMap, CLI::ignore_case));
    initProject->callback([&]() {
        project::ProjectApi projectApi(projectPath, projectName, projectType);
        projectApi.initialize();
        projectApi.addLayersStructure();
        projectApi.createReferenceBetweenFolders();
        projectApi.markAsInitialized();
        });
    CLI::App* dockerApi = app.add_subcommand("docker", "Api Docker");
    CLI::App* initDockerfile = dockerApi->add_subcommand("init-dockerfile", "Adiciona o dockerfile ao projeto");
    initDockerfile->callback([]() {
        project::Project project = project::Project::loadFrom("./fastin.json");
        if (!project.isInitialized())
            return;
        docker::DockerApi dockerApi(project);
        dockerApi.createDockerfile();
        });
    CLI::App* gitLabApi = app.add_subcommand("gitlab", "GitLab Api");
    CLI::App* createPipeline = gitLabApi->add_subcommand("init-pipeline", "Inicializa uma pipeline de build e deploy para uma branch");
    std::map<std::string, gitlab::DeployBranch> deployBranchsMap
    {
        {"develop", gitlab::DeployBranch::DEVELOP},
        {"homolog", gitlab::DeployBranch::HOMOLOG},
        {"main", gitlab::DeployBranch::MAIN}
    };
    createPipeline->add_option("-b,--branch", pipelineBranch, "Branch que a pipeline ira rodar [develop, homolog, main]")
        ->required()
        ->transform(CLI::CheckedTransformer(deployBranchsMap, CLI::ignore_case));
    createPipeline->callback([&]() {
        project::Project project = project::Project::loadFrom("./fastin.json");
        if (!project.isInitialized())
            return;
        gitlab::GitLab gitlabApi(project);
        gitlabApi.createPipelineForBranch(pipelineBranch);
    });
    CLI::App* createDotCiFolder = gitLabApi->add_subcommand("init-dot-ci", "Inicializa e gera o conteúdo dos arquivos da pasta .ci");
    createDotCiFolder->callback([]() {
        project::Project project = project::Project::loadFrom("./fastin.json");
        if (!project.isInitialized())
            return;
        gitlab::GitLab gitlabApi(project);
        gitlabApi.generateDotCIFolderContent();
    });
    CLI11_PARSE(app, argc, argv);
    return 0;
}
