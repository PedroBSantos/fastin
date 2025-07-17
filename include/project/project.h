#ifndef PROJECT_H

#define PROJECT_H

#include <string>
#include "nlohmann/json.hpp"
#include "spdlog/spdlog.h"
#include <fstream>

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
        Project(std::string createdAt, std::string runtimeVersion, std::string entrypoint, std::string name, ProjectType type);

    public:
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
        static Project loadFrom(std::string fastinFile);
    };
};

#endif