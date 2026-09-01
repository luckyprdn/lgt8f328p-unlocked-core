// QFP48 Demo Board (modelP48 variant only)
#if defined(__LGT8FX8P48__)
#define B6 22  // PB6
#define C7 23  // PC7
#define F0 24  // PF0
#define F1 33  // PF1
#define F2 34  // PF2
#define F3 35  // PF3
#define F4 36  // PF4
#define F5 37  // PF5
#define F6 38  // PF6
#define F7 39  // PF7
#else
#error "This example targets the QFP48 (48-pin) LGT8F328P variant only"
#endif

void setup() {
  pinMode(B6, OUTPUT);
  pinMode(C7, OUTPUT);
  pinMode(F0, OUTPUT);
  pinMode(F1, OUTPUT);
  pinMode(F2, OUTPUT);
  pinMode(F3, OUTPUT);
  pinMode(F4, OUTPUT);
  pinMode(F5, OUTPUT);
  pinMode(F6, OUTPUT);
  pinMode(F7, OUTPUT);
}

void loop() {
  digitalToggle(B6);
  digitalToggle(C7);
  digitalToggle(F0);
  digitalToggle(F1);
  digitalToggle(F2);
  digitalToggle(F3);
  digitalToggle(F4);
  digitalToggle(F5);
  digitalToggle(F6);
  digitalToggle(F7);
}
