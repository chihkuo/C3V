#include "dbus_client.h"
#include "DBusDefinition.h"

#define MODE 0

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <function> <value> [<function> <value> ...]" << std::endl;
        return 1;
    }

    DBusClient client;
    int srcIndex = std::atoi(argv[1]);

#if MODE == 0
    std::string command = argv[2];
    if (argc == 3) {
        int value;
    	int status = client.getMethod(command.c_str(), srcIndex, value);
    	printf(">> %s, status=%d, index=%d, value=%d\n", command.c_str(), status, srcIndex, value);
    }
    else {
    	int value = std::atoi(argv[3]);
    	int status = client.setMethod(command.c_str(), srcIndex, value);
    	printf("<< %s, status=%d, index=%d, value=%d\n", command.c_str(), status, srcIndex, value);
    }
#elif MODE == 1
    // Get Algo Data
    int mode = std::atoi(argv[2]);
    std::string data;
    int status;
    if (mode == 1)
        status = client.getTrack1Data(srcIndex, data);
    else if (mode == 2)
        status = client.getTrack2Data(srcIndex, data);
    else
        status = client.getDetectionData(srcIndex, data);

    printf("<< mode=%d, status=%d, index=%d\n", mode, status, srcIndex);
    printf("data: %s\n", data.c_str());
#elif MODE == 2
    // Set Tracking ROI  
    int x = std::atoi(argv[2]);
    int y = std::atoi(argv[3]);
    int width = std::atoi(argv[4]);
    int height = std::atoi(argv[5]);
    int status = client.setTrackingRoi(srcIndex, x, y, width, height);
    printf("SetTrackingRoi, status=%d, index=%d, x=%d, y=%d, w=%d, h=%d\n", status, srcIndex, x, y, width, height);
    status = client.setAiTrigger(srcIndex, 3);
    printf("SetAiTrigger 3, status=%d, index=%d\n", status, srcIndex);
#endif

    return 0;
}

