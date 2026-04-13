#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Box.H>
#include <FL/fl_message.H>
#include <FL/fl_draw.H> // For drawing/colors

// ======================================================================
// 1. BACKEND: EHR System (Core Logic)
// ======================================================================

struct MedicalRecord {
    std::string date;
    std::string symptoms;
    std::string diagnosis;
    std::string prescription;
    std::string doctorId; // Field to track the treating doctor
    MedicalRecord *next;
    MedicalRecord *prev;

    MedicalRecord(std::string dt, std::string sym, std::string dx, std::string px, std::string docId)
        : date(dt), symptoms(sym), diagnosis(dx), prescription(px), doctorId(docId), next(nullptr), prev(nullptr) {}
};

struct Patient {
    std::string id;
    std::string name;
    MedicalRecord *historyHead;
    MedicalRecord *historyTail;

    Patient(std::string patientId, std::string patientName)
        : id(patientId), name(patientName), historyHead(nullptr), historyTail(nullptr) {}

    ~Patient() {
        MedicalRecord *current = historyHead;
        while (current != nullptr) {
            MedicalRecord *toDelete = current;
            current = current->next;
            delete toDelete;
        }
    }
};

struct Doctor {
    std::string id;
    std::string name;
    std::string specialization;

    Doctor(std::string docId, std::string docName, std::string spec)
        : id(docId), name(docName), specialization(spec) {}
};

// ----------------------------------------------------------------------

class EHRSystem {
private:
    std::unordered_map<std::string, Patient*> patients;
    std::unordered_map<std::string, Doctor*> doctors;
    std::unordered_map<std::string, std::vector<std::string>> adjList; // Graph for linkages

    // Utility function for case-insensitive partial search
    bool smartSearch(const std::string& text, const std::string& query) {
        std::string lowerText = text;
        std::string lowerQuery = query;
        std::transform(lowerText.begin(), lowerText.end(), lowerText.begin(), ::tolower);
        std::transform(lowerQuery.begin(), lowerQuery.end(), lowerQuery.begin(), ::tolower);
        return lowerText.find(lowerQuery) != std::string::npos;
    }

public:
    ~EHRSystem() {
        for (auto& pair : patients) delete pair.second;
        for (auto& pair : doctors) delete pair.second;
    }
    
    const std::unordered_map<std::string, Patient*>& getPatients() const { return patients; }
    const std::unordered_map<std::string, Doctor*>& getDoctors() const { return doctors; }
    const std::unordered_map<std::string, std::vector<std::string>>& getAdjList() const { return adjList; }
    
    Doctor* getDoctor(const std::string& docId) {
        if (doctors.count(docId)) return doctors.at(docId);
        return nullptr;
    }

    void addDoctor(const std::string& id, const std::string& name, const std::string& spec) {
        if (doctors.count(id)) { fl_message("Doctor ID %s already exists.", id.c_str()); return; }
        doctors[id] = new Doctor(id, name, spec);
        adjList[id] = {};
        fl_message("Doctor %s added successfully.", name.c_str());
    }

    void addPatient(const std::string& id, const std::string& name) {
        if (patients.count(id)) { fl_message("Patient ID %s already exists.", id.c_str()); return; }
        patients[id] = new Patient(id, name);
        adjList[id] = {};
        fl_message("Patient %s added successfully.", name.c_str());
    }

    void linkDoctorPatient(const std::string& docId, const std::string& patId) {
        if (!doctors.count(docId) || !patients.count(patId)) { fl_message("Invalid Doctor or Patient ID."); return; }
        for(const auto& linkedId : adjList.at(docId)) {
            if (linkedId == patId) { fl_message("Doctor %s and Patient %s already linked.", docId.c_str(), patId.c_str()); return; }
        }

        adjList[docId].push_back(patId);
        adjList[patId].push_back(docId);
        fl_message("Linked Doctor %s with Patient %s", docId.c_str(), patId.c_str());
    }

