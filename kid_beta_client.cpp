#include<iostream>
#include<sstream>
#include<string>
#include<vector>
#include<unistd.h>
#include<fcntl.h>
#include<cstdlib>
#include<map>
#include<cstring>
#include<iomanip>
#include<algorithm>
#include<tuple>

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

//Global attributes
string playerNum; //Controlling player number
vector<player> playerList;
vector<string> regionOrder;
int nextResolveRegion_i = 0;
map<string, region> regionList;
int psCounter = 3;

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
    char buffer[256];
    int pipe;
    pipe = open(path.c_str(), O_RDONLY);
    if(pipe == -1){
        cout << "No such file! " << "\"" << path << "\"" << endl;
        exit(-1);
    }
    read(pipe, buffer, 256);
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
        aRegion.B = stoi(msg[++i]);
        aRegion.R = stoi(msg[++i]);
        aRegion.Y = stoi(msg[++i]);
        //Check init faction
        if(aRegion.name == "MO"){ aRegion.initFaction = "B"; } 
        else if(aRegion.name == "GW"){ aRegion.initFaction = "R"; } 
        else if(aRegion.name == "ES"){ aRegion.initFaction = "Y"; }
        else { aRegion.initFaction = "N";}
        aRegion.faction = "N";
        //Set white disc to false
        aRegion.whiteDisc = false;
        //Emplace region to the map
        regionList[aRegion.name] = aRegion;
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
    regionList[regionA].adjacency.push_back(regionB);
    regionList[regionB].adjacency.push_back(regionA);
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

string blueSupportCard(){
    //07:P1,[S/W/E],ES
    string answer;
    vector<string> options;
    int counter = 0;

    cout << "Using Blue Support card...\n"
        << "Select a region below for supporting maximum two followers\n";
    //Find out options
    for(int i = 0; i < regionOrder.size(); ++i){
        region curRegion = regionList[regionOrder[i]];
        if(curRegion.initFaction == "B" || curRegion.faction == "B"){
            //The faction's adjacency
            for(int j = 0; j < curRegion.adjacency.size(); ++j){
                //Check if a resolved region
                if(regionList[curRegion.adjacency[j]].faction != "N") continue;
                //Check if the adjacency is existing already in options
                if (find(options.begin(), options.end(), curRegion.adjacency[j]) == options.end()){
                    cout << "[" << counter << "] " << curRegion.adjacency[j] << endl;
                    options.push_back(curRegion.adjacency[j]);
                    counter++;
                }
            }
        }
    }
    //While loop
    while(1){
        cin >> answer;
        //Clear cin buffer
        cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        //Evaluate input
        if(answer.length() == 1 && isdigit(answer[0]) && stoi(answer) >= 0 && stoi(answer) < counter)
            break;
        cout << "Sorry, enter a valid number please." << endl;
    }

    return "07:S," + options[stoi(answer)] + "\n";
}

string redSupportCard(){
    //07:P1,[S/W/E],ES
    string answer;
    vector<string> options;
    int counter = 0;

    cout << "Using Red Support card...\n"
        << "Select a region below for supporting maximum two followers\n";
    //Find out options
    for(int i = 0; i < regionOrder.size(); ++i){
        region curRegion = regionList[regionOrder[i]];
        if(curRegion.initFaction == "R" || curRegion.faction == "R"){
            //The faction's adjacency
            for(int j = 0; j < curRegion.adjacency.size(); ++j){
                //Check if a resolved region
                if(regionList[curRegion.adjacency[j]].faction != "N") continue;
                //Check if the adjacency is existing already in options
                if (find(options.begin(), options.end(), curRegion.adjacency[j]) == options.end()){
                    cout << "[" << counter << "] " << curRegion.adjacency[j] << endl;
                    options.push_back(curRegion.adjacency[j]);
                    counter++;
                }
            }
        }
    }
    //While loop
    while(1){
        cin >> answer;
        //Clear cin buffer
        cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        //Evaluate input
        if(answer.length() == 1 && isdigit(answer[0]) && stoi(answer) >= 0 && stoi(answer) < counter)
            break;
        cout << "Sorry, enter a valid number please." << endl;
    }

    return "07:W," + options[stoi(answer)] + "\n";
}

