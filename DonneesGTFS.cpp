//
// Created by Mario Marchand on 16-12-29.
//

#include "DonneesGTFS.h"

using namespace std;

//! \brief construit un objet GTFS
//! \param[in] p_date: la date utilisée par le GTFS
//! \param[in] p_now1: l'heure du début de l'intervalle considéré
//! \param[in] p_now2: l'heure de fin de l'intervalle considéré
//! \brief Ces deux heures définissent l'intervalle de temps du GTFS; seuls les moments de [p_now1, p_now2) sont considérés
DonneesGTFS::DonneesGTFS(const Date &p_date, const Heure &p_now1, const Heure &p_now2)
        : m_date(p_date), m_now1(p_now1), m_now2(p_now2), m_nbArrets(0), m_tousLesArretsPresents(false)
{
}

//! \brief partitionne un string en un vecteur de strings
//! \param[in] s: le string à être partitionner
//! \param[in] delim: le caractère utilisé pour le partitionnement
//! \return le vecteur de string sans le caractère utilisé pour le partitionnement
vector<string> DonneesGTFS::string_to_vector(const string &s, char delim)
{
    stringstream ss(s);
    string item;
    vector<string> elems;
    while (getline(ss, item, delim))
    {
        elems.push_back(item);
    }
    return elems;
}

//! \brief ajoute les lignes dans l'objet GTFS
//! \param[in] p_nomFichier: le nom du fichier contenant les lignes
//! \throws logic_error si un problème survient avec la lecture du fichier
void DonneesGTFS::ajouterLignes(const std::string &p_nomFichier)
{
    try {
        ifstream file(p_nomFichier);

        if (file.good()) {
            string line;
            getline(file, line);

            vector<string> headers = string_to_vector(line, ',');
            std::unordered_map<string, unsigned int> headers_map = {
                    {"route_id",         0},
                    {"route_short_name", 2},
                    {"route_desc",       4},
                    {"route_color",      7}
            };

            for (line; getline(file, line);) {
                vector<string> champs = string_to_vector(line, ',');

                line.erase(std::remove(line.begin(), line.end(), '"'), line.end());
                line.erase(std::remove(line.begin(), line.end(), '\r'), line.end());

                Ligne ligne(
                        (unsigned) stoi(champs[headers_map.at("route_id")]),
                        champs[headers_map.at("route_short_name")],
                        champs[headers_map.at("route_desc")],
                        Ligne::couleurToCategorie(champs[headers_map.at("route_color")])
                );

                m_lignes.insert({ligne.getId(), ligne});
                m_lignes_par_numero.insert({ligne.getNumero(), ligne});
            }
        } else {

        }
    } catch(const ifstream::failure& e) {
        throw logic_error("Une erreur est survenue lors de la lecture du fichier.");
    }
}

//! \brief ajoute les stations dans l'objet GTFS
//! \param[in] p_nomFichier: le nom du fichier contenant les station
//! \throws logic_error si un problème survient avec la lecture du fichier
void DonneesGTFS::ajouterStations(const std::string &p_nomFichier)
{
    try {
        ifstream file(p_nomFichier);

        if (file.good()) {
            string line;
            getline(file, line);

            vector<string> headers = string_to_vector(line, ',');
            std::unordered_map<string, unsigned int> headers_map = {
                    {"stop_id",   0},
                    {"stop_name", 1},
                    {"stop_desc", 2},
                    {"stop_lat",  3},
                    {"stop_lon",  4}
            };

            for (line; getline(file, line);) {
                vector<string> champs = string_to_vector(line, ',');

                line.erase(std::remove(line.begin(), line.end(), '"'), line.end());
                line.erase(std::remove(line.begin(), line.end(), '\r'), line.end());

                Coordonnees coord(stod(champs[headers_map.at("stop_lat")]), stod(champs[headers_map.at("stop_lon")]));

                Station station(
                        (unsigned) stoi(champs[headers_map.at("stop_id")]),
                        champs[headers_map.at("stop_name")],
                        champs[headers_map.at("stop_desc")],
                        coord
                );

                m_stations.insert({station.getId(), station});
            }
        }
    } catch(const ifstream::failure& e) {
        throw logic_error("Une erreur est survenue lors de la lecture du fichier.");
    }
}

