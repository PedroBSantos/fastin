#ifndef GITLAB_H

#define GITLAB_H

#include <algorithm>
#include <string>
#include "spdlog/spdlog.h"
#include <yaml-cpp/yaml.h>
#include "nlohmann/json.hpp"
#include "../project/project.h"

namespace fs = std::filesystem;

namespace gitlab
{
    enum DeployBranch 
    {
        DEVELOP,
        HOMOLOG,
        MAIN
    };

    class GitLab
    {
    private:
        project::Project project;
        YAML::Node createBuildJob(std::string branch);
        YAML::Node createDeployJob(std::string branch);
        void generateAwsCliJsonInputFile();
    public:
        GitLab(project::Project project);
        GitLab(const GitLab& gitLab);
        virtual ~GitLab() = default;
        void createPipelineForBranch(DeployBranch deployBranch);
    };
};
#endif