CXX = g++
CXXFLAGS = -std=c++17 -g3 -Wall -I. -Isrc/app/encryptDecrypt -Isrc/app/fileHandling -Isrc/app/processes
LDFLAGS = -lstdc++fs  # Linking stdc++fs for older GCC versions

MAIN_TARGET = encrypt_decrypt
CRYPTION_TARGET = cryption

MAIN_SRC = main.cpp \
           src/processes/ProcessManagement.cpp \
           src/fileHandling/IO.cpp \
           src/fileHandling/ReadEnv.cpp \
           src/encryptDecrypt/Cryption.cpp

CRYPTION_SRC = src/encryptDecrypt/CryptionMain.cpp \
               src/encryptDecrypt/Cryption.cpp \
               src/fileHandling/IO.cpp \
               src/fileHandling/ReadEnv.cpp

MAIN_OBJ = $(MAIN_SRC:.cpp=.o)
CRYPTION_OBJ = $(CRYPTION_SRC:.cpp=.o)

all: $(MAIN_TARGET) $(CRYPTION_TARGET)

$(MAIN_TARGET): $(MAIN_OBJ)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

$(CRYPTION_TARGET): $(CRYPTION_OBJ)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(MAIN_OBJ) $(CRYPTION_OBJ) $(MAIN_TARGET) $(CRYPTION_TARGET)

.PHONY: clean all
