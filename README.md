[![PRs Welcome](https://img.shields.io/badge/PRs-welcome-brightgreen.svg)](https://github.com/xuyanghuang-tencent/LyraWithUnLua)

# 项目简介

演示Lua脚本在更加复杂项目中扮演的角色，以及UnLua相关功能和工具链的使用。

注：对于零基础的UE萌新来说，这个示例项目还是有些过于复杂了，可以先从[UnLua自带的示例](https://github.com/Tencent/UnLua#%E5%BC%80%E5%A7%8Bunlua%E4%B9%8B%E6%97%85)入手。

# 如何开始

## 开发环境
- [虚幻引擎 5.0.3](https://www.unrealengine.com/zh-CN/download?lang=zh-CN)
- [Visual Studio 2022](https://visualstudio.microsoft.com/zh-hans/vs/) 或 [JetBrains Rider 2022](https://www.jetbrains.com/rider/download/#section=windows)
- [Visual Studio Code](https://code.visualstudio.com/) 以及对应插件 [Lua Booster](https://marketplace.visualstudio.com/items?itemName=operali.lua-booster)

## 工程准备
1. 为`Lyra.uproject`生成工程文件
2. 使用 VS 或者 Rider 打开`Lyra.sln`
3. 选择 `DebugGame Editor` 配置来启动UE编辑器
4. 使用 VSCode 打开 `Lyra.code-workspace`

注：如果不需要C++相关的调试，可以直接双击 `Lyra.uproject` 打开游戏工程

不出意外的话，此时你已经可以在编辑器中点击运行游戏来体验Lyra游戏了。

## 智能提示
1. 在引擎主界面的工具栏中找到UnLua图标，在下拉菜单中选择[生成智能提示](https://github.com/Tencent/UnLua/blob/master/Docs/CN/IntelliSense.md)
2. 第一次全量生成比较慢，后续蓝图有变动，会自动更新，不需要再次手动生成
3. 生成成功后的文件可以在VSCode的工作区中的 `IntelliSense` 目录下找到

## 调试Lua脚本
1. 在 VSCode 中启动[任意Lua调试器](https://github.com/Tencent/UnLua/blob/master/Docs/CN/Debugging.md)，等待连接
2. 运行游戏，在 `Main.lua` 中就会主动连接本地调试器，默认端口8818
3. 在 Lua 代码中加断点就可以命中了

注：如果你的调试器支持Attach模式，可以直接F5附加到游戏进程上

# 演示内容
- [x] VSCode工程结构
- [x] 自定义启动入口脚本
- [x] 自定义ModuleLocator的使用
- [ ] 自定义加载器的使用
- [x] UMG相关的覆写
- [ ] 反射对象的覆写
- [ ] 非反射对象静态导出
- [x] 智能提示的生成与使用
- [x] 调试Lua脚本代码
- [ ] 调试Lua源码
- [ ] 工程打包
- [ ] 热更新

# 关于 Lyra Starter Game
这是在虚幻引擎5中提供的游戏玩法示例项目，也是用来演示有关UE5开发的最新最佳实践，相关文档可以在[这里](https://docs.unrealengine.com/5.0/zh-CN/lyra-sample-game-in-unreal-engine/)找到。

# 代码贡献
原则上不修改、不扩展Lyra原有的游戏玩法，尽量使用Lua绑定覆写的方式来完成。

# 交流
官方交流QQ群：936285107

更推荐使用[讨论](https://github.com/xuyanghuang-tencent/LyraWithUnLua/discussions)功能，无论是在UnLua功能上的疑问或者Lyra架构设计方面的灵感都欢迎讨论，沉淀知识到社区也是一种开源贡献。