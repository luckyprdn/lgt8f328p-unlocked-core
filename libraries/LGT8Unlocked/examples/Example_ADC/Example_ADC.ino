// Example_ADC — analog-to-digital high-level API (lgt::AdcExt)
// Koneksi: A0 = potensiometer / sensor analog.
#include <LGT8Unlocked.h>

void setup() {
  Serial.begin(115200);

  // 1) Rata-rata 16 sample — mengurangi noise
  uint16_t avg = AdcExt.readAverage(A0, 16);
  Serial.print("avg="); Serial.println(avg);

  // 2) Window monitor: true jika ADC di [100..200]
  bool inWindow = AdcExt.monitor(A0, 100, 200);
  Serial.print("window="); Serial.println(inWindow ? 1 : 0);

  // 3) PGA gain x16 (hanya 328P; 328D → Unsupported)
  lgt::Status s = AdcExt.setGain(lgt::Gain16);
  Serial.print("gain_status="); Serial.println((int)s);

  // 4) Kalibrasi offset (hanya 328P; 328D → Unsupported)
  s = AdcExt.calibrate();
  Serial.print("cal_status="); Serial.println((int)s);

  // Kembalikan PGA ke x1 supaya analogRead normal
  AdcExt.setGain(lgt::Gain1);
}

void loop() {}
