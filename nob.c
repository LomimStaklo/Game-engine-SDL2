#define NOB_IMPLEMENTATION 
#include "nob.h"
#include "src/assets.h"

#define LIBS_DIR        "src/libs"
#define INCLUDE_DIR     "src/include"

// ---- WINDOWS ----------------------------------
#ifdef _WIN32
#define COMPILER        "gcc"
#define EXECUTABLE_NAME "SKF.exe"    
#define PATH_SLASH      "\\"
#else 
// ---- LINUX ------------------------------------
#define COMPILER        "cc"
#define EXECUTABLE_NAME "SKF"
#define PATH_SLASH      "/"
#endif

typedef enum platform_t 
{
    PLATFORM_WIN32 = 0,
    PLATFORM_LINUX, // Aka. Posix
    PLATFORM_WEB,
#ifdef _WIN32
    PLATFORM_NATIVE = PLATFORM_WIN32,
#else // _WIN32
    PLATFORM_NATIVE = PLATFORM_LINUX,
#endif // Linux
} platform_t;

#define for_range_i(count_) for (size_t i = 0; i < (count_); i++)
#define for_range_j(count_) for (size_t j = 0; j < (count_); j++)
#define TO_STR(name) #name

void append_flags(Nob_Cmd *cmd, bool do_dbg_flag);
bool make_baked_assets(const char *baked_file, const char **src_paths, size_t src_count);

