# This Repo
Based on nillerusr's source engine base, built to bring Android a SteamOS level experience, both in gameplay feel and visual polish to the Source Engine base.
Mostly vibecoded, though no trash code is making it into the repo since I actually know C++. The progress is below, also with a detailed treemap.

## Features 
- GamepadUI (`-gamepadui`) for HL2, Portal, all gamepadui assets on apk so no need for extra files
- MangoHud (`-mangohud`) exact port with all features, customizable via `mangohud.conf` in your game directory (`srceng` or your custom game files path)
- Some GamepadUI fixes (key binding fix for mouse and touch)
- (GamepadUI) Touch menu in settings instead of steam input bs
- Bink video and bink audio, valve intro (media/valve.bik) with sound, to close it `-novid` or `-nointro`

## Current tasks 
- (Currently W.I.P) Fix Graphics, Shaders & Lighting and Merged episodes (not sure about merged episodes) from 20th anniversary update
- dxvk-native (Tested on csso-android-enhanced. Will implement it here soon and finish it up. It has some glitches, but it boots and runs fast. Needs fixes.)

<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 1000 420" width="1000" height="420">
<rect x="0.00" y="0.00" width="1000.00" height="420.00" fill="#0d1117" stroke="#0d1117" stroke-width="0"/>
<text x="4.00" y="22.00" font-family="sans-serif" font-size="16" fill="#fff" stroke="#0d1117" stroke-width="3" paint-order="stroke">Source Engine Android: 74.29% (26/35)</text>
<g><title>20th Anniversary: 9/15 (60%)</title>
<rect x="1.00" y="35.00" width="106.64" height="96.00" fill="#2ea043" stroke="#0d1117" stroke-width="0.5"/>
<rect x="107.64" y="35.00" width="106.64" height="96.00" fill="#2ea043" stroke="#0d1117" stroke-width="0.5"/>
<rect x="214.29" y="35.00" width="106.64" height="96.00" fill="#2ea043" stroke="#0d1117" stroke-width="0.5"/>
<rect x="320.93" y="35.00" width="106.64" height="96.00" fill="#2ea043" stroke="#0d1117" stroke-width="0.5"/>
<rect x="1.00" y="131.00" width="106.64" height="96.00" fill="#2ea043" stroke="#0d1117" stroke-width="0.5"/>
<rect x="107.64" y="131.00" width="106.64" height="96.00" fill="#2ea043" stroke="#0d1117" stroke-width="0.5"/>
<rect x="214.29" y="131.00" width="106.64" height="96.00" fill="#2ea043" stroke="#0d1117" stroke-width="0.5"/>
<rect x="320.93" y="131.00" width="106.64" height="96.00" fill="#2ea043" stroke="#0d1117" stroke-width="0.5"/>
<rect x="1.00" y="227.00" width="106.64" height="96.00" fill="#2ea043" stroke="#0d1117" stroke-width="0.5"/>
<rect x="107.64" y="227.00" width="106.64" height="96.00" fill="#6e7681" stroke="#0d1117" stroke-width="0.5"/>
<rect x="214.29" y="227.00" width="106.64" height="96.00" fill="#6e7681" stroke="#0d1117" stroke-width="0.5"/>
<rect x="320.93" y="227.00" width="106.64" height="96.00" fill="#6e7681" stroke="#0d1117" stroke-width="0.5"/>
<rect x="1.00" y="323.00" width="142.19" height="96.00" fill="#6e7681" stroke="#0d1117" stroke-width="0.5"/>
<rect x="143.19" y="323.00" width="142.19" height="96.00" fill="#6e7681" stroke="#0d1117" stroke-width="0.5"/>
<rect x="285.38" y="323.00" width="142.19" height="96.00" fill="#6e7681" stroke="#0d1117" stroke-width="0.5"/>
<rect x="0.00" y="34.00" width="428.57" height="386.00" fill="none" stroke="#0d1117" stroke-width="2"/>
<text x="5.00" y="50.00" font-family="sans-serif" font-size="12" fill="#fff" stroke="#0d1117" stroke-width="3" paint-order="stroke">20th Anniversary 60%</text></g>
<g><title>GamepadUI: 5/5 (100%)</title>
<rect x="429.57" y="35.00" width="85.05" height="106.22" fill="#2ea043" stroke="#0d1117" stroke-width="0.5"/>
<rect x="514.62" y="35.00" width="85.05" height="106.22" fill="#2ea043" stroke="#0d1117" stroke-width="0.5"/>
<rect x="599.67" y="35.00" width="85.05" height="106.22" fill="#2ea043" stroke="#0d1117" stroke-width="0.5"/>
<rect x="429.57" y="141.22" width="127.57" height="106.22" fill="#2ea043" stroke="#0d1117" stroke-width="0.5"/>
<rect x="557.14" y="141.22" width="127.57" height="106.22" fill="#2ea043" stroke="#0d1117" stroke-width="0.5"/>
<rect x="428.57" y="34.00" width="257.14" height="214.44" fill="none" stroke="#0d1117" stroke-width="2"/>
<text x="433.57" y="50.00" font-family="sans-serif" font-size="12" fill="#fff" stroke="#0d1117" stroke-width="3" paint-order="stroke">GamepadUI 100%</text></g>
<g><title>MangoHud: 4/4 (100%)</title>
<rect x="429.57" y="249.44" width="127.57" height="84.78" fill="#2ea043" stroke="#0d1117" stroke-width="0.5"/>
<rect x="557.14" y="249.44" width="127.57" height="84.78" fill="#2ea043" stroke="#0d1117" stroke-width="0.5"/>
<rect x="429.57" y="334.22" width="127.57" height="84.78" fill="#2ea043" stroke="#0d1117" stroke-width="0.5"/>
<rect x="557.14" y="334.22" width="127.57" height="84.78" fill="#2ea043" stroke="#0d1117" stroke-width="0.5"/>
<rect x="428.57" y="248.44" width="257.14" height="171.56" fill="none" stroke="#0d1117" stroke-width="2"/>
<text x="433.57" y="264.44" font-family="sans-serif" font-size="12" fill="#fff" stroke="#0d1117" stroke-width="3" paint-order="stroke">MangoHud 100%</text></g>
<g><title>Video: 4/4 (100%)</title>
<rect x="686.71" y="35.00" width="77.57" height="92.91" fill="#2ea043" stroke="#0d1117" stroke-width="0.5"/>
<rect x="764.29" y="35.00" width="77.57" height="92.91" fill="#2ea043" stroke="#0d1117" stroke-width="0.5"/>
<rect x="686.71" y="127.91" width="155.14" height="92.91" fill="#2ea043" stroke="#0d1117" stroke-width="0.5"/>
<rect x="686.71" y="220.82" width="155.14" height="92.91" fill="#2ea043" stroke="#0d1117" stroke-width="0.5"/>
<rect x="685.71" y="34.00" width="157.14" height="280.73" fill="none" stroke="#0d1117" stroke-width="2"/>
<text x="690.71" y="50.00" font-family="sans-serif" font-size="12" fill="#fff" stroke="#0d1117" stroke-width="3" paint-order="stroke">Video 100%</text></g>
<g><title>Android / CI: 3/4 (75%)</title>
<rect x="843.86" y="35.00" width="77.57" height="92.91" fill="#2ea043" stroke="#0d1117" stroke-width="0.5"/>
<rect x="921.43" y="35.00" width="77.57" height="92.91" fill="#2ea043" stroke="#0d1117" stroke-width="0.5"/>
<rect x="843.86" y="127.91" width="155.14" height="92.91" fill="#2ea043" stroke="#0d1117" stroke-width="0.5"/>
<rect x="843.86" y="220.82" width="155.14" height="92.91" fill="#6e7681" stroke="#0d1117" stroke-width="0.5"/>
<rect x="842.86" y="34.00" width="157.14" height="280.73" fill="none" stroke="#0d1117" stroke-width="2"/>
<text x="847.86" y="50.00" font-family="sans-serif" font-size="12" fill="#fff" stroke="#0d1117" stroke-width="3" paint-order="stroke">Android / CI 75%</text></g>
<g><title>dxvk-native: 1/3 (33%)</title>
<rect x="686.71" y="315.73" width="104.10" height="103.27" fill="#2ea043" stroke="#0d1117" stroke-width="0.5"/>
<rect x="790.81" y="315.73" width="104.10" height="103.27" fill="#6e7681" stroke="#0d1117" stroke-width="0.5"/>
<rect x="894.90" y="315.73" width="104.10" height="103.27" fill="#6e7681" stroke="#0d1117" stroke-width="0.5"/>
<rect x="685.71" y="314.73" width="314.29" height="105.27" fill="none" stroke="#0d1117" stroke-width="2"/>
<text x="690.71" y="330.73" font-family="sans-serif" font-size="12" fill="#fff" stroke="#0d1117" stroke-width="3" paint-order="stroke">dxvk-native 33%</text></g>
</svg>

