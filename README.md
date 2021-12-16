# YAMA GameEngine
Game Engine  project by Martynas Antipenkovas for Advanced Game programming module.

PLESAE HAVE LATEST VERSION OF VS2019 OTHERWISE THIS PROJECT WILL NOT BUILD

All of this was run with 10.0.19041.0 Windows SDK.

Built using C++20 with /latest flag. Tested to work in Visual Studio 2019 16.11.7 and Visual Studio 2022 17 Preview 1.
Since it's using the /latest flag it may be difficult to know if this is the correct version. The C++ language standard options in the project properties should contain the "C++20" flag.

Sometimes issues like "Frame not in module", (or "Guard_dipatch.asm not faund") can happen when building the application after some minor changes, usually this is solved by just rebuilding the engine.
Error containing "IMG2" message will disapear upon building again without the need to clean solution first.
Origin of these issues is unknown and they could not be steadily reproduced (probably something relating to built modules).

This repository also contains an C# based WEB API. It was built using Visual Studio  2019 16.11.7 and depends on the ASP.NET workload.

After some time the Web App falls asleep so the first connection usually takes a bit to get working. There's an option in Azure portal to disable this, but it is available to standard plan (while this uses student plan)

Profiling is enabled with PROFILING preprocessing command, that has to be added via "Project Properties" > "C++" > "Preprocessor". #DEFINE will not work as modules do not allow macros to be passed from one to another.
The PROFILING preprocessor definition is already added to the debug versions of Game and Editor configurations.

It will take a while for system symbols to load. Beware.

In order to test the game select the "Game Debug" in the configuration manager (Where you would usually select Release/Debug configuration).
("Game Release" configuration does not support the score uploading due to internal library issues).
In order to test the editor select "Editor Release".

x86 Configuration has its toolset set to v143, which is for Visual Studio 2021.
In case of needing to change the toolset, it can be changed via project settings to v142 (VS2019).
However, x64 is the platform that this project was designed and developed on. So it must be set to that.
