#include "../include/gitlab/gitlab.h"

using namespace gitlab;

GitLab::GitLab(project::Project project)
{
    this->project = project;
}

GitLab::GitLab(const GitLab& gitLab)
{
    this->project = gitLab.project;
}

void GitLab::createPipelineForBranch(DeployBranch deployBranch)
{
    std::string branch = "";
    switch (deployBranch)
    {
    case DEVELOP:
        branch = "develop";
        break;
    case HOMOLOG:
        branch = "homolog";
        break;
    case MAIN:
        branch = "main";
        break;
    default:
        break;
    }
    if (branch.empty())
    {
        spdlog::error("O nome da branch não pode ser vazio");
        return;
    }
    if (!fs::exists("fastin.json"))
    {
        spdlog::error("Não foi possível encontrar o arquivo fastin.json no diretório atual");
        return;
    }
    spdlog::info("Gerando pipeline de build e deploy para a branch " + branch);
    YAML::Node root;
    std::string buildStageName = "build-" + branch;
    std::string deployStageName = "deploy-" + branch;
    YAML::Node buildStage = this->createBuildJob(branch);
    YAML::Node deployStage = this->createDeployJob(branch);
    root[buildStageName] = buildStage;
    root[deployStageName] = deployStage;
    std::ofstream gitlabCiYmlFile(".gitlab-ci.yml", std::ios::app);
    gitlabCiYmlFile << root << std::endl << std::endl;
    gitlabCiYmlFile.close();
    spdlog::info("Arquivo .gitlab-ci.yml gerado com sucesso");
    spdlog::info("Pipeline de build e deploy gerada com sucesso");
}

YAML::Node GitLab::createBuildJob(std::string branch)
{
    spdlog::info("Gerando stage de build para a branch " + branch);
    YAML::Node buildStage;
    std::string buildStageName = "build-" + branch;
    buildStage["stage"].push_back("build");
    buildStage["tags"].push_back("linux");
    buildStage["only"].push_back(branch);
    YAML::Node script;
    script.push_back("true > $HOME/.aws/credentials");
    script.push_back("aws configure set aws_access_key_id $AWS_ACCESS_KEY_ID");
    script.push_back("aws configure set aws_secret_access_key $AWS_SECRET_ACCESS_KEY");
    script.push_back("aws configure set region $AWS_DEFAULT_REGION");
    script.push_back("aws ecr create-repository --repository-name $AWS_APPRUNNER_SERVICE_NAME --region $AWS_DEFAULT_REGION --output json || true");
    script.push_back("aws ecr get-login-password --region $AWS_DEFAULT_REGION | docker login --username AWS --password-stdin $AWS_ECR_URL");
    script.push_back("AWS_ECR_IMAGE_URL=$AWS_ECR_URL_DEV_HOM/$AWS_APPRUNNER_SERVICE_NAME");
    script.push_back("docker build -f \"Dockerfile\" -t $AWS_ECR_IMAGE_URL .");
    script.push_back("docker push $AWS_ECR_IMAGE_URL");
    buildStage["script"] = script;
    spdlog::info("Stage de build gerada");
    return buildStage;
}

YAML::Node GitLab::createDeployJob(std::string branch)
{
    spdlog::info("Gerando stage de deploy para a branch " + branch);
    YAML::Node deployStage;
    std::string buildStageName = "deploy-" + branch;
    deployStage["stage"].push_back("deploy");
    deployStage["tags"].push_back("linux");
    deployStage["only"].push_back(branch);
    YAML::Node beforeScript;
    beforeScript.push_back("AWS_ECR_IMAGE_URL=$AWS_ECR_URL_DEV_HOM/$AWS_APPRUNNER_SERVICE_NAME\":latest\"");
    beforeScript.push_back("GITLAB_PROJECT_URL=$CI_PROJECT_URL");
    beforeScript.push_back("GITLAB_PROJECT_BRANCH=$CI_COMMIT_BRANCH");
    beforeScript.push_back("sed -i \"s|AWS_APPRUNNER_SERVICE_NAME|$AWS_APPRUNNER_SERVICE_NAME|g\" aws-cli-input.json");
    beforeScript.push_back("sed -i \"s|AWS_IAM_ROLE|$AWS_IAM_ROLE|g\" aws-cli-input.json");
    beforeScript.push_back("sed -i \"s|AWS_ECR_IMAGE_URL|$AWS_ECR_IMAGE_URL|g\" aws-cli-input.json");
    beforeScript.push_back("sed -i \"s|AWS_APPRUNNER_VCPU|$AWS_APPRUNNER_VCPU|g\" aws-cli-input.json");
    beforeScript.push_back("sed -i \"s|AWS_APPRUNNER_MEMORY|$AWS_APPRUNNER_MEMORY|g\" aws-cli-input.json");
    beforeScript.push_back("sed -i \"s|AWS_OBSERVABILITY_ARN|$AWS_OBSERVABILITY_ARN|g\" aws-cli-input.json");
    beforeScript.push_back("sed -i \"s|AWS_APPRUNNER_VPC_CONNECTOR_ARN|$AWS_APPRUNNER_VPC_CONNECTOR_ARN|g\" aws-cli-input.json");
    beforeScript.push_back("sed -i \"s|GITLAB_PROJECT_URL|$GITLAB_PROJECT_URL|g\" aws-cli-input.json");
    beforeScript.push_back("sed -i \"s|GITLAB_PROJECT_BRANCH|$GITLAB_PROJECT_BRANCH|g\" aws-cli-input.json");
    beforeScript.push_back("sed -i \"s|APP_ENVIRONMENT|Development|g\" aws-cli-input.json");
    YAML::Node script;
    script.push_back("true > $HOME/.aws/credentials");
    script.push_back("aws configure set aws_access_key_id $AWS_ACCESS_KEY_ID");
    script.push_back("aws configure set aws_secret_access_key $AWS_SECRET_ACCESS_KEY");
    script.push_back("aws configure set region $AWS_DEFAULT_REGION");
    script.push_back("source ./.ci/commands.sh");
    script.push_back("deploy $AWS_APPRUNNER_SERVICE_NAME");
    deployStage["before_script"] = beforeScript;
    deployStage["script"] = script;
    spdlog::info("Stage de deploy gerada");
    return deployStage;
}