string yellowSupportCard(){
    ///07:P1,[S/W/E],ES
    string answer;
    vector<string> options;
    int counter = 0;

    cout << "Using Yellow Support card...\n"
        << "Select a region below for supporting maximum two followers\n";
    //Find out options
    for(int i = 0; i < regionOrder.size(); ++i){
        region curRegion = regionList[regionOrder[i]];
        if(curRegion.initFaction == "Y" || curRegion.faction == "Y"){
            //The faction's adjacency
            for(int j = 0; j < curRegion.adjacency.size(); ++j){
                //Check if a resolved region
                if(regionList[curRegion.adjacency[j]].faction != "N") continue;
                //Check if the adjacency is existing already in options
                if (find(options.begin(), options.end(), curRegion.adjacency[j]) == options.end()){
                    cout << "[" << counter << "] " << curRegion.adjacency[j] << endl;
                    options.push_back(curRegion.adjacency[j]);
                    counter++;
                }
            }
        }
    }
    //While loop
    while(1){
        cin >> answer;
        //Clear cin buffer
        cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        //Evaluate input
        if(answer.length() == 1 && isdigit(answer[0]) && stoi(answer) >= 0 && stoi(answer) < counter)
            break;
        cout << "Sorry, enter a valid number please." << endl;
    }

    return "07:E," + options[stoi(answer)] + "\n";
}

string assembleCard(){
    //08:ES,B,NO,R,MO,Y
    string input;
    vector<string> answer;
    vector<string> options;
    int counter = 0;

    cout << "Using Assemble card...\n"
        << "Select any 3 regions below for supporting 3 followers of Blue, Red, Yellow in order (separate input by space)\n";
    //Find out options
    for(int i = nextResolveRegion_i; i < regionOrder.size(); ++i){
        cout << "[" << counter << "] " << regionOrder[i] << endl;
        options.push_back(regionOrder[i]);
        counter++;
    }
    //While loop
    while(1){
        getline(cin, input);
        answer = split(input, ' ');
        if(answer.size() == 3 && answer[0].length() == 1 && answer[1].length() == 1 && answer[2].length() == 1 && //Check if right length
            isdigit(answer[0][0]) && isdigit(answer[1][0]) && isdigit(answer[2][0]) && //Check if right type
            stoi(answer[0]) >= 0 && stoi(answer[0]) < counter && //Check if in valid range
            stoi(answer[1]) >= 0 && stoi(answer[1]) < counter && //Check if in valid range
            stoi(answer[2]) >= 0 && stoi(answer[2]) < counter)   //Check if in valid range
            break;
        else if(answer.size() != 3)
            cout << "Sorry, enter 3 valid numbers please." << endl;
        else
            cout << "Sorry, enter valid numbers please." << endl;
    }
    return "08:" + options[stoi(answer[0])] + ",B," + options[stoi(answer[1])] + ",R," + options[stoi(answer[2])] + ",Y\n";
}

string manoeuvreCard(){
    //09:ES,R,NO,Y
    string input;
    vector<string> answer;
    vector<tuple<string, string>> options;
    int counter = 0;

    cout << "Using Manoeuvre card...\n"
        << "Select any 2 different regions below for swapping a follower (separate input by space)\n";
    //Find out options
    for(int i = nextResolveRegion_i; i < regionOrder.size(); ++i){
        region curRegion = regionList[regionOrder[i]];
        string follower;
        //Available blue followers
        if(curRegion.B > 0){
            cout << "[" << counter << "] " << curRegion.name << " B" << " ";
            follower = "B";
            options.push_back(make_tuple(curRegion.name, follower));
            counter++;
        }
        //Available red followers
        if(curRegion.R > 0){
            cout << "[" << counter << "] " << curRegion.name << " R" << " ";
            follower = "R";
            options.push_back(make_tuple(curRegion.name, follower));
            counter++;
        }
        //Available yellow followers
        if(curRegion.Y > 0){
            cout << "[" << counter << "] " << curRegion.name << " Y" << " ";
            follower = "Y";
            options.push_back(make_tuple(curRegion.name, follower));
            counter++;
        }
        cout << "\n";
    }
    //While loop
    while(1){
        getline(cin, input);
        answer = split(input, ' ');
        if(answer.size() == 2 && get<0>(options[stoi(answer[0])]) != get<0>(options[stoi(answer[1])]) &&    //Check if right length and different regions
            isdigit(answer[0][0]) && isdigit(answer[1][0]) &&       //Check if right type
            stoi(answer[0]) >= 0 && stoi(answer[0]) < counter &&    //Check if in valid range
            stoi(answer[1]) >= 0 && stoi(answer[1]) < counter)      //Check if in valid range
            break;
        else if(get<0>(options[stoi(answer[0])]) == get<0>(options[stoi(answer[1])]))
            cout << "Sorry, select different regions please." << endl;
        else
            cout << "Sorry, enter valid numbers please." << endl;
    }
    return "09:" + get<0>(options[stoi(answer[0])]) + "," + get<1>(options[stoi(answer[0])]) + "," + get<0>(options[stoi(answer[1])]) + "," + get<1>(options[stoi(answer[1])]) + "\n";
}

