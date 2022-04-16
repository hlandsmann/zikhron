path_to_srcipt="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
workspace=$(cd $path_to_srcipt;pwd)
cd $workspace

build_dir=$workspace/build
tmp_dir=/run/user/$(id -u)/$(basename $workspace)/build

rm -rf $tmp_dir

if mount | grep $build_dir; then
    sudo umount $build_dir
fi

mkdir -p $tmp_dir
mkdir -p $build_dir
sudo mount -o bind $tmp_dir $build_dir
# ln -s $tmp_dir $build_dir

mode="Debug"
# mode="Release"
cmake -B $build_dir \
    -DCMAKE_BUILD_TYPE=$mode \
    -Dskip_run_conan=OFF \
    --preset=ninja
(cd $build_dir; cmake --build . -j$(nproc))
