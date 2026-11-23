#!/bin/bash
# 将 basic 库打包成 .pm 并发布到本地仓库
#
# 用法:
#   ./pack.sh            # 编译 debug/release 两套静态库,打包并发布到 ~/.project_manager/repo/
#   ./pack.sh -f         # 同名同版本已存在时强制覆盖
#
# 产物:
#   <name>-<version>.pm + <name>-<version>.pm.sha256  →  ~/.project_manager/repo/
#
# 打包产物结构(符合 pm 包规范):
#   basic/
#   ├── manifest.json    # 包元数据
#   ├── include/         # 对外头文件(安装到项目 extern_inc/basic/)
#   └── lib/
#       ├── debug/libbasic.a
#       └── release/libbasic.a
set -e

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
PKG_NAME="basic"
PKG_VERSION="0.1.1"
FORCE_FLAG=""
if [ "$1" = "-f" ]; then
    FORCE_FLAG="-f"
fi

WORK_DIR="$ROOT/pack/build"
PKG_DIR="$WORK_DIR/$PKG_NAME"

SRCS=(
    "$ROOT/src/byte_buffer.cc"
    "$ROOT/src/debug.cc"
    "$ROOT/src/err_handle.cc"
    "$ROOT/src/logger.cc"
    "$ROOT/src/wejson.cc"
)

echo "==> 清理旧的打包产物"
rm -rf "$WORK_DIR"
mkdir -p "$PKG_DIR/include" "$PKG_DIR/lib/debug" "$PKG_DIR/lib/release"

build_lib() {
    # $1 = 模式名(debug/release), $2 = 编译 flags
    local mode="$1"
    local flags="$2"
    local obj_dir="$WORK_DIR/obj_$mode"
    mkdir -p "$obj_dir"

    echo "==> 编译 $mode 静态库 (flags: $flags)"
    for src in "${SRCS[@]}"; do
        local base
        base="$(basename "$src" .cc)"
        g++ -std=c++11 $flags -I"$ROOT/inc" -c "$src" -o "$obj_dir/$base.o"
    done

    ar rcs "$PKG_DIR/lib/$mode/lib$PKG_NAME.a" "$obj_dir"/*.o
    echo "    -> $PKG_DIR/lib/$mode/lib$PKG_NAME.a"
}

# debug: 调试信息、不做优化(与 pm 约定不同,这里只是本机项目自用,发布 release 请用可移植 flags)
build_lib debug "-g -O0"

# release: 可移植 flags(不用 -march=native,避免绑定构建机 CPU)
build_lib release "-O2 -mtune=generic"

echo "==> 拷贝头文件与 manifest"
cp "$ROOT"/inc/*.h "$PKG_DIR/include/"
cp "$ROOT/pack/manifest.json" "$PKG_DIR/"

echo "==> 打包并发布到本地仓库"
pm pkg create "$PKG_DIR" $FORCE_FLAG

echo "==> 完成,仓库内容:"
pm pkg list | grep -E "$PKG_NAME|name" || true
