#ifndef KEYCOORDINATES_H
#define KEYCOORDINATES_H

#include <map>
#include <string>
#include <cmath>

struct KeyCoord {
    int x;
    int y;
};

class KeyCoordinates {
public:
    KeyCoordinates();
    double calculateDistance(const KeyCoord& a, const KeyCoord& b);
    void fillCoordsMap();
    void FillDataBase();
    static void SetKeyWidth(double);
    static void SetGap(double);
    const std::map<std::string, KeyCoord>& getKeyCoords() const;

private:
    static double KEY_WIDTH_;
    static double GAP_;
    std::map<std::string, KeyCoord> key_coords_;
    std::map<std::pair<std::string, std::string>, double> calculated_dist_;
};

#endif // KEYCOORDINATES_H
