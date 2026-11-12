#pragma once
#include <QString>

struct Card {
    QString front;
    QString back;
    int correct = 0;   // times answered right
    int missed  = 0;   // times answered wrong
};