//! \brief ajoute les transferts dans l'objet GTFS
//! \breif Cette méthode doit âtre utilisée uniquement après que tous les arrêts ont été ajoutés
//! \brief les transferts (entre stations) ajoutés sont uniquement ceux pour lesquelles les stations sont prensentes dans l'objet GTFS
//! \brief les transferts d'une station à elle m^e ne sont pas ajoutés
//! \param[in] p_nomFichier: le nom du fichier contenant les station
//! \throws logic_error si un problème survient avec la lecture du fichier
//! \throws logic_error si tous les arrets de la date et de l'intervalle n'ont pas été ajoutés
void DonneesGTFS::ajouterTransferts(const std::string &p_nomFichier)
{
    try {
        if (m_tousLesArretsPresents) {
            ifstream file(p_nomFichier);

            if (file.good()) {
                string line;
                getline(file, line);

                vector<string> headers = string_to_vector(line, ',');
                std::unordered_map<string, unsigned int> headers_map = {
                        {"from_stop_id",      0},
                        {"to_stop_id",        1},
                        {"min_transfer_time", 3}
                };

                for (line; getline(file, line);) {
                    vector<string> champs = string_to_vector(line, ',');

                    line.erase(std::remove(line.begin(), line.end(), '"'), line.end());
                    line.erase(std::remove(line.begin(), line.end(), '\r'), line.end());

                    const unsigned int from_stop_id = (unsigned) stoi(champs[headers_map.at("from_stop_id")]);
                    const unsigned int to_stop_id = (unsigned) stoi(champs[headers_map.at("to_stop_id")]);
                    unsigned int min_transfer_time = (unsigned) stoi(champs[headers_map.at("min_transfer_time")]);

                    if (from_stop_id != to_stop_id && m_stations.count(from_stop_id) && m_stations.count(to_stop_id)) {
                        if (min_transfer_time == 0)
                            min_transfer_time = 1;

                        m_transferts.push_back(make_tuple(from_stop_id, to_stop_id, min_transfer_time));
                    }
                }
            }
        } else {
            throw logic_error("Tous les arrêts de la date et de l'intervalle n'ont pas été ajoutés.");
        }
    } catch(const ifstream::failure& e) {
        throw logic_error("Une erreur est survenue lors de la lecture du fichier.");
    }
}


//! \brief ajoute les services de la date du GTFS (m_date)
//! \param[in] p_nomFichier: le nom du fichier contenant les services
//! \throws logic_error si un problème survient avec la lecture du fichier
void DonneesGTFS::ajouterServices(const std::string &p_nomFichier)
{
    try {
        ifstream file(p_nomFichier);

        if (file.good()) {
            string line;
            getline(file, line);

            vector<string> headers = string_to_vector(line, ',');
            std::unordered_map<string, unsigned int> headers_map = {
                    {"service_id",     0},
                    {"date",           1},
                    {"exception_type", 2}
            };

            for (line; getline(file, line);) {
                vector<string> champs = string_to_vector(line, ',');

                line.erase(std::remove(line.begin(), line.end(), '"'), line.end());
                line.erase(std::remove(line.begin(), line.end(), '\r'), line.end());

                string str_date = champs[headers_map.at("date")];
                Date date(
                        (unsigned) stoi(str_date.substr(0, 4)),
                        (unsigned) stoi(str_date.substr(4, 2)),
                        (unsigned) stoi(str_date.substr(6, 2))
                );

                if (stoi(champs[headers_map.at("exception_type")]) == 1 && m_date == date) {
                    m_services.insert({champs[headers_map.at("service_id")]});
                }
            }
        }
    } catch (const ifstream::failure& e) {
        throw logic_error("Une erreur est survenue lors de la lecture du fichier.");
    }
}

//! \brief ajoute les voyages de la date
//! \brief seuls les voyages dont le service est présent dans l'objet GTFS sont ajoutés
//! \param[in] p_nomFichier: le nom du fichier contenant les voyages
//! \throws logic_error si un problème survient avec la lecture du fichier
void DonneesGTFS::ajouterVoyagesDeLaDate(const std::string &p_nomFichier)
{
    try {
        ifstream file(p_nomFichier);

        if (file.good()) {
            string line;
            getline(file, line);

            vector<string> headers = string_to_vector(line, ',');
            std::unordered_map<string, unsigned int> headers_map = {
                    {"route_id",      0},
                    {"service_id",    1},
                    {"trip_id",       2},
                    {"trip_headsign", 3}
            };

            for (line; getline(file, line);) {
                vector<string> champs = string_to_vector(line, ',');

                line.erase(std::remove(line.begin(), line.end(), '"'), line.end());
                line.erase(std::remove(line.begin(), line.end(), '\r'), line.end());

                string service_id = champs[headers_map.at("service_id")];

                if (m_services.count(service_id)) {
                    string trip_id = champs[headers_map.at("trip_id")];

                    Voyage voyage = Voyage(
                            trip_id,
                            (unsigned) stoi(champs[headers_map.at("route_id")]),
                            service_id,
                            champs[headers_map.at("trip_headsign")]
                    );

                    m_voyages.insert({trip_id, voyage});
                }
            }
        }
    } catch (const ifstream::failure& e) {
        throw logic_error("Une erreur est survenue lors de la lecture du fichier.");
    }
}

