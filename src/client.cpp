#include "client.h"

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
int Client() {
  rtc::Configuration config;
  auto pc = std::make_shared<rtc::PeerConnection>(config);


  std::string localDescription;
  std::vector<std::string> localCandidates;
  std::atomic<bool> gatheringComp{false};
  std::atomic<bool> chatRunning{true};
 
  pc->onGatheringStateChange([&](rtc::PeerConnection::GatheringState state) {
      if (state == rtc::PeerConnection::GatheringState::Complete) {
        gatheringComp = true;
        std::cout<< "<<----------------Gathering comp changed to true ------->>" << std::endl;
      }
      std::cout << "GatheringState: " << state << std::endl;
      }); 

  pc->onLocalDescription([&localDescription](rtc::Description desc) {
      localDescription = std::string(desc);
     // std::cout << "localDescription: " << localDescription << std::endl;
      });

  pc->onLocalCandidate([&localCandidates](rtc::Candidate cand) {
      localCandidates.push_back(std::string(cand));
      std::cout << localCandidates.back() << std::endl;
      });


  pc->onStateChange([](rtc::PeerConnection::State state) {
      std::cout << "State: " << state << std::endl;
      });

  pc->onDataChannel([&](std::shared_ptr<rtc::DataChannel> dc) {
      std::cout << "Given datachannel: " << dc->label() << std::endl;
      
      dc->onOpen([&, dc]() {
          std::cout << "DataChannel is opened (Answerer)" << std::endl;
          dc->send("Hello world");
          std::thread([chatRunning = &chatRunning, dc] {
            while(true) {
              std::string message;
              std::getline(std::cin, message);
              if (message == "/break"){
                *chatRunning = false;
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
  }); 
  
  std::string compat;
  std::getline(std::cin, compat);
  
  json recv = json::parse(base64_decode(compat));
  //std::cout << recv << std::endl;
    
  pc->setRemoteDescription({recv["sdp"].get<std::string>(), "offer"});
  for (const auto &cand : recv["candidate"])
    pc->addRemoteCandidate({cand.get<std::string>()});

  pc->setLocalDescription();
  while (!gatheringComp) {
    std::this_thread::sleep_for(100ms);
    std::cout << "Waiting gathering state: " << gatheringComp << std::endl;
  }
  

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
