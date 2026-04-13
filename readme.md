
-----

#  Advanced Electronic Health Record (EHR) System in C++ (GUI Version)

A **Graphical User Interface (GUI)**-based application demonstrating the power of core data structures (**Graphs, Hash Tables, Doubly Linked Lists**) in C++ to manage and visualize healthcare data, utilizing the **FLTK** library.

-----

##  Project Purpose

In the modern era, healthcare data is growing exponentially. Managing doctor-patient relationships and maintaining each patient's detailed medical history efficiently presents a significant **challenge**. This project addresses this problem by creating a small, robust EHR system in C++.

This system demonstrates how intelligently chosen fundamental data structures can be utilized to build a system that is **scalable, efficient, and well-organized**, offering both advanced querying and a visual interface.

-----

##  Advanced Features (GUI and Backend)

The system includes advanced querying, reporting, and visualization features beyond basic data management:

| Category | Feature | Description | Data Structure Benefit |
| :--- | :--- | :--- | :--- |
| **Interface** | **FLTK GUI** | The system runs in a graphical window with buttons and input fields, replacing the command-line menu. | Provides a simple, cross-platform Graphical User Experience (UX). |
| **Search** | **Smart Keyword Search** | Searches records (Symptoms, Diagnosis, Prescription) for a **partial** and **case-insensitive** keyword match. | **Efficient querying** over multiple linked records for highly relevant results. |
| **History** | **Enhanced Patient History** | Displays the **treating Doctor's Name, ID, and Specialization** alongside every medical record. | Links `MedicalRecord` (DLL) to `Doctor` (Hash Table) via `DoctorId`. |
| **Visualization** | **Link Tree** | Visually represents the **many-to-many relationships** between Doctors and Patients (the Graph) in an easy-to-read text-based tree format. | Direct representation of the **Graph (Adjacency List)**. |
| **Reporting** | **Tabular Data View** | Displays a summary of all Doctors, Patients, and Medical Records in a clear **tabular format**. | Efficient reporting by iterating over Hash Tables. |

-----

##  System Design and Data Structure Utilization (The "Why")

The core strength of this project lies in the strategic selection of its data structures, ensuring the system is both performant and structurally sound.

### 1\. Hash Table (`std::unordered_map`) ⚡

  - **Role**: Provides **instant access** to entities based on their unique ID.
  - **Why?**: Enables **O(1)** (constant time) average complexity for fetching any Doctor or Patient record, avoiding the need to search the entire list.

### 2\. Graph (Adjacency List) 

  - **Role**: Models the **many-to-many relationships** between doctors and patients.
  - **Why?**: In the real world, one doctor treats many patients, and one patient may see many doctors. The Graph structure (implemented as an Adjacency List) is the best way to represent this complex network.

### 3\. Doubly Linked List (`MedicalRecord`) 

  - **Role**: Stores a Patient's **chronological medical history** (timeline).
  - **Why?**: Linked Lists are perfect for maintaining sequential order. Adding a new record (node) to the end of the list is highly efficient (**O(1)**). The "Doubly" nature allows easy traversal both forward (latest history) and backward (oldest history).

-----

##  How to Compile and Run (FLTK Required)

Since this application uses the **FLTK (Fast Light Tool Kit)** library for its GUI, you must install the necessary dependencies before compiling and running the program.

### Step 1: Install FLTK Dependencies (Linux/Ubuntu)

Execute this command in your terminal to install the FLTK library and its development headers:

```sh
# Update dependencies and install the libfltk1.3-dev package
sudo apt update
sudo apt install libfltk1.3-dev
```

### Step 2: Compile the Code

1.  Save the C++ code into a file, for example, `ehr_gui.cpp`.

2.  Navigate to the directory where you saved the file.

3.  Use the **`fltk-config`** utility to dynamically add the required compilation flags. This command automatically links the FLTK headers and linker flags:

    ```sh
    g++ ehr_gui.cpp $(fltk-config --cxxflags --ldflags) -o ehr_gui -std=c++17
    ```

### Step 3: Run the Program

Once compilation is successful, run the program using the following command:

```sh
./ehr_gui
```

The system's **GUI window** will appear on your screen, allowing you to manage and query the data using the graphical elements.

-----

##  Code Structure

The code within the `ehr_gui.cpp` file is organized into three main sections:

1.  **Backend (Core Logic) Classes**:

      * `struct MedicalRecord`, `struct Patient`, `struct Doctor`: The fundamental data structures defining the entities.
      * `class EHRSystem`: The core class containing the Hash Tables, Graph (Adjacency List), and all data management functions.

2.  **Frontend (FLTK GUI)**:

      * Setup and configuration of FLTK widgets (`Fl_Window`, `Fl_Button`, `Fl_Input`) to create the user interface.
      * **Callback Functions**: These are triggered by GUI buttons and interface the user actions with the underlying `EHRSystem` class functions.

3.  **main() Function**:

      * Handles FLTK scheme and window initialization.
      * Loads **Sample Data** upon startup.
      * Manages the placement and configuration of all GUI widgets.
      * Initiates the GUI event loop using `Fl::run()`.