#include <openvino/openvino.hpp>

#include "memory_test.hpp"


void do_test(TestContext &test) {
    test.start();

    ov::Core core;
    ov::CompiledModel model = core.compile_model(test.model_path, test.device);

    test.end();
}
