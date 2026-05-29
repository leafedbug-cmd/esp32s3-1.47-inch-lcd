#pragma once
// =============================================================================
// Blip Terminal - Screen Base Class
// All screens inherit from this for consistent interface
// =============================================================================

#include "../display/display_mgr.h"
#include "../data/data_store.h"

class ScreenBase {
public:
    virtual ~ScreenBase() = default;

    // Called once when transitioning TO this screen
    virtual void on_enter() {}

    // Called once when transitioning AWAY from this screen
    virtual void on_exit() {}

    // Called every loop tick - draw into sprite, then call display_flush()
    virtual void draw() = 0;

    // Screen name for header
    virtual const char* name() const = 0;

    // Screen index string e.g. "1/6"
    virtual const char* index_str() const = 0;
};
