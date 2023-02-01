#include <iostream>
#include <sstream>
#include <locale>
#include <iomanip>
#include <string>

namespace kreuter
{
    struct stateEngine_T{
        private:
            tm einstempelzeit {0, 0, 0, 0, 0, 0, 0, 0, 0}; //Struct mit verschiedenen Zeiten z.B. tm_hour und tm_min
            tm anwesenheit {0, 0, 0, 0, 0, 0, 0, 0, 0}; //struct für die Dauer der Anwesenheit
            tm pausenzeit  {0, 0, 0, 0, 0, 0, 0, 0, 0};
            tm ausstempelzeit {0, 0, 0, 0, 0, 0, 0, 0, 0};
            tm sollzeit{0, 36, 7, 0, 0, 0, 0, 0, 0,};

            using state_T = enum{
                panic_S,
                idle_S,
                kommen_S,
                pause_S,
                auskunft_S,
                feierabend_S,
                exit_S
            };
            state_T nextState_;

            bool parseTime(std::string &str, std::tm &time) {
                std::istringstream stream(str);
                stream >> std::get_time(&time, "%R");
                return !stream.fail();
            }
            bool checkforproblems(struct tm *tmstruct){
                if (tmstruct->tm_min == 0 && tmstruct->tm_hour== 0){
                    return true;
                }
                else{
                    return false;
                }
            }

            bool hasProcessFinished(struct tm *tmstruct){
                // Wenn in der minute oder in der Stunde etwas eingetragen wurde, ist es okay
                if (tmstruct->tm_min != 0 || tmstruct->tm_hour!= 0){
                    return true;
                }
                else{
                    return false;
                }
            }
/*
            bool evaluateTransitionEinstempeln(int Auswahl){
                if(Auswahl == 1){
                    return true;
                }
                else{
                    return false;
                }
            }
*/

            //Die Funktion ist wichtig für die späteren Stati. Beim Pausenstatus und beim Feierabendstatus sollten vorher Buchungen da sein. 
            bool is_tm_hour_minute_empty(const struct tm& t) {
                return t.tm_hour == 0 && t.tm_min == 0;
            }

            tm stempeln(){
                // Erfüllt alle Teile des Stempeljobs - Eingabe und Prüfung auf richtiges Format
                struct tm stempelzeit{0, 0, 0, 0, 0, 0, 0, 0, 0};
                std::string str;
                std::cin >> str;
                parseTime(str, stempelzeit);
                if (parseTime(str, stempelzeit)==false) {
                    std::cout << "Uh, ungültiges Zeitformat...nochmal" << std::endl;
                    stempeln();
                } else {
                    std::cout << "Done!" << std::endl;
                }
                return stempelzeit;
            }

            tm zeitberechnung(struct tm *neue_zeit, struct tm *alte_zeit, struct tm *saldo){
                struct tm zeit{0, 0, 0, 0, 0, 0, 0, 0, 0};
                int dauer = (neue_zeit->tm_hour*60+neue_zeit->tm_min+saldo->tm_hour*60 + saldo->tm_min-alte_zeit->tm_hour*60-alte_zeit->tm_min);
                zeit.tm_min = dauer % 60;
                zeit.tm_hour = ((dauer - dauer % 60)/60);
                return zeit;
            }

            void printe_statistik(struct tm *anwesenheit, struct tm *pausenzeit){
                std::cout << "\t\tAnwesenheit heute:" << std::endl <<
                    "\t\tArbeitszeit: " << anwesenheit->tm_hour <<":" <<anwesenheit->tm_min << std::endl <<
                    "\t\tPausenzeit: " << pausenzeit->tm_hour << ":" <<pausenzeit->tm_min << std::endl;
            }

            state_T panic(){
                std::cout << "Alarm Alarm" << std::endl;
                return exit_S;
            }

            state_T runIdle(){
                while(true){
                    int Auswahl;
                    std::cout << "wat willse machen?" << std::endl <<
                    "(1) Einstempeln" << std::endl <<
                    "(2) Pause stempeln" << std::endl << 
                    "(3) Arbeitszeitabfrage" << std::endl <<
                    "(4) Feierabend machen" << std::endl <<
                    "(5) Programm verlassen" << std::endl;
                    std::cout << "Sollzeit: " << sollzeit.tm_hour << ":" << sollzeit.tm_min << std::endl;
                    std::cin >> Auswahl;

                    if (Auswahl ==1) return kommen_S;
                    if (Auswahl ==2) return pause_S;
                    if (Auswahl ==3) return auskunft_S;
                    if (Auswahl ==4) return feierabend_S;
                    if (Auswahl ==5) return exit_S;
                }
            }

