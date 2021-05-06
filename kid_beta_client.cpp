#include<iostream>
#include<sstream>
#include<string>
#include<vector>
#include<unistd.h>
#include<fcntl.h>
#include<cstdlib>
#include<map>

using namespace std;

struct player {
    string no;
    map<string, int> cards;  //8 cards
    int B;                   //Followers
    int R;
    int Y;
};

struct region {
    string name;
    string initFaction;        //Only work for ES, MO, GW
    string faction;            //Result of power struggle
    vector<string> adjacency;  //Adjacent regions
    bool whiteDisc;            //Note of can't be moved order
    int B;                     //Followers
    int R;
    int Y;
};

string playerNum; //Controlling player number
vector<player> playerList;
vector<string> regionOrder;
map<string, region> regionListInPSOrder;

vector<string> split(const string& s , char delimiter){
    vector<string> tokens;          // vector of tokens
    string token;                   // a single token
    istringstream tokenStream(s);   // an input string stream

    // Tokenize s by grabbing individual tokens
    while( getline( tokenStream , token , delimiter ) )
        tokens.push_back(token);    // add found token

    return tokens;
}

string readFromPipe(string path){
    char buffer[128];
    int pipe;
    pipe = open(path.c_str(), O_RDONLY);
    if(pipe == -1){
        cout << "No such file! " << "\"" << path << "\"" << endl;
        exit(-1);
    }
    read(pipe, buffer, 128);
    close(pipe);
    return buffer;
}

void writeToPipe(string path, string msg){
    int pipe;
    pipe = open(path.c_str(), O_WRONLY);
    if(pipe == -1){
        cout << "No such file! " << "\"" << path << "\"" << endl;
        exit(-1);
    }
    write(pipe, msg.c_str(), strlen(msg.c_str()));
    close(pipe);
}

void setMapPopulation(vector<string> msg){
    //02:ES,2,2,0;DE,1,1,2;...
    for(int i = 0; i < msg.size();){
        region aRegion;
        aRegion.name = msg[i];
        //Assign followers
        aRegion.B = stoi(msg[i++]);
        aRegion.R = stoi(msg[i++]);
        aRegion.Y = stoi(msg[i++]);
        //Check init faction
        if(aRegion.name == "MO"){ aRegion.initFaction = "B"; } 
        else if(aRegion.name == "GW"){ aRegion.initFaction = "R"; } 
        else if(aRegion.name == "ES"){ aRegion.initFaction = "Y"; }
        else { aRegion.initFaction = "N"; }
        //Set white disc to false
        aRegion.whiteDisc = false;
        //Emplace region to the map
        regionListInPSOrder[aRegion.name] = aRegion;
        i++;
    }
}

void set2FollowersEach(vector<string> msg){
    //03:P1,1,0,1
    player aPlayer;
    aPlayer.cards["B"] = 1;
    aPlayer.cards["R"] = 1;
    aPlayer.cards["Y"] = 1;
    aPlayer.cards["A"] = 2;
    aPlayer.cards["M"] = 1;
    aPlayer.cards["O"] = 1;
    aPlayer.cards["N"] = 1;
    aPlayer.no = msg[0];
    aPlayer.B = stoi(msg[1]);
    aPlayer.R = stoi(msg[2]);
    aPlayer.Y = stoi(msg[3]);
    playerList.push_back(aPlayer);
}

void setRegionOrder(vector<string> msg){
    //04:MO,NO...ES
    for(int i = 0; i < msg.size(); i++){
        regionOrder.push_back(msg[i]);
    }
}

void addAdjacency(string regionA, string regionB){
    regionListInPSOrder[regionA].adjacency.push_back(regionB);
    regionListInPSOrder[regionB].adjacency.push_back(regionA);
}

void setAdjacency(){
    addAdjacency("ES", "DE");
    addAdjacency("ES", "WA");
    addAdjacency("ES", "NO");
    addAdjacency("GW", "DE");
    addAdjacency("GW", "WA");
    addAdjacency("GW", "LA");
    addAdjacency("DE", "WA");
    addAdjacency("WA", "LA");
    addAdjacency("WA", "NO");
    addAdjacency("LA", "NO");
    addAdjacency("LA", "ST");
    addAdjacency("NO", "ST");
    addAdjacency("ST", "MO");
}

string supportCard(){
    //07:P1,[S/W/E],ES
    return "";
}

string assembleCard(){
    //08:ES,R,NO,Y,MO,B
    return "";
}

