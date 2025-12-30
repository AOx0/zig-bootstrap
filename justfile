build-ios cpu="apple_a10":
    CMAKE_GENERATOR=Ninja ./build aarch64-macos-none {{cpu}}
