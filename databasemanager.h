#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>
#include <QVariant>
#include <QDebug>

class DatabaseManager {
public:
    static DatabaseManager& instance() {
        static DatabaseManager instance;
        return instance;
    }

    bool createTable() {
        QSqlQuery query(db);
        QString createTableSQL = R"(
            CREATE TABLE IF NOT EXISTS KeyDistances (
                Key1 TEXT,
                Key2 TEXT,
                Distance REAL
            )
        )";

        if (!query.exec(createTableSQL)) {
            qDebug() << "Ошибка создания таблицы:" << query.lastError().text();
            return false;
        }
        return true;
    }

    bool insertKeyDistance(const QString &key1, const QString &key2, double distance) {
        QSqlQuery query(db);
        query.prepare("INSERT INTO KeyDistances (Key1, Key2, Distance) VALUES (:key1, :key2, :distance)");
        query.bindValue(":key1", key1);
        query.bindValue(":key2", key2);
        query.bindValue(":distance", distance);

        if (!query.exec()) {
            qDebug() << "Ошибка вставки данных:" << query.lastError().text();
            return false;
        }
        return true;
    }

    bool isDatabaseOpen() const {
        return db.isOpen();
    }

    // Метод для получения доступа к QSqlDatabase
    QSqlDatabase& getDatabase() {
        return db;
    }

private:
    QSqlDatabase db;

    // Приватный конструктор для Singleton
    DatabaseManager(const QString &dbName = "key_distances.db",
                    const QString &dbType = "QSQLITE") {
        // Инициализация базы данных
        db = QSqlDatabase::addDatabase(dbType);
        db.setDatabaseName(dbName);

        // Подключение к базе данных
        if (!db.open()) {
            qDebug() << "Не удалось открыть базу данных:" << db.lastError().text();
        } else {
            qDebug() << "База данных успешно открыта.";
            createTable();
        }
    }

    // Приватный деструктор
    ~DatabaseManager() {
        if (db.isOpen()) {
            db.close();
            qDebug() << "База данных закрыта.";
        }
    }

    // Удаляем конструктор копирования и оператор присваивания
    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;
};

#endif // DATABASEMANAGER_H
