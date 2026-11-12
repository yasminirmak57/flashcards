#include "deck.h"
#include <QFile>
#include <QHash>
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
        Card c{o["front"].toString(), o["back"].toString()};
        for (const auto &a : o["alt"].toArray())
            c.alt << a.toString();
        c.correct = o["correct"].toInt();
        c.missed = o["missed"].toInt();
        m_cards.push_back(c);
    }
    return true;
}

bool Deck::save(const QString &path) const {
    QJsonArray arr;
    for (const Card &c : m_cards) {
        QJsonObject o{{"front", c.front}, {"back", c.back},
                      {"correct", c.correct}, {"missed", c.missed}};
        if (!c.alt.isEmpty())
            o["alt"] = QJsonArray::fromStringList(c.alt);
        arr.append(o);
    }
    QFile f(path);
    if (!f.open(QIODevice::WriteOnly))
        return false;
    f.write(QJsonDocument(arr).toJson());
    return true;
}

void Deck::addCard(const QString &front, const QString &back) {
    m_cards.push_back({front, back});   // studied when you reach its group
}

void Deck::startSession(int from, int count) {
    m_queue.clear();
    int end = (count < 0) ? m_cards.size() : std::min<int>(m_cards.size(), from + count);
    for (int i = std::max(0, from); i < end; ++i)
        m_queue.push_back(i);
    std::shuffle(m_queue.begin(), m_queue.end(), *QRandomGenerator::global());
}

void Deck::mergeSynonyms(const Deck &from) {
    QHash<QString, QStringList> alts;
    for (const Card &c : from.m_cards)
        alts.insert(c.front, c.alt);
    for (Card &c : m_cards)
        if (c.alt.isEmpty() && alts.contains(c.front))
            c.alt = alts.value(c.front);
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
