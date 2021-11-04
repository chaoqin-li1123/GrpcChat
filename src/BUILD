load("@rules_proto//proto:defs.bzl", "proto_library")
load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_proto_library")
load("@com_github_grpc_grpc//bazel:cc_grpc_library.bzl", "cc_grpc_library")

proto_library(
    name = "chat_proto",
    srcs = ["chat.proto"],
)

cc_proto_library(
    name = "chat_cc_proto",
    deps = [":chat_proto"],
)

cc_grpc_library(
    name = "chat_cc_grpc",
    srcs = [":chat_proto"],
    grpc_only = True,
    deps = [":chat_cc_proto"],
)

cc_library(
    name = "async_client",
    hdrs = ["async_client.h"],
    deps = [
        ":chat_cc_grpc",
        ":async_stream",
        "//external:grpc",
    ],
)

cc_binary(
    name = "chat_client_main",
    srcs = ["chat_client_main.cc"],
    deps = [
        ":async_client",
        ":chat_cc_grpc",
        "//external:grpc",
    ],
)

cc_library(
    name = "async_stream",
    hdrs = ["async_stream.h"],
    deps = [
        ":chat_cc_grpc",
        "//external:grpc",
    ],
)

cc_library(
    name = "async_server",
    hdrs = ["async_server.h"],
    deps = [
        ":chat_cc_grpc",
        ":async_stream",
        "//external:grpc",
    ],
)

cc_binary(
    name = "chat_server_main",
    srcs = ["chat_server_main.cc"],
    deps = [
        ":async_server",
    ],
)