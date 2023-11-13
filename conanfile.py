from conans import ConanFile, CMake
import os, shutil

class QtGrpcConan(ConanFile):
    name = "QtGrpc"
    version = "0.3"
    python_requires = "CommonConanFile/0.1@oius/dev"
    python_requires_extend = "CommonConanFile.HeaderOnlyLibConanFile"
    exports_sources = "qt-grpc/*"
    includedir = "./"
    editablemode_includedir = "./"
