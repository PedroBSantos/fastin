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
    if (this->project.isConsole())
    {
        spdlog::error("Não é possível criar pipeline de CI para projetos do tipo CONSOLE");
        return;
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
    if (this->project.containsCiPipelineForBranch(branch))
    {
        spdlog::error("Já existe uma pipeline de CI configurada para a branch " + branch);
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
    gitlabCiYmlFile << root << std::endl;
    gitlabCiYmlFile.close();
    spdlog::info("Arquivo .gitlab-ci.yml gerado com sucesso");
    this->generateAwsCliJsonInputFile();
    this->project.addCiBranch(branch);
    project::Project::saveProject(this->project, "./fastin.json");
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
    script.push_back("deploy $AWS_APPRUNNER_SERVICE_NAME_" + branch + " $AWS_DEFAULT_REGION_" + branch);
    deployStage["before_script"] = beforeScript;
    deployStage["script"] = script;
    spdlog::info("Stage de deploy gerada");
    return deployStage;
}

void GitLab::generateAwsCliJsonInputFile()
{
    spdlog::info("Gerando o arquivo aws-cli-input.json");
    if (this->project.isConsole())
    {
        spdlog::error("Não é possível gerar o arquivo aws-cli-input.json para projetos do tipo CONSOLE");
        return;
    }
    if (fs::exists("aws-cli-input.json"))
    {
        spdlog::info("O arquivo aws-cli-input.json já existe no diretório atual");
        return;
    }
    std::string awsCliInputJsonFilePath = "aws-cli-input.json";
    nlohmann::json awsCliInputJson;
    awsCliInputJson["ServiceName"] = "AWS_APPRUNNER_SERVICE_NAME";
    awsCliInputJson["SourceConfiguration"] = {
        { "AuthenticationConfiguration", {
            { "AccessRoleArn", "AWS_IAM_ROLE" }
        }},
        { "AutoDeploymentsEnabled", false },
        { "ImageRepository", {
            { "ImageIdentifier", "AWS_ECR_IMAGE_URL" },
            { "ImageConfiguration", {
                { "Port", "8080" },
                { "RuntimeEnvironmentVariables", {
                    { "ASPNETCORE_ENVIRONMENT", "APP_ENVIRONMENT" }
                }}
            }},
            { "ImageRepositoryType", "ECR" }
        }}
    };
    awsCliInputJson["InstanceConfiguration"] = {
        { "Cpu", "AWS_APPRUNNER_VCPU" },
        { "Memory", "AWS_APPRUNNER_MEMORY" }
    };
    awsCliInputJson["ObservabilityConfiguration"] = {
        { "ObservabilityEnabled", true },
        { "ObservabilityConfigurationArn", "AWS_OBSERVABILITY_ARN" }
    };
    awsCliInputJson["NetworkConfiguration"] = {
        { "EgressConfiguration", {
            { "EgressType", "VPC" },
            { "VpcConnectorArn", "AWS_APPRUNNER_VPC_CONNECTOR_ARN" }
        }},
        { "IngressConfiguration", {
            { "IsPubliclyAccessible", true }
        }},
        { "IpAddressType", "IPV4" }
    };
    awsCliInputJson["HealthCheckConfiguration"] = {
        { "HealthyThreshold", 1 },
        { "Interval", 10 },
        { "Protocol", "TCP" },
        { "Timeout", 5 },
        { "UnhealthyThreshold", 5 }
    };
    awsCliInputJson["Tags"] = nlohmann::json::array({
        {
            { "Key", "GitLabProjectUrl" },
            { "Value", "GITLAB_PROJECT_URL" }
        },
        {
            { "Key", "GitLabProjectBranch" },
            { "Value", "GITLAB_PROJECT_BRANCH" }
        }
    });
    std::ofstream lockFile(awsCliInputJsonFilePath);
    lockFile << awsCliInputJson.dump(4);
    lockFile.close();
    spdlog::info("Arquivo aws-cli-input.json gerado com sucesso");
}

void GitLab::generateDotCIFolderContent()
{
    spdlog::info("Gerando arquivos .sh auxiliares cd CI/CD na pasta .ci");
    if (!fs::exists("fastin.json"))
    {
        spdlog::error("Não foi possível encontrar o arquivo fastin.json no diretório atual");
        return;
    }
    if (this->project.isConsole())
    {
        spdlog::error("Não é possível gerar a pasta .ci e os arquivos commands.sh e library.sh para projetos do tipo CONSOLE");
        return;
    }
    if (!fs::exists(".ci"))
        fs::create_directory(".ci");
    if (!fs::exists(".ci/commands.sh"))
    {
        std::ofstream commandsFile(".ci/commands.sh");
        std::string commandsFileContent = "source \"$PWD/.ci/library.sh\"\n\ndeploy() {\n  local result\n  local service_name\n  local service_arn\n  local service_url\n  service_name=$1\n  aws_region=$2\n  service_arn=$(contains_apprunner_service \"$service_name\" \"$aws_region\")\n  result=$?\n  if [ \"$service_arn\" != \"false\" ]; then\n      echo Serviço encontrado. Atualizando o serviço\n      aws apprunner start-deployment --service-arn \"$service_arn\"\n      echo Processo de atualização iniciado\n  else\n      echo Serviço não encontrado. Criando o serviço\n      response=$(aws apprunner create-service --cli-input-json file://aws-cli-input.json)\n      echo Criação 'do' serviço iniciada\n      service_url=$(echo \"$response\" | jq '.Service.ServiceUrl')\n      echo URL 'do' serviço \"$service_url\"\n  fi\n  return 0\n}\n";
        commandsFile << commandsFileContent;
        commandsFile.close();
        spdlog::info("Arquivo .ci/commands.sh gerado com sucesso");
    } else
        spdlog::info("O arquivo commands.sh já está presente na pasta .ci");
    if (!fs::exists(".ci/library.sh"))
    {
        std::ofstream libraryFile(".ci/library.sh");
        std::string listAppRunnerServices = "list_apprunner_services() {\n  local next_page_token=$1\n  local aws_region=$2\n  local response\n  response=$(aws apprunner list-services --region \"$aws_region\" --output json --next-token \"$next_page_token\")\n  echo \"$response\"\n}\n\n";
        std::string containsAppRunnerServices = "contains_apprunner_service() {\n  local service_name=$1\n  local aws_region=$2\n  local next_page_token=\"""\"\n  local response\n  local has_services\n  local services\n  local contains\n  local service_arn\n  while :\n  do\n    response=$(list_apprunner_services \"$next_page_token\" \"$aws_region\")\n    has_services=$(jq 'select(.ServiceSummaryList != [])' <<< \"$response\")\n    if [ ! \"$has_services\" ]; then\n      break\n    fi\n    services=$(jq -r '.ServiceSummaryList' <<< \"$response\")\n    contains=$(jq --arg service \"$service_name\" 'any(.[]; .ServiceName == $service)' <<< \"$services\")\n    if [ \"$contains\" = true ]; then\n      service_arn=$(jq -r --arg name \"$service_name\" '.[] | select(.ServiceName == $name) | .ServiceArn' <<< \"$services\")\n      echo \"$service_arn\"\n      return 0\n    fi\n    next_page_token=$(jq -r '.NextToken' <<< \"$response\")\n  done\n  echo false\n  return 0\n}\n";
        libraryFile << listAppRunnerServices + containsAppRunnerServices;
        libraryFile.close();
        spdlog::info("Arquivo .ci/library.sh gerado com sucesso");
    } else
        spdlog::info("O arquivo library.sh já está presente na pasta .ci");
}
