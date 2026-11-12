#include "deck.h"
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRandomGenerator>
#include <algorithm>

bool Deck::load(const QString &path) {
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly))
        return false;
    const QJsonArray arr = QJsonDocument::fromJson(f.readAll()).array();
    m_cards.clear();
    for (const auto &v : arr) {
        const QJsonObject o = v.toObject();
        m_cards.push_back({o["front"].toString(), o["back"].toString(),
                           o["correct"].toInt(), o["missed"].toInt()});
    }
    return true;
}

bool Deck::save(const QString &path) const {
    QJsonArray arr;
    for (const Card &c : m_cards) {
        arr.append(QJsonObject{{"front", c.front}, {"back", c.back},
                               {"correct", c.correct}, {"missed", c.missed}});
    }
    QFile f(path);
    if (!f.open(QIODevice::WriteOnly))
        return false;
    f.write(QJsonDocument(arr).toJson());
    return true;
}

void Deck::addCard(const QString &front, const QString &back) {
    m_cards.push_back({front, back});
    m_queue.push_back(m_cards.size() - 1);
}

void Deck::startSession() {
    m_queue.clear();
    for (int i = 0; i < m_cards.size(); ++i)
        m_queue.push_back(i);
    std::shuffle(m_queue.begin(), m_queue.end(), *QRandomGenerator::global());
}

bool Deck::hasCurrent() const { return !m_queue.isEmpty(); }

Card &Deck::current() { return m_cards[m_queue.front()]; }

void Deck::answer(bool gotIt) {
    int idx = m_queue.takeFirst();
    if (gotIt) {
        m_cards[idx].correct++;
    } else {
        m_cards[idx].missed++;
        m_queue.push_back(idx);       // see it again later
    }
}
