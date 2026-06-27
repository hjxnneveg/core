// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 Joshua C Marshall

#pragma once

#include "reporting.hpp"

#include <cstring>
#include <functional>
#include <iostream>
#include <string>
#include <vector>

namespace hjx {

class cli {
    struct option_t {
        std::string name;
        std::string param;
        std::string desc;
        std::function<void(const char*)> cb;
    };

    std::vector<option_t> options;

    const option_t *find(const char *name) const {
        for (const option_t &opt : options)
            if (opt.name == name) return &opt;

        return nullptr;
    }

public:
    struct usage {
        usage(const std::vector<option_t> &options, std::ostream &os) {
            os << "usage:\n";
            for (const option_t &opt : options) {
                os << "--" << opt.name;
                if (opt.param.size()) os << " " << opt.param;
                os << std::endl;
            }
        }
    };

    virtual void args(const std::vector<const char*>&) = 0;

    void eval(const char **argv) {
        std::vector<const char*> arguments;
        bool ignore_options = false;

        argv++;
        while (*argv) {
            const char *arg = *argv++;

            if (!strcmp(arg, "--")) { ignore_options = true; continue; }

            if (!ignore_options && !strncmp(arg, "--", 2)) {
                const option_t *opt = find(arg + 2);
                if (!opt) throw usage(options, std::cerr);

                const char *value = nullptr;
                if (!opt->param.empty()) {
                    value = *argv++;
                    if (!value) throw usage(options, std::cerr);
                }

                opt->cb(value);
                continue;
            }

            arguments.emplace_back(arg);
        }

        args(arguments);
    }

    void option(const char *name,
                const char *param,
                const char *desc,
                const std::function<void(const char*)> &cb) {
        ASSERT(name);
        ASSERT(name[0]);
        ASSERT_IMPLY(param, param[0]);
        ASSERT(!find(name));

        options.emplace_back(name,
                             param ? param : "",
                             desc ? desc : "",
                             cb);
    }
};

}
