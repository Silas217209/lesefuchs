#include <ctime>
#include <iostream>
#include <unordered_map>
#include <vector>

class Medium {
public:
    explicit Medium(int id, std::string titel) : m_id(id), m_titel(titel) {}
    virtual ~Medium() = default;

    int id() const { return m_id; }

    virtual bool darfAusleihen(int /*alter*/) { return true; }

    virtual int maxLeihDauer() const { return 30; };

    bool ausgeliehen() const { return m_ausgeliehen; };

    void setAusgeliehen(bool ausgeliehen) { m_ausgeliehen = ausgeliehen; };

    enum class MediumTyp {
        eBuch,
        eDVD,
        eCD,
        eComputerspiel
    };

    virtual MediumTyp typ() const = 0;


private:
    int m_id;
    bool m_ausgeliehen = false;
    std::string m_titel;
};

class Buch : public Medium {
public:
    explicit Buch(int id, std::string titel) : Medium(id, titel) {}

    int maxLeihDauer() const override {
        return 28;
    }

    bool darfAusleihen(int alter) override {
        return alter >= 0;
    }

    MediumTyp typ() const override {
        return MediumTyp::eBuch;
    }
};

class DVD : public Medium {
public:
    explicit DVD(int id, std::string titel, int altersBeschraenkung) : Medium(id, titel), m_altersBeschraenkung(altersBeschraenkung) {}

    int maxLeihDauer() const override {
        return 7;
    }

    bool darfAusleihen(int alter) override {
        return alter >= m_altersBeschraenkung;
    }


    MediumTyp typ() const override {
        return MediumTyp::eDVD;
    }

private:
    int m_altersBeschraenkung;
};

class CD : public Medium {
public:
    explicit CD(int id, std::string titel) : Medium(id, titel) {}

    int maxLeihDauer() const override {
        return 21;
    }

    MediumTyp typ() const override {
        return MediumTyp::eCD;
    }
};

class Computerspiel : public Medium {
public:
    explicit Computerspiel(int id, std::string titel, int altersBeschraenkung) :
        Medium(id, titel), m_altersBeschraenkung(altersBeschraenkung) {}

    int maxLeihDauer() const override {
        return 21;
    }

    bool darfAusleihen(int alter) override {
        return alter >= m_altersBeschraenkung;
    }

