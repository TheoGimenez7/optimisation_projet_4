#include <array>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>


struct result {
  std::array<float, 5> Q;

  float Qtot() const { return Q[0] + Q[1] + Q[2] + Q[3] + Q[4]; }
  float Ptot;
};

void sanitize(std::string &str) {
  const std::size_t pos = str.find_first_of(',');
  if (pos != std::string::npos)
    str.replace(pos, 1, ".");
}

int round_q(float Q) { return static_cast<int>(std::ceil(Q / 5.f)) * 5; }

struct data {
  int Qmax;
  float Hamont;
  std::array<int, 5> Q;
  std::array<float, 5> P;

  int Qtot() const { return Q[0] + Q[1] + Q[2] + Q[3] + Q[4]; }
  float Ptot() const { return P[0] + P[1] + P[2] + P[3] + P[4]; }
};

std::ostream &operator<<(std::ostream &out, const data &dat) {
  out << "Qmax : " << dat.Qmax << std::endl;
  out << "Hamont : " << dat.Hamont << std::endl;
  out << "Q : [" << dat.Q[0] << ", " << dat.Q[1] << ", " << dat.Q[2] << ", "
      << dat.Q[3] << ", " << dat.Q[4] << "] tot : " << dat.Qtot() << " m^3/s"
      << std::endl;
  out << "P : [" << dat.P[0] << ", " << dat.P[1] << ", " << dat.P[2] << ", "
      << dat.P[3] << ", " << dat.P[4] << "] tot : " << dat.Ptot() << " MW"
      << std::endl;

  return out;
}

namespace fs = std::filesystem;

std::vector<data> read_csv(fs::path path, std::size_t to_read) {
  std::vector<data> lines{};
  lines.reserve(to_read);

  if (!fs::is_regular_file(path))
    return lines;

  std::ifstream in(path);

  if (!in.is_open())
    return lines;

  const std::uintmax_t FILE_SIZE = fs::file_size(path);

  // begin reading //

  // skip 3 header lines
  in.ignore(FILE_SIZE, '\n');
  in.ignore(FILE_SIZE, '\n');
  in.ignore(FILE_SIZE, '\n');

  for (std::size_t i = 0; i < to_read; ++i) {
    std::string Qtot_str;
    std::string Hamont_str;
    std::array<std::string, 5> P_str;
    std::array<std::string, 5> Q_str;

    in.ignore(FILE_SIZE, ';');         // date
    in.ignore(FILE_SIZE, ';');         // Elav
    std::getline(in, Qtot_str, ';');   // Qtot
    in.ignore(FILE_SIZE, ';');         // Qturb
    in.ignore(FILE_SIZE, ';');         // Qvan
    std::getline(in, Hamont_str, ';'); // Hamont

    for (std::size_t j = 0; j < 5; ++j) {
      std::getline(in, Q_str[j], ';'); // Qi
      std::getline(in, P_str[j], ';'); // Pi
    }

    in.ignore(FILE_SIZE, '\n');

    sanitize(Qtot_str);
    sanitize(Hamont_str);

    const int Qmax = round_q(std::stof(Qtot_str));
    const float Hamont = std::stof(Hamont_str);

    std::array<int, 5> Q;
    std::array<float, 5> P;

    for (std::size_t j = 0; j < 5; ++j) {
      sanitize(Q_str[j]);
      sanitize(P_str[j]);
      Q[j] = static_cast<int>(std::round(std::stof(Q_str[j])));
      P[j] = std::stof(P_str[j]);
    }

    lines.push_back({
        .Qmax = Qmax,
        .Hamont = Hamont,
        .Q = Q,
        .P = P,
    });
  }

  return lines;
}

void write_csv_head(std::ostream &out) {
  out << "Qmax"
      << ";"
      << "Hamont"
      << ";"
      << "Q1"
      << ";"
      << "Q2"
      << ";"
      << "Q3"
      << ";"
      << "Q4"
      << ";"
      << "Q5"
      << ";"
      << "Ptot"
      << ";"
      << "Qtot"
      << ";"
      << ";"
      << ";"
      << "Qreel1"
      << ";"
      << "Qreel2"
      << ";"
      << "Qreel3"
      << ";"
      << "Qreel4"
      << ";"
      << "Qreel5"
      << ";"
      << "Preel_tot"
      << ";"
      << "Qreel_tot" << std::endl;
}
void write_csv_line(std::ostream &out, const data &dat, const result &res) {
  out << dat.Qmax << ";" << dat.Hamont << ";" << res.Q[1 - 1] << ";"
      << res.Q[2 - 1] << ";" << res.Q[3 - 1] << ";" << res.Q[4 - 1] << ";"
      << res.Q[5 - 1] << ";" << res.Ptot << ";" << res.Qtot() << ";"
      << ";"
      << ";" << dat.Q[1 - 1] << ";" << dat.Q[2 - 1] << ";" << dat.Q[3 - 1]
      << ";" << dat.Q[4 - 1] << ";" << dat.Q[5 - 1] << ";" << dat.Ptot() << ";"
      << dat.Qtot() << std::endl;
}

int main(int argc, char **argv) {
  fs::path path_in("./DataProjet2022_v1.csv");
  std::vector<data> file_data = read_csv(path_in, 100);

  fs::path path_out("data_out.csv");
  for (int i = 1; fs::exists(path_out); ++i) {
    path_out.replace_filename("data_out(" + std::to_string(i) + ").csv");
  }

  std::ofstream out(path_out, std::ios::trunc);

  write_csv_head(out);

  for (const auto &line : file_data) {
    std::stringstream sstream;
    sstream << "g++ -DMAC_HAMONT=" << line.Hamont << " -DMAC_QMAX=" << line.Qmax
            << " -o projet2_NM projet2_NM.cpp --std=c++20";
    system(sstream.str().c_str());
    system(".\\nomad\\nomad.exe param_NM.txt");

    std::ifstream istream("sol.txt");

    result res{};

    istream >> res.Q[0];
    istream >> res.Q[1];
    istream >> res.Q[2];
    istream >> res.Q[3];
    istream >> res.Q[4];

    istream >> res.Ptot;
    res.Ptot = -res.Ptot;
    istream.close();

    write_csv_line(out, line, res);
  }

  out.close();

  return 0;
}
