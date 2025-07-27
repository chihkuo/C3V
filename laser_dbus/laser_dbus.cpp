#include <iostream>
#include <csignal>
#include <cstring>
#include <mutex>
#include <condition_variable>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <vector>
#include <optional>
#include <chrono>
#include <thread>
#include <atomic>


#include "dbus_client.h"
#include "DBusDefinition.h"

std::atomic<bool> running(true);
std::mutex mtx;
std::condition_variable cv;
bool newDataReady = false;

void signalHandler(int signum) {
    std::cout << "\nInterrupt signal (" << signum << ") received. Exiting...\n";
    running = false;
    cv.notify_all();
}

class LaserRangefinder {
public:
    LaserRangefinder(const char* portName) : serialPort(-1) {
        serialPort = setupSerialPort(portName);
        if (serialPort == -1) {
            DBusClient client;
            client.setLaserDistance(1);
            throw std::runtime_error("Failed to open serial port.");
        } else {
            std::cout << "Successed to open serial port." << std::endl;
        }
    }

    ~LaserRangefinder() {
        if (serialPort != -1) {
            close(serialPort);
        }
    }

    bool startMeasurement() {
        std::vector<uint8_t> startCommand = { 0xAE, 0xA7, 0x04, 0x00, 0x0E, 0x12, 0xBC, 0xBE };
        sendCommand(startCommand);
        return true;//receiveAck();
    }

    bool stopMeasurement() {
        std::vector<uint8_t> stopCommand = { 0xAE, 0xA7, 0x04, 0x00, 0x0F, 0x13, 0xBC, 0xBE };
        sendCommand(stopCommand);
        return true;
    }

    bool getDistance(short& distance) {
        return receiveData(distance);
    }

private:
    int serialPort;

    short bytesToShort(uint8_t byte1, uint8_t byte2) {
        return static_cast<short>((byte2 << 8) | byte1);
    }

    int setupSerialPort(const char* portName) {
        int sp = open(portName, O_RDWR | O_NOCTTY | O_NDELAY);
        if (sp == -1) {
            std::cerr << "Error opening " << portName << std::endl;
            return -1;
        }

        termios options;
        memset(&options, 0, sizeof(options));

        cfsetispeed(&options, B9600);
        cfsetospeed(&options, B9600);
        options.c_cflag |= (CLOCAL | CREAD);
        options.c_cflag &= ~PARENB;
        options.c_cflag &= ~CSTOPB;
        options.c_cflag &= ~CSIZE;
        options.c_cflag |= CS8;

        options.c_cc[VMIN] = 0;
        options.c_cc[VTIME] = 5;

        tcsetattr(sp, TCSANOW, &options);

        return sp;
    }

    void sendCommand(const std::vector<uint8_t>& command) {
        write(serialPort, command.data(), command.size());
    }

    bool receiveBytes(std::vector<uint8_t>& buffer, size_t length) {
        while (buffer.size() < length) {
            uint8_t byte;
            ssize_t bytesRead = read(serialPort, &byte, 1);
            if (bytesRead == 1) {
                buffer.push_back(byte);
            } else {
                continue;
            }
        }
        return true;
    }

    bool receiveAck() {
        std::vector<uint8_t> ackBuffer;
        if (receiveBytes(ackBuffer, 8)) {
            if (ackBuffer == std::vector<uint8_t>{0xAE, 0xA7, 0x04, 0x00, 0x8E, 0x92, 0xBC, 0xBE}) {
                std::cout << "Ack received successfully." << std::endl;
                return true;
            } else {
                std::cerr << "Ack received failed." << std::endl;
                return false;
            }
        }
        return false;
    }

    bool receiveData(short& distance) {
        std::vector<uint8_t> buffer;
        buffer.reserve(100);
        uint8_t byte = 0;

        static int timeoutCount = 0;
        auto startTime = std::chrono::steady_clock::now();
        const auto timeout = std::chrono::milliseconds(200);

        while (true) {
            ssize_t bytesRead = read(serialPort, &byte, 1);
            if (bytesRead == 1) {
                buffer.push_back(byte);
                if (buffer.size() >= 2 && buffer[buffer.size() - 2] == 0xAE && buffer[buffer.size() - 1] == 0xA7) {
                    break;
                }
            }

            if (std::chrono::steady_clock::now() - startTime > timeout) {
                //std::cerr << "Timeout waiting for data header." << std::endl;
                if (++timeoutCount > 10) {
                    distance = -1;
                    return true;
                }

                return false;
            }
        }

        while (true) {
            ssize_t bytesRead = read(serialPort, &byte, 1);
            if (bytesRead == 1) {
                buffer.push_back(byte);
                if (byte == 0xBE) {
                    break;
                }
            }

            if (std::chrono::steady_clock::now() - startTime > timeout) {
                //std::cerr << "Timeout waiting for data end." << std::endl;
                if (++timeoutCount > 10) {
                    distance = -1;
                    return true;
                }

                return false;
            }
        }

        if (buffer.size() >= 10) {
            timeoutCount = 0;
            distance = bytesToShort(buffer[8], buffer[7]);
            return distance > 0;
        } else {
            return false;
        }
    }
};

void laserDistanceThread(LaserRangefinder& rangefinder, std::atomic<short>& distance) {
    short tempDistance = 0;

    while (running) {
        try {
            if (rangefinder.getDistance(tempDistance)) {
                distance = tempDistance;
            } else {
                distance = 0;
            }
        } catch (const std::exception& e) {
            std::cerr << "Error reading from serial port: " << e.what() << std::endl;
            distance = 0;
        }

        {
            std::lock_guard<std::mutex> lock(mtx);
            newDataReady = true;
        }
        cv.notify_one();

        std::unique_lock<std::mutex> lock(mtx);
        cv.wait_for(lock, std::chrono::milliseconds(40), [] { return !running; });
    }
}

int main(int argc, char** argv) {
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);

    const char* portName = "/dev/ttyS2"; 
    std::unique_ptr<LaserRangefinder> rangefinder = nullptr;
    std::atomic<short> distance(0);
    std::thread laserThread;

    try {
        rangefinder = std::make_unique<LaserRangefinder>(portName);
        if (!rangefinder->startMeasurement()) {
            std::cerr << "Error: Failed to start laser measurement." << std::endl;
            return -1;
        }
        laserThread = std::thread(laserDistanceThread, std::ref(*rangefinder), std::ref(distance));
    } catch (const std::runtime_error& e) {
        std::cerr << e.what() << std::endl;
        return -1;
    }

    short tmpDistance = 0;
    DBusClient client;
    while (running) {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [] { return newDataReady || !running; });
        if (!running) break;
        newDataReady = false;

        if (tmpDistance != distance) {
            tmpDistance = distance;
            client.setLaserDistance(tmpDistance);
            //std::cout << "Distance: " << static_cast<int>(tmpDistance) << std::endl;
        }
    }

    if (rangefinder) {
        rangefinder->stopMeasurement();
    }

    if (laserThread.joinable()) {
        laserThread.join();
    }

    std::cout << "LaserRangefinder Program terminated." << std::endl;
    return 0;
}