int main(int argc, char **argv)
{
    NOB_GO_REBUILD_URSELF(argc, argv);

    bool do_run          = false;
    bool do_static       = false;
    bool do_debug        = false;
    bool do_assets_baked = false;

    platform_t platform  = PLATFORM_NATIVE; 
    const char *compiler = COMPILER;

    // ---- USER ARGS -------------------------------------------------------------
    for (size_t i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "-run") == 0)          do_run = true;
        if (strcmp(argv[i], "-static") == 0)       do_static = true;
        if (strcmp(argv[i], "-dbg") == 0)          do_debug = true;
        if (strcmp(argv[i], "-assets-baked") == 0) do_assets_baked = true;

        if (strcmp(argv[i], "-platform") == 0)
        {
            const char *target_str = (i + 1) <= argc ? argv[i + 1] : "native";

            if      (strcmp(target_str, "native") == 0) platform = PLATFORM_NATIVE;   
            else if (strcmp(target_str, "linux") == 0)  platform = PLATFORM_LINUX;   
            else if (strcmp(target_str, "win32") == 0)  platform = PLATFORM_WIN32;   
            else if (strcmp(target_str, "web") == 0)    platform = PLATFORM_WEB;

            if (platform == PLATFORM_WEB) compiler = "emcc";
        }

        if (strcmp(argv[i], "-cc") == 0)
            compiler = (i + 1) <= argc ? argv[i + 1] : COMPILER;

        if (strcmp(argv[i], "-help") == 0 || strcmp(argv[i], "--help") == 0)
        {
            printf( 
                " -run                       Runs the executable when it gets compiled.\n"
                " -assets-baked              Compiles the assets into the binary.\n"
                " -platform <platform>       Target a platform (default: 'native'): 'win32', 'linux', 'web'.\n"
                " -cc <compiler>             Change the compiler used but with same flags.\n"
                " -static                    Links the game statically instead of dynamically.\n"
                " -dbg                       Compile with debug info.\n"
                " -help, --help              Brings up this.\n\n"
            );

            return EXIT_SUCCESS;
        }
    }

    // ---- ASSET BAKING ----------------------------------------------------------
    if (do_assets_baked)
    {
        const char *src_paths[] = 
        {
        #define X(name, type, path) path,
            ASSET_XLIST
        #undef X
        };

        bool success = make_baked_assets("src/assets_baked.h", src_paths, NOB_ARRAY_LEN(src_paths));
        if (success == false)
        {
            nob_log(NOB_ERROR, "Coundn't bake the files");
            return EXIT_FAILURE;
        }
    }

    Nob_Cmd cmd = {0};
    
    switch (platform)
    {        
        // ---- WINDOWS --------------------------------------------------------------
        case PLATFORM_WIN32:
        {
            nob_cmd_append(&cmd, compiler);
            nob_cmd_append(&cmd, "-o", EXECUTABLE_NAME);
            nob_cmd_append(&cmd, "-I", INCLUDE_DIR);
            nob_cmd_append(&cmd, "src/main.c", "src/game.c");
            
            // Linking for SDL2main.a and other static libs  
            nob_cmd_append(&cmd, "-L", LIBS_DIR);
            nob_cmd_append(&cmd, "-lmingw32", "-lSDL2main", "-lSDL2", "-lSDL2_image");
            
            if (nob_file_exists("icon/resources.res"))
                nob_cmd_append(&cmd, "icon/resources.res");

            if (do_static) 
            {
                nob_cmd_append(&cmd,
                    "-static", "-static-libgcc", "-Wl,--dynamicbase", 
                    "-Wl,--nxcompat", "-Wl,--high-entropy-va", "-lm", "-luser32", "-lgdi32", "-lwinmm", 
                    "-limm32", "-lole32", "-loleaut32", "-lshell32", "-lsetupapi", "-lversion", "-luuid"
                );
            }
            
            if (do_assets_baked) nob_cmd_append(&cmd, "-DASSETS_BAKED");
            
            append_flags(&cmd, do_debug);
            //nob_cmd_append(&cmd, "-mwindows");
        } break;
        
        // ---- LINUX ----------------------------------------------------------------
        case PLATFORM_LINUX:
        {
            nob_cmd_append(&cmd, compiler);
            nob_cmd_append(&cmd, "-o", EXECUTABLE_NAME);
            nob_cmd_append(&cmd, "-I", INCLUDE_DIR);
            nob_cmd_append(&cmd, "src/main.c", "src/game.c");
            
            // For Linux system libary of SDL2 is used
            nob_cmd_append(&cmd, "-lSDL2", "-lSDL2_image", "-lm");

            if (do_static) 
            {
                nob_log(NOB_ERROR, "No support for Linux static linking");
                return EXIT_FAILURE;
            }
            
            if (do_assets_baked) nob_cmd_append(&cmd, "-DASSETS_BAKED");
            
            append_flags(&cmd, do_debug);
        } break;
        
        // ---- WEB ------------------------------------------------------------------
        case PLATFORM_WEB: 
        {
            if (!do_assets_baked) {
                nob_log(NOB_ERROR, "Must include '-assets-baked' flag to compile for web");
                return EXIT_FAILURE;
            } 
            
            // For some unknown wizard reason the emscripten is a .bat file on windows
            // On Windows the script invokes cmd.exe to start the process   
            #ifdef _WIN32
            nob_cmd_append(&cmd, "cmd.exe", "/c");
            #endif // _WIN32
    
            nob_cmd_append(&cmd, compiler);
            nob_cmd_append(&cmd, "-o", "index.html");
            nob_cmd_append(&cmd, "-I", INCLUDE_DIR); 
            nob_cmd_append(&cmd, "src/main.c", "src/game.c");
            nob_cmd_append(&cmd,
                "-s", "USE_SDL=2" ,
                "-s", "USE_SDL_IMAGE=2",
                "-s", "SDL2_IMAGE_FORMATS=[png,jpg]",
                "-s", "ALLOW_MEMORY_GROWTH=1",
                "-s", "ASYNCIFY",
                "-DASSETS_BAKED", "-O3"
            );
            
            // And the cmd.exe must exit 
            #ifdef _WIN32
            nob_cmd_append(&cmd, "&&", "exit");
            #endif // _WIN32
        } break;

        default: 
        {
            nob_log(NOB_ERROR, "Wrong platform lul");
            return EXIT_FAILURE;
        } break;
    }
    
    // ---- COMPILATION -----------------------------------------------------------
    time_t start = time(NULL);
    if (!nob_cmd_run_sync(cmd)) return EXIT_FAILURE;
    time_t end = time(NULL);

    nob_log(NOB_INFO, "Comptime: %d.0s", (int32_t)(end - start));
    
    // ---- RUN -------------------------------------------------------------------
    if (do_run)
    {
        Nob_Cmd run_cmd = {0};

        if (platform == PLATFORM_WEB)
        {
            const char *port = "8000";
            nob_log(NOB_INFO, "The '-run' flag, will start a python server on port %s", port);
            nob_cmd_append(&run_cmd, "python", "-m", "http.server", port);
        }
        else         
        {
            nob_cmd_append(&run_cmd, "." PATH_SLASH EXECUTABLE_NAME);
        }
        
        if (!nob_cmd_run_sync(run_cmd)) return EXIT_FAILURE;
    }
    
    return EXIT_SUCCESS;
}