    void addMedicalRecord(const std::string& patId, const std::string& date,
                          const std::string& sym, const std::string& dx, const std::string& px,
                          const std::string& docId) {
        if (!patients.count(patId)) { fl_message("Patient not found."); return; }
        if (!doctors.count(docId)) { fl_message("Doctor not found for the record."); return; }
        
        bool isLinked = false;
        for(const auto& linkedId : adjList.at(docId)) {
            if (linkedId == patId) { isLinked = true; break; }
        }
        if (!isLinked) { fl_message("Doctor %s is not linked to Patient %s. Record not added.", docId.c_str(), patId.c_str()); return; }


        Patient* patient = patients.at(patId);
        MedicalRecord* newRec = new MedicalRecord(date, sym, dx, px, docId);

        if (!patient->historyHead) {
            patient->historyHead = newRec;
            patient->historyTail = newRec;
        } else {
            patient->historyTail->next = newRec;
            newRec->prev = patient->historyTail;
            patient->historyTail = newRec;
        }

        fl_message("Medical record added by Doctor %s for patient %s", docId.c_str(), patient->name.c_str());
    }

    std::string getPatientHistory(const std::string& patId) {
        std::ostringstream oss;

        if (!patients.count(patId)) return "Patient not found.";

        Patient* p = patients.at(patId);
        if (!p->historyHead) return "No medical records for " + p->name;

        oss << "Medical History for " << p->name << " (" << p->id << ")\n";
        oss << "-------------------------------------------\n";
        MedicalRecord* cur = p->historyHead;
        int count = 1;
        while (cur) {
            Doctor* treatingDoc = getDoctor(cur->doctorId);
            std::string docInfo = treatingDoc ? 
                (treatingDoc->name + " (ID: " + treatingDoc->id + ", Spec: " + treatingDoc->specialization + ")") : 
                ("Unknown Doctor (ID: " + cur->doctorId + ")");

            oss << "Record " << count++ << ":\n";
            oss << "Date: " << cur->date << "\n";
            oss << "Doctor: " << docInfo << "\n";
            oss << "Symptoms: " << cur->symptoms << "\n";
            oss << "Diagnosis: " << cur->diagnosis << "\n";
            oss << "Prescription: " << cur->prescription << "\n";
            oss << "-------------------------------------------\n";
            cur = cur->next;
        }
        return oss.str();
    }

    std::string findPatientsByKeyword(const std::string& keyword) {
        std::ostringstream oss;
        if (keyword.empty()) return "Please enter a keyword.";

        bool found = false;
        for (auto& pair : patients) {
            Patient* p = pair.second;
            MedicalRecord* cur = p->historyHead;
            while (cur) {
                if (smartSearch(cur->symptoms, keyword) || 
                    smartSearch(cur->diagnosis, keyword) || 
                    smartSearch(cur->prescription, keyword)) {
                    
                    oss << "- " << p->name << " (ID: " << p->id << ") - Match in Record on " << cur->date << "\n";
                    found = true;
                    break; // Move to the next patient once a match is found
                }
                cur = cur->next;
            }
        }

        if (!found)
            return "No patients or records found matching: " + keyword;
        return "Search Results for '" + keyword + "':\n" + oss.str();
    }

