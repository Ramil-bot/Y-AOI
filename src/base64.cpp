#include "base64.h"
#include <vector>
#include <iostream>
#include <sstream>
#include <vector>
#include <nlohmann/json.hpp>

static const char* base64_chars = 
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789+/";


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

