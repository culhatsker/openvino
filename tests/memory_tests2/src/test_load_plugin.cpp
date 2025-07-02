#include <openvino/openvino.hpp>

#include "memory_test.hpp"


void do_test(TestContext &test) {
    test.start();

    ov::Core core;
    core.get_versions(test.device);

    test.end();
}
