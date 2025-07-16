#ifndef DOCKERFILE_API_H

#define DOCKERFILE_API_H

#include <string>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include "spdlog/spdlog.h"
#include "../project/project.h"

using namespace std;
namespace fs = std::filesystem;

class DockerApi
{
private:
    Project project;
    std::string createBaseStage();
    std::string createBuildStage();
    std::string createPublishStage();
    std::string createFinalStage();

public:
    DockerApi(Project project);
    DockerApi(const DockerApi& dockerApi);
    virtual ~DockerApi() = default;
    void createDockerfile();
};

#endif