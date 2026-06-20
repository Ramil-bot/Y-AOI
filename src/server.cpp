#include "server.h"

#include <rtc/rtc.hpp>
#include <algorithm>
#include <atomic>
#include <chrono>
#include <future>
#include <iostream>
#include <memory>
#include <random>
#include <stdexcept>
#include <thread>
#include <unordered_map>
#include "base64.h"
#include <nlohmann/json.hpp>


using nlohmann::json;
using namespace std::chrono_literals;
int Server(Cmdline &params) {
  rtc::Configuration config;
  
  std::string localDescription;
  std::vector<std::string> localCandidates;
  std::atomic<bool> chatRunning;

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
  std::atomic<bool> gatheringComp{false};
  if (!gatheringComp) {
  pc->onGatheringStateChange([&](rtc::PeerConnection::GatheringState state) {
      if (state == rtc::PeerConnection::GatheringState::Complete) {
      gatheringComp = true;
      }
      std::cout << "GatheringState: " << state << std::endl;
      });}

  pc->onLocalDescription([&localDescription](rtc::Description desc) {
      localDescription = std::string(desc);
      //std::cout << "localDescription: " << localDescription << std::endl;
      });


  pc->onLocalCandidate([&localCandidates](rtc::Candidate cand) {
      localCandidates.push_back(std::string(cand));
      std::cout << localCandidates.back() << std::endl;
      });
  
  auto dc = pc->createDataChannel("test");

      dc->onOpen([&]() {
        std::cout << "DataChannel opened, you can send data's" << std::endl;

        std::thread([chatRunning = &chatRunning, dc]() {
          while (true){
            std::string message;
            std::getline(std::cin, message);
            if(message == "/break"){
              *chatRunning =false;
              break;
            }
            dc->send(message);
          }
        }).detach();
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
    std::this_thread::sleep_for(100ms);
    std::cout << "Gathering :" << gatheringComp << std::endl;
  }

  json token;
  token["sdp"] = localDescription;
  token["candidate"] = localCandidates;
  std::string compat = base64_encode(token.dump());
  std::cout << "=== Token ===\n" << compat << std::endl;

  std::cout << "Put offer to the Answerer\n";
  std::cout << "Put ansewe here ->:\n";
  std::string answerStr;
  std::cin >> answerStr;
  std::cout << answerStr << std::endl;
  
  json answerJson = json::parse(base64_decode(answerStr));
  std::cout << answerJson << std::endl;
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
