#pragma once
#include <windows.h>
#include <string>
#include <vector>
#include <unordered_map>

class LaunchArgsManager {
public:
    LaunchArgsManager() {
        int argcW = 0;
        LPWSTR* argvW = CommandLineToArgvW(GetCommandLineW(), &argcW);
        if (!argvW) return;

        std::vector<std::string> utf8Args;
        utf8Args.reserve(argcW);

        for (int i = 0; i < argcW; ++i) {
            int len = WideCharToMultiByte(CP_UTF8, 0, argvW[i], -1, nullptr, 0, nullptr, nullptr);
            std::string arg(len - 1, '\0');
            WideCharToMultiByte(CP_UTF8, 0, argvW[i], -1, arg.data(), len, nullptr, nullptr);
            utf8Args.push_back(std::move(arg));
        }

        LocalFree(argvW);

        for (size_t i = 1; i < utf8Args.size(); ++i) { // skip argv[0] = exe name
            if (utf8Args[i].size() > 0 && utf8Args[i][0] == '-') {
                std::string key = utf8Args[i];
                std::string val = "";

                if (i + 1 < utf8Args.size() && utf8Args[i + 1][0] != '-') {
                    val = utf8Args[i + 1];
                    ++i;
                }

                args[key] = val;
            }
        }

    }

    bool has_arg(const std::string& name) const {
        return args.find(name) != args.end();
    }

    std::string get_arg(const std::string& name, const std::string& def = "") const {
        auto it = args.find(name);
        if (it != args.end()) {
            return it->second;
        }
        return def;
    }

private:
    std::unordered_map<std::string, std::string> args;
};
