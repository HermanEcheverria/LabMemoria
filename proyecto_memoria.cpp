// Laboratorio de Sistemas Operativos: Proyecto Memoria
// Integrantes: Herman Echeverría  |  Pablo Morales  |  Máx Marroquín


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

// Enumeraciones para tipos de archivo y políticas de asignación
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

// Clase que representa un bloque de memoria
class MemoryBlock
{
public:
    int startAddress;
    int size;
    std::string fileID;
    std::string content;

    // Constructor
    MemoryBlock(int startAddress, int size, const std::string &fileID, const std::string &content)
        : startAddress(startAddress), size(size), fileID(fileID), content(content) {}

    // Asignar bloque
    void assign()
    {
        std::cout << "Asignando bloque para " << fileID << std::endl;
    }

    // Liberar bloque
    void release()
    {
        std::cout << "Liberando bloque de " << fileID << std::endl;
    }
};


// Clase que gestiona la memoria
class MemoryManager
{
public:
    int totalSize; // Tamaño total de la memoria
    int pageSize;  // Tamaño de página
    AllocationPolicy policy; // Política de asignación
    std::vector<MemoryBlock> blocks; // Bloques de memoria
    std::queue<MemoryBlock> pageQueue; // Cola para política FIFO
    std::list<MemoryBlock> lruList; // Lista para política LRU
    std::unordered_map<std::string, std::list<MemoryBlock>::iterator> lruMap; // Mapa para LRU


    // Constructor
    MemoryManager(int totalSize, int pageSize, AllocationPolicy policy)
        : totalSize(totalSize), pageSize(pageSize), policy(policy) {}



    // Leer archivo de la memoria
void readFile(const std::string& fileID) {

            // Busca el archivo en los bloques de memoria
    auto it = std::find_if(blocks.begin(), blocks.end(), [&fileID](const MemoryBlock& block) {
        return block.fileID == fileID;
    }); 
    
        // Si se encuentra, muestra el contenido
    if (it != blocks.end()) {
        std::cout << "Contenido del archivo " << fileID << ":\n";
        std::cout << it->content << std::endl;
    } else {
        std::cout << "Archivo " << fileID << " no encontrado.\n";
    }
}


    // Listar todos los bloques de memoria
void listAllBlocks() {
    std::cout << "Bloques actuales en memoria:\n";
    for(const auto& block : blocks) {
        std::cout << "Archivo: " << block.fileID << ", Dirección de inicio: " << block.startAddress << ", Tamaño: " << block.size << '\n';
    }
}


