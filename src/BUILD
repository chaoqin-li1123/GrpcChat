load("@rules_proto//proto:defs.bzl", "proto_library")
load("@rules_cc//cc:defs.bzl", "cc_binary")
load("@com_github_grpc_grpc//bazel:cc_grpc_library.bzl", "cc_grpc_library")



cc_library(
    name = "async_client",
    hdrs = ["async_client.h"],
    deps = [
        "//api:chat_cc_grpc",
        ":async_stream",
        "//external:grpc",
    ],
    visibility = [
        "//exe:__pkg__",
    ],
)


cc_library(
    name = "async_stream",
    hdrs = ["async_stream.h"],
    deps = [
        "//api:chat_cc_grpc",
        "//external:grpc",
    ],
)

cc_library(
    name = "async_server",
    hdrs = ["async_server.h"],
    deps = [
        "//api:chat_cc_grpc",
        ":async_stream",
        "//external:grpc",
    ],
    visibility = [
        "//exe:__pkg__",
    ],
)

