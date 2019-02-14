/* ***********************************************************************
 * Andama
 * (C) 2017 by Yiannis Bourkelis (hello@andama.org)
 *
 * This file is part of Andama.
 *
 * Andama is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Andama is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Andama.  If not, see <http://www.gnu.org/licenses/>.
 * ***********************************************************************/

#ifndef CLIENTSERVERPROTOCOL_H
#define CLIENTSERVERPROTOCOL_H

#include "PasswordProtection.h"
#include "helperfuncs.h"
//#include "protocolsupervisor.h"


#include "../Shared/AndamaHeaders/shared_enums.h"
#include "../Shared/AndamaHeaders/socket_functions.h"
#include "../Shared/AndamaHeaders/shared_constants.h"
#include "../Shared/Cryptography/openssl_aes.h"

#ifdef WIN32
//#define NOMINMAX
#define in_addr_t uint32_t
#pragma comment(lib, "Ws2_32.lib")
#else
#include <arpa/inet.h>
#endif

#include <vector>
#include <mutex>
#include <map>
#include <string>
#include <random>
#include <QObject>
#include <QDebug>

using namespace helperfuncs;


class clientServerProtocol : public QObject
{
    Q_OBJECT

public:
    //constructors
    clientServerProtocol();

    //public variables
#ifdef WIN32
    SOCKET activeSocket;
#else
    int activeSocket;
#endif

    //public methods
    void proccessMessage(const std::array<char, 1> &command);

    //public enums
    enum msgType{ MSG_ID = 0,
                  MSG_CONNECTION_ACCEPTED = 1, //to remote pc me enimerwse oti to aitima syndesis egine apodekto
                  MSG_REMOTE_CLIENT_ACCEPTED = 2, //apodextika to aitima syndesis pou ekane to remote pc.ksekinw na stelnw screenshots
                  MSG_SCREENSHOT = 3,
                  MSG_SCREENSHOT_DIFF = 4,
                  MSG_SCREENSHOT_REQUEST = 5,
                  MSG_SCREENSHOT_DIFF_REQUEST = 6,
                  MSG_MOUSE = 7,
                  MSG_MOUSE_CURSOR = 8,
                  MSG_KEYBOARD = 9,
                  MSG_CONNECT_ID_NOT_FOUND = 10, // epistrefetai apo ton server otan to Remote ID pou zitithike apo to connect den vrethike
                  MSG_NO_INTERNET_CONNECTION = 11,
                  MSG_NO_PROXY_CONNECTION = 12,
                  MSG_REMOTE_COMPUTER_DISCONNECTED = 13,
                  MSG_LOCAL_PASSWORD_GENERATED = 14,
                  MSG_BAN_IP_WRONG_ID = 15, // synolika 5 bytes
                  MSG_WARNING_BAN_IP_WRONG_ID = 16, // synolika 2 bytes
                  MSG_CONNECT_PASSWORD_NOT_CORRECT = 17,
                  MSG_BAN_IP_WRONG_PWD = 18, // synolika 1 byte
                  MSG_WARNING_BAN_IP_WRONG_PWD = 19, // synolika 2 byte
                  MSG_ERROR_CANNOT_CONNECT_SAME_ID = 20, // synolika 2 byte
                  MSG_ERROR_APP_VERSION_NOT_ACCEPTED = 21, // (mono command)
                  MSG_ERROR_PROTOCOL_VERSION_NOT_ACCEPTED = 22, // (mono command)
                  MSG_P2P_CONNECT_TO_REMOTE_CLIENT_UPNP_PORT = 23, //O server enimerwse oti gia tin syndesi pou epixeirithike, o client exei dilwmeni p2p upnp porta, kai stelnei tis plirofories
                  MSG_P2P_SERVER_BIND_PORT_OK = 24 //otan o p2p server kanei bind tin port, stelnei to message afto wste na ksekinisei to upnpengine
                };


    //public variables
    std::vector<char> myID;

    //public static variables

    //password brute force attack protection
    static const int  MAX_WRONG_PWD_TRIES         = 10;  //10; //default: diplasio apo to MAX_WRONG_ID_TRIES_WARNING
    static const int  MAX_WRONG_PWD_TRIES_WARNING = 5;  //5;
    const unsigned int PASSWORD_LENGTH = 4; // default 6



