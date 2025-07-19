#include "../include/utils/enum_utils.h"

std::string utils::convertDeployBranchEnumToString(gitlab::DeployBranch deployBranch)
{
    switch (deployBranch)
    {
    case gitlab::DEVELOP:
        return "develop";
    case gitlab::HOMOLOG:
        return "homolog";
    case gitlab::MAIN:
        return "main";
    default:
        return "";
    }
}

std::string utils::convertProjectTypeEnumToString(project::ProjectType projectType)
{
    switch (projectType)
    {
    case project::WEBAPI:
        return "webapi";
    case project::WORKER:
        return "worker";
    case project::CONSOLE:
        return "console";
    default:
        return "";
    }
}

std::string utils::convertDeployBranchEnumToEnvironment(std::string branch)
{
    if (branch == "develop")
        return "Development";
    if (branch == "homolog")
        return "Staging";
    if (branch == "main" || branch == "master")
        return "Production";
    return "";
}
