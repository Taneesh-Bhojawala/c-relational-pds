# 🗄️ C-Based Relational Persistent Data Store (PDS)

A lightweight, custom relational database management system built entirely from scratch in C. 

This project demonstrates the practical implementation of core Database Management and Operating System concepts, bypassing standard libraries to handle raw file data persistence, custom memory indexing, and strict relational integrity.

## ✨ Core Features & Architecture

* **Custom Array-Based Indexing:** Separates raw binary data (`.dat`) from index arrays (`.ndx`). The system loads the index into primary memory at runtime, enabling direct disk offset lookups and bypassing costly sequential file reads.
* **Data Integrity & Relational Mapping:** Supports schema relationships with dynamic runtime management, complete with foreign key constraints and cascading deletes.
* **Generic Data Handling:** Utilizes anonymous pointers (`void *`) allowing the system to seamlessly store, retrieve, and update any structured data (e.g., `Course`, `Hospital`) without altering core internal logic.
* **Soft Deletion & Recovery:** Implements a robust soft-delete mechanism. Records are flagged `is_deleted` and keys are cached, allowing for full state recovery via an `UNDELETE` command without physical file fragmentation.

## ⏱️ Performance & Time Complexity

By keeping the index array in memory, the system optimizes disk interactions:
* **Search / Read:** O(N) memory scan through the index array, followed by an **O(1) disk seek** directly to the byte offset.
* **Insert:** O(1) insertion at the end of the `.dat` file and index array.
* **Update / Delete:** O(N) memory search to locate the index, followed by O(1) in-place file overwrite or flag toggle.

## 🚀 Project Evolution

The system architecture was progressively scaled across three distinct phases:

* **PDS 1.0 (Single-Table Generic Store):** Foundation established for raw binary file I/O, primary key indexing, and void pointer data handling.
* **PDS 2.0 (Two-Table Generic Store):** Expanded to handle independent generic tables concurrently under a single unified database connection (`Db_Info`), isolating distinct `.dat` and `.ndx` files.
* **PDS 3.0 (Relational Data Store):** Introduced relationship mapping. Manages a strict active runtime state of exactly two entity tables and one relationship table concurrently to enforce 1-to-1 mapping and foreign key constraints.

## 🏗️ Constraints

* **Concurrent State Limit:** The active runtime state is strictly limited to a maximum of two open entity tables and one open relationship table at any given time.
* **Capacity:** The primary memory index array has a hardcoded limit of 10,000 records per table at runtime (`#define MAX 10000`).
* **Record Sizing:** Table record sizes (`rec_size`) are fixed upon database initialization via the `OPEN` command.
* **Primary Keys:** Keys are strictly unique integers.
