#include <ctime>
#include <iostream>
#include <unordered_map>
#include <vector>

class Medium {
public:
    explicit Medium(int id) : m_id(id) {}
    virtual ~Medium() = default;

    int id() const { return m_id; }

    virtual bool darfAusleihen(int /*alter*/) { return true; }

    virtual int maxLeihDauer() const { return 30; };

    bool ausgeliehen() const { return m_ausgeliehen; };

    void setAusgeliehen(bool ausgeliehen) { m_ausgeliehen = ausgeliehen; };

private:
    int m_id;
    bool m_ausgeliehen = false;
};

class Kunde {
public:
    Kunde(int id, int alter) : m_id(id), m_alter(alter) {}

    int id() const { return m_id; }

    int alter() const { return m_alter; }

private:
    int m_id;
    int m_alter;
};


class Leihvorgang {
public:
    Leihvorgang(const Kunde *kundenId, const Medium *mediumId) :
        m_kunden(kundenId), m_medium(mediumId), m_ausleihDatum(time(nullptr)) {};

    const Kunde *pKunde() const { return m_kunden; }

    const Medium *pMedium() const { return m_medium; }

    bool istUeberfaellig() const {
        const time_t jetzt = time(nullptr);
        const double diffsec = difftime(jetzt, m_ausleihDatum);
        return diffsec > m_medium->maxLeihDauer() * 60 * 60 * 24;
    }

    int ueberfaelligTage() const {
        if (!istUeberfaellig()) {
            return 0;
        }

        const time_t jetzt = time(nullptr);
        const double diffsec = difftime(jetzt, m_ausleihDatum);
        const int diffDays = static_cast<int>(diffsec / (60 * 60 * 24));
        return diffDays - m_medium->maxLeihDauer();
    }

private:
    const Kunde *m_kunden;
    const Medium *m_medium;
    time_t m_ausleihDatum;
};

class Bibliothek {
public:
    void mediumHinzufuegen(Medium *medium) { m_medien.emplace(medium->id(), medium); }

    void kundeHinzufuegen(Kunde *kunde) { m_kunden.emplace(kunde->id(), kunde); }

    enum class AusleihFehler {
        eErfolg,
        eKundeExistiertNicht,
        eMediumExistiertNicht,
        eZuJung,
        eBereitsAusgeliehen,
    };

    AusleihFehler ausleihen(Kunde *kunde, Medium *medium) {
        if (m_kunden.find(kunde->id()) == m_kunden.end()) {
            return AusleihFehler::eKundeExistiertNicht;
        }

        if (m_medien.find(medium->id()) == m_medien.end()) {
            return AusleihFehler::eMediumExistiertNicht;
        }

        if (!medium->darfAusleihen(kunde->alter())) {
            return AusleihFehler::eZuJung;
        }

        if (medium->ausgeliehen()) {
            return AusleihFehler::eBereitsAusgeliehen;
        }

        medium->setAusgeliehen(true);
        m_leihvorgaenge.emplace_back(kunde, medium);

        return AusleihFehler::eErfolg;
    }

    enum class RueckgabeFehler {
        eErfolg,
        eKundeExistiertNicht,
        eMediumExistiertNicht,
        eNichtAusgeliehen,
    };

    RueckgabeFehler zurueckgeben(Kunde *kunde, Medium *medium) {
        if (m_kunden.find(kunde->id()) == m_kunden.end()) {
            return RueckgabeFehler::eKundeExistiertNicht;
        }

        if (m_medien.find(medium->id()) == m_medien.end()) {
            return RueckgabeFehler::eMediumExistiertNicht;
        }

        for (auto it = m_leihvorgaenge.begin(); it < m_leihvorgaenge.end(); ++it) {
            if (it.base()->pKunde()->id() != kunde->id()) {
                continue;
            }

            if (it.base()->pMedium()->id() != medium->id()) {
                continue;
            }

            medium->setAusgeliehen(false);
            m_leihvorgaenge.erase(it);
            return RueckgabeFehler::eErfolg;
        }

        return RueckgabeFehler::eNichtAusgeliehen;
    }

    std::vector<const Kunde *> sauemigeKunden() {
        std::vector<const Kunde *> sauemigeKunden;
        for (auto l: m_leihvorgaenge) {
            if (l.istUeberfaellig()) {
                std::cout << "Kunde " << l.pKunde()->id() << " hat Medium " << l.pMedium()->id() << " überfällig!\n";
                sauemigeKunden.emplace_back(l.pKunde());
            }
        }

        return sauemigeKunden;
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
            std::cout << l.pKunde()->id() << " <-> " << l.pMedium()->id() << "\n";
        }
    }

private:
    std::unordered_map<int, Medium *> m_medien;
    std::unordered_map<int, Kunde *> m_kunden;
    std::vector<Leihvorgang> m_leihvorgaenge;
};


int main() {
    Bibliothek bib;

    auto *k0 = new Kunde(0, 21);
    auto *k1 = new Kunde(1, 22);
    auto *k2 = new Kunde(2, 17);

    auto *m0 = new Medium(0);
    auto *m1 = new Medium(1);
    auto *m2 = new Medium(2);

    bib.kundeHinzufuegen(k0);
    bib.kundeHinzufuegen(k1);
    bib.kundeHinzufuegen(k2);

    bib.mediumHinzufuegen(m0);
    bib.mediumHinzufuegen(m1);
    bib.mediumHinzufuegen(m2);

    bib.ausleihen(k0, m0);
    bib.ausleihen(k0, m1);
    bib.ausleihen(k0, m2);

    bib.zeigeKunden();
    std::cout << "\n";
    bib.zeigeMedien();
    std::cout << "\n";
    bib.zeigeVerleihe();

    bib.zurueckgeben(k0, m1);
    std::cout << "\n";
    bib.zeigeVerleihe();
    bib.sauemigeKunden();

    delete k0;
    delete k1;
    delete k2;

    delete m0;
    delete m1;
    delete m2;

    return 0;
}
