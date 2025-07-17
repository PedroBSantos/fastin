#ifndef PROJECT_API_H

#define PROJECT_API_H

#include <string>
#include <filesystem>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include "spdlog/spdlog.h"
#include "nlohmann/json.hpp"
#include "pugixml.hpp"
#include "project.h"

namespace fs = std::filesystem;

namespace project
{
    class ProjectApi
    {
    private:
        std::string projectPath;
        std::string projectName;
        ProjectType projectType;
    public:
        ProjectApi(std::string projectPath, std::string projectName, ProjectType projectType);
        ProjectApi(const ProjectApi& projectApi);
        virtual ~ProjectApi() = default;
        void addLayersStructure();
        void initialize();
        void createReferenceBetweenFolders();
        void markAsInitialized();
    };
};

#endif