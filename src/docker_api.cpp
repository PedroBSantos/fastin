#include "../include/docker/docker_api.h"

using namespace docker;

DockerApi::DockerApi(project::Project project)
{
    this->project = project;
}

DockerApi::DockerApi(const DockerApi& dockerApi)
{
    this->project = dockerApi.project;
}

void DockerApi::createDockerfile()
{
    if (!fs::exists("fastin.json"))
    {
        spdlog::error("Não foi possível encontrar o arquivo fastin.json no diretório atual");
        return;
    }
    if (this->project.isConsole())
    {
        spdlog::error("Não é possível gerar o arquivo Dockerfile para projetos do tipo CONSOLE");
        return;
    }
    if (fs::exists("Dockerfile"))
    {
        spdlog::error("O arquivo Dockerfile já existe no diretório atual");
        return;
    }
    spdlog::info("Gerando o Dockerfile");
    std::string baseStage = this->createBaseStage();
    std::string buildStage = this->createBuildStage();
    std::string publishStage = this->createPublishStage();
    std::string finalStage = this->createFinalStage();
    std::string dockerfileContent = baseStage + buildStage + publishStage + finalStage;
    std::ofstream dockerfile("Dockerfile");
    dockerfile << dockerfileContent;
    dockerfile.close();
    spdlog::info("Geração do Dockerfile finalizada");
}

std::string DockerApi::createBaseStage()
{
    spdlog::info("Criando base stage");
    std::string runtimeVersion = this->project.getRuntimeVersion();
    std::string imageRuntime = this->project.isWebApi() ? "aspnet:" : "runtime:";
    std::string fromCommand = "FROM mcr.microsoft.com/dotnet/" + imageRuntime + runtimeVersion + " AS base\n";
    std::string workdirCommand = "WORKDIR /app\n";
    std::string exposeCommand = "EXPOSE 8080\n";
    std::string envCommand = "ENV ASPNETCORE_URLS=http://+:8080\n\n";
    std::string baseStageCommand = fromCommand + workdirCommand + exposeCommand + envCommand;
    return baseStageCommand;
}

std::string DockerApi::createBuildStage()
{
    spdlog::info("Criando build stage");
    std::string runtimeVersion = this->project.getRuntimeVersion();
    std::string projectEntrypoint = this->project.getEntrypoint();
    std::string fromCommand = "FROM mcr.microsoft.com/dotnet/sdk:" + runtimeVersion + " AS build\n";
    std::string workdirCommand1 = "WORKDIR /app/\n";
    std::string copyCommand = "COPY . .\n";
    std::string runCommand1 = "RUN dotnet restore\n";
    std::string workdirCommand2 = "WORKDIR ";
    workdirCommand2 = workdirCommand2.append("\"")
                                     .append("/app/src/")
                                     .append(projectEntrypoint)
                                     .append("\"")
                                     .append("\n");
    std::string runCommand2 = "RUN dotnet build ";
    runCommand2 = runCommand2.append("\"")
                             .append(projectEntrypoint)
                             .append(".csproj")
                             .append("\"")
                             .append(" -c Release -o /app/build\n\n");
    std::string buildStageCommand = fromCommand + workdirCommand1 + copyCommand + runCommand1 + workdirCommand2 + runCommand2;
    return buildStageCommand;
}

std::string DockerApi::createPublishStage()
{
    spdlog::info("Criando publish stage");
    std::string projectEntrypoint = this->project.getEntrypoint();
    std::string fromCommand = "FROM build AS publish\n";
    std::string runCommand = "RUN dotnet publish ";
    runCommand = runCommand.append("\"")
                           .append(projectEntrypoint)
                           .append(".csproj")
                           .append("\"")
                           .append(" -c Release -o /app/publish /p:UseAppHost=false\n\n");
    std::string publishStageCommand = fromCommand + runCommand;
    return publishStageCommand;
}

std::string DockerApi::createFinalStage()
{
    spdlog::info("Criando final stage");
    std::string projectEntrypoint = this->project.getEntrypoint();
    std::string fromCommand = "FROM base AS final\n";
    std::string workdirCommand = "WORKDIR /app\n";
    std::string copyCommand = "COPY --from=publish /app/publish .\n";
    std::string entrypointCommand = "ENTRYPOINT [\"dotnet\", ";
    entrypointCommand = entrypointCommand.append("\"")
                                         .append(projectEntrypoint)
                                         .append(".dll")
                                         .append("\"")
                                         .append("]\n");
    std::string finalStageCommand = fromCommand + workdirCommand + copyCommand + entrypointCommand;
    return finalStageCommand;
}
