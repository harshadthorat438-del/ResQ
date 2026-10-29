#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <mutex>
#include <filesystem>
#include "../third_party/sqlite3.h"
#include "../models/Request.h"
#include "../models/RescueTeam.h"
#include "../models/Resource.h"
#include "../models/Location.h"
#include "../models/Road.h"
#include "../models/Allocation.h"
#include "../models/Operation.h"

namespace services {

class DatabaseService {
private:
    sqlite3* db{nullptr};
    std::string dbPath{"data/disaster_system.db"};
    std::mutex dbMutex;

    DatabaseService() {
        ensureDirectory();
        open();
        initializeSchema();
    }

    void ensureDirectory() {
        try {
            std::filesystem::path p("data");
            if (!std::filesystem::exists(p)) {
                std::filesystem::create_directories(p);
            }
        } catch (...) {}
    }

public:
    static DatabaseService& getInstance() {
        static DatabaseService instance;
        return instance;
    }

    ~DatabaseService() {
        close();
    }

    // Disable copy and assignment
    DatabaseService(const DatabaseService&) = delete;
    DatabaseService& operator=(const DatabaseService&) = delete;

    bool open() {
        std::lock_guard<std::mutex> lock(dbMutex);
        if (db) return true;
        int rc = sqlite3_open(dbPath.c_str(), &db);
        if (rc != SQLITE_OK) {
            std::cerr << "[DATABASE ERROR] Failed to open SQLite DB at " << dbPath 
                      << ": " << sqlite3_errmsg(db) << "\n";
            return false;
        }
        // Enable WAL mode and foreign keys for high reliability and performance
        char* err = nullptr;
        sqlite3_exec(db, "PRAGMA journal_mode = WAL;", nullptr, nullptr, &err);
        sqlite3_exec(db, "PRAGMA foreign_keys = ON;", nullptr, nullptr, &err);
        if (err) sqlite3_free(err);
        return true;
    }

    bool openCustom(const std::string& customPath) {
        std::lock_guard<std::mutex> lock(dbMutex);
        if (db) {
            sqlite3_close(db);
            db = nullptr;
        }
        dbPath = customPath;
        int rc = sqlite3_open(dbPath.c_str(), &db);
        if (rc != SQLITE_OK) {
            std::cerr << "[DATABASE ERROR] Failed to open SQLite DB at " << dbPath 
                      << ": " << sqlite3_errmsg(db) << "\n";
            return false;
        }
        char* err = nullptr;
        sqlite3_exec(db, "PRAGMA journal_mode = WAL;", nullptr, nullptr, &err);
        sqlite3_exec(db, "PRAGMA foreign_keys = ON;", nullptr, nullptr, &err);
        if (err) sqlite3_free(err);
        return true;
    }

    void close() {
        std::lock_guard<std::mutex> lock(dbMutex);
        if (db) {
            sqlite3_close(db);
            db = nullptr;
        }
    }