    MediumTyp typ() const override {
        return MediumTyp::eComputerspiel;
    }

private:
    int m_altersBeschraenkung;
};

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

    const std::vector<Leihvorgang> &leihvorgaenge() const { return m_leihvorgaenge; }

    const std::unordered_map<int, Medium *> &medien() const {
        return m_medien;
    }

    const std::unordered_map<int, Kunde *> &kunden() const {
        return m_kunden;
    }

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

        for (auto it = m_leihvorgaenge.begin(); it != m_leihvorgaenge.end(); ++it) {
            if (it->pKunde()->id() != kunde->id()) {
                continue;
            }

            if (it->pMedium()->id() != medium->id()) {
                continue;
            }

            if (it->istUeberfaellig()) {
                std::cout << "Erfolg, aber übrezogen\n";
                medium->setAusgeliehen(false);
                m_leihvorgaenge.erase(it);
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

    // --- KUNDEN ANLEGEN ---
    auto *k0 = new Kunde(0, 21, "Mueller", "Max", "Hauptstraße 12", "max.m@example.com", "015112345678");
    auto *k1 = new Kunde(1, 17, "Schneider", "Lena", "Bahnhofstraße 5", "lena.s@example.com", "017312345678");
    auto *k2 = new Kunde(2, 14, "Weber", "Felix", "Am Park 8", "felix.w@example.com", "016012345678");
    auto *k3 = new Kunde(3, 35, "Schmidt", "Anna", "Gartenweg 3", "anna.s@example.com", "017612345678");

    // --- MEDIEN ANLEGEN ---
    auto *buch1 = new Buch(0, "Book 1");                      // Book with generic title
    auto *dvd1 = new DVD(1, "Film 1", 16);                      // DVD (Film) with generic title and age restriction
    auto *cd1 = new CD(2, "CD 1");                              // CD with generic title
    auto *spiel1 = new Computerspiel(3, "Game 1", 12);          // Computerspiel with generic title and age restriction

    // --- KUNDEN UND MEDIEN ZUR BIBLIOTHEK HINZUFÜGEN ---
    std::cout << "--- Bibliothek wird mit Kunden und Medien gefüllt ---\n";
    bib.kundeHinzufuegen(k0);
    bib.kundeHinzufuegen(k1);
    bib.kundeHinzufuegen(k2);
    bib.kundeHinzufuegen(k3);

    bib.mediumHinzufuegen(buch1);
    bib.mediumHinzufuegen(dvd1);
    bib.mediumHinzufuegen(cd1);
    bib.mediumHinzufuegen(spiel1);

    // --- AUSLEIHEN VON MEDIEN ---
    std::cout << "\n--- Ausleihen von Medien ---\n";

    std::cout << "Max (21) leiht 'Book 1' aus:\n";
    auto result1 = bib.ausleihen(k0, buch1);
    std::cout << "Ergebnis: " << (result1 == Bibliothek::AusleihFehler::eErfolg ? "Erfolg" : "Fehler") << "\n";

    std::cout << "Felix (14) versucht 'Film 1' auszuleihen:\n";
    auto result2 = bib.ausleihen(k2, dvd1);
    std::cout << "Ergebnis: " << (result2 == Bibliothek::AusleihFehler::eZuJung ? "Zu jung" : "Anderer Fehler") << "\n";

    std::cout << "Lena (17) leiht 'CD 1' aus:\n";
    auto result3 = bib.ausleihen(k1, cd1);
    std::cout << "Ergebnis: " << (result3 == Bibliothek::AusleihFehler::eErfolg ? "Erfolg" : "Fehler") << "\n";

    std::cout << "Anna versucht bereits ausgeliehene 'CD 1' auszuleihen:\n";
    auto result4 = bib.ausleihen(k3, cd1);
    std::cout << "Ergebnis: " << (result4 == Bibliothek::AusleihFehler::eBereitsAusgeliehen ? "Bereits ausgeliehen" : "Anderer Fehler") << "\n";

    std::cout << "Felix (14) leiht 'Game 1' aus:\n";
    auto result5 = bib.ausleihen(k2, spiel1);
    std::cout << "Ergebnis: " << (result5 == Bibliothek::AusleihFehler::eErfolg ? "Erfolg" : "Fehler") << "\n";

    // --- AKTUELLEN STATUS ANZEIGEN ---
    std::cout << "\n--- Aktueller Status der Bibliothek ---\n";
    std::cout << "Registrierte Kunden:\n";
    bib.zeigeKunden();

    std::cout << "\nVerfügbare Medien:\n";
    bib.zeigeMedien();

    std::cout << "\nAktuelle Leihvorgänge:\n";
    bib.zeigeVerleihe();

    // --- RÜCKGABE VON MEDIEN ---
    std::cout << "\n--- Rückgabe von Medien ---\n";
    std::cout << "Max gibt 'Book 1' zurück:\n";
    auto rueckgabe1 = bib.zurueckgeben(k0, buch1);
    std::cout << "Ergebnis: " << (rueckgabe1 == Bibliothek::RueckgabeFehler::eErfolg ? "Erfolg" : "Fehler") << "\n";

    std::cout << "Max versucht 'Film 1' zurückzugeben (nicht ausgeliehen):\n";
    auto rueckgabe2 = bib.zurueckgeben(k0, dvd1);
    std::cout << "Ergebnis: " << (rueckgabe2 == Bibliothek::RueckgabeFehler::eNichtAusgeliehen ? "Nicht ausgeliehen" : "Anderer Fehler") << "\n";

    std::cout << "\n--- Säumige Kunden überprüfen ---\n";
    auto saeumige = bib.sauemigeKunden();
    std::cout << "Anzahl säumiger Kunden: " << saeumige.size() << "\n";

    std::cout << "\n--- Aktualisierter Status nach Rückgaben ---\n";
    bib.zeigeVerleihe();

    // --- AUFRÄUMEN ---
    delete k0;
    delete k1;
    delete k2;
    delete k3;
    delete buch1;
    delete dvd1;
    delete cd1;
    delete spiel1;

    return 0;
}