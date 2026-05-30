#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <iostream>
#include <string>
#include <cstring>
#include <chrono>


struct Record {
    int id = 0;
    int year = 0;
    char type[100] = { 0 };
    char name[100] = { 0 };
    char author[100] = { 0 };
    bool isDeleted = false; 
};

struct AuthorIndex {
    char author[100] = { 0 }; 
    long long fileOffset = 0; 
};


std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, last - first + 1);
}


bool ParseCSVLine(const std::string& line, Record& rec) {
    std::string temp = line;

    size_t lastComma = temp.rfind(',');
    if (lastComma == std::string::npos) return false;
    std::string year_str = trim(temp.substr(lastComma + 1));
    temp = temp.substr(0, lastComma);

    size_t secondLastComma = temp.rfind(',');
    if (secondLastComma == std::string::npos) return false;
    std::string type_str = trim(temp.substr(secondLastComma + 1));
    temp = temp.substr(0, secondLastComma);

    size_t thirdLastComma = temp.rfind(',');
    if (thirdLastComma == std::string::npos) return false;
    std::string author_str = trim(temp.substr(thirdLastComma + 1));
    temp = temp.substr(0, thirdLastComma);

    size_t firstComma = temp.find(',');
    if (firstComma == std::string::npos) return false;
    std::string id_str = trim(temp.substr(0, firstComma));
    std::string name_str = trim(temp.substr(firstComma + 1));

    if (id_str.empty() || name_str.empty() || author_str.empty() || type_str.empty() || year_str.empty()) {
        return false;
    }

    try {
        rec.id = std::stoi(id_str);
        rec.year = std::stoi(year_str);
        
        strncpy(rec.name, name_str.c_str(), 99);
        rec.name[99] = '\0';
        strncpy(rec.author, author_str.c_str(), 99);
        rec.author[99] = '\0';
        strncpy(rec.type, type_str.c_str(), 99);
        rec.type[99] = '\0';
        rec.isDeleted = false;
    }
    catch (...) {
        return false;
    }
    return true;
}

int main() {
    std::string txtPath = "books_dataset.txt";
    std::string dataBinPath = "library_data.bin";
    std::string indexBinPath = "author_index.bin";
    
  
    int maxRecordsToParse = 1000000; 

    std::cout << "==========================================================" << std::endl;
    std::cout << "        DATABASE BUILDER & INDEX GENERATOR ENGINE         " << std::endl;
    std::cout << "==========================================================" << std::endl;
    std::cout << "Target Limit: " << maxRecordsToParse << " records." << std::endl;
    std::cout << "Opening '" << txtPath << "'... Please wait." << std::endl;

    FILE* txtFile = fopen(txtPath.c_str(), "r");
    if (!txtFile) {
        std::cerr << "[CRITICAL ERROR] Source text file '" << txtPath << "' not found in directory!" << std::endl;
        return -1;
    }

    FILE* dataFile = fopen(dataBinPath.c_str(), "wb");
    FILE* indexFile = fopen(indexBinPath.c_str(), "wb");

    if (!dataFile || !indexFile) {
        std::cerr << "[CRITICAL ERROR] Failed to create binary output streams!" << std::endl;
        if (txtFile) fclose(txtFile);
        if (dataFile) fclose(dataFile);
        if (indexFile) fclose(indexFile);
        return -1;
    }

    char buffer[512];
    int recordCount = 0;

    auto startTime = std::chrono::high_resolution_clock::now();

    while (fgets(buffer, sizeof(buffer), txtFile) != nullptr && recordCount < maxRecordsToParse) {
        Record rec;
        if (ParseCSVLine(buffer, rec)) {
            
      
            long long structuralOffset = ftell(dataFile);

            fwrite(&rec, sizeof(Record), 1, dataFile);

    
            AuthorIndex idx;
            strncpy(idx.author, rec.author, 99);
            idx.author[99] = '\0';
            idx.fileOffset = structuralOffset; // Link the index cell to the data file offset

       
            fwrite(&idx, sizeof(AuthorIndex), 1, indexFile);

            recordCount++;
            
            
            if (recordCount % 100000 == 0) {
                std::cout << " -> Progress: " << recordCount << " rows parsed and indexed successfully." << std::endl;
            }
        }
    }

    // Stop execution clock
    auto endTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> totalDuration = endTime - startTime;

    fclose(txtFile);
    fclose(dataFile);
    fclose(indexFile);

    std::cout << "==========================================================" << std::endl;
    std::cout << "[SUCCESS] Disk Serialization Process Terminated." << std::endl;
    std::cout << " -> Total Parsed Records : " << recordCount << std::endl;
    std::cout << " -> Execution/Build Time : " << totalDuration.count() << " ms (" << totalDuration.count() / 1000.0 << " seconds)" << std::endl;
    std::cout << " -> Data Storage State   : '" << dataBinPath << "' is ready." << std::endl;
    std::cout << " -> Index Storage State  : '" << indexBinPath << "' is ready." << std::endl;
    std::cout << "==========================================================" << std::endl;

    return 0;
}