    bool initializeSchema() {
        std::lock_guard<std::mutex> lock(dbMutex);
        if (!db) return false;

        const char* schemaSql = R"(
            -- 1. Disaster Reports Table
            CREATE TABLE IF NOT EXISTS disaster_reports (
                id TEXT PRIMARY KEY,
                person_name TEXT NOT NULL,
                location TEXT NOT NULL,
                emergency_type TEXT NOT NULL,
                priority TEXT NOT NULL,
                affected_people INTEGER NOT NULL DEFAULT 1,
                immediate_rescue_count INTEGER NOT NULL DEFAULT 0,
                required_resource_type TEXT NOT NULL,
                required_resource_qty INTEGER NOT NULL DEFAULT 1,
                required_capability TEXT NOT NULL,
                description TEXT,
                timestamp TEXT NOT NULL,
                status TEXT NOT NULL DEFAULT 'Pending',
                assigned_team_id TEXT DEFAULT '',
                urgency_score REAL NOT NULL DEFAULT 0.0,
                escalated INTEGER NOT NULL DEFAULT 0
            );

            CREATE INDEX IF NOT EXISTS idx_reports_status ON disaster_reports(status);
            CREATE INDEX IF NOT EXISTS idx_reports_urgency ON disaster_reports(urgency_score);
            CREATE INDEX IF NOT EXISTS idx_reports_location ON disaster_reports(location);

            -- 2. Rescue Teams Table
            CREATE TABLE IF NOT EXISTS rescue_teams (
                id TEXT PRIMARY KEY,
                name TEXT NOT NULL,
                type TEXT NOT NULL,
                current_location TEXT NOT NULL,
                capacity INTEGER NOT NULL DEFAULT 6,
                equipment TEXT,
                status TEXT NOT NULL DEFAULT 'Available',
                assigned_request_id TEXT DEFAULT '',
                assigned_zone TEXT DEFAULT '',
                eta_minutes REAL DEFAULT 0.0
            );

            CREATE INDEX IF NOT EXISTS idx_teams_status ON rescue_teams(status);
            CREATE INDEX IF NOT EXISTS idx_teams_capability ON rescue_teams(type);

            -- 3. Resources Table
            CREATE TABLE IF NOT EXISTS resources (
                id TEXT PRIMARY KEY,
                name TEXT UNIQUE NOT NULL,
                category TEXT NOT NULL,
                quantity INTEGER NOT NULL DEFAULT 0,
                min_threshold INTEGER NOT NULL DEFAULT 10,
                unit TEXT NOT NULL DEFAULT 'units'
            );

            CREATE INDEX IF NOT EXISTS idx_resources_name ON resources(name);

            -- 4. Locations Table
            CREATE TABLE IF NOT EXISTS locations (
                id TEXT PRIMARY KEY,
                name TEXT UNIQUE NOT NULL,
                type TEXT NOT NULL,
                x REAL NOT NULL DEFAULT 400.0,
                y REAL NOT NULL DEFAULT 250.0
            );

            CREATE INDEX IF NOT EXISTS idx_locations_name ON locations(name);

            -- 5. Roads Table
            CREATE TABLE IF NOT EXISTS roads (
                id TEXT PRIMARY KEY,
                from_location TEXT NOT NULL,
                to_location TEXT NOT NULL,
                distance REAL NOT NULL,
                status TEXT NOT NULL DEFAULT 'Open'
            );

            CREATE INDEX IF NOT EXISTS idx_roads_endpoints ON roads(from_location, to_location);

            -- 6. Rescue Operations Table
            CREATE TABLE IF NOT EXISTS rescue_operations (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                report_id TEXT NOT NULL,
                team_id TEXT NOT NULL,
                status TEXT NOT NULL,
                dispatched_at TEXT NOT NULL,
                completed_at TEXT,
                notes TEXT
            );

            CREATE INDEX IF NOT EXISTS idx_operations_report ON rescue_operations(report_id);

            -- 7. Resource Allocations Table
            CREATE TABLE IF NOT EXISTS resource_allocations (
                id TEXT PRIMARY KEY,
                request_id TEXT NOT NULL,
                person_name TEXT,
                location TEXT,
                resource_name TEXT NOT NULL,
                quantity INTEGER NOT NULL,
                timestamp TEXT NOT NULL,
                notes TEXT
            );

            CREATE INDEX IF NOT EXISTS idx_allocations_request ON resource_allocations(request_id);

            -- 8. Operation History Table
            CREATE TABLE IF NOT EXISTS operation_history (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                timestamp TEXT NOT NULL,
                description TEXT NOT NULL,
                category TEXT NOT NULL,
                related_id TEXT
            );

            CREATE INDEX IF NOT EXISTS idx_history_timestamp ON operation_history(timestamp);
        )";

