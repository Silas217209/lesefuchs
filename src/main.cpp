/*
 * Lesefuchs
 * Ein Bibliothek in Ihrer Region möchte ein neues Buchungssystem installieren. So sollen sowohl
 * alle Medien als auch alle Kunden und Leihvorgänge digital digital verarbeitet werden. Sie sollen
 * helfen, dieses System objektorientiert umzusetzen.
 * Grundlage für die Datenbank sind die vorhandenen Medien, diese gibt es als Bücher, Spiele und
 * DVDs, CDs und Computerspiele (mit Altersbeschränkung) und Anzahl vorhandener Medien eines
 * Typs. Jedes Medium hat eine eigene ID und einen Zustand ausgeliehen oder nicht.
 * Es gibt eine Liste, die alle Kunden enthält, dabei ist das Alter wichtig, damit Kinder keine Medien
 * ausleihen können, die nicht für sie zu gelassen sind.
 * Es gibt eine Liste, die die Medien enthält.
 * Die Leihvorgänge müssen gespeichert werden und beim Ausleihen beachtet werden, dass noch
 * eine Exemplar des Mediums vorhanden ist. Bei der Rückgabe muss die Anzahl ebenfalls beachtet
 * werden und eine Hinweis zu überzogenen Leihzeit .
 * Die Höchstdauer der Ausleihe ist bei verschiedenen Medien unterschiedlich, Filme kürzer als
 * Spiele. Bücher dürfen länger behalten werden.
 * Die Datenbank enthält eine Anzahl von mindestens 10 Kunden (als dynamische Liste, gern auch
 * objektorientiert implementiert, aber kein Muss), die spezielle Personen (Name, Vorname, Adresse)
 * mit Email und Telefonnummern sind. Es muss möglich sein säumige Kunden zu informieren, dass
 * sie Strafe zahlen müssen, wenn sie überzogen haben.
 * Täglich wird die Kundendatenbank einmal auf säumige Kunden überprüft.
 * Die Bibliothek möchte eine Übersicht über alle aktuell ausgeliehenen Medien haben. Es sollte
 * möglich sein, Medien, Kunden und Leihvorgänge über eine Benutzerober äche (z. B.
 * Konsolenanwendung oder gra sche Ober äche) zu verwalten.
 */

#include <unordered_map>
#include <iostream>
#include <vector>

class Medium {
public:
    Medium(int id) : m_id(id) {
    }

    int id() const {
        return m_id;
    }

    virtual bool darfAusleihen(int alter) { return true; }

private:
    int m_id;
};

class Kunde {
public:
    Kunde(int id): m_id(id) {
    };

    int id() const {
        return m_id;
    }

    int alter() const {
        return m_alter;
    }

private:
    int m_id;
    int m_alter;
};


class Leihvorgang {
public:
    Leihvorgang(int kundenId, int mediumId): m_kundenId(kundenId), m_mediumId(mediumId), m_ausleihDatum(time(nullptr)) {
    };

    int kundenId() const {
        return m_kundenId;
    }

    int mediumId() const {
        return m_mediumId;
    }

private:
    int m_kundenId;
    int m_mediumId;
    time_t m_ausleihDatum;
};

class Bibliothek {
public:
    void mediumHinzufuegen(Medium *medium) {
        m_medien.emplace(medium->id(), medium);
    }

    void kundeHinzufuegen(Kunde *kunde) {
        m_kunden.emplace(kunde->id(), kunde);
    }

    bool ausleihen(Kunde *kunde, Medium *medium) {
        if (m_kunden.find(kunde->id()) == m_kunden.end()) {
            return false;
        }

        if (m_medien.find(medium->id()) == m_medien.end()) {
            return false;
        }

        if (!medium->darfAusleihen(kunde->alter())) {
            return false;
        }

        m_leihvorgaenge.emplace_back(kunde->id(), medium->id());

        return true;
    }

    bool zurueckgeben(Kunde *kunde, Medium *medium) {
        if (m_kunden.find(kunde->id()) == m_kunden.end()) {
            return false;
        }

        if (m_medien.find(medium->id()) == m_medien.end()) {
            return false;
        }

        for (auto it = m_leihvorgaenge.begin(); it < m_leihvorgaenge.end(); ++it) {
            if (it.base()->kundenId() != kunde->id()) {
                continue;
            }

            if (it.base()->mediumId() != medium->id()) {
                continue;
            }

            m_leihvorgaenge.erase(it);
            return true;
        }
    }

    void zeigeKunden() {
        for (const auto &kunde: m_kunden) {
            std::cout << "Kunde ID: " << kunde.first << "\n";
        }
    }

    void zeigeMedien() {
        for (const auto &medium: m_medien) {
            std::cout << "Medium ID: " << medium.first << "\n";
        }
    }

    void zeigeVerleihe() {
        std::cout << "Kunde <-> Medium\n";
        for (const auto &l: m_leihvorgaenge) {
            std::cout << l.kundenId() << " <-> " << l.mediumId() << "\n";
        }
    }

private:
    std::unordered_map<int, Medium *> m_medien;
    std::unordered_map<int, Kunde *> m_kunden;
    std::vector<Leihvorgang> m_leihvorgaenge;
};


int main() {
    Bibliothek bib;

    auto k0 = new Kunde(0);
    auto k1 = new Kunde(1);
    auto k2 = new Kunde(2);

    auto m0 = new Medium(0);
    auto m1 = new Medium(1);
    auto m2 = new Medium(2);

    bib.kundeHinzufuegen(k0);
    bib.kundeHinzufuegen(k1);
    bib.kundeHinzufuegen(k2);

    bib.mediumHinzufuegen(m0);
    bib.mediumHinzufuegen(m1);
    bib.mediumHinzufuegen(m2);

    bib.ausleihen(k0, m1);

    bib.zeigeKunden();
    std::cout << "\n";
    bib.zeigeMedien();
    std::cout << "\n";
    bib.zeigeVerleihe();

    bib.zurueckgeben(k0, m1);
    std::cout << "\n";
    bib.zeigeVerleihe();

    delete k0;
    delete k1;
    delete k2;

    delete m0;
    delete m1;
    delete m2;

    return 0;
}
