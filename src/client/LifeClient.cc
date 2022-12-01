#include "client/LifeClient.h"

LifeClient::LifeClient(IPNetwork *net, const char *servername, short port, bool wait)
    : net(net), server(IPAddress(servername, port)) {}

LifeClient::~LifeClient() {}

int LifeClient::start() {
    char buffer[100];
    LOG(INFO) << "Loggin into server...";
    LogonMessage *request = LogonMessage::createRequest(getNextSequenceNumber());
    ssize_t received_bytes = net->request(server, request, sizeof(LogonMessage), buffer, sizeof(buffer));
    delete request;
    LogonMessage *result = (LogonMessage *)buffer;
    client_id = result->client_id;
    timesteps = result->timesteps;
    x1 = result->start_x;
    y1 = result->start_y;
    x2 = result->end_x;
    y2 = result->end_y;
    LOG(INFO) << "[CLIENT-" << client_id << "] "
              << "Login completed";
    return received_bytes > 0 ? 0 : -1;
};

void LifeClient::loop() {
    while (timestep < timesteps) {
        LOG(INFO) << "[CLIENT-" << client_id << "] "
                  << "Simulating cycle " << timestep;
        makeStep();
        LOG(INFO) << "[CLIENT-" << client_id << "] "
                  << "Signaling doneness to server";
        char buffer[100];
        BarrierMessage *request = BarrierMessage::createRequest(getNextSequenceNumber(), client_id, timestep);
        net->request(server, request, sizeof(BarrierMessage), buffer, sizeof(buffer));
        timestep++;
    }
};

void LifeClient::makeStep() {
    int width = (x2 - x1) + 2;
    int height = (y2 - y1) + 2;
    LocalBoard *board = new LocalBoard(width, height);
    board->clear();
    // read remote board
    for (int x = 0; x < board->getWidth(); x++) {
        for (int y = 0; y < board->getHeight(); y++) {
            board->setPos(x, y, getRemotePos(x + x1 - 1, y + y1 - 1));
        }
    }
    // do calculation
    board->step();
    // write remote board
    for (int x = 1; x < board->getWidth() - 1; x++) {
        for (int y = 1; y < board->getHeight() - 1; y++) {
            setRemotePos(x + x1 - 1, y + y1 - 1, board->getPos(x, y));
        }
    }
    delete board;
};

life_status_t LifeClient::getRemotePos(int x, int y) {
    char buffer[100];
    BoardGetMessage *request = BoardGetMessage::createRequest(getNextSequenceNumber(), x, y);
    net->request(server, request, sizeof(BoardGetMessage), buffer, sizeof(buffer));
    BoardGetMessage *result = (BoardGetMessage *)buffer;
    delete request;
    return result->state;
}

bool LifeClient::setRemotePos(int x, int y, life_status_t status) {
    char buffer[100];
    BoardSetMessage *request = BoardSetMessage::createRequest(getNextSequenceNumber(), x, y, status);
    net->request(server, request, sizeof(BoardSetMessage), buffer, sizeof(buffer));
    BoardSetMessage *result = (BoardSetMessage *)buffer;
    delete request;
    return result->confirmed;
}

unsigned int LifeClient::getNextSequenceNumber() { return sequence_number++; }