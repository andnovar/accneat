#include "std.h" // Must be included first. Precompiled header with standard library includes.
#include "staticexperiment.h"

using namespace NEAT;
using namespace std;

static struct XorInit {
    XorInit() {
        create_static_experiment("xor", [] () {
                const real_t T = 1.0;
                const real_t F = 0.0;
                const real_t weight = 1.0;

                vector<Test> tests = {
                    {{
                            {{F, F, F}, {F}, weight},
                    }},
                    {{
                            {{F, F, T}, {T}, weight},
                    }},
                    {{
                            {{F, T, F}, {T}, weight},
                    }},
                    {{
                            {{F, T, T}, {F}, weight}
                    }},
                    {{
                            {{T, F, F}, {T}, weight}
                    }},
                    {{
                            {{T, F, T}, {F}, weight}
                    }},
                    {{
                            {{T, T, F}, {F}, weight}
                    }},
                    {{
                            {{T, T, T}, {T}, weight}
                    }}
                };

                return tests;
            });
    }
} init;
