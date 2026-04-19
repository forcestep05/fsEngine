CXX := clang++
CXX_FLAGS := -std=c++17 -O3
OUTPUT := -o game_engine_linux\

INCLUDES :=  -I./ -I./game_engine/src/thirdparty/ -I./SDL2 -I./SDL_image -I./SDL_mixer -I./SDL_ttf 

LUA_INCLUDES := -I./game_engine/src/thirdparty/LuaBridge/ -I./game_engine/src/thirdparty/lua/

BOX2D_INCLUDES := -I./game_engine/src/thirdparty/box2d/src/ -I./game_engine/src/thirdparty/box2d/

LIBDIRS := -lSDL2 -lSDL2_image -lSDL2_mixer -lSDL2_ttf -llua5.4

BOX2D_COMPILES := ./game_engine/src/thirdparty/box2d/src/collision/*.cpp ./game_engine/src/thirdparty/box2d/src/common/*.cpp ./game_engine/src/thirdparty/box2d/src/dynamics/*.cpp ./game_engine/src/thirdparty/box2d/src/rope/*.cpp

COMPILES := ./game_engine/src/*.cpp 

main:
	$(CXX) $(COMPILES) $(BOX2D_COMPILES) $(CXX_FLAGS) $(INCLUDES) $(LUA_INCLUDES) $(BOX2D_INCLUDES) $(LIBDIRS) $(OUTPUT)
	