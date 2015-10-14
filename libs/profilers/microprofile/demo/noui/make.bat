cl demo_noui.cpp /MT %* -I ..\.. fakework.cpp -DMICROPROFILE_UI=0 -DMICROPROFILE_WEBSERVER=1 -DMICROPROFILE_GPU_TIMERS=0 ws2_32.lib advapi32.lib winmm.lib 