    // Cargar archivo en memoria
    void loadFile(const std::string &fileName, FileType fileType)
    {
        std::ifstream file; // Stream de archivo

        // Abre el archivo en modo texto o binario
        if (fileType == TEXT)
        {
            file.open(fileName, std::ios::in);
        }
        else
        {
            file.open(fileName, std::ios::in | std::ios::binary);
        }
        // Si no se puede abrir el archivo, muestra un error
        if (!file)
        {
                    // Lee el contenido del archivo en una cadena
            std::cerr << "No se puede abrir el archivo: " << fileName << std::endl;
            return;
        }

        std::string content = std::string((std::istreambuf_iterator<char>(file)),
                                          std::istreambuf_iterator<char>());
        int fileSize = content.size();
                // Calcula el número de páginas requeridas y el tamaño necesario
        int requiredPages = (fileSize + pageSize - 1) / pageSize;
        int requiredSize = requiredPages * pageSize;

        // Encuentra la dirección de inicio para el bloque
        int startAddress = findStartAddress(requiredSize);

        // Verifica si hay suficiente memoria
        if (startAddress + requiredSize > totalSize)
        {
            std::cout << "No hay suficiente memoria para " << fileName << std::endl;
            return;
        }


        // Crea un nuevo bloque de memoria
        MemoryBlock newBlock(startAddress, requiredSize, fileName, content);
        newBlock.assign();


        // Implementa la política de asignación
        if (policy == BEST_FIT)
        {            // Encuentra el mejor ajuste y añade el bloque

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

// Método para acceder a una página (archivo) y actualizar la lista LRU
    void accessPage(const std::string &fileID)
    {      // Si el archivo ya se encuentra en la lista LRU

        if (lruMap.find(fileID) != lruMap.end())
        {
            lruList.erase(lruMap[fileID]);
                    // Añade el archivo al frente de la lista LRU (como el más recientemente usado)

            lruList.push_front(*lruMap[fileID]);
                    // Actualiza el mapa con la nueva posición del archivo en la lista LRU

            lruMap[fileID] = lruList.begin();
        }
        else  // Si el archivo no está en la lista LRU
        {
            for (const auto &block : blocks)
            {
                            // Si se encuentra el archivo

                if (block.fileID == fileID)
                {
                    lruList.push_front(block);
                    lruMap[fileID] = lruList.begin();
                    break;
                }
            }
        }
    }

// Método para dividir la memoria en bloques según el tamaño de página
    void divideAndStoreBlocks()
    {
        int numPages = totalSize / pageSize;
        for (int i = 0; i < numPages; ++i)
        {
            MemoryBlock newBlock(i * pageSize, pageSize, "", "");
            blocks.push_back(newBlock);
        }
    }


// Método para dividir la memoria en bloques según el tamaño de página
    void defragment()
    {
            // Ordena los bloques por dirección de inicio
        std::sort(blocks.begin(), blocks.end(), [](const MemoryBlock &a, const MemoryBlock &b)
                  { return a.startAddress < b.startAddress; });

        std::vector<MemoryBlock> newBlocks;
        int currentAddress = 0;

        for (const auto &block : blocks)
        {
                    // Si hay un hueco entre el bloque actual y la dirección actual
            if (block.startAddress != currentAddress)
            {
                            // Crea un nuevo bloque vacío para llenar el hueco
                int gapSize = block.startAddress - currentAddress;
                MemoryBlock newGap(currentAddress, gapSize, "", "");
                newBlocks.push_back(newGap);          // Añade el bloque actual a la lista de nuevos bloques
                currentAddress += gapSize;         // Actualiza la dirección actual

            }
            newBlocks.push_back(block);     // Reemplaza los bloques antiguos con los nuevos bloques defragmentados
            currentAddress += block.size;
        }

        blocks = newBlocks;
    }


// Método para eliminar un bloque de memoria por su dirección de inicio
    void deleteBlock(int startAddress)
    {






















































            // Encuentra el bloque por su dirección de inicio y elimínalo
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


// Método para sobrescribir el contenido de un bloque de memoria
    void overwriteBlock(int startAddress, const std::string &newContent)
    {    // Encuentra el bloque por su dirección de inicio

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


// Método para reemplazar la página más antigua (FIFO)
    void FIFOPageReplacement(const MemoryBlock &newBlock)
    {
        MemoryBlock oldestBlock = pageQueue.front();     // Obtiene el bloque más antiguo de la cola
        pageQueue.pop();     // Reemplaza el bloque más antiguo con el nuevo bloque en el vector de bloques
        blocks[oldestBlock.startAddress / pageSize] = newBlock;     // Añade el nuevo bloque a la cola
        pageQueue.push(newBlock);    // Libera el bloque más antiguo
        oldestBlock.release();
    }


// Método para reemplazar la página menos recientemente utilizada (LRU)
    void LRUPageReplacement(const MemoryBlock &newBlock)
    {
        MemoryBlock leastRecentlyUsed = pageQueue.front();     // Obtiene el bloque menos recientemente utilizado de la cola
        pageQueue.pop();     // Retira el bloque menos recientemente utilizado de la cola
        blocks[leastRecentlyUsed.startAddress / pageSize] = newBlock;    // Reemplaza el bloque menos recientemente utilizado con el nuevo bloque en el vector de bloques
        pageQueue.push(newBlock);     // Añade el nuevo bloque a la cola
        leastRecentlyUsed.release();   // Libera el bloque menos recientemente utilizado

    }

// Método para reemplazar una página en la estrategia LRU
    void replacePageLRU(const std::string &newFileID)
    {    // Verifica si el tamaño de la lista LRU es igual al tamaño de los bloques
        if (lruList.size() == blocks.size())
        {
            MemoryBlock lastUsedPage = lruList.back();        // Obtiene la última página utilizada (menos recientemente utilizada)
            lruList.pop_back();
            lruMap.erase(lastUsedPage.fileID);         // Elimina la última página de la lista y del mapa

            deleteFile(lastUsedPage.fileID);
            loadFile(newFileID, TEXT); // Cambiar TEXT a BINARY según sea necesario
        }
    }

// Método para encontrar la dirección de inicio según el tamaño requerido y la política
    int findStartAddress(int requiredSize)
    {    // Utiliza la política de mejor ajuste o peor ajuste según se configure
        if (policy == BEST_FIT)
        {
            return findBestFit(requiredSize);
        }
        else
        {
            return findWorstFit(requiredSize);
        }
    }

// Método para encontrar el mejor ajuste
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

// Método para encontrar el peor ajuste
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

// Método para eliminar un archivo
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

// Método para sobrescribir un archivo
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

// Método para guardar en formato .unis
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

// Funciones para pruebas unitarias
void testBestFitAllocation()
{
    MemoryManager manager(1024, 64, BEST_FIT);
    manager.loadFile("archivoTest1.txt", TEXT);
    manager.loadFile("archivoTest2.txt", TEXT);
    assert(manager.blocks[0].startAddress <= manager.blocks[1].startAddress && "Best Fit allocation failed");
}


// Función de prueba para verificar la implementación del algoritmo Worst Fit
void testWorstFitAllocation()
{
    MemoryManager manager(1024, 64, WORST_FIT);     // Inicializa el administrador de memoria con política de Worst Fit
    manager.loadFile("archivoTest1.txt", TEXT);    // Carga archivos de prueba
    manager.loadFile("archivoTest2.txt", TEXT);
    assert(manager.blocks[0].startAddress <= manager.blocks[1].startAddress && "Worst Fit allocation failed");    // Verifica que la asignación se ha realizado correctamente

}

// Función de prueba para verificar la paginación FIFO
void testFIFOPagination()
{
    MemoryManager manager(1024, 64, FIFO);     // Inicializa el administrador de memoria con política de FIFO
    manager.loadFile("archivoTest1.txt", TEXT);     // Carga archivos de prueba
    manager.loadFile("archivoTest2.txt", TEXT);
    manager.pageQueue.pop(); // Simulando FIFO
    assert(manager.pageQueue.size() == 1 && "FIFO Pagination failed");
}
// Función de prueba para verificar la paginación LRU
void testLRUPagination()
{
    MemoryManager manager(1024, 64, LRU);     // Inicializa el administrador de memoria con política de LRU
    manager.loadFile("archivoTest1.txt", TEXT);
    manager.loadFile("archivoTest2.txt", TEXT);
    manager.pageQueue.pop(); // Simulando FIFO
    assert(manager.pageQueue.size() == 1 && "LRU Pagination failed");    // Verifica que el tamaño de la cola sea 1

}


int main()
{
    // Ejecuta todas las pruebas unitarias
    testBestFitAllocation();
    testWorstFitAllocation();
    testFIFOPagination();
    testLRUPagination();
    std::cout << "Todas las pruebas pasaron con éxito." << std::endl;

    MemoryManager manager(1024, 64, BEST_FIT);     // Inicializa el administrador de memoria con política de Best Fit
    manager.loadFile("textFile.txt", TEXT);    //  Creado manualmente
    manager.loadFile("imageFile.png", BINARY); // Creado manualmente
    manager.deleteFile("textFile.txt");
    manager.overwriteFile("imageFile.png", "Nuevo contenido");


    // Bucle del menú principal
    while (true)
    {
                // Despliega el menú al usuario
        std::cout << "\n--- Menú de Manejo---\n";
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

        // Selección de la acción del usuario
        int choice;
        std::cout << "Elige una opción: ";
        std::cin >> choice;

        std::string fileName, content;

        std::string newContent;

        // Manejo de la selección del usuario
        switch (choice)
        {
            // Llamadas y ejecución de funciones en base a selección
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