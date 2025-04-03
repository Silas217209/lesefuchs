#include <ctime>
#include <fstream>
#include <iomanip>
#include <ios>
#include <iostream>
#include <limits>
#include <map>
#include <vector>
#include <string>
#include <unordered_map>

//---------------------------Klassen--------------------------------

class Medium {
public:
    explicit Medium(int id, std::string titel) : m_id(id), m_titel(titel) {
    }

    explicit Medium(std::ifstream *db, int id): m_id(id) {
        db->read(reinterpret_cast<char *>(&m_ausgeliehen), sizeof(m_ausgeliehen));

        unsigned int titellaenge;
        db->read(reinterpret_cast<char *>(&titellaenge), sizeof(titellaenge));

        std::string titel;
        titel.resize(titellaenge);
        db->read(&titel[0], titellaenge);

        m_titel = titel;
    }

    virtual ~Medium() = default;

    int id() const {
        return m_id;
    }

    std::string titel() const {
        return m_titel;
    }

    virtual bool darfAusleihen(int /*alter*/) {
        return true;
    }

    virtual int maxLeihDauer() const {
        return 30;
    };

    bool ausgeliehen() const {
        return m_ausgeliehen;
    };

    void setAusgeliehen(bool ausgeliehen) {
        m_ausgeliehen = ausgeliehen;
    };

    enum class MediumTyp: char { eBuch, eDVD, eCD, eComputerspiel };

    virtual MediumTyp typ() const = 0;

    virtual std::string typName() const = 0;

    virtual void serialisieren(std::ofstream *db) const = 0;

    static Medium *vonDateiLesen(std::ifstream *db);

protected:
    void commonSerialisierung(std::ofstream *db, MediumTyp typ) const {
        // 1 Byte: Id schreiben
        db->write(reinterpret_cast<const char *>(&m_id), sizeof(m_id));

        // 1 Byte: Typ schreiben
        db->write(reinterpret_cast<char *>(&typ), sizeof(char));

        // 1 Byte: ausgeliehen
        db->write(reinterpret_cast<const char *>(&m_ausgeliehen), sizeof(m_ausgeliehen));

        // 4 Byte: titellänge (bytezahl)
        unsigned int titellaenge = static_cast<unsigned int>(m_titel.size());
        db->write(reinterpret_cast<char *>(&titellaenge), sizeof(titellaenge));

        // X Bytes: titel
        db->write(m_titel.data(), m_titel.size());
    }

private:
    int m_id;
    bool m_ausgeliehen = false;
    std::string m_titel;
};

class Buch : public Medium {
public:
    Buch(int id, std::string titel, std::string author, std::string verlag, std::string genre,
         int erscheinungsjahr,
         int seitenzahl) : Medium(id, titel), m_author(author), m_verlag(verlag), m_genre(genre),
                           m_erscheinungsJahr(erscheinungsjahr),
                           m_seitenzahl(seitenzahl) {
    }

    Buch(std::ifstream *db, int id): Medium(db, id) {
        unsigned int authorlaenge;
        db->read(reinterpret_cast<char *>(&authorlaenge), sizeof(authorlaenge));

        std::string author;
        author.resize(authorlaenge);
        db->read(&author[0], authorlaenge);

        m_author = author;

        unsigned int verlaglaenge;
        db->read(reinterpret_cast<char *>(&verlaglaenge), sizeof(verlaglaenge));

        std::string verlag;
        verlag.resize(verlaglaenge);
        db->read(&verlag[0], verlaglaenge);

        m_verlag = verlag;

        unsigned int genrelaenge;
        db->read(reinterpret_cast<char *>(&genrelaenge), sizeof(genrelaenge));

        std::string genre;
        genre.resize(genrelaenge);
        db->read(&genre[0], genrelaenge);

        m_genre = genre;

        db->read(reinterpret_cast<char *>(&m_erscheinungsJahr), sizeof(m_erscheinungsJahr));

        db->read(reinterpret_cast<char *>(&m_seitenzahl), sizeof(m_seitenzahl));
    }

    int maxLeihDauer() const override {
        return 28;
    }

    bool darfAusleihen(int alter) override {
        return alter >= 0;
    }

    MediumTyp typ() const override {
        return MediumTyp::eBuch;
    }

    std::string typName() const override {
        return "Buch";
    }

    void serialisieren(std::ofstream *db) const override {
        commonSerialisierung(db, typ());
        // 4 Byte: authorlänge (bytezahl)
        unsigned int authorlaenge = static_cast<unsigned int>(m_author.size());
        db->write(reinterpret_cast<char *>(&authorlaenge), sizeof(authorlaenge));

        // X Bytes: titel
        db->write(m_author.data(), m_author.size());

        // 4 Byte: verlaglänge (bytezahl)
        unsigned int verlaglaenge = static_cast<unsigned int>(m_verlag.size());
        db->write(reinterpret_cast<char *>(&verlaglaenge), sizeof(verlaglaenge));

        // X Bytes: titel
        db->write(m_verlag.data(), m_verlag.size());

        // 4 Byte: verlaglänge (bytezahl)
        unsigned int genrelaenge = static_cast<unsigned int>(m_genre.size());
        db->write(reinterpret_cast<char *>(&genrelaenge), sizeof(genrelaenge));

        // X Bytes: titel
        db->write(m_genre.data(), m_genre.size());

        // 4 Bytes: erscheinungsjahr
        db->write(reinterpret_cast<const char *>(&m_erscheinungsJahr), sizeof(m_erscheinungsJahr));

        // 4 Bytes: seitenzahl
        db->write(reinterpret_cast<const char *>(&m_seitenzahl), sizeof(m_seitenzahl));
    }

    std::string getAuthor() {
        return m_author;
    }

    std::string getGenre() {
        return m_genre;
    }

private:
    std::string m_author;
    std::string m_verlag;
    std::string m_genre;
    int m_erscheinungsJahr;
    int m_seitenzahl;
};

class DVD : public Medium {
public:
    explicit DVD(int id, std::string titel, int altersBeschraenkung, int spielDauerMinuten, int erscheinungsjahr,
                 std::string regisseur, std::string hauptdarsteller, std::string genre) : Medium(id, titel),
        m_altersBeschraenkung(altersBeschraenkung), m_spielDauerMinuten(spielDauerMinuten),
        m_erscheinungsjahr(erscheinungsjahr), m_regisseur(regisseur), m_hauptdarsteller(hauptdarsteller),
        m_genre(genre) {
    }

