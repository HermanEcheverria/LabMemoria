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

enum FileType
{
    TEXT,
    BINARY
};

enum AllocationPolicy
{
    BEST_FIT,
    WORST_FIT,
    FIFO,
    LRU
};

class MemoryBlock
{
public:
    int startAddress;
    int size;
    std::string fileID;
    std::string content;

    MemoryBlock(int startAddress, int size, const std::string &fileID, const std::string &content)
        : startAddress(startAddress), size(size), fileID(fileID), content(content) {}

    void assign()
    {
        std::cout << "Asignando bloque para " << fileID << std::endl;
    }

    void release()
    {
        std::cout << "Liberando bloque de " << fileID << std::endl;
    }
};

class MemoryManager
{
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


void readFile(const std::string& fileID) {
    auto it = std::find_if(blocks.begin(), blocks.end(), [&fileID](const MemoryBlock& block) {
        return block.fileID == fileID;
    });
    if (it != blocks.end()) {
        std::cout << "Contenido del archivo " << fileID << ":\n";
        std::cout << it->content << std::endl;
    } else {
        std::cout << "Archivo " << fileID << " no encontrado.\n";
    }
}

void listAllBlocks() {
    std::cout << "Bloques actuales en memoria:\n";
    for(const auto& block : blocks) {
        std::cout << "Archivo: " << block.fileID << ", Dirección de inicio: " << block.startAddress << ", Tamaño: " << block.size << '\n';
    }
}

    void loadFile(const std::string &fileName, FileType fileType)
    {
        std::ifstream file;

        if (fileType == TEXT)
        {
            file.open(fileName, std::ios::in);
        }
        else
        {
            file.open(fileName, std::ios::in | std::ios::binary);
        }

        if (!file)
        {
            std::cerr << "No se puede abrir el archivo: " << fileName << std::endl;
            return;
        }

        std::string content = std::string((std::istreambuf_iterator<char>(file)),
                                          std::istreambuf_iterator<char>());
        int fileSize = content.size();
        int requiredPages = (fileSize + pageSize - 1) / pageSize;
        int requiredSize = requiredPages * pageSize;

        int startAddress = findStartAddress(requiredSize);

        if (startAddress + requiredSize > totalSize)
        {
            std::cout << "No hay suficiente memoria para " << fileName << std::endl;
            return;
        }

        MemoryBlock newBlock(startAddress, requiredSize, fileName, content);
        newBlock.assign();

        if (policy == BEST_FIT)
        {
            int bestStart = findBestFit(requiredSize);
            blocks.push_back(newBlock);
        }
        else if (policy == WORST_FIT)
        {
            int worstStart = findWorstFit(requiredSize);
            blocks.push_back(newBlock);
        }
        else if (policy == FIFO)
        {
            if (pageQueue.size() >= totalSize / pageSize)
            {
                FIFOPageReplacement(newBlock);
            }
            else
            {
                pageQueue.push(newBlock);
            }
        }
        else if (policy == LRU)
        {
            if (pageQueue.size() >= totalSize / pageSize)
            {
                LRUPageReplacement(newBlock);
            }
            else
            {
                pageQueue.push(newBlock);
            }
        }
        else
        {
            blocks.push_back(newBlock);
            pageQueue.push(newBlock);
        }

        if (blocks.size() > 1 && (float)blocks.size() / totalSize > 0.8)
        {
            defragment();
        }

        accessPage(fileName);
    }

    void accessPage(const std::string &fileID)
    {
        if (lruMap.find(fileID) != lruMap.end())
        {
            lruList.erase(lruMap[fileID]);
            lruList.push_front(*lruMap[fileID]);
            lruMap[fileID] = lruList.begin();
        }
        else
        {
            for (const auto &block : blocks)
            {
                if (block.fileID == fileID)
                {
                    lruList.push_front(block);
                    lruMap[fileID] = lruList.begin();
                    break;
                }
            }
        }
    }

    void divideAndStoreBlocks()
    {
        int numPages = totalSize / pageSize;
        for (int i = 0; i < numPages; ++i)
        {
            MemoryBlock newBlock(i * pageSize, pageSize, "", "");
            blocks.push_back(newBlock);
        }
    }

