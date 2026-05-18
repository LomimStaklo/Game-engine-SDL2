#define NOB_IMPLEMENTATION 
#include "nob.h"

// ---- WINDOWS ----------------------------------
#ifdef _WIN32
#define EXECUTABLE_NAME "SKF.exe"    
#define LIBS_DIR        "src/libs"
#define INCLUDE_DIR     "src/include"
#define ASSETS_DIR      "assets"
#define PATH_SLASH      "\\"

// ---- LINUX ------------------------------------
#else 
#define EXECUTABLE_NAME "SKF"   
#define INCLUDE_DIR     "src/include"
#define ASSETS_DIR      "assets"
#define PATH_SLASH      "/"
#endif

int main(int argc, char **argv)
{
    NOB_GO_REBUILD_URSELF(argc, argv);

    bool do_run          = false;
    bool do_static       = false;
    bool do_debug        = false;
    
    const char *compiler = "gcc";

    for (size_t i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "-run") == 0)
            do_run = true;
        
        if (strcmp(argv[i], "-static") == 0)
            do_static = true;

        if (strcmp(argv[i], "-dbg") == 0)
            do_debug = true;
        
        if (strcmp(argv[i], "-cc") == 0)
            compiler = (i + 1) <= argc ? argv[i + 1] : "gcc";
    }

    Nob_Cmd cmd = {0};
    
    nob_cmd_append(&cmd, compiler);
    nob_cmd_append(&cmd, "-o", EXECUTABLE_NAME);
    nob_cmd_append(&cmd, "-I", INCLUDE_DIR);
    nob_cmd_append(&cmd, "src/main.c", "src/game.c");
    
    // ---- WINDOWS ----------------------------------
    #ifdef _WIN32

    // Linking static libs  
    nob_cmd_append(&cmd, "-L", LIBS_DIR);
    
    if (do_static) 
    {
        nob_cmd_append(&cmd, "-lmingw32", "-lSDL2main", "-lSDL2", "-lSDL2_image");
        nob_cmd_append(&cmd,
            "-static", "-static-libgcc", "-Wl,--dynamicbase", 
            "-Wl,--nxcompat", "-Wl,--high-entropy-va", "-lm", "-luser32", "-lgdi32", "-lwinmm", 
            "-limm32", "-lole32", "-loleaut32", "-lshell32", "-lsetupapi", "-lversion", "-luuid"
        );
    } else
    {
        nob_cmd_append(&cmd, "-lmingw32", "-lSDL2main", "-lSDL2", "-lSDL2_image");
    }

    nob_cmd_append(&cmd, "-mwindows");
    
    // ---- LINUX ------------------------------------
    #else 
    if (do_static) 
    {
        nob_log(NOB_ERROR, "No support for Linux static linking");
        return EXIT_FAILURE;
    }
    
    nob_cmd_append(&cmd, "-lSDL2", "-lSDL2_image", "-lm");
    #endif

    // --- FLAGS --------------------------------------------
    nob_cmd_append(&cmd, 
        "-std=c99", "-march=native", "-O3", (do_debug) ? "-g" : "-s",
        "-pedantic", "-fstack-protector", 
        "-Wall", "-Wextra", "-Wshadow","-Wswitch-enum", "-Wconversion", "-Wsign-conversion", 
        "-Wdouble-promotion", "-Wundef", "-Wuninitialized",  "-Wnull-dereference","-Wno-#warnings",
        "-Wformat=2", "-Wfloat-equal", "-Wcast-qual", "-Wcast-align", 
        "-Wmissing-prototypes", "-Wstrict-prototypes", "-Wstrict-aliasing=2", 
        "-Warray-bounds"
    );
    
    clock_t start = clock();
    if (!nob_cmd_run_sync(cmd)) return EXIT_FAILURE;
    nob_log(NOB_INFO, "Comptime: %.2fs", (double)(clock() - start) / CLOCKS_PER_SEC);
    
    if (do_run)
    {
        Nob_Cmd run_cmd = {0};
        nob_cmd_append(&run_cmd, "." PATH_SLASH EXECUTABLE_NAME);
        
        if (!nob_cmd_run_sync(run_cmd)) return EXIT_FAILURE;
    }
    
    return EXIT_SUCCESS;
}
