#include <iostream>
#include <thread>
#include <mutex>
#include <semaphore>
#include <chrono>

using namespace std;

// ===============================
// Shared Resources
// ===============================
mutex icuBedLock;          // Protects the ICU Bed
mutex dispenserLock;       // Protects the Medication Dispenser
mutex logLock;             // Prevents mixed console output

// Binary semaphore represents the ICU Treatment Room.
// Only ONE patient can be treated at a time.
binary_semaphore treatmentRoom(1);

// Shared resource status
int icuBedAvailable = 1;
int dispenserInUse = 0;

// ===============================
// Thread-safe logging
// ===============================
void logAction(const string& msg) {
    lock_guard<mutex> logGuard(logLock);
    cout << msg << endl;
    this_thread::sleep_for(chrono::milliseconds(50));
}

// ===============================
// Thread A - Dr. Reyes
// ===============================
void drReyes(int patientId) {

    cout << "\nThread A (Dr. Reyes): Patient "
         << patientId
         << " arrived." << endl;

    cout << "Thread A (Dr. Reyes): Waiting to enter ICU Treatment Room..."
         << endl;

    // Binary semaphore controls admission
    treatmentRoom.acquire();

    cout << "Thread A (Dr. Reyes): Entered ICU Treatment Room."
         << endl;

    // Consistent lock ordering prevents deadlock
    lock_guard<mutex> bedGuard(icuBedLock);
    cout << "Thread A (Dr. Reyes): Acquired ICU Bed Lock."
         << endl;

    lock_guard<mutex> dispenserGuard(dispenserLock);
    cout << "Thread A (Dr. Reyes): Acquired Medication Dispenser Lock."
         << endl;

    icuBedAvailable = 0;
    dispenserInUse = 1;

    logAction("Thread A (Dr. Reyes): Treating Patient "
              + to_string(patientId));

    // Resources become available again
    icuBedAvailable = 1;
    dispenserInUse = 0;

    cout << "Thread A (Dr. Reyes): Releasing ICU resources."
         << endl;

    treatmentRoom.release();

    cout << "Thread A (Dr. Reyes): Left ICU Treatment Room."
         << endl;

    cout << "Thread A (Dr. Reyes): Done."
         << endl;
}

// ===============================
// Thread B - Nars Dela Cruz
// ===============================
void narsDela(int patientId) {

    cout << "\nThread B (Nars Dela Cruz): Patient "
         << patientId
         << " arrived."
         << endl;

    cout << "Thread B (Nars Dela Cruz): Waiting to enter ICU Treatment Room..."
         << endl;

    treatmentRoom.acquire();

    cout << "Thread B (Nars Dela Cruz): Entered ICU Treatment Room."
         << endl;

    // SAME ORDER as Thread A
    lock_guard<mutex> bedGuard(icuBedLock);
    cout << "Thread B (Nars Dela Cruz): Acquired ICU Bed Lock."
         << endl;

    lock_guard<mutex> dispenserGuard(dispenserLock);
    cout << "Thread B (Nars Dela Cruz): Acquired Medication Dispenser Lock."
         << endl;

    icuBedAvailable = 0;
    dispenserInUse = 1;

    logAction("Thread B (Nars Dela Cruz): Treating Patient "
              + to_string(patientId));

    icuBedAvailable = 1;
    dispenserInUse = 0;

    cout << "Thread B (Nars Dela Cruz): Releasing ICU resources."
         << endl;

    treatmentRoom.release();

    cout << "Thread B (Nars Dela Cruz): Left ICU Treatment Room."
         << endl;

    cout << "Thread B (Nars Dela Cruz): Done."
         << endl;
}

// ===============================
// Thread C - Audit
// ===============================
void auditResources() {

    cout << "\nThread C (Audit): Checking resource status..."
         << endl;

    // Audit only needs to read shared resources.
    // It follows the SAME mutex order.
    lock_guard<mutex> bedGuard(icuBedLock);
    lock_guard<mutex> dispenserGuard(dispenserLock);

    logAction(
        "Thread C (Audit): ICU Bed Available = "
        + to_string(icuBedAvailable)
        + " | Medication Dispenser In Use = "
        + to_string(dispenserInUse));

    cout << "Thread C (Audit): Audit completed."
         << endl;
}

// ===============================
// Main
// ===============================
int main() {

    cout << "=============================================================\n";
    cout << "      OSPITAL NG MAYNILA - ICU RESOURCE MANAGEMENT SYSTEM\n";
    cout << "=============================================================\n";
    cout << "Synchronization Methods Used:\n";
    cout << "1. Binary Semaphore - ICU Treatment Room\n";
    cout << "2. Mutex - ICU Bed\n";
    cout << "3. Mutex - Medication Dispenser\n";
    cout << "=============================================================\n\n";

    thread t1(drReyes, 1);
    thread t2(narsDela, 2);
    thread t3(auditResources);

    t1.join();
    t2.join();
    t3.join();

    cout << "\n=============================================================\n";
    cout << "All threads completed successfully.\n";
    cout << "Final ICU Bed Available = " << icuBedAvailable << endl;
    cout << "Final Medication Dispenser In Use = " << dispenserInUse << endl;
    cout << "=============================================================\n";

    return 0;
}