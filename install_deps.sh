#!/usr/bin/env bash
set -e

echo "Updating apt repositories..."
apt-get update

echo "Installing dependencies..."
# Dependencies list from README.md
apt-get install -y \
    build-essential \
    libglew-dev \
    libglu1-mesa-dev \
    libsqlite3-dev \
    libx11-dev \
    libgl-dev \
    pkg-config \
    libopencv-dev \
    libeigen3-dev \
    libc6-dev \
    libspdlog-dev \
    libboost-all-dev \
    libgflags-dev \
    git \
    libglfw3-dev \
    glslang-dev \
    glslang-tools \
    libjpeg-dev \
    libusb-1.0.0-dev \
    libuvc-dev \
    libopencv-dev \
    libopenxr-dev \
    libopenxr1-monado \
    libpng-dev \
    libsdl2-dev \
    libtiff-dev \
    udev \
    libudev-dev \
    libvulkan-dev \
    vulkan-validationlayers \
    libwayland-dev \
    wayland-protocols \
    libx11-xcb-dev \
    libxcb-glx0-dev \
    libxcb-randr0-dev \
    libxkbcommon-dev \
    cmake

echo "Dependencies installed successfully."
