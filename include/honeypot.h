#pragma once
#include <string>
#include <vector>

struct HoneypotFile {
    std::wstring path;       // Full path to canary file
    std::wstring app_name;   // Which app it belongs to
    std::wstring data_type;  // What it pretends to be
};

// Create honeypot canary files next to real session files
std::vector<HoneypotFile> deploy_honeypots();

// Remove all honeypot files on exit
void cleanup_honeypots(const std::vector<HoneypotFile>& honeypots);
