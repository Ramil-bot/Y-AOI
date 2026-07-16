#pragma once

namespace Data {
  struct InputVenue;
}

namespace Main {
  class Session;
}

namespace Api {
  struct MessageToSend;
  struct SendAction;

  void  SendVenue(SendAction action, Data::InputVenue venue);
}
