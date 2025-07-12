#ifndef DOCKERFILE_API_H

#define DOCKERFILE_API_H

#include <string>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <ranges>
#include <algorithm>
#include "../project/project.h"

using namespace std;
namespace fs = std::filesystem;

class DockerFileApi
{
private:
    Project project;
    std::string createBaseStage();
    std::string createBuildStage();
    std::string createPublishStage();
    std::string createFinalStage();

public:
    DockerFileApi(Project project);
    DockerFileApi(const DockerFileApi& dockerfileApi);
    virtual ~DockerFileApi() = default;
    void createDockerFile();
};

#endif