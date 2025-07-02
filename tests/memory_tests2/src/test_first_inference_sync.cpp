#include <openvino/openvino.hpp>

#include "memory_test.hpp"


void do_test(TestContext &test) {
    test.start();

    ov::Core core;
    ov::CompiledModel model = core.compile_model(test.model_path, test.device);

    auto ireq = model.create_infer_request();

    for (auto input: model.inputs()) {
        ireq.set_tensor(input, {input});
    }

    ireq.infer();

    test.end();
}