            state_T runKommen(){
                std::string str;
                if (is_tm_hour_minute_empty(anwesenheit)==true){
                    //Morgens noch nicht dagewesen
                    std::cout << "Moin, die Pflicht ruft - willste einstempeln? Gib hh:mm ein" << std::endl;
                    einstempelzeit = stempeln();
                }
                else {
                    std::cout << "Welcome back! Gib hh:mm ein, um dich wieder einzustempeln." << std::endl;
                    einstempelzeit = stempeln();
                    pausenzeit = zeitberechnung(&einstempelzeit, &ausstempelzeit, &pausenzeit);
                    printe_statistik(&anwesenheit, &pausenzeit);
                }
                return idle_S;

            }

            state_T runPause(){
                std::string str;
                //Zur Pause wird das erste Mal die Arbeitszeit evaluiert. Die geleistete Arbeitszeit ist Ausstempelzeit - Einstempelzeit
                if (is_tm_hour_minute_empty(pausenzeit)==true){
                    std::cout << "Wird Zeit für ein Päuschen, gib die Zeit in hh:mm ein." << std::endl;
                }
                else {
                    std::cout << "Man man man, ganz schön viele Pausen... gib hh:mm ein." << std::endl;
                }
                ausstempelzeit = stempeln();
                anwesenheit = zeitberechnung(&ausstempelzeit, &einstempelzeit, &anwesenheit);
                printe_statistik(&anwesenheit, &pausenzeit);
                return idle_S;
            } 

            state_T runAuskunft(){
                /*
                Zur Arbeitszeitabfrage wird zunächst geschaut, ob die Aus- oder Einstempelzeit größer ist. 
                Je nachdem muss die Pausenzeit bis jetzt hochgerechnet werden oder die Anwesenheitszeit. 
                */
                struct tm arbeitszeitsaldo{0, 0, 0, 0, 0, 0, 0, 0, 0};
                time_t currentTime = time(nullptr);
                std::cout << "Es ist " << ctime(&currentTime);
                tm* localTime = localtime(&currentTime);
                if ((ausstempelzeit.tm_hour*60+ausstempelzeit.tm_min) >= (einstempelzeit.tm_hour*60+ausstempelzeit.tm_min)){
                    //In dem Szenario ist jetzt das Ausstempeln aktueller als das Einstempeln
                    tm aktuell = zeitberechnung(localTime, &ausstempelzeit, &pausenzeit);
                    tm restzeit = zeitberechnung(&sollzeit, &anwesenheit, localTime);
                    printe_statistik(&anwesenheit, &aktuell);
                    std::cout << "\t\tFeierabend (aprrox): " << restzeit.tm_hour << ":" << restzeit.tm_min << std::endl;
                      
                }
                else{
                    tm aktuell = zeitberechnung(localTime, &einstempelzeit, &anwesenheit);
                    tm restzeit = zeitberechnung(&sollzeit, &anwesenheit, localTime);
                    printe_statistik(&aktuell, &pausenzeit);
                    std::cout << "\t\tFeierabend (aprrox): " << restzeit.tm_hour << ":" << restzeit.tm_min << std::endl;

                }
                return idle_S;
            }   

            state_T runFeierabend(){
                std::string str;
                //Die geleistete Arbeitszeit ist Ausstempelzeit - Einstempelzeit
                std::cout << "Feierabend ist um hh:mm" << std::endl;
                ausstempelzeit = stempeln();
                anwesenheit = zeitberechnung(&ausstempelzeit, &einstempelzeit, &anwesenheit);
                printe_statistik(&anwesenheit, &pausenzeit);
                return exit_S;
            } 

        public:
            int exec(){
                while(nextState_ != exit_S){
                    switch(nextState_){
                        default:
                            case panic_S:
                                nextState_ = panic();
                                break;
                            case idle_S:
                                nextState_ = runIdle();
                                break;
                            case kommen_S:
                                nextState_ = runKommen();
                                break;
                            case pause_S:
                                nextState_ = runPause();
                                break;
                            case auskunft_S:
                                nextState_ = runAuskunft();
                                break;    
                            case feierabend_S:
                                nextState_ = runFeierabend();
                                break;                            
                    }
                }
                return 1;
            }
            stateEngine_T(int argc, char**argv){
                nextState_ = idle_S;
            }
        
    };
}

int main(int argc, char** argv){
    kreuter::stateEngine_T states{argc, argv};
    return states.exec();
}