bool make_baked_assets(const char *baked_file, const char **src_paths, size_t src_count)
{
    // Lists of sizes and offsets needed for loading assets 
    int32_t *asset_sizes_list   = (int32_t *)malloc(src_count * sizeof(int32_t));
    int32_t *asset_offsets_list = (int32_t *)malloc(src_count * sizeof(int32_t));
    uint8_t *data = NULL; // All data lives here
    
    size_t total_size = 0;
    // Iterate threw the list of game assets
    for_range_i(src_count)
    {
        FILE *fsrc = fopen(src_paths[i], "rb");
        if (!fsrc) 
        {
            nob_log(NOB_ERROR, "The file '%s' cant be open", src_paths[i]);
            return false;
        }

        nob_log(NOB_INFO, "Baking: '%s'", src_paths[i]);

        fseek(fsrc, 0, SEEK_END);
        int32_t size = ftell(fsrc);
        rewind(fsrc);

        // Asset size counting 
        asset_sizes_list[i] = size;
        total_size += size; 
        
        // Asset offset counting
        int32_t offset = 0;
        for_range_j(i) offset += asset_sizes_list[j];
        asset_offsets_list[i] = offset;
          
        data = (uint8_t *)realloc((void *)data, total_size);

        // Read the contents
        fread(data + asset_offsets_list[i], 1, size, fsrc);
        fclose(fsrc);
    }

    // Open file where the assets will be baked 
    FILE *fdst = fopen("src/assets_baked.h", "wb+");
    if (!fdst) 
    {
        nob_log(NOB_ERROR, "The path 'src/assets_baked.h' cant be open");
        return false;
    }

    // ---- CODE MAKING -----------------------------------------------------------------
    fprintf(fdst, 
        "#ifndef _ASSETS_BAKED_H\n"
        "#define _ASSETS_BAKED_H\n"
        "#include \"assets.h\"\n"
    );

    fprintf(fdst, "\nasset_t global_assets[ASSET_COUNT] = {\n");
    // I sugesst you dont look at this code  
    #define X(name, type_, path) \
    fprintf(fdst, "    [%s] = {.path = \"%s\", .type = %s, .size = %d, .offset = %d},\n", \
        TO_STR(ASSET_##name), path, \
        TO_STR(type_), asset_sizes_list[ASSET_##name], asset_offsets_list[ASSET_##name]);
        ASSET_XLIST
    #undef X
    
    fprintf(fdst, "};\n");
    fprintf(fdst, "\nstatic const uint8_t assets_baked_data[] = {\n");

    // The array has 20 bytes per line 
    size_t leftover_count = total_size % 20;

    for (size_t i = 0; i <= (total_size - leftover_count); i += 20)
    {
        fprintf(fdst,
            "    0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, "
            "0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x,\n",
            data[i + 0], data[i + 1], data[i + 2], data[i + 3], data[i + 4],
            data[i + 5], data[i + 6], data[i + 7], data[i + 8], data[i + 9],
            data[i + 10], data[i + 11], data[i + 12], data[i + 13], data[i + 14],
            data[i + 15], data[i + 16], data[i + 17], data[i + 18], data[i + 19]
        );
    }

    for (size_t i = (total_size - leftover_count); i < total_size; i++)
    {
        if (i == (total_size - leftover_count)) fprintf(fdst, "    "); 
        
        fprintf(fdst, "0x%02x", data[i]);
        if ((i + 1) < total_size) fprintf(fdst, ", "); 
        if ((i + 1) == total_size) fprintf(fdst, "\n");
    }

    fprintf(fdst, "};\n");
    fprintf(fdst, "#endif // _ASSETS_BAKED_H\n");

    nob_log(NOB_INFO, "Sizeof: %.2f MB, %.2f KB, %zu BYTES", 
        (double)total_size / (1024.0 * 1024.0), 
        (double)total_size / 1024.0,
        total_size
    );
    
    free(asset_sizes_list);
    free(asset_offsets_list);
    free(data);
    fclose(fdst);
    
    return true;
}

void append_flags(Nob_Cmd *cmd, bool do_dbg_flag)
{
    // --- FLAGS --------------------------------------------
    nob_cmd_append(cmd, 
        "-std=c99", "-O3", (do_dbg_flag) ? "-g" : "-s",
        "-pedantic", "-fstack-protector", 
        "-Wall", "-Wextra", "-Wshadow","-Wswitch-enum", "-Wconversion", "-Wsign-conversion", 
        "-Wdouble-promotion", "-Wundef", "-Wuninitialized", "-Wnull-dereference",
        "-Wformat=2", "-Wfloat-equal", "-Wcast-qual", "-Wcast-align", 
        "-Wmissing-prototypes", "-Wstrict-prototypes", "-Wstrict-aliasing=2", 
        "-Warray-bounds", "-Wno-#warnings"
    );
}
