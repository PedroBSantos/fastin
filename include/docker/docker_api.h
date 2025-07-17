#ifndef DOCKERFILE_API_H

#define DOCKERFILE_API_H

#include <string>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include "spdlog/spdlog.h"
#include "../project/project.h"

namespace fs = std::filesystem;

namespace docker
{
    class DockerApi
    {
    private:
        project::Project project;
        std::string createBaseStage();
        std::string createBuildStage();
        std::string createPublishStage();
        std::string createFinalStage();

    public:
        DockerApi(project::Project project);
        DockerApi(const DockerApi& dockerApi);
        virtual ~DockerApi() = default;
        void createDockerfile();
    };
};

#endif