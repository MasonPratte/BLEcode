#include<windows.h>
#include<stdio.h>
#include<string>
#include <iostream>
#include <fstream>

//using namespace std;

int Status;
DWORD bytecount = 0;//amount of bytes read or written
HANDLE ComPort;
COMMTIMEOUTS timeouts;
std::string usrreq = ""; //string var for request received from com port


void makepkt(char* packet, char* bytearray, int size, int startpoint) {//function to make 64 byte or smaller packets
    int i = 0;
    while (i<size) {
        packet[i] = bytearray[(startpoint + i)];
        i++;
    }

}

void ReadData() {
    char data[32] = { 0 };//char data received from port
    FlushFileBuffers(ComPort);

    if (!ReadFile(ComPort, data, 32, &bytecount, NULL)) {
        printf("error reading serial data");
    }

    //printf("%ld bytes read\n", sizeof(data));

    for (int i = 0; i < 32; i++) {
        //printf("%c", data[i]);
        if (data[i]>=33) {
            usrreq += data[i];
        }
    }
    //printf("\n");

    //usrreq = data;
    std::cout << "request: "<<usrreq <<"\n";
}

void SendData() {
    std::string filepath = ""; //string for storing selected file path
    int numpackets;//var for number of packets
    char packet[64] = { 0 }; //character packet to send
    int index = 0;
    int packetsize = 64;
    
    if (usrreq == "admin") {                  //if statements set file path
        filepath = "../keypadAdmin.html";
    }
    else if (usrreq == "beginner") {
        filepath = "../keypadBeginner.html";
    }
    else if (usrreq == "experience") {
        filepath = "../keypadExperience.html";
    }
    else if (usrreq == "styles") {
        filepath = "../styles/style.css";
    }
    else if (usrreq == "scripts") {
        filepath = "../scripts/keypad.js";
    }

    std::cout << filepath;
    std::ifstream file(filepath);

    file.seekg(0, std::ios::end);
    size_t len = file.tellg();
    char* buffer = new char[len];
    file.seekg(0, std::ios::beg);
    file.read(buffer, len);
    file.close();
    

    if (buffer == NULL)
        printf("failure converting file to bytes");
    else
        printf("file converted to bytes. Size: %d\n", len);

    numpackets = len / 64;
    printf("%d 64 byte packets made\n",numpackets);
    if ((len%64)!=0) {
        numpackets += 1;
        printf("1 non 64 byte packet made\n");
    }

    std::string tmp = std::to_string(len);
    char const* len_arr = tmp.c_str();    //converting length of file to char array to send to smart device

    if (!WriteFile(ComPort, len_arr, sizeof(len_arr), &bytecount, NULL)) {
        printf("error writing serial data");
    }
    else{
        printf("file size sent");
    }

    while (numpackets > 0) {

        if (numpackets>1 || (len%64)==0) {
            makepkt(packet, buffer, 64, index);
        }
        else {
            makepkt(packet, buffer, (len%64), index);
            packetsize = (len%64);
        }


        if (!WriteFile(ComPort, packet, packetsize , &bytecount, NULL)) {
            printf("error writing serial data");
        }
        printf("%ld bytes written\n", bytecount);
        numpackets--;
    }
    
    usrreq = ""; //resetting ussrreq var for next file request
}


int main(){

    ComPort = CreateFile(L"COM3",                //port name
        GENERIC_READ | GENERIC_WRITE, //Read/Write
        0,                            // No Sharing
        0,                         // No Security
        OPEN_EXISTING,// Open existing port only
        FILE_ATTRIBUTE_NORMAL,            // Non Overlapped I/O
        0);        // Null for Comm Devices

    if (ComPort == INVALID_HANDLE_VALUE)
        printf("Error in opening serial port\n");
    else
        printf("opening serial port successful\n");


    DCB serialParams = { 0 };
    serialParams.DCBlength = sizeof(serialParams);

    GetCommState(ComPort, &serialParams);
    serialParams.BaudRate = 9600;
    serialParams.ByteSize = 8;
    serialParams.StopBits = ONESTOPBIT;
    serialParams.Parity = NOPARITY;
    SetCommState(ComPort, &serialParams);

    // Set timeouts
    COMMTIMEOUTS timeout = { 0 };
    timeout.ReadIntervalTimeout = 500;
    timeout.ReadTotalTimeoutConstant = 500;
    timeout.ReadTotalTimeoutMultiplier = 50;
    timeout.WriteTotalTimeoutConstant = 500;
    timeout.WriteTotalTimeoutMultiplier = 100;

    SetCommTimeouts(ComPort, &timeout);

    Sleep(1);
    
    while (ComPort != INVALID_HANDLE_VALUE) {
        while (usrreq == "") {
            ReadData();
        }
        printf("finished reading request \n");
        SendData();
    }

    //printf("error in program");
    CloseHandle(ComPort);//Closing the Serial Port

    return 0;
}
