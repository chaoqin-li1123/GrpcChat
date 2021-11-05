load("@rules_cc//cc:defs.bzl", "cc_binary")

cc_binary(
    name = "chat_client_main",
    srcs = ["chat_client_main.cc"],
    deps = [
        "//src:async_client",
        "//api:chat_cc_grpc",
        "//external:grpc",
    ],
)

cc_binary(
    name = "chat_server_main",
    srcs = ["chat_server_main.cc"],
    deps = [
        "//src:async_server",
        "//api:chat_cc_grpc",
    ],
)