//! \brief ajoute les arrets aux voyages présents dans le GTFS si l'heure du voyage appartient à l'intervalle de temps du GTFS
//! \brief De plus, on enlève les voyages qui n'ont pas d'arrêts dans l'intervalle de temps du GTFS
//! \brief De plus, on enlève les stations qui n'ont pas d'arrets dans l'intervalle de temps du GTFS
//! \param[in] p_nomFichier: le nom du fichier contenant les arrets
//! \post assigne m_tousLesArretsPresents à true
//! \throws logic_error si un problème survient avec la lecture du fichier
void DonneesGTFS::ajouterArretsDesVoyagesDeLaDate(const std::string &p_nomFichier)
{
    try {
        ifstream file(p_nomFichier);

        if (file.good()) {
            string line;
            getline(file, line);

            vector<string> headers = string_to_vector(line, ',');
            std::unordered_map<string, unsigned int> headers_map = {
                    {"trip_id",        0},
                    {"arrival_time",   1},
                    {"departure_time", 2},
                    {"stop_id",        3},
                    {"stop_sequence",  4}
            };

            for (line; getline(file, line);) {
                vector<string> champs = string_to_vector(line, ',');

                line.erase(std::remove(line.begin(), line.end(), '"'), line.end());
                line.erase(std::remove(line.begin(), line.end(), '\r'), line.end());

                string trip_id = champs[headers_map.at("trip_id")];

                if (m_voyages.count(trip_id)) {
                    string arrival_time = champs[headers_map.at("arrival_time")];
                    string departure_time = champs[headers_map.at("departure_time")];

                    vector<unsigned int> arrival_tokens;
                    vector<unsigned int> departure_tokens;

                    string buffer;

                    // On récupère les différentes partie de l'heure d'arrivée
                    stringstream atss(arrival_time);
                    while (std::getline(atss, buffer, ':')) {
                        arrival_tokens.push_back((unsigned) stoi(buffer));
                    }

                    // On récupère les différentes partie de l'heure de départ
                    stringstream dtss(departure_time);
                    while (std::getline(dtss, buffer, ':')) {
                        departure_tokens.push_back((unsigned) stoi(buffer));
                    }

                    Heure *arrival_hour = new Heure(arrival_tokens[0], arrival_tokens[1], arrival_tokens[2]);
                    Heure *departure_hour = new Heure(departure_tokens[0], departure_tokens[1], departure_tokens[2]);

                    if (*departure_hour >= m_now1 && *arrival_hour < m_now2) {
                        Arret::Ptr a_ptr = make_shared<Arret>(
                                (unsigned) stoi(champs[headers_map.at("stop_id")]),
                                *arrival_hour,
                                *departure_hour,
                                (unsigned) stoi(champs[headers_map.at("stop_sequence")]),
                                trip_id
                        );

                        m_nbArrets++;
                        m_voyages[trip_id].ajouterArret(a_ptr);
                    }
                }
            }

            auto it = m_voyages.begin();

            // On enlève les voyages n'ayant aucun arrêt
            while (it != m_voyages.end()) {
                if (it->second.getNbArrets() == 0) {
                    it = m_voyages.erase(it);
                } else {
                    ++it;
                }
            }

            it = m_voyages.begin();

            // Pour chaque voyage, on ajoute les arrêts aux arrêts de la station concernée
            while (it != m_voyages.end()) {
                for (const auto &f : it->second.getArrets()) {
                    m_stations[f->getStationId()].addArret(f);
                }
                it++;
            }

            auto it2 = m_stations.begin();

            // On enlève les stations n'ayant aucun arrêt
            while (it2 != m_stations.end()) {
                if (it2->second.getNbArrets() == 0) {
                    it2 = m_stations.erase(it2);
                } else {
                    ++it2;
                }
            }

            m_tousLesArretsPresents = true;
        }
    } catch(const ifstream::failure& e) {
        throw logic_error("Une erreur est survenue lors de la lecture du fichier.");
    }
}

unsigned int DonneesGTFS::getNbArrets() const
{
    return m_nbArrets;
}

