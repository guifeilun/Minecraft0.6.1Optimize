# Minecraft PE 0.6.1 — XP / Win2K fork
Based on JackTulli's build                                                                                                                                                        
# 项目目标：                                                                                                                                                                      
1.实现无限世界                                                                                                                                                                     
2.全体概念UI                                                                                                                                                                       
3.directsound ✔                                                                                                                                                                  
4.删除usetouchscreen检测                                                                                                                                                           
5.添加设置页面滚动逻辑                                                                                                                                                              
6.修复chunks闪退bug                                                                                                                                                                
7.补全合成表                                                                                                                                                                       
8.创造模式物品栏加入下界岩方块                                                                                                                                                      
9.直接添加红石灯贴图                                                                                                                                                               
10.windows缩放默认开启
11.缩小文件体积 ✔                                                                                                                                                                 
# JackTulli's changes
1.Redstone ✔
2./time command ✔
3.HUD ✔ 
4.Save / load ✔
5.删除了 Kolyah35 信用文本、GitHub 图标位图和可点击的 URL 处理程序。 StartMenuScreen 和 TouchStartMenuScreen ✔
6.OpenGL上下文请求已从2.1降低到1.1 ✔

# Building
支持的构建是 Windows + Visual Studio 2017构建工具 与 v141_xp工具集。PowerShell脚本处理vcvars，选择正确的 Ninja，并将 二进制文件写入 build-xp\MinecraftPE.exe.

# from the repo root
powershell.exe -ExecutionPolicy Bypass -File .\build-xp.ps1

# clean rebuild
.\build-xp.ps1 -Clean
要求：

Visual Studio 2017 Build Tools with the v141_xp (Windows XP) toolset installed. The script pins -vcvars_ver=14.16 so newer toolsets won't silently take over.
CMake 3.21 or newer.
Ninja on PATH (or update the $NinjaDir parameter in build-xp.ps1).
Git (some CMake FetchContent deps clone at configure time).
输出为 build-xp\MinecraftPE.exe。 data\ 目录旁边的exe 包含游戏资产（terrain.png，items.png，语言文件等）—— 这些被复制到 build-xp\data\作为构建的一部分。
