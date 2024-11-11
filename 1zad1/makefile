# Имя исполняемого файла
TARGET = 1zad1.exe

# Исходные файлы
SOURCES = 1zad1.cpp

# Флаги компилятора
CXXFLAGS = -g -std=c++20

# Компилятор
CXX = g++

$(TARGET): $(SOURCES)
	g++ $(CXXFLAGS) $(SOURCES) -o $@

# Правило для очистки (удаления объектных файлов и исполняемого файла)
clean:
	rm -f $(OBJECTS) $(TARGET)

# Правило для очистки и пересборки
rebuild: clean $(TARGET)

.PHONY: clean rebuild #  Указывает, что clean и rebuild — это не имена файлов