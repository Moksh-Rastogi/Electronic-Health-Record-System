
#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <limits>
#include <algorithm>
using namespace std;

// To handle the newline character left by `cin`
void clearInputBuffer() {
     cin.ignore( numeric_limits< streamsize>::max(), '\n');
}

// ======================================================================
// 1. DATA STRUCTURE DEFINITIONS
// ======================================================================

// Node for the Doubly Linked List (stores a single medical visit)
struct MedicalRecord {
     string date;
     string symptoms;
     string diagnosis;
     string prescription;
    MedicalRecord *next;
    MedicalRecord *prev;

    // Constructor for easy creation
    MedicalRecord( string dt,  string sym,  string dx,  string px)
        : date(dt), symptoms(sym), diagnosis(dx), prescription(px), next(nullptr), prev(nullptr) {}
};

// Patient Structure
struct Patient {
     string id;
     string name;
    MedicalRecord *historyHead; // Head of the doubly linked list
    MedicalRecord *historyTail; // Tail for efficient appending

    Patient( string patientId,  string patientName)
        : id(patientId), name(patientName), historyHead(nullptr), historyTail(nullptr) {}

    // Destructor to free the linked list memory
    ~Patient() {
        MedicalRecord *current = historyHead;
        while (current != nullptr) {
            MedicalRecord *toDelete = current;
            current = current->next;
            delete toDelete;
        }
    }
};

// Doctor Structure
struct Doctor {
     string id;
     string name;
     string specialization;

    Doctor( string docId,  string docName,  string spec)
        : id(docId), name(docName), specialization(spec) {}
};

// ======================================================================
// 2. MAIN EHR SYSTEM CLASS
// ======================================================================

class EHRSystem {
private:
    // Hash Tables for fast O(1) average time access
     unordered_map< string, Patient*> patients;
     unordered_map< string, Doctor*> doctors;

    // Graph (Adjacency List) to represent doctor-patient relationships
     unordered_map< string,  vector< string>> adjList;

public:
    // Destructor to clean up all dynamically allocated memory
    ~EHRSystem() {
        for (auto const& [id, patientPtr] : patients) {
            delete patientPtr;
        }
        for (auto const& [id, doctorPtr] : doctors) {
            delete doctorPtr;
        }
    }

    // --- Core Functionalities ---

    void addDoctor(const  string& id, const  string& name, const  string& specialization) {
        if (doctors.find(id) != doctors.end()) {
             cout << "Error: Doctor with ID '" << id << "' already exists.\n";
            return;
        }
        Doctor* newDoctor = new Doctor(id, name, specialization);
        doctors[id] = newDoctor;
        adjList[id] = {}; // Initialize adjacency list for the new doctor
         cout << "✅ Doctor '" << name << "' added successfully.\n";
    }

    void addPatient(const  string& id, const  string& name) {
        if (patients.find(id) != patients.end()) {
             cout << "Error: Patient with ID '" << id << "' already exists.\n";
            return;
        }
        Patient* newPatient = new Patient(id, name);
        patients[id] = newPatient;
        adjList[id] = {}; // Initialize adjacency list for the new patient
         cout << "✅ Patient '" << name << "' added successfully.\n";
    }

    void linkDoctorPatient(const  string& docId, const  string& patientId) {
        if (doctors.find(docId) == doctors.end()) {
             cout << "Error: Doctor with ID '" << docId << "' not found.\n";
            return;
        }
        if (patients.find(patientId) == patients.end()) {
             cout << "Error: Patient with ID '" << patientId << "' not found.\n";
            return;
        }

        // Add edge in both directions for the many-to-many relationship
        adjList[docId].push_back(patientId);
        adjList[patientId].push_back(docId);
         cout << "🔗 Successfully linked Dr. " << doctors[docId]->name << " and Patient " << patients[patientId]->name << ".\n";
    }

    void addMedicalRecord(const  string& patientId, const  string& date, const  string& symptoms, const  string& diagnosis, const  string& prescription) {
        if (patients.find(patientId) == patients.end()) {
             cout << "Error: Patient with ID '" << patientId << "' not found.\n";
            return;
        }

        Patient* patient = patients[patientId];
        MedicalRecord* newRecord = new MedicalRecord(date, symptoms, diagnosis, prescription);

        // Append to the patient's doubly linked list
        if (patient->historyHead == nullptr) {
            // List is empty
            patient->historyHead = newRecord;
            patient->historyTail = newRecord;
        } else {
            // Append to the end
            patient->historyTail->next = newRecord;
            newRecord->prev = patient->historyTail;
            patient->historyTail = newRecord;
        }
         cout << "📝 Medical record added for patient '" << patient->name << "'.\n";
    }

    // --- Display and Search Functionalities ---

    void displayPatientHistory(const  string& patientId) {
        if (patients.find(patientId) == patients.end()) {
             cout << "Error: Patient with ID '" << patientId << "' not found.\n";
            return;
        }

        Patient* patient = patients[patientId];
         cout << "\n--- Medical History for " << patient->name << " (ID: " << patient->id << ") ---\n";
        
        if (patient->historyHead == nullptr) {
             cout << "No medical records found.\n";
            return;
        }

        MedicalRecord* current = patient->historyHead;
        int recordCount = 1;
        while (current != nullptr) {
             cout << "Record " << recordCount++ << ":\n";
             cout << "  Date:         " << current->date << "\n";
             cout << "  Symptoms:     " << current->symptoms << "\n";
             cout << "  Diagnosis:    " << current->diagnosis << "\n";
             cout << "  Prescription: " << current->prescription << "\n";
             cout << "-------------------------------------------\n";
            current = current->next;
        }
    }
    
