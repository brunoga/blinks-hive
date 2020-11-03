#include <blinklib.h>

void setup() {}

union FaceValue {
  struct {
    byte unused : 7;
    bool in_game : 1;
  };

  byte as_byte;
};

static bool in_game_;
static bool connected_[FACE_COUNT];

void loop() {
  // Update connection state.
  bool alone = true;
  FOREACH_FACE(face) {
    if (isValueReceivedOnFaceExpired(face)) {
      // It does not look like this face is connected.
      connected_[face] = false;
    } else {
      // Face seems to be connected.
      alone = false;

      // Parce incoming face value.
      FaceValue face_value;
      face_value.as_byte = getLastValueReceivedOnFace(face);

      if (in_game_ || face_value.in_game) {
        // We were already in game or the other Blink is telling us that it is
        // in game.
        in_game_ = true;
        connected_[face] = true;
      }
    }
  }

  bool button_clicked = buttonSingleClicked() && !hasWoken();

  if (alone) {
    // We are not connected to any Blinks.
    if (buttonDoubleClicked()) {
      // Reset in game state.
      in_game_ = false;

      return;
    }

    if (in_game_) {
      // And it looks like we are in game so we are being moved.
      setColor(YELLOW);
    } else {
      // Not in a game, so we wait to be clicked.
      if (button_clicked) {
        // It looks like we are the first piece in the game.
        in_game_ = true;

        return;
      }

      setColor(RED);
    }
  } else {
    // We are connected to at least one other Blink.
    if (in_game_) {
      // And we are in a game.
      setColor(GREEN);
    } else {
      // But we are not in a game.
      setColor(RED);
    }
  }

  // Publish our in game status.
  FaceValue face_value;
  face_value.in_game = in_game_;
  setValueSentOnAllFaces(face_value.as_byte);

  // Consume any pending wake status.
  hasWoken();
}