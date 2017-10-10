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
    ifstream file(p_nomFichier);

    if(file.good()) {
        string line;
        getline(file, line);

        vector<string> headers = string_to_vector(line, ',');
        std::unordered_map<string, unsigned int> headers_map;

        for(unsigned int i = 0; i < headers.size(); i++) {
            headers_map[headers[i]] = i;
        }

        for(line; getline(file, line);)
        {
            line.erase(std::remove(line.begin(), line.end(), '"'), line.end());
            vector<string> champs = string_to_vector(line, ',');

            Ligne ligne(
                    (unsigned) stoi(champs[headers_map.at("route_id")]),
                    champs[headers_map.at("route_short_name")],
                    champs[headers_map.at("route_desc")],
                    Ligne::couleurToCategorie(champs[headers_map.at("route_color")])
            );

            this->m_lignes.insert({ligne.getId(), ligne});
            this->m_lignes_par_numero.insert({ligne.getNumero(), ligne});
        }
    }
}

//! \brief ajoute les stations dans l'objet GTFS
//! \param[in] p_nomFichier: le nom du fichier contenant les station
//! \throws logic_error si un problème survient avec la lecture du fichier
void DonneesGTFS::ajouterStations(const std::string &p_nomFichier)
{
    ifstream file(p_nomFichier);

    if(file.good()) {
        string line;
        getline(file, line);

        vector<string> headers = string_to_vector(line, ',');
        std::unordered_map<string, unsigned int> headers_map;

        for(unsigned int i = 0; i < headers.size(); i++) {
            headers_map[headers[i]] = i;
        }

        for(line; getline(file, line);)
        {
            line.erase(std::remove(line.begin(), line.end(), '"'), line.end());
            vector<string> champs = string_to_vector(line, ',');

            Coordonnees coord(stod(champs[headers_map.at("stop_lat")]), stod(champs[headers_map.at("stop_lon")]));

            Station station(
                    (unsigned) stoi(champs[headers_map.at("stop_id")]),
                    champs[headers_map.at("stop_name")],
                    champs[headers_map.at("stop_desc")],
                    coord
            );

            this->m_stations.insert({station.getId(), station});
        }
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
}


//! \brief ajoute les services de la date du GTFS (m_date)
//! \param[in] p_nomFichier: le nom du fichier contenant les services
//! \throws logic_error si un problème survient avec la lecture du fichier
void DonneesGTFS::ajouterServices(const std::string &p_nomFichier)
{
    ifstream file(p_nomFichier);

    if(file.good()) {
        string line;
        getline(file, line);

        vector<string> headers = string_to_vector(line, ',');
        std::unordered_map<string, unsigned int> headers_map;

        for(unsigned int i = 0; i < headers.size(); i++) {
            headers_map[headers[i]] = i;
        }

        for(line; getline(file, line);)
        {
            line.erase(std::remove(line.begin(), line.end(), '"'), line.end());
            vector<string> champs = string_to_vector(line, ',');

            string str_date = champs[headers_map.at("date")];
            Date date(
                    (unsigned) stoi(str_date.substr(0, 4)),
                    (unsigned) stoi(str_date.substr(4, 2)),
                    (unsigned) stoi(str_date.substr(6, 2))
            );

            if(stoi(champs[headers_map.at("exception_type")]) == 1 && this->m_date == date) {
                this->m_services.insert({champs[headers_map.at("service_id")]});
            }
        }
    }
}

//! \brief ajoute les voyages de la date
//! \brief seuls les voyages dont le service est présent dans l'objet GTFS sont ajoutés
//! \param[in] p_nomFichier: le nom du fichier contenant les voyages
//! \throws logic_error si un problème survient avec la lecture du fichier
void DonneesGTFS::ajouterVoyagesDeLaDate(const std::string &p_nomFichier)
{
    ifstream file(p_nomFichier);

    if(file.good()) {
        string line;
        getline(file, line);

        vector<string> headers = string_to_vector(line, ',');
        std::unordered_map<string, unsigned int> headers_map;

        for(unsigned int i = 0; i < headers.size(); i++) {
            headers_map[headers[i]] = i;
        }

        for(line; getline(file, line);)
        {
            line.erase(std::remove(line.begin(), line.end(), '"'), line.end());
            vector<string> champs = string_to_vector(line, ',');

            string service_id = champs[headers_map.at("service_id")];

            if(this->m_services.count(service_id)) {
                string trip_id = champs[headers_map.at("trip_id")];

                Voyage voyage = Voyage(
                        trip_id,
                        (unsigned) stoi(champs[headers_map.at("route_id")]),
                        service_id,
                        champs[headers_map.at("trip_headsign")]
                );

                this->m_voyages.insert({trip_id, voyage});
            }
        }
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
    ifstream file(p_nomFichier);

    if(file.good()) {
        string line;
        getline(file, line);

        vector<string> headers = string_to_vector(line, ',');
        std::unordered_map<string, unsigned int> headers_map;

        for(unsigned int i = 0; i < headers.size(); i++) {
            headers_map[headers[i]] = i;
        }

        for(line; getline(file, line);)
        {
            line.erase(std::remove(line.begin(), line.end(), '"'), line.end());
            vector<string> champs = string_to_vector(line, ',');

            string trip_id = champs[headers_map.at("trip_id")];

            if(this->m_voyages.count(trip_id)) {
                string arrival_time = champs[headers_map.at("arrival_time")];
                string departure_time = champs[headers_map.at("departure_time")];

                vector<unsigned int> arrival_tokens;
                vector<unsigned int> departure_tokens;

                string buffer;

                stringstream atss(arrival_time);
                while (std::getline(atss, buffer, ':')) {
                    arrival_tokens.push_back((unsigned) stoi(buffer));
                }

                stringstream dtss(departure_time);
                while (std::getline(dtss, buffer, ':')) {
                    departure_tokens.push_back((unsigned) stoi(buffer));
                }

                Heure *arrival_hour = new Heure(arrival_tokens[0], arrival_tokens[1], arrival_tokens[2]);
                Heure *departure_hour = new Heure(departure_tokens[0], departure_tokens[1], departure_tokens[2]);

                if(*arrival_hour >= this->m_now1 && *departure_hour < this->m_now2) {
                    Arret::Ptr a_ptr = make_shared<Arret>(
                            (unsigned) stoi(champs[headers_map.at("stop_id")]),
                            *arrival_hour,
                            *departure_hour,
                            (unsigned) stoi(champs[headers_map.at("stop_sequence")]),
                            trip_id
                    );

                    this->m_voyages[trip_id].ajouterArret(a_ptr);
                }
            }
        }

        auto it = this->m_voyages.begin();

        while(it != this->m_voyages.end()) {
            if(it->second.getArrets().empty()) {
                it = this->m_voyages.erase(it);
            } else {
                ++it;
            }
        }

        it = this->m_voyages.begin();

        while(it != this->m_voyages.end()) {
            // Pour chaque arrêt dans m_voyages,
            // ajoutez une copie du Arret::Ptr aux arrêts de la station de m_station concernée par cet arrêt
        }
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



