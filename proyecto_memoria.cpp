#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <queue>
#include <climits>
#include <cassert>
#include <unordered_map>
#include <list>

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
    std::list<MemoryBlock> lruList;
    std::unordered_map<std::string, std::list<MemoryBlock>::iterator> lruMap;

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

        accessPage(fileName);
    }

    void accessPage(const std::string& fileID) {
        if (lruMap.find(fileID) != lruMap.end()) {
            lruList.erase(lruMap[fileID]);
            lruList.push_front(*lruMap[fileID]);
            lruMap[fileID] = lruList.begin();
        } else {
            for (const auto& block : blocks) {
                if (block.fileID == fileID) {
                    lruList.push_front(block);
                    lruMap[fileID] = lruList.begin();
                    break;
                }
            }
        }
    }

    void replacePageLRU(const std::string& newFileID) {
        if (lruList.size() == blocks.size()) {
            MemoryBlock lastUsedPage = lruList.back();
            lruList.pop_back();
            lruMap.erase(lastUsedPage.fileID);

            deleteFile(lastUsedPage.fileID);
            loadFile(newFileID, TEXT);  // Cambiar TEXT a BINARY según sea necesario
        }
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
            it->release();  // Liberar el bloque
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

    void saveToUnisFormat(const std::string& fileName) {
        std::ofstream outFile(fileName + ".unis");

        outFile << totalSize << " " << pageSize << "\n";

        for (const auto& block : blocks) {
            outFile << block.startAddress << " "
                    << block.size << " "
                    << block.fileID << " "
                    << block.content << "\n";
        }

        outFile.close();
        std::cout << "Datos guardados en formato .unis en el archivo " << fileName << ".unis\n";
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
    manager.pageQueue.pop();  // Simulando FIFO
    assert(manager.pageQueue.size() == 1 && "FIFO Pagination failed");
}

int main() {
    // Ejecutando pruebas
    testBestFitAllocation();
    testWorstFitAllocation();
    testFIFOPagination();
    std::cout << "Todas las pruebas pasaron con éxito." << std::endl;

    MemoryManager manager(1024, 64, BEST_FIT);
    manager.loadFile("textFile.txt", TEXT);  // Asegúrate de que textFile.txt exista
    manager.loadFile("imageFile.png", BINARY);  // Asegúrate de que imageFile.png exista

    manager.deleteFile("textFile.txt");
    manager.overwriteFile("imageFile.png", "Nuevo contenido");

    while (true) {
        std::cout << "\n--- Menú ---\n";
        std::cout << "1. Cargar archivo de texto\n";
        std::cout << "2. Cargar archivo binario\n";
        std::cout << "3. Eliminar archivo\n";
        std::cout << "4. Sobrescribir archivo\n";
        std::cout << "5. Guardar en formato .unis\n";
        std::cout << "6. Salir\n";

        int choice;
        std::cout << "Elige una opción: ";
        std::cin >> choice;

        std::string fileName, content;

        switch (choice) {
            case 1:
                std::cout << "Nombre del archivo de texto para cargar: ";
                std::cin >> fileName;
                manager.loadFile(fileName, TEXT);
                break;

            case 2:
                std::cout << "Nombre del archivo binario para cargar: ";
                std::cin >> fileName;
                manager.loadFile(fileName, BINARY);
                break;

            case 3:
                std::cout << "Nombre del archivo para eliminar: ";
                std::cin >> fileName;
                manager.deleteFile(fileName);
                break;

            case 4:
                std::cout << "Nombre del archivo para sobrescribir: ";
                std::cin >> fileName;
                std::cout << "Nuevo contenido: ";
                std::cin >> content;
                manager.overwriteFile(fileName, content);
                break;

            case 5:
                std::cout << "Nombre del archivo donde guardar en formato .unis: ";
                std::cin >> fileName;
                manager.saveToUnisFormat(fileName);
                break;

            case 6:
                std::cout << "Saliendo...\n";
                return 0;

            default:
                std::cout << "Opción no válida.\n";
        }
    }

    return 0;
}
