#ifndef DOCKERFILE_API_H

#define DOCKERFILE_API_H

#include <string>
#include <cstdlib>
#include <filesystem>
#include <fstream>

using namespace std;
namespace fs = std::filesystem;

class DockerFileApi
{
public:
    DockerFileApi();
    virtual ~DockerFileApi() = default;
    void createDockerFile();
    void createBaseStage();
    void createBuildStage();
    void createPublishStage();
    void createFinalStage();
};

#endif