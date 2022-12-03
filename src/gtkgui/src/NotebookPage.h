#pragma once

class NotebookPage {
public:
    virtual ~NotebookPage() = default;
    virtual void switchPage([[maybe_unused]] bool active){};
};