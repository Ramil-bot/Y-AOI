#include <iostream>
#include <rtc/rtc.hpp>

#include "parse_cl.h"

#include <nlohmann/json.hpp>

#include <algorithm>
#include <chrono>
#include <future>
#include <iostream>
#include <memory>
#include <random>
#include <stdexcept>
#include <thread>
#include <unordered_map>

using namespace std::chrono_literals;
using std::shared_ptr;
using std::weak_ptr;
template <class T> weak_ptr<T> make_weak_ptr(shared_ptr<T> ptr) {return ptr;}

using nlohmann::json;

std::string localId;
std::unordered_map<std::string, shared_ptr<rtc::PeerConnection>> peerConnectionMap;
std::unordered_map<std::string, shared_ptr<rtc::DataChannel>> dataChannelMap;

shared_ptr<rtc::PeerConnection> createPeerConnection(const rtc::Configuration &config, weak_ptr<rtc::WebSocket> wws, std::string id);
std::string randomId(size_t length);

int Server(Cmdline &params);
int Client();

std::string base64_encode(const std::string &in);
std::string base64_decode(const std::string &in);

int main(int argc, char **argv) try {
  Cmdline params(argc, argv);
     
  int mode;
  std::cout << "Mode 1 - host, 2 - client: ";
  std::cin >> mode;
  std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
  
  if (mode == 1 ) {
    Server(params);
  } else if (mode == 2) {
    Client();
  }
  std::cout << "Cleaning up..." << std::endl;

  dataChannelMap.clear();
  peerConnectionMap.clear();
  return 0;
} catch (const std::exception &e) {
  std::cout << "Error: " << e.what() << std::endl;
  dataChannelMap.clear();
  peerConnectionMap.clear();
  return -1;
}

int Client() {
  rtc::InitLogger(rtc::LogLevel::Info);
  rtc::Configuration config;
  auto pc = std::make_shared<rtc::PeerConnection>(config);
  std::string compat;
  std::getline(std::cin, compat);
  
  json recv = json::parse(base64_decode(compat));
    
  pc->setRemoteDescription({recv["sdp"].get<std::string>(), "offer"});
  for (const auto &cand : recv["candidate"])
    pc->addRemoteCandidate({cand.get<std::string>()});

  std::string localDescription;
  std::vector<std::string> localCandidates;
  bool gatheringComp = false;
  
  pc->onLocalDescription([&localDescription](rtc::Description desc) {
      localDescription = std::string(desc);
      std::cout << "localDescription: " << localDescription << std::endl;
      });

  pc->onLocalCandidate([&localCandidates](rtc::Candidate cand) {
      localCandidates.push_back(std::string(cand));
      std::cout << localCandidates.back() << std::endl;
      });

  pc->onGatheringStateChange([&](rtc::PeerConnection::GatheringState state) {
      if (state == rtc::PeerConnection::GatheringState::Complete) {
        gatheringComp = true;
      }
      std::cout << "GatheringState: " << state << std::endl;
      }); 


  pc->onStateChange([](rtc::PeerConnection::State state) {
      std::cout << "State: " << state << std::endl;
      });

  pc->onDataChannel([&](std::shared_ptr<rtc::DataChannel> dc) {
      std::cout << "Given datachannel: " << dc->label() << std::endl;
      
      dc->onOpen([dc]() {
          std::cout << "DataChannel is opened (Answerer)" << std::endl;
          dc->send("Hello world");
      });

      dc->onMessage([](auto msg) {
           if (std::holds_alternative<std::string>(msg)) {
             std::cout << "Got message: " << std::get<std::string>(msg) << std::endl;
          } else {
             auto bin = std::get<std::vector<std::byte>>(msg);
             std::cout << "Got binary message: " << bin.size() << std::endl;
          }
      });
  }); 

  pc->setLocalDescription();
  while (!gatheringComp)
    std::this_thread::sleep_for(100ms);
  

  std::cout << "========> Hello hui <========="<<std::endl;
  json token;
  token["sdp"] = localDescription;
  token["candidate"] = localCandidates;
  std::string newcomp = base64_encode(token.dump());
  std::cout << "=== Token ===\n" << newcomp << std::endl;
  std::cin.get();

  while (pc->state() != rtc::PeerConnection::State::Closed &&
          pc->state() != rtc::PeerConnection::State::Failed) {
          std::this_thread::sleep_for(500ms);
  }

  std::cout << "Done. Closing..." << std::endl;
  return 0;
}