    void displayPatientInfo(const  string& patientId) {
        if (patients.find(patientId) == patients.end()) {
             cout << "Error: Patient with ID '" << patientId << "' not found.\n";
            return;
        }
        Patient* p = patients[patientId];
         cout << "\n--- Patient Information ---\n";
         cout << "ID:   " << p->id << "\n";
         cout << "Name: " << p->name << "\n";
         cout << "Consulted Doctors:\n";
        
        bool hasDoctors = false;
        for (const auto& connectedId : adjList[patientId]) {
            if (doctors.count(connectedId)) { // Check if the connected node is a doctor
                 cout << " - Dr. " << doctors[connectedId]->name << " (" << doctors[connectedId]->specialization << ")\n";
                hasDoctors = true;
            }
        }
        if (!hasDoctors) {
             cout << " - None on record.\n";
        }
         cout << "---------------------------\n";
    }

    void findPatientsBySymptom(const  string& symptom) {
         vector<Patient*> foundPatients;
         string lowerSymptom = symptom;
         transform(lowerSymptom.begin(), lowerSymptom.end(), lowerSymptom.begin(), ::tolower);

        // Iterate through all patients (hash table)
        for (auto const& [id, patientPtr] : patients) {
            // Iterate through their medical history (linked list)
            MedicalRecord* current = patientPtr->historyHead;
            while (current != nullptr) {
                 string lowerRecordSymptoms = current->symptoms;
                 transform(lowerRecordSymptoms.begin(), lowerRecordSymptoms.end(), lowerRecordSymptoms.begin(), ::tolower);
                
                // Simple substring search
                if (lowerRecordSymptoms.find(lowerSymptom) !=  string::npos) {
                    foundPatients.push_back(patientPtr);
                    break; // Found a match, no need to check other records for this patient
                }
                current = current->next;
            }
        }

         cout << "\n--- Patients with symptom: '" << symptom << "' ---\n";
        if (foundPatients.empty()) {
             cout << "No patients found with this symptom.\n";
        } else {
            for (const auto& patient : foundPatients) {
                 cout << " - " << patient->name << " (ID: " << patient->id << ")\n";
            }
        }
         cout << "-------------------------------------------\n";
    }
};

// ======================================================================
// 3. USER INTERFACE (MENU)
// ======================================================================

void printMenu() {
     cout << "\n===== Miniature EHR System Menu =====\n";
     cout << "1. Add Doctor\n";
     cout << "2. Add Patient\n";
     cout << "3. Link Doctor and Patient\n";
     cout << "4. Add Patient Medical Record\n";
     cout << "5. View Patient Medical History\n";
     cout << "6. View Patient Info and Doctors\n";
     cout << "7. Find Patients by Symptom\n";
     cout << "0. Exit\n";
     cout << "=====================================\n";
     cout << "Enter your choice: ";
}

int main() {
    EHRSystem ehr;
    int choice;

    
    ehr.addDoctor("D001", "Ronith", "Cardiologist");
    ehr.addDoctor("D002", "Harsimran", "Dermatologist");
    ehr.addPatient("P101", "Kapish");
    ehr.addPatient("P102", "Medhansh");
    ehr.linkDoctorPatient("D001", "P101");
    ehr.linkDoctorPatient("D002", "P101");
    ehr.linkDoctorPatient("D002", "P102");
    ehr.addMedicalRecord("P101", "2025-09-20", "Chest pain, dizziness", "Angina", "Aspirin");
    ehr.addMedicalRecord("P101", "2025-09-25", "Itchy rash on arm", "Eczema", "Hydrocortisone cream");
    ehr.addMedicalRecord("P102", "2025-09-24", "Dry skin, persistent rash", "Psoriasis", "Topical Steroids");
    
     cout << "\n--- System pre-populated with sample data. ---";

    do {
        printMenu();
         cin >> choice;
        clearInputBuffer(); 
         string id, name, spec, docId, patId, date, sym, dx, px;

        switch (choice) {
            case 1:
                 cout << "Enter Doctor ID: ";
                 getline( cin, id);
                 cout << "Enter Doctor Name: ";
                 getline( cin, name);
                 cout << "Enter Specialization: ";
                 getline( cin, spec);
                ehr.addDoctor(id, name, spec);
                break;
            case 2:
                 cout << "Enter Patient ID: ";
                 getline( cin, id);
                 cout << "Enter Patient Name: ";
                 getline( cin, name);
                ehr.addPatient(id, name);
                break;
            case 3:
                 cout << "Enter Doctor ID: ";
                 getline( cin, docId);
                 cout << "Enter Patient ID: ";
                 getline( cin, patId);
                ehr.linkDoctorPatient(docId, patId);
                break;
            case 4:
                 cout << "Enter Patient ID: ";
                 getline( cin, patId);
                 cout << "Enter Date (YYYY-MM-DD): ";
                 getline( cin, date);
                 cout << "Enter Symptoms: ";
                 getline( cin, sym);
                 cout << "Enter Diagnosis: ";
                 getline( cin, dx);
                 cout << "Enter Prescription: ";
                 getline( cin, px);
                ehr.addMedicalRecord(patId, date, sym, dx, px);
                break;
            case 5:
                 cout << "Enter Patient ID to view history: ";
                 getline( cin, patId);
                ehr.displayPatientHistory(patId);
                break;
            case 6:
                 cout << "Enter Patient ID to view info: ";
                 getline( cin, patId);
                ehr.displayPatientInfo(patId);
                break;
            case 7:
                 cout << "Enter symptom to search for: ";
                 getline( cin, sym);
                ehr.findPatientsBySymptom(sym);
                break;
            case 0:
                 cout << "Exiting system. Goodbye!\n";
                break;
            default:
                 cout << "Invalid choice. Please try again.\n";
                break;
        }

    } while (choice != 0);

    return 0;
}