    void defragment()
    {
        std::sort(blocks.begin(), blocks.end(), [](const MemoryBlock &a, const MemoryBlock &b)
                  { return a.startAddress < b.startAddress; });

        std::vector<MemoryBlock> newBlocks;
        int currentAddress = 0;

        for (const auto &block : blocks)
        {
            if (block.startAddress != currentAddress)
            {
                int gapSize = block.startAddress - currentAddress;
                MemoryBlock newGap(currentAddress, gapSize, "", "");
                newBlocks.push_back(newGap);
                currentAddress += gapSize;
            }
            newBlocks.push_back(block);
            currentAddress += block.size;
        }

        blocks = newBlocks;
    }

    void deleteBlock(int startAddress)
    {
        auto it = std::remove_if(blocks.begin(), blocks.end(), [startAddress](const MemoryBlock &block)
                                 { return block.startAddress == startAddress; });

        if (it != blocks.end())
        {
            it->release(); // Liberar el bloque
            blocks.erase(it, blocks.end());
            std::cout << "Bloque en dirección " << startAddress << " eliminado.\n";
        }
        else
        {
            std::cout << "Bloque en dirección " << startAddress << " no encontrado.\n";
        }
    }

    void overwriteBlock(int startAddress, const std::string &newContent)
    {
        auto it = std::find_if(blocks.begin(), blocks.end(), [startAddress](const MemoryBlock &block)
                               { return block.startAddress == startAddress; });

        if (it != blocks.end())
        {
            it->content = newContent;
            std::cout << "Bloque en dirección " << startAddress << " sobrescrito.\n";
        }
        else
        {
            std::cout << "Bloque en dirección " << startAddress << " no encontrado para sobrescribir.\n";
        }
    }

    void FIFOPageReplacement(const MemoryBlock &newBlock)
    {
        MemoryBlock oldestBlock = pageQueue.front();
        pageQueue.pop();
        blocks[oldestBlock.startAddress / pageSize] = newBlock;
        pageQueue.push(newBlock);
        oldestBlock.release();
    }

    void LRUPageReplacement(const MemoryBlock &newBlock)
    {
        MemoryBlock leastRecentlyUsed = pageQueue.front();
        pageQueue.pop();
        blocks[leastRecentlyUsed.startAddress / pageSize] = newBlock;
        pageQueue.push(newBlock);
        leastRecentlyUsed.release(); // Release the least recently used block
    }

    void replacePageLRU(const std::string &newFileID)
    {
        if (lruList.size() == blocks.size())
        {
            MemoryBlock lastUsedPage = lruList.back();
            lruList.pop_back();
            lruMap.erase(lastUsedPage.fileID);

            deleteFile(lastUsedPage.fileID);
            loadFile(newFileID, TEXT); // Cambiar TEXT a BINARY según sea necesario
        }
    }

    int findStartAddress(int requiredSize)
    {
        if (policy == BEST_FIT)
        {
            return findBestFit(requiredSize);
        }
        else
        {
            return findWorstFit(requiredSize);
        }
    }

    int findBestFit(int requiredSize)
    {
        int bestSize = INT_MAX;
        int bestStart = 0;

        for (const auto &block : blocks)
        {
            int gap = (block.startAddress - bestStart) - requiredSize;
            if (gap >= 0 && gap < bestSize)
            {
                bestSize = gap;
                bestStart = block.startAddress;
            }
        }
        return bestStart;
    }

    int findWorstFit(int requiredSize)
    {
        int worstSize = -1;
        int worstStart = 0;

        for (const auto &block : blocks)
        {
            int gap = (block.startAddress - worstStart) - requiredSize;
            if (gap > worstSize)
            {
                worstSize = gap;
                worstStart = block.startAddress;
            }
        }
        return worstStart;
    }

void deleteFile(const std::string &fileID)
{
    auto it = std::remove_if(blocks.begin(), blocks.end(), [&fileID](const MemoryBlock &block)
                             { return block.fileID == fileID; });

    if (it != blocks.end())
    {
        blocks.erase(it, blocks.end());
        std::cout << "Archivo " << fileID << " eliminado.\n";
    }
    else
    {
        std::cout << "Archivo " << fileID << " no encontrado.\n";
    }
}


void overwriteFile(const std::string &fileID, const std::string &newContent)
{
    auto it = std::find_if(blocks.begin(), blocks.end(), [&fileID](const MemoryBlock &block)
                            { return block.fileID == fileID; });

    if (it != blocks.end())
    {
        it->content = newContent;
        std::cout << "Archivo " << fileID << " sobrescrito.\n";
    }
    else
    {
        std::cout << "Archivo " << fileID << " no encontrado para sobrescribir.\n";
    }
}


