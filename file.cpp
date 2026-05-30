#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <algorithm>
#include <chrono>

// 309 kayıt olan record
struct Record {
    int id = 0;
    int year = 0;
    char type[100] = { 0 };
    char name[100] = { 0 };
    char author[100] = { 0 };
    bool isDeleted = false; // Tombstone marker
};

// 108 olan
struct AuthorIndex {
    char author[100] = { 0 };
    long long fileOffset = 0; // Pointer to library_data.bin
};


void DisplayRecordHeader() {
    std::cout << "\n-------------------------------------------------------------------------------------------------------" << std::endl;
    printf("%-10s | %-6s | %-15s | %-25s | %-35s\n", "ID", "YEAR", "TYPE", "AUTHOR", "BOOK NAME");
    std::cout << "-------------------------------------------------------------------------------------------------------" << std::endl;
}

void DisplaySingleRecord(const Record& rec) {
    printf("%-10d | %-6d | %-15.15s | %-25.25s | %-35.35s\n", rec.id, rec.year, rec.type, rec.author, rec.name);
}


class IndexedLibraryDB {
private:
    std::string dataFileName;
    std::string indexFileName;

public:
    IndexedLibraryDB(std::string dataFile, std::string indexFile) 
        : dataFileName(dataFile), indexFileName(indexFile) {}

    
    bool CreateRecord(const Record& newRec) {
        FILE* dFile = fopen(dataFileName.c_str(), "ab");
        FILE* iFile = fopen(indexFileName.c_str(), "ab");
        if (!dFile || !iFile) {
            if (dFile) fclose(dFile);
            if (iFile) fclose(iFile);
            return false;
        }

        
        fseek(dFile, 0, SEEK_END);
        long long writtenOffset = ftell(dFile);

      
        fwrite(&newRec, sizeof(Record), 1, dFile);

        
        AuthorIndex idx;
        strncpy(idx.author, newRec.author, 99);
        idx.author[99] = '\0';
        idx.fileOffset = writtenOffset;
        fwrite(&idx, sizeof(AuthorIndex), 1, iFile);

        fclose(dFile);
        fclose(iFile);
        return true;
    }


    std::vector<Record> SearchByAuthorIndexed(const std::string& authorName) {
        std::vector<Record> results;
        
        FILE* iFile = fopen(indexFileName.c_str(), "rb");
        FILE* dFile = fopen(dataFileName.c_str(), "rb");
        
        if (!iFile || !dFile) {
            if (iFile) fclose(iFile);
            if (dFile) fclose(dFile);
            std::cerr << "[ERROR] DB files missing. Please run Builder first!" << std::endl;
            return results;
        }

        AuthorIndex idxTemp;
       
        while (fread(&idxTemp, sizeof(AuthorIndex), 1, iFile)) {
            if (_stricmp(idxTemp.author, authorName.c_str()) == 0) {
                
         
                fseek(dFile, idxTemp.fileOffset, SEEK_SET);
                
                Record realRec;
                fread(&realRec, sizeof(Record), 1, dFile);

                if (!realRec.isDeleted) {
                    results.push_back(realRec);
                }
            }
        }

        fclose(iFile);
        fclose(dFile);
        return results;
    }

  
    bool SearchById(int targetId, Record& result) {
        FILE* dFile = fopen(dataFileName.c_str(), "rb");
        if (!dFile) return false;

        Record temp;
        while (fread(&temp, sizeof(Record), 1, dFile)) {
            if (temp.id == targetId && !temp.isDeleted) {
                result = temp;
                fclose(dFile);
                return true;
            }
        }
        fclose(dFile);
        return false;
    }

  
    bool UpdateRecordIndexed(int targetId, const Record& updatedRec) {
        FILE* dFile = fopen(dataFileName.c_str(), "r+b"); // r/w 
        if (!dFile) return false;

        Record temp;
        long long currentOffset = 0;
        bool found = false;

        while (fread(&temp, sizeof(Record), 1, dFile)) {
            if (temp.id == targetId && !temp.isDeleted) {
                fseek(dFile, currentOffset, SEEK_SET);
                fwrite(&updatedRec, sizeof(Record), 1, dFile);
                found = true;
                break;
            }
            currentOffset = ftell(dFile);
        }
        fclose(dFile);
        return found;
    }

   
    bool DeleteRecordIndexed(int targetId) {
        FILE* dFile = fopen(dataFileName.c_str(), "r+b");
        if (!dFile) return false;

        Record temp;
        long long currentOffset = 0;
        bool found = false;

        while (fread(&temp, sizeof(Record), 1, dFile)) {
            if (temp.id == targetId && !temp.isDeleted) {
                temp.isDeleted = true; 
                fseek(dFile, currentOffset, SEEK_SET);
                fwrite(&temp, sizeof(Record), 1, dFile);
                found = true;
                break;
            }
            currentOffset = ftell(dFile);
        }
        fclose(dFile);
        return found;
    }
};


