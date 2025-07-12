#include "../include/dockerfile/dockerfile_api.h"

DockerFileApi::DockerFileApi(Project project)
{
    this->project = project;
}

DockerFileApi::DockerFileApi(const DockerFileApi& dockerfileApi)
{
    this->project = dockerfileApi.project;
}

void DockerFileApi::createDockerFile()
{
    if (!fs::exists("fastin.json"))
        return;
    std::string baseStage = this->createBaseStage();
    std::string buildStage = this->createBuildStage();
    std::string publishStage = this->createPublishStage();
    std::string finalStage = this->createFinalStage();
    std::string dockerfileContent = baseStage + buildStage + publishStage + finalStage;
    std::ofstream dockerfile("Dockerfile");
    dockerfile << dockerfileContent;
    dockerfile.close();
}

std::string DockerFileApi::createBaseStage()
{
    std::string runtimeVersion = this->project.getRuntimeVersion();
    std::string fromCommand = "FROM mcr.microsoft.com/dotnet/aspnet:" + runtimeVersion + " AS base\n";
    std::string workdirCommand = "WORKDIR /app\n";
    std::string exposeCommand = "EXPOSE 8080\n";
    std::string envCommand = "ENV ASPNETCORE_URLS=http://+:8080\n\n";
    std::string baseStageCommand = fromCommand + workdirCommand + exposeCommand + envCommand;
    return baseStageCommand;
}

std::string DockerFileApi::createBuildStage()
{
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

std::string DockerFileApi::createPublishStage()
{
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

std::string DockerFileApi::createFinalStage()
{
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