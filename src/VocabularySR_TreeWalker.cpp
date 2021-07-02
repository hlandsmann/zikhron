#include <chrono>
#include <fmt/format.h>
#include "VocabularySR.h"
VocabluarySR_TreeWalker::VocabluarySR_TreeWalker() {
    worker = std::thread([this]() {
        while (running) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            // fmt::print("Print from thread\n");
        }
    });
}

VocabluarySR_TreeWalker::~VocabluarySR_TreeWalker() {
    running = false;
    worker.join();
}