    explicit DVD(std::ifstream *db, int id): Medium(db, id) {
        db->read(reinterpret_cast<char *>(&m_altersBeschraenkung), sizeof(m_altersBeschraenkung));
        db->read(reinterpret_cast<char *>(&m_spielDauerMinuten), sizeof(m_spielDauerMinuten));
        db->read(reinterpret_cast<char *>(&m_erscheinungsjahr), sizeof(m_erscheinungsjahr));

        unsigned int regisseurlaenge;
        db->read(reinterpret_cast<char *>(&regisseurlaenge), sizeof(regisseurlaenge));

        std::string regisseur;
        regisseur.resize(regisseurlaenge);
        db->read(&regisseur[0], regisseurlaenge);

        m_regisseur = regisseur;

        unsigned int hauptdarstellerlaenge;
        db->read(reinterpret_cast<char *>(&hauptdarstellerlaenge), sizeof(hauptdarstellerlaenge));

        std::string hauptdarsteller;
        hauptdarsteller.resize(hauptdarstellerlaenge);
        db->read(&hauptdarsteller[0], hauptdarstellerlaenge);

        m_hauptdarsteller = hauptdarsteller;

        unsigned int genrelaenge;
        db->read(reinterpret_cast<char *>(&genrelaenge), sizeof(genrelaenge));

        std::string genre;
        genre.resize(genrelaenge);
        db->read(&genre[0], genrelaenge);

        m_genre = genre;
    }

    int maxLeihDauer() const override {
        return 7;
    }

    bool darfAusleihen(int alter) override {
        return alter >= m_altersBeschraenkung;
    }


    MediumTyp typ() const override {
        return MediumTyp::eDVD;
    }

    std::string typName() const override {
        return "DVD";
    }

    void serialisieren(std::ofstream *db) const override {
        commonSerialisierung(db, typ());

        // 4 Bytes: m_altersBeschraenkung
        db->write(reinterpret_cast<const char *>(&m_altersBeschraenkung), sizeof(m_altersBeschraenkung));

        // 4 Bytes: m_spielDauerMinuten
        db->write(reinterpret_cast<const char *>(&m_spielDauerMinuten), sizeof(m_spielDauerMinuten));

        // 4 Bytes: m_erscheinungsjahr
        db->write(reinterpret_cast<const char *>(&m_erscheinungsjahr), sizeof(m_erscheinungsjahr));

        // 4 Bytes: regisseurlaenge (bytezahl)
        unsigned int regisseurlaenge = static_cast<unsigned int>(m_regisseur.size());
        db->write(reinterpret_cast<const char *>(&regisseurlaenge), sizeof(regisseurlaenge));

        // X Bytes: m_regisseur
        db->write(m_regisseur.data(), m_regisseur.size());

        // 4 Bytes: hauptdarstellerlaenge (bytezahl)
        unsigned int hauptdarstellerlaenge = static_cast<unsigned int>(m_hauptdarsteller.size());
        db->write(reinterpret_cast<const char *>(&hauptdarstellerlaenge), sizeof(hauptdarstellerlaenge));

        // X Bytes: m_hauptdarsteller
        db->write(m_hauptdarsteller.data(), m_hauptdarsteller.size());

        // 4 Bytes: genrelaenge (bytezahl)
        unsigned int genrelaenge = static_cast<unsigned int>(m_genre.size());
        db->write(reinterpret_cast<const char *>(&genrelaenge), sizeof(genrelaenge));

        // X Bytes: m_genre
        db->write(m_genre.data(), m_genre.size());
    }

    std::string getRegisseur() {
        return m_regisseur;
    }

    std::string getGenre() {
        return m_genre;
    }

private:
    int m_altersBeschraenkung;
    int m_spielDauerMinuten;
    int m_erscheinungsjahr;
    std::string m_regisseur;
    std::string m_hauptdarsteller;
    std::string m_genre;
};

class CD : public Medium {
public:
    explicit CD(int id, std::string titel, std::string genre, std::string kuenstler, int spielDauerMinuten,
                int erscheinungsjahr) : Medium(id, titel),
                                        m_kuenstler(kuenstler), m_genre(genre), m_spielDauerMinuten(spielDauerMinuten),
                                        m_erscheinungsjahr(erscheinungsjahr) {
    }

    explicit CD(std::ifstream *db, int id): Medium(db, id) {
        db->read(reinterpret_cast<char *>(&m_spielDauerMinuten), sizeof(m_spielDauerMinuten));

        db->read(reinterpret_cast<char *>(&m_erscheinungsjahr), sizeof(m_erscheinungsjahr));

        unsigned int kuenstlerlaenge;
        db->read(reinterpret_cast<char *>(&kuenstlerlaenge), sizeof(kuenstlerlaenge));

        std::string kuenstler;
        kuenstler.resize(kuenstlerlaenge);
        db->read(&kuenstler[0], kuenstlerlaenge);

        m_kuenstler = kuenstler;

        unsigned int genrelaenge;
        db->read(reinterpret_cast<char *>(&genrelaenge), sizeof(genrelaenge));

        std::string genre;
        genre.resize(genrelaenge);
        db->read(&genre[0], genrelaenge);

        m_genre = genre;
    }

    int maxLeihDauer() const override {
        return 21;
    }

    MediumTyp typ() const override {
        return MediumTyp::eCD;
    }


    std::string typName() const override {
        return "CD";
    }

    void serialisieren(std::ofstream *db) const override {
        commonSerialisierung(db, typ());

        // 4 Bytes: m_spielDauerMinuten
        db->write(reinterpret_cast<const char *>(&m_spielDauerMinuten), sizeof(m_spielDauerMinuten));

        // 4 Bytes: m_erscheinungsjahr
        db->write(reinterpret_cast<const char *>(&m_erscheinungsjahr), sizeof(m_erscheinungsjahr));

        // 4 Bytes: kuenstlerlaenge (bytezahl)
        unsigned int kuenstlerlaenge = static_cast<unsigned int>(m_kuenstler.size());
        db->write(reinterpret_cast<const char *>(&kuenstlerlaenge), sizeof(kuenstlerlaenge));

        // X Bytes: m_kuenstler
        db->write(m_kuenstler.data(), m_kuenstler.size());

        // 4 Bytes: genrelaenge (bytezahl)
        unsigned int genrelaenge = static_cast<unsigned int>(m_genre.size());
        db->write(reinterpret_cast<const char *>(&genrelaenge), sizeof(genrelaenge));

        // X Bytes: m_genre
        db->write(m_genre.data(), m_genre.size());
    }

    std::string getKuenstler() {
        return m_kuenstler;
    }

    std::string getGenre() {
        return m_genre;
    }

private:
    std::string m_kuenstler;
    std::string m_genre;
    int m_spielDauerMinuten;
    int m_erscheinungsjahr;
};

