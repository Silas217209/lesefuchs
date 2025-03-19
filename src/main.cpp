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

// TODO(Max): abgeleitete Klassen: Bücher, DVD (mit Altersbeschränkung), CD, Computerspiele (mit Altersbeschränkung)

class Kunde {
public:
    Kunde(int id, int alter, std::string name, std::string vorname, std::string adresse, std::string email,
          std::string telefon) :
        m_id(id), m_alter(alter), m_name(name), m_vorname(vorname), m_adresse(adresse), m_email(email),
        m_telefon(telefon) {}

    int id() const { return m_id; }
    int alter() const { return m_alter; }
    std::string name() const { return m_name; }
    std::string vorname() const { return m_vorname; }
    std::string adresse() const { return m_adresse; }
    std::string email() const { return m_email; }
    std::string telefon() const { return m_telefon; }

private:
    int m_id;
    int m_alter;
    std::string m_name;
    std::string m_vorname;
    std::string m_adresse;
    std::string m_email;
    std::string m_telefon;
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
        eErfolgUeberzogen, /// Rückgabe erfolgreich, aber Überzogen
        eKundeExistiertNicht,
        eMediumExistiertNicht,
        eNichtAusgeliehen,
    };

    RueckgabeFehler zurueckgeben(Kunde *kunde, Medium *medium) {
        if (m_kunden.find(kunde->id()) == m_kunden.end()) {
            std::cout << "Kunde existiert nicht\n";
            return RueckgabeFehler::eKundeExistiertNicht;
        }

        if (m_medien.find(medium->id()) == m_medien.end()) {
            std::cout << "Medium existiert nicht\n";
            return RueckgabeFehler::eMediumExistiertNicht;
        }

        for (auto it = m_leihvorgaenge.begin(); it < m_leihvorgaenge.end(); ++it) {
            if (it.base()->pKunde()->id() != kunde->id()) {
                continue;
            }

            if (it.base()->pMedium()->id() != medium->id()) {
                continue;
            }

            if (it.base()->istUeberfaellig()) {
                std::cout << "Erfolg, aber übrezogen\n";
                return RueckgabeFehler::eErfolgUeberzogen;
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
                sauemigeKunden.emplace_back(l.pKunde());
            }
        }

        return sauemigeKunden;
    }

    // TODO: Löschen
    void zeigeKunden() {
        for (const auto &kunde: m_kunden) {
            std::cout << "Kunde ID: " << kunde.first << "\n";
        }
    }

    // TODO: Löschen
    void zeigeMedien() {
        for (const auto &medium: m_medien) {
            std::cout << "Medium ID: " << medium.first << "\n";
        }
    }

    // TODO: Löschen
    void zeigeVerleihe() {
        std::cout << "Kunde <-> Medium\n";
        for (const auto &l: m_leihvorgaenge) {
            std::cout << l.pKunde()->vorname() << " " << l.pKunde()->name() << " <-> " << l.pMedium()->id() << "\n";
        }
    }

private:
    std::unordered_map<int, Medium *> m_medien;
    std::unordered_map<int, Kunde *> m_kunden;
    std::vector<Leihvorgang> m_leihvorgaenge;
};


int main() {
    Bibliothek bib;

    auto *k0 = new Kunde(0, 21, "Mueller", "Max", "Hauptstraße 12", "max.m@example.com", "015112345678");
    auto *k1 = new Kunde(1, 17, "Schneider", "Lena", "Bahnhofstraße 5", "lena.s@example.com", "017312345678");
    auto *k2 = new Kunde(2, 14, "Weber", "Felix", "Am Park 8", "felix.w@example.com", "016012345678");

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
