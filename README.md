﻿Crawler
=======

- Crawler is a quick prototiping platform for Windows and Visual Studio (C++11).
- Crawler is just a premake script + many common libraries ready to use.
- You just put sources and headers into any `apps/` subfolder as you like.
- Executing `crawler.bat` will
  - Scan your disk structure crawling for headers and sources. Automatically.
  - Append `include/` paths to the solution include paths. Automatically.
  - Append sources to the solution. Automatically.
  - Create a new application in the solution.  Automatically.
  - Create/delete projects as needed. Automatically.
  - Refresh your solution. Automatically.
  - Compile and link statically everything. It requires you to press ´F5´ on VS.
- I have collected many samples from many repos to showcase the build tech. Feel free to browse the [apps/ folder](apps). You can compile them with the `.sln` file generated above. 
- And that is all (hopefully). The following libraries are already provided.

## Dependencies included
- [x] Animations: [spine-runtime](http://es.esotericsoftware.com/)
- [x] Audio: [soloud](http://soloud-audio.com/)
- [x] Data: [json11](https://github.com/dropbox/json11), [pugixml](http://pugixml.org/), [stb](https://github.com/nothings/stb), [nanosvg](https://github.com/memononen/nanosvg)
- [x] Databases: [sqlite3](https://www.sqlite.org/)
- [x] Dialogs: [nfd](https://github.com/mlabbe/nativefiledialog)
- [x] Frameworks: [SDL2](https://www.libsdl.org/), [GLFW3](http://www.glfw.org/), [SFML2](http://www.sfml-dev.org/), [freeglut](http://freeglut.sourceforge.net/), [glwt](https://github.com/rikusalminen/glwt), [sketch2d](https://github.com/island-org/island/blob/master/examples/01-processing/sketch2d.h) (*)
- [x] Extensions: [gl3w](https://github.com/skaslev/gl3w), [glew](http://glew.sourceforge.net/), [glxw](https://github.com/rikusalminen/glxw)
- [x] Input: [ois](http://sourceforge.net/projects/wgois/)
- [x] Math: [eigen](http://eigen.tuxfamily.org/), [glm](http://glm.g-truc.net/0.9.7/index.html) (*)
- [x] Meshes: [assimp](http://assimp.sourceforge.net/)
- [x] Network: [enet](http://enet.bespin.org/)
- [x] Physics: [bullet2](https://github.com/bulletphysics/bullet3), [bullet3](https://github.com/bulletphysics/bullet3) (*)
- [x] Profilers: [microprofile](https://bitbucket.org/jonasmeyer/microprofile), [remotery](https://github.com/Celtoys/Remotery) (*)
- [x] Rendering: [nanovg](https://github.com/memononen/nanovg), [libtess2]()
- [x] Scripting: [lua](http://www.lua.org/)
- [x] User interfaces: [imgui](https://github.com/ocornut/imgui)
- [x] [*] Yes, all included. Stick to the one you like the most :)

## Repository structure
- :open_file_folder:`/`, README, LICENSES and main script.
- :open_file_folder:`apps/`, where all your applications are (a folder per executable).
- :open_file_folder:`libs/`, where all the tech love/hate is. Sorted by context.
- :open_file_folder:`patches/`, where code patches are located (if needed).

## Install
```bash
git clone https://github.com/r-lyeh/crawler && cd crawler && crawler
```

## Updates
```bash
git pull && crawler
```

## F.A.Q.
- Why?
  - It is always a pain to set up, compile and test most of the projects I find on the internet. Specially on Windows. I just want to drop sources on a folder and give them a try. Without messing with dependencies, include paths, libs and makefiles.
- How to add my new app/program to the solution?
  - Create a new subfolder in `apps/`.
  - Put your awesome game in there, and execute `crawler.bat`.
  - All your game source files will be scanned and added into solution.
  - All your game header paths will be scanned and added into solution.
  - Check that `.prj/crawler.sln` solution is already updated. Profit.
- How to clean the project/disk tree?
  - Execute `crawler clean`.
- Why there are no git submodules for each library?
  - Ease of use. Newbies can just download the whole repository as a `zip` file. And everything should be there already. Additionally, it takes git a while (ages!) to initialize & download everything when there are many submodules already in the repository.
- Can you add [xxx] library?
  - Probably, as long as the library is widely supported / useful for anyone.
- Can you add [xxx] scripting language?
  - Very probably, as long as it is already located in the [Scriptorium repository](https://github.com/r-lyeh/Scriptorium).
- Does it compile on linux or osx?
  - Support should be mostly in, but I cannot test it. Pull-requests anyone?
- Does it compile on android or ios?
  - Nope. Pull-requests anyone?
- What is the license for the repo?
  - Premake script is Public Domain. 
  - For every library in use here, check [LICENSE file](LICENSE).
  - For every sample in `apps/` folder, check each folder.
- What about L/GPL licenses?
  - There is no L/GPL software in the repository.
- What is the build model (trade-off) for the solution configurations ?
  - Everything is linked statically, so you can redistribute your .exe more easily. 
  - Also check the following directives/compilation flags table:


▼targets\defines▶ | #HAS_DEBUGINFO | #HAS_OPTIMIZATIONS | #MASTER
--- | --- | --- | --- | ---
**DEBUG** | :heavy_check_mark::heavy_check_mark::heavy_check_mark: | :heavy_multiplication_x: | :heavy_multiplication_x:
**DEBUGOPT** | :heavy_check_mark::heavy_check_mark: | :heavy_check_mark: | :heavy_multiplication_x:
**RELEASE**/**NDEBUG** | :heavy_check_mark: | :heavy_check_mark::heavy_check_mark: | :heavy_multiplication_x: 
**SHIPPING** | :heavy_multiplication_x: | :heavy_check_mark::heavy_check_mark::heavy_check_mark: | :heavy_check_mark:


## Changelog
- v0.0.0 (2015/10/16): Initial commit
