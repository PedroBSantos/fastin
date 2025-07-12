#include <iostream>
#include "../include/project/project_api.h"
#include "../include/dockerfile/dockerfile_api.h"
#include "../include/project/project.h"

using namespace std;

int main(int argc, char const *argv[])
{
    ProjectApi projectApi("/home/pedro/dev", "Initializer", WORKER);
    projectApi.initialize();
    projectApi.addLayersStructure();
    projectApi.createReferenceBetweenFolders();
    projectApi.markAsInitialized();

    
    Project project = Project::loadFrom("/home/pedro/dev/Initializer/fastin.json");

    DockerFileApi dockerfileApi(project);
    dockerfileApi.createDockerFile();
    return 0;
}
