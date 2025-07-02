# Copyright (C) 2018-2025 Intel Corporation
# SPDX-License-Identifier: Apache-2.0
#

cmake_minimum_required(VERSION 3.13)
project("OpenVINO Memory Tests" CXX)

find_package(OpenVINO REQUIRED)

function(add_test)
    set(EXE_NAME "test_${ARGV0}")
    add_executable(${EXE_NAME} "src/${EXE_NAME}.cpp")
    target_link_libraries(${EXE_NAME} openvino::runtime)
    # target_compile_definitions(${EXE_NAME} PUBLIC TEST_NAME=${ARGV0})
endfunction()


add_test(load_plugin)
add_test(load_network)
add_test(first_inference_sync)
add_test(first_inference_async)
