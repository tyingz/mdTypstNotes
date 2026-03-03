# Variables
CXX = g++
CXXFLAGS = -Wall -std=c++17
LIBS = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
    
# Nombre del ejecutable
TARGET = main
    
# Archivos fuente
SRC = main.cpp
    
# Regla principal
all: $(TARGET)
    
$(TARGET): $(SRC)
	$(CXX) $(SRC) -o $(TARGET) $(CXXFLAGS) $(LIBS)
    
# Limpiar archivos binarios
clean:
	rm -f $(TARGET)
    
# Compilar y ejecutar de una vez
run: all
	./$(TARGET)