    std::string getAllDataInTable() {
        std::ostringstream oss;
        oss << std::left;

        // --- DOCTORS TABLE ---
        oss << "--- DOCTORS ---\n";
        oss << std::setw(10) << "ID" << std::setw(20) << "Name" << std::setw(20) << "Specialization" << "\n";
        oss << "-------------------------------------------------------------------\n";
        for (const auto& pair : doctors) {
            const Doctor* d = pair.second;
            oss << std::setw(10) << d->id << std::setw(20) << d->name << std::setw(20) << d->specialization << "\n";
        }

        // --- PATIENTS TABLE ---
        oss << "\n--- PATIENTS ---\n";
        oss << std::setw(10) << "ID" << std::setw(20) << "Name" << "\n";
        oss << "-------------------------------------------------------------------\n";
        for (const auto& pair : patients) {
            const Patient* p = pair.second;
            oss << std::setw(10) << p->id << std::setw(20) << p->name << "\n";
        }
        
        // --- MEDICAL RECORDS (Summary) ---
        oss << "\n--- MEDICAL RECORDS (Summary) ---\n";
        oss << std::setw(10) << "Pat ID" << std::setw(20) << "Patient Name" << std::setw(15) << "Latest Date" << std::setw(10) << "Records" << "\n";
        oss << "-------------------------------------------------------------------\n";
        for (const auto& pair : patients) {
            const Patient* p = pair.second;
            int count = 0;
            MedicalRecord* cur = p->historyHead;
            std::string latestDate = "N/A";
            while(cur) {
                latestDate = cur->date;
                count++;
                cur = cur->next;
            }
             oss << std::setw(10) << p->id << std::setw(20) << p->name << std::setw(15) << latestDate << std::setw(10) << count << "\n";
        }

        return oss.str();
    }
    
    std::string getLinkTree() {
        std::ostringstream oss;
        oss << "--- DOCTOR-PATIENT LINKAGES ---\n";
        
        bool foundLinks = false;
        for (const auto& pair : doctors) {
            const std::string& docId = pair.first;
            const Doctor* doc = pair.second;
            const std::vector<std::string>& linkedPatients = adjList.at(docId);

            if (!linkedPatients.empty()) {
                oss << "\nDOCTOR: " << doc->name << " (" << docId << ", " << doc->specialization << ")\n";
                oss << "  | \n";
                for (size_t i = 0; i < linkedPatients.size(); ++i) {
                    const std::string& patId = linkedPatients[i];
                    const Patient* pat = patients.at(patId);
                    
                    std::string prefix = (i == linkedPatients.size() - 1) ? "  \\--- " : "  +--- ";
                    oss << prefix << "PATIENT: " << pat->name << " (" << patId << ")\n";
                }
                foundLinks = true;
            }
        }
        
        if (!foundLinks) {
            return "No Doctor-Patient linkages found.";
        }
        return oss.str();
    }
};

// ======================================================================
// 2. FRONTEND: FLTK GUI - CALLBACKS
// ======================================================================

EHRSystem ehr;

void addRecordCallback(Fl_Widget*, void* data) {
    Fl_Input** in = (Fl_Input**)data;
    // in[0]=PatID, in[1]=Date, in[2]=Sym, in[3]=Dx, in[4]=Px, in[5]=DocID
    ehr.addMedicalRecord(in[0]->value(), in[1]->value(), in[2]->value(), in[3]->value(), in[4]->value(), in[5]->value());
}

void viewHistoryCallback(Fl_Widget*, void* data) {
    Fl_Input* in = (Fl_Input*)data;
    std::string result = ehr.getPatientHistory(in->value());
    fl_message("%s", result.c_str());
}

void smartSearchCallback(Fl_Widget*, void* data) {
    Fl_Input* in = (Fl_Input*)data;
    std::string result = ehr.findPatientsByKeyword(in->value());
    fl_message("%s", result.c_str());
}

void showAllDataCallback(Fl_Widget*, void*) {
    std::string result = ehr.getAllDataInTable();
    fl_message("%s", result.c_str());
}

// CORRECTION: Using FL_COURIER and a hardcoded size for maximum compatibility.
void showLinkTreeCallback(Fl_Widget*, void*) {
    std::string result = ehr.getLinkTree();
    
    // FL_COURIER is a standard monospaced font available in most FLTK environments.
    // Using a hardcoded size (14) to avoid Fl::normal_size() errors.
    fl_message_font(FL_COURIER, 14); 
    
    fl_message("%s", result.c_str());
    
    // Reset font to default (FL_HELVETICA and size 12)
    fl_message_font(FL_HELVETICA, 12); 
}

// ======================================================================
// 3. MAIN FUNCTION: GUI Setup with Sample Data
// ======================================================================

