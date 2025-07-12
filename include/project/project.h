#ifndef PROJECT_H

#define PROJECT_H

#include <string>
#include "../../lib/nlohmann/json.hpp"
#include "project_api.h"

using namespace std;
using namespace nlohmann;

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

#endif