# Source Engine
[![GitHub Actions Status](https://github.com/nillerusr/source-engine/actions/workflows/build.yml/badge.svg)](https://github.com/nillerusr/source-engine/actions/workflows/build.yml) [![GitHub Actions Status](https://github.com/nillerusr/source-engine/actions/workflows/tests.yml/badge.svg)](https://github.com/nillerusr/source-engine/actions/workflows/tests.yml)
 Discord: [![Discord Server](https://img.shields.io/discord/672055862608658432.svg)](https://discord.gg/hZRB7WMgGw)
 

Information from [wikipedia](https://wikipedia.org/wiki/Source_(game_engine)):

Source is a 3D game engine developed by Valve.
It debuted as the successor to GoldSrc with Half-Life: Source in June 2004,
followed by Counter-Strike: Source and Half-Life 2 later that year.
Source does not have a concise version numbering scheme; instead, it was released in incremental versions

Source code is based on TF2 2018 leak. Don't use it for commercial purposes.

This project is using waf buildsystem. If you have waf-related questions look https://waf.io/book

# Features:
- Android, OSX, FreeBSD, Windows, Linux( glibc, musl ) support
- Arm support( except windows )
- 64bit support
- Modern toolchains support
- Fixed many undefined behaviours
- Touch support( even on windows/linux/osx )
- VTF 7.5 support
- PBR support
- bsp v19-v21 support( bsp v21 support is partial, portal 2 and csgo maps works fine )
- mdl v46-49 support
- Removed useless/unnecessary dependencies
- Achivement system working without steam
- Fixed many bugs
- Serverbrowser works without steam

# Current tasks
- Rewrite materialsystem for OpenGL render
- dxvk-native support
- Elbrus port
- Bink audio support( for video_bink )

# How to Build?
- [Building instructions(EN)](https://github.com/nillerusr/source-engine/wiki/Source-Engine-(EN))
- [Building instructions(RU)](https://github.com/nillerusr/source-engine/wiki/Source-Engine-(RU))

# Support me
BTC: `bc1qnjq92jj9uqjtafcx2zvnwd48q89hgtd6w8a6na`

ETH: `0x5d0D561146Ed758D266E59B56e85Af0b03ABAF46`

XMR: `48iXvX61MU24m5VGc77rXQYKmoww3dZh6hn7mEwDaLVTfGhyBKq2teoPpeBq6xvqj4itsGh6EzNTzBty6ZDDevApCFNpsJ`
