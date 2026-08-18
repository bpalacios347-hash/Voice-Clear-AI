#include "service/ServiceMain.h"

int main(int argc, char** argv) {
    return VoiceClear::Service::ServiceMain::GetInstance().Run(argc, argv);
}
