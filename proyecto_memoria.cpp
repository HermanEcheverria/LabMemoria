// Laboratorio - Proyecto Memoria
// Integrantes: Herman Echeverría | Pablo Morales | Máx Marroquín

#include <iostream>
#include <vector>
#include <string>
#include <algorithm>

// bloque de memoria
class MemoryBlock {
public:
    int startAddress;  // Dirección de inicio
    int size;          // Tamaño
    std::string fileID; // Identificador del archivo

    // Constructor
    MemoryBlock(int startAddress, int size, const std::string& fileID)
        : startAddress(startAddress), size(size), fileID(fileID) {}

    void assign() {
        std::cout << "Asignando bloque para " << fileID << std::endl;
    }

    void release() {
        std::cout << "Liberando bloque de " << fileID << std::endl;
    }
};

// gestionar la memoria
class MemoryManager {
public:
    int totalSize;  
    int pageSize;   
    std::vector<MemoryBlock> blocks;

    // Constructor
    MemoryManager(int totalSize, int pageSize) : totalSize(totalSize), pageSize(pageSize) {}

    void loadFile(const std::string& fileName, int fileSize) {
        int startAddress = 0;
        for (const auto& block : blocks) {
            startAddress = std::max(startAddress, block.startAddress + block.size);
        }

        MemoryBlock newBlock(startAddress, fileSize, fileName);
        newBlock.assign();
        blocks.push_back(newBlock);
    }

    void deleteFile(const std::string& fileID) {
        auto it = std::remove_if(blocks.begin(), blocks.end(), [&fileID](const MemoryBlock& block) {
            return block.fileID == fileID;
        });

        if (it != blocks.end()) {
            blocks.erase(it, blocks.end());
            std::cout << "Archivo " << fileID << " eliminado.\n";
        } else {
            std::cout << "Archivo " << fileID << " no encontrado.\n";
        }
    }
};

int main() {
    MemoryManager manager(1024, 64);

    manager.loadFile("archivo1.txt", 200);
    manager.loadFile("imagen1.png", 300);

    manager.deleteFile("archivo1.txt");

    return 0;
}