class Computerspiel : public Medium {
public:
    explicit Computerspiel(int id, std::string titel, int altersBeschraenkung, int erscheinungsjahr, std::string studio,
                           std::string genre) : Medium(id, titel),
                                                m_altersBeschraenkung(altersBeschraenkung),
                                                m_erscheinungsjahr(erscheinungsjahr),
                                                m_studio(studio), m_genre(genre) {
    }

    explicit Computerspiel(std::ifstream *db, int id): Medium(db, id) {
        db->read(reinterpret_cast<char *>(&m_altersBeschraenkung), sizeof(m_altersBeschraenkung));

        db->read(reinterpret_cast<char *>(&m_erscheinungsjahr), sizeof(m_erscheinungsjahr));

        unsigned int studiolaenger;
        db->read(reinterpret_cast<char *>(&studiolaenger), sizeof(studiolaenger));

        std::string studio;
        studio.resize(studiolaenger);
        db->read(&studio[0], studiolaenger);

        m_studio = studio;

        unsigned int genrelaenge;
        db->read(reinterpret_cast<char *>(&genrelaenge), sizeof(genrelaenge));

        std::string genre;
        genre.resize(genrelaenge);
        db->read(&genre[0], genrelaenge);

        m_genre = genre;
    }

    int maxLeihDauer() const override {
        return 21;
    }

    bool darfAusleihen(int alter) override {
        return alter >= m_altersBeschraenkung;
    }

    MediumTyp typ() const override {
        return MediumTyp::eComputerspiel;
    }

    std::string typName() const override {
        return "PC-Spiel";
    }

    void serialisieren(std::ofstream *db) const override {
        commonSerialisierung(db, typ());

        // 4 Bytes: m_altersBeschraenkung
        db->write(reinterpret_cast<const char *>(&m_altersBeschraenkung), sizeof(m_altersBeschraenkung));

        // 4 Bytes: m_erscheinungsjahr
        db->write(reinterpret_cast<const char *>(&m_erscheinungsjahr), sizeof(m_erscheinungsjahr));

        // 4 Bytes: studiolaenge (bytezahl)
        unsigned int studiolaenge = static_cast<unsigned int>(m_studio.size());
        db->write(reinterpret_cast<const char *>(&studiolaenge), sizeof(studiolaenge));

        // X Bytes: m_studio
        db->write(m_studio.data(), m_studio.size());

        // 4 Bytes: genrelaenge (bytezahl)
        unsigned int genrelaenge = static_cast<unsigned int>(m_genre.size());
        db->write(reinterpret_cast<const char *>(&genrelaenge), sizeof(genrelaenge));

        // X Bytes: m_genre
        db->write(m_genre.data(), m_genre.size());
    }

    std::string getStudio() {
        return m_studio;
    }

    std::string getGenre() {
        return m_genre;
    }

private:
    int m_altersBeschraenkung;
    int m_erscheinungsjahr;
    std::string m_studio;
    std::string m_genre;
};

Medium *Medium::vonDateiLesen(std::ifstream *db) {
    int id;
    db->read(reinterpret_cast<char *>(&id), sizeof(id));

    char type;
    db->read(&type, sizeof(type));
    MediumTyp typ = static_cast<MediumTyp>(type);

    switch (typ) {
        case MediumTyp::eBuch:
            return new Buch(db, id);
        case MediumTyp::eDVD:
            return new DVD(db, id);
        case MediumTyp::eCD:
            return new CD(db, id);
        case MediumTyp::eComputerspiel:
            return new Computerspiel(db, id);
    }
    return nullptr;
}

class Kunde {
public:
    Kunde(int id,
          int alter,
          std::string name,
          std::string vorname,
          std::string adresse,
          std::string email,
          std::string telefon) : m_id(id), m_alter(alter), m_name(name), m_vorname(vorname), m_adresse(adresse),
                                 m_email(email),
                                 m_telefon(telefon) {
    }

    Kunde(std::ifstream *db) {
        db->read(reinterpret_cast<char *>(&m_id), sizeof(m_id));
        db->read(reinterpret_cast<char *>(&m_alter), sizeof(m_alter));

        unsigned int namelaenge;
        db->read(reinterpret_cast<char *>(&namelaenge), sizeof(namelaenge));

        std::string name;
        name.resize(namelaenge);
        db->read(&name[0], namelaenge);
        m_name = name;

        unsigned int vornamelaenge;
        db->read(reinterpret_cast<char *>(&vornamelaenge), sizeof(vornamelaenge));

        std::string vorname;
        vorname.resize(vornamelaenge);
        db->read(&vorname[0], vornamelaenge);
        m_vorname = vorname;

        unsigned int adresselaenge;
        db->read(reinterpret_cast<char *>(&adresselaenge), sizeof(adresselaenge));

        std::string adresse;
        adresse.resize(adresselaenge);
        db->read(&adresse[0], adresselaenge);
        m_adresse = adresse;

        unsigned int emaillaenge;
        db->read(reinterpret_cast<char *>(&emaillaenge), sizeof(emaillaenge));

        std::string email;
        email.resize(emaillaenge);
        db->read(&email[0], emaillaenge);
        m_email = email;

        unsigned int telefonlaenge;
        db->read(reinterpret_cast<char *>(&telefonlaenge), sizeof(telefonlaenge));

        std::string telefon;
        telefon.resize(telefonlaenge);
        db->read(&telefon[0], telefonlaenge);
        m_telefon = telefon;
    }

    int id() const {
        return m_id;
    }

    int alter() const {
        return m_alter;
    }

    std::string name() const {
        return m_name;
    }

    std::string vorname() const {
        return m_vorname;
    }

    std::string adresse() const {
        return m_adresse;
    }

    std::string email() const {
        return m_email;
    }

    std::string telefon() const {
        return m_telefon;
    }