string outmanoeuvreCard(){
    //10:ES,R,Y,NO,B
    string answer, msgTemp;
    vector<string> options;
    int counter = 0;
    bool twoFollowersCase = true;

    cout << "Using Outmanoeuvre card...\n"
        << "First selection\n"
        << "Select a region with any 2 followers for swapping, 1 followers if no 2 available in any case\n";
    //Find out options
    //2 followers case in first selection
    for(int i = nextResolveRegion_i; i < regionOrder.size(); ++i){
        region curRegion = regionList[regionOrder[i]];
        int followers = curRegion.B + curRegion.R + curRegion.Y;
        //Check if any its border region is available for exchanging
        bool isAvailableBorder = false;
        for(int j = 0; j < curRegion.adjacency.size(); ++j){
            region borderRegion = regionList[curRegion.adjacency[j]];
            int borderFollowers = borderRegion.B + borderRegion.R + borderRegion.Y;
            if(borderRegion.faction != "N" && borderFollowers == 0) continue;
            isAvailableBorder = true;
        }
        if(followers >= 2 && isAvailableBorder){
            if(curRegion.B >= 2){
                cout << "[" << counter << "] " << curRegion.name << " " << "BB" << " ";
                options.push_back(curRegion.name + "BB");
                counter++;
            }
            if(curRegion.R >= 2){
                cout << "[" << counter << "] " << curRegion.name << " " << "RR" << " ";
                options.push_back(curRegion.name + "RR");
                counter++;
            }
            if(curRegion.Y >= 2){
                cout << "[" << counter << "] " << curRegion.name << " " << "YY" << " ";
                options.push_back(curRegion.name + "YY");
                counter++;
            }
            if(curRegion.B > 0 && curRegion.R > 0){
                cout << "[" << counter << "] " << curRegion.name << " " << "BR" << " ";
                options.push_back(curRegion.name + "BR");
                counter++;
            }
            if(curRegion.B > 0 && curRegion.Y > 0){
                cout << "[" << counter << "] " << curRegion.name << " " << "BY" << " ";
                options.push_back(curRegion.name + "BY");
                counter++;
            }
            if(curRegion.R > 0 && curRegion.Y > 0){
                cout << "[" << counter << "] " << curRegion.name << " " << "RY" << " ";
                options.push_back(curRegion.name + "RY");
                counter++;
            }
            cout << "\n";
        }
    }
    //1 followers case (no 2 followers case available) in first selection
    if(counter == 0){
        twoFollowersCase = false;
        for(int i = nextResolveRegion_i; i < regionOrder.size(); ++i){
            region curRegion = regionList[regionOrder[i]];
            //Check if any its border region is available for exchanging
            bool isAvailableBorder = false;
            for(int j = 0; j < curRegion.adjacency.size(); ++j){
                region borderRegion = regionList[curRegion.adjacency[j]];
                int borderFollowers = borderRegion.B + borderRegion.R + borderRegion.Y;
                if(borderRegion.faction != "N" && borderFollowers == 0) continue;
                isAvailableBorder = true;
            }
            if(isAvailableBorder){
                if(curRegion.B > 0){
                    cout << "[" << counter << "] " << curRegion.name << " " << "B";
                    options.push_back(curRegion.name + "B");
                    counter++;
                }
                if(curRegion.R > 0){
                    cout << "[" << counter << "] " << curRegion.name << " " << "R";
                    options.push_back(curRegion.name + "R");
                    counter++;
                }
                if(curRegion.Y > 0){
                    cout << "[" << counter << "] " << curRegion.name << " " << "Y";
                    options.push_back(curRegion.name + "Y");
                    counter++;
                }
                cout << "\n";
            }
        }
    }
    //While loop
    while(1){
        cin >> answer;
        //Clear cin buffer
        cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        //Evaluate input
        if(isdigit(answer[0]) && stoi(answer) >= 0 && stoi(answer) < counter)
            break;
        cout << "Sorry, enter a valid number please." << endl;
    }
    if(twoFollowersCase)
        msgTemp = "10:" + options[stoi(answer)].substr(0, 2) + "," + options[stoi(answer)].substr(2, 1) + "," + options[stoi(answer)].substr(3, 1) + ",";
    else
        msgTemp = "10:" + options[stoi(answer)].substr(0, 2) + "," + options[stoi(answer)].substr(2, 1) + ",";
    
    cout << "Chose region and followers: " <<  options[stoi(answer)].substr(0, 2) << " " << options[stoi(answer)].substr(2, options[stoi(answer)].length()-2) << "\n";

    //In second selection
    counter = 0;
    region firstChosenRegion = regionList[options[stoi(answer)].substr(0, 2)];
    answer = "";
    options.clear();
    cout << "Second selection\n"
        << "Select a region with a follower for swapping\n";
    for(int i = 0; i < firstChosenRegion.adjacency.size(); ++i){
        region curRegion = regionList[firstChosenRegion.adjacency[i]];
        //Check if a resolved region
        if(curRegion.faction != "N") continue;
        if(curRegion.B > 0){
            cout << "[" << counter << "] " << curRegion.name << " " << "B" << " ";
            options.push_back(curRegion.name + "B");
            counter++;
        }
        if(curRegion.R > 0){
            cout << "[" << counter << "] " << curRegion.name << " " << "R" << " ";
            options.push_back(curRegion.name + "R");
            counter++;
        }
        if(curRegion.Y > 0){
            cout << "[" << counter << "] " << curRegion.name << " " << "Y" << " ";
            options.push_back(curRegion.name + "Y");
            counter++;
        }
        cout << "\n";
    }
    //While loop
    while(1){
        cin >> answer;
        //Clear cin buffer
        cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        //Evaluate input
        if(isdigit(answer[0]) && stoi(answer) >= 0 && stoi(answer) < counter)
            break;
        cout << "Sorry, enter a valid number please." << endl;
    }
    return msgTemp + options[stoi(answer)].substr(0, 2) + "," + options[stoi(answer)].substr(2, 1) + "\n";
}

