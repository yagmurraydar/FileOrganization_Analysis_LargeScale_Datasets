#  Scalable Indexed Library Automation System (BMI3241)

An optimized, file-based core database management unit engineered to handle large-scale library automation structures up to **1 Million records** with millisecond-level inquiry latencies. This system completely bypasses high RAM consumption models by introducing a custom-built secondary index engine on disk storage.

##  Architectural Core Features
- **Fixed-Length Records (FLR):** Standardized 309-byte data blocks enable absolute $O(1)$ sector computing via byte offset math.
- **Secondary Indexing Framework:** Detached 108-byte `author_index.bin` structure to eliminate high-overhead full file scans.
- **Tombstone Strategy (Lazy Deletion):** Injects a 1-byte active logical boolean marker to secure constant time complexity during high-frequency delete execution blocks.
- **Deterministic RAM Boundary:** Keeps active memory usage static (< 5 MB) regardless of order-of-magnitude increases in data volume (10K to 1M scale).

## Execution & System Workflow

### 1. Database Building Phase (`db_builder.cpp`)
Converts raw, delimited text records (`books_dataset.txt`) into serialized, fixed-length target data streams and links index offset maps concurrently:
- Generates `library_data.bin` (Main Data Store)
- Generates `author_index.bin` (Secondary Index Store)

### 2. Live Interactive Hub (`main.cpp`)
Launches the fully reactive console user interface providing real-time dynamic CRUD modules. 

##  Compilation & Deployment (Release Mode)
To activate maximum compiler loop unrolling and hardware-level performance configurations, build the code using the `-O3` production flag:

# Compilation
g++ -O3 db_builder.cpp -o db_builder
g++ -O3 main.cpp -o library_app

# Directory Structure Constraints
# Ensure the compiled executable sits adjacent to the source data:
.
├── library_app.exe
├── db_builder.exe
├── books_dataset.txt
├── library_data.bin
└── author_index.bin


Dataset Scale,      Main Data File Size,       Index File Size,     DB Build Latency,       Author Search Latency
"10,000 (10K)",      ~3.09 MB,                  ~1.08 MB,                 ~4ms,                 ~0.15 ms
"100,000 (100K)",    ~30.90 MB,                  ~10.80 MB,              ~410 ms,               ~0.85 ms
"1,000,000 (1M)",     ~309.00 MB,                ~108.00 MB,             ~4,250 ms,             ~6.40 ms

Group Members 
This project was designed, implemented, and defended collectively by: 
Yağmur Aydar
Zülfiye Buse Güneş
Erdem Yüksel
