# The C++ version of PyTorch is required to run this code. Unfortunately, linking to
# it is a real PITA. The find_package(Torch) command is supposed to do this, but it
# corrupts the build settings (e.g. setting debug options) and is generally a pain to
# work with.
#
# The Torchure DLL wraps this in a more friendly framework, at the cost of being
# slightly less efficient than raw C++
set(TORCHURE_LIBRARIES "$ENV{TORCHURE_ROOT_DIR}/lib/torchure.lib")
set(TORCHURE_INCLUDE_DIRS "$ENV{TORCHURE_ROOT_DIR}/include")
set(TORCHURE_BIN_DIR "$ENV{TORCHURE_ROOT_DIR}/bin")