int Server(Cmdline &params) {
  rtc::InitLogger(rtc::LogLevel::Info);
  rtc::Configuration config;
  
  std::string localDescription;
  std::vector<std::string> localCandidates;

  std::string stunServer = ""; // << Don't forget edit this pleaaaaaaaaase
  if (params.noStun()) {
    std::cout 
      << "No STUN is configed. Only local hosts and public IP address supported."
      << std::endl;
  } else {
    if (params.stunServer().substr(0, 5).compare("stun:") != 0) {
      stunServer = "stun:";
    }
    stunServer += params.stunServer() + ":" + std::to_string(params.stunPort());
    std::cout << "STUN server is " << stunServer << std::endl;
    config.iceServers.emplace_back(stunServer);
  }

  auto pc = std::make_shared<rtc::PeerConnection>(config);

  pc->onStateChange([](rtc::PeerConnection::State state) {
      std::cout << "State: " << state << std::endl;
      });
  bool gatheringComp = false;
  if (!gatheringComp) {
  pc->onGatheringStateChange([&](rtc::PeerConnection::GatheringState state) {
      if (state == rtc::PeerConnection::GatheringState::Complete) {
      gatheringComp = true;
      }
      std::cout << "GatheringState: " << state << std::endl;
      });}

  pc->onLocalDescription([&localDescription](rtc::Description desc) {
      localDescription = std::string(desc);
      std::cout << "localDescription: " << localDescription << std::endl;
      });


  pc->onLocalCandidate([&localCandidates](rtc::Candidate cand) {
      localCandidates.push_back(std::string(cand));
      std::cout << localCandidates.back() << std::endl;
      });
  
  auto dc = pc->createDataChannel("test");
  dc->onOpen([&]() {
      std::cout << "DataChannel opened, you can send data's" << std::endl;
      });

  dc->onMessage([](auto msg) {
      if (std::holds_alternative<std::string>(msg)) {
          std::cout << "Got message: " << std::get<std::string>(msg) << std::endl;
      } else {
          auto bin = std::get<std::vector<std::byte>>(msg);
          std::cout << "Got binary message: " << bin.size() << std::endl;
      }
  });

  pc->setLocalDescription();

  while (!gatheringComp) {
    std::this_thread::sleep_for(10000ms);
  }

  json token;
  token["sdp"] = localDescription;
  token["candidate"] = localCandidates;
  std::string compat = base64_encode(token.dump());
  std::cout << "=== Token ===\n" << compat << std::endl;

  std::cout << "Put offer to the Answerer\n";
  std::cout << "Put ansewe here ->:\n";
  std::string answerStr;
  std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
  while (answerStr == "") {
    std::cin >> answerStr;
  }
  std::cout << std::endl;

  
  json answerJson = json::parse(base64_decode(answerStr));
  pc->setRemoteDescription({answerJson["sdp"].get<std::string>(), "answer"});

  for (const auto& cand : answerJson["candidate"]) {
    pc->addRemoteCandidate({cand.get<std::string>()});
  }
  
  
  while (pc->state() != rtc::PeerConnection::State::Closed &&
     pc->state() != rtc::PeerConnection::State::Failed) {
     std::this_thread::sleep_for(500ms);
  }
  std::cout << "Done. Closing..." << std::endl;
  return 0;

}

static const char* base64_chars = 
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789+/";

std::string base64_encode(const std::string &in) {
  static const char* chars = base64_chars;
  std::string out;
  int val = 0, valb=-6;
  for(uint8_t c: in) {
    val = (val << 8) + c;
    valb += 8;
    while (valb >= 0) {
      out.push_back(chars[(val >> valb) & 0x3F]);
      valb -= 6;
    }
  }
  
  if (valb > -6) out.push_back(chars[((val << 8) >> (valb + 8)) & 0x3F]);
  while (out.size() % 4) out.push_back('=');
  return out;
}

std::string base64_decode(const std::string &in) {
  std::string out;
  std::vector<int> T(256, -1);
  for (int i=0; i<64; i++) {
    T[base64_chars[i]] = i;
  }
  int val = 0, valb = -8;
  for (unsigned char c : in) {
    if (T[c] == -1) {
      if (c == '=') break;
      continue;
    }
    val = (val << 6) + T[c];
    valb += 6;
    if (valb >= 0) {
      out.push_back(char((val>>valb) & 0xFF));
      valb -= 8;
    }
  }
  return out;
}
