#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>

using namespace std;

mutex icuBedLock;
mutex dispenserLock;    
mutex logLock;

int icuBedAvailable  = 1;   // 1 = available, 0 = occupied
int dispenserInUse   = 0;   // 0 = free, 1 = in use

void logAction(const string& msg) {
    logLock.lock();
    cout << msg << endl;
    this_thread::sleep_for(chrono::milliseconds(50));
    logLock.unlock();
}

void drReyes(int patientId) {
    cout << "Thread A (Dr. Reyes): Patient " << patientId << " arrived - requesting resources..." << endl;

    cout << "Thread A (Dr. Reyes): locking ICU Bed No. 3..." << endl;
    icuBedLock.lock();                  // [PROBLEM LINE]
    cout << "Thread A (Dr. Reyes): ICU Bed LOCKED." << endl;

    this_thread::sleep_for(chrono::milliseconds(100));

    cout << "Thread A (Dr. Reyes): locking Medication Dispenser..." << endl;
    dispenserLock.lock();               // waits forever, Nars holds the lock
    cout << "Thread A (Dr. Reyes): Medication Dispenser LOCKED." << endl;

    icuBedAvailable = 0;
    dispenserInUse  = 1;
    logAction("Thread A (Dr. Reyes): treating Patient " + to_string(patientId));

    dispenserLock.unlock();
    icuBedLock.unlock();
    cout << "Thread A (Dr. Reyes): resources released. Done." << endl;
}

void narsDela(int patientId) {
    cout << "Thread B (Nars Dela Cruz): Patient " << patientId << " arrived - requesting resources..." << endl;

    cout << "Thread B (Nars Dela Cruz): locking Medication Dispenser..." << endl;
    dispenserLock.lock();               // [PROBLEM LINE]
    cout << "Thread B (Nars Dela Cruz): Medication Dispenser LOCKED." << endl;

    this_thread::sleep_for(chrono::milliseconds(100));

    cout << "Thread B (Nars Dela Cruz): locking ICU Bed No. 3..." << endl;
    icuBedLock.lock();                  // waits forever, Dr. Reyes holds the lock
    cout << "Thread B (Nars Dela Cruz): ICU Bed LOCKED." << endl;

    dispenserInUse  = 1;
    icuBedAvailable = 0;
    logAction("Thread B (Nars Dela Cruz): treating Patient " + to_string(patientId));

    icuBedLock.unlock();
    dispenserLock.unlock();
    cout << "Thread B (Nars Dela Cruz): resources released. Done." << endl;
}

void auditResources() {
    cout << "Thread C (Audit): locking log first..." << endl;
    logLock.lock();                     // [PROBLEM LINE]

    this_thread::sleep_for(chrono::milliseconds(50));

    cout << "Thread C (Audit): locking ICU Bed..." << endl;
    icuBedLock.lock();                  // [PROBLEM LINE]

    cout << "Thread C (Audit): locking Dispenser..." << endl;
    dispenserLock.lock();               // [PROBLEM LINE]

    cout << "Audit Report: ICU Bed Available = " << icuBedAvailable
         << " | Dispenser In Use = " << dispenserInUse << endl;

    dispenserLock.unlock();
    icuBedLock.unlock();
    logLock.unlock();
    cout << "Thread C (Audit): done." << endl;
}

int main() {
    cout << "============================================================" << endl;
    cout << "  OSPITAL NG MAYNILA — ICU Resource Management System" << endl;
    cout << "  MODE: UNSYNCHRONIZED (No resource ordering)" << endl;
    cout << "============================================================" << endl;
    cout << "ICU Bed Available = " << icuBedAvailable
         << " | Dispenser In Use = " << dispenserInUse << endl;
    cout << "------------------------------------------------------------" << endl;

    thread t1(drReyes,   1);   // Dr. Reyes  — Patient 1
    thread t2(narsDela,  2);   // Nars Dela Cruz — Patient 2
    thread t3(auditResources);

    t1.join();
    t2.join();
    t3.join();

    //  This block will never be reached 
    cout << "------------------------------------------------------------" << endl;
    cout << "All threads done." << endl;
    cout << "ICU Bed Available = " << icuBedAvailable
         << " | Dispenser In Use = " << dispenserInUse << endl;

    return 0;
}