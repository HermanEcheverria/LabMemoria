// Laboratorio - Proyecto Memoria
// Integrantes: Herman Echeverría | Pablo Morales | Máx Marroquín

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <queue>
#include <climits>
#include <cassert>

enum FileType {
    TEXT,
    BINARY
};

enum AllocationPolicy {
    BEST_FIT,
    WORST_FIT
};

class MemoryBlock {
public:
    int startAddress;
    int size;
    std::string fileID;
    std::string content;

    MemoryBlock(int startAddress, int size, const std::string& fileID, const std::string& content)
        : startAddress(startAddress), size(size), fileID(fileID), content(content) {}

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
    AllocationPolicy policy;
    std::vector<MemoryBlock> blocks;
    std::queue<MemoryBlock> pageQueue;

    MemoryManager(int totalSize, int pageSize, AllocationPolicy policy)
        : totalSize(totalSize), pageSize(pageSize), policy(policy) {}

    void loadFile(const std::string& fileName, FileType fileType) {
        std::ifstream file;

        if (fileType == TEXT) {
            file.open(fileName, std::ios::in);
        } else {
            file.open(fileName, std::ios::in | std::ios::binary);
        }

        if (!file) {
            std::cerr << "No se puede abrir el archivo: " << fileName << std::endl;
            return;
        }

        std::string content = std::string((std::istreambuf_iterator<char>(file)),
                              std::istreambuf_iterator<char>());
        int fileSize = content.size();
        int requiredPages = (fileSize + pageSize - 1) / pageSize;
        int requiredSize = requiredPages * pageSize;

        int startAddress = findStartAddress(requiredSize);

        if (startAddress + requiredSize > totalSize) {
            std::cout << "No hay suficiente memoria para " << fileName << std::endl;
            return;
        }

        MemoryBlock newBlock(startAddress, requiredSize, fileName, content);
        newBlock.assign();
        blocks.push_back(newBlock);
        pageQueue.push(newBlock);
    }

    int findStartAddress(int requiredSize) {
        if (policy == BEST_FIT) {
            return findBestFit(requiredSize);
        } else {
            return findWorstFit(requiredSize);
        }
    }

    int findBestFit(int requiredSize) {
        int bestSize = INT_MAX;
        int bestStart = 0;

        for (const auto& block : blocks) {
            int gap = (block.startAddress - bestStart) - requiredSize;
            if (gap >= 0 && gap < bestSize) {
                bestSize = gap;
                bestStart = block.startAddress;
            }
        }
        return bestStart;
    }

    int findWorstFit(int requiredSize) {
        int worstSize = -1;
        int worstStart = 0;

        for (const auto& block : blocks) {
            int gap = (block.startAddress - worstStart) - requiredSize;
            if (gap > worstSize) {
                worstSize = gap;
                worstStart = block.startAddress;
            }
        }
        return worstStart;
    }

    void deleteFile(const std::string& fileID) {
        auto it = std::remove_if(blocks.begin(), blocks.end(), [&fileID](const MemoryBlock& block) {
            return block.fileID == fileID;
        });

        if (it != blocks.end()) {
            it->release();  
            blocks.erase(it, blocks.end());
            std::cout << "Archivo " << fileID << " eliminado.\n";
        } else {
            std::cout << "Archivo " << fileID << " no encontrado.\n";
        }
    }

    void overwriteFile(const std::string& fileID, const std::string& newContent) {
        for (auto& block : blocks) {
            if (block.fileID == fileID) {
                block.content = newContent;
                std::cout << "Archivo " << fileID << " sobrescrito.\n";
                return;
            }
        }
        std::cout << "Archivo " << fileID << " no encontrado para sobrescribir.\n";
    }
};

void testBestFitAllocation() {
    MemoryManager manager(1024, 64, BEST_FIT);
    manager.loadFile("testFile1.txt", TEXT);
    manager.loadFile("testFile2.txt", TEXT);
    assert(manager.blocks[0].startAddress <= manager.blocks[1].startAddress && "Best Fit allocation failed");
}

void testWorstFitAllocation() {
    MemoryManager manager(1024, 64, WORST_FIT);
    manager.loadFile("testFile1.txt", TEXT);
    manager.loadFile("testFile2.txt", TEXT);
    assert(manager.blocks[0].startAddress <= manager.blocks[1].startAddress && "Worst Fit allocation failed");
}

void testFIFOPagination() {
    MemoryManager manager(1024, 64, BEST_FIT);
    manager.loadFile("testFile1.txt", TEXT);
    manager.loadFile("testFile2.txt", TEXT);
    manager.pageQueue.pop();  
    assert(manager.pageQueue.size() == 1 && "FIFO Pagination failed");
}

int main() {
    // Ejecutando 
    testBestFitAllocation();
    testWorstFitAllocation();
    testFIFOPagination();
    std::cout << "Todas las pruebas pasaron con éxito." << std::endl;

    MemoryManager manager(1024, 64, BEST_FIT);
    manager.loadFile("textFile.txt", TEXT);  
    manager.loadFile("imageFile.png", BINARY);  

    manager.deleteFile("textFile.txt");
    manager.overwriteFile("imageFile.png", "Nuevo contenido");

}