string negotiateCard(){
    //11:ES,WA
    string input;
    vector<string> answer;
    vector<string> options;
    int counter = 0;

    cout << "Using Negotiate card...\n"
        << "Select any 2 different regions below for swapping their position,\n"
        << "the first selection will be added a white disc (separate input by space)\n";
    //Find out options
    for(int i = nextResolveRegion_i; i < regionOrder.size(); ++i){
        cout << "[" << counter << "] " << regionOrder[i] << endl;
        options.push_back(regionOrder[i]);
        counter++;
    }
    //While loop
    while(1){
        getline(cin, input);
        answer = split(input, ' ');
        if(answer.size() == 2 && options[stoi(answer[0])] != options[stoi(answer[1])] &&    //Check if right length and different regions
            isdigit(answer[0][0]) && isdigit(answer[1][0]) &&       //Check if right type
            stoi(answer[0]) >= 0 && stoi(answer[0]) < counter &&    //Check if in valid range
            stoi(answer[1]) >= 0 && stoi(answer[1]) < counter)      //Check if in valid range
            break;
        cout << "Sorry, enter valid numbers please." << endl;
    }
    return "11:" + options[stoi(answer[0])] + "," + options[stoi(answer[1])] + "\n";
}

string summonAFollower(){
    //12:ES,B
    string answer;
    vector<string> options;
    int counter = 0;

    cout << "Summoning a follower...\n"
        << "Select a region's follower and place it to your court\n";
    //Find out options
    for(int i = nextResolveRegion_i; i < regionOrder.size(); ++i){
        region curRegion = regionList[regionOrder[i]];
        //Available blue followers
        if(curRegion.B > 0){
            cout << "[" << counter << "] " << curRegion.name << " B" << " ";
            options.push_back(curRegion.name + "B");
            counter++;
        }
        //Available red followers
        if(curRegion.R > 0){
            cout << "[" << counter << "] " << curRegion.name << " R" << " ";
            options.push_back(curRegion.name + "R");
            counter++;
        }
        //Available yellow followers
        if(curRegion.Y > 0){
            cout << "[" << counter << "] " << curRegion.name << " Y" << " ";
            options.push_back(curRegion.name + "Y");
            counter++;
        }
        cout << "\n";
    }
    //While loop
    while(1){
        cin >> answer;
        //Clear cin buffer
        cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        //Evaluate input
        if(isdigit(answer[0]) && stoi(answer) >= 0 && stoi(answer) < counter)
            break;
        cout << "Sorry, enter a valid number please." << endl;
    }
    return "12:" + options[stoi(answer)].substr(0, 2) + "," + options[stoi(answer)].substr(2, 1) + "\n";
}

