#ifndef MAC_HAMONT
#warning MAC_HAMONT should be defind
#define MAC_HAMONT 0
#endif // ! MAC_HAMONT

#ifndef MAC_QMAX
#warning MAC_QMAX should be defind
#define MAC_QMAX 0
#endif // ! MAC_QMAX

#include <array>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>


const int Q_MAX_TURB = 160;

float H_aval(const float Qtot) {
  const float p1 = -1.453e-06;
  const float p2 = 0.007022;
  const float p3 = 99.98;

  return p1 * Qtot * Qtot + p2 * Qtot + p3;
}

std::array<std::function<float(float, float)>, 5> P_turb = {
    [](float x, float y) -> float {
      const float p00 = -233.3;
      const float p10 = 13.44;
      const float p01 = 0.1889;
      const float p20 = -0.1935;
      const float p11 = -0.02236;
      const float p02 = 0.005538;
      const float p21 = 0.0004944;
      const float p12 = -3.527e-05;
      const float p03 = -1.594e-05;
      return p00 + p10 * x + p01 * y + p20 * x * x + p11 * x * y + p02 * y * y +
             p21 * x * x * y + p12 * x * y * y + p03 * y * y * y;
    },
    [](float x, float y) -> float {
      const float p00 = -1.382;
      const float p10 = 0.09969;
      const float p01 = -1.945;
      const float p20 = -0.001724;
      const float p11 = 0.09224;
      const float p02 = 0.007721;
      const float p21 = -0.001096;
      const float p12 = -6.622e-05;
      const float p03 = -1.933e-05;
      return p00 + p10 * x + p01 * y + p20 * x * x + p11 * x * y + p02 * y * y +
             p21 * x * x * y + p12 * x * y * y + p03 * y * y * y;
    },
    [](float x, float y) -> float {
      const float p00 = -102.4;
      const float p10 = 5.921;
      const float p01 = -0.5012;
      const float p20 = -0.08557;
      const float p11 = 0.02467;
      const float p02 = 0.003842;
      const float p21 = -0.0002079;
      const float p12 = -2.209e-05;
      const float p03 = -1.179e-05;
      return p00 + p10 * x + p01 * y + p20 * x * x + p11 * x * y + p02 * y * y +
             p21 * x * x * y + p12 * x * y * y + p03 * y * y * y;
    },
    [](float x, float y) -> float {
      const float p00 = -514.8;
      const float p10 = 29.72;
      const float p01 = 2.096;
      const float p20 = -0.4288;
      const float p11 = -0.1336;
      const float p02 = 0.005654;
      const float p21 = 0.002048;
      const float p12 = -5.026e-07;
      const float p03 = -1.999e-05;
      return p00 + p10 * x + p01 * y + p20 * x * x + p11 * x * y + p02 * y * y +
             p21 * x * x * y + p12 * x * y * y + p03 * y * y * y;
    },
    [](float x, float y) -> float {
      const float p00 = -212.1;
      const float p10 = 12.17;
      const float p01 = 0.004397;
      const float p20 = -0.1746;
      const float p11 = -0.006808;
      const float p02 = 0.004529;
      const float p21 = 0.0002936;
      const float p12 = -4.211e-05;
      const float p03 = -1.176e-05;
      return p00 + p10 * x + p01 * y + p20 * x * x + p11 * x * y + p02 * y * y +
             p21 * x * x * y + p12 * x * y * y + p03 * y * y * y;
    },
};

const int nbArgs = 5;

int main(int argc, char **argv) {
  // check arguments and fill variables
  double x[nbArgs];

  bool x0read = false;
  if (argc >= 2) {
    std::string x0file = argv[1];
    std::ifstream in(argv[1]);
    for (int i = 0; i < nbArgs; i++) {
      if (in.fail()) {
        std::cerr << "Error reading file " << x0file << " for x0." << std::endl;
        x0read = false;
        break;
      }
      in >> x[i];
      x0read = true;
    }
    in.close();
  }

  // Input of constants
  const float hAmont = MAC_HAMONT;
  float qMax = MAC_QMAX;

  const float Haval = H_aval(qMax);

  for (size_t i = 0; i < nbArgs; i++) {
    qMax -= x[i];
  }

  // if too much water, return infinity (ensure problem limits)
  if (qMax < 0.0) {
    std::cout << std::numeric_limits<float>::max() - 1 << std::endl;
    return 0;
  }

  float res = 0.0;

  for (size_t i = 0; i < nbArgs; i++) {
    float qi = x[i];
    const float Hchute = hAmont - Haval - 0.5e-5 * qi * qi;
    res += P_turb[i](Hchute, qi);
  }

  std::cout << -res << std::endl;

  return 0;
}