        char* errMsg = nullptr;
        int rc = sqlite3_exec(db, schemaSql, nullptr, nullptr, &errMsg);
        if (rc != SQLITE_OK) {
            std::cerr << "[DATABASE SCHEMA ERROR] " << (errMsg ? errMsg : "Unknown error") << "\n";
            if (errMsg) sqlite3_free(errMsg);
            return false;
        }

        std::cout << "[DATABASE] SQLite relational schema verified at " << dbPath << ".\n";
        return true;
    }

    bool isDatabaseEmpty() {
        std::lock_guard<std::mutex> lock(dbMutex);
        if (!db) return true;

        const char* query = "SELECT (SELECT COUNT(*) FROM disaster_reports) + "
                            "(SELECT COUNT(*) FROM rescue_teams) + "
                            "(SELECT COUNT(*) FROM resources) + "
                            "(SELECT COUNT(*) FROM locations);";
        sqlite3_stmt* stmt = nullptr;
        int count = 0;
        if (sqlite3_prepare_v2(db, query, -1, &stmt, nullptr) == SQLITE_OK) {
            if (sqlite3_step(stmt) == SQLITE_ROW) {
                count = sqlite3_column_int(stmt, 0);
            }
            sqlite3_finalize(stmt);
        }
        return count == 0;
    }

    // =========================================================================
    // Transaction Support
    // =========================================================================
    bool beginTransaction() {
        char* err = nullptr;
        int rc = sqlite3_exec(db, "BEGIN TRANSACTION;", nullptr, nullptr, &err);
        if (rc != SQLITE_OK && err) sqlite3_free(err);
        return rc == SQLITE_OK;
    }

    bool commitTransaction() {
        char* err = nullptr;
        int rc = sqlite3_exec(db, "COMMIT;", nullptr, nullptr, &err);
        if (rc != SQLITE_OK && err) sqlite3_free(err);
        return rc == SQLITE_OK;
    }

    bool rollbackTransaction() {
        char* err = nullptr;
        int rc = sqlite3_exec(db, "ROLLBACK;", nullptr, nullptr, &err);
        if (rc != SQLITE_OK && err) sqlite3_free(err);
        return rc == SQLITE_OK;
    }

    // =========================================================================
    // 1. Disaster Reports Operations
    // =========================================================================
    bool insertReport(const models::Request& r) {
        std::lock_guard<std::mutex> lock(dbMutex);
        if (!db) return false;

        const char* sql = R"(
            INSERT INTO disaster_reports (
                id, person_name, location, emergency_type, priority,
                affected_people, immediate_rescue_count, required_resource_type,
                required_resource_qty, required_capability, description,
                timestamp, status, assigned_team_id, urgency_score, escalated
            ) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
            ON CONFLICT(id) DO UPDATE SET
                person_name=excluded.person_name,
                location=excluded.location,
                emergency_type=excluded.emergency_type,
                priority=excluded.priority,
                affected_people=excluded.affected_people,
                immediate_rescue_count=excluded.immediate_rescue_count,
                required_resource_type=excluded.required_resource_type,
                required_resource_qty=excluded.required_resource_qty,
                required_capability=excluded.required_capability,
                description=excluded.description,
                timestamp=excluded.timestamp,
                status=excluded.status,
                assigned_team_id=excluded.assigned_team_id,
                urgency_score=excluded.urgency_score,
                escalated=excluded.escalated;
        )";

        sqlite3_stmt* stmt = nullptr;
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;

        sqlite3_bind_text(stmt, 1, r.id.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, r.personName.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 3, r.location.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 4, r.emergencyType.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 5, r.priority.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt, 6, r.affectedPeople);
        sqlite3_bind_int(stmt, 7, r.immediateRescueCount);
        sqlite3_bind_text(stmt, 8, r.requiredResourceType.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt, 9, r.requiredResourceQty);
        sqlite3_bind_text(stmt, 10, r.requiredCapability.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 11, r.description.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 12, r.timestamp.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 13, r.status.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 14, r.assignedTeamId.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_double(stmt, 15, r.calculateUrgencyScore());
        sqlite3_bind_int(stmt, 16, r.escalated ? 1 : 0);

        int rc = sqlite3_step(stmt);
        sqlite3_finalize(stmt);
        return rc == SQLITE_DONE;
    }

