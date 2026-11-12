#pragma once
#include <QString>
#include <QStringList>

struct Card {
    QString front;
    QString back;
    QStringList alt;   // other accepted answers (synonyms), not shown on the card
    int correct = 0;   // times answered right
    int missed  = 0;   // times answered wrong
};
