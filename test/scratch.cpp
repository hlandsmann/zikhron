// #include <spdlog/spdlog.h>
//
// #include <cstring>
// #include <string>
//
// #include <errno.h>
// #include <stdio.h>
// #include <stdlib.h>
// #include <unistd.h>
//
// #define PIPE_READ 0
// #define PIPE_WRITE 1
//
// int createChild(const char* szCommand, char* const aArguments[], char* const aEnvironment[], const char* szMessage)
// {
//     int aStdinPipe[2];
//     int aStdoutPipe[2];
//     int nChild;
//     char nChar;
//     int nResult;
//
//     if (pipe(aStdinPipe) < 0) {
//         perror("allocating pipe for child input redirect");
//         return -1;
//     }
//     if (pipe(aStdoutPipe) < 0) {
//         close(aStdinPipe[PIPE_READ]);
//         close(aStdinPipe[PIPE_WRITE]);
//         perror("allocating pipe for child output redirect");
//         return -1;
//     }
//
//     nChild = fork();
//     if (0 == nChild) {
//         // child continues here
//
//         // redirect stdin
//         if (dup2(aStdinPipe[PIPE_READ], STDIN_FILENO) == -1) {
//             exit(errno);
//         }
//
//         // redirect stdout
//         if (dup2(aStdoutPipe[PIPE_WRITE], STDOUT_FILENO) == -1) {
//             exit(errno);
//         }
//
//         // redirect stderr
//         if (dup2(aStdoutPipe[PIPE_WRITE], STDERR_FILENO) == -1) {
//             exit(errno);
//         }
//
//         // all these are for use by parent only
//         close(aStdinPipe[PIPE_READ]);
//         close(aStdinPipe[PIPE_WRITE]);
//         close(aStdoutPipe[PIPE_READ]);
//         close(aStdoutPipe[PIPE_WRITE]);
//
//         chdir("/home/harmen/zikhron/sudachi-0.7.5/");
//
//         // run child process image
//         // replace this with any exec* function find easier to use ("man exec")
//         nResult = execve(szCommand, aArguments, aEnvironment);
//         spdlog::info("exec {}", szCommand);
//
//         // if we get here at all, an error occurred, but we are in the child
//         // process, so just exit
//         exit(nResult);
//         spdlog::info("error {}", szCommand);
//     } else if (nChild > 0) {
//         // parent continues here
//
//         // close unused file descriptors, these are for child only
//         close(aStdinPipe[PIPE_READ]);
//         close(aStdoutPipe[PIPE_WRITE]);
//         // sleep(1);
//
//         // Include error check here
//         if (NULL != szMessage) {
//             spdlog::info("Hello World: {}", szMessage);
//             write(aStdinPipe[PIPE_WRITE], szMessage, strlen(szMessage));
//         }
//         sleep(1);
//         spdlog::info("slept");
//
//         std::string response;
//         // Just a char by char read here, you can change it accordingly
//         while (read(aStdoutPipe[PIPE_READ], &nChar, 1) == 1) {
//             write(STDOUT_FILENO, &nChar, 1);
//             response += nChar;
//         }
//         spdlog::info("done, msg: {}", response);
//
//         // done with these in this example program, you would normally keep these
//         // open of course as long as you want to talk to the child
//         close(aStdinPipe[PIPE_WRITE]);
//         close(aStdoutPipe[PIPE_READ]);
//     } else {
//         // failed to create child
//         close(aStdinPipe[PIPE_READ]);
//         close(aStdinPipe[PIPE_WRITE]);
//         close(aStdoutPipe[PIPE_READ]);
//         close(aStdoutPipe[PIPE_WRITE]);
//     }
//     return nChild;
// }
//
// int main(int argc, char* argv_[])
// {
//     spdlog::info("Hello {:03d}", 3);
//     char* const* aEnv = environ; //{nullptr};
//     // char* argv[] = {"-jar", "/home/harmen/zikhron/sudachi-0.7.5/sudachi-0.7.5.jar", nullptr};
//     char* argv[] = {"", "-jar", "sudachi-0.7.5.jar", nullptr};
//     // char* argv[] = {"sudachi-0.7.5.jar", nullptr};
//     // /usr/bin/java --jar /home/harmen/zikhron/sudachi-0.7.4/sudachi-0.7.4.jar
//     std::string text1 = "投降してほしけりゃてめえをあと百万体呼んで来るんだな\n";
//     // std::string text2 = "あとでジュース おごってあげるね - 暑そうだから";
//     // return createChild("/usr/bin/mecab", argv_, aEnv,text1.c_str());
//     return createChild("/usr/bin/java", argv, aEnv, text1.c_str());
//     // return createChild("/home/harmen/src/zikhron/build/test/sr_treewalker", argv_, aEnv, "hello world");
//     // return createChild("/home/harmen/src/zikhron/build/test/callee", argv, aEnv, "hello world");
//     // return createChild("/usr/bin/ls", argv, aEnv, "hello world");
//     // return createChild("/usr/bin/printenv", argv_, aEnv, "");
// }
//
// // auto main() -> int
// // {
// //     std::string text1 = "投降してほしけりゃてめえをあと百万体呼んで来るんだな";
// //     createChild("mecab", nullptr, nullptr, text1.c_str());
// //
// //     return 0;
// #pragma once

#include <boost/di.hpp>
#include <memory>
#include <iostream>

namespace di = boost::di;

struct EarlyInit1 {
    EarlyInit1() { std::cout << "EarlyInit1 constructed\n"; }
};
struct EarlyInit2 {
    EarlyInit2() { std::cout << "EarlyInit2 constructed\n"; }
};

struct App {
    App(std::shared_ptr<EarlyInit1> e, std::shared_ptr<EarlyInit2> e2) {
        std::cout << "App constructed\n";
    }
};

int main() {
    auto injector = di::make_injector();

    // 👇 Force early construction
    // auto& early = injector.create<EarlyInit1&>();

    // Continue with normal injection
    auto app = injector.create<std::unique_ptr<App>>();

    return 0;
}
