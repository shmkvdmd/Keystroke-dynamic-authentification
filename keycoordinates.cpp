#include "keycoordinates.h"
#include "databasemanager.h"

KeyCoordinates::KeyCoordinates() {
    fillCoordsMap();
}

double KeyCoordinates::calculateDistance(const KeyCoord& a, const KeyCoord& b) {
    int deltaX = std::abs(a.x - b.x);
    int deltaY = std::abs(a.y - b.y);

    double horizontalDistance = deltaX * (KEY_WIDTH_ + GAP_);
    double verticalDistance = deltaY * (KEY_WIDTH_ + GAP_);

    return std::sqrt(horizontalDistance * horizontalDistance + verticalDistance * verticalDistance);
}

void KeyCoordinates::fillCoordsMap() {
    key_coords_["Escape"] = {0, 0};
    key_coords_["F1"] = {2, 0};
    key_coords_["F2"] = {3, 0};
    key_coords_["F3"] = {4, 0};
    key_coords_["F4"] = {5, 0};
    key_coords_["F5"] = {7, 0};
    key_coords_["F6"] = {8, 0};
    key_coords_["F7"] = {9, 0};
    key_coords_["F8"] = {10, 0};
    key_coords_["F9"] = {12, 0};
    key_coords_["F10"] = {13, 0};
    key_coords_["F11"] = {14, 0};
    key_coords_["F12"] = {15, 0};

    key_coords_["`"] = {0, 1};
    key_coords_["1"] = {1, 1};
    key_coords_["2"] = {2, 1};
    key_coords_["3"] = {3, 1};
    key_coords_["4"] = {4, 1};
    key_coords_["5"] = {5, 1};
    key_coords_["6"] = {6, 1};
    key_coords_["7"] = {7, 1};
    key_coords_["8"] = {8, 1};
    key_coords_["9"] = {9, 1};
    key_coords_["0"] = {10, 1};
    key_coords_["-"] = {11, 1};
    key_coords_["="] = {12, 1};
    key_coords_["Backspace"] = {13, 1};

    key_coords_["Tab"] = {0, 2};
    key_coords_["Q"] = {1, 2};
    key_coords_["W"] = {2, 2};
    key_coords_["E"] = {3, 2};
    key_coords_["R"] = {4, 2};
    key_coords_["T"] = {5, 2};
    key_coords_["Y"] = {6, 2};
    key_coords_["U"] = {7, 2};
    key_coords_["I"] = {8, 2};
    key_coords_["O"] = {9, 2};
    key_coords_["P"] = {10, 2};
    key_coords_["["] = {11, 2};
    key_coords_["]"] = {12, 2};
    key_coords_["\\"] = {13, 2};

    key_coords_["Caps Lock"] = {0, 3};
    key_coords_["A"] = {1, 3};
    key_coords_["S"] = {2, 3};
    key_coords_["D"] = {3, 3};
    key_coords_["F"] = {4, 3};
    key_coords_["G"] = {5, 3};
    key_coords_["H"] = {6, 3};
}

const std::map<std::string, KeyCoord>& KeyCoordinates::getKeyCoords() const {
    return key_coords_;
}

void KeyCoordinates::FillDataBase() {
    DatabaseManager& dbManager = DatabaseManager::instance();
    QSqlDatabase& db = dbManager.getDatabase();

    if (!db.isOpen()) {
        qDebug() << "База данных не открыта!";
        return;
    }

    const auto& key_coords_ = getKeyCoords();
    QSqlQuery query(db);

    if (!db.transaction()) {
        qDebug() << "Не удалось начать транзакцию:" << db.lastError().text();
        return;
    }

    QSqlQuery checkQuery(db);
    checkQuery.prepare("SELECT COUNT(*) FROM KeyDistances WHERE Key1 = :key1 AND Key2 = :key2");

    QSqlQuery updateQuery(db);
    updateQuery.prepare("UPDATE KeyDistances SET Distance = :distance WHERE Key1 = :key1 AND Key2 = :key2");

    QSqlQuery insertQuery(db);
    insertQuery.prepare("INSERT INTO KeyDistances (Key1, Key2, Distance) VALUES (:key1, :key2, :distance)");

    qDebug() << "Начало заполнения";

    for (const auto& key1 : key_coords_) {
        for (const auto& key2 : key_coords_) {
            if (key1.first != key2.first) {
                checkQuery.bindValue(":key1", QString::fromStdString(key1.first));
                checkQuery.bindValue(":key2", QString::fromStdString(key2.first));

                if (!checkQuery.exec()) {
                    qDebug() << "Ошибка при проверке записи:" << checkQuery.lastError().text();
                    db.rollback();
                    return;
                }

                checkQuery.next();
                int count = checkQuery.value(0).toInt();

                double distance = calculateDistance(key1.second, key2.second);
                calculated_dist_[std::make_pair(key1.first, key2.first)] = distance;

                if (count == 0) {
                    insertQuery.bindValue(":key1", QString::fromStdString(key1.first));
                    insertQuery.bindValue(":key2", QString::fromStdString(key2.first));
                    insertQuery.bindValue(":distance", distance);

                    if (!insertQuery.exec()) {
                        qDebug() << "Ошибка при вставке данных о расстоянии между клавишами:" << insertQuery.lastError().text();
                        db.rollback();
                        return;
                    }
                } else {
                    updateQuery.bindValue(":key1", QString::fromStdString(key1.first));
                    updateQuery.bindValue(":key2", QString::fromStdString(key2.first));
                    updateQuery.bindValue(":distance", distance);

                    if (!updateQuery.exec()) {
                        qDebug() << "Ошибка при обновлении данных о расстоянии между клавишами:" << updateQuery.lastError().text();
                        db.rollback();
                        return;
                    }
                }
            }
        }
    }
    if (!db.commit()) {
        qDebug() << "Ошибка при фиксации транзакции:" << db.lastError().text();
    } else {
        qDebug() << "Данные успешно добавлены или обновлены в базе.";
    }
}

double KeyCoordinates::KEY_WIDTH_ = 12.5; // Ширина клавиши в мм
double KeyCoordinates::GAP_ = 7.5;        // Зазор между клавишами в мм

void KeyCoordinates::SetKeyWidth(double key_width){
    KEY_WIDTH_ = key_width;
}

void KeyCoordinates::SetGap(double gap){
    GAP_ = gap;
}
