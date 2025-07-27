#ifndef PROJECT_H

#define PROJECT_H

#include <string>
#include "nlohmann/json.hpp"
#include "spdlog/spdlog.h"
#include <fstream>
#include <vector>
#include <algorithm>

namespace fs = std::filesystem;

namespace project
{
    enum ProjectType
    {
        WEBAPI,
        WORKER,
        CONSOLE
    };

    class Project
    {
    private:
        std::string createdAt;
        std::string runtimeVersion;
        std::string entrypoint;
        std::string name;
        ProjectType type;
        bool initialized;
        std::vector<std::string> ciBranchs;

    public:
        Project(std::string createdAt, std::string runtimeVersion, std::string entrypoint, std::string name, ProjectType type);
        Project() = default;
        virtual ~Project() = default;
        Project(const Project& project);
        std::string getCreatedAt();
        std::string getRuntimeVersion();
        std::string getEntrypoint();
        std::string getName();
        std::string getType();
        bool isWorker();
        bool isConsole();
        bool isWebApi();
        bool isInitialized();
        bool containsCiPipelineForBranch(std::string branch);
        void addCiBranch(std::string branch);
        static Project loadFrom(std::string fastinFile);
        static void saveProject(const Project& project, std::string fastinFile);
    };
};

#endif