void doAssemble(vector<string> msg){
    //Subtract the card usage
    playerList[stoi(msg[0].substr(1, 1))-1].cards["A"]--;
    //Add followers to regions
    regionList[msg[2]].B++;
    regionList[msg[2*2]].R++;
    regionList[msg[2*3]].Y++;
    //Take off the appointed follower from region add it to player
    if(msg[9] == "B"){
        regionList[msg[8]].B--;
        playerList[stoi(msg[0].substr(1, 1))-1].B++;
    }else if(msg[9] == "R"){
        regionList[msg[8]].R--;
        playerList[stoi(msg[0].substr(1, 1))-1].R++;
    }else if(msg[9] == "Y"){
        regionList[msg[8]].Y--;
        playerList[stoi(msg[0].substr(1, 1))-1].Y++;
    }
}

void doBlueSupport(vector<string> msg){
    //Subtract the card usage
    playerList[stoi(msg[0].substr(1, 1))-1].cards["B"]--;
    //Add followers to region
    regionList[msg[3]].B += 2;
    //Take off the appointed follower from region add it to player
    if(msg[5] == "B"){
        regionList[msg[4]].B--;
        playerList[stoi(msg[0].substr(1, 1))-1].B++;
    }else if(msg[5] == "R"){
        regionList[msg[4]].R--;
        playerList[stoi(msg[0].substr(1, 1))-1].R++;
    }else if(msg[5] == "Y"){
        regionList[msg[4]].Y--;
        playerList[stoi(msg[0].substr(1, 1))-1].Y++;
    }
}

void doRedSupport(vector<string> msg){
    //Subtract the card usage
    playerList[stoi(msg[0].substr(1, 1))-1].cards["R"]--;
    //Add followers to region
    regionList[msg[3]].R += 2;
    //Take off the appointed follower from region add it to player
    if(msg[5] == "B"){
        regionList[msg[4]].B--;
        playerList[stoi(msg[0].substr(1, 1))-1].B++;
    }else if(msg[5] == "R"){
        regionList[msg[4]].R--;
        playerList[stoi(msg[0].substr(1, 1))-1].R++;
    }else if(msg[5] == "Y"){
        regionList[msg[4]].Y--;
        playerList[stoi(msg[0].substr(1, 1))-1].Y++;
    }
}

void doYellowSupport(vector<string> msg){
    //Subtract the card usage
    playerList[stoi(msg[0].substr(1, 1))-1].cards["Y"]--;
    //Add followers to region
    regionList[msg[3]].Y += 2;
    //Take off the appointed follower from region add it to player
    if(msg[5] == "B"){
        regionList[msg[4]].B--;
        playerList[stoi(msg[0].substr(1, 1))-1].B++;
    }else if(msg[5] == "R"){
        regionList[msg[4]].R--;
        playerList[stoi(msg[0].substr(1, 1))-1].R++;
    }else if(msg[5] == "Y"){
        regionList[msg[4]].Y--;
        playerList[stoi(msg[0].substr(1, 1))-1].Y++;
    }
}

