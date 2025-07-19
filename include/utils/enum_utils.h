#ifndef ENUM_UTILS_H

#define ENUM_UTILS_H

#include <string>
#include "../gitlab/gitlab.h"
#include "../project/project.h"

namespace utils 
{
    std::string convertDeployBranchEnumToString(gitlab::DeployBranch deployBranch);
    std::string convertProjectTypeEnumToString(project::ProjectType projectType);
    std::string convertDeployBranchEnumToEnvironment(std::string branch);
};

#endif