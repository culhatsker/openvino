#include <stdint.h>
#include <string>
#include <iostream>
#include <fstream>


#ifdef _WIN32
#include <windows.h>
#include <tlhelp32.h>

static PROCESS_MEMORY_COUNTERS getMemoryInfo() {
    static PROCESS_MEMORY_COUNTERS pmc;
    pmc.cb = sizeof(PROCESS_MEMORY_COUNTERS);
    if (!GetProcessMemoryInfo(GetCurrentProcess(), &pmc, pmc.cb))
        throw std::runtime_error("Can't get system memory values");
    return pmc;
}

static size_t getThreadsNum() {
    // first determine the id of the current process
    DWORD const id = GetCurrentProcessId();

    // then get a process list snapshot.
    HANDLE const snapshot = CreateToolhelp32Snapshot( TH32CS_SNAPALL, 0 );

    // initialize the process entry structure.
    PROCESSENTRY32 entry = { 0 };
    entry.dwSize = sizeof( entry );

    // get the first process info.
    BOOL  ret = true;
    ret = Process32First( snapshot, &entry );
    while( ret && entry.th32ProcessID != id ) {
        ret = Process32Next( snapshot, &entry );
    }
    CloseHandle( snapshot );
    return ret
        ?   entry.cntThreads
        :   -1;
}
#endif


struct MemoryCounters {
    // memory size in kb
    int64_t virtual_size = -1;
    int64_t virtual_peak = -1;
    int64_t resident_size = -1;
    int64_t resident_peak = -1;

    int32_t thread_count = -1;

    static MemoryCounters sample() {
        MemoryCounters out;
#ifdef _WIN32
        // windows stats
        auto meminfo = getMemoryInfo();
        out.virtual_size = meminfo.PagefileUsage / 1024;
        out.virtual_peak = meminfo.PeakPagefileUsage / 1024;
        out.resident_size = meminfo.WorkingSetSize / 1024;
        out.resident_peak = meminfo.PeakWorkingSetSize / 1024;
        out.thread_count = (int32_t) getThreadsNum();
#else
        // unix stats
        std::ifstream file;
        file.open("/proc/self/status");
        std::string line;
        while (true) {
            if (!std::getline(file, line)) {
                break;
            }
            int delim_pos = line.find(':');
            if (delim_pos == std::string::npos) {
                continue;
            }
            auto prefix = line.substr(0, delim_pos);
            int value_start = line.find_first_not_of("\t ", delim_pos + 1); 
            int value_end = line.find_first_of("\t ", value_start);
            if (value_start == std::string::npos) {
                continue;
            }
            auto value = line.substr(value_start, value_end - value_start);
            long ivalue = std::atol(value.c_str());
            if (prefix == "VmSize") {
                out.virtual_size = ivalue;
            } else if (prefix == "VmPeak") {
                out.virtual_peak = ivalue;
            } else if (prefix == "VmRSS") {
                out.resident_size = ivalue;
            } else if (prefix == "VmHWM") {
                out.resident_peak = ivalue;
            } else if (prefix == "Threads") {
                out.thread_count = (int32_t) ivalue;
            } 
        }
#endif
        return out;
    }

    MemoryCounters subtract_base(MemoryCounters &base) {
        return {
            virtual_size - base.virtual_size,
            virtual_peak - base.virtual_peak,
            resident_size - base.resident_size,
            resident_peak - base.resident_peak,
            thread_count // doesn't make sense to subtract it
        };
    }
};

struct TestContext {
    std::string model_path;
    std::string device;
    MemoryCounters memstart;

    static TestContext from_args(int argc, char **argv) {
        std::string model_path;
        std::string device = "CPU";

        if (argc <= 1 || argc > 3) {
            std::cerr << "Usage: <executable> MODEL_PATH [DEVICE=CPU]" << std::endl;
            exit(-1);
        }
        if (argc > 1) {
            model_path = argv[1];
            // assert path exists
        }
        if (argc > 2) {
            device = argv[2];
            // assert known string
        }

        return {model_path, device};
    }

    void start() {
        memstart = MemoryCounters::sample();
    }
    
    void end() {
        auto memdiff = MemoryCounters::sample().subtract_base(memstart);
        // auto model_path = json_escape(model_path);  // required for Windows
        std::cout << "TEST_RESULT: {"
        << "\"model_path\": \"" << model_path << "\", "
        << "\"device\": \"" << device << "\", "

        << "\"virtual_size\": " << memdiff.virtual_size << ", "
        << "\"virtual_peak\": " << memdiff.virtual_peak << ", "
        << "\"resident_size\": " << memdiff.resident_size << ", "
        << "\"resident_peak\": " << memdiff.resident_peak << ", "
        << "\"thread_count\": " << memdiff.thread_count
        << "}" << std::endl;
    }
};


void do_test(TestContext &test);


int main(int argc, char **argv) {
    TestContext test = TestContext::from_args(argc, argv);
    do_test(test);
}
