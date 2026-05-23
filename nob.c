#define NOB_IMPLEMENTATION 
#include "nob.h"
#include "src/assets.h"

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

#define for_range_i(count_) for (size_t i = 0; i < (count_); i++)
#define for_range_j(count_) for (size_t j = 0; j < (count_); j++)
#define to_str(name) #name

bool make_baked_assets(const char *baked_file, const char **src_paths, size_t src_count);

int main(int argc, char **argv)
{
    NOB_GO_REBUILD_URSELF(argc, argv);

    bool do_run          = false;
    bool do_static       = false;
    bool do_debug        = false;
    bool do_assets_baked = false;
    
    const char *compiler = "gcc";

    for (size_t i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "-run") == 0)
            do_run = true;
        
        if (strcmp(argv[i], "-static") == 0)
            do_static = true;

        if (strcmp(argv[i], "-dbg") == 0)
            do_debug = true;
        
        if (strcmp(argv[i], "-assets-baked") == 0)
            do_assets_baked = true;

        if (strcmp(argv[i], "-cc") == 0)
            compiler = (i + 1) <= argc ? argv[i + 1] : "gcc";
    }

    if (do_assets_baked)
    {
        const char *src_paths[] = {
        #define X(name, type, file) (type == ASSET_TYPE_IMAGE) ? IMAGE_PATH(file) : SOUND_PATH(file),
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
    
    nob_cmd_append(&cmd, compiler);
    nob_cmd_append(&cmd, "-o", EXECUTABLE_NAME);
    nob_cmd_append(&cmd, "-I", INCLUDE_DIR);
    nob_cmd_append(&cmd, "src/main.c", "src/game.c");
    
    if (do_assets_baked) nob_cmd_append(&cmd, "-DASSETS_BAKED");

    // ---- WINDOWS ----------------------------------
    #ifdef _WIN32

    // Linking for SDL2main.a and other static libs 
    nob_cmd_append(&cmd, "-L", LIBS_DIR);
    nob_cmd_append(&cmd, "-lmingw32", "-lSDL2main", "-lSDL2", "-lSDL2_image");
    
    if (do_static) 
    {
        nob_cmd_append(&cmd,
            "-static", "-static-libgcc", "-Wl,--dynamicbase", 
            "-Wl,--nxcompat", "-Wl,--high-entropy-va", "-lm", "-luser32", "-lgdi32", "-lwinmm", 
            "-limm32", "-lole32", "-loleaut32", "-lshell32", "-lsetupapi", "-lversion", "-luuid"
        );
    }

    //nob_cmd_append(&cmd, "-mwindows");
    
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

bool make_baked_assets(const char *baked_file, const char **src_paths, size_t src_count)
{
    // Lists of sizes and offsets needed for loading assets 
    int32_t *asset_sizes_list   = (int32_t *)malloc(src_count * sizeof(int32_t));
    int32_t *asset_offsets_list = (int32_t *)malloc(src_count * sizeof(int32_t));
    uint8_t *data = NULL; // All data lives here
    
    int64_t total_size = 0;
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
    #define X(name, type_, file) \
    fprintf(fdst, "    [%s] = {.path = \"%s\", .type = %s, .size = %d, .offset = %d},\n", \
        to_str(ASSET_##name), (type_ == ASSET_TYPE_IMAGE) ? IMAGE_PATH(file) : file, \
        to_str(type_), asset_sizes_list[ASSET_##name], asset_offsets_list[ASSET_##name]);
        ASSET_XLIST
    #undef X
    
    fprintf(fdst, "};\n");
    fprintf(fdst, "\nconst uint8_t assets_baked_data[] = {\n");

    // The array has 20 bytes per line 
    int32_t leftover_count = total_size % 20;

    for (int64_t i = 0; i <= (total_size - leftover_count); i += 20)
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

    for (int64_t i = (total_size - leftover_count); i < total_size; i++)
    {
        if (i == (total_size - leftover_count)) fprintf(fdst, "    "); 
        
        fprintf(fdst, "0x%02x", data[i]);
        if ((i + 1) < total_size) fprintf(fdst, ", "); 
        if ((i + 1) == total_size) fprintf(fdst, "\n");
    }

    fprintf(fdst, "};\n");
    fprintf(fdst, "#endif // _ASSETS_BAKED_H\n");

    nob_log(NOB_INFO, "Sizeof: %.2f MB, %.2f KB, %d BYTES", 
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