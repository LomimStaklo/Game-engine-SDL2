import os
import sys
from time import time

# ====== CONFIG ======= #
cc       = "gcc" # clang
src      = "src/main.c src/game.c src/systems.c"
out      = "main.exe"
flags    = "-Wall -Wextra -Wenum-compare -pedantic -Wshadow -std=c99 -s -fstack-protector" # -mwindows
lib_path = "src/lib"
inc_path = "src/include"
libs     = "-lmingw32 -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf"
# ====== CONFIG ======= #

def run_time(func) -> None:
    _start_t = time()
    func()
    duration =  time() - _start_t
    print(f"[i] {duration:.2f} sec")

def run_cmd(cmd: str, msg: str) -> None:
    print("[*]", msg, "...")
    if os.system(cmd): 
        print("[!]", msg,"failed")
        exit(1)

# ====== VARIABLES ======= # 
argc      = len(sys.argv)
cmd_comp  = f"{cc} -o {out} {src} -I {inc_path} -L {lib_path} {libs} {flags}"
cmd_run   = f".\\{out}"
do_run    = False
do_comp   = False
# ====== VARIABLES ======= # 

if argc <= 1:
    run_cmd(cmd_comp, "Compiling")
    run_cmd(cmd_run, "Running")
else:
    for i in range(1, argc):
        match sys.argv[i]:
            case "-comp": do_comp = True
            case "-run": do_run = True
            case _: print("[!]", "Unknown instrustion", sys.argv[i])

if do_comp: run_time(lambda: run_cmd(cmd_comp, "Compiling"))
if do_run: run_cmd(cmd_run, "Running")