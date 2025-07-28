g++ src/*.cpp -D_WIN32_WINNT=0x0A00 -DWINVER=0x0A00 -static -Isrc -Iinclude -Llib -lssl -lcrypto -lcrypt32 -lraylib -lopengl32 -lgdi32 -lwinmm -lenet -lws2_32 -lwinmm -std=c++17 -o game.exe
PAUSE