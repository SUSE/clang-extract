#!/bin/sh

LLVM_VERSION_NUMBER=$1

if [ "$LLVM_VERSION_NUMBER" = "" ]; then
  echo "Error: expected a number to be passed as argument."
  echo ""
  echo "Usage: suse-install-llvm.sh <NUMBER>"
  exit 1
fi

sudo zypper install libLLVM$LLVM_VERSION_NUMBER \
                    libLLVM$LLVM_VERSION_NUMBER-debuginfo \
                    llvm$LLVM_VERSION_NUMBER \
                    llvm$LLVM_VERSION_NUMBER-devel \
                    llvm$LLVM_VERSION_NUMBER-debuginfo \
                    clang$LLVM_VERSION_NUMBER \
                    clang$LLVM_VERSION_NUMBER-debuginfo \
                    libclang-cpp$LLVM_VERSION_NUMBER-debuginfo \
                    libclang-cpp$LLVM_VERSION_NUMBER-debuginfo \
                    libclang-cpp$LLVM_VERSION_NUMBER \
                    clang$LLVM_VERSION_NUMBER-devel
