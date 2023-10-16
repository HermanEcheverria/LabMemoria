// Laboratorio - Proyecto Memoria
// Integrantes: Herman Echeverría | Pablo Morales | Máx Marroquín

#include <iostream>
#include <vector>
#include <string>


class MemoryBlock {
public:
    int startAddress; 
    int size;        
    std::string fileID;  

   
    void assign();  
    void release(); 
    void read();   
    void write();   
};


class MemoryManager {
public:
    int totalSize;  
    int pageSize;   
    std::vector<MemoryBlock> blocks;


    void loadFile(std::string fileName);      
    void saveFile(std::string fileName);    
    void deleteFile(std::string fileID);    
    void readFile(std::string fileID);       
    void defragment();                       
};

int main() {
    
    return 0;
}
