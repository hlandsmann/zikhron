path_to_srcipt="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
workspace=$(cd $path_to_srcipt;pwd)
cd $workspace

build_dir=$workspace/build
mkdir -p $build_dir

mode="Release"
cmake -B $build_dir \
    -DCMAKE_BUILD_TYPE=$mode \
    --preset=default
(cd $build_dir; cmake --build . -j$(nproc))