void doManoeuvre(vector<string> msg){
    //Subtract the card usage
    playerList[stoi(msg[0].substr(1, 1))-1].cards["M"]--;
    //Swap first follower
    if(msg[3] == "B"){
        regionList[msg[2]].B--;
        regionList[msg[4]].B++;
    }else if(msg[3] == "R"){
        regionList[msg[2]].R--;
        regionList[msg[4]].R++;
    }else if(msg[3] == "Y"){
        regionList[msg[2]].Y--;
        regionList[msg[4]].Y++;
    }
    //Swap second follower
    if(msg[5] == "B"){
        regionList[msg[2]].B++;
        regionList[msg[4]].B--;
    }else if(msg[5] == "R"){
        regionList[msg[2]].R++;
        regionList[msg[4]].R--;
    }else if(msg[5] == "Y"){
        regionList[msg[2]].Y++;
        regionList[msg[4]].Y--;
    }
    //Take off the appointed follower from region add it to player
    if(msg[7] == "B"){
        regionList[msg[6]].B--;
        playerList[stoi(msg[0].substr(1, 1))-1].B++;
    }else if(msg[7] == "R"){
        regionList[msg[6]].R--;
        playerList[stoi(msg[0].substr(1, 1))-1].R++;
    }else if(msg[7] == "Y"){
        regionList[msg[6]].Y--;
        playerList[stoi(msg[0].substr(1, 1))-1].Y++;
    }
}

void doOutmanoeuvre(vector<string> msg){
    //Subtract the card usage
    playerList[stoi(msg[0].substr(1, 1))-1].cards["O"]--;
    //Swap first follower
    if(msg[3] == "B"){
        regionList[msg[2]].B--;
        regionList[msg[5]].B++;
    }else if(msg[3] == "R"){
        regionList[msg[2]].R--;
        regionList[msg[5]].R++;
    }else if(msg[3] == "Y"){
        regionList[msg[2]].Y--;
        regionList[msg[5]].Y++;
    }
    //Swap second follower
    if(msg[4] == "B"){
        regionList[msg[2]].B--;
        regionList[msg[5]].B++;
    }else if(msg[4] == "R"){
        regionList[msg[2]].R--;
        regionList[msg[5]].R++;
    }else if(msg[4] == "Y"){
        regionList[msg[2]].Y--;
        regionList[msg[5]].Y++;
    }
    //Swap third follower
    if(msg[6] == "B"){
        regionList[msg[2]].B++;
        regionList[msg[5]].B--;
    }else if(msg[6] == "R"){
        regionList[msg[2]].R++;
        regionList[msg[5]].R--;
    }else if(msg[6] == "Y"){
        regionList[msg[2]].Y++;
        regionList[msg[5]].Y--;
    }
    //Take off the appointed follower from region add it to player
    if(msg[8] == "B"){
        regionList[msg[7]].B--;
        playerList[stoi(msg[0].substr(1, 1))-1].B++;
    }else if(msg[8] == "R"){
        regionList[msg[7]].R--;
        playerList[stoi(msg[0].substr(1, 1))-1].R++;
    }else if(msg[8] == "Y"){
        regionList[msg[7]].Y--;
        playerList[stoi(msg[0].substr(1, 1))-1].Y++;
    }
}

void doNegotiate(vector<string> msg){
    int a_i, b_i;
    string temp;

    //Subtract the card usage
    playerList[stoi(msg[0].substr(1, 1))-1].cards["N"]--;
    //Added white disc
    regionList[msg[2]].whiteDisc = true;
    //Locate two regions index
    for(int i = nextResolveRegion_i; i < regionOrder.size(); ++i){
        if(regionOrder[i] == msg[2]) a_i = i;
        if(regionOrder[i] == msg[3]) b_i = i;
    }
    //Swap two regions order
    temp = regionOrder[a_i];
    regionOrder[a_i] = regionOrder[b_i];
    regionOrder[b_i] = temp;
    //Take off the appointed follower from region add it to player
    if(msg[5] == "B"){
        regionList[msg[4]].B--;
        playerList[stoi(msg[0].substr(1, 1))-1].B++;
    }else if(msg[5] == "R"){
        regionList[msg[4]].R--;
        playerList[stoi(msg[0].substr(1, 1))-1].R++;
    }else if(msg[5] == "Y"){
        regionList[msg[4]].Y--;
        playerList[stoi(msg[0].substr(1, 1))-1].Y++;
    }
}