int main() {
    // --- FLTK Configuration for Modern Look ---
    Fl::scheme("plastic"); 
    Fl::set_color(FL_BACKGROUND_COLOR, 0xEEEEEE00);
    Fl::set_color(FL_BACKGROUND2_COLOR, 0xFFFFFF00);
    
    // --- Window Setup ---
    Fl_Window *win = new Fl_Window(650, 620, "Advanced Mini EHR System");
    win->color(FL_BACKGROUND_COLOR);
    
    // ------------------------------------------------------------------
    //  ADD SAMPLE DATA
    // ------------------------------------------------------------------
    ehr.addDoctor("D001", "Ronith", "Cardiologist");
    ehr.addDoctor("D002", "Harsimran", "Dermatologist");
    ehr.addPatient("P101", "Kapish");
    ehr.addPatient("P102", "Medhansh");
    
    ehr.linkDoctorPatient("D001", "P101");
    ehr.linkDoctorPatient("D002", "P101");
    ehr.linkDoctorPatient("D002", "P102");
    
    ehr.addMedicalRecord("P101", "2025-09-20", "Chest pain, dizziness", "Angina", "Aspirin", "D001");
    ehr.addMedicalRecord("P101", "2025-09-25", "Itchy rash on arm", "Eczema", "Hydrocortisone cream", "D002");
    ehr.addMedicalRecord("P102", "2025-09-24", "Dry skin, persistent rash", "Psoriasis", "Topical Steroids", "D002");
    // ------------------------------------------------------------------

    // --- Layout Constants ---
    const int PADDING = 20;
    const int WIDGET_H = 25;
    const int BUTTON_H = 35;
    const int LABEL_W = 120;
    const int INPUT_W = 180;
    const int COLUMN_GAP = 50;

    // ------------------------------------------------------------------
    // LEFT COLUMN (Doctor/Patient/Linking)
    // ------------------------------------------------------------------
    int x_left = PADDING;
    int y_cursor = PADDING;

    // --- SECTION: Add Doctor ---
    Fl_Box *doctorLabel = new Fl_Box(x_left, y_cursor, LABEL_W + INPUT_W, WIDGET_H, "Add Doctor");
    doctorLabel->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
    doctorLabel->labelsize(16);
    doctorLabel->labelfont(FL_BOLD);
    y_cursor += WIDGET_H + 5;

    Fl_Input *docId = new Fl_Input(x_left + LABEL_W, y_cursor, INPUT_W, WIDGET_H, "ID:");
    y_cursor += WIDGET_H + 5;
    Fl_Input *docName = new Fl_Input(x_left + LABEL_W, y_cursor, INPUT_W, WIDGET_H, "Name:");
    y_cursor += WIDGET_H + 5;
    Fl_Input *docSpec = new Fl_Input(x_left + LABEL_W, y_cursor, INPUT_W, WIDGET_H, "Specialization:");
    y_cursor += WIDGET_H + 5;
    
    Fl_Button *addDocBtn = new Fl_Button(x_left, y_cursor, LABEL_W + INPUT_W, BUTTON_H, "Add Doctor");
    addDocBtn->color(FL_DARK_CYAN);
    addDocBtn->labelcolor(FL_WHITE);
    Fl_Input* doctorInputs[3] = {docId, docName, docSpec};
    addDocBtn->callback([](Fl_Widget*, void* data){
        Fl_Input** in = (Fl_Input**)data;
        ehr.addDoctor(in[0]->value(), in[1]->value(), in[2]->value());
    }, doctorInputs);
    y_cursor += BUTTON_H + PADDING;

    // --- SECTION: Add Patient ---
    Fl_Box *patientLabel = new Fl_Box(x_left, y_cursor, LABEL_W + INPUT_W, WIDGET_H, "Add Patient");
    patientLabel->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
    patientLabel->labelsize(16);
    patientLabel->labelfont(FL_BOLD);
    y_cursor += WIDGET_H + 5;

    Fl_Input *patId = new Fl_Input(x_left + LABEL_W, y_cursor, INPUT_W, WIDGET_H, "ID:");
    y_cursor += WIDGET_H + 5;
    Fl_Input *patName = new Fl_Input(x_left + LABEL_W, y_cursor, INPUT_W, WIDGET_H, "Name:");
    y_cursor += WIDGET_H + 5;
    
    Fl_Button *addPatBtn = new Fl_Button(x_left, y_cursor, LABEL_W + INPUT_W, BUTTON_H, "Add Patient");
    addPatBtn->color(FL_DARK_CYAN);
    addPatBtn->labelcolor(FL_WHITE);
    Fl_Input* patientInputs[2] = {patId, patName};
    addPatBtn->callback([](Fl_Widget*, void* data){
        Fl_Input** in = (Fl_Input**)data;
        ehr.addPatient(in[0]->value(), in[1]->value());
    }, patientInputs);
    y_cursor += BUTTON_H + PADDING;

    // --- SECTION: Link Doctor & Patient ---
    Fl_Box *linkLabel = new Fl_Box(x_left, y_cursor, LABEL_W + INPUT_W, WIDGET_H, "Link Doctor & Patient");
    linkLabel->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
    linkLabel->labelsize(16);
    linkLabel->labelfont(FL_BOLD);
    y_cursor += WIDGET_H + 5;

    Fl_Input *linkDocId = new Fl_Input(x_left + LABEL_W, y_cursor, INPUT_W, WIDGET_H, "Doc ID:");
    y_cursor += WIDGET_H + 5;
    Fl_Input *linkPatId = new Fl_Input(x_left + LABEL_W, y_cursor, INPUT_W, WIDGET_H, "Pat ID:");
    y_cursor += WIDGET_H + 5;
    
    Fl_Button *linkBtn = new Fl_Button(x_left, y_cursor, LABEL_W + INPUT_W, BUTTON_H, "Link");
    linkBtn->color(FL_GRAY);
    Fl_Input* linkInputs[2] = {linkDocId, linkPatId};
    linkBtn->callback([](Fl_Widget*, void* data){
        Fl_Input** in = (Fl_Input**)data;
        ehr.linkDoctorPatient(in[0]->value(), in[1]->value());
    }, linkInputs);
    y_cursor += BUTTON_H + PADDING; 
    
    // --- NEW SECTION: Global Actions (Left Bottom) ---
    Fl_Box *globalLabel = new Fl_Box(x_left, y_cursor, LABEL_W + INPUT_W, WIDGET_H, "Global Actions");
    globalLabel->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
    globalLabel->labelsize(16);
    globalLabel->labelfont(FL_BOLD);
    y_cursor += WIDGET_H + 5;
    
    Fl_Button *showDataBtn = new Fl_Button(x_left, y_cursor, (LABEL_W + INPUT_W - 5)/2, BUTTON_H, "Show All Data");
    showDataBtn->color(FL_DARK_YELLOW);
    showDataBtn->labelcolor(FL_WHITE);
    showDataBtn->callback(showAllDataCallback);
    
    Fl_Button *linkTreeBtn = new Fl_Button(x_left + (LABEL_W + INPUT_W - 5)/2 + 5, y_cursor, (LABEL_W + INPUT_W - 5)/2, BUTTON_H, "Link Tree");
    linkTreeBtn->color(FL_DARK_GREEN);
    linkTreeBtn->labelcolor(FL_WHITE);
    linkTreeBtn->callback(showLinkTreeCallback);
    
    int max_y_left = y_cursor + BUTTON_H;


    // ------------------------------------------------------------------
    // RIGHT COLUMN (Medical Record / Queries)
    // ------------------------------------------------------------------
    int x_right = x_left + LABEL_W + INPUT_W + COLUMN_GAP;
    y_cursor = PADDING;

    // --- SECTION: Add Medical Record ---
    Fl_Box *recLabel = new Fl_Box(x_right, y_cursor, LABEL_W + INPUT_W, WIDGET_H, "Add Medical Record");
    recLabel->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
    recLabel->labelsize(16);
    recLabel->labelfont(FL_BOLD);
    y_cursor += WIDGET_H + 5;

    Fl_Input *recPatId = new Fl_Input(x_right + LABEL_W, y_cursor, INPUT_W, WIDGET_H, "Pat ID:");
    y_cursor += WIDGET_H + 5;
    Fl_Input *recDocId = new Fl_Input(x_right + LABEL_W, y_cursor, INPUT_W, WIDGET_H, "Doc ID (Treating):");
    y_cursor += WIDGET_H + 5;
    Fl_Input *recDate = new Fl_Input(x_right + LABEL_W, y_cursor, INPUT_W, WIDGET_H, "Date (YYYY-MM-DD):");
    y_cursor += WIDGET_H + 5;
    Fl_Input *recSym = new Fl_Input(x_right + LABEL_W, y_cursor, INPUT_W, WIDGET_H, "Symptoms:");
    y_cursor += WIDGET_H + 5;
    Fl_Input *recDx = new Fl_Input(x_right + LABEL_W, y_cursor, INPUT_W, WIDGET_H, "Diagnosis:");
    y_cursor += WIDGET_H + 5;
    Fl_Input *recPx = new Fl_Input(x_right + LABEL_W, y_cursor, INPUT_W, WIDGET_H, "Prescription:");
    y_cursor += WIDGET_H + 5;

    Fl_Button *recBtn = new Fl_Button(x_right, y_cursor, LABEL_W + INPUT_W, BUTTON_H, "Add Record");
    recBtn->color(FL_DARK_CYAN);
    recBtn->labelcolor(FL_WHITE);
    Fl_Input* recInputs[6] = {recPatId, recDocId, recDate, recSym, recDx, recPx};
    recBtn->callback(addRecordCallback, recInputs);
    y_cursor += BUTTON_H + PADDING;
    
    // --- SECTION: Query Tools ---
    
    // Separator line
    Fl_Box *separator = new Fl_Box(x_right, y_cursor, LABEL_W + INPUT_W, 2);
    separator->box(FL_FLAT_BOX);
    separator->color(FL_GRAY);
    y_cursor += 10;
    
    Fl_Box *queryLabel = new Fl_Box(x_right, y_cursor, LABEL_W + INPUT_W, WIDGET_H, "Query Tools");
    queryLabel->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
    queryLabel->labelsize(16);
    queryLabel->labelfont(FL_BOLD);
    y_cursor += WIDGET_H + 5;

    // View History
    Fl_Input *viewPatId = new Fl_Input(x_right + LABEL_W, y_cursor, INPUT_W, WIDGET_H, "Pat ID (View History):");
    y_cursor += WIDGET_H + 5;
    Fl_Button *viewBtn = new Fl_Button(x_right, y_cursor, LABEL_W + INPUT_W, BUTTON_H, "View History");
    viewBtn->callback(viewHistoryCallback, viewPatId);
    y_cursor += BUTTON_H + PADDING;

    // Find by Keyword (Smart Search)
    Fl_Input *symInput = new Fl_Input(x_right + LABEL_W, y_cursor, INPUT_W, WIDGET_H, "Keyword (Smart Search):");
    symInput->tooltip("Searches records by partial, case-insensitive match in Symptoms, Diagnosis, or Prescription.");
    y_cursor += WIDGET_H + 5;
    Fl_Button *symBtn = new Fl_Button(x_right, y_cursor, LABEL_W + INPUT_W, BUTTON_H, "Smart Find Patients");
    symBtn->color(FL_DARK_BLUE);
    symBtn->labelcolor(FL_WHITE);
    symBtn->callback(smartSearchCallback, symInput);
    
    // Finalize window
    int window_height = std::max(max_y_left, y_cursor + BUTTON_H) + PADDING;
    win->size(x_right + LABEL_W + INPUT_W + PADDING, window_height);
    win->end();
    win->show();
    return Fl::run();
}