    bool updateReportStatus(const std::string& id, const std::string& status, const std::string& teamId = "") {
        std::lock_guard<std::mutex> lock(dbMutex);
        if (!db) return false;

        std::string sql;
        if (!teamId.empty()) {
            sql = "UPDATE disaster_reports SET status = ?, assigned_team_id = ? WHERE id = ?;";
        } else {
            sql = "UPDATE disaster_reports SET status = ? WHERE id = ?;";
        }

        sqlite3_stmt* stmt = nullptr;
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) return false;

        sqlite3_bind_text(stmt, 1, status.c_str(), -1, SQLITE_TRANSIENT);
        if (!teamId.empty()) {
            sqlite3_bind_text(stmt, 2, teamId.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 3, id.c_str(), -1, SQLITE_TRANSIENT);
        } else {
            sqlite3_bind_text(stmt, 2, id.c_str(), -1, SQLITE_TRANSIENT);
        }

        int rc = sqlite3_step(stmt);
        sqlite3_finalize(stmt);
        return rc == SQLITE_DONE;
    }

    bool escalateReport(const std::string& id) {
        std::lock_guard<std::mutex> lock(dbMutex);
        if (!db) return false;

        const char* sql = "UPDATE disaster_reports SET priority = 'Critical', escalated = 1, "
                          "urgency_score = urgency_score + 60.0 WHERE id = ?;";
        sqlite3_stmt* stmt = nullptr;
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;

        sqlite3_bind_text(stmt, 1, id.c_str(), -1, SQLITE_TRANSIENT);
        int rc = sqlite3_step(stmt);
        sqlite3_finalize(stmt);
        return rc == SQLITE_DONE;
    }

    std::vector<models::Request> loadAllReports() {
        std::lock_guard<std::mutex> lock(dbMutex);
        std::vector<models::Request> list;
        if (!db) return list;

        const char* sql = "SELECT id, person_name, location, emergency_type, priority, "
                          "affected_people, immediate_rescue_count, required_resource_type, "
                          "required_resource_qty, required_capability, description, "
                          "timestamp, status, assigned_team_id, urgency_score, escalated "
                          "FROM disaster_reports ORDER BY urgency_score DESC;";

        sqlite3_stmt* stmt = nullptr;
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
            while (sqlite3_step(stmt) == SQLITE_ROW) {
                models::Request r;
                r.id = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
                r.personName = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
                r.location = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
                r.emergencyType = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
                r.priority = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
                r.affectedPeople = sqlite3_column_int(stmt, 5);
                r.immediateRescueCount = sqlite3_column_int(stmt, 6);
                r.requiredResourceType = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));
                r.requiredResourceQty = sqlite3_column_int(stmt, 8);
                r.requiredCapability = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 9));
                const char* desc = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 10));
                r.description = desc ? desc : "";
                r.timestamp = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 11));
                r.status = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 12));
                const char* teamId = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 13));
                r.assignedTeamId = teamId ? teamId : "";
                r.escalated = sqlite3_column_int(stmt, 15) != 0;
                list.push_back(r);
            }
            sqlite3_finalize(stmt);
        }
        return list;
    }

    // =========================================================================
    // 2. Rescue Teams Operations
    // =========================================================================
    bool insertTeam(const models::RescueTeam& t) {
        std::lock_guard<std::mutex> lock(dbMutex);
        if (!db) return false;

        const char* sql = R"(
            INSERT INTO rescue_teams (
                id, name, type, current_location, capacity, equipment,
                status, assigned_request_id, assigned_zone, eta_minutes
            ) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
            ON CONFLICT(id) DO UPDATE SET
                name=excluded.name,
                type=excluded.type,
                current_location=excluded.current_location,
                capacity=excluded.capacity,
                equipment=excluded.equipment,
                status=excluded.status,
                assigned_request_id=excluded.assigned_request_id,
                assigned_zone=excluded.assigned_zone,
                eta_minutes=excluded.eta_minutes;
        )";

        sqlite3_stmt* stmt = nullptr;
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;

        sqlite3_bind_text(stmt, 1, t.id.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, t.name.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 3, t.type.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 4, t.currentLocation.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt, 5, t.capacity);
        sqlite3_bind_text(stmt, 6, t.equipment.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 7, t.status.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 8, t.assignedRequestId.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 9, t.assignedZone.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_double(stmt, 10, t.etaMinutes);

        int rc = sqlite3_step(stmt);
        sqlite3_finalize(stmt);
        return rc == SQLITE_DONE;
    }

    bool updateTeamStatus(const std::string& id, const std::string& status, 
                          const std::string& reqId = "", const std::string& zone = "", double eta = 0.0) {
        std::lock_guard<std::mutex> lock(dbMutex);
        if (!db) return false;

        const char* sql = "UPDATE rescue_teams SET status = ?, assigned_request_id = ?, "
                          "assigned_zone = ?, eta_minutes = ? WHERE id = ?;";
        sqlite3_stmt* stmt = nullptr;
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;

        sqlite3_bind_text(stmt, 1, status.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, reqId.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 3, zone.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_double(stmt, 4, eta);
        sqlite3_bind_text(stmt, 5, id.c_str(), -1, SQLITE_TRANSIENT);

        int rc = sqlite3_step(stmt);
        sqlite3_finalize(stmt);
        return rc == SQLITE_DONE;
    }

    std::vector<models::RescueTeam> loadAllTeams() {
        std::lock_guard<std::mutex> lock(dbMutex);
        std::vector<models::RescueTeam> list;
        if (!db) return list;

        const char* sql = "SELECT id, name, type, current_location, capacity, equipment, "
                          "status, assigned_request_id, assigned_zone, eta_minutes FROM rescue_teams;";
        sqlite3_stmt* stmt = nullptr;
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
            while (sqlite3_step(stmt) == SQLITE_ROW) {
                models::RescueTeam t;
                t.id = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
                t.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
                t.type = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
                t.currentLocation = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
                t.capacity = sqlite3_column_int(stmt, 4);
                const char* eq = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
                t.equipment = eq ? eq : "";
                t.status = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
                const char* reqId = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));
                t.assignedRequestId = reqId ? reqId : "";
                const char* zone = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 8));
                t.assignedZone = zone ? zone : "";
                t.etaMinutes = sqlite3_column_double(stmt, 9);
                list.push_back(t);
            }
            sqlite3_finalize(stmt);
        }
        return list;
    }

    // =========================================================================
    // 3. Resources Operations
    // =========================================================================
    bool insertOrUpdateResource(const models::Resource& r) {
        std::lock_guard<std::mutex> lock(dbMutex);
        if (!db) return false;

        const char* sql = R"(
            INSERT INTO resources (id, name, category, quantity, min_threshold, unit)
            VALUES (?, ?, ?, ?, ?, ?)
            ON CONFLICT(name) DO UPDATE SET
                quantity = resources.quantity + excluded.quantity,
                category = excluded.category,
                min_threshold = excluded.min_threshold,
                unit = excluded.unit;
        )";

        sqlite3_stmt* stmt = nullptr;
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;

        sqlite3_bind_text(stmt, 1, r.id.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, r.name.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 3, r.category.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt, 4, r.quantity);
        sqlite3_bind_int(stmt, 5, r.minThreshold);
        sqlite3_bind_text(stmt, 6, r.unit.c_str(), -1, SQLITE_TRANSIENT);

        int rc = sqlite3_step(stmt);
        sqlite3_finalize(stmt);
        return rc == SQLITE_DONE;
    }

    bool deductResource(const std::string& name, int qty) {
        std::lock_guard<std::mutex> lock(dbMutex);
        if (!db) return false;

        const char* sql = "UPDATE resources SET quantity = quantity - ? WHERE name = ? AND quantity >= ?;";
        sqlite3_stmt* stmt = nullptr;
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;

        sqlite3_bind_int(stmt, 1, qty);
        sqlite3_bind_text(stmt, 2, name.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt, 3, qty);

        int rc = sqlite3_step(stmt);
        int changes = sqlite3_changes(db);
        sqlite3_finalize(stmt);
        return (rc == SQLITE_DONE && changes > 0);
    }

    bool returnResource(const std::string& name, int qty) {
        std::lock_guard<std::mutex> lock(dbMutex);
        if (!db) return false;

        const char* sql = "UPDATE resources SET quantity = quantity + ? WHERE name = ?;";
        sqlite3_stmt* stmt = nullptr;
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;

        sqlite3_bind_int(stmt, 1, qty);
        sqlite3_bind_text(stmt, 2, name.c_str(), -1, SQLITE_TRANSIENT);

        int rc = sqlite3_step(stmt);
        sqlite3_finalize(stmt);
        return rc == SQLITE_DONE;
    }

    std::vector<models::Resource> loadAllResources() {
        std::lock_guard<std::mutex> lock(dbMutex);
        std::vector<models::Resource> list;
        if (!db) return list;

        const char* sql = "SELECT id, name, category, quantity, min_threshold, unit FROM resources;";
        sqlite3_stmt* stmt = nullptr;
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
            while (sqlite3_step(stmt) == SQLITE_ROW) {
                models::Resource r;
                r.id = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
                r.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
                r.category = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
                r.quantity = sqlite3_column_int(stmt, 3);
                r.minThreshold = sqlite3_column_int(stmt, 4);
                r.unit = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
                list.push_back(r);
            }
            sqlite3_finalize(stmt);
        }
        return list;
    }

    // =========================================================================
    // 4. Locations & Roads Operations
    // =========================================================================
    bool insertLocation(const models::Location& l) {
        std::lock_guard<std::mutex> lock(dbMutex);
        if (!db) return false;

        const char* sql = R"(
            INSERT INTO locations (id, name, type, x, y) VALUES (?, ?, ?, ?, ?)
            ON CONFLICT(name) DO UPDATE SET type=excluded.type, x=excluded.x, y=excluded.y;
        )";

        sqlite3_stmt* stmt = nullptr;
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;

        sqlite3_bind_text(stmt, 1, l.id.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, l.name.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 3, l.type.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_double(stmt, 4, l.x);
        sqlite3_bind_double(stmt, 5, l.y);

        int rc = sqlite3_step(stmt);
        sqlite3_finalize(stmt);
        return rc == SQLITE_DONE;
    }

    std::vector<models::Location> loadAllLocations() {
        std::lock_guard<std::mutex> lock(dbMutex);
        std::vector<models::Location> list;
        if (!db) return list;

        const char* sql = "SELECT id, name, type, x, y FROM locations;";
        sqlite3_stmt* stmt = nullptr;
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
            while (sqlite3_step(stmt) == SQLITE_ROW) {
                models::Location l;
                l.id = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
                l.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
                l.type = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
                l.x = sqlite3_column_double(stmt, 3);
                l.y = sqlite3_column_double(stmt, 4);
                list.push_back(l);
            }
            sqlite3_finalize(stmt);
        }
        return list;
    }

    bool insertRoad(const models::Road& rd) {
        std::lock_guard<std::mutex> lock(dbMutex);
        if (!db) return false;

        const char* sql = R"(
            INSERT INTO roads (id, from_location, to_location, distance, status)
            VALUES (?, ?, ?, ?, ?)
            ON CONFLICT(id) DO UPDATE SET
                from_location=excluded.from_location,
                to_location=excluded.to_location,
                distance=excluded.distance,
                status=excluded.status;
        )";

        sqlite3_stmt* stmt = nullptr;
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;

        sqlite3_bind_text(stmt, 1, rd.id.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, rd.from.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 3, rd.to.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_double(stmt, 4, rd.distance);
        sqlite3_bind_text(stmt, 5, rd.status.c_str(), -1, SQLITE_TRANSIENT);

        int rc = sqlite3_step(stmt);
        sqlite3_finalize(stmt);
        return rc == SQLITE_DONE;
    }

    bool updateRoadStatus(const std::string& roadId, const std::string& status) {
        std::lock_guard<std::mutex> lock(dbMutex);
        if (!db) return false;

        const char* sql = "UPDATE roads SET status = ? WHERE id = ?;";
        sqlite3_stmt* stmt = nullptr;
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;

        sqlite3_bind_text(stmt, 1, status.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, roadId.c_str(), -1, SQLITE_TRANSIENT);

        int rc = sqlite3_step(stmt);
        sqlite3_finalize(stmt);
        return rc == SQLITE_DONE;
    }

    std::vector<models::Road> loadAllRoads() {
        std::lock_guard<std::mutex> lock(dbMutex);
        std::vector<models::Road> list;
        if (!db) return list;

        const char* sql = "SELECT id, from_location, to_location, distance, status FROM roads;";
        sqlite3_stmt* stmt = nullptr;
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
            while (sqlite3_step(stmt) == SQLITE_ROW) {
                models::Road rd;
                rd.id = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
                rd.from = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
                rd.to = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
                rd.distance = sqlite3_column_double(stmt, 3);
                rd.status = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
                list.push_back(rd);
            }
            sqlite3_finalize(stmt);
        }
        return list;
    }

    // =========================================================================
    // 5. Allocations & Operations Operations
    // =========================================================================
    bool insertAllocation(const models::Allocation& a) {
        std::lock_guard<std::mutex> lock(dbMutex);
        if (!db) return false;

        const char* sql = R"(
            INSERT INTO resource_allocations (id, request_id, person_name, location, resource_name, quantity, timestamp, notes)
            VALUES (?, ?, ?, ?, ?, ?, ?, ?);
        )";

        sqlite3_stmt* stmt = nullptr;
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;

        sqlite3_bind_text(stmt, 1, a.id.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, a.requestId.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 3, a.personName.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 4, a.location.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 5, a.resourceName.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt, 6, a.quantity);
        sqlite3_bind_text(stmt, 7, a.timestamp.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 8, a.notes.c_str(), -1, SQLITE_TRANSIENT);

        int rc = sqlite3_step(stmt);
        sqlite3_finalize(stmt);
        return rc == SQLITE_DONE;
    }

    std::vector<models::Allocation> loadAllAllocations() {
        std::lock_guard<std::mutex> lock(dbMutex);
        std::vector<models::Allocation> list;
        if (!db) return list;

        const char* sql = "SELECT id, request_id, person_name, location, resource_name, quantity, timestamp, notes "
                          "FROM resource_allocations ORDER BY id ASC;";
        sqlite3_stmt* stmt = nullptr;
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
            while (sqlite3_step(stmt) == SQLITE_ROW) {
                models::Allocation a;
                a.id = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
                a.requestId = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
                const char* pn = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
                a.personName = pn ? pn : "";
                const char* loc = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
                a.location = loc ? loc : "";
                a.resourceName = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
                a.quantity = sqlite3_column_int(stmt, 5);
                a.timestamp = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
                const char* notes = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));
                a.notes = notes ? notes : "";
                list.push_back(a);
            }
            sqlite3_finalize(stmt);
        }
        return list;
    }

    bool insertOperationRecord(const std::string& reportId, const std::string& teamId, 
                               const std::string& status, const std::string& notes = "") {
        std::lock_guard<std::mutex> lock(dbMutex);
        if (!db) return false;

        const char* sql = "INSERT INTO rescue_operations (report_id, team_id, status, dispatched_at, notes) "
                          "VALUES (?, ?, ?, datetime('now', 'localtime'), ?);";
        sqlite3_stmt* stmt = nullptr;
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;

        sqlite3_bind_text(stmt, 1, reportId.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, teamId.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 3, status.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 4, notes.c_str(), -1, SQLITE_TRANSIENT);

        int rc = sqlite3_step(stmt);
        sqlite3_finalize(stmt);
        return rc == SQLITE_DONE;
    }

    bool updateOperationRecordStatus(const std::string& reportId, const std::string& newStatus) {
        std::lock_guard<std::mutex> lock(dbMutex);
        if (!db) return false;

        const char* sql = "UPDATE rescue_operations SET status = ?, completed_at = datetime('now', 'localtime') "
                          "WHERE report_id = ? AND status != 'Completed' AND status != 'Cancelled';";
        sqlite3_stmt* stmt = nullptr;
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;

        sqlite3_bind_text(stmt, 1, newStatus.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, reportId.c_str(), -1, SQLITE_TRANSIENT);

        int rc = sqlite3_step(stmt);
        sqlite3_finalize(stmt);
        return rc == SQLITE_DONE;
    }

    // =========================================================================
    // 6. Operation History Operations
    // =========================================================================
    bool insertHistory(const models::Operation& op) {
        std::lock_guard<std::mutex> lock(dbMutex);
        if (!db) return false;

        const char* sql = "INSERT INTO operation_history (timestamp, description, category, related_id) "
                          "VALUES (?, ?, ?, ?);";
        sqlite3_stmt* stmt = nullptr;
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;

        sqlite3_bind_text(stmt, 1, op.timestamp.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, op.description.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 3, op.category.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 4, op.relatedId.c_str(), -1, SQLITE_TRANSIENT);

        int rc = sqlite3_step(stmt);
        sqlite3_finalize(stmt);
        return rc == SQLITE_DONE;
    }

    std::vector<models::Operation> loadAllHistory() {
        std::lock_guard<std::mutex> lock(dbMutex);
        std::vector<models::Operation> list;
        if (!db) return list;

        const char* sql = "SELECT id, timestamp, description, category, related_id FROM operation_history "
                          "ORDER BY id DESC;";
        sqlite3_stmt* stmt = nullptr;
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
            while (sqlite3_step(stmt) == SQLITE_ROW) {
                models::Operation op;
                op.id = std::to_string(sqlite3_column_int(stmt, 0));
                op.timestamp = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
                op.description = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
                op.category = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
                const char* rel = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
                op.relatedId = rel ? rel : "";
                list.push_back(op);
            }
            sqlite3_finalize(stmt);
        }
        return list;
    }

    // =========================================================================
    // 7. Clear All Data (for Demo reset)
    // =========================================================================
    bool clearAllData() {
        std::lock_guard<std::mutex> lock(dbMutex);
        if (!db) return false;

        char* err = nullptr;
        sqlite3_exec(db, "DELETE FROM disaster_reports;", nullptr, nullptr, &err);
        sqlite3_exec(db, "DELETE FROM rescue_teams;", nullptr, nullptr, &err);
        sqlite3_exec(db, "DELETE FROM resources;", nullptr, nullptr, &err);
        sqlite3_exec(db, "DELETE FROM locations;", nullptr, nullptr, &err);
        sqlite3_exec(db, "DELETE FROM roads;", nullptr, nullptr, &err);
        sqlite3_exec(db, "DELETE FROM rescue_operations;", nullptr, nullptr, &err);
        sqlite3_exec(db, "DELETE FROM resource_allocations;", nullptr, nullptr, &err);
        sqlite3_exec(db, "DELETE FROM operation_history;", nullptr, nullptr, &err);
        if (err) sqlite3_free(err);
        return true;
    }
};

} // namespace services
