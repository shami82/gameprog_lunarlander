# ------------------------------------------------------------
#  Detect the operating system once, early in the file
# ------------------------------------------------------------
UNAME_S := $(shell uname -s)

# ------------------------------------------------------------
#  Source files
# ------------------------------------------------------------
SRCS = main.cpp CS3113/*.cpp  # entities

# ------------------------------------------------------------
#  Target name
# ------------------------------------------------------------
TARGET = raylib_app

# ------------------------------------------------------------
#  Compiler / basic flags
# ------------------------------------------------------------
CXX      = g++
CXXFLAGS = -std=c++11

# ------------------------------------------------------------
#  Raylib configuration (pkg‑config works on macOS too)
# ------------------------------------------------------------
RAYLIB_CFLAGS = $(shell pkg-config --cflags raylib)
RAYLIB_LIBS   = $(shell pkg-config --libs   raylib)

# ------------------------------------------------------------
#  Platform‑specific settings
# ------------------------------------------------------------
# ---------- macOS ----------
ifeq ($(UNAME_S), Darwin)
    # macOS configuration
    CXXFLAGS += -arch x86_64 $(RAYLIB_CFLAGS)
    LIBS = $(RAYLIB_LIBS) -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo
    EXEC = ./$(TARGET)
else ifneq (,$(findstring MINGW,$(UNAME_S)))
    # Windows configuration (assumes raylib in C:/raylib)
    CXXFLAGS += -IC:/raylib/include
    LIBS = -LC:/raylib/lib -lraylib -lopengl32 -lgdi32 -lwinmm
    TARGET := $(TARGET).exe
    EXEC = ./$(TARGET)
else
    # Linux/WSL fallback
    CXXFLAGS +=
    LIBS = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
    EXEC = ./$(TARGET)
endif

# ------------------------------------------------------------
#  Build rule
# ------------------------------------------------------------
$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) -o $@ $(SRCS) $(LIBS)

# ------------------------------------------------------------
#  Convenience targets
# ------------------------------------------------------------
.PHONY: clean run

clean:
	@rm -f $(TARGET) $(TARGET).exe

run: $(TARGET)
	$(EXEC)