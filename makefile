cc = cl
libSDL = C:\dev\C\libs\SDL2-2.0.14\lib\x64
libSDL_image = C:\dev\C\libs\SDL2_image-2.0.5\lib\x64
includeSDL = C:\dev\C\libs\SDL2-2.0.14\include
includeSDL_image = C:\dev\C\libs\SDL2_image-2.0.5\include

all: snake.exe

snake.exe: src/*.c
	$(cc) /TC /Fo:objs/ /Fe:bin/snake.exe src/*.c /I $(includeSDL) /I $(includeSDL_image) /link /LIBPATH:$(libSDL) /LIBPATH:$(libSDL_image) SDL2.lib SDL2main.lib SDL2_image.lib shell32.lib /SUBSYSTEM:WINDOWS