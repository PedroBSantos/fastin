#include <iostream>
#include "../include/project/project_api.h"
#include "../include/dockerfile/dockerfile_api.h"

using namespace std;

int main(int argc, char const *argv[])
{
    ProjectApi projectApi("/home/pedro/dev", "Initializer", CONSOLE);
    projectApi.initialize();
    projectApi.addLayersStructure();
    projectApi.createReferenceBetweenFolders();
    projectApi.markAsInitialized();

    DockerFileApi dockerfileApi;
    dockerfileApi.createDockerFile();
    dockerfileApi.createBaseStage();
    dockerfileApi.createBuildStage();
    dockerfileApi.createPublishStage();
    dockerfileApi.createFinalStage();
    return 0;
}
