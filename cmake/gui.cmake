# Include this to make the current target a Windows app (no console)

if (WIN32)
   # /ENTRY:mainCRTStartup keeps the same "main" function instead of requiring "WinMain"
   message("${TARGET} will create a GUI; disabling the console window")
   set(SUBSYSTEM_LINKER_OPTIONS "/SUBSYSTEM:WINDOWS")
else()
   set(SUBSYSTEM_LINKER_OPTIONS "-mwindows")
endif()

target_link_options(${TARGET} PRIVATE ${SUBSYSTEM_LINKER_OPTIONS})