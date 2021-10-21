load("@rules_proto//proto:defs.bzl", "proto_library")
load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_proto_library")
load("@com_github_grpc_grpc//bazel:cc_grpc_library.bzl", "cc_grpc_library")

proto_library(
    name = "bank_proto",
    srcs = ["bank.proto"],
)

cc_proto_library(
    name = "bank_cc_proto",
    deps = [":bank_proto"],
)

cc_grpc_library(
    name = "bank_cc_grpc",
    srcs = [":bank_proto"],
    grpc_only = True,
    deps = [":bank_cc_proto"],
)

cc_library(
    name = "sync_bank_server",
    srcs = ["sync_bank_server.cc"],
    hdrs = ["sync_bank_server.h"],
    deps = [
        ":bank_cc_grpc",
        "@com_github_grpc_grpc//:grpc++",
    ],
)

cc_binary(
    name = "bank_client_main",
    srcs = ["bank_client_main.cc"],
    deps = [
        ":bank_cc_grpc",
        "@com_github_grpc_grpc//:grpc++",
    ],
)

cc_library(
    name = "async_bank_server",
    hdrs = ["async_bank_server.h"],
    deps = [
        ":bank_cc_grpc",
        "@com_github_grpc_grpc//:grpc++",
    ],
)

cc_binary(
    name = "bank_server_main",
    srcs = ["bank_server_main.cc"],
    deps = [
        ":sync_bank_server",
        ":async_bank_server",
    ],
)