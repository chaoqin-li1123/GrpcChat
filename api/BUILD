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
    visibility = ["//visibility:public"],
)