void actionInfo(vector<string> msg){
    //13:P1;[A,ES,B,NO,R,MO,Y];[ES.B]
    cout << msg[0] << " ";
    if(msg[1] == "A"){
        cout << "used Assemble\n";
        cout << msg[2] << " added a " << msg[3] << " follower\n";
        cout << msg[4] << " added a " << msg[5] << " follower\n";
        cout << msg[6] << " added a " << msg[7] << " follower\n";
        cout << msg[0] << " took a " << msg[9] << " follower from " << msg[8] << "\n";
        doAssemble(msg);
        psCounter = 3;
    }else if(msg[1] == "S"){
        if(msg[2] == "S"){
            cout << "used Blue Support\n";
            cout << msg[3] << " added 2 blue followers\n";
            cout << msg[0] << " took a " << msg[5] << " follower from " << msg[4] << "\n";
            doBlueSupport(msg);
            psCounter = 3;
        }else if(msg[2] == "W"){
            cout << "used Red Support\n";
            cout << msg[3] << " added 2 red followers\n";
            cout << msg[0] << " took a " << msg[5] << " follower from " << msg[4] << "\n";
            doRedSupport(msg);
            psCounter = 3;
        }else if(msg[2] == "E"){
            cout << "used Yellow Support\n";
            cout << msg[3] << " added 2 yellow followers\n";
            cout << msg[0] << " took a " << msg[5] << " follower from " << msg[4] << "\n";
            doYellowSupport(msg);
            psCounter = 3;
        }
    }else if(msg[1] == "M"){
        cout << "used Manoeuvre\n";
        cout << "swapped " << msg[3] << " from " << msg[2] << " with " << msg[5] << " from " << msg[4] << "\n";
        cout << msg[0] << " took a " << msg[7] << " follower from " << msg[6] << "\n";
        doManoeuvre(msg);
        psCounter = 3;
    }else if(msg[1] == "O"){
        cout << "used Outmanoeuvre\n";
        //2 cases possible, swap 2 with 1
        if(msg.size() == 8){
            cout << "swapped " << msg[3] << " and " << msg[4] << " from " << msg[2] << " with " << msg[6] << " from " << msg[5] << "\n";
            cout << msg[0] << " took a " << msg[8] << " follower from " << msg[7] << "\n";
            doOutmanoeuvre(msg);
        }
        else{ //or swap 1 with 1
            cout << "swapped " << msg[3] << " from " << msg[2] << " with " << msg[5] << " from " << msg[4] << "\n";
            cout << msg[0] << " took a " << msg[7] << " follower from " << msg[6] << "\n";
            doManoeuvre(msg);
        }
        psCounter = 3;
    }else if(msg[1] == "N"){
        cout << "used Negotiate\n";
        cout << "swapped PS order of regions of " << msg[2] << "(added white disc) and " << msg[3] << "\n";
        cout << msg[0] << " took a " << msg[5] << " follower from " << msg[4] << "\n";
        doNegotiate(msg);
        psCounter = 3;
    }else{
        cout << "passed\n";
        psCounter--;
    }
}

void powerStruggle(vector<string> msg){
    //14:ES,[BRYF]
    regionList[msg[0]].faction = msg[1];
    nextResolveRegion_i++;
    psCounter = 3;
    cout << "Region " << msg[0] << " is resolved to " << msg[1] << endl;
}

string playerPass(){
    return "15:Pass\n";
}

void winnerAnnounce(vector<string> msg){
    //16:P2,[C/F]
    cout << "The game winner is " << msg[0] << (msg[1]=="C" ? " by coronation" : " by French invasion") << endl;
}