size_t DonneesGTFS::getNbLignes() const
{
    return m_lignes.size();
}

size_t DonneesGTFS::getNbStations() const
{
    return m_stations.size();
}

size_t DonneesGTFS::getNbTransferts() const
{
    return m_transferts.size();
}

size_t DonneesGTFS::getNbServices() const
{
    return m_services.size();
}

size_t DonneesGTFS::getNbVoyages() const
{
    return m_voyages.size();
}

void DonneesGTFS::afficherLignes() const
{
    std::cout << "======================" << std::endl;
    std::cout << "   LIGNES GTFS   " << std::endl;
    std::cout << "   COMPTE = " << m_lignes.size() << "   " << std::endl;
    std::cout << "======================" << std::endl;
    for (const auto & ligneM : m_lignes_par_numero)
    {
        cout << ligneM.second;
    }
    std::cout << std::endl;
}

void DonneesGTFS::afficherStations() const
{
    std::cout << "========================" << std::endl;
    std::cout << "   STATIONS GTFS   " << std::endl;
    std::cout << "   COMPTE = " << m_stations.size() << "   " << std::endl;
    std::cout << "========================" << std::endl;
    for (const auto & stationM : m_stations)
    {
        std::cout << stationM.second << endl;
    }
    std::cout << std::endl;
}

void DonneesGTFS::afficherTransferts() const
{
    std::cout << "========================" << std::endl;
    std::cout << "   TRANSFERTS GTFS   " << std::endl;
    std::cout << "   COMPTE = " << m_transferts.size() << "   " << std::endl;
    std::cout << "========================" << std::endl;
    for (unsigned int i = 0; i < m_transferts.size(); ++i)
    {
        std::cout << "De la station " << get<0>(m_transferts.at(i)) << " vers la station " << get<1>(m_transferts.at(i))
        <<
        " en " << get<2>(m_transferts.at(i)) << " secondes" << endl;
        
    }
    std::cout << std::endl;
    
}


void DonneesGTFS::afficherArretsParVoyages() const
{
    std::cout << "=====================================" << std::endl;
    std::cout << "   VOYAGES DE LA JOURNÉE DU " << m_date << std::endl;
    std::cout << "   " << m_now1 << " - " << m_now2 << std::endl;
    std::cout << "   COMPTE = " << m_voyages.size() << "   " << std::endl;
    std::cout << "=====================================" << std::endl;
    
    for (const auto & voyageM : m_voyages)
    {
        unsigned int ligne_id = voyageM.second.getLigne();
        auto l_itr = m_lignes.find(ligne_id);
        cout << (l_itr->second).getNumero() << " ";
        cout << voyageM.second << endl;
        for (const auto & a: voyageM.second.getArrets())
        {
            unsigned int station_id = a->getStationId();
            auto s_itr = m_stations.find(station_id);
            std::cout << a->getHeureArrivee() << " station " << s_itr->second << endl;
        }
    }
    
    std::cout << std::endl;
}

void DonneesGTFS::afficherArretsParStations() const
{
    std::cout << "========================" << std::endl;
    std::cout << "   ARRETS PAR STATIONS   " << std::endl;
    std::cout << "   Nombre d'arrêts = " << m_nbArrets << std::endl;
    std::cout << "========================" << std::endl;
    for ( const auto & stationM : m_stations)
    {
        std::cout << "Station " << stationM.second << endl;
        for ( const auto & arretM : stationM.second.getArrets())
        {
            string voyage_id = arretM.second->getVoyageId();
            auto v_itr = m_voyages.find(voyage_id);
            unsigned int ligne_id = (v_itr->second).getLigne();
            auto l_itr = m_lignes.find(ligne_id);
            std::cout << arretM.first << " - " << (l_itr->second).getNumero() << " " << v_itr->second << std::endl;
        }
    }
    std::cout << std::endl;
}

const std::map<std::string, Voyage> &DonneesGTFS::getVoyages() const
{
    return m_voyages;
}

const std::map<unsigned int, Station> &DonneesGTFS::getStations() const
{
    return m_stations;
}

const std::vector<std::tuple<unsigned int, unsigned int, unsigned int> > &DonneesGTFS::getTransferts() const
{
    return m_transferts;
}

Heure DonneesGTFS::getTempsFin() const
{
    return m_now2;
}

Heure DonneesGTFS::getTempsDebut() const
{
    return m_now1;
}

const std::unordered_map<unsigned int, Ligne> &DonneesGTFS::getLignes() const
{
    return m_lignes;
}