    void saveToUnisFormat(const std::string &fileName)
    {
        std::ofstream outFile(fileName + ".unis");

        outFile << totalSize << " " << pageSize << "\n";

        for (const auto &block : blocks)
        {
            outFile << block.startAddress << " "
                    << block.size << " "
                    << block.fileID << " "
                    << block.content << "\n";
        }

        outFile.close();
        std::cout << "Datos guardados en formato .unis en el archivo " << fileName << ".unis\n";
    }
};

void testBestFitAllocation()
{
    MemoryManager manager(1024, 64, BEST_FIT);
    manager.loadFile("archivoTest1.txt", TEXT);
    manager.loadFile("archivoTest2.txt", TEXT);
    assert(manager.blocks[0].startAddress <= manager.blocks[1].startAddress && "Best Fit allocation failed");
}

void testWorstFitAllocation()
{
    MemoryManager manager(1024, 64, WORST_FIT);
    manager.loadFile("archivoTest1.txt", TEXT);
    manager.loadFile("archivoTest2.txt", TEXT);
    assert(manager.blocks[0].startAddress <= manager.blocks[1].startAddress && "Worst Fit allocation failed");
}

void testFIFOPagination()
{
    MemoryManager manager(1024, 64, FIFO);
    manager.loadFile("archivoTest1.txt", TEXT);
    manager.loadFile("archivoTest2.txt", TEXT);
    manager.pageQueue.pop(); // Simulando FIFO
    assert(manager.pageQueue.size() == 1 && "FIFO Pagination failed");
}

void testLRUPagination()
{
    MemoryManager manager(1024, 64, LRU);
    manager.loadFile("archivoTest1.txt", TEXT);
    manager.loadFile("archivoTest2.txt", TEXT);
    manager.pageQueue.pop(); // Simulando FIFO
    assert(manager.pageQueue.size() == 1 && "LRU Pagination failed");
}




int main()
{
    // Ejecutando pruebas
    testBestFitAllocation();
    testWorstFitAllocation();
    testFIFOPagination();
    testLRUPagination();
    std::cout << "Todas las pruebas pasaron con éxito." << std::endl;

    MemoryManager manager(1024, 64, BEST_FIT);
    manager.loadFile("textFile.txt", TEXT);    // Asegúrate de que textFile.txt exista
    manager.loadFile("imageFile.png", BINARY); // Asegúrate de que imageFile.png exista

    manager.deleteFile("textFile.txt");
    manager.overwriteFile("imageFile.png", "Nuevo contenido");

    while (true)
    {
        std::cout << "\n--- Menú ---\n";
        std::cout << "1. Cargar archivo de texto\n";
        std::cout << "2. Cargar archivo binario\n";
        std::cout << "3. Eliminar archivo\n";
        std::cout << "4. Sobrescribir archivo\n";
        std::cout << "5. Guardar en formato .unis\n";
        std::cout << "6. Eliminar bloque\n";
        std::cout << "7. Sobreescribir bloque\n";
        std::cout << "8. Leer archivo\n";
        std::cout << "9. Listar todos los bloques\n";  
        std::cout << "10. Salir\n";

        int choice;
        std::cout << "Elige una opción: ";
        std::cin >> choice;

        std::string fileName, content;

        std::string newContent;

        switch (choice)
        {
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
            std::cin.ignore(); // Limpia el buffer
            std::cout << "Nuevo contenido: ";
            std::getline(std::cin, content); // Usa getline para manejar espacios
            manager.overwriteFile(fileName, content);
            break;



        case 5:
            std::cout << "Nombre del archivo donde guardar en formato .unis: ";
            std::cin >> fileName;
            manager.saveToUnisFormat(fileName);
            break;

        case 6:
            int deleteAddress;
            std::cout << "Dirección del bloque a eliminar: ";
            std::cin >> deleteAddress;
            manager.deleteBlock(deleteAddress);
            break;

        case 7:
            int overwriteAddress;
            std::cout << "Dirección del bloque a sobrescribir: ";
            std::cin >> overwriteAddress;
            std::cout << "Nuevo contenido: ";
            std::cin >> newContent;
            manager.overwriteBlock(overwriteAddress, newContent);
            break;

            case 8:
                std::cout << "Nombre del archivo para leer: ";
                std::cin >> fileName;
                manager.readFile(fileName);
                break;

            case 9:
                manager.listAllBlocks();  // Llama a la función para listar todos los bloques
                break;

            case 10:
                std::cout << "Saliendo...\n";
                return 0;

            default:
                std::cout << "Opción no válida.\n";
        }
    }

    return 0;
}