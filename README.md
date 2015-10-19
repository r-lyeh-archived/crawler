﻿Crawler
=======

- Crawler is a quick prototiping platform for Windows and Visual Studio (C++11).
- Crawler is just a premake script + many common libraries ready to use.
- You just put sources and headers into any `apps/` subfolder as you like.
- Executing `crawler.bat` will
  - Scan your disk structure crawling for headers and sources. Automatically.
  - Create/delete projects as needed. Automatically.
  - Create new applications in the solution.  Automatically.
  - Refresh all projects into solution. Automatically.
  - Refresh all include paths for every project in the solution. Automatically.
  - Refresh all sources for every project in the solution. Automatically.
  - Rewrite all licenses found into LICENSE file. Automatically.
  - Compile and link statically everything.
- I have collected many samples from many repos to showcase the build tech. Feel free to browse the [apps/ folder](apps). You can compile them with the `.sln` file generated above. 
- And that is all (hopefully). The following libraries are already provided.

## Dependencies included
- [x] Animations: [spine-runtime](http://es.esotericsoftware.com/)
- [x] Audio: [soloud](http://soloud-audio.com/), [modplug](http://modplug-xmms.sourceforge.net/)
- [x] Data: [json11](https://github.com/dropbox/json11), [pugixml](http://pugixml.org/), [stb_image](https://github.com/nothings/stb), [nanosvg](https://github.com/memononen/nanosvg)
- [x] Databases: [sqlite3](https://www.sqlite.org/)
- [x] Dialogs: [native-file-dialog](https://github.com/mlabbe/nativefiledialog)
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
- [x] Text: [stb_truetype](https://github.com/nothings/stb), [fontstash](https://github.com/memononen/fontstash)
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
- How to compile from command-line?
  - Execute `crawler make`.
- Why there are no git submodules for each library?
  - Ease of use. Newbies can just download the whole repository as a `zip` file. And everything should be there already. Additionally, it takes git a while (ages!) to initialize & download everything when there are many submodules already in the repository.
- Can you add [xxx] library?
  - Probably, as long as the library is widely supported / useful for anyone.
- Can you add [xxx] scripting language?
  - Probably, as long as it is already located in the [Scriptorium repository](https://github.com/r-lyeh/Scriptorium).
- Does it compile on linux or osx?
  - Partial support already. Pull-requests anyone?
- Does it compile on android or ios?
  - Nope. Pull-requests anyone?
- What is the license for the repo?
  - Premake script is Public Domain. 
- What is the license for the libraries?
  - For every library in use here, check [LICENSE file](LICENSE).
- What is the license for the samples?
  - For every sample in `apps/` folder, check each folder separately.
- What about L/GPL?
  - There is no L/GPL software in the repository.
- What is the build model (trade-off) for the solution configurations ?
  - Everything is linked statically, so you can redistribute your .exe more easily. 
  - Also check the following directives/compilation flags table:


▼targets\defines▶ | #HAS_DEBUGINFO | #HAS_OPTIMIZATIONS | #MASTER
--- | --- | --- | --- | ---
**DEBUG** | (3):heavy_check_mark::heavy_check_mark::heavy_check_mark: | (0):heavy_multiplication_x: | (0):heavy_multiplication_x: 
**DEBUGOPT** | (2):heavy_check_mark::heavy_check_mark: | (1):heavy_check_mark: | (0):heavy_multiplication_x: 
**RELEASE**/**NDEBUG** | (1):heavy_check_mark: | (2):heavy_check_mark::heavy_check_mark: | (0):heavy_multiplication_x: 
**SHIPPING** | (0):heavy_multiplication_x:  | (3):heavy_check_mark::heavy_check_mark::heavy_check_mark: | (1):heavy_check_mark:

## Licenses (autogenerated; may be wrong)
- Check [LICENSE](LICENSE) file as well.
- [assimp](libs/mesh/assimp/LICENSE), BSD licensed.
- [boost](libs/mesh/assimp/code/BoostWorkaround/boost/LICENSE_1_0.txt), BOOST licensed.
- [bullet2](libs/physics/bullet2/COPYING.txt), ZLIB/LIBPNG licensed.
- [doc](libs/script/lua/doc/readme.html), MIT licensed.
- [eigen](libs/math/eigen/COPYING.BSD), BSD licensed.
- [enet](libs/network/enet/README.txt), MIT licensed.
- [freeglut](libs/frameworks/freeglut/COPYING), MIT licensed.
- [GL](libs/audio/soloud/demos/common/glew/GL/LICENSE.txt), MIT licensed.
- [glew](libs/extensions/glew/LICENSE.txt), MIT licensed.
- [glfw3](libs/frameworks/glfw3/COPYING.txt), ZLIB/LIBPNG licensed.
- [glm](libs/math/glm/copying.txt), MIT licensed.
- [glut](libs/mesh/assimp/samples/glut/README-win32.txt), PD licensed.
- [glwt](libs/frameworks/glwt/LICENSE), ZLIB/LIBPNG licensed.
- [glxw](libs/extensions/glxw/LICENSE), ZLIB/LIBPNG licensed.
- [herringbone](libs/data/stb/stb/data/herringbone/license.txt), PD licensed.
- [imgui](libs/ui/imgui/LICENSE), MIT licensed.
- [json11](libs/json/json11/LICENSE.txt), MIT licensed.
- [libmodplug](libs/audio/soloud/ext/libmodplug/COPYING), PD licensed.
- [libtess2](libs/render/libtess2/LICENSE.txt), SGI licensed.
- [nanosvg](libs/svg/nanosvg/LICENSE.txt), ZLIB/LIBPNG licensed.
- [nanovg](libs/render/nanovg/LICENSE.txt), ZLIB/LIBPNG licensed.
- [nativefiledialog](libs/ui/nativefiledialog/LICENSE), ZLIB/LIBPNG licensed.
- [poly2tri](libs/mesh/assimp/contrib/poly2tri/LICENSE), BSD licensed.
- [pugixml](libs/xml/pugixml/README.md), MIT licensed.
- [remotery](libs/profilers/remotery/LICENSE), APACHE2 licensed.
- [SDL2](libs/frameworks/SDL2/COPYING.txt), ZLIB/LIBPNG licensed.
- [SFML2](libs/frameworks/SFML2/license.txt), ZLIB/LIBPNG licensed.
- [sketch2d](libs/creative/sketch2d/LICENSE), MIT licensed.
- [soloud](libs/audio/soloud/LICENSE), ZLIB/LIBPNG licensed.
- [spine-lua](libs/animation/spine-runtimes/spine-lua/README.md), BSD licensed.
- [spine-runtimes](libs/animation/spine-runtimes/LICENSE), BSD licensed.
- [stb](libs/data/stb/stb/README.md), PD licensed.
- [tedsid2dump](libs/audio/soloud/src/tools/tedsid2dump/readme.txt), PD licensed.
- [tedsid](libs/audio/soloud/src/audiosource/tedsid/readme.txt), PD licensed.
- [win32](libs/mesh/assimp/contrib/zlib/win32/README-WIN32.txt), ZLIB/LIBPNG licensed.
- [zlib](libs/mesh/assimp/contrib/zlib/README), ZLIB/LIBPNG licensed.

## Changelog
- v0.0.1 (2015/10/19): Upgrade SoLoud; Add new samples; Fix script
- v0.0.0 (2015/10/16): Initial commit
