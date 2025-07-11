#include "../include/dockerfile/dockerfile_api.h"

DockerFileApi::DockerFileApi() {}

void DockerFileApi::createDockerFile()
{
    if (!fs::exists("fastin.json"))
        return;
    std::string createFileCommand = "touch Dockerfile";
    std::system(createFileCommand.c_str());
}

void DockerFileApi::createBaseStage()
{
    std::string baseStageCommand = "FROM mcr.microsoft.com/dotnet/aspnet:8.0 AS base\n WORKDIR /app \nEXPOSE 8080 \nENV ASPNETCORE_URLS=http://+:8080";
    std::ofstream lockFile("Dockerfile");
    lockFile << baseStageCommand;
    lockFile.close();
}

void DockerFileApi::createBuildStage()
{
    std::string buildStageCommand = "FROM mcr.microsoft.com/dotnet/sdk:8.0 AS build\n WORKDIR /app/ \nCOPY . . \nRUN dotnet restore \nWORKDIR \"/app/src/Sso.Api.WebApi\" \nRUN dotnet build \"Sso.Api.WebApi.csproj\" -c Release -o /app/build";
    std::ofstream lockFile("Dockerfile");
    lockFile << buildStageCommand;
    lockFile.close();
}

void DockerFileApi::createPublishStage()
{
    std::string publishStageCommand = "FROM build AS publish \nRUN dotnet publish \"Sso.Api.WebApi.csproj\" -c Release -o /app/publish /p:UseAppHost=false";
    std::ofstream lockFile("Dockerfile");
    lockFile << publishStageCommand;
    lockFile.close();
}

void DockerFileApi::createFinalStage()
{
    std::string finalStageCommand = "FROM base AS final \nWORKDIR /app \nCOPY --from=publish /app/publish . \nENTRYPOINT [\"dotnet\", \"Sso.Api.WebApi.dll\"]";
    std::ofstream lockFile("Dockerfile");
    lockFile << finalStageCommand;
    lockFile.close();
}