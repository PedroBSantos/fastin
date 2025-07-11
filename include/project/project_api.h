#ifndef PROJECT_API_H

#define PROJECT_API_H

#include <string>
#include <filesystem>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include "../nlohmann/json.hpp"

using namespace std;
using namespace nlohmann;
namespace fs = std::filesystem;

class ProjectApi
{
private:
    std::string projectPath;
    std::string projectName;
public:
    ProjectApi(std::string projectPath, std::string projectName);
    ProjectApi(const ProjectApi& projectApi);
    virtual ~ProjectApi() = default;
    void addLayersStructure();
    void initialize();
    void createReferenceBetweenFolders();
    void markAsInitialized();
};

#endif