int main() {
    IndexedLibraryDB db("library_data.bin", "author_index.bin");
    int choice = 0;

    std::cout << "=== BMI3241 File Organization INDEXED Engine Loaded ===" << std::endl;

    while (true) {
        std::cout << "\n=========================================" << std::endl;
        std::cout << "     INDEXED STORAGE ENGINE CONTROL      " << std::endl;
        std::cout << "=========================================" << std::endl;
        std::cout << "1. Insert New Book (Create & Index)" << std::endl;
        std::cout << "2. Search Book by Author Name (Indexed Read)" << std::endl;
        std::cout << "3. Search Book by Unique ID (Direct Read)" << std::endl;
        std::cout << "4. Modify Existing Book (Update)" << std::endl;
        std::cout << "5. Erase Book from Storage (Delete)" << std::endl;
        std::cout << "6. Terminate Application (Exit)" << std::endl;
        std::cout << "Enter selection (1-6): ";

        if (!(std::cin >> choice)) {
            std::cin.clear();
            std::cin.ignore(1000, '\n');
            std::cout << "[ERROR] Invalid numeric input!" << std::endl;
            continue;
        }
        std::cin.ignore(1000, '\n');

        if (choice == 6) {
            std::cout << "\nSystem shutting down gracefully. Goodbye!" << std::endl;
            break;
        }

        if (choice == 1) {
            Record userRec;
            std::string buffer;

            std::cout << "Enter Book ID (Int): "; std::cin >> userRec.id;
            std::cout << "Enter Release Year (Int): "; std::cin >> userRec.year;
            std::cin.ignore(1000, '\n');

            std::cout << "Enter Category: "; std::getline(std::cin, buffer);
            strncpy(userRec.type, buffer.c_str(), 99);
            std::cout << "Enter Title: "; std::getline(std::cin, buffer);
            strncpy(userRec.name, buffer.c_str(), 99);
            std::cout << "Enter Author: "; std::getline(std::cin, buffer);
            strncpy(userRec.author, buffer.c_str(), 99);
            userRec.isDeleted = false;

            if (db.CreateRecord(userRec)) {
                std::cout << "[SUCCESS] Committed. Record serialized and indexed on disk." << std::endl;
            } else {
                std::cout << "[ERROR] Disk I/O Write failed!" << std::endl;
            }
        }
        else if (choice == 2) {
            std::string authorQuery;
            std::cout << "Enter target Author Name to query: ";
            std::getline(std::cin, authorQuery);

           
            auto start = std::chrono::high_resolution_clock::now();
            std::vector<Record> matches = db.SearchByAuthorIndexed(authorQuery);
            auto end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double, std::milli> elapsed = end - start;

            if (!matches.empty()) {
                DisplayRecordHeader();
                for (const auto& r : matches) DisplaySingleRecord(r);
                std::cout << "-------------------------------------------------------------------------------------------------------" << std::endl;
                std::cout << "[METRIC] Query completed in " << elapsed.count() << " ms. (" << matches.size() << " rows retrieved)" << std::endl;
            } else {
                std::cout << "[NOT FOUND] No rows match the query criteria. (Time taken: " << elapsed.count() << " ms)" << std::endl;
            }
        }
        else if (choice == 3) {
            int searchId;
            std::cout << "Enter target Unique Book ID: ";
            std::cin >> searchId;

            Record foundRec;
            if (db.SearchById(searchId, foundRec)) {
                DisplayRecordHeader();
                DisplaySingleRecord(foundRec);
                std::cout << "-------------------------------------------------------------------------------------------------------" << std::endl;
            } else {
                std::cout << "[NOT FOUND] Record missing or deleted." << std::endl;
            }
        }
        else if (choice == 4) {
            int updateId; std::cout << "Enter target Book ID to UPDATE: "; std::cin >> updateId;
            Record oldRec;
            if (db.SearchById(updateId, oldRec)) {
                std::cout << "\nCurrent state of record:" << std::endl;
                DisplayRecordHeader(); DisplaySingleRecord(oldRec);
                std::cout << "-------------------------------------------------------------------------------------------------------" << std::endl;

                Record newRec;
                newRec.id = updateId;
                std::string buffer;
                
                std::cout << "\nEnter NEW Year: "; std::cin >> newRec.year;
                std::cin.ignore(1000, '\n');
                std::cout << "Enter NEW Category: "; std::getline(std::cin, buffer);
                strncpy(newRec.type, buffer.c_str(), 99);
                std::cout << "Enter NEW Title: "; std::getline(std::cin, buffer);
                strncpy(newRec.name, buffer.c_str(), 99);
                std::cout << "Enter NEW Author: "; std::getline(std::cin, buffer);
                strncpy(newRec.author, buffer.c_str(), 99);
                newRec.isDeleted = false;

                if (db.UpdateRecordIndexed(updateId, newRec)) {
                    std::cout << "[SUCCESS] Disk sector updated permanently." << std::endl;
                }
            } else {
                std::cout << "[ERROR] Target record missing." << std::endl;
            }
        }
        else if (choice == 5) {
            int deleteId; std::cout << "Enter target Book ID to DELETE: "; std::cin >> deleteId;
            if (db.DeleteRecordIndexed(deleteId)) {
                std::cout << "[SUCCESS] Record invalidated via Tombstone injection." << std::endl;
            } else {
                std::cout << "[ERROR] Deletion failed. Target record missing." << std::endl;
            }
        }
        else {
            std::cout << "[WARNING] Selection out of bounds!" << std::endl;
        }
    }
    return 0;
}