    //public methods
    void setRemoteComputerOS(OS os);


#ifdef WIN32
    static void cleanup(const SOCKET socketfd);
    static bool isIPBannedForWrongPasswords(in_addr_t clientIP, SOCKET socketfd);
    static bool addWrongPasswordIPProtection(in_addr_t clientIP, SOCKET socketfd);
#else
    static void cleanup(const int socketfd);
    static bool isIPBannedForWrongPasswords(in_addr_t clientIP, int socketfd);
    static bool addWrongPasswordIPProtection(in_addr_t clientIP, int socketfd);
#endif

    void sendHeartBeat();
    OS getRemoteComputerOS();


    void sendKeyboard(int portableVKey, int portableModifiers, int keyEvent);
    void sendMouse(int x, int y, int button, int mouseEvent, int wheelDelta, int wheelDeltaSign, int wheelOrientation);
    void sendMouseCursorType(Qt::CursorShape cursorShape);
    void sendUPnPPort(int port); //stelnei ston server tin porta poy o client akouei gia eiserxomenes P2P syndeseis
    void setConnectionState(connectionState state);
    connectionState getConnectionState();
    void sendDisconnectFromRemoteComputer();
    void Connect(const std::vector<char> &remoteID, const std::vector<char> &remotePassword);
    void RequestScreenshot();
    void RequestScreenshotDiff();
    static void resetWrongPasswordIPProtection(in_addr_t clientIP);

    void setLocalPassword(std::string plain_password);
    const std::string &getLocalPlainPassword() const;
    const std::vector<openssl_aes::byte> &getLocalPasswordHash() const;
    const std::string &getLocalPasswordDoubleHash() const;
    const std::string &getRemotePasswordDoubleHash() const;
    const std::vector<openssl_aes::byte> &getRemotePasswordHash() const;
    const std::string &getRemotePlainPassword() const;
    void setRemotePassword(std::string plain_password);
    const std::vector<openssl_aes::byte> &getRemotePasswordDoubleMD5() const;
    const std::vector<openssl_aes::byte> &getLocalPasswordDoubleMD5() const;
signals:
    void sig_messageReceived(const clientServerProtocol *clientserver_protocol, const int msgType, const std::vector<char> &vdata = std::vector<char>());
    void sig_exception(QString ex);

private:
    //private variables
    OS _remoteComputerOS;
    connectionState m_connection_state = disconnected;
    std::mutex connection_state_mutex;
    //protocolSupervisor _protocolSupervisor;
    std::vector<char> cachedID; //apothikevei to cachedID pou stalthike apo ton server
    std::string m_localPlainPassword;
    std::vector<openssl_aes::byte> m_localPasswordHash256;
    std::string m_localPasswordDoubleHash256;
    std::string m_remotePlainPassword;
    std::vector<openssl_aes::byte> m_remotePasswordHash256;
    std::string m_remotePasswordDoubleHash256;
    std::vector<openssl_aes::byte> m_remotePasswordDoubleMD5;
    std::vector<openssl_aes::byte> m_localPasswordDoubleMD5;

    //random password generator
    //random ID generator
    std::string passwordCharset = "123456789123456789abcdefgkmnprstxz123456789"; //52 chars. Thelw na emfanizontai oi arithmpoi me megalyteri syxnotita. Exw aferaisei grammata poy moiazoun me arithoums p.x. o. Exw aferesei grammata poy mporei na mperdepsoun to xristi
    std::random_device rd;
    std::mt19937 mt = std::mt19937(rd());
    std::uniform_int_distribution<unsigned int> distribution = std::uniform_int_distribution<unsigned int>(0, passwordCharset.length() - 1);


    //private methods
    void createConnectCommandData(std::vector<char> &all_data, const std::vector<char> &remoteComputerID, const std::vector<char> &remoteComputerPassword);
    void ConnectP2P(const std::vector<char> remotePassword);
    void createConnectP2PCommandData(std::vector<char> &all_data, const std::vector<char> remoteComputerPassword);
    std::string generateRandomPassword(int length);
};

#endif // CLIENTSERVERPROTOCOL_H