string manoeuvreCard(){
    //09:ES,R,NO,Y
    return "";
}

string outmanoeuvreCard(){
    //10:ES,RY,NO,B
    return "";
}

string negotiateCard(){
    //11:ES,WA
    return "";
}

string summonAFollower(){
    //12: B,ES
    return "";
}

void actionInfo(vector<string> msg){
    //13: P1; [A, ES, R, NO, Y, MO, B]; [ES.B]
}

void powerStruggle(vector<string> msg){
    //14:ES,[BRYF]
    regionListInPSOrder[msg[0]].faction = msg[1];
    cout << "Region " << msg[0] << " is resolved to " << msg[1] << endl;
}

string playerPass(){
    return "15:Pass\n";
}

void winnerAnnounce(vector<string> msg){
    //16:P2,[C/F]
    cout << "The game winner is " << msg[0] << (msg[1]=="C" ? " by coronation" : " by French invasion") << endl;
}

string playTurn(vector<string> msg){
    //06:P1
    if(msg[0].substr(1,1) == playerNum){
        cout << "Your turn\n"
        << "Please select a card or pass\n"
        << "[1] Support\n"
        << "[2] Assemble\n"
        << "[3] Manoeuvre\n"
        << "[4] Outmanoeuvre\n"
        << "[5] Negotiate\n"
        << "[6] Pass\n";
        int actionNum;
        //Cin loop
        for (string line; getline(cin, line);){
            string options = "1|2|3|4|5|6";
            //Empty input
            if(line.empty()){ continue; }
            //Evaluation input
            if(options.find(line)!=string::npos && isdigit(line[0])){
                //cout << line << endl;
                actionNum = stoi(line);
                break;
            }
            cout << "Sorry, enter a valid number please." << endl;
        }
        //Action hub
        switch(actionNum){
        case 1:
            return supportCard();
        case 2:
            return assembleCard();
        case 3:
            return manoeuvreCard();
        case 4:
            return outmanoeuvreCard();
        case 5:
            return negotiateCard();
        case 6:
            return playerPass();
        default:
            break;
        }
    } else{
        cout << "It's " << msg[0] << "'s turn";
    }

    return "";
}

int main(int argc, char* arg[]){
    if(argc!=3){
        cerr << "Usage: " << arg[0] << " player_number team_name\n";
        cerr << 
            "  player_number - A number between 1 and 3 inclusive.\n"               // argv[1]
            "  team_name     - A unique string to distinguish server instances.\n";  // argv[2]
        exit(1);
    }
    //Path & Client's number
    string listenFrom = "/tmp/";
    string speakTo = "/tmp/";   
    playerNum = arg[1];
    cout << "Your number is " << playerNum << endl;
    listenFrom = listenFrom + arg[2] + "toP" + arg[1];
    speakTo = speakTo + arg[2] + "fromP" + arg[1];
    //Test
    //cout << playerNum << " " << listenFrom << " " << speakTo << endl;
    
    //Control logic
    while(1){
        string buffer = readFromPipe(listenFrom);
        if(buffer.length() > 0){
            cout << buffer << endl;
            int protocol = stoi(buffer.substr(0, 2));
            //Get msg tokens
            vector<string> msgChunk = split(buffer.substr(3), ';');
            vector<string> msg;
            for(int i = 0; i < msgChunk.size(); i++){
                vector<string> msgTemp = split(msgChunk[i], ',');
                msg.insert(msg.end(), msgTemp.begin(), msgTemp.end());
            }
            //Test
            //cout << protocol << endl;
            // for(int i = 0; i < msg.size(); i++){
            //     cout << msg[i] << " ";
            // }
            // cout << endl;

            //Actions
            switch(protocol){
                case 2:
                    setMapPopulation(msg);
                    break;
                case 3:
                    set2FollowersEach(msg);
                    break;
                case 4:
                    setRegionOrder(msg);
                    setAdjacency();
                    break;
                case 6:{
                    string saying = playTurn(msg);
                    if(saying.length() > 0){
                        writeToPipe(speakTo, saying);
                        // saying = summonAFollower();
                        // writeToPipe(speakTo, saying);
                    }
                    break;
                }
                case 13:
                    actionInfo(msg);
                    break;
                case 14:
                    powerStruggle(msg);
                    break;
                case 16:
                    winnerAnnounce(msg);
                    break;
                default:
                    break;
            }
            if(protocol == 99){
                break;
            }
        }
    }

    return 0;
}