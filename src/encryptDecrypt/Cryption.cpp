#include "Cryption.hpp"
#include "../processes/Task.hpp"
#include "../fileHandling/ReadEnv.cpp"
#include<ctime>
#include<iomanip>

using namespace std;

int executeCryption(const string& taskData) {
    Task task = Task::fromString(taskData);
    ReadEnv env;
    string envKey = env.getenv();
    int key = stoi(envKey);
    
    stringstream buffer;
    buffer << task.f_stream.rdbuf();
    string content = buffer.str(); 

    for (char& ch : content) {
        if (task.action == Action::ENCRYPT) {
            ch = (ch + key) % 256;
        } else {
            ch = (ch - key + 256) % 256;
        }
    }

    // Write modified content back to the file
    task.f_stream.clear();
    task.f_stream.seekp(0);
    task.f_stream << content;
    task.f_stream.close();

    time_t t= time(nullptr);
    tm* now=localtime(&t);
    cout<<"Exiting encryption/decryption at: "<< put_time(now, "%Y-%m-%d %H:%M:%S") <<endl;
    
    return 0;
}