    void serialisieren(std::ofstream *db) const {
        // 4 Bytes: m_id
        db->write(reinterpret_cast<const char *>(&m_id), sizeof(m_id));

        // 4 Bytes: m_alter
        db->write(reinterpret_cast<const char *>(&m_alter), sizeof(m_alter));

        // 4 Bytes: m_name length
        unsigned int namelaenge = static_cast<unsigned int>(m_name.size());
        db->write(reinterpret_cast<const char *>(&namelaenge), sizeof(namelaenge));

        // X Bytes: m_name
        db->write(m_name.data(), m_name.size());

        // 4 Bytes: m_vorname length
        unsigned int vornamelaenge = static_cast<unsigned int>(m_vorname.size());
        db->write(reinterpret_cast<const char *>(&vornamelaenge), sizeof(vornamelaenge));

        // X Bytes: m_vorname
        db->write(m_vorname.data(), m_vorname.size());

        // 4 Bytes: m_adresse length
        unsigned int adresselaenge = static_cast<unsigned int>(m_adresse.size());
        db->write(reinterpret_cast<const char *>(&adresselaenge), sizeof(adresselaenge));

        // X Bytes: m_adresse
        db->write(m_adresse.data(), m_adresse.size());

        // 4 Bytes: m_email length
        unsigned int emaillaenge = static_cast<unsigned int>(m_email.size());
        db->write(reinterpret_cast<const char *>(&emaillaenge), sizeof(emaillaenge));

        // X Bytes: m_email
        db->write(m_email.data(), m_email.size());

        // 4 Bytes: m_telefon length
        unsigned int telefonlaenge = static_cast<unsigned int>(m_telefon.size());
        db->write(reinterpret_cast<const char *>(&telefonlaenge), sizeof(telefonlaenge));

        // X Bytes: m_telefon
        db->write(m_telefon.data(), m_telefon.size());
    }

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
    Leihvorgang(const Kunde *kunde, const Medium *medium) : m_kunden(kunde), m_medium(medium),
                                                            m_ausleihDatum(time(nullptr)) {
    };

    const Kunde *pKunde() const {
        return m_kunden;
    }

    const Medium *pMedium() const {
        return m_medium;
    }

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

    std::string rueckgabeDatum() const {
        time_t rueckgabeDatum = m_ausleihDatum + (m_medium->maxLeihDauer() * 24 * 60 * 60);
        char buffer[11]; // Enough for "dd.mm.yyyy\0"
        size_t size = strftime(buffer, sizeof(buffer), "%d.%m.%Y", localtime(&rueckgabeDatum));

        return std::string(buffer, size);
    }

    void serialisieren(std::ofstream *db, int kundenIndex, int mediumIndex) const {
        // 4 Bytes: kundenIndex
        db->write(reinterpret_cast<const char *>(&kundenIndex), sizeof(kundenIndex));

        // 4 Bytes: mediumIndex
        db->write(reinterpret_cast<const char *>(&mediumIndex), sizeof(mediumIndex));

        // 8 Bytes: m_ausleihDatum
        db->write(reinterpret_cast<const char *>(&m_ausleihDatum), sizeof(m_ausleihDatum));
    }

private:
    const Kunde *m_kunden;
    const Medium *m_medium;
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

    const std::vector<Leihvorgang> &leihvorgaenge() const {
        return m_leihvorgaenge;
    }

    const std::map<int, Medium *> &medien() const {
        return m_medien;
    }

    const std::map<int, Kunde *> &kunden() const {
        return m_kunden;
    }

