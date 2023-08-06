path_to_srcipt="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
workspace=$(cd "$path_to_srcipt"||exit;pwd)
cd "$workspace" || exit

build_dir=$workspace/build

mkdir -p "$build_dir"

mode="Debug"
# mode="Release"
cmake -B "$build_dir" \
    -DCMAKE_BUILD_TYPE=$mode \
    --preset=ninja
(cd "$build_dir"||exit; cmake --build . -j"$(nproc)")
