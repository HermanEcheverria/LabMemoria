// Laboratorio - Proyecto Memoria
// Integrantes: Herman Echeverría | Pablo Morales | Máx Marroquín

#include <iostream>
#include <vector>
#include <string>
#include <algorithm>

#include <vector>
#include <string>
#include <algorithm>

class MemoryBlock {
public:
    int startAddress;
    int size;
    std::string fileID;

    MemoryBlock(int startAddress, int size, const std::string& fileID)
        : startAddress(startAddress), size(size), fileID(fileID) {}

    void assign() {
        std::cout << "Asignando bloque para " << fileID << std::endl;
    }

    void release() {
        std::cout << "Liberando bloque de " << fileID << std::endl;
    }
};

class MemoryManager {
public:
    int totalSize;
    int pageSize;
    std::vector<MemoryBlock> blocks;

    MemoryManager(int totalSize, int pageSize) : totalSize(totalSize), pageSize(pageSize) {}

    void loadFile(const std::string& fileName, int fileSize) {
        int requiredPages = (fileSize + pageSize - 1) / pageSize; // Redondeo hacia arriba
        int requiredSize = requiredPages * pageSize;

        int startAddress = 0;
        for (const auto& block : blocks) {
            startAddress = std::max(startAddress, block.startAddress + block.size);
        }

        // Verificar si hay suficiente memoria
        if (startAddress + requiredSize > totalSize) {
            std::cout << "No hay suficiente memoria para " << fileName << std::endl;
            return;
        }

        MemoryBlock newBlock(startAddress, requiredSize, fileName);
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

    void defragment() {
        std::cout << "Iniciando defragmentación..." << std::endl;
        std::sort(blocks.begin(), blocks.end(), [](const MemoryBlock& a, const MemoryBlock& b) {
            return a.startAddress < b.startAddress;
        });

        for (size_t i = 1; i < blocks.size(); ++i) {
            if (blocks[i].startAddress < blocks[i - 1].startAddress + blocks[i - 1].size) {
                int overlap = (blocks[i - 1].startAddress + blocks[i - 1].size) - blocks[i].startAddress;
                blocks[i].startAddress += overlap;
            }
        }
        std::cout << "Defragmentación completada." << std::endl;
    }
};

int main() {
    MemoryManager manager(1024, 64);
    manager.loadFile("archivo1.txt", 200);
    manager.loadFile("imagen1.png", 300);
    manager.deleteFile("archivo1.txt");
    manager.defragment();
    return 0;
}