void displayCurrentTable(){
    cout << "---------------------------Regions-----------------------------\n";
    cout << "Name  " << "OG_F  " << "Faction  " << "Blue  " << "Red  " << "Yellow  " << "W_Disc  " << "Adjacency\n";
    for(int i = 0; i < regionOrder.size(); ++i){
        region curRegion = regionList[regionOrder[i]];
        cout << curRegion.name << setw(5)
            << curRegion.initFaction << setw(6)
            << curRegion.faction << setw(9)
            << curRegion.B << setw(6)
            << curRegion.R << setw(5)
            << curRegion.Y << setw(8)
            << curRegion.whiteDisc << setw(9);
        for(int j = 0; j < curRegion.adjacency.size(); ++j){
            cout << curRegion.adjacency[j] << setw(4);
        }
        cout << "\n\n";
    }
    cout << "---------------------------Players----------------------------\n";
    cout << "No  " << "Blue  " << "Red  " << "Yellow  " << "Cards/Usage\n";
    //Print controlling player info
    player curPlayer = playerList[stoi(playerNum)-1];
    cout << curPlayer.no << setw(3)
        << curPlayer.B << setw(6)
        << curPlayer.R << setw(5)
        << curPlayer.Y << setw(8);
    for(auto it = curPlayer.cards.begin(); it != curPlayer.cards.end(); ++it){
        cout << it->first << "/" << it->second << setw(3);
    }
    cout << "\n\n";
    //Print other players info
    for(int i = 0; i < playerList.size(); ++i){
        if(i == stoi(playerNum)-1) continue;
        curPlayer = playerList[i];
        cout << curPlayer.no << setw(3)
            << curPlayer.B << setw(6)
            << curPlayer.R << setw(5)
            << curPlayer.Y << setw(8);
        for(auto it = curPlayer.cards.begin(); it != curPlayer.cards.end(); ++it){
            cout << it->first << "/" << it->second << setw(3);
        }
        cout << "\n\n";
    }
    cout << "\nPower struggle count down: " << psCounter << "\n\n";
}

string playTurn(vector<string> msg){
    //06:P1
    if(msg[0].substr(1,1) == playerNum){
        player curPlayer = playerList[stoi(playerNum)-1];
        vector<string (*)()> actions;
        int counter = 0;
        string answer;
        
        //Show current table
        displayCurrentTable();
        cout << "Your turn\n"
            << "Please select a card or pass\n";
        //Check card usage and print if it's available
        if(curPlayer.cards["B"] > 0){
            cout << "[" << counter << "] " << "Blue Support\n";
            actions.push_back(blueSupportCard);
            counter++;
        }
        if(curPlayer.cards["R"] > 0){
            cout << "[" << counter << "] " << "Red Support\n";
            actions.push_back(redSupportCard);
            counter++;
        }
        if(curPlayer.cards["Y"] > 0){
            cout << "[" << counter << "] " << "Yellow Support\n";
            actions.push_back(yellowSupportCard);
            counter++;
        }
        if(curPlayer.cards["A"] > 0){
            cout << "[" << counter << "] " << "Assemble\n";
            actions.push_back(assembleCard);
            counter++;
        }
        if(curPlayer.cards["M"] > 0){
            cout << "[" << counter << "] " << "Manoeuvre\n";
            actions.push_back(manoeuvreCard);
            counter++;
        }
        if(curPlayer.cards["O"] > 0){
            cout << "[" << counter << "] " << "Outmanoeuvre\n";
            actions.push_back(outmanoeuvreCard);
            counter++;
        }
        if(curPlayer.cards["N"] > 0){
            cout << "[" << counter << "] " << "Negotiate\n";
            actions.push_back(negotiateCard);
            counter++;
        }
        cout << "[" << counter << "] " << "Pass\n";
        actions.push_back(playerPass);
        //cin loop
        while(1){
            cin >> answer;
            //Clear cin buffer
            cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            //Evaluate input
            if(answer.length() == 1 && isdigit(answer[0]) && stoi(answer) >= 0 && stoi(answer) <= counter)
                break;
            cout << "Sorry, enter a valid number please." << endl;
        }
        //Action execution
        return actions[stoi(answer)]();
    }else{
        cout << "It's " << msg[0] << "'s turn\n";
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
    bool on = true;
    while(on){
        //An input string stream
        istringstream strStream(readFromPipe(listenFrom));
        //Get each line a time
        for(string buffer; getline(strStream, buffer);){
            if(!isdigit(buffer[0]) || buffer.empty())
                break;
            cout << buffer << endl;
            //Get protocol number
            int protocol = stoi(buffer.substr(0, 2));
            //Get msg tokens
            vector<string> msgChunk = split(buffer.substr(3), ';');
            vector<string> msg;
            for(int i = 0; i < msgChunk.size(); i++){
                vector<string> msgTemp = split(msgChunk[i], ',');
                msg.insert(msg.end(), msgTemp.begin(), msgTemp.end());
            }
            //Test
            // cout << protocol << endl;
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
                        cout << saying;
                        writeToPipe(speakTo, saying);
                        //If take an action not pass
                        if(saying.substr(0, 2) != "15"){
                            saying = summonAFollower();
                            cout << saying;
                            writeToPipe(speakTo, saying);
                        }
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
                on = false;
            }
        }
    }
    return 0;
}