    // Gesamten Bibliotheksspeicher löschen
    void clear() {
        for (auto &kunde: m_kunden) {
            delete kunde.second;
        }

        for (auto &medium: m_medien) {
            delete medium.second;
        }
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

    std::vector<const Leihvorgang *> sauemigeMedien() {
        std::vector<const Leihvorgang *> sauemigeKunden;
        for (Leihvorgang l: m_leihvorgaenge) {
            if (l.istUeberfaellig()) {
                sauemigeKunden.emplace_back(&l);
            }
        }

        return sauemigeKunden;
    }

    void entfKunde(int ID) {
        // sicherstellen, dass der Kunde alles zurückgegeben hat
        for (const auto &loan: m_leihvorgaenge) {
            if (loan.pKunde()->id() == ID) {
                std::cout << "Cannot delete customer - active loan exists!\n";
                return;
            }
        }
        auto it = m_kunden.find(ID);
        if (it != m_kunden.end()) {
            delete it->second;
            m_kunden.erase(it);
        } else {
            std::cout << "error!\n";
        }
    }

    void entfMedien(int ID) {
        auto it = m_medien.find(ID);
        if (it != m_medien.end()) {
            // Sicherstellen, dass das Medium nicht verliehen ist
            if (it->second->ausgeliehen()) {
                std::cout << "Cannot delete medium - it is currently loaned out!\n";
                return;
            }
            delete it->second;
            m_medien.erase(it);
        } else {
            std::cout << "error!\n";
        }
    }

    // speichert die Bibliothek in einer binär Datei.
    void speichern(std::string datei) {
        // Open the file in binary mode
        std::ofstream db(datei, std::ios::binary);
        if (!db) {
            std::cerr << "Error opening file for writing\n";
            return;
        }

        // 4 Bytes: Medienanzahl
        unsigned int medienZahl = static_cast<unsigned int>(m_medien.size());
        db.write(reinterpret_cast<const char *>(&medienZahl), sizeof(medienZahl));

        std::unordered_map<int, int> kundenIdMap;
        std::unordered_map<int, int> medienIdMap;

        int medienIndex = 0;
        for (auto &medium: m_medien) {
            medienIdMap[medium.first] = medienIndex;
            medium.second->serialisieren(&db);
            medienIndex += 1;
        }


        // 4 Bytes: Kundenzahl
        unsigned int kundenzahl = static_cast<unsigned int>(m_kunden.size());
        db.write(reinterpret_cast<const char *>(&kundenzahl), sizeof(kundenzahl));

        int kundenIndex = 0;
        for (auto &kunde: m_kunden) {
            kundenIdMap[kunde.first] = kundenIndex;
            kunde.second->serialisieren(&db);

            kundenIndex += 1;
        }

        // 4 Bytes: Leihvorgangzahl
        unsigned int leihvorgangZahl = static_cast<unsigned int>(m_leihvorgaenge.size());
        db.write(reinterpret_cast<const char *>(&leihvorgangZahl), sizeof(leihvorgangZahl));

        for (auto &leihvorgang: m_leihvorgaenge) {
            leihvorgang.serialisieren(&db, kundenIdMap[leihvorgang.pKunde()->id()],
                                      medienIdMap[leihvorgang.pMedium()->id()]);
        }

        db.close();
    }

    bool lesen(std::string datei) {
        std::ifstream db(datei, std::ios::binary);
        if (!db.good()) {
            return false;
        }

        unsigned int medienAnzahl;
        db.read(reinterpret_cast<char *>(&medienAnzahl), sizeof medienAnzahl);

        for (int i = 0; i < medienAnzahl; i++) {
            Medium *ptr = Medium::vonDateiLesen(&db);
            m_medien[ptr->id()] = ptr;
        }

        unsigned int kundenzahl;
        db.read(reinterpret_cast<char *>(&kundenzahl), sizeof kundenzahl);

        for (int i = 0; i < kundenzahl; i++) {
            Kunde *ptr = new Kunde(&db);
            m_kunden[ptr->id()] = ptr;
        }

        unsigned int leihvorgangzahl;
        db.read(reinterpret_cast<char *>(&leihvorgangzahl), sizeof leihvorgangzahl);

        for (int i = 0; i < leihvorgangzahl; i++) {
            int kundenIndex;
            int medienIndex;
            time_t ausleihDatum;

            db.read(reinterpret_cast<char *>(&kundenIndex), sizeof(kundenIndex));
            db.read(reinterpret_cast<char *>(&medienIndex), sizeof(medienIndex));
            db.read(reinterpret_cast<char *>(&ausleihDatum), sizeof(ausleihDatum));

            m_leihvorgaenge.emplace_back(m_kunden[kundenIndex], m_medien[medienIndex]);
        }

        return true;
    }

private:
    std::map<int, Medium *> m_medien;
    std::map<int, Kunde *> m_kunden;
    std::vector<Leihvorgang> m_leihvorgaenge;
};

//----------------Bibliothek global deklariert---------------------

Bibliothek bib;

//---------Automatenzeug Zustandsüberführung und Ausgabe------------

int delta(int Zustand, std::string Eingabe) {
    int FZ = -1;
    switch (Zustand) {
        case 0:
            if (Eingabe == "liste") {
                FZ = 1;
            } else if (Eingabe == "akt") {
                FZ = 2;
            } else if (Eingabe == "be") {
                FZ = 16;
            } else {
                std::cout << "error!\n";
                FZ = 0;
            }
            break;
        case 1:
            FZ = 0;
            break;
        case 2:
            if (Eingabe == "hinzu") {
                FZ = 3;
            } else if (Eingabe == "entf") {
                FZ = 10;
            } else if (Eingabe == "sCH") {
                FZ = 13;
            } else if (Eingabe == "verl") {
                FZ = 14;
            } else if (Eingabe == "ruck") {
                FZ = 15;
            } else {
                std::cout << "error!\n";
                FZ = 0;
            }
            break;
        case 3:
            if (Eingabe == "k") {
                FZ = 4;
            } else if (Eingabe == "m") {
                FZ = 5;
            } else {
                std::cout << "error!\n";
                FZ = 0;
            }
            break;
        case 4:
            FZ = 0;
            break;
        case 5:
            if (Eingabe == "CD") {
                FZ = 6;
            } else if (Eingabe == "B") {
                FZ = 7;
            } else if (Eingabe == "D") {
                FZ = 8;
            } else if (Eingabe == "C") {
                FZ = 9;
            } else {
                std::cout << "error!\n";
                FZ = 0;
            }
            break;
        case 6:
            FZ = 0;
            break;
        case 7:
            FZ = 0;
            break;
        case 8:
            FZ = 0;
            break;
        case 9:
            FZ = 0;
            break;
        case 10:
            if (Eingabe == "k") {
                FZ = 11;
            } else if (Eingabe == "m") {
                FZ = 12;
            } else {
                std::cout << "error!\n";
                FZ = 0;
            }
            break;
        case 11:
            FZ = 0;
            break;
        case 12:
            FZ = 0;
            break;
        case 13:
            FZ = 0;
            break;
        case 14:
            FZ = 0;
            break;
        case 15:
            FZ = 0;
            break;
    }
    return FZ;
}

std::string lambda(int Zustand, int &KID, int &MID) {
    std::string FZ = "";
    switch (Zustand) {
        case 0:
            // Startzustand
            std::cout << "\n-----------------------------------------------------\n";
            std::cout << "Uebersicht der Bibliothek (liste)\nAktion (akt)\nBeenden(be)\n-> ";
            std::getline(std::cin, FZ);
            break;
        case 1:
            // Listenaufruf
            std::cout << "\n---------- Aktueller Status der Bibliothek ----------\n";
            std::cout << "Registrierte Kunden:\n";
            std::cout << std::left
                    << std::setw(5) << "ID"
                    << std::setw(6) << "Alter"
                    << std::setw(12) << "Name"
                    << std::setw(12) << "Vorname"
                    << std::setw(20) << "Adresse"
                    << std::setw(30) << "E-Mail"
                    << std::setw(15) << "Telefon"
                    << "\n";

            std::cout << std::setfill('-') << std::setw(100) << "-" << std::setfill(' ') << "\n";

            for (const auto &kunde: bib.kunden()) {
                std::cout << std::left
                        << std::setw(5) << kunde.second->id()
                        << std::setw(6) << kunde.second->alter()
                        << std::setw(12) << kunde.second->name()
                        << std::setw(12) << kunde.second->vorname()
                        << std::setw(20) << kunde.second->adresse()
                        << std::setw(30) << kunde.second->email()
                        << std::setw(15) << kunde.second->telefon()
                        << "\n";
            }

            std::cout << "\nVerfuegbare Medien:\n";
            std::cout << std::left
                    << std::setw(5) << "ID"
                    << std::setw(12) << "Typ"
                    << std::setw(10) << "Ausgl."
                    << std::setw(40) << "Titel"
                    << std::setw(40) << "Details"
                    << "\n";
            std::cout << std::setfill('-') << std::setw(107) << "-" << std::setfill(' ') << "\n";

            for (const auto &entry: bib.medien()) {
                Medium *m = entry.second;
                std::string details = "";

                if (m->typ() == Medium::MediumTyp::eBuch) {
                    // Cast to Buch and get additional information.
                    auto buch = dynamic_cast<Buch *>(m);
                    if (buch) {
                        details = "Autor: " + buch->getAuthor() + ", Genre: " + buch->getGenre();
                    }
                } else if (m->typ() == Medium::MediumTyp::eDVD) {
                    auto dvd = dynamic_cast<DVD *>(m);
                    if (dvd) {
                        details = "Regisseur: " + dvd->getRegisseur() + ", Genre: " + dvd->getGenre();
                    }
                } else if (m->typ() == Medium::MediumTyp::eCD) {
                    auto cd = dynamic_cast<CD *>(m);
                    if (cd) {
                        details = "Kuenstler: " + cd->getKuenstler() + ", Genre: " + cd->getGenre();
                    }
                } else if (m->typ() == Medium::MediumTyp::eComputerspiel) {
                    auto spiel = dynamic_cast<Computerspiel *>(m);
                    if (spiel) {
                        details = "Studio: " + spiel->getStudio() + ", Genre: " + spiel->getGenre();
                    }
                }

                std::cout << std::left
                        << std::setw(5) << m->id()
                        << std::setw(12) << m->typName()
                        << std::setw(10) << (m->ausgeliehen() ? "ja" : "nein")
                        << std::setw(40) << m->titel()
                        << std::setw(40) << details
                        << "\n";
            }
            std::cout << "\nAktuelle Leihvorgaenge:\n";

            std::cout << std::left
                    << std::setw(12) << "Typ"
                    << std::setw(15) << "Verzugs-Tage"
                    << std::setw(15) << "R-Datum"
                    << std::setw(5) << "K-ID"
                    << std::setw(12) << "Vorname"
                    << std::setw(12) << "Name"
                    << std::setw(5) << "M-ID"
                    << std::setw(25) << "Medium"
                    << "\n";

        // Adjust the width to match the total column size
            std::cout << std::setfill('-') << std::setw(86) << "-" << std::setfill(' ') << "\n";

            for (const auto &l: bib.leihvorgaenge()) {
                std::cout << std::left
                        << std::setw(12) << l.pMedium()->typName()
                        << std::setw(15) << l.ueberfaelligTage()
                        << std::setw(15) << l.rueckgabeDatum()
                        << std::setw(5) << l.pKunde()->id()
                        << std::setw(12) << l.pKunde()->vorname()
                        << std::setw(12) << l.pKunde()->name()
                        << std::setw(5) << l.pMedium()->id()
                        << std::setw(25) << l.pMedium()->titel()
                        << "\n";
            }

            break;
        case 2:
            //Aktionsauswahl
            std::cout << "\n-----------------------------------------------------\n";
            std::cout <<
                    "Hinzufuegen(hinzu)\nEntfernen(entf)\nsaeumige Kunden-Check(sCH)\nVerleihen(verl)\nRueckgabe(ruck)\n-> ";
            std::getline(std::cin, FZ);
            break;
        case 3:
            // Auswahl für Hinzufuegen
            std::cout << "\nKunde(k)\nMedium(m)\n-> ";
            std::getline(std::cin, FZ);
            break;
        case 4: {
            // Kunde Hinzu
            int Alter = 0;
            std::string NN;
            std::string VN;
            std::string Ad;
            std::string Em;
            std::string Tel;
            // Eingabe
            std::cout << "Alter: ";
            while (!(std::cin >> Alter)) {
                // Prüft, ob die Eingabe fehlerhaft ist
                std::cin.clear(); // Fehlerzustand zurücksetzen
                std::cin.ignore(10000, '\n'); // Ungültige Eingabe aus dem Eingabepuffer löschen
                std::cout << "Ungueltige Eingabe! Bitte eine Zahl eingeben\n-> ";
            }
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            // Puffer leeren, damit `getline` korrekt funktioniert
            std::cout << "Nachname: ";
            std::getline(std::cin, NN);
            std::cout << "Vorname: ";
            std::getline(std::cin, VN);
            std::cout << "Addresse: ";
            std::getline(std::cin, Ad);
            std::cout << "E-mail: ";
            std::getline(std::cin, Em);
            std::cout << "Telefon-Nr.: ";
            std::getline(std::cin, Tel);
            // Erstellen des Kunden
            auto *k = new Kunde(KID, Alter, NN, VN, Ad, Em, Tel);
            bib.kundeHinzufuegen(k);
            KID++;
            break;
        }
        case 5:
            // Medium Auswahl
            std::cout << "\nCD(CD)\nBuch(B)\nDVD(D)\nComputerspiel(C)\n-> ";
            std::getline(std::cin, FZ);
            break;
        case 6: {
            // CD Hinzu
            std::string Titel;
            std::string genre;
            std::string kuenstler;
            int spielDauerMinuten;
            int erscheinungsjahr;

            std::cout << "Titel: ";
            std::getline(std::cin, Titel);

            std::cout << "Genre: ";
            std::getline(std::cin, genre);

            std::cout << "Kuenstler: ";
            std::getline(std::cin, kuenstler);

            std::cout << "Spieldauer (minuten): ";
            while (!(std::cin >> spielDauerMinuten)) {
                // Prüft, ob die Eingabe fehlerhaft ist
                std::cin.clear(); // Fehlerzustand zurücksetzen
                std::cin.ignore(10000, '\n'); // Ungültige Eingabe aus dem Eingabepuffer löschen
                std::cout << "Ungueltige Eingabe! Bitte eine Zahl eingeben\n-> ";
            }
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

            std::cout << "Erscheinungsjahr: ";
            while (!(std::cin >> erscheinungsjahr)) {
                // Prüft, ob die Eingabe fehlerhaft ist
                std::cin.clear(); // Fehlerzustand zurücksetzen
                std::cin.ignore(10000, '\n'); // Ungültige Eingabe aus dem Eingabepuffer löschen
                std::cout << "Ungueltige Eingabe! Bitte eine Zahl eingeben\n-> ";
            }
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

            auto *cd = new CD(MID, Titel, genre, kuenstler, spielDauerMinuten, erscheinungsjahr);
            bib.mediumHinzufuegen(cd);
            MID++;
            break;
        }
        case 7: {
            //Buch hinzu
            std::string Titel;
            std::string verlag;
            std::string genre;
            std::string author;
            int erscheinungsjahr;
            int seitenzahl;

            std::cout << "Titel: ";
            std::getline(std::cin, Titel);

            std::cout << "Verlag: ";
            std::getline(std::cin, verlag);

            std::cout << "Genre: ";
            std::getline(std::cin, genre);

            std::cout << "Author: ";
            std::getline(std::cin, author);

            std::cout << "Erscheinungsjahr: ";
            while (!(std::cin >> erscheinungsjahr)) {
                // Prüft, ob die Eingabe fehlerhaft ist
                std::cin.clear(); // Fehlerzustand zurücksetzen
                std::cin.ignore(10000, '\n'); // Ungültige Eingabe aus dem Eingabepuffer löschen
                std::cout << "Ungueltige Eingabe! Bitte eine Zahl eingeben\n-> ";
            }
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

            std::cout << "Seitenzahl: ";
            while (!(std::cin >> seitenzahl)) {
                // Prüft, ob die Eingabe fehlerhaft ist
                std::cin.clear(); // Fehlerzustand zurücksetzen
                std::cin.ignore(10000, '\n'); // Ungültige Eingabe aus dem Eingabepuffer löschen
                std::cout << "Ungueltige Eingabe! Bitte eine Zahl eingeben\n-> ";
            }
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

            auto *buch = new Buch(MID, Titel, author, verlag, genre, erscheinungsjahr, seitenzahl);
            bib.mediumHinzufuegen(buch);
            MID++;
            break;
        }
        case 8: {
            //DVD hinzu
            std::string Titel;
            std::string regisseur;
            std::string genre;
            std::string hauptdarsteller;
            int AB;
            int spielDauerMinuten;
            int erscheinungsJahr;

            std::cout << "Titel: ";
            std::getline(std::cin, Titel);

            std::cout << "Regisseur: ";
            std::getline(std::cin, regisseur);

            std::cout << "Genre: ";
            std::getline(std::cin, genre);

            std::cout << "Alterbeschraenkung: ";
            while (!(std::cin >> AB)) {
                // Prüft, ob die Eingabe fehlerhaft ist
                std::cin.clear(); // Fehlerzustand zurücksetzen
                std::cin.ignore(10000, '\n'); // Ungültige Eingabe aus dem Eingabepuffer löschen
                std::cout << "Ungueltige Eingabe! Bitte eine Zahl eingeben\n-> ";
            }
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

            std::cout << "Spieldauer (minuten): ";
            while (!(std::cin >> spielDauerMinuten)) {
                // Prüft, ob die Eingabe fehlerhaft ist
                std::cin.clear(); // Fehlerzustand zurücksetzen
                std::cin.ignore(10000, '\n'); // Ungültige Eingabe aus dem Eingabepuffer löschen
                std::cout << "Ungueltige Eingabe! Bitte eine Zahl eingeben\n-> ";
            }
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

            std::cout << "Erscheinungsjahr: ";
            while (!(std::cin >> erscheinungsJahr)) {
                // Prüft, ob die Eingabe fehlerhaft ist
                std::cin.clear(); // Fehlerzustand zurücksetzen
                std::cin.ignore(10000, '\n'); // Ungültige Eingabe aus dem Eingabepuffer löschen
                std::cout << "Ungueltige Eingabe! Bitte eine Zahl eingeben: ";
            }
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

            std::cout << "Hauptdarsteller: ";
            while (!(std::cin >> hauptdarsteller)) {
                // Prüft, ob die Eingabe fehlerhaft ist
                std::cin.clear(); // Fehlerzustand zurücksetzen
                std::cin.ignore(10000, '\n'); // Ungültige Eingabe aus dem Eingabepuffer löschen
                std::cout << "Ungueltige Eingabe! Bitte eine Zahl eingeben\n-> ";
            }
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

            auto *dvd = new DVD(MID, Titel, AB, spielDauerMinuten, erscheinungsJahr, regisseur, hauptdarsteller, genre);
            bib.mediumHinzufuegen(dvd);
            MID++;
            break;
        }
        case 9: {
            // Comp hinzu
            std::string Titel;
            std::string studio;
            std::string genre;
            int AB;
            int erscheinungsjahr;

            std::cout << "Titel: ";
            std::getline(std::cin, Titel);

            std::cout << "Studio: ";
            std::getline(std::cin, studio);

            std::cout << "Genre: ";
            std::getline(std::cin, genre);

            std::cout << "Alterbeschraenkung: ";
            while (!(std::cin >> AB)) {
                // Prüft, ob die Eingabe fehlerhaft ist
                std::cin.clear(); // Fehlerzustand zurücksetzen
                std::cin.ignore(10000, '\n'); // Ungültige Eingabe aus dem Eingabepuffer löschen
                std::cout << "Ungueltige Eingabe! Bitte eine Zahl eingeben\n-> ";
            }
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');


            std::cout << "Erscheinungsjahr: ";
            while (!(std::cin >> erscheinungsjahr)) {
                // Prüft, ob die Eingabe fehlerhaft ist
                std::cin.clear(); // Fehlerzustand zurücksetzen
                std::cin.ignore(10000, '\n'); // Ungültige Eingabe aus dem Eingabepuffer löschen
                std::cout << "Ungueltige Eingabe! Bitte eine Zahl eingeben\n-> ";
            }
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

            auto *comp = new Computerspiel(MID, Titel, AB, erscheinungsjahr, studio, genre);
            bib.mediumHinzufuegen(comp);
            MID++;
            break;
        }
        case 10:
            //Auswahl entf
            std::cout << "Kunde(k) oder Medium(m):";
            std::getline(std::cin, FZ);
            break;
        case 11: {
            // Kunde entf
            int ID;

            std::cout << "ID des Kunden: ";
            while (!(std::cin >> ID)) {
                // Prüft, ob die Eingabe fehlerhaft ist
                std::cin.clear(); // Fehlerzustand zurücksetzen
                std::cin.ignore(10000, '\n'); // Ungültige Eingabe aus dem Eingabepuffer löschen
                std::cout << "Ungueltige Eingabe! Bitte eine Zahl eingeben\n-> ";
            }
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

            bib.entfKunde(ID);
            break;
        }
        case 12:
            // Medium entf
            int ID;

            std::cout << "ID des Mediums:";
            while (!(std::cin >> ID)) {
                // Prüft, ob die Eingabe fehlerhaft ist
                std::cin.clear(); // Fehlerzustand zurücksetzen
                std::cin.ignore(10000, '\n'); // Ungültige Eingabe aus dem Eingabepuffer löschen
                std::cout << "Ungueltige Eingabe! Bitte eine Zahl eingeben\n-> ";
            }
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

            bib.entfMedien(ID);
            break;
        case 13: {
            // Ausgabe der Tabelle für säumige Kunden (überfällige Leihvorgänge)
            std::cout << "\n------------------ Säumige Medien ------------------\n";
            std::cout << std::left
                    << std::setw(5) << "CID"
                    << std::setw(12) << "Name"
                    << std::setw(12) << "Vorname"
                    << std::setw(20) << "Medium"
                    << std::setw(15) << "Verzugs-Tage"
                    << "\n";
            std::cout << std::setfill('-') << std::setw(64) << "-"
                    << std::setfill(' ') << "\n";

            for (const auto &loan: bib.sauemigeMedien()) {
                const Kunde *k = loan->pKunde();
                const Medium *m = loan->pMedium();
                std::cout << std::left
                        << std::setw(5) << k->id()
                        << std::setw(12) << k->name()
                        << std::setw(12) << k->vorname()
                        << std::setw(20) << m->titel()
                        << std::setw(15) << loan->ueberfaelligTage()
                        << "\n";
            }
            break;
        }
        case 14: {
            //Verleihen
            int KuID;
            int MeID;
            Kunde *kundep = nullptr;
            Medium *medienp = nullptr;

            std::cout << "ID des Kunden:";
            while (!(std::cin >> KuID)) {
                // Prüft, ob die Eingabe fehlerhaft ist
                std::cin.clear(); // Fehlerzustand zurücksetzen
                std::cin.ignore(10000, '\n'); // Ungültige Eingabe aus dem Eingabepuffer löschen
                std::cout << "Ungueltige Eingabe! Bitte eine Zahl eingeben\n-> ";
            }
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::map<int, Kunde *> kunde = bib.kunden();

            auto gefKunde = kunde.find(KuID);
            if (gefKunde != kunde.end()) {
                kundep = gefKunde->second;
            } else {
                std::cout << "Kunde nicht Gefunden!\n";
                break;
            }

            std::cout << "ID des Mediums:";
            while (!(std::cin >> MeID)) {
                // Prüft, ob die Eingabe fehlerhaft ist
                std::cin.clear(); // Fehlerzustand zurücksetzen
                std::cin.ignore(10000, '\n'); // Ungültige Eingabe aus dem Eingabepuffer löschen
                std::cout << "Ungueltige Eingabe! Bitte eine Zahl eingeben\n-> ";
            }
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

            std::map<int, Medium *> medien = bib.medien();
            auto gefundenesMedium = medien.find(MeID);
            if (gefundenesMedium != medien.end()) {
                medienp = gefundenesMedium->second;
            } else {
                std::cout << "Medium nicht Gefunden!\n";
                break;
            }

            auto result = bib.ausleihen(kundep, medienp);
            std::cout << "Ergebnis: " << (result == Bibliothek::AusleihFehler::eErfolg ? "Erfolg" : "Fehler") << "\n";
            break;
        }
        case 15:
            int KuID;
            int MeID;
            Kunde *kundep = nullptr;
            Medium *medienp = nullptr;

            std::cout << "ID des Kunden:";
            while (!(std::cin >> KuID)) {
                // Prüft, ob die Eingabe fehlerhaft ist
                std::cin.clear(); // Fehlerzustand zurücksetzen
                std::cin.ignore(10000, '\n'); // Ungültige Eingabe aus dem Eingabepuffer löschen
                std::cout << "Ungueltige Eingabe! Bitte eine Zahl eingeben\n-> ";
            }
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

            std::map<int, Kunde *> kunde = bib.kunden();
            auto gefKunde = kunde.find(KuID);
            if (gefKunde != kunde.end()) {
                kundep = gefKunde->second;
            } else {
                std::cout << "Kunde nicht Gefunden!\n";
                break;
            }

            std::cout << "ID des Mediums:";
            while (!(std::cin >> MeID)) {
                // Prüft, ob die Eingabe fehlerhaft ist
                std::cin.clear(); // Fehlerzustand zurücksetzen
                std::cin.ignore(10000, '\n'); // Ungültige Eingabe aus dem Eingabepuffer löschen
                std::cout << "Ungueltige Eingabe! Bitte eine Zahl eingeben\n-> ";
            }
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

            std::map<int, Medium *> medien = bib.medien();
            auto gefundenesMedium = medien.find(MeID);
            if (gefundenesMedium != medien.end()) {
                medienp = gefundenesMedium->second;
            } else {
                std::cout << "Medium nicht Gefunden!\n";
                break;
            }

            auto rueckgabe = bib.zurueckgeben(kundep, medienp);
            std::cout << "Ergebnis: " << (rueckgabe == Bibliothek::RueckgabeFehler::eErfolg ? "Erfolg" : "Fehler")
                    << "\n";
            break;
    }
    return FZ;
}

int main() {
    const std::string dateiName = "daten.db";

    // wenn datenbank nicht existiert, basiskunden anlegen
    if (!bib.lesen(dateiName)) {
        // --- KUNDEN ANLEGEN ---
        auto *k0 = new Kunde(0, 21, "Mueller", "Max", "Hauptstrasse 12", "max.m@example.com", "015112345678");
        auto *k1 = new Kunde(1, 17, "Schneider", "Lena", "Bahnhofstrasse 5", "lena.s@example.com", "017312345678");
        auto *k2 = new Kunde(2, 14, "Weber", "Felix", "Am Park 8", "felix.w@example.com", "016012345678");
        auto *k3 = new Kunde(3, 35, "Schmidt", "Anna", "Gartenweg 3", "anna.s@example.com", "017612345678");
        auto *k4 = new Kunde(4, 29, "Fischer", "Tom", "Bergstrasse 22", "tom.f@example.com", "017412345678");
        auto *k5 = new Kunde(5, 23, "Wagner", "Lisa", "Ringstrasse 10", "lisa.w@example.com", "015212345678");
        auto *k6 = new Kunde(6, 41, "Becker", "Markus", "Lindenweg 7", "markus.b@example.com", "017912345678");
        auto *k7 = new Kunde(7, 19, "Hoffmann", "Sophie", "Schulstrasse 15", "sophie.h@example.com", "016512345678");
        auto *k8 = new Kunde(8, 33, "Krause", "Julian", "Muehlenweg 4", "julian.k@example.com", "015812345678");
        auto *k9 = new Kunde(9, 26, "Lehmann", "Emma", "Dorfstrasse 9", "emma.l@example.com", "017012345678");

        // --- MEDIEN ANLEGEN ---
        auto *buch1 = new Buch(0, "Herr der Ringe", "J. R. R. Tolkien", "Klett-Cotta Verlag", "High Fantasy", 1969,
                               1292);
        auto *dvd1 = new DVD(1, "Harry Potter und der Stein der Weisen", 6, 152, 2001, "Christopher Columbus",
                             "Daniel Radcliff", "Fantasy");
        auto *cd1 = new CD(2, "CD 1", "Rock", "Band XYZ", 45, 2020);
        auto *spiel1 = new Computerspiel(3, "Game 1", 12, 2018, "Game Studio ABC", "Action-Adventure");

        // --- KUNDEN UND MEDIEN ZUR BIBLIOTHEK HINZUFÜGEN ---
        bib.kundeHinzufuegen(k0);
        bib.kundeHinzufuegen(k1);
        bib.kundeHinzufuegen(k2);
        bib.kundeHinzufuegen(k3);
        bib.kundeHinzufuegen(k4);
        bib.kundeHinzufuegen(k5);
        bib.kundeHinzufuegen(k6);
        bib.kundeHinzufuegen(k7);
        bib.kundeHinzufuegen(k8);
        bib.kundeHinzufuegen(k9);
        bib.mediumHinzufuegen(buch1);
        bib.mediumHinzufuegen(dvd1);
        bib.mediumHinzufuegen(cd1);
        bib.mediumHinzufuegen(spiel1);
    }

    // --- Automat ---
    int KID = 10;
    int MID = 4;
    int FZ = 0;
    std::cout << "\n----------------------- START -----------------------\n"; //
    while (FZ != 16) {
        std::string const anfrage = lambda(FZ, KID, MID);
        FZ = delta(FZ, anfrage);
    }
    std::cout << "\n----------------------- ENDE -----------------------\n";
    // --- AUFRÄUMEN ---
    // daten in datei speichern
    bib.speichern(dateiName);
    // speicher freigeben
    bib.clear();

    return 0;
}
