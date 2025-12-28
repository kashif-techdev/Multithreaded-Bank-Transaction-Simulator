# 🏦 Bank Transaction Simulator (C++ | Operating Systems)

A **multithreaded Bank Transaction Simulator** implemented in **C++**, demonstrating core **Operating System concepts** such as **threads, mutexes, synchronization, and shared resources**.  
This project simulates real-world banking operations where multiple customers perform transactions concurrently in a safe and controlled environment.

---

## 📌 Project Overview

In modern banking systems, multiple users access shared resources (accounts) at the same time.  
This project simulates that scenario using **multithreading**, ensuring **data consistency** and **race-condition prevention** through proper synchronization techniques.

---

## 🎯 Objectives

- Simulate concurrent bank transactions  
- Implement **thread synchronization** using mutex locks  
- Prevent race conditions and inconsistent data  
- Understand real-world **Operating System concepts**

---

## ⚙️ Features

✔️ Multiple bank accounts  
✔️ Concurrent customer transactions  
✔️ Deposit & withdrawal operations  
✔️ Thread-safe balance updates  
✔️ Mutex-based synchronization  
✔️ Real-time transaction logs  

---

## 🧠 OS Concepts Used

- **Multithreading (std::thread)**
- **Mutex Locks (std::mutex)**
- **Critical Section Handling**
- **Race Condition Prevention**
- **Shared Resource Management**

---

## 🛠️ Technologies Used

| Technology | Purpose |
|----------|--------|
| **C++** | Core programming language |
| **Threads** | Simulate multiple customers |
| **Mutex** | Synchronization |
| **OOP** | Clean and modular design |

---

## 📂 Project Structure

```
📁 Bank-Simulator
 ├── bank_simulator.cpp   # Main source file
 ├── README.md            # Project documentation
```

---

## ▶️ How to Run the Project

### 1️⃣ Compile the program
```bash
g++ bank_simulator.cpp -o bank_simulator -pthread
```

### 2️⃣ Run the executable
```bash
./bank_simulator
```

---

## 🧪 Sample Output

```
Customer 1 deposited 500
Customer 2 withdrew 300
Account Balance Updated Safely
```

*(Actual output may vary depending on thread execution order)*

---

## 🚧 Problem Solved

Without synchronization, multiple threads accessing the same bank account can lead to:

❌ Incorrect balances  
❌ Data corruption  
❌ Race conditions  

✅ This project solves these issues using **mutex locks** to ensure **only one thread accesses critical data at a time**.

---

## 📚 Learning Outcomes

- Strong understanding of **multithreading**
- Practical use of **mutex & synchronization**
- Hands-on experience with **OS-level programming**
- Improved C++ concurrency skills

---

## 👨‍💻 Author

**Kashif Khan**  
BSCS Student | Air University Islamabad  
📌 Interests: Operating Systems, C++, Flutter, AI, ML  

---

## ⭐ Future Improvements

- Add file-based transaction logs  
- Implement semaphores  
- Add GUI or console menu  
- Simulate deadlock scenarios  

---

## 🌟 Give it a Star!

If you found this project helpful or educational, don’t forget to ⭐ star the repository!
