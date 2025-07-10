#include <iostream>
#include "../include/project/project_api.h"

using namespace std;

int main(int argc, char const *argv[])
{
    ProjectApi projectApi("/home/pedro/dev", "Initializer");
    projectApi.initialize();
    projectApi.addLayersStructure();
    projectApi.createReferenceBetweenFolders();
    projectApi.markAsInitialized();
    return 0;
}
