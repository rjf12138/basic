# basic 库打包说明(pm 包)

## 打包

```bash
./pack.sh        # 编译 debug/release 两套 libbasic.a,打包并发布到 ~/.project_manager/repo/
./pack.sh -f     # 同名同版本已存在时强制覆盖(本地迭代用)
```

产物:

- `~/.project_manager/repo/basic-0.1.1.pm`(+ `.sha256` 摘要边车)
- 包内结构:

```
basic/
├── manifest.json          # name/version/lib_type=static/link_libs=["basic"]
├── include/               # 对外头文件(7 个 .h)
└── lib/
    ├── debug/libbasic.a   # -g -O0
    └── release/libbasic.a # -O2 -mtune=generic(可移植,无 -march=native)
```

## 其它项目依赖 basic

```bash
pm init MyApp .
pm pkg install basic -p MyApp        # 或 pm deps add basic -p MyApp
```

安装后会自动完成(无需手工配置):

- 头文件 → `MyApp/extern_inc/basic/*.h`
- 库文件 → `MyApp/lib/{debug,release}/libbasic.a`
- 配置 → `.pmproject` 写入 `dependencies` 与 `deps.libs: ["basic"]`

## 代码里怎么引用

pm 把 `extern_inc/` 加在 include 路径上,**头文件要带包名前缀**:

```cpp
#include <basic/wejson.h>
#include <basic/byte_buffer.h>

using namespace basic;
```

basic 头文件内部互引用使用同目录相对形式(`"byte_buffer.h"` 等),
平铺到 `extern_inc/basic/` 后仍能互相找到,无需额外配置。

## 升级版本

1. 修改 `pack/manifest.json` 的 `version` 与 `pack.sh` 的 `PKG_VERSION`(保持一致);
2. `./pack.sh`;
3. 项目里 `pm pkg install basic@<新版> -p MyApp`(旧版本残留会自动清理)。
