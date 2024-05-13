#include <array>
#include <vector>
#include <algorithm>

const int W_WIDTH = 400;
const int W_HEIGHT = 400;
const int S_WIDTH = 10;
const int S_HEIGHT = 10;

const int NUM_ROWS = W_HEIGHT / S_HEIGHT;
const int NUM_COLS = W_WIDTH / S_WIDTH;

std::array<std::vector<int>, NUM_ROWS* NUM_COLS> g_ObjectListSector;

void InitObjectListSector() {
    for (auto& sector : g_ObjectListSector) {
        sector.reserve(10); // �� ������ ��ü ���� �°� �ʱ� �뷮�� ������ �� �ֽ��ϴ�.
    }
}

int GetSectorIndex(int row, int col) {
    return row * NUM_COLS + col;
}

void AddObjectToSector(int objectId, int x, int y) {
    int row = y / S_HEIGHT;
    int col = x / S_WIDTH;
    int sectorIndex = GetSectorIndex(row, col);
    g_ObjectListSector[sectorIndex].emplace_back(objectId);
}

void RemoveObjectFromSector(int objectId, int x, int y) {
    int row = y / S_HEIGHT;
    int col = x / S_WIDTH;
    int sectorIndex = GetSectorIndex(row, col);
    auto& sector = g_ObjectListSector[sectorIndex];
    sector.erase(std::remove(sector.begin(), sector.end(), objectId), sector.end());
}

void UpdateObjectSector(int objectId, int newX, int newY, int oldX = -1, int oldY = -1) {
    int newRow = newY / S_HEIGHT;
    int newCol = newX / S_WIDTH;

    if (oldX == -1 || oldY == -1) {
        // �ʱ� ���� �� ��ġ Ȯ�� �� SECTOR �߰�
        AddObjectToSector(objectId, newX, newY);
    }
    else {
        int oldRow = oldY / S_HEIGHT;
        int oldCol = oldX / S_WIDTH;

        if (oldRow != newRow || oldCol != newCol) {
            // ���� SECTOR���� ����
            RemoveObjectFromSector(objectId, oldX, oldY);
            // ���ο� SECTOR�� �߰�
            AddObjectToSector(objectId, newX, newY);
        }
    }
}

void SendSectorObjectInfo(int clientId, int x, int y) {
    int row = y / S_HEIGHT;
    int col = x / S_WIDTH;
    int sectorIndex = GetSectorIndex(row, col);
    for (int objectId : g_ObjectListSector[sectorIndex]) {
        // �ش� SECTOR�� �ִ� Ŭ���̾�Ʈ���� objectId�� �ش��ϴ� ���� ����
        SendObjectInfoToClient(clientId, objectId);
    }
}

void HandleClientConnect(int clientId, int x, int y) {
    UpdateObjectSector(clientId, x, y);
    SendSectorObjectInfo(clientId, x, y);
}

void HandleClientMovement(int clientId, int oldX, int oldY, int newX, int newY) {
    UpdateObjectSector(clientId, newX, newY, oldX, oldY);
    SendSectorObjectInfo(clientId, newX, newY);
}

void HandleClientDisconnect(int clientId, int x, int y) {
    RemoveObjectFromSector(clientId, x, y);
}