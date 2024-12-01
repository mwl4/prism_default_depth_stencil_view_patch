Unofficial patch for Euro Truck Simulator 2 & American Truck Simulator >1.53

This repository tries to address issue which appears when you use Reshade with DirectX 11.

If your console and game.log is full of following error:
```
00:01:43.483 : <ERROR> [dx11] Error creating default depth stencil view (0x80070057)
00:01:43.483 : <ERROR> [dx11] Error creating default depth stencil view (0x80070057)
00:01:43.483 : <ERROR> [dx11] Error creating default depth stencil view (0x80070057)
00:01:43.483 : <ERROR> [dx11] Error creating default depth stencil view (0x80070057)
00:01:43.484 : <ERROR> [dx11] Error creating default depth stencil view (0x80070057)
00:01:43.484 : <ERROR> [dx11] Error creating default depth stencil view (0x80070057)
00:01:43.484 : <ERROR> [dx11] Error creating default depth stencil view (0x80070057)
```
and potentially 
then this patch is for you. The patch will not fix any other error.

## Download

Download is available in Releases: https://github.com/mwl4/prism_default_depth_stencil_view_patch/releases  
Just download proper version of `prism_default_depth_stencil_view_patch.dll` (according to your game version)

## Install

Copy paste `prism_default_depth_stencil_view_patch.dll` into `<Game installation directory>\bin\win_x64\plugins\`. If `plugins` directory does not exist, then simply create it.

Typically it will be:
- `C:\Program Files (x86)\Steam\steamapps\common\Euro Truck Simulator 2\bin\win_x64\plugins\`
- `C:\Program Files (x86)\Steam\steamapps\common\American Truck Simulator\bin\win_x64\plugins\`
- `D:\SteamLibrary\steamapps\common\Euro Truck Simulator 2\bin\win_x64\plugins\`
- `D:\SteamLibrary\steamapps\common\American Truck Simulator\bin\win_x64\plugins\`

You can also go to Steam, right click on your game -> `Properties` -> `Installed Files` -> `Browse`  
Then Windows Explorer will be opened in game installation directory, go to `bin`, then `win_x64`, then `plugins` (if does not exist, create it), put `prism_default_depth_stencil_view_patch.dll` in here.

Run your game.

In `Documents/Euro Truck Simulator 2/game.log` or `Documents/American Truck Simulator/game.log` there should be:
```
00:00:05.909 : loading 'prism_default_depth_stencil_view_patch' '[...]/bin/win_x64/plugins/prism_default_depth_stencil_view_patch.dll'
00:00:07.203 : [prism_default_depth_stencil_view_patch] Patcher originally made for game version: 1.XX.
00:00:07.203 : [prism_default_depth_stencil_view_patch] Successfully patched game code at: 00XXXXXXXXXXXXX
```

`<ERROR> [dx11] Error creating default depth stencil view (0x80070057)` error should be gone now.

Enjoy
