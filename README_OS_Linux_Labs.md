
# OS Linux Labs

This repository contains implementations and solutions for Operating Systems (OS) laboratory assignments, focusing on core Linux concepts and system-level programming. These assignments explore process management, concurrency, device drivers, and encryption mechanisms.

## Lab Assignments

### **Lab 1: Custom Shell Implementation**
- **Features**:
  - Command execution with history tracking.
  - Support for background processes (`&`).
- **Key Concepts**:
  - Process management with `fork()`, `execvp()`, and `wait()`.
  - Linked list implementation for storing command history.

### **Lab 2: Concurrent Linked List**
- **Features**:
  - A thread-safe singly linked list using `pthread_mutex_t`.
  - Functions:
    - `insert_value`: Insert values in ascending order.
    - `remove_value`: Remove a specific value.
    - `print_list`: Display list contents.
    - `count_list`: Count nodes based on a predicate.
  - Fine-grained thread safety for concurrent operations.
- **Key Concepts**:
  - Mutex synchronization.
  - Data structure management in a concurrent environment.

### **Lab 3: Encryption/Decryption Device Driver**
- **Features**:
  - Linux kernel module implementing a character device with two encryption modes:
    - **Caesar Cipher**: Rotational character encryption.
    - **XOR Cipher**: Bitwise XOR encryption.
  - Key operations:
    - `encdec_ioctl`: Set encryption key and read state.
    - `encdec_read_caesar` and `encdec_write_caesar`: Read and write for Caesar cipher.
    - `encdec_read_xor` and `encdec_write_xor`: Read and write for XOR cipher.
  - Memory management using `kmalloc` and `kfree`.
- **Key Concepts**:
  - Linux kernel programming.
  - Device drivers and file operations.
  - Encryption and decryption techniques.

## How to Use

### Clone the Repository
```bash
git clone https://github.com/KingBaruh/OS-Linux-Labs.git
cd OS-Linux-Labs
```

### Compile and Run

#### Lab 1 (Shell):
1. Navigate to the lab1 directory:
   ```bash
   cd lab1
   ```
2. Compile the shell:
   ```bash
   gcc lab1_shell.c -o my-shell
   ```
3. Run the shell:
   ```bash
   ./my-shell
   ```

#### Lab 2 (Concurrent List):
1. Navigate to the lab2 directory:
   ```bash
   cd lab2
   ```
2. Compile the program:
   ```bash
   gcc -pthread concurrent_list.c -o concurrent_list
   ```
3. Run the program:
   ```bash
   ./concurrent_list
   ```

#### Lab 3 (Device Driver):
1. Navigate to the lab3 directory:
   ```bash
   cd lab3
   ```
2. Build the kernel module:
   ```bash
   make
   ```
3. Load the module:
   ```bash
   sudo insmod encdec.ko memory_size=<size>
   ```
4. Interact with the device files (`/dev/encdec_caesar` and `/dev/encdec_xor`) using `cat` or custom programs.

## Requirements
- **Operating System**: Linux
- **Compiler**: GCC
- **Libraries**: `pthread`
- **Kernel Development Tools** (for Lab 3).

## Learning Objectives
- Develop a deeper understanding of process and thread management.
- Learn synchronization techniques for data structures.
- Explore practical applications of OS concepts like concurrency, encryption, and device drivers.

## License
This repository is open for educational purposes. Contributions are welcome!
