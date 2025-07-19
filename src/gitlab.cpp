#include "../include/gitlab/gitlab.h"
#include "../include/utils/enum_utils.h"

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
    std::string branch = utils::convertDeployBranchEnumToString(deployBranch);
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
    std::transform(branch.begin(), branch.end(), branch.begin(), ::toupper);
    script.push_back("true > $HOME/.aws/credentials");
    script.push_back("aws configure set aws_access_key_id $AWS_ACCESS_KEY_ID_" + branch);
    script.push_back("aws configure set aws_secret_access_key $AWS_SECRET_ACCESS_KEY_" + branch);
    script.push_back("aws configure set region $AWS_DEFAULT_REGION_" + branch);
    script.push_back("aws ecr create-repository --repository-name $AWS_APPRUNNER_SERVICE_NAME_" + branch 
                   + " --region $AWS_DEFAULT_REGION_" + branch
                   + " --output json || true");
    script.push_back("aws ecr get-login-password --region $AWS_DEFAULT_REGION_" + branch
                   + " | docker login --username AWS --password-stdin $AWS_ECR_URL_" + branch);
    script.push_back("AWS_ECR_IMAGE_URL=$AWS_ECR_URL_" + branch + "/$AWS_APPRUNNER_SERVICE_NAME_" + branch);
    script.push_back("docker build -f \"Dockerfile\" -t $AWS_ECR_IMAGE_URL .");
    script.push_back("docker push $AWS_ECR_IMAGE_URL");
    buildStage["script"] = script;
    spdlog::info("Stage de build gerada");
    return buildStage;
}

YAML::Node GitLab::createDeployJob(std::string branch)
{
    std::string environment = utils::convertDeployBranchEnumToEnvironment(branch);
    spdlog::info("Gerando stage de deploy para a branch " + branch);
    YAML::Node deployStage;
    std::string buildStageName = "deploy-" + branch;
    deployStage["stage"].push_back("deploy");
    deployStage["tags"].push_back("linux");
    deployStage["only"].push_back(branch);
    YAML::Node beforeScript;
    std::transform(branch.begin(), branch.end(), branch.begin(), ::toupper);
    beforeScript.push_back("AWS_ECR_IMAGE_URL=$AWS_ECR_URL_" + branch + "/$AWS_APPRUNNER_SERVICE_NAME_" + branch + "\":latest\"");
    beforeScript.push_back("GITLAB_PROJECT_URL=$CI_PROJECT_URL");
    beforeScript.push_back("GITLAB_PROJECT_BRANCH=$CI_COMMIT_BRANCH");
    std::string sedCommand = "sed -i ";
    sedCommand.append("\"")
              .append("s|AWS_APPRUNNER_SERVICE_NAME|")
              .append("$AWS_APPRUNNER_SERVICE_NAME_")
              .append(branch)
              .append("|g")
              .append("\"")
              .append(" aws-cli-input.json");
    beforeScript.push_back(sedCommand);
    sedCommand = "sed -i ";
    sedCommand.append("\"")
              .append("s|AWS_IAM_ROLE|")
              .append("$AWS_IAM_ROLE_")
              .append(branch)
              .append("|g")
              .append("\"")
              .append(" aws-cli-input.json");
    beforeScript.push_back(sedCommand);
    sedCommand.clear();
    sedCommand = "sed -i ";
    sedCommand.append("\"")
              .append("s|AWS_APPRUNNER_VCPU|")
              .append("$AWS_APPRUNNER_VCPU_")
              .append(branch)
              .append("|g")
              .append("\"")
              .append(" aws-cli-input.json");
    beforeScript.push_back("sed -i \"s|AWS_ECR_IMAGE_URL|$AWS_ECR_IMAGE_URL|g\" aws-cli-input.json");
    sedCommand.clear();
    sedCommand = "sed -i ";
    sedCommand.append("\"")
              .append("s|AWS_APPRUNNER_MEMORY|")
              .append("$AWS_APPRUNNER_MEMORY_")
              .append(branch)
              .append("|g")
              .append("\"")
              .append(" aws-cli-input.json");
    beforeScript.push_back(sedCommand);
    sedCommand.clear();
    sedCommand = "sed -i ";
    sedCommand.append("\"")
              .append("s|AWS_OBSERVABILITY_ARN|")
              .append("$AWS_OBSERVABILITY_ARN_")
              .append(branch)
              .append("|g")
              .append("\"")
              .append(" aws-cli-input.json");
    beforeScript.push_back(sedCommand);
    sedCommand.clear();
    sedCommand = "sed -i ";
    sedCommand.append("\"")
              .append("s|AWS_APPRUNNER_VPC_CONNECTOR_ARN|")
              .append("$AWS_APPRUNNER_VPC_CONNECTOR_ARN_")
              .append(branch)
              .append("|g")
              .append("\"")
              .append(" aws-cli-input.json");
    beforeScript.push_back(sedCommand);
    beforeScript.push_back("sed -i \"s|GITLAB_PROJECT_URL|$GITLAB_PROJECT_URL|g\" aws-cli-input.json");
    beforeScript.push_back("sed -i \"s|GITLAB_PROJECT_BRANCH|$GITLAB_PROJECT_BRANCH|g\" aws-cli-input.json");
    sedCommand.clear();
    sedCommand = "sed -i ";
    sedCommand.append("\"")
              .append("s|APP_ENVIRONMENT|")
              .append(environment)
              .append("|g")
              .append("\"")
              .append(" aws-cli-input.json");
    beforeScript.push_back(sedCommand);
    YAML::Node script;
    script.push_back("true > $HOME/.aws/credentials");
    script.push_back("aws configure set aws_access_key_id $AWS_ACCESS_KEY_ID_" + branch);
    script.push_back("aws configure set aws_secret_access_key $AWS_SECRET_ACCESS_KEY_" + branch);
    script.push_back("aws configure set region $AWS_DEFAULT_REGION_" + branch);
    script.push_back("source ./.ci/commands.sh");
    script.push_back("deploy $AWS_APPRUNNER_SERVICE_NAME_" + branch);
    deployStage["before_script"] = beforeScript;
    deployStage["script"] = script;
    spdlog::info("Stage de deploy gerada");
    return deployStage;
}
