#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>

using namespace std;

struct Share {
    int x;
    long long y;
};

bool safe_base_convert(const string& value_str, int base, long long& result) {
    try {
        size_t idx;
        result = stoll(value_str, &idx, base);
        return idx == value_str.length();
    } catch (...) {
        return false;
    }
}

string trim(const string& s) {
    size_t start = s.find_first_not_of(" \t\n\r");
    size_t end = s.find_last_not_of(" \t\n\r");
    return (start == string::npos) ? "" : s.substr(start, end - start + 1);
}

string extract_json_value(const string& line) {
    size_t quote1 = line.find("\"", line.find(":"));
    size_t quote2 = line.find("\"", quote1 + 1);
    if (quote1 != string::npos && quote2 != string::npos)
        return line.substr(quote1 + 1, quote2 - quote1 - 1);
    return "";
}

int extract_json_int(const string& line) {
    size_t colon = line.find(":");
    if (colon != string::npos) {
        string num_str = trim(line.substr(colon + 1));
        try {
            return stoi(num_str);
        } catch (...) {}
    }
    return -1;
}

void parseJSON(const string& filename, int& n, int& k, vector<Share>& shares) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Error opening file.\n";
        exit(1);
    }

    string line;
    int currentX = 0;
    string baseStr, valueStr;
    bool insideShare = false;

    while (getline(file, line)) {
        line = trim(line);

        if (line.find("\"n\"") != string::npos) {
            n = extract_json_int(line);
        } else if (line.find("\"k\"") != string::npos) {
            k = extract_json_int(line);
        }

        else if (line.find("{") != string::npos && line.find("base") == string::npos && line.find("value") == string::npos) {
            // e.g., "1": {
            size_t quote = line.find("\"");
            size_t quote2 = line.find("\"", quote + 1);
            if (quote != string::npos && quote2 != string::npos) {
                string idxStr = line.substr(quote + 1, quote2 - quote - 1);
                try {
                    currentX = stoi(idxStr);
                    insideShare = true;
                    baseStr = "";
                    valueStr = "";
                } catch (...) {
                    insideShare = false;
                }
            }
        }

        else if (insideShare && line.find("\"base\"") != string::npos) {
            baseStr = extract_json_value(line);
        } else if (insideShare && line.find("\"value\"") != string::npos) {
            valueStr = extract_json_value(line);
        }

        else if (insideShare && line.find("}") != string::npos) {
            insideShare = false;
            int base;
            long long value;
            try {
                base = stoi(baseStr);
                if (safe_base_convert(valueStr, base, value)) {
                    shares.push_back({currentX, value});
                } else {
                    cout << "Skipping invalid share at x = " << currentX << " (value = " << valueStr << " in base " << base << ")\n";
                }
            } catch (...) {
                cout << "Skipping share at x = " << currentX << " due to base parsing error.\n";
            }
        }
    }
    file.close();
}

long long lagrangeInterpolation(const vector<Share>& shares, int k) {
    long double secret = 0.0;

    for (int i = 0; i < k; ++i) {
        long double xi = shares[i].x;
        long double yi = shares[i].y;
        long double li = 1.0;

        for (int j = 0; j < k; ++j) {
            if (i != j) {
                long double xj = shares[j].x;
                li *= (0.0 - xj) / (xi - xj);
            }
        }
        secret += yi * li;
    }

    return llround(secret);
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        cout << "Usage: " << argv[0] << " <filename.json>" << endl;
        return 1;
    }

    int n = 0, k = 0;
    vector<Share> shares;

    parseJSON(argv[1], n, k, shares);

    if ((int)shares.size() < k) {
        cout << "Not enough valid shares to compute the secret.\n";
        return 1;
    }

    long long secret = lagrangeInterpolation(shares, k);
    cout << "Secret from " << argv[1] << ": " << secret << endl;
    return 0;
}
