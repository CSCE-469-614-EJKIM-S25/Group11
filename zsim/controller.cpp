#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <queue>
#include <signal.h>
#include <string>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

// Original MAX_CONCURRENT 14
#define MAX_CONCURRENT 1
#define TOTAL_SIMULATIONS 21

std::vector<pid_t> running;
std::ofstream masterLog;

void logMessage(const std::string &msg) {
  time_t now = time(nullptr);
  char buf[64];
  strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", localtime(&now));
  masterLog << "[" << buf << "] " << msg << std::endl;
  masterLog.flush();
}

void signalHandler(int signum) {
  logMessage("Received signal " + std::to_string(signum) + ". Cleaning up...");
  for (pid_t pid : running) {
    kill(pid, SIGTERM);
  }
  while (!running.empty()) {
    int status;
    waitpid(-1, &status, 0);
    running.pop_back();
  }
  logMessage("All simulations terminated. Exiting.");
  masterLog.close();
  exit(0);
}

int main() {
  masterLog.open("controller.log");

  signal(SIGTERM, signalHandler);
  signal(SIGINT, signalHandler);

  std::string replPolicy;
  std::cout << "Enter the replacement policy you want to test (LRU, LFU, "
               "SRRIP, SHiP): ";
  std::getline(std::cin, replPolicy);

  // dedup and freqmine are exluded because they do not work
  std::vector<std::pair<std::string, std::string>> simulations = {
      {"SPEC", "bzip2"},
      {"SPEC", "gcc"},
      {"SPEC", "mcf"},
      {"SPEC", "hmmer"},
      {"SPEC", "sjeng"},
      {"SPEC", "libquantum"},
      {"SPEC", "xalan"},
      {"SPEC", "milc"},
      {"SPEC", "cactusADM"},
      {"SPEC", "leslie3d"},
      {"SPEC", "namd"},
      {"SPEC", "soplex"},
      {"SPEC", "calculix"},
      {"SPEC", "lbm"},
      {"PARSEC", "blackscholes"},
      {"PARSEC", "bodytrack"},
      {"PARSEC", "canneal"},
      //{"PARSEC", "dedup"},
      {"PARSEC", "fluidanimate"},
      //{"PARSEC", "freqmine"},
      {"PARSEC", "streamcluster"},
      {"PARSEC", "swaptions"},
      {"PARSEC", "x264"}};

  std::queue<int> remaining;
  for (int i = 0; i < TOTAL_SIMULATIONS; ++i) {
    remaining.push(i);
  }

  while (!remaining.empty() || !running.empty()) {
    while (running.size() < MAX_CONCURRENT && !remaining.empty()) {
      int simId = remaining.front();
      remaining.pop();

      std::string suite = simulations[simId].first;
      std::string benchmark = simulations[simId].second;

      std::string command =
          "./projectrunscript " + suite + " " + benchmark + " " + replPolicy;
      std::cout << "Running command: " << command << std::endl;
      logMessage("Running command: " + command);

      pid_t pid = fork();
      if (pid == -1) {
        perror("fork failed");
        exit(1);
      } else if (pid == 0) {
        // Child process
        std::string logFile =
            "sim_logs/sim_" + suite + "_" + benchmark + ".log";
        int fd = open(logFile.c_str(), O_CREAT | O_WRONLY | O_TRUNC, 0644);
        if (fd < 0) {
          perror("open log file failed");
          exit(1);
        }
        dup2(fd, STDOUT_FILENO);
        dup2(fd, STDERR_FILENO);
        close(fd);

        execl("./projectrunscript", "./projectrunscript", suite.c_str(),
              benchmark.c_str(), replPolicy.c_str(), (char *)nullptr);
        perror("exec failed");
        exit(1);
      } else {
        running.push_back(pid);
        logMessage("Launched simulation " + suite + " " + benchmark +
                   " with PID " + std::to_string(pid));
      }
    }

    int status;
    pid_t finished = waitpid(-1, &status, 0);
    if (finished == -1) {
      perror("waitpid failed");
      exit(1);
    }

    running.erase(std::remove(running.begin(), running.end(), finished),
                  running.end());

    if (WIFEXITED(status)) {
      int exitCode = WEXITSTATUS(status);
      logMessage("Simulation with PID " + std::to_string(finished) +
                 " completed. Exit code: " + std::to_string(exitCode));
    } else if (WIFSIGNALED(status)) {
      int signalNum = WTERMSIG(status);
      logMessage("Simulation with PID " + std::to_string(finished) +
                 " terminated by signal: " + std::to_string(signalNum));
    } else {
      logMessage("Simulation with PID " + std::to_string(finished) +
                 " ended abnormally.");
    }
  }

  logMessage("All simulations finished successfully!");
  masterLog.close();
  return 0;
}
